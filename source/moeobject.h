#ifndef MOEOBJECT_H
#define MOEOBJECT_H

#include <QMap>
#include <QHash>
#include <QObject>
#include <QVariant>
#include <QPointer>
#include <QScriptable>
#include <QScriptValue>
#include <QThreadStorage>

class MoeEngine;
class MoeObject;

struct AnimationState;
typedef QSharedPointer<AnimationState> AnimationStatePointer;

typedef quintptr MoeObjectPtr;
typedef QPointer<MoeObject> MoeObjectPointer;
typedef QMap<MoeObjectPtr, MoeObjectPointer> MoeObjectPtrMap;

class MoeObject : public QObject, public QScriptable
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit MoeObject(MoeObject* parent =NULL);
    virtual ~MoeObject() {instances.localData().remove(ptr());}

    virtual MoeEngine* engine() const;

    inline MoeObjectPtr ptr() const{return (MoeObjectPtr)this;}
    template<typename T> static inline T* instance(MoeObjectPtr ptr) {
        return qobject_cast<T*>(instances.localData().value(ptr).data());
    }
    inline int countInstances() {
        return instances.localData().count();
    }

    Q_INVOKABLE inline void destroy() {deleteLater();}
    Q_INVOKABLE QString toString() const;

public slots:
    void animate(QString key, QScriptValue to, QScriptValue callback = QScriptValue(), qreal modifier=4);
    void killAnimation(QString key);

protected slots:
    void animateTick();

private:
    static QThreadStorage<MoeObjectPtrMap> instances;
    QHash<QString, AnimationStatePointer> animations;
};

#endif // MOEOBJECT_H
