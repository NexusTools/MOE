/*
  @name         "SimplexNoise1234"
  @author       "Luke Fay <aero@nexustools.net>"
  @credits      ["Ken Perlin", "Stefan Gustavson <stegu@itn.liu.se>"]
  @copyright    "2003-2011"
  @license      "Public Domain"
  @dependancies ["prototype"]
*/

var Simplex = Class.create({

    /*
    * Permutation table. This is just a random jumble of all numbers 0-255,
    * repeated twice to avoid wrapping the index at 255 for each lookup.
    * This needs to be exactly the same for all instances on all platforms,
    * so it's easiest to just keep it as explicit data.
    * This also removes the need for any initialisation of this class.
    *
    * Note that making this an var[] instead of a char[] might make the
    * code run faster on platforms with a high penalty for unaligned single
    * byte addressing. Intel x86 is generally single-byte-friendly, but
    * some other CPUs are faster with 4-aligned reads.
    * However, a char[] is smaller, which avoids cache trashing, and that
    * is probably the most important aspect on most architectures.
    * This array is accessed a *lot* by the noise functions.
    * A vector-valued noise over 3D accesses it 96 times, and a
    * var-valued 4D noise 64 times. We want this to fit in the cache!
    */
    perm: [151,160,137,91,90,15,
    131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
    190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
    88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
    77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
    102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
    135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
    5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
    223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
    129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
    251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
    49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
    151,160,137,91,90,15,
    131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
    190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
    88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
    77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
    102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
    135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
    5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
    223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
    129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
    251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
    49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
    ],

    // A lookup table to traverse the simplex around a given povar in 4D.
    // Details can be found where this table is used, in the 4D noise method.
    /* TODO: This should not be required, backport it from Bill's GLSL code! */
    simplex: [[0,1,2,3],[0,1,3,2],[0,0,0,0],[0,2,3,1],[0,0,0,0],[0,0,0,0],[0,0,0,0],[1,2,3,0],
    [0,2,1,3],[0,0,0,0],[0,3,1,2],[0,3,2,1],[0,0,0,0],[0,0,0,0],[0,0,0,0],[1,3,2,0],
    [0,0,0,0],[0,0,0,0],[0,0,0,0],[0,0,0,0],[0,0,0,0],[0,0,0,0],[0,0,0,0],[0,0,0,0],
    [1,2,0,3],[0,0,0,0],[1,3,0,2],[0,0,0,0],[0,0,0,0],[0,0,0,0],[2,3,0,1],[2,3,1,0],
    [1,0,2,3],[1,0,3,2],[0,0,0,0],[0,0,0,0],[0,0,0,0],[2,0,3,1],[0,0,0,0],[2,1,3,0],
    [0,0,0,0],[0,0,0,0],[0,0,0,0],[0,0,0,0],[0,0,0,0],[0,0,0,0],[0,0,0,0],[0,0,0,0],
    [2,0,1,3],[0,0,0,0],[0,0,0,0],[0,0,0,0],[3,0,1,2],[3,0,2,1],[0,0,0,0],[3,1,2,0],
    [2,1,0,3],[0,0,0,0],[0,0,0,0],[0,0,0,0],[3,1,0,2],[0,0,0,0],[3,2,0,1],[3,2,1,0]],
    FASTFLOOR: function(x){ // lol
        return Math.floor(x);//((x)>0) ? ((var)x) : (((var)x)-1);
    },


    /*
    * Helper functions to compute gradients-dot-residualvectors (1D to 4D)
    * Note that these generate gradients of more than unit length. To make
    * a close match with the value range of classic Perlin noise, the final
    * noise values need to be rescaled to fit nicely within [-1,1].
    * (The simplex noise functions as such also have different scaling.)
    * Note also that these noise functions are the most practical and useful
    * signed version of Perlin noise. To return values according to the
    * RenderMan specification from the SL noise() and pnoise() functions,
    * the noise values need to be scaled and offset to [0,1], like this:
    * var SLnoise = (Simplex::noise(x,y,z) + 1.0) * 0.5;
    */

    grad1D: function(hash,x) {
        var h = hash & 15;
        var grad = 1.0 + (h & 7); // Gradient value 1.0, 2.0, ..., 8.0
        if ((h&8)>0) grad = -grad; // Set a random sign for the gradient
        return ( grad * x ); // Multiply the gradient with the distance
    },
    grad2D: function(hash,x,y) {
        var h = hash & 7; // Convert low 3 bits of hash code
        var u = h<4 ? x : y; // varo 8 simple gradient directions,
        var v = h<4 ? y : x; // and compute the dot product with (x,y).

        return ((h&1)>0? -u : u) + ((h&2)>0? -2.0*v : 2.0*v);
    },
    grad3D: function(hash,x,y,z) {
        var h = hash & 15; // Convert low 4 bits of hash code varo 12 simple
        var u = h<8 ? x : y; // gradient directions, and compute dot product.
        var v = h<4 ? y : h==12||h==14 ? x : z; // Fix repeats at h = 12 to 15
        return ((h&1)>0? -u : u) + ((h&2)>0? -v : v);
    },

    grad4D: function(hash,x,y,z,t) {
        var h = hash & 31; // Convert low 5 bits of hash code varo 32 simple
        var u = h<24 ? x : y; // gradient directions, and compute dot product.
        var v = h<16 ? y : z;
        var w = h<8 ? z : t;
        return ((h&1)>0? -u : u) + ((h&2)>0? -v : v) + ((h&4)>0? -w : w);
    },


    // funky numbers
    F2: 0.366025403, //0.5*(sqrt(3.0)-1.0)
    G2: 0.211324865, //(3.0-Math.sqrt(3.0))/6.0
    F3: 0.333333333, //"skewing factor"
    G3: 0.166666667, //"skewing factor"
    F4: 0.309016994, //(Math.sqrt(5.0)-1.0)/4.0
    G4: 0.138196601, //(5.0-Math.sqrt(5.0))/20.0


    // 1D simplex noise
    noise1D: function(x) {

      var i0 = this.FASTFLOOR(x);
      var i1 = i0 + 1;
      var x0 = x - i0;
      var x1 = x0 - 1.0;

      var n0, n1;

      var t0 = 1.0 - x0*x0;
    // if(t0 < 0.0f) t0 = 0.0f;
      t0 *= t0;
      n0 = t0 * t0 * this.grad1D(this.perm[i0 & 0xff], x0);

      var t1 = 1.0 - x1*x1;
    // if(t1 < 0.0f) t1 = 0.0f;
      t1 *= t1;
      n1 = t1 * t1 * this.grad1D(this.perm[i1 & 0xff], x1);
      // The maximum value of this noise is 8*(3/4)^4 = 2.53125
      // A factor of 0.395 would scale to fit exactly within [-1,1], but
      // we want to match PRMan's 1D noise, so we scale it down some more.
      return 0.25 * (n0 + n1);

    },

    // 2D simplex noise
    noise2D: function(x,y) {

        var n0, n1, n2; // Noise contributions from the three corners

        // Skew the input space to determine which simplex cell we're in
        var s = (x+y)*this.F2; // Hairy factor for 2D
        var xs = x + s;
        var ys = y + s;
        var i = this.FASTFLOOR(xs);
        var j = this.FASTFLOOR(ys);

        var t = (i+j)*this.G2;
        var X0 = i-t; // Unskew the cell origin back to (x,y) space
        var Y0 = j-t;
        var x0 = x-X0; // The x,y distances from the cell origin
        var y0 = y-Y0;

        // For the 2D case, the simplex shape is an equilateral triangle.
        // Determine which simplex we are in.
        var i1, j1; // Offsets for second (middle) corner of simplex in (i,j) coords
        if(x0>y0) {i1=1; j1=0;} // lower triangle, XY order: (0,0)->(1,0)->(1,1)
        else {i1=0; j1=1;} // upper triangle, YX order: (0,0)->(0,1)->(1,1)

        // A step of (1,0) in (i,j) means a step of (1-c,-c) in (x,y), and
        // a step of (0,1) in (i,j) means a step of (-c,1-c) in (x,y), where
        // c = (3-sqrt(3))/6

        var x1 = x0 - i1 + this.G2; // Offsets for middle corner in (x,y) unskewed coords
        var y1 = y0 - j1 + this.G2;
        var x2 = x0 - 1.0 + 2.0 * this.G2; // Offsets for last corner in (x,y) unskewed coords
        var y2 = y0 - 1.0 + 2.0 * this.G2;

        // Wrap the vareger indices at 256, to avoid indexing perm[] out of bounds
        var ii = i & 0xff;
        var jj = j & 0xff;

        // Calculate the contribution from the three corners
        var t0 = 0.5 - x0*x0-y0*y0;
        if(t0 < 0.0) n0 = 0.0;
        else {
          t0 *= t0;
          n0 = t0 * t0 * this.grad2D(this.perm[ii+this.perm[jj]], x0, y0);
        }

        var t1 = 0.5 - x1*x1-y1*y1;
        if(t1 < 0.0) n1 = 0.0;
        else {
          t1 *= t1;
          n1 = t1 * t1 * this.grad2D(this.perm[ii+i1+this.perm[jj+j1]], x1, y1);
        }

        var t2 = 0.5 - x2*x2-y2*y2;
        if(t2 < 0.0) n2 = 0.0;
        else {
          t2 *= t2;
          n2 = t2 * t2 * this.grad2D(this.perm[ii+1+this.perm[jj+1]], x2, y2);
        }

        // Add contributions from each corner to get the final noise value.
        // The result is scaled to return values in the varerval [-1,1].
        return 40.0 * (n0 + n1 + n2); // TODO: The scale factor is preliminary!
    },

    //3D simplex noise
    noise3D: function(x,y,z) {

        var n0, n1, n2, n3; // Noise contributions from the four corners

        // Skew the input space to determine which simplex cell we're in
        var s = (x+y+z)*this.F3; // Very nice and simple skew factor for 3D
        var xs = x+s;
        var ys = y+s;
        var zs = z+s;
        var i = this.FASTFLOOR(xs);
        var j = this.FASTFLOOR(ys);
        var k = this.FASTFLOOR(zs);

        var t = (i+j+k)*this.G3;
        var X0 = i-t; // Unskew the cell origin back to (x,y,z) space
        var Y0 = j-t;
        var Z0 = k-t;
        var x0 = x-X0; // The x,y,z distances from the cell origin
        var y0 = y-Y0;
        var z0 = z-Z0;

        // For the 3D case, the simplex shape is a slightly irregular tetrahedron.
        // Determine which simplex we are in.
        var i1, j1, k1; // Offsets for second corner of simplex in (i,j,k) coords
        var i2, j2, k2; // Offsets for third corner of simplex in (i,j,k) coords

    /* This code would benefit from a backport from the GLSL version! */
        if(x0>=y0) {
          if(y0>=z0)
            { i1=1; j1=0; k1=0; i2=1; j2=1; k2=0; } // X Y Z order
            else if(x0>=z0) { i1=1; j1=0; k1=0; i2=1; j2=0; k2=1; } // X Z Y order
            else { i1=0; j1=0; k1=1; i2=1; j2=0; k2=1; } // Z X Y order
          }
        else { // x0<y0
          if(y0<z0) { i1=0; j1=0; k1=1; i2=0; j2=1; k2=1; } // Z Y X order
          else if(x0<z0) { i1=0; j1=1; k1=0; i2=0; j2=1; k2=1; } // Y Z X order
          else { i1=0; j1=1; k1=0; i2=1; j2=1; k2=0; } // Y X Z order
        }

        // A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
        // a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
        // a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
        // c = 1/6.

        var x1 = x0 - i1 + this.G3; // Offsets for second corner in (x,y,z) coords
        var y1 = y0 - j1 + this.G3;
        var z1 = z0 - k1 + this.G3;
        var x2 = x0 - i2 + 2.0*this.G3; // Offsets for third corner in (x,y,z) coords
        var y2 = y0 - j2 + 2.0*this.G3;
        var z2 = z0 - k2 + 2.0*this.G3;
        var x3 = x0 - 1.0 + 3.0*this.G3; // Offsets for last corner in (x,y,z) coords
        var y3 = y0 - 1.0 + 3.0*this.G3;
        var z3 = z0 - 1.0 + 3.0*this.G3;

        // Wrap the vareger indices at 256, to avoid indexing perm[] out of bounds
        var ii = i & 0xff;
        var jj = j & 0xff;
        var kk = k & 0xff;

        // Calculate the contribution from the four corners
        var t0 = 0.6 - x0*x0 - y0*y0 - z0*z0;
        if(t0 < 0.0) n0 = 0.0;
        else {
          t0 *= t0;
          n0 = t0 * t0 * this.grad3D(this.perm[ii+this.perm[jj+this.perm[kk]]], x0, y0, z0);
        }

        var t1 = 0.6 - x1*x1 - y1*y1 - z1*z1;
        if(t1 < 0.0) n1 = 0.0;
        else {
          t1 *= t1;
          n1 = t1 * t1 * this.grad3D(this.perm[ii+i1+this.perm[jj+j1+this.perm[kk+k1]]], x1, y1, z1);
        }

        var t2 = 0.6 - x2*x2 - y2*y2 - z2*z2;
        if(t2 < 0.0) n2 = 0.0;
        else {
          t2 *= t2;
          n2 = t2 * t2 * this.grad3D(this.perm[ii+i2+this.perm[jj+j2+this.perm[kk+k2]]], x2, y2, z2);
        }

        var t3 = 0.6 - x3*x3 - y3*y3 - z3*z3;
        if(t3<0.0) n3 = 0.0;
        else {
          t3 *= t3;
          n3 = t3 * t3 * this.grad3D(this.perm[ii+1+this.perm[jj+1+this.perm[kk+1]]], x3, y3, z3);
        }

        // Add contributions from each corner to get the final noise value.
        // The result is scaled to stay just inside [-1,1]
        return 32.0 * (n0 + n1 + n2 + n3); // TODO: The scale factor is preliminary!
    },

    // 4D simplex noise
    noise4D: function(x,y,z,w) {

        var n0, n1, n2, n3, n4; // Noise contributions from the five corners

        // Skew the (x,y,z,w) space to determine which cell of 24 simplices we're in
        var s = (x + y + z + w) * this.F4; // Factor for 4D skewing
        var xs = x + s;
        var ys = y + s;
        var zs = z + s;
        var ws = w + s;
        var i = this.FASTFLOOR(xs);
        var j = this.FASTFLOOR(ys);
        var k = this.FASTFLOOR(zs);
        var l = this.FASTFLOOR(ws);

        var t = (i + j + k + l) * this.G4; // Factor for 4D unskewing
        var X0 = i - t; // Unskew the cell origin back to (x,y,z,w) space
        var Y0 = j - t;
        var Z0 = k - t;
        var W0 = l - t;

        var x0 = x - X0; // The x,y,z,w distances from the cell origin
        var y0 = y - Y0;
        var z0 = z - Z0;
        var w0 = w - W0;

        // For the 4D case, the simplex is a 4D shape I won't even try to describe.
        // To find out which of the 24 possible simplices we're in, we need to
        // determine the magnitude ordering of x0, y0, z0 and w0.
        // The method below is a good way of finding the ordering of x,y,z,w and
        // then find the correct traversal order for the simplex we???re in.
        // First, six pair-wise comparisons are performed between each possible pair
        // of the four coordinates, and the results are used to add up binary bits
        // for an vareger index.
        var c1 = (x0 > y0) ? 32 : 0;
        var c2 = (x0 > z0) ? 16 : 0;
        var c3 = (y0 > z0) ? 8 : 0;
        var c4 = (x0 > w0) ? 4 : 0;
        var c5 = (y0 > w0) ? 2 : 0;
        var c6 = (z0 > w0) ? 1 : 0;
        var c = c1 + c2 + c3 + c4 + c5 + c6;

        var i1, j1, k1, l1; // The vareger offsets for the second simplex corner
        var i2, j2, k2, l2; // The vareger offsets for the third simplex corner
        var i3, j3, k3, l3; // The vareger offsets for the fourth simplex corner

        // simplex[c] is a 4-vector with the numbers 0, 1, 2 and 3 in some order.
        // Many values of c will never occur, since e.g. x>y>z>w makes x<z, y<w and x<w
        // impossible. Only the 24 indices which have non-zero entries make any sense.
        // We use a thresholding to set the coordinates in turn from the largest magnitude.
        // The number 3 in the "simplex" array is at the position of the largest coordinate.
        i1 = this.simplex[c][0]>=3 ? 1 : 0;
        j1 = this.simplex[c][1]>=3 ? 1 : 0;
        k1 = this.simplex[c][2]>=3 ? 1 : 0;
        l1 = this.simplex[c][3]>=3 ? 1 : 0;
        // The number 2 in the "this.simplex" array is at the second largest coordinate.
        i2 = this.simplex[c][0]>=2 ? 1 : 0;
        j2 = this.simplex[c][1]>=2 ? 1 : 0;
        k2 = this.simplex[c][2]>=2 ? 1 : 0;
        l2 = this.simplex[c][3]>=2 ? 1 : 0;
        // The number 1 in the "this.simplex" array is at the second smallest coordinate.
        i3 = this.simplex[c][0]>=1 ? 1 : 0;
        j3 = this.simplex[c][1]>=1 ? 1 : 0;
        k3 = this.simplex[c][2]>=1 ? 1 : 0;
        l3 = this.simplex[c][3]>=1 ? 1 : 0;
        // The fifth corner has all coordinate offsets = 1, so no need to look that up.

        var x1 = x0 - i1 + this.G4; // Offsets for second corner in (x,y,z,w) coords
        var y1 = y0 - j1 + this.G4;
        var z1 = z0 - k1 + this.G4;
        var w1 = w0 - l1 + this.G4;
        var x2 = x0 - i2 + 2.0*this.G4; // Offsets for third corner in (x,y,z,w) coords
        var y2 = y0 - j2 + 2.0*this.G4;
        var z2 = z0 - k2 + 2.0*this.G4;
        var w2 = w0 - l2 + 2.0*this.G4;
        var x3 = x0 - i3 + 3.0*this.G4; // Offsets for fourth corner in (x,y,z,w) coords
        var y3 = y0 - j3 + 3.0*this.G4;
        var z3 = z0 - k3 + 3.0*this.G4;
        var w3 = w0 - l3 + 3.0*this.G4;
        var x4 = x0 - 1.0 + 4.0*this.G4; // Offsets for last corner in (x,y,z,w) coords
        var y4 = y0 - 1.0 + 4.0*this.G4;
        var z4 = z0 - 1.0 + 4.0*this.G4;
        var w4 = w0 - 1.0 + 4.0*this.G4;

        // Wrap the vareger indices at 256, to avoid indexing perm[] out of bounds
        var ii = i & 0xff;
        var jj = j & 0xff;
        var kk = k & 0xff;
        var ll = l & 0xff;

        // Calculate the contribution from the five corners
        var t0 = 0.6 - x0*x0 - y0*y0 - z0*z0 - w0*w0;
        if(t0 < 0.0) n0 = 0.0;
        else {
          t0 *= t0;
          n0 = t0 * t0 * this.grad4D(this.perm[ii+this.perm[jj+this.perm[kk+this.perm[ll]]]], x0, y0, z0, w0);
        }

       var t1 = 0.6 - x1*x1 - y1*y1 - z1*z1 - w1*w1;
        if(t1 < 0.0) n1 = 0.0;
        else {
          t1 *= t1;
          n1 = t1 * t1 * this.grad4D(this.perm[ii+i1+this.perm[jj+j1+this.perm[kk+k1+this.perm[ll+l1]]]], x1, y1, z1, w1);
        }

       var t2 = 0.6 - x2*x2 - y2*y2 - z2*z2 - w2*w2;
        if(t2 < 0.0) n2 = 0.0;
        else {
          t2 *= t2;
          n2 = t2 * t2 * this.grad4D(this.perm[ii+i2+this.perm[jj+j2+this.perm[kk+k2+this.perm[ll+l2]]]], x2, y2, z2, w2);
        }

       var t3 = 0.6 - x3*x3 - y3*y3 - z3*z3 - w3*w3;
        if(t3 < 0.0) n3 = 0.0;
        else {
          t3 *= t3;
          n3 = t3 * t3 * this.grad4D(this.perm[ii+i3+this.perm[jj+j3+this.perm[kk+k3+this.perm[ll+l3]]]], x3, y3, z3, w3);
        }

       var t4 = 0.6 - x4*x4 - y4*y4 - z4*z4 - w4*w4;
        if(t4 < 0.0) n4 = 0.0;
        else {
          t4 *= t4;
          n4 = t4 * t4 * this.grad4D(this.perm[ii+1+this.perm[jj+1+this.perm[kk+1+this.perm[ll+1]]]], x4, y4, z4, w4);
        }

        // Sum up and scale the result to cover the range [-1,1]
        return 27.0 * (n0 + n1 + n2 + n3 + n4); // TODO: The scale factor is preliminary!
    }

});
