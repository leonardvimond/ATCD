
// -*- C++ -*-
// $Id$
// Definition for Win32 Export directives.
// This file is generated automatically by generate_export_file.pl STATICPLANNER_EXEC
// ------------------------------
#ifndef STATICPLANNER_EXEC_EXPORT_H
#define STATICPLANNER_EXEC_EXPORT_H

#include "ace/config-all.h"

#if defined (ACE_AS_STATIC_LIBS) && !defined (STATICPLANNER_EXEC_HAS_DLL)
#  define STATICPLANNER_EXEC_HAS_DLL 0
#endif /* ACE_AS_STATIC_LIBS && STATICPLANNER_EXEC_HAS_DLL */

#if !defined (STATICPLANNER_EXEC_HAS_DLL)
#  define STATICPLANNER_EXEC_HAS_DLL 1
#endif /* ! STATICPLANNER_EXEC_HAS_DLL */

#if defined (STATICPLANNER_EXEC_HAS_DLL) && (STATICPLANNER_EXEC_HAS_DLL == 1)
#  if defined (STATICPLANNER_EXEC_BUILD_DLL)
#    define STATICPLANNER_EXEC_Export ACE_Proper_Export_Flag
#    define STATICPLANNER_EXEC_SINGLETON_DECLARATION(T) ACE_EXPORT_SINGLETON_DECLARATION (T)
#    define STATICPLANNER_EXEC_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_EXPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  else /* STATICPLANNER_EXEC_BUILD_DLL */
#    define STATICPLANNER_EXEC_Export ACE_Proper_Import_Flag
#    define STATICPLANNER_EXEC_SINGLETON_DECLARATION(T) ACE_IMPORT_SINGLETON_DECLARATION (T)
#    define STATICPLANNER_EXEC_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK) ACE_IMPORT_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#  endif /* STATICPLANNER_EXEC_BUILD_DLL */
#else /* STATICPLANNER_EXEC_HAS_DLL == 1 */
#  define STATICPLANNER_EXEC_Export
#  define STATICPLANNER_EXEC_SINGLETON_DECLARATION(T)
#  define STATICPLANNER_EXEC_SINGLETON_DECLARE(SINGLETON_TYPE, CLASS, LOCK)
#endif /* STATICPLANNER_EXEC_HAS_DLL == 1 */

// Set STATICPLANNER_EXEC_NTRACE = 0 to turn on library specific tracing even if
// tracing is turned off for ACE.
#if !defined (STATICPLANNER_EXEC_NTRACE)
#  if (ACE_NTRACE == 1)
#    define STATICPLANNER_EXEC_NTRACE 1
#  else /* (ACE_NTRACE == 1) */
#    define STATICPLANNER_EXEC_NTRACE 0
#  endif /* (ACE_NTRACE == 1) */
#endif /* !STATICPLANNER_EXEC_NTRACE */

#if (STATICPLANNER_EXEC_NTRACE == 1)
#  define STATICPLANNER_EXEC_TRACE(X)
#else /* (STATICPLANNER_EXEC_NTRACE == 1) */
#  if !defined (ACE_HAS_TRACE)
#    define ACE_HAS_TRACE
#  endif /* ACE_HAS_TRACE */
#  define STATICPLANNER_EXEC_TRACE(X) ACE_TRACE_IMPL(X)
#  include "ace/Trace.h"
#endif /* (STATICPLANNER_EXEC_NTRACE == 1) */

#endif /* STATICPLANNER_EXEC_EXPORT_H */

// End of auto generated file.