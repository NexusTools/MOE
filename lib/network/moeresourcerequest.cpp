#include "moeresourcerequest.h"
#include "core/moeengine.h"

#include <QRegularExpression>
#include <QDomDocument>


QThreadStorage<QHashString> MoeResourceRequest::boundPaths;

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
                newData = QVariantList() << currentData << newData;

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
    if(!receivedConnection)
        return;

    QMetaMethod receivedXmlSignal = QMetaMethod::fromSignal(&MoeResourceRequest::receivedXML);
    QMetaMethod receivedJsonSignal = QMetaMethod::fromSignal(&MoeResourceRequest::receivedJSON);
    QMetaMethod receivedStringSignal = QMetaMethod::fromSignal(&MoeResourceRequest::receivedString);
    QMetaMethod receivedChildListSignal = QMetaMethod::fromSignal(&MoeResourceRequest::receivedChildList);

    Q_ASSERT(receivedXmlSignal.isValid());
    Q_ASSERT(receivedJsonSignal.isValid());
    Q_ASSERT(receivedStringSignal.isValid());
    Q_ASSERT(receivedChildListSignal.isValid());

    qDebug() << this << "processing received data" << _url;

    emit receivedData(dat);
    if(isSignalConnected(receivedJsonSignal)) {
        engine()->scriptEngine()->pushContext();
        QScriptValue value = engine()->scriptEngine()->evaluate(QString("(%1)").arg(QString(dat)), "evalJSON");
        engine()->scriptEngine()->popContext();
        emit receivedJSON(value);
    }
    bool childListSignalConnected = isSignalConnected(receivedChildListSignal);
    bool stringSignalConnected = isSignalConnected(receivedStringSignal);
    if(childListSignalConnected || stringSignalConnected) {
        QString stringData(QString::fromUtf8(dat));

        if(stringSignalConnected)
            emit receivedString(stringData);

        if(childListSignalConnected) {
            QStringList children;
            QRegularExpression regExp("<[^>]*((src|href|source|file)=(?<src>\"[^\"]+|'[^']+|[^\\s\"'>]+))[^>]+", QRegularExpression::CaseInsensitiveOption);
            QRegularExpressionMatchIterator i = regExp.globalMatch(dat);

            while(i.hasNext()) {
                QRegularExpressionMatch match = i.next();
                QString fileMatch = match.captured("src");
                if(fileMatch.startsWith('"') || fileMatch.startsWith('\''))
                    fileMatch = fileMatch.mid(1);
                QString base = _realBase;
                if(!base.endsWith('/'))
                    base += '/';
                QString resolved(MoeUrl::locate(fileMatch, base).toString());
                qDebug() << fileMatch << resolved << base;
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

    if(isSignalConnected(receivedXmlSignal)) {
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
