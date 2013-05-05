#ifndef MOEOBJECT_H
#define MOEOBJECT_H

#include <QDebug>
#include <QObject>
#include <QScriptable>

class MoeEngine;

typedef QPointer<QObject> MoeObjectPointer;
typedef QMap<quintptr, MoeObjectPointer> MoeObjectPointerMap;

class MoeObject : public QObject, public QScriptable
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit MoeObject(MoeObject* parent =NULL);
    virtual MoeEngine* engine() const;

    inline quintptr uniqueID() const{return (quintptr)this;}

    Q_INVOKABLE QString toString() const;
};

inline QDebug operator <<(QDebug debug, const MoeObject & moe) {
    return (debug << moe.toString().toLocal8Bit().data());
}

#endif // MOEOBJECT_H
