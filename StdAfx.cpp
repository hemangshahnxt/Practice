// stdafx.cpp : source file that includes just the standard includes
//	Practice.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

// 180030501	update 2
// 180030723	update 3
// 180031101	update 4


#if _MSC_FULL_VER < 180031101
#pragma message ("**************************************************************************")
#pragma message ("**************************************************************************")
#pragma message ("Hey you! Install Update 4 for Visual C++ 12 / Visual Studio 2013!!")

#if _MSC_FULL_VER < 180030501
#pragma message ("Release mode will refuse to build unless >= update 2")
#endif

#pragma message ("**************************************************************************")
#pragma message ("**************************************************************************")
#endif

#if _MSC_FULL_VER < 180030501
#ifndef _DEBUG
#error Codegen issues in compiler; install at least Update 2 for Visual C++ 12 / Visual Studio 2013!!
#endif
#endif

#include "stdafx.h"
