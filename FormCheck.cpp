// FormCheck.cpp: implementation of the FormCheck class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FormCheck.h"
#include "FormFormat.h"
#include "NxStandard.h"
#include "FormQuery.h"
#include "FormLayer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - No g_ptrRemoteData

using namespace ADODB;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(FormCheck, FormControl) 

FormCheck::FormCheck(int SetX, int SetY, int SetWidth, int SetHeight, int SetFormat, CString SetSource)
{
	x = SetX;
	y = SetY;
	format = SetFormat;
	source = SetSource;
}

FormCheck::FormCheck (FormControl *control)
	:FormControl(control)
{
}

FormCheck::~FormCheck()
{

}
//ON_BN_CLICKED
BOOL FormCheck::PreTranslateMessage(MSG *pMsg)
{
//	if (pMsg->message == WM_PAINT) 
//	{	OnPaint();
//		return true;
//	}
	return false;
}

void FormCheck::OnPaint()
{
//	DRAWITEMSTRUCT DrawItemStruct;

//	DrawItemStruct.rcItem.bottom = y + height;
//	DrawItemStruct.rcItem.top = height;
//	DrawItemStruct.rcItem.left = x;
//	DrawItemStruct.rcItem.right = x + width;
	
//	DrawItemStruct.hDC = GetDC()->m_hDC;
//	DrawItem(&DrawItemStruct);
//	AfxMessageBox ("Paint");
}

void FormCheck::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC *dc;
	CPen pen(PS_SOLID, 1, (COLORREF)0);
	CBrush brush((COLORREF)0x00FFFFFF);

	dc = CDC::FromHandle(lpDrawItemStruct->hDC);
	dc->SelectObject(&pen);
	dc->SelectObject(&brush);
	dc->Rectangle(&lpDrawItemStruct->rcItem);
	if (check)
	{	dc->MoveTo (lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top);
		dc->LineTo (lpDrawItemStruct->rcItem.right - 1, lpDrawItemStruct->rcItem.bottom -1);
		dc->MoveTo (lpDrawItemStruct->rcItem.right - 1, lpDrawItemStruct->rcItem.top);
		dc->LineTo (lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.bottom -1);
	}

	ReleaseDC(dc);
}

BOOL FormCheck::Create(CWnd* pParentWnd, long nSetupGroupID)
{
	DWORD dwStyle = WS_TABSTOP | BS_FLAT | WS_VISIBLE | WS_CHILD;
	// (j.jones 2013-05-08 16:30) - PLID 54511 - added checkbox type, our existing CHECK is actually a radio button.
	// We would need CHECKBOX + CHECK to be a functional checkbox, so the actual format value will be 133120. (131072 + 2048)
	if(format & CHECKBOX) {
		dwStyle |= BS_AUTOCHECKBOX;
	}
	else {
		dwStyle |= BS_AUTORADIOBUTTON;
	}
	if (format & GROUP)
		dwStyle |= WS_GROUP;

	RECT rect;
	rect.top = y;
	rect.left = x;
	rect.bottom = y + height;
	rect.right = x + width;

	m_nSetupGroupID = nSetupGroupID;

#ifdef SHOW_SOURCE_ONLY
	return CButton::Create(source, dwStyle, rect, pParentWnd, nID);
#else
	return CButton::Create("", dwStyle, rect, pParentWnd, nID);
#endif	
}

void FormCheck::PrintOut(CDC *pDC)
{
#ifdef SHOW_SOURCE_ONLY
		pDC->TextOut(
			(int)((double)x * PRINT_X_SCALE + PRINT_X_OFFSET) - 5, 
			(int)PRINT_Y_OFFSET - (int)((double)y * PRINT_Y_SCALE),
			source);
#else
	if (GetCheck()) {

		CString strOut = "X";

		// (j.jones 2006-08-01 15:50) - PLID 21520 - if we are in HCFA Box 1a,
		// check to see if they want to print out a P instead of an X
		if(id >= 72 && id <= 78 && m_nSetupGroupID != -1) { //Box 1a InsType checkboxes are IDs 72 through 78
			if(IsRecordsetEmpty("SELECT ID FROM HCFASetupT WHERE ID = %li AND PrintPInBox1a = 0", m_nSetupGroupID)) {
				strOut = "P";
			}
		}

		pDC->TextOut(
			(int)((double)x * PRINT_X_SCALE + PRINT_X_OFFSET ) - 5, 
			(int)PRINT_Y_OFFSET - (int)((double)y * PRINT_Y_SCALE),
			strOut);
	}
#endif
}

void FormCheck::Save(int iDocumentID)
{
	CString str;

	try {
		long nAffected = 0;
		ExecuteSql(&nAffected, adCmdText, 
			"SET NOCOUNT OFF\r\n" // (a.walling 2011-05-27 12:37) - PLID 43866 - Explicitly set NOCOUNT
			"UPDATE FormHistoryT SET [Value] = '%d' WHERE DocumentID = %d and ControlID = %d",
			check, iDocumentID, id);
		if (nAffected == 0) {
			ExecuteSql("INSERT INTO FormHistoryT ( DocumentID, ControlID, [Value] ) VALUES ( %d, %d, '%d' )", iDocumentID, id, check);
		}
	}
	NxCatchAll("Error in saving historical document information");
}

BOOL FormCheck::Refresh(FormLayer* layer)
{
	// (j.jones 2007-06-22 09:31) - PLID 25665 - when initially refreshing,
	// reset m_bIsEdited to FALSE
	m_bIsEdited = FALSE;

	if (format & IGNORESOURCE) {
		SetCheck(value);
		check = value;
	}
	else {
		try {
			_variant_t var;
			CString tmpStr;

			if (layer->m_prsHistory->State != adStateClosed && !layer->m_prsHistory->eof) {
				tmpStr.Format("ControlID = %li", (long)id);
				layer->m_prsHistory->MoveFirst();
				while(!layer->m_prsHistory->eof) {
					var = layer->m_prsHistory->Fields->Item["ControlID"]->Value;
					if(var.lVal==id) {
						var = layer->m_prsHistory->Fields->Item["Value"]->Value;
						if (var.vt == VT_NULL)
							SetWindowText("");
						else
							SetCheck(atoi(CString(var.bstrVal)));

						// (j.jones 2007-06-22 09:31) - PLID 25665 - we are overriding
						// the control, so mark m_bIsEdited as TRUE
						// (j.jones 2007-10-26 10:47) - PLID 25665 - Only if it is non-static.
						if(!(format & STATIC)) {
							m_bIsEdited = TRUE;
						}
						
						layer->m_prsHistory->MoveFirst();

						return TRUE;
					}
					layer->m_prsHistory->MoveNext();
				}
				layer->m_prsHistory->MoveFirst();
			}


			if(layer->m_rs->State != adStateClosed && !layer->m_rs->eof && source != "") {
				layer->m_rs->MoveFirst();
				var = layer->m_rs->Fields->Item[_variant_t(source)]->Value;
				if (var.vt == VT_BSTR)
					var = (long)atoi(CString(var.bstrVal));
				else if(var.vt == VT_BOOL) {
					if(var.boolVal == -1) {
						var.Clear();
						//var.vt = VT_I4;
						var = (long)65535;
					}
					else {
						var.Clear();
						//var.vt = VT_I4;
						var = (long)0;
					}
				}
				else if(var.vt == VT_I2 || var.vt == VT_UI1)
					var = (long)VarByte(var);
				if(var.vt!=VT_EMPTY && var.vt != VT_NULL) {
					if(var.lVal==1 && id==170)
						var.lVal = 65535;
					SetCheck(var.lVal == value);
					check = var.lVal == value;
				}
			}
			else {
				// (j.jones 2006-04-14 11:30) - PLID 20160 - we manually control this box,
				// and it is allowed for both radio buttons to be unchecked
				if(id == 171 || id == 170)
					return TRUE;

				// (j.jones 2007-05-09 11:56) - PLID 25950 - This code always checked
				// boxes that we don't try to load, which is common on the rare forms
				// like MICR. Now it won't try to check the box if the source is empty.
				if(source != "") {
					var.Clear();
					var = (long)0;
					SetCheck(var.lVal == value);
				}
				else {
					SetCheck(0);
				}
			}
		}
		NxCatchAll("Error In FormCheck");
	}
	
	return TRUE;
}