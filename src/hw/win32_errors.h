
/* $Id: win32_errors.h,v 1.3 2001/11/03 09:45:40 peter Exp $ */


#ifndef REAPER_HW_WIN32_ERRORS_H
#define REAPER_HW_WIN32_ERRORS_H

#ifdef __cplusplus
extern "C" {
#endif


char* ds_error(int e);
char* di_error(int e);

const char* win32_strerror();

#ifdef __cplusplus
}
#endif

#endif

