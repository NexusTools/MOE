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

    Q_INVOKABLE inline bool hasDefaultValue() const{return contains("");}
    Q_INVOKABLE inline QVariant defaultValue(QVariant def =QVariant()) const{return value("", def);}
    Q_INVOKABLE inline QVariant value(QString key, QVariant def =QVariant()) const{return _args.value(key).isNull() ? def : _args.value(key);}
    Q_INVOKABLE inline bool contains(QString key) const{return _args.contains(key);}
    Q_INVOKABLE inline void insert(QString key, QVariant val) {_args.insert(key, val);}

    inline QVariantMap toMap() const{return _args;}
    Q_INVOKABLE QStringList keys() const;

private:
    QVariantMap _args;
    QString _error;
};

#endif // QARGUMENTPARSER_H
