#ifndef REAPER_OBJECT_OBJECT_H
#define REAPER_OBJECT_OBJECT_H

#include "object/base.h"

namespace reaper {
namespace object {

// Shots
ShotBase* laser(const Matrix &m, CompanyID c, double s, ID id);
ShotBase* light_missile(const Matrix &m, CompanyID c, double s, ID id);
ShotBase* heavy_missile(const Matrix &m, CompanyID c, double s, ID id);



}
}
#endif

/*
 * $Author: fizzgig $
 * $Date: 2002/05/21 20:50:30 $
 * $Log: object.h,v $
 * Revision 1.92  2002/05/21 20:50:30  fizzgig
 * fixed strange bug (ptr allocated by strdup not valid)
 *
 * Revision 1.91  2002/04/09 03:36:31  pstrand
 * eff.ptr koll, shot-parent-id, quadtree-update-update
 *
 * Revision 1.90  2001/08/27 12:55:29  peter
 * objektmeck...
 *
 * Revision 1.89  2001/08/06 12:16:37  peter
 * MegaMerge (se strandy_test-grenen för diffar...)
 *
 * Revision 1.88.2.2  2001/08/05 14:01:31  peter
 * objektmeck...
 *
 * Revision 1.88.2.1  2001/08/03 13:44:11  peter
 * pragma once obsolete...
 *
 * Revision 1.88  2001/07/30 23:43:29  macke
 * Häpp, då var det kört.
 *
 * Revision 1.87  2001/07/06 01:47:32  macke
 * Refptrfix/headerfilsstäd/objekt-skapande/mm
 *
 * Revision 1.86  2001/06/07 05:14:25  macke
 * Reaper v0.9
 *
 */

