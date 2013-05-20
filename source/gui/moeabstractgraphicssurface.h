#ifndef MOEABSTRACTGRAPHICSSURFACE_H
#define MOEABSTRACTGRAPHICSSURFACE_H

#include "abstractsurfacebackend.h"
#include "moegraphicscontainer.h"
#include "renderrecorder.h"

#include <QThreadStorage>
#include <QSharedPointer>
#include <QWeakPointer>
#include <QScriptValue>
#include <QPointer>
#include <QTimer>
#include <QUrl>

typedef QPointer<MoeAbstractGraphicsSurface> MoeAbstractGraphicsSurfacePointer;

class MoeAbstractGraphicsSurface : public MoeGraphicsContainer
{
    Q_OBJECT
    Q_PROPERTY(QPoint mousePos READ mousePos)
public:
    enum RenderStateFlag {
        NotReady = 0x0,
        SurfaceDirty = 0x1,
        ViewReady = 0x2
    };
    Q_DECLARE_FLAGS(RenderState, RenderStateFlag)

    void render(RenderRecorder*, QRect);

    Q_INVOKABLE inline bool isVisibleToSurface() {return _connected;}
    Q_INVOKABLE inline bool isSurface() const{return true;}
    Q_INVOKABLE inline MoeAbstractGraphicsSurface* surface() {return this;}

    Q_INVOKABLE virtual inline bool isRemote() const{return false;}

    Q_INVOKABLE bool isKeyPressed(int btn) const{return buttons.contains(btn);}
    inline int mouseButtons() const{return _mouseButtons;}
    inline QPoint mousePos() const{return _mousePos;}

public slots:
    virtual void openUrl(QUrl url) {qWarning() << this << "openUrl method isn't implemented." << url;}

    inline void enableRepaintDebug(bool fullFrame =true, QColor repaintColor =QColor(0, 250, 0, 60)) {
        repaintDebug = fullFrame ? RepaintDebugFrame : RepaintDebugArea;
        this->repaintColor = repaintColor;
    }

    inline void disableRepaintDebug() {
        repaintDebug = RepaintDebugOff;
        repaintRegions.clear();
        repaint();
    }

signals:
    void connected();
    void disconnected();

protected slots:
    inline void setGeometry(QRect rect) {
        MoeGraphicsObject::setGeometry(rect.x(), rect.y(), rect.width(), rect.height());
    }

    inline void prepareNextFrame(){
        //qDebug() << "Request to Prepare Next Frame" << this;
        if(!_connected) {
            _connected = true;
            emit connected();
            repaint();
        }

        if(renderState.testFlag(SurfaceDirty) && !renderState.testFlag(ViewReady)) {
            forceRenderTimer.start();
            renderTimer.start();
        }

        renderState |= ViewReady;
    }

    inline void keyType(char c) {
        if(!keyboardFocus.isNull())
            keyboardFocus.data()->keyTypedEvent(c);
        else
            keyTypedEvent(c);
    }

    inline void keyPress(int k) {
        if(!buttons.contains(k))
            buttons.append(k);
        if(!keyboardFocus.isNull())
            keyboardFocus.data()->keyPressedEvent(k);
        else
            keyPressedEvent(k);
    }

    inline void keyRelease(int k) {
        buttons.removeOne(k);
        if(!keyboardFocus.isNull())
            keyboardFocus.data()->keyTypedEvent(k);
        else
            keyTypedEvent(k);
    }

    inline void resetKeys() {
        QList<int> btnCopy = buttons;
        foreach(int k, btnCopy)
            keyRelease(k);
    }

    inline void mouseMove(QPoint p){
        makeCurrent();
        _mousePos = p;

        if(!mouseDragFocus.isNull())
            mouseDragFocus.data()->mouseDraggedEvent(mouseDragFocus.data()->mapFromSurface(p));
        else {
            MoeGraphicsObjectPointer newHoverFocus = _localGeometry.contains(p) ? MoeGraphicsContainer::mouseMovedEvent(p) : 0;

            if(mouseHoverFocus != newHoverFocus) {
                if(!mouseHoverFocus.isNull()) {
                    if(newHoverFocus.isNull())
                        mouseLeaveEvent();
                    disconnect(mouseHoverFocus.data(), SIGNAL(cursorChanged(QCursor)), backend(), SLOT(setCursor(QCursor)));
                } else if(!newHoverFocus.isNull())
                    mouseEnterEvent();
                mouseHoverFocus = newHoverFocus;
                if(!mouseHoverFocus.isNull()) {
                    connect(mouseHoverFocus.data(), SIGNAL(cursorChanged(QCursor)), backend(), SLOT(setCursor(QCursor)), Qt::DirectConnection);
                    if(backend())
                        backend()->setCursor(mouseHoverFocus.data()->cursor());
                }
            }
        }
    }

    inline void mousePress(QPoint p, int b){
        _mouseButtons = b;
        if(!mouseDragFocus.isNull())
            mouseDragFocus.data()->mousePressedEvent(mouseDragFocus.data()->mapFromSurface(p), b);
        else if(!mouseHoverFocus.isNull()) {
            mouseHoverFocus.data()->mousePressedEvent(mouseHoverFocus.data()->mapFromSurface(p), b);
            if(mouseHoverFocus.data()->canUseKeyFocus())
                keyboardFocus = mouseHoverFocus.data();
            mouseDragFocus = mouseHoverFocus;
        }
    }

    inline void mouseRelease(QPoint p, int b){
        _mouseButtons = b;
        if(mouseDragFocus.data()) {
            mouseDragFocus.data()->mouseReleasedEvent(mouseDragFocus.data()->mapFromSurface(p), b);
            if(!b) // All buttons released
                mouseDragFocus = 0;
        }
    }

    inline void notifyDisconnected() {
        if(!_connected)
            return;

        resetKeys();
        _connected = false;
        emit disconnected();
    }

private slots:
    inline void renderNow(){
        if(!renderState.testFlag(SurfaceDirty)
                || !renderState.testFlag(ViewReady))
            return;
        renderTimer.stop();
        forceRenderTimer.stop();
        render(0, repaintRegion);
        repaintRegion = QRect();
        renderState = NotReady;
    }

    inline void disconnectQuit() {
        disconnect(quitConnection);
    }

protected:
    explicit inline MoeAbstractGraphicsSurface(AbstractSurfaceBackend* backend, MoeObject* par =0) : MoeGraphicsContainer(par) {
        _mouseButtons = 0;
        _connected = false;
        renderState = NotReady;
        _background = QBrush(Qt::darkMagenta);
        _border = Qt::transparent;
        repaintDebug = RepaintDebugOff;
        qRegisterMetaType<RenderInstructions>("RenderInstructions");

        _backend = backend;
        connect(backend, SIGNAL(mouseMove(QPoint)), this, SLOT(mouseMove(QPoint)), Qt::QueuedConnection);
        connect(backend, SIGNAL(mousePress(QPoint,int)), this, SLOT(mousePress(QPoint,int)), Qt::QueuedConnection);
        connect(backend, SIGNAL(mouseRelease(QPoint,int)), this, SLOT(mouseRelease(QPoint,int)), Qt::QueuedConnection);

        connect(backend, SIGNAL(keyType(char)), this, SLOT(keyType(char)), Qt::QueuedConnection);
        connect(backend, SIGNAL(keyPress(int)), this, SLOT(keyPress(int)), Qt::QueuedConnection);
        connect(backend, SIGNAL(keyRelease(int)), this, SLOT(keyRelease(int)), Qt::QueuedConnection);

        connect((QObject*)engine(), SIGNAL(changingContent()), this, SLOT(disconnectQuit()));
        quitConnection = connect(backend, SIGNAL(destroyed()), (QObject*)engine(), SLOT(quit()), Qt::QueuedConnection);
        connect(this, SIGNAL(destroyed()), backend, SLOT(deleteLater()), Qt::QueuedConnection);
        connect(this, SIGNAL(resized(QSizeF)), backend, SLOT(setSize(QSizeF)), Qt::QueuedConnection);
        connect(this, SIGNAL(moved(QPointF)), backend, SLOT(setPos(QPointF)), Qt::QueuedConnection);

        connect(backend, SIGNAL(geometryChanged(QRect)), this, SLOT(prepareNextFrame()), Qt::QueuedConnection);
        connect(backend, SIGNAL(geometryChanged(QRect)), this, SLOT(setGeometry(QRect)), Qt::QueuedConnection);
        connect(backend, SIGNAL(readyForFrame()), this, SLOT(prepareNextFrame()), Qt::QueuedConnection);

        connect(backend, SIGNAL(disconnected()), this, SLOT(notifyDisconnected()), Qt::QueuedConnection);

        forceRenderTimer.setInterval(500);
        forceRenderTimer.setSingleShot(true);
        forceRenderTimer.moveToThread(thread());
        connect(&forceRenderTimer, SIGNAL(timeout()), this, SLOT(renderNow()));
        renderTimer.setInterval(0);
        renderTimer.setSingleShot(true);
        renderTimer.moveToThread(thread());
        connect(&renderTimer, SIGNAL(timeout()), this, SLOT(renderNow()));
        setGeometry(backend->geom());
    }


    inline void repaint(QRect region =QRect()){
        if(!_connected)
            return;

        if(region.isNull() || !_background.isOpaque())
            region = _localGeometry;
        else
            region &= _localGeometry;

        //qDebug() << "Request to Repaint" << this << region;
        if(region.isEmpty())
            return;

        if(repaintDebug)
            repaintRegions.append(region);

        if(repaintRegion.isEmpty())
            repaintRegion = region;
        else
            repaintRegion |= region;

        if(renderState.testFlag(ViewReady) && !renderState.testFlag(SurfaceDirty)) {
            forceRenderTimer.start();
            renderTimer.start();
        }

         renderState |= SurfaceDirty;
    }

    inline void updateCursor(QCursor cur) {
        if(activeCursor.shape() == cur.shape())
            return;

        emit cursorChanged(cur);
        activeCursor = cur;
    }

    inline AbstractSurfaceBackend* backend() {
        return _backend.data();
    }

private:
    inline void makeCurrent() {
        _currentSurface.setLocalData(MoeAbstractGraphicsSurfacePointer(this));
    }

    static inline MoeAbstractGraphicsSurface* current() {
        return _currentSurface.localData().data();
    }

    enum RepaintDebugMode {
        RepaintDebugOff,
        RepaintDebugArea,
        RepaintDebugFrame
    };


    QList<int> buttons;
    int _mouseButtons;
    QPoint _mousePos;

    MoeGraphicsObjectPointer keyboardFocus;
    MoeGraphicsObjectPointer mouseDragFocus;
    MoeGraphicsObjectPointer mouseHoverFocus;

    QMetaObject::Connection quitConnection;
    static QThreadStorage<MoeAbstractGraphicsSurfacePointer> _currentSurface;
    QPointer<AbstractSurfaceBackend> _backend;
    RepaintDebugMode repaintDebug;
    QList<QRect> repaintRegions;
    RenderState renderState;
    QCursor activeCursor;
    QColor repaintColor;
    QRect repaintRegion;
    QTimer forceRenderTimer;
    QTimer renderTimer;
    bool _connected;
};

#endif // MOEABSTRACTGRAPHICSSURFACE_H
