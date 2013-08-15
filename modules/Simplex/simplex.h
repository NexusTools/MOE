#ifndef SIMPLEX_H
#define SIMPLEX_H

#include <core/moemodule.h>

#include "simplex_global.h"


class SIMPLEXSHARED_EXPORT Simplex : public MoeModule
{
    Q_OBJECT
public:
    Q_INVOKABLE Simplex();

    inline int FASTFLOOR(float x){
        return((x)>0) ? ((int)x) : (((int)x)-1);
    }

    Q_INVOKABLE inline float grad( int hash, float x ) {
        int h = hash & 15;
        float grad = 1.0f + (h & 7);   // Gradient value 1.0, 2.0, ..., 8.0
        if ((h&8)>0) grad = -grad;         // Set a random sign for the gradient
        return ( grad * x );           // Multiply the gradient with the distance
    }

    Q_INVOKABLE inline float grad( int hash, float x, float y ) {
        int h = hash & 7;      // Convert low 3 bits of hash code
        float u = h<4 ? x : y;  // into 8 simple gradient directions,
        float v = h<4 ? y : x;  // and compute the dot product with (x,y).
        return ((h&1)>0? -u : u) + ((h&2)>0? -2.0f*v : 2.0f*v);
    }

    Q_INVOKABLE inline float grad( int hash, float x, float y , float z ) {
        int h = hash & 15;     // Convert low 4 bits of hash code into 12 simple
        float u = h<8 ? x : y; // gradient directions, and compute dot product.
        float v = h<4 ? y : h==12||h==14 ? x : z; // Fix repeats at h = 12 to 15
        return ((h&1)>0? -u : u) + ((h&2)>0? -v : v);
    }

    Q_INVOKABLE inline float grad( int hash, float x, float y, float z, float t ) {
        int h = hash & 31;      // Convert low 5 bits of hash code into 32 simple
        float u = h<24 ? x : y; // gradient directions, and compute dot product.
        float v = h<16 ? y : z;
        float w = h<8 ? z : t;
        return ((h&1)>0? -u : u) + ((h&2)>0? -v : v) + ((h&4)>0? -w : w);
    }

    Q_INVOKABLE float noise1D(float x);
    Q_INVOKABLE float noise2D(float x, float y);
    Q_INVOKABLE float noise3D(float x, float y, float z);
    Q_INVOKABLE float noise4D(float x, float y, float z, float w);

};

#endif // SIMPLEX_H
