#ifndef MOEENGINEVIEW_H
#define MOEENGINEVIEW_H

#include <QWidget>
#include <QPointer>

#include "moeengine.h"
#include "moegraphicssurface.h"

class MoeEngineView : public QWidget
{
    Q_OBJECT
public:
    explicit MoeEngineView(QWidget *parent = 0);

    void inject(QString key, QObject* obj);

public slots:
    void start();
    void quit();

protected slots:
    void renderInstructions();

private:
    QPointer<MoeEngine> engine;
    QPointer<MoeGraphicsSurface> surface;
};

#endif // MOEENGINEVIEW_H
