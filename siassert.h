#ifndef _SIASSERT_H
#define _SIASSERT_H

#include <stdio.h>

#ifdef _WIN32
#define ASM_INT _asm int 3
#else
#define ASM_INT asm ("int $3");
#endif

#undef ASSERT
#define ASSERT(x) if (!(x)) {printf("ASSERTION FAILED %s line %d",__FILE__,__LINE__); ASM_INT};

#endif // _SIASSERT_H
