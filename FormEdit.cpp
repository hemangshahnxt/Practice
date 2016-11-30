// FormEdit.cpp : implementation file
//a simple edit box

#include "stdafx.h"
#include "FormEdit.h"
#include "FormFormat.h"
#include "FormLayer.h"
#include "FormQuery.h"
#include "GlobalFinancialUtils.h"
#include "NxStandard.h"
#include "InternationalUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

extern int MINI_FONT_SIZE;

/////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC(FormEdit, FormControl) 

FormEdit::FormEdit()
{
}

FormEdit::FormEdit(FormControl *control)
	:FormControl (control)
{
}

BOOL FormEdit::Create (int flags, RECT &rect, CWnd *parent)
{
	return CEdit::Create(flags, rect, parent, nID);
}

void FormEdit::PrintOut(CDC *pDC)
{	CString value;
	CFont* pFont, *pOldFont = NULL;
	UINT align;

	if (!(format & STATIC))
	{	GetWindowText(value);
		if (format & PHONE)
		{	value.Replace('(', ' ');
			value.Replace(')', ' ');
		}

		CRect thisSize;
		GetWindowRect(thisSize);

		CDC *pThisDC = GetDC();

		CRect rect = thisSize;
		// (j.jones 2005-02-08 15:56) - PLID 14641 - I freely admit that I have no idea
		// what the mathematical difference is between the amount of text that fits in the box
		// on the screen and the amount of text that fits in the same area when printing.
		// But if I compare the current width to 50% larger, it seems to cut off pretty accurately.
		pThisDC->DrawText(value,rect,DT_VCENTER|DT_LEFT|DT_SINGLELINE|DT_CALCRECT);
		while(rect.Width() > (thisSize.Width() * 1.5)) {
			value = value.Left(value.GetLength()-1);
			rect = thisSize;
			pThisDC->DrawText(value,rect,DT_VCENTER|DT_LEFT|DT_SINGLELINE|DT_CALCRECT);
		}

		/////////////////////////////////////////
		// Make a special exception for a number
		// of controls
		// (j.jones 2008-02-19 16:36) - PLID 29020 - added HCFA Box 32a to this list
		if ((id >= 397 && id <= 421) || id == 3549 || id == 5421 || id == 5427) {
			pFont = new CFont;
			//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
			CreateCompatiblePointFont(pFont, MINI_FONT_SIZE * 100, "Arial");
			pOldFont = (CFont *)pDC->SelectObject(pFont);
		}

		if (format & RIGHTALIGN) {
			align = pDC->SetTextAlign(TA_RIGHT);
			pDC->TextOut(
				(int)((double)(x+60) * PRINT_X_SCALE) + (int)PRINT_X_OFFSET, 
				(int)PRINT_Y_OFFSET - (int)((double)y * PRINT_Y_SCALE),
				value);
			pDC->SetTextAlign(align);
		}
		else {
			pDC->TextOut(
				(int)((double)x * PRINT_X_SCALE) + (int)PRINT_X_OFFSET, 
				(int)PRINT_Y_OFFSET - (int)((double)y * PRINT_Y_SCALE),
				value);
		}			

		if (pOldFont) {
			pDC->SelectObject(pOldFont);
			delete pFont;
		}

		// (a.walling 2007-08-07 17:01) - PLID 26996 - Need to release the device context we created
		ReleaseDC(pThisDC);
	}
}

bool FormEdit::Collide(int x1, int y1)
{
//	if (x1 >= x && x1 <= x + width && y1 >= y && y1 >= y + height)
	if (GetFocus() == this)
		return true;
	return false;
}

BOOL FormEdit::Refresh(FormLayer* layer)
{

		// (j.jones 2007-06-22 09:31) - PLID 25665 - when initially refreshing,
		// reset m_bIsEdited to FALSE
		m_bIsEdited = FALSE;

		RECT rect;
		COleVariant		tmpVar;
		CString tmpStr;
		int				flags;

		rect.left = x;
		rect.right = x + width;
		rect.top = y;
		rect.bottom = y + height;
		flags = WS_VISIBLE | WS_CHILD | ES_LEFT;
		if (format & READONLY)
			flags |= ES_READONLY;
		else flags |= WS_TABSTOP;

		source.Replace("[","");
		source.Replace("]","");

		if (format & EDIT) {
			////////////////////////////////////////////
			// This will make the bottom three HCFA box
			// fonts small
			if ((id >= 397 && id <= 421) || id == 3549)
				SetFont(layer->m_pMiniFont, false);
			else
				SetFont(layer->m_pFont, false);
		}
		else if (format & STATIC)
			SetFont(layer->m_pStaticFont, false);

#ifdef _DEBUG
		if (GetAsyncKeyState(VK_CONTROL)) {
			tmpStr.Format("%d-%d", form, id);
			SetWindowText(tmpStr);
			return TRUE;
		}
#endif

		//JMJ 1/27/2004 - for reasons as yet unexplained (likely due to the bizarre way the forms work),
		//you can get with alarming frequency and error here when restoring defaults. Problem is,
		//the state will be "Executing", not "Open". We need to wait for it to be "Open".
		//It's very quick so this loop would only go through a few times.
		if(layer->m_prsHistory->State != adStateClosed) {
			while(layer->m_prsHistory->State != adStateOpen) {
			}
		}

		if (layer->m_prsHistory->State != adStateClosed && !layer->m_prsHistory->eof) {
			try {
				tmpStr.Format("ControlID = %li", (long)id);
				layer->m_prsHistory->MoveFirst();
				while(!layer->m_prsHistory->eof) {
					tmpVar = layer->m_prsHistory->Fields->Item["ControlID"]->Value;
					if(tmpVar.lVal==id) {
						tmpVar = layer->m_prsHistory->Fields->Item["Value"]->Value;
						if (tmpVar.vt == VT_NULL)
							SetWindowText("");
						else
							SetWindowText(CString(tmpVar.bstrVal));

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
			NxCatchAll("Error reading historical data");
		}

		// (a.walling 2014-03-13 09:01) - PLID 61359 - places that called SetWindowText with source #ifdef _DEBUG and "" if not now just use ShowEmptyControlSource

		try
		{
			if (!(layer->m_rs->bof && layer->m_rs->eof)) {
				layer->m_rs->MoveLast();
				layer->m_rs->MoveFirst();
			}

			if (format & GROUP && (!(format & CHECK)))// if we are in a chart
			{	if (value < layer->m_rs->GetRecordCount()) {
					layer->m_rs->MoveFirst();
					layer->m_rs->Move(value);
				}
				else //no record
				{
					ShowEmptyControlSource(this, source);
					return FALSE;
				}
			}
			else if (format & STATIC) {
				SetWindowText(source);
				return TRUE;
			}
			else if (layer->m_rs->State != adStateClosed && !layer->m_rs->bof) { // (a.walling 2014-03-18 09:06) - PLID 61359 - Stop throwing _com_errors all the time
				layer->m_rs->MoveFirst();
			}

			if (!(format & STATIC) && layer->m_rs->State != adStateClosed && !layer->m_rs->eof && source != "")
			{
				tmpVar = layer->m_rs->Fields->Item[_variant_t(source)]->Value;
				switch (tmpVar.vt)
				{	case VT_BSTR:
						tmpStr = CString(tmpVar.bstrVal);
						break;
					case VT_CY:
						tmpStr = FormatCurrencyForInterface(VarCurrency(tmpVar),FALSE,FALSE);
						break;
					case VT_I2:
					case VT_UI1:
						tmpStr.Format("%i", VarByte(tmpVar));
						break;
					case VT_I4:
						tmpStr.Format("%li", tmpVar.lVal);
						break;
					case VT_R4:
						tmpStr.Format("%g", tmpVar.fltVal);
						break;
					case VT_R8:
						tmpStr.Format("%g", tmpVar.dblVal);
						break;
					default: 
						//this could be an error, but could be a blank box.  Just leaving it blank is the best solution EITHER way.
						ShowEmptyControlSource(this, source);
						tmpStr.Empty();
						break;
				}

				// (j.jones 2010-01-05 11:04) - PLID 36525 - FormatItemText was formerly used here,
				// which did not use the overloaded FormEdit::SetWindowText. FormatText will change
				// tmpStr and use the overloaded FormEdit::SetWindowText.
				if (format & PHONE) {
					if(format & WIDE) {
						CString strIn = tmpStr;
						FormatText(strIn, tmpStr, "(###)   ###-####");
					}
					else {
						CString strIn = tmpStr;
						FormatText(strIn, tmpStr, "(###)###-####");
					}
				}

//				if (!tmpStr.IsEmpty()) 
				{	
	#ifdef	SHOW_SOURCE_ONLY
					SetWindowText(source);
	#else
					SetWindowText (tmpStr);
	#endif
				}
			}
			else {
				ShowEmptyControlSource(this, source);
			}
		}
		//this could be an error, but could be a blank box.  Just leaving it blank is the best solution EITHER way.
		catch (_com_error)
		{	
			ShowEmptyControlSource(this, source);
		}
		return TRUE;
}

void FormEdit::ResetFont()
{
	SetFont(m_pOldFont);
	m_pOldFont = NULL;
}

void FormEdit::UnPunctuate()
{
	CString	punc_value, unpunc_value;
	TCHAR temp;
	int i=0,j=0;
	GetWindowText(punc_value);
	for(i=0;i<punc_value.GetLength();i++) {
		temp = punc_value.GetAt(i);
		// (j.jones 2006-10-16 14:29) - PLID 23044 - added support for ampersands, I found no documentation
		// suggesting they were illegal, and a client demanded it
		if((temp>='0' && temp<='9') || (temp>='A' && temp<='z') || temp == ' ' || temp == '&') {
			unpunc_value.Insert(j,temp);
			j++;
		}
		else if(punc_value.GetLength()>=8 && temp==',' && (i>=1 && i<=3) && punc_value.GetAt(i+4)=='.') {
			Sleep(0);
		}
		else {
			unpunc_value.Insert(j,' ');
			j++;
		}
	}

	SetWindowText(unpunc_value);

}

void FormEdit::Capitalize()
{
	CString	value;

	GetWindowText(value);
	value.MakeUpper();
	SetWindowText(value);
}

void FormEdit::Save(int iDocumentID)
{
	CString str, strValue;
	_variant_t var;

	if (!(format & STATIC)) {
		try {
			GetWindowText(strValue);

			str.Format(
				"SET NOCOUNT OFF\r\n" // (a.walling 2011-05-27 12:37) - PLID 43866 - Explicitly set NOCOUNT
				"UPDATE FormHistoryT SET [Value] = '%s' WHERE DocumentID = %d and ControlID = %d",
				_Q(strValue), iDocumentID, id);
			GetRemoteData()->Execute(_bstr_t(str),&var,adCmdText);
			if(var.lVal == 0) {
				str.Format("INSERT INTO FormHistoryT ( DocumentID, ControlID, [Value] ) VALUES ( %d, %d, '%s' )", iDocumentID, id, _Q(strValue));
				GetRemoteData()->Execute(_bstr_t(str),NULL,adCmdText);
			}
		}
		NxCatchAll("Error in saving historical document information");
	}
}

void FormEdit::SetWindowText(LPCTSTR lpszString)
{
	//TES 5/13/2008 - PLID 24675 - We need this so that when the text is manually set (by a Refresh() or whatever), we don't
	// flag ourselves as edited when we get the EN_CHANGED message.
	m_bSelfEditing = TRUE;
	CEdit::SetWindowText(lpszString);
	m_bSelfEditing = FALSE;
}