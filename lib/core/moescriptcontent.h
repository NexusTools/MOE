#ifndef MOESCRIPTCONTENT_H
#define MOESCRIPTCONTENT_H

#include "moecontentplugin.h"

class MoeScriptContent : public MoeContentPlugin
{
    Q_OBJECT
public:
    inline explicit MoeScriptContent(QString content, MoeEngine* eng) : MoeContentPlugin(eng) {_path=content;}

    virtual QString name() const;
    virtual QString path() const;

private:
    QString _path;
};

#endif // MOESCRIPTCONTENT_H
