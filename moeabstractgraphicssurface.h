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

typedef QPointer<MoeAbstractGraphicsSurface> MoeAbstractGraphicsSurfacePointer;

class MoeAbstractGraphicsSurface : public MoeGraphicsContainer
{
    Q_OBJECT
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

public slots:
    inline void repaint(QRect region =QRect()){
        if(!_connected)
            return;

        if(region.isNull() || _background.alpha() < 255)
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

        if(renderState.testFlag(ViewReady) && !renderState.testFlag(SurfaceDirty)){
            //qDebug() << "Starting Repaint Timer";
            renderTimer.start();
        }

         renderState |= SurfaceDirty;
    }

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
            //qDebug() << "Starting Repaint Timer";
            renderTimer.start();
        }

        renderState |= ViewReady;
    }

    inline void mouseMove(QPoint p){
        makeCurrent();

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
                    backend()->setCursor(mouseHoverFocus.data()->cursor());
                }
            }
        }
    }

    inline void mousePress(QPoint p, int b){
        if(!mouseDragFocus.isNull())
            mouseDragFocus.data()->mousePressedEvent(mouseDragFocus.data()->mapFromSurface(p), b);
        else if(!mouseHoverFocus.isNull())
            mouseHoverFocus.data()->mousePressedEvent(mouseHoverFocus.data()->mapFromSurface(p), b);
        else {
            MoeGraphicsContainer::mousePressedEvent(p, b);
            if(mouseHoverFocus.data() != mouseDragFocus.data()) {
                //qDebug() << "Started dragging" << mouseHoverFocus.data();
                mouseDragFocus = mouseHoverFocus;
            }
        }
    }

    inline void mouseRelease(QPoint p, int b){
        if(mouseDragFocus.data()) {
            mouseDragFocus.data()->mouseReleasedEvent(mouseDragFocus.data()->mapFromSurface(p), b);
            if(!b) // All buttons released
                mouseDragFocus = 0;
        } else
            MoeGraphicsContainer::mouseReleasedEvent(p, b);
    }

    inline void notifyDisconnected() {
        if(!_connected)
            return;

        //qDebug() << "Surface Backend Disconnected" << this;
        _connected = false;
        emit disconnected();
    }

private slots:
    inline void renderNow(){
        //qDebug() << "Rendering Surface" << repaintRegion << this;
        render(0, repaintRegion);
        repaintRegion = QRect();
        renderState = NotReady;
    }

protected:
    explicit inline MoeAbstractGraphicsSurface(AbstractSurfaceBackend* backend, MoeObject* par =0) : MoeGraphicsContainer(par) {
        _connected = false;
        renderState = NotReady;
        _background = QColor(Qt::darkMagenta).rgba();
        _foreground = qRgba(0, 0, 0, 0);
        _border = qRgba(0, 0, 0, 0);
        repaintDebug = RepaintDebugOff;
        qRegisterMetaType<RenderInstructions>("RenderInstructions");

        _backend = backend;
        connect(backend, SIGNAL(mouseMove(QPoint)), this, SLOT(mouseMove(QPoint)), Qt::QueuedConnection);

        connect(this, SIGNAL(destroyed()), backend, SLOT(deleteLater()), Qt::QueuedConnection);
        connect(this, SIGNAL(resized(QSizeF)), backend, SLOT(setSize(QSizeF)), Qt::QueuedConnection);
        connect(this, SIGNAL(moved(QPointF)), backend, SLOT(setPos(QPointF)), Qt::QueuedConnection);

        connect(backend, SIGNAL(geometryChanged(QRect)), this, SLOT(setGeometry(QRect)), Qt::QueuedConnection);
        connect(backend, SIGNAL(readyForFrame()), this, SLOT(prepareNextFrame()), Qt::QueuedConnection);

        connect(backend, SIGNAL(disconnected()), this, SLOT(notifyDisconnected()), Qt::QueuedConnection);

        renderTimer.setInterval(0);
        renderTimer.setSingleShot(true);
        renderTimer.moveToThread(thread());
        connect(&renderTimer, SIGNAL(timeout()), this, SLOT(renderNow()));
        setGeometry(backend->geom());
    }

    inline void updateCursor(QCursor cur) {
        if(activeCursor.shape() == cur.shape())
            return;

        emit cursorChanged(cur);
        activeCursor = cur;
    }

    inline AbstractSurfaceBackend* backend() {
        return _backend;
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

    MoeGraphicsObjectPointer keyboardFocus;
    MoeGraphicsObjectPointer mouseDragFocus;
    MoeGraphicsObjectPointer mouseHoverFocus;

    static QThreadStorage<MoeAbstractGraphicsSurfacePointer> _currentSurface;
    AbstractSurfaceBackend* _backend;
    RepaintDebugMode repaintDebug;
    QList<QRect> repaintRegions;
    RenderState renderState;
    QCursor activeCursor;
    QColor repaintColor;
    QRect repaintRegion;
    QTimer renderTimer;
    bool _connected;
};

#endif // MOEABSTRACTGRAPHICSSURFACE_H
