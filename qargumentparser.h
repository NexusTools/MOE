#ifndef QARGUMENTPARSER_H
#define QARGUMENTPARSER_H

#include <QVariantMap>
#include <QStringList>

class QArgumentParser : public QObject
{
    Q_OBJECT
public:
    inline QArgumentParser() {}
    QArgumentParser(int argc, char *argv[]);
    QArgumentParser(char *args[]);
    QArgumentParser(QVariantMap args);
    QArgumentParser(QList<QVariant> args);
    QArgumentParser(QList<QByteArray> args);
    inline QArgumentParser(QStringList args) {parse(args);}

    Q_INVOKABLE void parse(QStringList args);
    Q_INVOKABLE inline QString lastError() const{return _error;}

    Q_INVOKABLE inline QVariant defaultValue(QString def =QString()) const{return value("", def);}
    Q_INVOKABLE inline QVariant value(QString key, QString def =QString()) const{return _args.value(key, def);}
    Q_INVOKABLE inline bool contains(QString key) const{return _args.contains(key);}
    Q_INVOKABLE inline void insert(QString key, QVariant val) {_args.insert(key, val);}

    Q_INVOKABLE QStringList keys() const;

    inline QVariantMap toMap() const{return _args;}

private:
    QVariantMap _args;
    QString _error;
};

#endif // QARGUMENTPARSER_H
