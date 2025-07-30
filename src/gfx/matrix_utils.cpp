#include "gfx/matrix_utils.h"
#include "hw/gl.h"
#include <cmath>

namespace reaper {
namespace gfx {
namespace matrix_utils {

// Replacement for gluPerspective
void perspective(double fovy, double aspect, double zNear, double zFar)
{
    // Convert fovy from degrees to radians
    double fovy_rad = fovy * M_PI / 180.0;
    double f = 1.0 / tan(fovy_rad / 2.0);
    
    double matrix[16] = {0};
    matrix[0] = f / aspect;  // [0][0]
    matrix[5] = f;           // [1][1]
    matrix[10] = (zFar + zNear) / (zNear - zFar);     // [2][2]
    matrix[11] = -1.0;       // [2][3]
    matrix[14] = (2.0 * zFar * zNear) / (zNear - zFar); // [3][2]
    
    glMultMatrixd(matrix);
}

// Replacement for gluLookAt
void look_at(double eyeX, double eyeY, double eyeZ,
             double centerX, double centerY, double centerZ,
             double upX, double upY, double upZ)
{
    // Calculate forward vector
    double fx = centerX - eyeX;
    double fy = centerY - eyeY;
    double fz = centerZ - eyeZ;
    
    // Normalize forward vector
    double f_len = sqrt(fx*fx + fy*fy + fz*fz);
    fx /= f_len;
    fy /= f_len;
    fz /= f_len;
    
    // Calculate side vector (forward cross up)
    double sx = fy * upZ - fz * upY;
    double sy = fz * upX - fx * upZ; 
    double sz = fx * upY - fy * upX;
    
    // Normalize side vector
    double s_len = sqrt(sx*sx + sy*sy + sz*sz);
    sx /= s_len;
    sy /= s_len;
    sz /= s_len;
    
    // Calculate up vector (side cross forward)
    double ux = sy * fz - sz * fy;
    double uy = sz * fx - sx * fz;
    double uz = sx * fy - sy * fx;
    
    // Create transformation matrix
    double matrix[16] = {
        sx,  ux, -fx, 0,
        sy,  uy, -fy, 0,
        sz,  uz, -fz, 0,
        0,   0,   0,  1
    };
    
    glMultMatrixd(matrix);
    glTranslated(-eyeX, -eyeY, -eyeZ);
}

// Replacement for gluOrtho2D
void ortho_2d(double left, double right, double bottom, double top)
{
    // This is equivalent to glOrtho(left, right, bottom, top, -1, 1)
    glOrtho(left, right, bottom, top, -1.0, 1.0);
}

} // namespace matrix_utils
} // namespace gfx  
} // namespace reaper