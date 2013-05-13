#include "moeresourcerequest.h"

#include <QDomDocument>

inline void merge(QVariantMap& map, QDomNode& node) {
    QDomNodeList childs = node.childNodes();
    if(!childs.isEmpty()) {
        for(int i=0; i<childs.length(); i++) {
            QDomNode child = childs.at(i);
            if(child.isComment())
                continue;
            QVariant newData;
            QString name = child.nodeName();
            if(child.hasChildNodes()) {
                QVariantMap childMap;
                merge(childMap, child);
                newData = childMap;
            } else
                newData = child.nodeValue();

            QVariant currentData = map.value(name);
            if(currentData.type() == QVariant::List)
                newData = currentData.toList() << newData;
            else if(!currentData.isNull())
                newData = QVariantList() << currentData;
            map.insert(name, newData);
        }
    }
    if(node.hasAttributes()) {
        QVariantMap attr;
        QDomNamedNodeMap atr = node.attributes();
        for(int i=0; i<atr.length(); i++)
            attr.insert(atr.item(i).nodeName(), atr.item(i).nodeValue());
        map.insert("#attr", attr);
    }
}

void MoeResourceRequest::completeCallback(QByteArray dat){
    static QMetaMethod jsonSignal = metaObject()->method(metaObject()->indexOfSignal("receivedJSON(QScriptValue)"));
    static QMetaMethod stringSignal = metaObject()->method(metaObject()->indexOfSignal("receivedString(QString)"));
    static QMetaMethod xmlSignal = metaObject()->method(metaObject()->indexOfSignal("receivedXML(QVariantMap)"));
    static QMetaMethod childListSignal = metaObject()->method(metaObject()->indexOfSignal("receivedChildList(QStringList)"));

    emit receivedData(dat);
    if(isSignalConnected(jsonSignal)) {
        engine()->scriptEngine()->pushContext();
        QScriptValue value = engine()->scriptEngine()->evaluate(QString("(%1)").arg(QString(dat)), "evalJSON");
        engine()->scriptEngine()->popContext();
        emit receivedJSON(value);
    }
    if(isSignalConnected(childListSignal) || isSignalConnected(stringSignal)) {
        QString stringData(QString::fromUtf8(dat));

        if(isSignalConnected(stringSignal))
            emit receivedString(stringData);

        if(isSignalConnected(childListSignal)) {
            QStringList children;
            QRegularExpression regExp("<[^>]*((src|href|source|file)=(?<src>\"[^\"]+|'[^']+|[^\\s\"'>]+))[^>]+", QRegularExpression::CaseInsensitiveOption);
            QRegularExpressionMatchIterator i = regExp.globalMatch(dat);
            QString base = _url.toString();
            if(!base.endsWith('/'))
                base += '/';

            while(i.hasNext()) {
                QRegularExpressionMatch match = i.next();
                QString fileMatch = match.captured("src");
                if(fileMatch.startsWith('"') || fileMatch.startsWith('\''))
                    fileMatch = fileMatch.mid(1);
                QString resolved(MoeUrl::locate(fileMatch, base).toString());
                if(resolved.startsWith(base)) {
                    resolved = resolved.mid(base.length());
                    if(resolved.endsWith('/'))
                        resolved = resolved.left(resolved.length()-1);
                    if(resolved.contains('/'))
                        continue; // Isn't direct child...
                    if(resolved.isEmpty())
                        continue; // Same as parent
                    if(children.contains(resolved))
                        continue; // Already found
                    children << resolved;
                }
            }
            emit receivedChildList(children);
        }
    }

    if(isSignalConnected(xmlSignal)) {
        qDebug() << "Emitting XML Data";
        QVariantMap xmlData;

        QString errorMsg;
        int errorLine;
        int errorColumn;

        QDomDocument xmlParser;
        if(xmlParser.setContent(dat, &errorMsg, &errorLine, &errorColumn))
            merge(xmlData, xmlParser);
        else {
            xmlData.insert("errorMessage", errorMsg);
            xmlData.insert("errorLine", errorLine);
            xmlData.insert("errorColumn", errorColumn);
        }
        emit receivedXML(xmlData);
    }

    disconnectAll();
}
