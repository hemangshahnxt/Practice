#include "stdafx.h"
#include "EmrFonts.h"
#include <NxDataUtilitiesLib/NxSafeArray.h>

// (a.walling 2011-11-11 11:11) - PLID 46621 - Unify font handing for emr items

// (a.walling 2012-10-03 12:09) - PLID 53002 - Revert to old default fonts -- 10pt Arial

////
/// Emr Fonts via EmrFont

namespace EmrFonts
{

// (a.walling 2011-11-11 11:11) - PLID 46621 - Get the LOGFONT from which we will derive our other fonts
const LOGFONT& GetBaseLogFont()
{
	static LOGFONT baseLogFont = {0};
	if (!baseLogFont.lfHeight) {
		VERIFY(::GetObject(afxGlobalData.fontRegular, sizeof(baseLogFont), &baseLogFont));
	}

	return baseLogFont;
}

// (a.walling 2012-10-03 12:09) - PLID 53002 - Get the default EMR font or the custom font from properties
const LOGFONT& GetBaseEmrListLogFont()
{
	static LOGFONT lfDefault = 
	{
		0,
		0,
		0,
		0,
		FW_DONTCARE,
		FALSE,
		FALSE,
		FALSE,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		5 /*CLEARTYPE_QUALITY*/,
		FF_DONTCARE,
		NULL
	};

	if (!lfDefault.lfHeight) {
		strcpy(lfDefault.lfFaceName, "Arial");

		try {
			// Get the computer name
			// (r.gonet 2016-05-19 18:13) - NX-100689 - Get the system name from the property 
			//manager rather than doing something ad-hoc.
			CString strComputerName = g_propManager.GetSystemName();
			// Based on username and computer name, get the EmrItemFont remote property

			_variant_t varFont = GetRemotePropertyImage("EmrItemFont", 0, GetCurrentUserName() + ("-" + strComputerName), false);

			if (varFont.vt == (VT_UI1 | VT_ARRAY)) {
				Nx::SafeArray<BYTE> sa(varFont.Detach().parray);

				if (sa.GetSize() == sizeof(LOGFONT)) {
					memcpy(&lfDefault, sa.begin(), sizeof(LOGFONT));
				}
			}
		} NxCatchAll(__FUNCTION__);

		if (!lfDefault.lfHeight) {
			lfDefault.lfHeight = 10 * 10;
		}

		lfDefault.lfQuality = 5; /*CLEARTYPE_QUALITY*/
	}

	return lfDefault;
}

////
/// Implementation

HFONT CreateEmrFont(LOGFONT& logFont)
{
	CDC screenDC;
	screenDC.Attach(::GetDC(NULL));

	// convert nPointSize to logical units based on pDC
	POINT pt;
	// 72 points/inch, 10 decipoints/point
	pt.y = ::MulDiv(::GetDeviceCaps(screenDC, LOGPIXELSY), logFont.lfHeight, 720);
	pt.x = 0;
	::DPtoLP(screenDC, &pt, 1);
	POINT ptOrg = { 0, 0 };
	::DPtoLP(screenDC, &ptOrg, 1);
	logFont.lfHeight = -abs(pt.y - ptOrg.y);

	HFONT hFont = ::CreateFontIndirect(&logFont);

	ASSERT(hFont);

	return hFont;
}

// (a.walling 2011-11-11 11:11) - PLID 46621 - Creates a new HFONT for the EmrFont wrapper
// (d.thompson 2015-10-14 13:24) - PLID 67352 - Misc type conversion issues in vs2015.
HFONT CreateEmrFont(LPCSTR szFaceName, BYTE charset, long height, long weight = FW_DONTCARE, BOOL underline = FALSE, BOOL italics = FALSE)
{
	LOGFONT logFont = {
		height * 10,
		0,
		0,
		0,
		weight,
		(BYTE)italics,
		(BYTE)underline,
		FALSE,
		charset,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		5 /*CLEARTYPE_QUALITY*/,
		FF_DONTCARE,
		NULL
	};

	if (strchr(szFaceName, ',')) {
		CString strExistingFont = FindExistingFont(szFaceName);

		ASSERT(!strExistingFont.IsEmpty());

		strncpy_s(logFont.lfFaceName, _countof(logFont.lfFaceName), strExistingFont, _TRUNCATE);
	} else {
		strncpy_s(logFont.lfFaceName, _countof(logFont.lfFaceName), szFaceName, _TRUNCATE);
	}

	return CreateEmrFont(logFont);
}

HFONT CreateEmrFont(long height, long weight = FW_DONTCARE, BOOL underline = FALSE, BOOL italics = FALSE)
{
	return CreateEmrFont(GetBaseLogFont().lfFaceName, DEFAULT_CHARSET, height, weight, underline, italics);
}

// (a.walling 2012-10-03 12:09) - PLID 53002 - List fonts may use the custom font in properties
HFONT CreateEmrListFont(long height, long weight = FW_DONTCARE, BOOL underline = FALSE, BOOL italics = FALSE)
{
	LOGFONT lf = GetBaseEmrListLogFont();

	double ratio = height / 10.0;

	lf.lfHeight = (long)(ratio * lf.lfHeight);

	if (weight != FW_DONTCARE) {
		lf.lfWeight = weight;
	}
	if (underline) {
		lf.lfUnderline = underline;
	}
	if (italics) {
		lf.lfItalic = italics;
	}

	return CreateEmrFont(lf);
}

EmrFont GetTopicHeaderFont()
{
	static HFONT hFont = CreateEmrFont("Corbel,Calibri,Tahoma,Arial", DEFAULT_CHARSET, 16);
	return hFont;
}

EmrFont GetTopicHeaderWebdingsFont()
{
	static HFONT hFont = CreateEmrFont("Webdings", SYMBOL_CHARSET, 16);
	return hFont;
}

EmrFont GetTopicHeaderSmallerFont()
{
	static HFONT hFont = CreateEmrFont("Verdana,Arial", DEFAULT_CHARSET, 12);
	return hFont;
}

EmrFont GetTitleFont()
{
	static HFONT hFont = CreateEmrFont("Arial", DEFAULT_CHARSET, 10, FW_BOLD);
	return hFont;
}

EmrFont GetRegularFont()
{	
	static HFONT hFont = CreateEmrFont(10);
	return hFont;
}

// (z.manning 2012-07-20 15:01) - PLID 51676
EmrFont GetSmallFont()
{	
	static HFONT hFont = CreateEmrFont(8);
	return hFont;
}

EmrFont GetBoldFont()
{
	static HFONT hFont = CreateEmrFont(10, FW_SEMIBOLD);
	return hFont;
}

EmrFont GetUnderlineFont()
{
	static HFONT hFont = CreateEmrFont(10, FW_DONTCARE, TRUE);
	return hFont;
}

EmrFont GetTinyUnderlineFont()
{
	static HFONT hFont = CreateEmrFont(8, FW_DONTCARE, TRUE);
	return hFont;
}

///

EmrFont GetRegularListFont()
{	
	static HFONT hFont = CreateEmrListFont(10);
	return hFont;
}

EmrFont GetBoldListFont()
{
	static HFONT hFont = CreateEmrListFont(10, FW_SEMIBOLD);
	return hFont;
}

EmrFont GetUnderlineListFont()
{
	static HFONT hFont = CreateEmrListFont(10, FW_DONTCARE, TRUE);
	return hFont;
}

}
