#include "moeengine.h"
#include "moeobject.h"

#include <QMetaProperty>
#include <QScriptEngine>
#include <QStringList>
#include <QDebug>
#include <QColor>
#include <QBrush>
#include <QEvent>

#include <qmath.h>

struct AnimationState {
    QMetaProperty metaProp;
    QScriptValue callback;
    QVariant target, last;
    qreal modifier;
};

QThreadStorage<MoeObjectPtrMap> MoeObject::instances;

MoeObject::MoeObject(MoeObject* parent) : QObject(parent)
{
    Q_ASSERT(!MoeClientEngine::current().isNull());
    moveToThread(MoeClientEngine::current().data());
    instances.localData().insert(ptr(), this);
    connect(engine(), SIGNAL(cleanup()), this, SLOT(cleanup()));
    connect(engine(), SIGNAL(destroyed()), this, SLOT(deleteLater()));
    connect(engine()->scriptEngine(), SIGNAL(destroyed()), this, SLOT(deleteLater()));
}

MoeClientEngine* MoeObject::engine() const
{
    return qobject_cast<MoeClientEngine*>(thread());
}

QString MoeObject::toString() const {
    QString toString(metaObject()->className());
    toString += '(';
    for(int i=0; i<metaObject()->propertyCount(); i++) {
        if(i > 0)
            toString += ", ";

        QString debugBuffer;
        QDebug debug(&debugBuffer);
        QMetaProperty prop = metaObject()->property(i);
        debug << prop.read(this).toString();
        toString += QString("%1 = %2").arg(prop.name(), debugBuffer);
    }
    toString += ")";

    return toString;
}

void MoeObject::animate(QString key, QScriptValue to, QScriptValue callback, qreal modifier) {
    bool addToList = false;
    AnimationStatePointer state = animations.value(key);
    if(!state.data()) {
        int propIndex = metaObject()->indexOfProperty(key.toLocal8Bit().data());
        if(!propIndex) {
            engine()->scriptEngine()->currentContext()->throwError(QString("Native property '%1' does not exist in '%2' or any of its parents.").arg(key).arg(metaObject()->className()));
            return;
        }

        state = AnimationStatePointer(new AnimationState);
        state->metaProp = metaObject()->property(propIndex);
        addToList = true;
    }

    switch(state->metaProp.type()) {
        case QVariant::Double:
            state->target = to.toNumber();
            break;

        case QVariant::Color:
            state->target = QVariant::fromValue<QColor>(engine()->scriptEngine()->fromScriptValue<QColor>(to));
            break;

        case QVariant::Brush:
            state->target = QVariant::fromValue<QBrush>(engine()->scriptEngine()->fromScriptValue<QBrush>(to));
            break;

        default:
        {
            engine()->scriptEngine()->currentContext()->throwError(QString("Type '%1' cannot be animated. Cannot animate property '%2'.").arg(state->metaProp.typeName()).arg(key));
            animations.remove(key);
            return;
        }
    }

    if(callback.isFunction()) {
        state->callback = callback;
        state->modifier = modifier;
    } else {
        state->modifier = !callback.isUndefined() && !callback.isNull() &&
                callback.isNumber() ? callback.toNumber() : modifier;
        state->callback = QScriptValue();
    }

    state->last = state->metaProp.read(this);
    if(addToList) {
        animations.insert(key, state);
        if(!animateConnection)
            animateConnection = connect(engine(), SIGNAL(preciseTick(qreal)), this, SLOT(animateTick(qreal)), Qt::UniqueConnection);
    }
}

enum SmoothResultFlag {
    NoChange = 0x0,
    ValueChanged = 0x1,
    ValueFarFromTarget = 0x2
};

Q_DECLARE_FLAGS(SoothResultFlags, SmoothResultFlag)

#define ANIMATE_START_GROUP(T) T val = metaProp.read(this).value<T>(); \
if(val != state->last.value<T>()) \
    break; \
T target = state->target.value<T>();

#define ANIMATE_PROPERTY_REAL(READ, WRITE) qreal READ = val.READ(); \
    result |= smoothReal(READ, target.READ(), state->modifier); \
    if(result.testFlag(ValueChanged)) \
        val.WRITE(READ);

#define ANIMATE_PROPERTY_INT(READ, WRITE) int READ = val.READ(); \
    result |= smoothInt(READ, target.READ(), state->modifier); \
    if(result.testFlag(ValueChanged)) \
        val.WRITE(READ);

#define ANIMATE_END(T) if(result.testFlag(ValueChanged)) { \
    metaProp.write(this, val); \
    state->last = val; \
}

#define ANIMATE_REAL() qreal val = state->metaProp.read(this).toReal(); \
if(val != state->last.toReal()) \
    break; \
result = smoothReal(val, state->target.toReal(), state->modifier); \
ANIMATE_END(qreal)

inline SoothResultFlags smoothReal(qreal& value, qreal to, qreal modifier) {
    if(value < to) {
        value += (qreal)qCeil((to - value)/modifier*100.0)/100.0;
        if(value >= to) {
            value = to;
            return ValueChanged;
        }
    } else if(value > to) {
        value -= (qreal)qCeil((value - to)/modifier*100.0)/100.0;
        if(value <= to) {
            value = to;
            return ValueChanged;
        }
    } else
        return NoChange;

    return SoothResultFlags(ValueChanged | ValueFarFromTarget);
}

inline SoothResultFlags smoothInt(int& value, int to, qreal modifier) {
    if(value < to) {
        value += qCeil((qreal)(to - value)/modifier);
        if(value >= to) {
            value = to;
            return ValueChanged;
        }
    } else if(value > to) {
        value -= qCeil((qreal)(value - to)/modifier);
        if(value <= to) {
            value = to;
            return ValueChanged;
        }
    } else
        return NoChange;

    return SoothResultFlags(ValueChanged | ValueFarFromTarget);
}

void MoeObject::cleanup() {
    qDebug() << "Cleaning up" << this;
    deleteLater();
}

void MoeObject::animateTick(qreal) {
    if(!animateConnection)
        return;

    QList<AnimationStatePointer> _animations = animations.values();
    QList<QScriptValue> callbacks;
    animations.clear();

    foreach(AnimationStatePointer state, _animations) {
        QMetaProperty metaProp = state->metaProp;
        SoothResultFlags result = NoChange;
        switch(metaProp.type()) {
            case QVariant::Brush:
            {
                QBrush bval = state->metaProp.read(this).value<QBrush>();
                QBrush last = state->last.value<QBrush>();
                QBrush btarget = state->target.value<QBrush>();

                if(!last.gradient() && !btarget.gradient()) { //TODO: Implement Gradient Handling
                    if(bval.gradient()) // Value changed to gradient
                        break;

                    QColor val = bval.color();
                    if(val != last.color())
                        break;
                    QColor target = btarget.color();

                    ANIMATE_PROPERTY_INT(red, setRed)
                    ANIMATE_PROPERTY_INT(green, setGreen)
                    ANIMATE_PROPERTY_INT(blue, setBlue)
                    ANIMATE_PROPERTY_INT(alpha, setAlpha)

                    bval = QBrush(val);
                } else {
                    result |= ValueChanged;
                    bval = btarget;
                }

                if(result.testFlag(ValueChanged)) {
                    metaProp.write(this, bval);
                    state->last = bval;
                }

                break;
            }

            case QVariant::Color:
            {
                ANIMATE_START_GROUP(QColor)
                ANIMATE_PROPERTY_INT(red, setRed)
                ANIMATE_PROPERTY_INT(green, setGreen)
                ANIMATE_PROPERTY_INT(blue, setBlue)
                ANIMATE_PROPERTY_INT(alpha, setAlpha)
                ANIMATE_END(QColor)

                break;
            }

            case QVariant::Double:
            {
                ANIMATE_REAL()
                break;
            }

            default:
                break;
        }

        if(result.testFlag(ValueFarFromTarget))
            animations.insert(state->metaProp.name(), state);
        else if(state->callback.isFunction())
            callbacks << state->callback;
    }

    if(!callbacks.isEmpty())
        foreach(QScriptValue callback, callbacks)
            callback.call();

    if(animations.isEmpty())
        disconnect(animateConnection);
}

void MoeObject::killAnimation(QString key) {
    animations.take(key);
    if(animations.isEmpty())
        disconnect(animateConnection);
}
