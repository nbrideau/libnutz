
#ifndef _LIBNUTZ_H_INCLUDED
#define _LIBNUTZ_H_INCLUDED

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Force halt after X connections
#undef SRV_FORCE_HALT
//#define SRV_FORCE_HALT 100

// For helgrind do not detach threads
#undef THREAD_NO_DETACH
//#define THREAD_NO_DETACH


#endif

