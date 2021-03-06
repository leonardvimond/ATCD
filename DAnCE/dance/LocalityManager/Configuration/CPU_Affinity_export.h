// -*- C++ -*-
// Definition for Win32 Export directives.
// ------------------------------
#ifndef CPU_AFFINITY_EXPORT_H
#define CPU_AFFINITY_EXPORT_H

#include "ace/config-all.h"

#if defined (ACE_AS_STATIC_LIBS) && !defined (CPU_AFFINITY_HAS_DLL)
#  define CPU_AFFINITY_HAS_DLL 0
#endif /* ACE_AS_STATIC_LIBS && CPU_AFFINITY_HAS_DLL */

#if !defined (CPU_AFFINITY_HAS_DLL)
#  define CPU_AFFINITY_HAS_DLL 1
#endif /* ! CPU_AFFINITY_HAS_DLL */

#if defined (CPU_AFFINITY_HAS_DLL) && (CPU_AFFINITY_HAS_DLL == 1)
#  if defined (CPU_AFFINITY_BUILD_DLL)
#    define CPU_Affinity_Export ACE_Proper_Export_Flag
#    define CPU_AFFINITY_SINGLETON_DECLARATION(T) ACE_EXPORT_SINGLETON_DECLARATION (T)
#    define CPU_AFFINITY_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_EXPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  else /* CPU_AFFINITY_BUILD_DLL */
#    define CPU_Affinity_Export ACE_Proper_Import_Flag
#    define CPU_AFFINITY_SINGLETON_DECLARATION(T) ACE_IMPORT_SINGLETON_DECLARATION (T)
#    define CPU_AFFINITY_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_IMPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  endif /* CPU_AFFINITY_BUILD_DLL */
#else /* CPU_AFFINITY_HAS_DLL == 1 */
#  define CPU_Affinity_Export
#  define CPU_AFFINITY_SINGLETON_DECLARATION(T)
#  define CPU_AFFINITY_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#endif /* CPU_AFFINITY_HAS_DLL == 1 */

// Set CPU_AFFINITY_NTRACE = 0 to turn on library specific tracing even if
// tracing is turned off for ACE.
#if !defined (CPU_AFFINITY_NTRACE)
#  if (ACE_NTRACE == 1)
#    define CPU_AFFINITY_NTRACE 1
#  else /* (ACE_NTRACE == 1) */
#    define CPU_AFFINITY_NTRACE 0
#  endif /* (ACE_NTRACE == 1) */
#endif /* !CPU_AFFINITY_NTRACE */

#if (CPU_AFFINITY_NTRACE == 1)
#  define CPU_AFFINITY_TRACE(X)
#else /* (CPU_AFFINITY_NTRACE == 1) */
#  if !defined (ACE_HAS_TRACE)
#    define ACE_HAS_TRACE
#  endif /* ACE_HAS_TRACE */
#  define CPU_AFFINITY_TRACE(X) ACE_TRACE_IMPL(X)
#  include "ace/Trace.h"
#endif /* (CPU_AFFINITY_NTRACE == 1) */

#endif /* CPU_AFFINITY_EXPORT_H */

// End of auto generated file.
