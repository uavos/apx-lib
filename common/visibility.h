/****************************************************************************
 *
 *   Copyright (c) 2019 Aliaksei Stratsilatau. All rights reserved.
 *
 ****************************************************************************/

#pragma once

#ifdef __EXPORT
#undef __EXPORT
#endif
#define __EXPORT __attribute__ ((visibility ("default")))

#ifdef __PRIVATE
#undef __PRIVATE
#endif
#define __PRIVATE __attribute__ ((visibility ("hidden")))

#ifdef __cplusplus

#define __BEGIN_DECLS   extern "C" {
#define __END_DECLS		}

#else

#define __BEGIN_DECLS
#define __END_DECLS

#endif
