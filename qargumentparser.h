#ifndef QARGUMENTPARSER_H
#define QARGUMENTPARSER_H

#include <QVariant>
#include <QStringList>

class QArgumentParser : public QObject
{
Q_OBJECT
public:
    inline QArgumentParser() {}
    QArgumentParser(int argc, char *argv[]);
    QArgumentParser(char *args[]);
    QArgumentParser(QHash<QString, QVariant> args);
    QArgumentParser(QList<QVariant> args);
    QArgumentParser(QList<QByteArray> args);
    inline QArgumentParser(QStringList args) {parse(args);}

    Q_INVOKABLE bool parse(QStringList args);
    Q_INVOKABLE inline QString lastError() const{return _error;}

    Q_INVOKABLE inline QVariant defaultValue() const{return value("");}
    Q_INVOKABLE inline QHash<QString, QVariant> getArguments() const{return _args;}
    Q_INVOKABLE inline QVariant value(QString key) const{return _args.value(key);}
    Q_INVOKABLE inline bool contains(QString key) const{return _args.contains(key);}
    Q_INVOKABLE inline void insert(QString key, QVariant val) {_args.insert(key, val);}

private:
    QHash<QString, QVariant> _args;
    QString _error;
};

#endif // QARGUMENTPARSER_H
