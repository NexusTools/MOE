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

            case RenderInstruction::BlitGLBuffer:
            {
                quintptr id = inst.arguments.first().value<quintptr>();
                qDebug() << "Blitting GLBuffer" << id;
                GLRenderBuffer* glBuff = glBuffers.value(id);
                if(!glBuff) {
                    qWarning() << "Unknown GLBuffer specified" << id;
                    continue;
                }

                QVariant dest = inst.arguments.at(1);
                QVariant data = inst.arguments.at(2);

                QRect blitDest;
                QPixmap source;

                switch(data.type()) {
                    case QVariant::ByteArray:
                        if(!source.loadFromData(data.toByteArray()))
                            source = getCheckerBoardImage(Qt::red);
                        break;

                    default:
                        qWarning() << "Incompatible DataType Specified for Blitting Source";
                    continue;
                }

                switch(dest.type()) {
                    case QVariant::Point:
                        blitDest = QRect(dest.toPoint(), source.size());
                        break;

                    case QVariant::Size:
                        blitDest = QRect(QPoint(0, 0), dest.toSize());
                        break;

                    case QVariant::Rect:
                        blitDest = dest.toRect();
                        break;

                    default:
                        qWarning() << "Incompatible DataType Specified for Destination";
                    continue;
                }

                glDisable(GL_CULL_FACE);
                glDisable(GL_TEXTURE_2D);
                glDisable(GL_DEPTH_TEST);
                qDebug() << "Blitting buffer" << blitDest << source.size();
                if(p.begin(glBuff->fbo)) {
                    p.drawPixmap(QRect(QPoint(0,0),
                        glBuff->fbo->size()), source);
                    p.end();
                }
                source.save("/home/luke/Desktop/source.png");
                glBuff->fbo->toImage().save("/home/luke/Desktop/test.png");
                glEnable(GL_TEXTURE_2D);
                glEnable(GL_DEPTH_TEST);
                glEnable(GL_CULL_FACE);
                break;
            }

            case RenderInstruction::AllocateGLBuffer:
            {
                quintptr id = inst.arguments.first().value<quintptr>();
                QSize size = inst.arguments.at(1).toSize();
                qDebug() << "Allocating GLBuffer" << id << size;

                QGLFramebufferObjectFormat bufferFormat;
                bufferFormat.setInternalTextureFormat(GL_RGBA8);
                bufferFormat.setSamples(0);

                GLRenderBuffer* glBuffer = glBuffers.value(id);
                if(!glBuffer) {
                    glBuffer = new GLRenderBuffer;

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

                    glEnable(GL_CULL_FACE);
                    glEnable(GL_DEPTH_TEST);
                    glEnable(GL_TEXTURE_2D);
                    glLoadIdentity();

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

                activeGLBuffer->fbo->release();
                activeGLBuffer = 0;

                glDisable(GL_TEXTURE_2D);
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
                vec2::list textCoords;
                if(inst.arguments.size() > 3)
                    textCoords = inst.arguments.at(3).value<vec2::list>();

                foreach(vec3 v, vectors)
                    qDebug() << v.vector();
                foreach(vec2 v, textCoords)
                    qDebug() << v.vector();

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

                if(!textCoords.isEmpty()) {
                    if(!model->textCoords.isCreated())
                        model->textCoords.create();
                    if(model->textCoords.bind()) {
                        model->textCoords.allocate(textCoords.data(), textCoords.size()*sizeof(vec2));
                        model->textCoords.release();
                    } else
                        qWarning() << "Failed to allocate texture coordinate buffer...";
                } else if(model->textCoords.isCreated())
                    model->textCoords.destroy();

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

            case RenderInstruction::UpdateGLModelTexture:
            {
                MoeObjectPtr ptr = inst.arguments.first().value<MoeObjectPtr>();

                GLModel* model = glModels.value(ptr);
                if(!model) {
                    qWarning() << "Attempted To Render Unallocated Model" << ptr;
                    break;
                }

                MoeObjectPtr buffPtr = inst.arguments.at(1).value<MoeObjectPtr>();
                GLRenderBuffer* buffer = glBuffers.value(buffPtr);
                if(!buffer) {
                    qWarning() << "Attempted To Attach Unknown Render Buffer" << buffPtr;
                    break;
                }

                qDebug() << "Set Texture for Model" << ptr << buffPtr << buffer->fbo->texture();
                model->texture = buffer->fbo->texture();
                break;
            }

            case RenderInstruction::UpdateGLModelShader:
            {
                MoeObjectPtr ptr = inst.arguments.first().value<MoeObjectPtr>();

                GLModel* model = glModels.value(ptr);
                if(!model) {
                    qWarning() << "Attempted To Render Unallocated Model" << ptr;
                    break;
                }

                QString shaderName = inst.arguments.at(1).toString();
                qDebug() << "Set Shader for Model" << ptr << shaderName;
                model->shader = shaderPrograms.value(shaderName);
                if(!model->shader) {
                    model->shader = new ShaderProgram;

                    model->shader->program.addShader(getShader(shaderName == "coloured" ? ":/shaders/matrix.vert" : ":/shaders/matrix-texture.vert"));
                    model->shader->program.addShader(getShader(shaderName == "coloured" ? ":/shaders/colour.frag" : ":/shaders/texture.frag"));
                    if(!model->shader->program.link()) {
                        qWarning() << model->shader->program.log();
                        return;
                    }

                    model->shader->attrib.vector = model->shader->program.attributeLocation("vertexPosition");
                    model->shader->attrib.colour = model->shader->program.attributeLocation("vertexColour");
                    model->shader->attrib.matrix = model->shader->program.uniformLocation("modelMatrix");
                    model->shader->attrib.camMatrix = model->shader->program.uniformLocation("matrix");
                    model->shader->attrib.texture = model->shader->program.uniformLocation("texture");
                    model->shader->attrib.texCoord = model->shader->program.attributeLocation("texCoord");

                    qDebug() << "Compiled Shader Program" << shaderName
                                         << "Attributes"
                                         << model->shader->attrib.vector
                                         << model->shader->attrib.colour
                                         << model->shader->attrib.texCoord
                                         << "Uniforms"
                                         << model->shader->attrib.matrix
                                         << model->shader->attrib.camMatrix
                                         << model->shader->attrib.texture;

                    shaderPrograms.insert(shaderName, model->shader);
                }
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

                if(!model->shader->program.bind()) {
                    qWarning() << "Cannot bind Shader Program";
                    begin(p);
                    break;
                }

                model->shader->program.setUniformValue(model->shader->attrib.camMatrix, activeGLBuffer->camMatrix);
                model->shader->program.setUniformValue(model->shader->attrib.matrix, model->matrix);
                model->shader->program.enableAttributeArray(model->shader->attrib.vector);
                model->shader->program.enableAttributeArray(model->shader->attrib.colour);

                if(!model->vectors.bind()) {
                    qWarning() << "Failed to bind Vector Buffer";
                    break;
                }
                model->shader->program.setAttributeBuffer(model->shader->attrib.vector, GL_FLOAT, 0, 3);

                if(!model->colours.bind()) {
                    qWarning() << "Failed to bind Colour Buffer";
                    model->vectors.release();
                    break;
                }
                model->shader->program.setAttributeBuffer(model->shader->attrib.colour, GL_FLOAT, 0, 3);

                if(model->textCoords.isCreated()) {
                    if(model->shader->attrib.texCoord == -1) {
                        qWarning() << "Texture Coordinate buffer compiled but shader has no texCoord attribute";
                        break;
                    }
                    if(!model->texture) {
                        qWarning() << "Texture Coordinate buffer compiled but shader has no texCoord attribute";
                        break;
                    }

                    model->shader->program.enableAttributeArray(model->shader->attrib.texCoord);
                    if(!model->textCoords.bind()) {
                        qWarning() << "Failed to bind Colour Buffer";
                        model->vectors.release();
                        model->colours.release();
                        break;
                    }
                    model->shader->program.setAttributeBuffer(model->shader->attrib.texCoord, GL_FLOAT, 0, 2);

                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, model->texture);
                    model->shader->program.setUniformValue(model->shader->attrib.texture, 0);
                }

                glDrawArrays(GL_TRIANGLES, 0, model->vectors.size());
                if(model->textCoords.isCreated()) {
                    model->textCoords.release();
                    model->shader->program.disableAttributeArray(model->shader->attrib.texCoord);
                }
                model->colours.release();
                model->vectors.release();

                model->shader->program.disableAttributeArray(model->shader->attrib.colour);
                model->shader->program.disableAttributeArray(model->shader->attrib.vector);

                model->shader->program.release();

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
