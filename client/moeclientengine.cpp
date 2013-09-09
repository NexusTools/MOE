#include "moeclientengine.h"

#include "gui/moegraphicsimage.h"
#include "gui/moegraphicscontainer.h"
#include "gui/moegraphicssurface.h"
#include "gui/moegraphicstext.h"

#include "opengl/moeglscene.h"
#include "opengl/moeglspritesheet.h"
#include "opengl/moeglgraphicsview.h"
#include "opengl/moeglslshaderprogram.h"
#include "opengl/moeglvertexmodel.h"
#include "opengl/moeglcubemodel.h"

void __moe_client__registerScriptConverters(QScriptEngine* eng);

void MoeClientEngine::initializeScriptEngine(QScriptEngine* eng) {
    __moe_client__registerScriptConverters(eng);
}

void MoeClientEngine::initializeContentEnvironment(QScriptEngine* eng, QScriptValue globalObject) {
    QList<const QMetaObject*> classesToLoad;
    classesToLoad.append(&MoeGraphicsImage::staticMetaObject);
    classesToLoad.append(&MoeGraphicsObject::staticMetaObject);
    classesToLoad.append(&MoeResourceRequest::staticMetaObject);
    classesToLoad.append(&MoeGraphicsContainer::staticMetaObject);
    classesToLoad.append(&MoeGraphicsSurface::staticMetaObject);
    classesToLoad.append(&MoeGraphicsText::staticMetaObject);

    // OpenGL - Might be moved to separate module
    classesToLoad.append(&MoeGLScene::staticMetaObject);
    classesToLoad.append(&MoeGLSpriteSheet::staticMetaObject);
    classesToLoad.append(&MoeGLGraphicsView::staticMetaObject);
    classesToLoad.append(&MoeGLSLShaderProgram::staticMetaObject);
    classesToLoad.append(&MoeGLVertexModel::staticMetaObject);
    classesToLoad.append(&MoeGLCubeModel::staticMetaObject);

    foreach(const QMetaObject* metaObject, classesToLoad)  {
        QString key(metaObject->className());
        if(key.startsWith("Moe"))
            key = key.mid(3);

        qDebug() << key;
        globalObject.setProperty(key, eng->newQMetaObject(metaObject));
    }
}

