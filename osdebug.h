#ifndef __OSDEBUG_H__
#define __OSDEBUG_H__

#ifdef ENABLE_DEBUG
    #define DBGOUT printf
#else
    #define DBGOUT(...)
#endif

#endif
