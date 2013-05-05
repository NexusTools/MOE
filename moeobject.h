#ifndef MOEOBJECT_H
#define MOEOBJECT_H

#include <QDebug>
#include <QObject>
#include <QPointer>
#include <QScriptable>
#include <QThreadStorage>

class MoeEngine;
class MoeObject;

typedef quintptr MoeObjectPtr;
typedef QPointer<MoeObject> MoeObjectPointer;
typedef QMap<MoeObjectPtr, MoeObjectPointer> MoeObjectPtrMap;

class MoeObject : public QObject, public QScriptable
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit MoeObject(MoeObject* parent =NULL);
    inline virtual ~MoeObject() {instances.localData().remove(ptr());}

    virtual MoeEngine* engine() const;

    inline MoeObjectPtr ptr() const{return (MoeObjectPtr)this;}
    inline MoeObject* instanceForPtr(MoeObjectPtr) const{
        return instances.localData().value(ptr()).data();
    }
    inline int countInstances() {
        return instances.localData().count();
    }

    Q_INVOKABLE QString toString() const;

private:
    static QThreadStorage<MoeObjectPtrMap> instances;
};

inline QDebug operator <<(QDebug debug, const MoeObject & moe) {
    return (debug << moe.toString().toLocal8Bit().data());
}

#endif // MOEOBJECT_H
