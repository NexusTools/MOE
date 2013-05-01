#include "moescriptregisters.h"


#include "renderrecorder.h"
#include "moegraphicscontainer.h"
#include <QScriptEngine>
#include <QRegExp>
#include <QColor>
#include <QDebug>
#include <QFile>

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



QScriptValue qrectToScriptValue(QScriptEngine *engine, QRect const &in)
{
    QScriptValue rect = engine->newObject();
    rect.setProperty("y", in.top());
    rect.setProperty("x", in.left());
    rect.setProperty("width", in.width());
    rect.setProperty("height", in.height());
    return rect;
}

void qrectFromScriptValue(const QScriptValue &object, QRect &out)
{
    out.setTop(object.property("y").toInteger());
    out.setLeft(object.property("x").toInteger());
    out.setWidth(object.property("width").toInteger());
    out.setHeight(object.property("height").toInteger());
}

QScriptValue qrectfToScriptValue(QScriptEngine *engine, QRectF const &in)
{
    QScriptValue rect = engine->newObject();
    rect.setProperty("y", in.top());
    rect.setProperty("x", in.left());
    rect.setProperty("width", in.width());
    rect.setProperty("height", in.height());
    return rect;
}

void qrectfFromScriptValue(const QScriptValue &object, QRectF &out)
{
    out.setTop(object.property("y").toNumber());
    out.setLeft(object.property("x").toNumber());
    out.setWidth(object.property("width").toNumber());
    out.setHeight(object.property("height").toNumber());
}

QScriptValue qsizefToScriptValue(QScriptEngine *engine, QSizeF const &in)
{
    QScriptValue rect = engine->newObject();
    rect.setProperty("width", in.width());
    rect.setProperty("height", in.height());
    return rect;
}

void qsizefFromScriptValue(const QScriptValue &object, QSizeF &out)
{
    out.setWidth(object.property("width").toNumber());
    out.setHeight(object.property("height").toNumber());
}

QScriptValue qpointfToScriptValue(QScriptEngine *engine, QPointF const &in)
{
    QScriptValue rect = engine->newObject();
    rect.setProperty("x", in.x());
    rect.setProperty("y", in.y());
    return rect;
}

void qpointfFromScriptValue(const QScriptValue &object, QPointF &out)
{
    out.setX(object.property("x").toNumber());
    out.setY(object.property("y").toNumber());
}

QScriptValue qrgbToScriptValue(QScriptEngine *engine, QRgb const &in)
{
    QScriptValue rgb = engine->newObject();
    rgb.setProperty("red", qRed(in));
    rgb.setProperty("green", qGreen(in));
    rgb.setProperty("blue", qBlue(in));
    if(qAlpha(in) < 255)
        rgb.setProperty("alpha", qAlpha(in));
    return rgb;
}

void qrgbFromScriptValue(const QScriptValue &object, QRgb &out)
{
    out = Qt::black;
    if(object.isString()) {
        QString string = object.toString();

        static QRegExp hex("^#([\\dabcdef]{3,6})$", Qt::CaseInsensitive, QRegExp::RegExp2);
        if(hex.exactMatch(string)) {
            string = hex.cap(1);
            if(string.length() < 6)
                string = QString("%1%1%2%2%3%3").arg(string[0]).arg(string[1]).arg(string[2]);
            out = (QRgb)string.toInt(0, 16);
            return;
        }

        static QRegExp cssRgb("rgba?\\(\\s*(\\d)+\\s*,\\s*(\\d)+\\s*,\\s*(\\d+)\\s*(,\\s*(\\d+(\\.\\d+)?))?\\)", Qt::CaseInsensitive, QRegExp::RegExp2);
        if(cssRgb.exactMatch(string)) {
            bool alphaOkay;
            out = qRgba(cssRgb.cap(1).toInt(), cssRgb.cap(2).toInt(), cssRgb.cap(3).toInt(), (int)(cssRgb.cap(5).toFloat(&alphaOkay) * 255));
            if(!alphaOkay)
                out = qRgb(cssRgb.cap(1).toInt(), cssRgb.cap(2).toInt(), cssRgb.cap(3).toInt());

            return;
        }

        static QHash<QString, QRgb> namedColors;
        if(namedColors.isEmpty()) {
            qDebug() << "Loading Name List...";
            QFile reader(":/data/rgb.txt");
            if(reader.open(QFile::ReadOnly)) {
                QRegExp colorLine("(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\w.+\\w)", Qt::CaseInsensitive, QRegExp::RegExp2);
                while(!reader.atEnd()) {
                    QString line = reader.readLine().trimmed();
                    if(colorLine.exactMatch(line))
                        namedColors.insert(colorLine.cap(4).toLower(), qRgb(colorLine.cap(1).toInt(), colorLine.cap(2).toInt(), colorLine.cap(3).toInt()));
                }
            } else
                qWarning() << "Failed to open rgb.txt";
            qDebug() << namedColors.size() << "Named Colors Loaded";
        }
        string = string.toLower().trimmed();
        if(namedColors.contains(string)) {
            out = namedColors.value(string);
            return;
        }
    } else if(object.isObject()) {
        if(object.property("alpha").isValid())
            out = qRgba(object.property("red").toInt32(), object.property("green").toInt32(), object.property("blue").toInt32(), object.property("alpha").toInt32());
        else
            out = qRgb(object.property("red").toInt32(), object.property("green").toInt32(), object.property("blue").toInt32());
        return;
    } else if(object.isArray()) {
        int len = object.property("length").toInt32();
        if(len == 4)
            out = qRgba(object.property(0).toInt32(), object.property(1).toInt32(), object.property(2).toInt32(), object.property(3).toInt32());
        else if(len == 3)
            out = qRgba(object.property(0).toInt32(), object.property(1).toInt32(), object.property(2).toInt32(), 255);
        return;
    } else if(object.isNumber() || object.isNumber()) {
        out = (QRgb)object.toInt32();
        return;
    }

    object.engine()->currentContext()->throwError(QString("RGB types expect a valid css color, or numeric color code."));
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

void __moe_registerScriptConverters(QScriptEngine* eng) {
    qRegisterMetaType<QRgb>("QRgb");

    qScriptRegisterMetaType<RenderRecorder*>(eng, renderRecorderToScriptValue, renderRecorderFromScriptValue);
    qScriptRegisterMetaType<MoeGraphicsContainer*>(eng, graphicsContainerToScriptValue, graphicsContainerFromScriptValue);

    qScriptRegisterMetaType<QPointF>(eng, qpointfToScriptValue, qpointfFromScriptValue);
    qScriptRegisterMetaType<QSizeF>(eng, qsizefToScriptValue, qsizefFromScriptValue);
    qScriptRegisterMetaType<QRectF>(eng, qrectfToScriptValue, qrectfFromScriptValue);
    qScriptRegisterMetaType<QFont>(eng, qfontToScriptValue, qfontFromScriptValue);
    qScriptRegisterMetaType<QRect>(eng, qrectToScriptValue, qrectFromScriptValue);
    qScriptRegisterMetaType<QRgb>(eng, qrgbToScriptValue, qrgbFromScriptValue);
}