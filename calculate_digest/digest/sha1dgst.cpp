//#include "stdafx.h"
#if !defined(OPENSSL_NO_SHA1) && !defined(OPENSSL_NO_SHA)
#undef  SHA_0
#define SHA_1

#include "sha_locl.h"

#endif
