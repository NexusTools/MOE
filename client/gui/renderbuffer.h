#ifndef RENDERBUFFER_H
#define RENDERBUFFER_H

#include <QThreadStorage>
#include <QMetaMethod>
#include <QObject>
#include <QThread>
#include <QPixmap>
#include <QDebug>
#include <QHash>

class RenderBuffer;
class MoeAbstractGraphicsSurface;

typedef QMap<quintptr, RenderBuffer*> RenderBufferStorage;

class RenderBuffer : public QObject
{
    Q_OBJECT
    friend class RenderRecorder;

public:
    static inline RenderBuffer* instance(QObject* buffer, bool createIfNoneExist =true) {
        RenderBuffer* instance = 0;
        if(renderBufferStorage.hasLocalData())
            instance = renderBufferStorage.localData()->value((quintptr)buffer, 0);
        if(createIfNoneExist && !instance) {
            instance = new RenderBuffer(buffer);
            if(!renderBufferStorage.hasLocalData())
                renderBufferStorage.setLocalData(new RenderBufferStorage);
            renderBufferStorage.localData()->insert((quintptr)buffer, instance);
        }

        return instance;
    }

    static inline quintptr idFor(QObject* obj, bool createIfNoneExist =true) {
        RenderBuffer* buffer = instance(obj, createIfNoneExist);
        if(buffer)
            return buffer->id();
        return 0;
    }

    inline quintptr id() {
        return _id;
    }

    inline bool updateSurfaceVersion(MoeAbstractGraphicsSurface* surface) {
        if(!surfaceDataVersion.contains((quintptr)surface)) {
            //TODO: Notify the newly added surface when this buffer is destroyed
            qDebug() << this << "has been added to" << surface << "at version" << _dataId;
            surfaceDataVersion.insert((quintptr)surface, _dataId);
            return true;
        }
        if(surfaceDataVersion.value((quintptr)surface) == _dataId)
            return false;

        qDebug() << this << "updated for" << surface << "with version" << _dataId;
        surfaceDataVersion.insert((quintptr)surface, _dataId);
        return true;
    }

    inline QByteArray data() const{
        return _data;
    }
    
public slots:
    inline void load(QByteArray data) {
        _data = data;
        _dataId = qHash(_data);
        qDebug() << this << "new data version" << _dataId;
    }

    inline void surfaceDestroyed(QObject* obj) {
        surfaceDataVersion.remove((quintptr)obj);
    }
    
private:
    inline explicit RenderBuffer(QObject *buffer) {
        _id = (quintptr)buffer;
        connect(buffer, SIGNAL(destroyed()), this, SLOT(deleteLater()), Qt::QueuedConnection);
    }

    quintptr _id;
    uint _dataId;
    QByteArray _data;
    QMap<quintptr, uint> surfaceDataVersion;
    static QThreadStorage<RenderBufferStorage*> renderBufferStorage;
};

#endif // RENDERBUFFER_H
