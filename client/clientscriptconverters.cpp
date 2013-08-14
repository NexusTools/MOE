#include "gui/renderrecorder.h"
#include "gui/moegraphicscontainer.h"

#include <QScriptEngine>

inline QScriptValue renderRecorderToScriptValue(QScriptEngine *engine, RenderRecorder* const &in)
{
    return engine->newQObject(in);
}

inline void renderRecorderFromScriptValue(const QScriptValue &object, RenderRecorder* &out)
{
    out = qobject_cast<RenderRecorder*>(object.toQObject());
}

inline QScriptValue graphicsContainerToScriptValue(QScriptEngine *engine, MoeGraphicsContainer* const &in)
{
    return engine->newQObject(in);
}

inline void graphicsContainerFromScriptValue(const QScriptValue &object, MoeGraphicsContainer* &out)
{
    out = qobject_cast<MoeGraphicsContainer*>(object.toQObject());
}

QScriptValue qfontToScriptValue(QScriptEngine *engine, QFont const &in)
{
    QScriptValue font = engine->newObject();
    font.setProperty("family", in.family());
    font.setProperty("size", in.pointSize());
    return font;
}

void qfontFromScriptValue(const QScriptValue &object, QFont &out)
{
    if(object.isString()) {
        // TODO: Implement CSS Font Parsing
    } else if(object.isObject()) {
        out = QFont(object.property("family").toString(), object.property("size").toInt32());
        return;
    }

    object.engine()->currentContext()->throwError(QString("Font types expect a valid css font string, or font object."));
}

void __moe_client__registerScriptConverters(QScriptEngine* eng) {
    qScriptRegisterMetaType<RenderRecorder*>(eng, renderRecorderToScriptValue, renderRecorderFromScriptValue);
    qScriptRegisterMetaType<MoeGraphicsContainer*>(eng, graphicsContainerToScriptValue, graphicsContainerFromScriptValue);

    qScriptRegisterMetaType<QFont>(eng, qfontToScriptValue, qfontFromScriptValue);
}
