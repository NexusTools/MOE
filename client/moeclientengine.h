#ifndef MOECLIENTENGINE_H
#define MOECLIENTENGINE_H

#include <core/moeengine.h>

class MoeClientEngine : public MoeEngine
{
    Q_OBJECT
public:
    explicit inline MoeClientEngine() {}

protected:
    virtual void initializeScriptEngine(QScriptEngine* eng);
    virtual void initializeContentEnvironment(QScriptEngine* eng, QScriptValue globalObject);
    
};

#endif // MOECLIENTENGINE_H
