#ifndef REAPER_GFX_MATRIX_UTILS_H
#define REAPER_GFX_MATRIX_UTILS_H

/*
 * Matrix utility functions to replace GLU matrix functions
 * This provides replacements for gluPerspective, gluLookAt, gluOrtho2D
 */

namespace reaper {
namespace gfx {
namespace matrix_utils {

// Replacement for gluPerspective
void perspective(double fovy, double aspect, double zNear, double zFar);

// Replacement for gluLookAt  
void look_at(double eyeX, double eyeY, double eyeZ,
             double centerX, double centerY, double centerZ,
             double upX, double upY, double upZ);

// Replacement for gluOrtho2D
void ortho_2d(double left, double right, double bottom, double top);

} // namespace matrix_utils
} // namespace gfx
} // namespace reaper

#endif // REAPER_GFX_MATRIX_UTILS_H