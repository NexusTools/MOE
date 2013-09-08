#ifndef GLSLSHADERPROGRAM_H
#define GLSLSHADERPROGRAM_H

#include <core/moeobject.h>

#include <QGLShaderProgram>

class MoeGLSLShaderProgram : public MoeObject
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit MoeGLSLShaderProgram();
    
};

#endif // GLSLSHADERPROGRAM_H
