#ifndef MOEENGINESYSTEMHOOK_H
#define MOEENGINESYSTEMHOOK_H

#include <QObject>

class MoeEngine;

class MoeEngineSystemHook : public QObject
{
    Q_OBJECT
public:
    static void setup(MoeEngine*, char** sysName =0);

protected slots:
    void updateSystemAppName(QString name);

protected:
    explicit MoeEngineSystemHook(MoeEngine* engine, char** sysName =0);
    
private:
    char** sysName;
    MoeEngine *engine;
};

#endif // MOEENGINESYSTEMHOOK_H
