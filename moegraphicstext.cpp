#include "moegraphicstext.h"

#include <QFontMetrics>

void MoeGraphicsText::sizeToText() {
    QFontMetrics fontMetrics(_font);
    setSize(fontMetrics.size(0, _text) + QSize(_margin*2, _margin*2));
}

void MoeGraphicsText::paintImpl(RenderRecorder* p, QRect) {
    p->setFont(_font);
    p->drawText(QRect(QPoint(_margin, _margin), localSize()-QSize(_margin*2, _margin*2)), _text);
}
