#ifndef __MYBOOL_H

#define __MYBOOL_H

#ifndef BOOL
#ifdef __cplusplus
typedef bool BOOL;
const BOOL FALSE = false;
const BOOL TRUE = true;
#else
typedef enum { FALSE, TRUE } BOOL;
#endif
#endif

#endif
