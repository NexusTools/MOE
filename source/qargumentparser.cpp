#include "qargumentparser.h"

#include <QRegExp>


QArgumentParser::QArgumentParser(char *args[])
{
    QStringList _args;
    int i = 1;
    forever
    {
        if(!args[i])
            break;
        i++;
        _args.append(args[i]);
    }

    parse(_args);
}

QArgumentParser::QArgumentParser(int argc, char *argv[])
{
    QStringList _args;
    for(int i=1; i<argc; i++)
    {
        if(!argv[i])
            continue;

        _args.append(argv[i]);
    }

    parse(_args);
}

QArgumentParser::QArgumentParser(QVariantMap args)
{
    QMapIterator<QString, QVariant> iterator(args);
    while(iterator.hasNext())
    {
        iterator.next();
        _args.insert(iterator.key(), iterator.value());
    }
}

QArgumentParser::QArgumentParser(QList<QVariant> args)
{
    QStringList _args;
    foreach(QVariant var, args)
    {
        _args.append(var.toString());
    }

    parse(_args);
}

QArgumentParser::QArgumentParser(QList<QByteArray> args)
{
    QStringList _args;
    foreach(QByteArray data, args)
    {
        _args.append(QString(data));
    }

    parse(_args);
}

inline QVariant listToValue(QStringList val)
{
    const QRegExp floating("^\\-?\\d+\\.\\d+$");
    const QRegExp numeric("^\\-?\\d+$");

    switch(val.size())
    {
        case 0:
        return QVariant();

        case 1:
        {
            QString _val = val.first();
            if(floating.exactMatch(_val))
                return _val.toDouble();
            else if(numeric.exactMatch(_val))
                return _val.toLongLong();
            else
                return _val;
            break;
        }
    }

    return val;
}

QStringList QArgumentParser::keys() const
{
    QStringList keys;
    foreach(QString key, _args.keys())
        keys.append(key);
    return keys;
}

void QArgumentParser::parse(QStringList args)
{
    QString key;
    QStringList val;
    const QRegExp keyValuePair("^(\\-|\\-\\-|/)['\"]?([^=]+)['\"]?(=['\"]?(.+)['\"]?)?$");

    foreach(QString arg, args)
    {
        if(keyValuePair.exactMatch(arg))
        {

            if(!key.isEmpty() || !val.isEmpty())
            {
                insert(key, listToValue(val));
                key.clear();
                val.clear();
            }

            if(keyValuePair.cap(4).isEmpty())
                key = keyValuePair.cap(2).toLower();
            else
                insert(keyValuePair.cap(2).toLower(), listToValue(keyValuePair.cap(4).split(',')));

        } else
            val.append(arg);
    }

    if(!key.isEmpty() || !val.isEmpty())
        insert(key, listToValue(val));
}
