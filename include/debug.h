#ifndef __DEBUG_H
#define __DEBUG_H

#ifdef SUPPORT_UBOOT
#define CRITICAL 0
#define ALWAYS 0
#define INFO 1
#define SPEW 2
#define DEBUGLEVEL 1

#define dprintf(level, x...) do { if ((level) <= DEBUGLEVEL) { printf(x); } } while (0)
#endif

#endif
