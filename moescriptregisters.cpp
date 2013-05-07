#include "moescriptregisters.h"
#include "renderrecorder.h"
#include "moegraphicscontainer.h"

#include <QScriptValueIterator>
#include <QRadialGradient>
#include <QLinearGradient>
#include <QScriptEngine>
#include <QCursor>
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

QScriptValue qsizeToScriptValue(QScriptEngine *engine, QSize const &in)
{
    QScriptValue rect = engine->newObject();
    rect.setProperty("width", in.width());
    rect.setProperty("height", in.height());
    return rect;
}

void qsizeFromScriptValue(const QScriptValue &object, QSize &out)
{
    out.setWidth(object.property("width").toInt32());
    out.setHeight(object.property("height").toInt32());
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

QScriptValue qpointToScriptValue(QScriptEngine *engine, QPoint const &in)
{
    QScriptValue rect = engine->newObject();
    rect.setProperty("x", in.x());
    rect.setProperty("y", in.y());
    return rect;
}

void qpointFromScriptValue(const QScriptValue &object, QPoint &out)
{
    out.setX(object.property("x").toInt32());
    out.setY(object.property("y").toInt32());
}

QScriptValue qcolorToScriptValue(QScriptEngine *engine, QColor const &in)
{
    QScriptValue rgb = engine->newObject();
    rgb.setProperty("red", in.red());
    rgb.setProperty("green", in.green());
    rgb.setProperty("blue", in.blue());
    rgb.setProperty("alpha", in.alpha());
    return rgb;
}

void qcolorFromScriptValue(const QScriptValue &object, QColor &out)
{
    if(object.isString()) {
        QString string = object.toString();

        static QRegExp hex("^#([\\dabcdef]{3,6})$", Qt::CaseInsensitive, QRegExp::RegExp2);
        if(hex.exactMatch(string)) {
            string = hex.cap(1);
            if(string.length() < 6)
                string = QString("%1%1%2%2%3%3").arg(string[0]).arg(string[1]).arg(string[2]);
            out = QColor::fromRgb((QRgb)string.toInt(0, 16));
            return;
        }

        static QRegExp cssRgb("rgba?\\(\\s*(\\d)+\\s*,\\s*(\\d)+\\s*,\\s*(\\d+)\\s*(,\\s*(\\d+(\\.\\d+)?))?\\)", Qt::CaseInsensitive, QRegExp::RegExp2);
        if(cssRgb.exactMatch(string)) {
            bool alphaOkay;
            out = QColor(cssRgb.cap(1).toInt(), cssRgb.cap(2).toInt(), cssRgb.cap(3).toInt(), (int)(cssRgb.cap(5).toFloat(&alphaOkay) * 255));
            if(!alphaOkay)
                out = QColor(cssRgb.cap(1).toInt(), cssRgb.cap(2).toInt(), cssRgb.cap(3).toInt());

            return;
        }

        static QHash<QString, QColor> namedColors;
        if(namedColors.isEmpty()) {
            QFile reader(":/data/rgb.txt");
            if(reader.open(QFile::ReadOnly)) {
                QRegExp colorLine("(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\w.+\\w)", Qt::CaseInsensitive, QRegExp::RegExp2);
                while(!reader.atEnd()) {
                    QString line = reader.readLine().trimmed();
                    if(colorLine.exactMatch(line))
                        namedColors.insert(colorLine.cap(4).toLower(), QColor(colorLine.cap(1).toInt(), colorLine.cap(2).toInt(), colorLine.cap(3).toInt()));
                }
            } else
                qWarning() << "Failed to open rgb.txt";
        }
        string = string.toLower().trimmed();
        if(namedColors.contains(string)) {
            out = namedColors.value(string);
            return;
        }
    } else if(object.isObject()) {
        if(object.property("alpha").isValid())
            out = QColor(object.property("red").toInt32(), object.property("green").toInt32(), object.property("blue").toInt32(), object.property("alpha").toInt32());
        else
            out = QColor(object.property("red").toInt32(), object.property("green").toInt32(), object.property("blue").toInt32());
        return;
    } else if(object.isArray()) {
        int len = object.property("length").toInt32();
        if(len == 4)
            out = QColor(object.property(0).toInt32(), object.property(1).toInt32(), object.property(2).toInt32(), object.property(3).toInt32());
        else if(len == 3)
            out = QColor(object.property(0).toInt32(), object.property(1).toInt32(), object.property(2).toInt32(), 255);
        return;
    } else if(object.isNumber() || object.isNumber()) {
        out = QColor::fromRgb((QRgb)object.toInt32());
        return;
    }

    object.engine()->currentContext()->throwError(QString("RGB types expect a valid css color, or numeric color code."));
}

QScriptValue qbrushToScriptValue(QScriptEngine *engine, QBrush const &in)
{
    QScriptValue rect = engine->newObject();
    rect.setProperty("red", in.color().red());
    rect.setProperty("green", in.color().green());
    rect.setProperty("blue", in.color().blue());
    rect.setProperty("alpha", in.color().alpha());
    return rect;
}

inline void __initGradientBrush__(QGradient* gradient, QGradientStops& stops) {
    gradient->setStops(stops);
    gradient->setSpread(QGradient::PadSpread);
    gradient->setCoordinateMode(QGradient::ObjectBoundingMode);
}

void qbrushFromScriptValue(const QScriptValue &object, QBrush &out)
{
    if(object.isObject()) {
        QScriptValue type = object.property("type");
        QScriptValue stops = object.property("stops");
        if(stops.isArray() && type.isString()) {
            QGradientStops qstops;
            QScriptValueIterator stop(stops);
            while(stop.hasNext()) {
                stop.next();
                if(stop.flags().testFlag(QScriptValue::SkipInEnumeration))
                    continue;
                if(stop.value().isArray()) {
                    qreal pos = stop.value().property(0).toNumber();
                    QColor col = object.engine()->fromScriptValue<QColor>(stop.value().property(1));
                    qstops << QGradientStop(pos, col);
                } else if(stop.value().isObject()) {
                    qreal pos = stop.value().property("pos").toNumber();
                    QColor col = object.engine()->fromScriptValue<QColor>(stop.value().property("color"));
                    qstops << QGradientStop(pos, col);
                }
            }

            QString typeString = type.toString();
            if(typeString == "linear") {
                QLinearGradient gradient(object.engine()->fromScriptValue<QPoint>(object.property("start")),
                                object.engine()->fromScriptValue<QPoint>(object.property("stop")));
                __initGradientBrush__(&gradient, qstops);
                out = QBrush(gradient);
            } else if(typeString == "radial") {
                QRadialGradient gradient;
                __initGradientBrush__(&gradient, qstops);
                out = QBrush(gradient);
            } else {
                object.engine()->currentContext()->throwError(QString("%1 is not a valid radial type.").arg(typeString));
                return;
            }
            return;
        }
    }

    QColor color;
    qcolorFromScriptValue(object, color);
    out = QBrush(color);
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

QScriptValue qcursorToScriptValue(QScriptEngine *engine, QCursor const &in)
{
    switch(in.shape()) {
    case Qt::BlankCursor:
        return "none";

    case Qt::ArrowCursor:
        return "default";

    case Qt::WaitCursor:
        return "wait";

    case Qt::BusyCursor:
        return "progress";

    case Qt::PointingHandCursor:
        return "pointer";

    default:
        return engine->nullValue();
    }
}

void qcursorFromScriptValue(const QScriptValue &object, QCursor &out)
{
    if(object.isString()) {
        QString cur = object.toString();
        if(cur == "none") {
            out.setShape(Qt::BlankCursor);
            return;
        } else if(cur == "default") {
            out.setShape(Qt::BlankCursor);
            return;
        } else if(cur == "wait") {
            out.setShape(Qt::WaitCursor);
            return;
        } else if(cur == "progress") {
            out.setShape(Qt::BusyCursor);
            return;
        } else if(cur == "pointer") {
            out.setShape(Qt::PointingHandCursor);
            return;
        }
    } else if(object.isNumber()) {
        out.setShape(((Qt::CursorShape)object.toInt32()));
        return;
    }

    object.engine()->currentContext()->throwError(QString("Cursor types expect a valid css string."));
}

void __moe_registerScriptConverters(QScriptEngine* eng) {
    qRegisterMetaType<QRgb>("QRgb");

    qScriptRegisterMetaType<RenderRecorder*>(eng, renderRecorderToScriptValue, renderRecorderFromScriptValue);
    qScriptRegisterMetaType<MoeGraphicsContainer*>(eng, graphicsContainerToScriptValue, graphicsContainerFromScriptValue);

    qScriptRegisterMetaType<QCursor>(eng, qcursorToScriptValue, qcursorFromScriptValue);

    qScriptRegisterMetaType<QPointF>(eng, qpointfToScriptValue, qpointfFromScriptValue);
    qScriptRegisterMetaType<QPoint>(eng, qpointToScriptValue, qpointFromScriptValue);

    qScriptRegisterMetaType<QSizeF>(eng, qsizefToScriptValue, qsizefFromScriptValue);
    qScriptRegisterMetaType<QSize>(eng, qsizeToScriptValue, qsizeFromScriptValue);

    qScriptRegisterMetaType<QRectF>(eng, qrectfToScriptValue, qrectfFromScriptValue);
    qScriptRegisterMetaType<QRect>(eng, qrectToScriptValue, qrectFromScriptValue);

    qScriptRegisterMetaType<QBrush>(eng, qbrushToScriptValue, qbrushFromScriptValue);
    qScriptRegisterMetaType<QColor>(eng, qcolorToScriptValue, qcolorFromScriptValue);
    qScriptRegisterMetaType<QFont>(eng, qfontToScriptValue, qfontFromScriptValue);
}
