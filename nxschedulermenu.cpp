#include "stdafx.h"
#include "nxschedulermenu.h"
#include "appointmentsdlg.h" // For pending/in/out/noshow popup menu options
#include "schedulerrc.h"

CNxSchedulerMenu::CNxSchedulerMenu()
{
}

CNxSchedulerMenu::~CNxSchedulerMenu()
{
	Detach();
	ASSERT(m_hMenu == NULL);    // defaul CMenu::~CMenu will destroy
}

void CNxSchedulerMenu::AppendColorMenuItem(UINT nID, COLORREF color)
{
	VERIFY(AppendMenu(MF_ENABLED | MF_BYPOSITION | MF_OWNERDRAW, nID, (LPCTSTR)color));
}

BOOL CNxSchedulerMenu::CreatePopupMenu(EPopupStyle style)
{
	CMenu::CreatePopupMenu();

	switch (style) {
	case eReservation:
		AppendMenu(MF_ENABLED, ID_RES_EDIT, "&Edit");
		AppendMenu(MF_ENABLED | MF_SEPARATOR, 1, (LPCTSTR)NULL);
		if (GetCurrentUserPermissions(bioAppointment) & (SPT_______1____ANDPASS))
			AppendMenu(MF_ENABLED, ID_RES_CUT, "Cu&t");
		AppendMenu(MF_ENABLED, ID_RES_COPY, "&Copy");
		AppendMenu(MF_ENABLED, ID_RES_PASTE, "&Paste");
		AppendMenu(MF_ENABLED | MF_SEPARATOR, 5, (LPCTSTR)NULL);
		if (GetCurrentUserPermissions(bioAppointment) & (SPT________2___ANDPASS))
			AppendMenu(MF_ENABLED, ID_RES_DELETE, "C&ancel Appt");
			AppendMenu(MF_GRAYED, ID_RESTORE_APPT, "Restore Appt"); // (j.luckoski 2012-05-02 17:06) - PLID 11597 - Context menu to restore cancelled appt
		AppendMenu(MF_ENABLED, ID_GOTOPATIENT, "Go To Patient");
		//DRT 6/13/2008 - PLID 9679 - This is no longer auto-generated always, it will be
		//	inserted when it is needed, sometimes as a single option, sometimes as a 
		//	popup menu.
		//AppendMenu(MF_ENABLED, ID_PRINTSUPERBILL, "Print &Superbill");
		break;

	case eTemplateBlock:
		// (c.haag 2006-12-05 10:38) - PLID 23666 - Have different options for blocks
		AppendMenu(MF_ENABLED, ID_RES_EDIT, "Create &Appointment");
		AppendMenu(MF_ENABLED, ID_RES_DELETEBLOCK, "&Delete Precision Template");
		AppendMenu(MF_ENABLED | MF_SEPARATOR, 1, (LPCTSTR)NULL);
		// (z.manning, 05/03/2007) - PLID 25896 - Added the option to paste onto precision templates.
		AppendMenu(MF_ENABLED, ID_RES_PASTE, "&Paste");
		SetDefaultItem(ID_RES_EDIT);
		break;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////

#define COLOR_BOX_WIDTH     20
#define COLOR_BOX_HEIGHT    20


void CNxSchedulerMenu::MeasureItem(LPMEASUREITEMSTRUCT lpMIS)
{
	// all items are of fixed size
	lpMIS->itemWidth = strlen((char*)lpMIS->itemData) * 5;
}

void CNxSchedulerMenu::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	static struct _popup_data {
		CString strName;
		COLORREF clr;
	} *popup_data;

	CDC* pDC = CDC::FromHandle(lpDIS->hDC);	
	popup_data = (_popup_data*)lpDIS->itemData;

//	if (lpDIS->itemAction & ODA_DRAWENTIRE)
//	{
		RECT rc = lpDIS->rcItem;
		rc.right = rc.left + 20;

		// Paint the color item in the color requested
		CBrush br(popup_data->clr);
		CBrush bl(COLORREF(0));

		rc.top++; rc.bottom++;
		pDC->FillRect(&rc, &br);
		pDC->FrameRect(&rc, &bl);

		pDC->TextOut(rc.left + 25, rc.top + 2, (LPCTSTR)popup_data->strName, strlen((LPCTSTR)popup_data->strName));
/*	}

	if ((lpDIS->itemState & ODS_SELECTED) &&
		(lpDIS->itemAction & (ODA_SELECT | ODA_DRAWENTIRE)))
	{
		// item has been selected - hilite frame
		COLORREF crHilite = RGB(255-GetRValue(cr),
						255-GetGValue(cr), 255-GetBValue(cr));
		CBrush br(crHilite);
		pDC->FrameRect(&lpDIS->rcItem, &br);
	}

	if (!(lpDIS->itemState & ODS_SELECTED) &&
		(lpDIS->itemAction & ODA_SELECT))
	{
		// Item has been de-selected -- remove frame
		CBrush br(cr);
		pDC->FrameRect(&lpDIS->rcItem, &br);
	}*/
}