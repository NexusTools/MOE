#ifndef RENDERBUFFER_H
#define RENDERBUFFER_H

#include <QThreadStorage>
#include <QMetaMethod>
#include <QObject>
#include <QThread>
#include <QPixmap>
#include <QHash>

class RenderBuffer;
class MoeGraphicsSurface;

typedef QMap<quintptr, RenderBuffer*> RenderBufferStorage;

class RenderBuffer : public QObject
{
    Q_OBJECT
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
    
public slots:
    inline void load(QByteArray data) {
        _data = data;
        _dataId = qHash(_data);
    }

    inline void surfaceDestroyed(QObject* obj) {
        surfaceDataVersion.remove((quintptr)obj);
    }
    
private:
    friend class RenderRecorder;

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
