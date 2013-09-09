#include "qpaintersurfacebackend.h"

inline QGLShader* getShader(QString path, QGLShader::ShaderTypeBit type=
                                            (QGLShader::ShaderTypeBit)-1) {
    static QHash<QString, QGLShader*> shaders;
    QGLShader* shader = shaders.value(path);
    if(!shader) {
        if(type == -1) {
            if(path.endsWith(".vert"))
                type = QGLShader::Vertex;
            else if(path.endsWith(".frag"))
                type = QGLShader::Fragment;
            else if(path.endsWith(".geom"))
                type = QGLShader::Geometry;
            else
                throw "Invalid shader";
        }
        shader = new QGLShader(type);

        if(!shader->compileSourceFile(path))
            qWarning() << path << shader->log();
    }
    return shader;
}

void QPainterSurfaceBackend::paint(QPainter& p) {
    activeGLBuffer = 0;
    p.setPen(Qt::black);
    p.setBrush(Qt::darkMagenta);
    p.setClipRect(pendingPaintRect);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setRenderHint(QPainter::HighQualityAntialiasing);

    foreach(RenderInstruction inst, pendingInstructions) {
        switch(inst.type){
            case RenderInstruction::FillRect:
                p.fillRect(inst.arguments.at(0).toRectF(), QColor::fromRgba(inst.arguments.at(1).toUInt()));
                break;

            case RenderInstruction::DrawLine:
                p.drawLine(inst.arguments.at(0).toPointF(),
                           inst.arguments.at(1).toPointF());
                break;

            case RenderInstruction::DrawPixel:
                p.drawPoint(inst.arguments.first().toPointF());
                break;

            case RenderInstruction::DrawRect:
                if(inst.arguments.length() >= 2)
                    p.drawRoundedRect(inst.arguments.at(0).toRectF(), inst.arguments.at(1).toReal(), inst.arguments.at(1).toReal(), Qt::AbsoluteSize);
                else
                    p.drawRect(inst.arguments.at(0).toRectF());
                break;

            case RenderInstruction::DrawText:
                p.drawText(inst.arguments.at(0).toRectF(), Qt::AlignCenter, inst.arguments.at(1).toString());
                break;

            case RenderInstruction::UpdatePen:
                if(inst.arguments.isEmpty())
                    p.setPen(Qt::NoPen);
                else
                    p.setPen(QPen(QColor::fromRgba(inst.arguments.at(0).toUInt()), inst.arguments.at(1).toInt()));
                break;

            case RenderInstruction::UpdateBrush:
                if(inst.arguments.isEmpty())
                    p.setBrush(Qt::NoBrush);
                else
                    p.setBrush(inst.arguments.at(0).value<QBrush>());
                break;

            case RenderInstruction::UpdateClipRect:
                if(inst.arguments.isEmpty())
                    p.setClipRect(pendingPaintRect, Qt::ReplaceClip);
                else
                    p.setClipRect(inst.arguments.first().toRectF(), Qt::ReplaceClip);
            break;

            case RenderInstruction::UpdateOpacity:
                p.setOpacity(inst.arguments.first().toFloat());
            break;

            case RenderInstruction::UpdateFont:
                p.setFont(QFont(inst.arguments.at(0).toString(), inst.arguments.at(1).toInt()));
            break;

            case RenderInstruction::UpdateTransform:
                if(inst.arguments.isEmpty())
                    p.setTransform(QTransform());
                else
                    p.setTransform(inst.arguments.first().value<QTransform>());
            break;

            case RenderInstruction::ResizeGLScene:
            {
                quintptr id = inst.arguments.first().value<quintptr>();
                QSize size = inst.arguments.at(1).toSize();

                QGLFramebufferObjectFormat bufferFormat;
                bufferFormat.setInternalTextureFormat(GL_RGBA8);
                bufferFormat.setMipmap(true);

                GLRenderBuffer* glBuffer = glBuffers.value(id);
                if(!glBuffer) {
                    glBuffer = new GLRenderBuffer;

                    glBuffer->shaderProgram.addShader(getShader(":/shaders/matrix.vert"));
                    glBuffer->shaderProgram.addShader(getShader(":/shaders/colour.frag"));
                    if(!glBuffer->shaderProgram.link()) {
                        qWarning() << glBuffer->shaderProgram.log();
                        return;
                    }

                    glBuffer->attrib.vector = glBuffer->shaderProgram.attributeLocation("vertexPosition");
                    glBuffer->attrib.colour = glBuffer->shaderProgram.attributeLocation("vertexColour");
                    glBuffer->attrib.matrix = glBuffer->shaderProgram.uniformLocation("modelMatrix");
                    glBuffer->attrib.camMatrix = glBuffer->shaderProgram.uniformLocation("matrix");

                    glBuffer->fbo = new QGLFramebufferObject(size, bufferFormat);
                    glBuffers.insert(id, glBuffer);
                } else if(glBuffer->fbo->size() != size) {
                    delete glBuffer->fbo;
                    glBuffer->fbo = new QGLFramebufferObject(size, bufferFormat);
                    glBuffers.insert(id, glBuffer);
                }
                break;
            }

            case RenderInstruction::UpdateGLMatrix:
            {
                quintptr id = inst.arguments.first().value<quintptr>();

                GLRenderBuffer* glBuffer = glBuffers.value(id);
                if(glBuffer) {
                    glBuffer->camMatrix = inst.arguments.at(1).value<QMatrix4x4>();
                } else
                    qWarning() << "Buffer Not Initialized" << id;
                break;
            }

            case RenderInstruction::BeginRenderGLScene:
            {
                quintptr id = inst.arguments.first().value<quintptr>();

                GLRenderBuffer* glBuffer = glBuffers.value(id);
                if(!glBuffer || !glBuffer->fbo) {
                    qWarning() << "GLBuffer Not Initialized" << id;
                    return;
                }


                p.end();
                if(glBuffer->fbo->bind()) {
                    glViewport(0, 0, glBuffer->fbo->size().width(),
                               glBuffer->fbo->size().height());

                    glClearColor(0, 0, 0, 0);
                    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

                    glEnable(GL_DEPTH_TEST);
                    glEnable(GL_CULL_FACE);
                    glLoadIdentity();

                    if(!glBuffer->shaderProgram.bind()) {
                        qWarning() << "Cannot bind Shader Program";
                        begin(p);
                        break;
                    }

                    glBuffer->shaderProgram.setUniformValue(glBuffer->attrib.camMatrix, glBuffer->camMatrix);
                    glBuffer->shaderProgram.enableAttributeArray(glBuffer->attrib.vector);
                    glBuffer->shaderProgram.enableAttributeArray(glBuffer->attrib.colour);

                    activeGLBuffer = glBuffer;
                } else {
                    qWarning() << "Cannot bind FBO";
                    begin(p);
                }
                break;
            }


            case RenderInstruction::FinishRenderGLScene:
            {
                if(!activeGLBuffer) {
                    qWarning() << "No Active GL Scene";
                    break;
                }

                activeGLBuffer->shaderProgram.disableAttributeArray(activeGLBuffer->attrib.vector);
                activeGLBuffer->shaderProgram.disableAttributeArray(activeGLBuffer->attrib.colour);
                activeGLBuffer->shaderProgram.release();

                activeGLBuffer->fbo->release();
                activeGLBuffer = 0;

                glDisable(GL_DEPTH_TEST);
                glDisable(GL_CULL_FACE);
                begin(p);

                break;
            }

            case RenderInstruction::AllocateGLModel:
            {
                MoeObjectPtr ptr = inst.arguments.first().value<MoeObjectPtr>();
                vec3::list vectors = inst.arguments.at(1).value<vec3::list>();
                vec3::list colours = inst.arguments.at(2).value<vec3::list>();

                GLModel* model = glModels.value(ptr);
                if(!model) {
                    model = new GLModel;
                    model->vectors.create();
                    model->colours.create();
                    glModels.insert(ptr, model);
                }

                if(model->vectors.bind()) {
                    model->vectors.allocate(vectors.data(), vectors.size()*sizeof(vec3));
                    model->vectors.release();
                } else
                    qWarning() << "Failed to allocate vector buffer...";

                if(model->colours.bind()) {
                    model->colours.allocate(colours.data(), colours.size()*sizeof(vec3));
                    model->colours.release();
                } else
                    qWarning() << "Failed to allocate colour buffer...";

                break;
            }

            case RenderInstruction::UpdateGLModelMatrix:
            {
                MoeObjectPtr ptr = inst.arguments.first().value<MoeObjectPtr>();

                GLModel* model = glModels.value(ptr);
                if(!model) {
                    qWarning() << "Attempted To Render Unallocated Model" << ptr;
                    break;
                }

                model->matrix = inst.arguments.at(1).value<QMatrix4x4>();

                break;
            }

            case RenderInstruction::RenderGLModel:
            {
                if(!activeGLBuffer) {
                    qWarning() << "No Active GL Scene";
                    break;
                }

                MoeObjectPtr ptr = inst.arguments.first().value<MoeObjectPtr>();

                GLModel* model = glModels.value(ptr);
                if(!model) {
                    qWarning() << "Attempted To Render Unallocated Model" << ptr;
                    break;
                }

                activeGLBuffer->shaderProgram.setUniformValue(activeGLBuffer->attrib.matrix, model->matrix);

                if(!model->vectors.bind()) {
                    qWarning() << "Failed to bind Vector Buffer";
                    break;
                }
                activeGLBuffer->shaderProgram.setAttributeBuffer(activeGLBuffer->attrib.vector, GL_FLOAT, 0, 3);

                if(!model->colours.bind()) {
                    qWarning() << "Failed to bind Colour Buffer";
                    model->vectors.release();
                    break;
                }
                activeGLBuffer->shaderProgram.setAttributeBuffer(activeGLBuffer->attrib.colour, GL_FLOAT, 0, 3);

                glDrawArrays(GL_TRIANGLES, 0, model->vectors.size());
                model->colours.release();
                model->vectors.release();

                break;
            }

            case RenderInstruction::BufferLoadImage:
            {
                quintptr id = inst.arguments.first().value<quintptr>();
                QByteArray data = inst.arguments.at(1).toByteArray();

                QPixmap buffer;
                buffer.loadFromData(data);
                renderBuffers.insert(id, buffer);
                break;
            }

            case RenderInstruction::RenderBuffer:
            {
                quintptr id = inst.arguments.first().value<quintptr>();
                QRectF dest = inst.arguments.at(1).toRectF();
                QPixmap buffer = renderBuffers.value(id);

                if(buffer.isNull()) {
                    GLRenderBuffer* glBuffer = glBuffers.value(id);
                    if(glBuffer && glBuffer->fbo)
                        blitBuffer(dest.toRect(), glBuffer->fbo);
                    else if(!renderBuffers.contains(id)) {
                        p.drawTiledPixmap(dest, getCheckerBoardImage());
                        return;
                    } else
                        p.drawTiledPixmap(dest, getCheckerBoardImage(Qt::red));
                } else {
                    QSizeF scaledSize(buffer.size().scaled(dest.width(), dest.height(), Qt::KeepAspectRatio));
                    p.drawPixmap(QRectF(dest.topLeft() + QPointF(dest.width()/2-scaledSize.width()/2,
                                       dest.height()/2-scaledSize.height()/2), scaledSize), buffer,
                                        QRectF(QPoint(0,0),buffer.size()));
                }
            }
            break;

            default:
                qWarning() << "Unhandled Recorder Instruction" << inst.type;
            break;
        }
    }
    pendingInstructions.clear();
    markRendered();
}
