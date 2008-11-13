/* CoreGraphics - CGBase.h
 * Copyright (c) 2000 Apple Computer, Inc.
 * All rights reserved.
 */

#ifndef CGBASE_H_
#define CGBASE_H_

#include <stdbool.h>
#include <stddef.h>

#define CG_EXTERN_C_BEGIN
#define CG_EXTERN_C_END

#if !defined(CG_EXTERN)
#define CG_EXTERN extern
#endif

#if !defined(CG_PRIVATE_EXTERN)
#define CG_PRIVATE_EXTERN __private_extern__
#endif

#if !defined(CG_INLINE)
//#  if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
//#    define CG_INLINE static inline
//#  elif defined(__MWERKS__) || defined(__cplusplus)
#define CG_INLINE static inline
//#  elif defined(__GNUC__)
//#    define CG_INLINE static __inline__
//#  else
//#    define CG_INLINE static    
//#  endif
#endif

#if !defined(__GNUC__) && !defined(__MWERKS__)
#define __attribute__(attribute)
#endif

CG_EXTERN_C_END

#endif	/* CGBASE_H_ */
