// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__F2B94DAF_9A7D_11D1_B2C7_00001B4B970B__INCLUDED_)
#define AFX_STDAFX_H__F2B94DAF_9A7D_11D1_B2C7_00001B4B970B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define _AFX_ALL_WARNINGS

#define _USE_MATH_DEFINES 

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

// (a.walling 2008-04-01 12:13) - PLID 29497 - We require Windows 2000 or later
// (a.walling 2008-04-03 17:13) - PLID 29497 - I thought we needed this defined for gdi+ to work correctly, but it does without. Due to
// MFC issues, if _WIN32_WINNT is 0x0500+ and we on VC6 and a newer platform SDK, an inconsistency in a structure definition will cause
// an access violation.
// (a.walling 2007-11-06 12:12) - PLID 27974 - VS2008
#if _MSC_VER > 1300
// .NET visual studio
// (a.walling 2010-10-21 14:46) - Added these to limit ambiguity of target versions

#ifndef WINVER                          // Specifies that the minimum required platform is Windows 2000
#define WINVER 0x0600           // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT            // Specifies that the minimum required platform is Windows 2000
#define _WIN32_WINNT 0x0600     // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINDOWS          // Specifies that the minimum required platform is Windows 2000
#define _WIN32_WINDOWS 0x0600 // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_IE                       // Specifies that the minimum required platform is Internet Explorer 6.0.
#define _WIN32_IE 0x0800        // Change this to the appropriate value to target other versions of IE.
#endif

#else
// VC6
#define _WIN32_WINNT 0x0400  //Let us use function requiring NT 4 or later
#endif

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxole.h>         // MFC OLE classes
#include <afxodlgs.h>       // MFC OLE dialog classes
#include <afxdisp.h>        // MFC OLE automation classes
#include <afxadv.h>
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
#include <afxtempl.h>

// (a.walling 2012-04-25 16:21) - PLID 49987 - Include ATL core headers
#include <atlbase.h>
#include <atlcom.h>
//d.thompson 10/6/2015 - PLID 67294 - This microsoft header does not compile without warnings!  We seem to have no choice but to temporarily disable the warnings while we include it.
//	Additionally, this is a workaround for boost.  Practice does not ever actually include <utility>, but boost does.  If we don't put this here, then 
//	boost will include utility, have not disabled the warning, and cause a warning, which due to build settings, is treated as an error.
#pragma warning(push)
#pragma warning( disable : 4996)
#pragma warning( disable : 4503)
#include <utility>
#pragma warning(pop)

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#include <deque>
#include <stack>
#include <queue>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <list>

// (a.walling 2014-07-08 14:19) - PLID 62812 - Use MFCArray
#include <NxDataUtilitiesLib/MFCArray.h>

//TES 6/13/2008 - PLID 30392 - This imports WinSock V1; we use V2 now, and commenting this out doesn't seem to hurt anything.
//#include <afxsock.h>		// MFC socket extensions

/***** MINIDUMP DEBUG DEFINES *****/
// (a.walling 2008-05-16 11:24)
// uncomment these defines, or include them in the build config, to enable minidumping.
// Currently this will be used only for investigative purposes, although eventually it
// may be a good idea to get this in releases for access violations.

// include the code to create minidumps
//#define DBG_CREATE_MINIDUMP

// uncomment this line to automatically create a minidump on an access violation.
//#define DBG_CREATE_MINIDUMP_ON_AV

// uncomment this line to automatically create a minidump on any handled exception
//#define DBG_CREATE_MINIDUMP_ON_EXCEPTION

/*****                        *****/

// (a.walling 2007-11-06 15:03) - PLID 27800 - VS2008 - Need to include these here
#include <mshtml.h>
#include <mshtmhst.h>

#include "NxAdoImport.h"

// (a.walling 2013-05-09 20:03) - PLID 56623 - Moved topaz sig pad include out of global scope, encapsulating internals in CTopazSigPadInfo
// (a.walling 2011-07-05 15:44) - PLID 44445 - If you are using the bad typelibs, this will
// fail to compile, unless you define GIVE_ME_THE_BAD_ADO_TYPELIBS

//#define GIVE_ME_THE_BAD_ADO_TYPELIBS

#ifndef GIVE_ME_THE_BAD_ADO_TYPELIBS

__if_exists(ADODB::_Connection_Deprecated) {
	struct AssertCompatibleADOTypeLibrary_Practice : public ADO_Backwards_Compatible_TypeLibrary
	{
	};
};

#else

#pragma message("WARNING - may be using the bad ADO typelibs!")

#endif

#import "NxDataList.tlb"

#import "NxDataList2.tlb"

// this is VS.NET or higher
#include <comdef.h>

// (a.walling 2011-03-11 13:52) - no PLID - Just making the smart pointers globally available
#include <boost/static_assert.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/scoped_array.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/cast.hpp>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/optional.hpp>
#include <boost/noncopyable.hpp>
#include <boost/ref.hpp>

#include <NxSystemUtilitiesLib/NxSmartPtr.h>

using boost::scoped_ptr;
using boost::scoped_array;
using boost::shared_ptr;
using boost::make_shared;
using boost::weak_ptr;
using boost::enable_shared_from_this;
using boost::polymorphic_downcast;

// (a.walling 2011-09-07 18:01) - PLID 45448 - NxAdo unification
#define NO_NXADO_NAMESPACE
#define NO_ADODB_NAMESPACE

// (a.walling 2008-04-01 12:15) - PLID 29497 - GDI+ is here!
// (a.walling 2008-04-14 14:20) - PLID 29630 - Use our special header
#include "NxGdiPlus.h"

// (a.walling 2011-09-16 13:01) - PLID 45531 - Include MFC Feature pack definitions
#include <afxcontrolbars.h>

// (a.walling 2007-11-06 12:12) - PLID 28000 - VS2008 - Handle our namespaces better
#if _MSC_VER > 1300

namespace SmallSTDOLE2Lib {

};

#else

// VC 6.0
#import "stdole2.tlb" rename_namespace("SmallSTDOLE2Lib") exclude("GUID", "DISPPARAMS", "EXCEPINFO", \
	"OLE_XPOS_PIXELS", "OLE_YPOS_PIXELS", "OLE_XSIZE_PIXELS", "OLE_YSIZE_PIXELS", "OLE_XPOS_HIMETRIC", \
	"OLE_YPOS_HIMETRIC", "OLE_XSIZE_HIMETRIC", "OLE_YSIZE_HIMETRIC", "OLE_XPOS_CONTAINER", \
	"OLE_YPOS_CONTAINER", "OLE_XSIZE_CONTAINER", "OLE_YSIZE_CONTAINER", "OLE_HANDLE", "OLE_OPTEXCLUSIVE", \
	"OLE_CANCELBOOL", "OLE_ENABLEDEFAULTBOOL", "FONTSIZE", "OLE_COLOR", \
	"IUnknown", "IDispatch", "IEnumVARIANT", "IFont", "IPicture", "IFontDisp", "IPictureDisp")

#endif // _MSC_VER > 1300

#import "NxTab.tlb" rename_namespace("NxTab") inject_statement("using namespace SmallSTDOLE2Lib;")

#import "richtexteditor.tlb" inject_statement("using namespace SmallSTDOLE2Lib;")

#import "nxcolumngraph.tlb" rename_namespace("ColumnGraph") inject_statement("using namespace SmallSTDOLE2Lib;")

#import "NxTime.tlb"

#include <PragmaTodo.h>

// Include resource .h files
#include "resource.h"

// Include NexTech Libraries
#include <NxUILib/NxStatic.h>
#include <NxUILib/GlobalDrawingUtils.h>
#include <NxUILib/NexTechIconButton.h>
#include <NxUILib/NxEdit.h>
#include <NxUILib/NxButton.h>
#include <NxUILib/NexTechDialog.h>

// Include Controls Extended by Practice
#include "NxIconButton.h"
#include "NxLabel.h"
#include "DateTimePicker.h"
#include "NxDialog.h"
#include "NxColor.h"

// Several other common headers
#include "Practice.h"
#include "NxTabView.h"
#include "GlobalUtils.h"
#include "MainFrm.h"
#include "Pracprops.h"
#include "NxStandard.h"
#include "PathStringUtils.h"
#include "NxException.h"
#include "NxMessageDef.h"
#include "NxSecurity.h"
#include "PracticeLicense.h"
#include "NxSliderCtrl.h"
//TES 8/9/2007 - PLID 26883 - Copied the parameterized functions out of PracProps, and into this file,
// which can be shared with other projects.
#include "GlobalParamUtils.h"
#include <afxdhtml.h>
#include <afxdlgs.h>
#include <afx.h>

#include <NxTaskDialog.h>

#include <ImportDll.h>
#include <Guard.h>
#include <WindowlessUtils.h>
#include <HotSpot.h>
#include <NxInkPictureText.h>
#include <NexTechDialogView.h>
#include <NxScrollAnchor.h>
#include <SpellExUtils.h>
#include <FileUtils.h>
#include <GlobalLabUtils.h>

#include "wfxctl32.h"

#include "SelectPacketDlg.h"
#include "SingleSelectDlg.h"
#include "MultiSelectDlg.h"
#include "DontShowDlg.h"
#include "ShowConnectingFeedbackDlg.h"

#include "NxTWAIN.h"

// (a.walling 2014-04-30 15:19) - PLID 61989 - import a typelibrary rather than a dll
#import "wmp.tlb"
#import <NxWindowless.tlb>
#import "OPOSScanner.tlb"
#import "OPOSCashDrawer.tlb"
#import "OPOSMSR.tlb"
#import "OPOSPOSPrinter.tlb"

#include "opos.h"

// (a.walling 2012-08-03 14:31) - PLID 51956 - Compiler limits exceeded with imported NxAccessor typelib - get WordApp[lication] out of stdafx

#include "peplus.h"

#include <NxDataUtilitiesLib/iString.h>
#include <NxAlgorithm.h>

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.


#endif // !defined(AFX_STDAFX_H__F2B94DAF_9A7D_11D1_B2C7_00001B4B970B__INCLUDED_)
