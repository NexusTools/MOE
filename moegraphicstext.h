#ifndef MOEGRAPHICSTEXT_H
#define MOEGRAPHICSTEXT_H

#include "moegraphicsobject.h"
#include "renderrecorder.h"

class MoeGraphicsText : public MoeGraphicsObject
{
    Q_OBJECT
    Q_PROPERTY(QFont font READ font WRITE setFont)
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(int margin READ margin WRITE setMargin)
public:
    Q_INVOKABLE MoeGraphicsText(QString text =QString(), QFont fon =QFont("Arial", 12), MoeGraphicsContainer* parent =0) {
        _foreground = qRgb(0, 0, 0);
        setContainer(parent);
        _margin = 0;
        _text = text;
        _font = fon;
        sizeToText();
    }

    inline QString text() const{return _text;}
    inline qreal margin() const{return _margin;}
    inline QFont font() const{return _font;}

    Q_INVOKABLE inline void setMargin(int m){_margin=m;sizeToText();}
    Q_INVOKABLE inline void setText(QString txt){_text=txt;sizeToText();}
    Q_INVOKABLE inline void setFont(QFont fon){_font=fon;sizeToText();}
    Q_INVOKABLE void sizeToText();

    void paintImpl(RenderRecorder *, QRect);

private:
    QFont _font;
    QString _text;
    int _margin;
};

#endif // MOEGRAPHICSTEXT_H
