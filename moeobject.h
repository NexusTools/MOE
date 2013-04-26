#ifndef MOEOBJECT_H
#define MOEOBJECT_H

#include <QObject>
#include <QScriptable>
#include <QThreadStorage>

class MoeEngine;

class MoeObject : public QObject
{
    Q_OBJECT
public:
    MoeObject();
    virtual MoeEngine* engine() const;

private:
};

#endif // MOEOBJECT_H
