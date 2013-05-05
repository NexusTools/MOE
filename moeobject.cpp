#include "moeengine.h"
#include "moeobject.h"
#include <QMetaProperty>
#include <QScriptEngine>
#include <QStringList>
#include <QDebug>
#include <QColor>
#include <qmath.h>

struct AnimationState {
    QMetaProperty metaProp;
    QScriptValue callback;
    QVariant target;
    qreal modifier;
};

QThreadStorage<MoeObjectPtrMap> MoeObject::instances;

MoeObject::MoeObject(MoeObject* parent) : QObject(parent)
{
    moveToThread(MoeEngine::threadEngine().data());
    instances.localData().insert(ptr(), MoeObjectPointer(this));
    connect(engine(), SIGNAL(destroyed()), this, SLOT(deleteLater()));
}

MoeEngine* MoeObject::engine() const
{
    return qobject_cast<MoeEngine*>(thread());
}

QString MoeObject::toString() const {
    QString toString = metaObject()->className();
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
    toString += ')';

    return toString;
}

void MoeObject::animate(QString key, QScriptValue to, QScriptValue callback, qreal modifier) {
    bool addToList = false;
    AnimationStatePointer state = animations.value(key);
    if(!state.data()) {
        int propIndex = metaObject()->indexOfProperty(key.toLocal8Bit().data());
        if(!propIndex) {
            engine()->scriptEngine()->currentContext()->throwError("Unknown Property");
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

        default:
        {
            engine()->scriptEngine()->currentContext()->throwError("Unknown Type, Cannot Animate");
            animations.remove(key);
            return;
        }
    }

    state->callback = callback;
    state->modifier = modifier;
    if(addToList) {
        if(animations.isEmpty())
            connect(engine(), SIGNAL(tick()), this, SLOT(animateTick()));

        animations.insert(key, state);
    }
}

enum SmoothResultFlag {
    NoChange = 0x0,
    ValueChanged = 0x1,
    ValueFarFromTarget = 0x2
};

Q_DECLARE_FLAGS(SoothResultFlags, SmoothResultFlag)

#define ANIMATE_START_GROUP(T) T val = metaProp.read(this).value<T>(); \
T target = state->target.value<T>();

#define ANIMATE_PROPERTY_REAL(READ, WRITE) qreal READ = val.READ(); \
    result |= smoothReal(READ, target.READ(), state->modifier); \
    if(result.testFlag(ValueChanged)) \
        val.WRITE(READ); \

#define ANIMATE_PROPERTY_INT(READ, WRITE) int READ = val.READ(); \
    result |= smoothInt(READ, target.READ(), state->modifier); \
    if(result.testFlag(ValueChanged)) \
        val.WRITE(READ); \


#define ANIMATE_END_GROUP(T) if(result.testFlag(ValueChanged)) \
    metaProp.write(this, val);

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
        value += (int)qCeil((qreal)(to - value)/modifier);
        if(value >= to) {
            value = to;
            return ValueChanged;
        }
    } else if(value > to) {
        value -= (int)qCeil((qreal)(value - to)/modifier);
        if(value <= to) {
            value = to;
            return ValueChanged;
        }
    } else
        return NoChange;

    return SoothResultFlags(ValueChanged | ValueFarFromTarget);
}

void MoeObject::animateTick() {
    QList<AnimationStatePointer> _animations = animations.values();
    animations.clear();

    foreach(AnimationStatePointer state, _animations) {
        QMetaProperty metaProp = state->metaProp;
        SoothResultFlags result = NoChange;
        switch(metaProp.type()) {
            case QVariant::Color:
            {
                ANIMATE_START_GROUP(QColor)
                ANIMATE_PROPERTY_INT(red, setRed)
                ANIMATE_PROPERTY_INT(green, setGreen)
                ANIMATE_PROPERTY_INT(blue, setBlue)
                ANIMATE_PROPERTY_INT(alpha, setAlpha)
                ANIMATE_END_GROUP(QColor)

                break;
            }

            case QVariant::Double:
            {
                qreal val = state->metaProp.read(this).toReal();
                qreal target = state->target.toReal();
                result = smoothReal(val, target, state->modifier);
                if(result.testFlag(ValueChanged))
                    metaProp.write(this, val);
                break;
            }

            default:
                break;
        }

        if(result.testFlag(ValueFarFromTarget))
            animations.insert(state->metaProp.name(), state);
        else if(state->callback.isFunction())
            state->callback.call();
    }

    if(animations.isEmpty())
        disconnect(engine(), SIGNAL(tick()), this, SLOT(animateTick()));
}

void MoeObject::killAnimation(QString key) {
    animations.take(key);
    if(animations.isEmpty())
        disconnect(engine(), SIGNAL(tick()), this, SLOT(animateTick()));
}
