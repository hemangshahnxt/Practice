
// FormDate.cpp: implementation of the FormDate class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FormDate.h"
#include "FormLayer.h"
#include "GlobalUtils.h"
#include "NxStandard.h"
#include "FormQuery.h"
#include "DateTimeUtils.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - No g_ptrRemoteData

using namespace ADODB;

extern int MINI_FONT_SIZE;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FormDate::FormDate()
{
}

FormDate::FormDate(FormControl *control)
	: FormEdit(control)
{
}

FormDate::~FormDate()
{

}

int FormDate::Birthdate()
{
	return 1;
	COleDateTime	dt;
	int				month, 
					day,
					thisyear;
	tagVARIANT		tmpVar;

	if (true)//format & BIRTHDATE)
	{	dt = (COleDateTime)GetDateValue();
		month = dt.GetMonth();
		day = dt.GetDay();
		thisyear = COleDateTime::GetCurrentTime().GetYear();
		while (dt.GetYear() > thisyear)
			dt.SetDate(dt.GetYear() - 100, month, day);
		tmpVar = (COleVariant)dt;
		SetDateValue(tmpVar);
	}
}

COleDateTime FormDate::GetDateValue()
{
	CString			str;
	COleDateTime	dt;

	GetWindowText(str);
	dt.ParseDateTime(str);
	return dt;
}


void FormDate::SetDateValue(COleDateTime dt)
{
	CString			str, strFormat;

	if (!dt.GetStatus()) {
		
		//initialize with the default
		strFormat = "%m/%d/%Y";
		
		if(format & YEARFIRST) {
			if(format & WIDE)
				strFormat = "%Y    %m     %d";
			else
				strFormat = "%Y/%m/%d";
		}
		else {
			if(format & WIDE)
				strFormat = "%m    %d     %Y";
			else
				strFormat = "%m/%d/%Y";
		}

		if(format & TWODIGIT)
			strFormat.Replace("Y","y");

		if(format & NOSLASH)
			strFormat.Replace("/","");

		//now format the date
		if ((format & TWODIGIT) && (dt.GetYear() < 1900 )) {
			// (a.walling 2009-03-03 12:44) - PLID 33310 - The %y formatter will fail if the year is not in the 20th century
			// so this includes the sentinel value 12/30/1899. Must be blank, then, if before 1900.
			str = "";
		} else {
			str = dt.Format(strFormat);
		}
	}
	// (a.walling 2009-03-03 12:39) - PLID 33310 josh says this should be blank 
	//else SetWindowText("111111");
	
	SetWindowText(str);
}

BOOL FormDate::Create (CWnd *parent, int id)
{
	RECT rect;

	rect.left = x;
	rect.top = y;
	rect.right = x + width;
	rect.bottom = y + height;
	return CEdit::Create (WS_TABSTOP | WS_CHILD | WS_VISIBLE, rect, parent, id);
}
/*
CString MakeFormatText(LPCTSTR fmtText) 
{
	CString Ans;//("");
	Ans.Empty();
	int len = strlen(fmtText);
	int i;
	for (i=0; i<len; i++) {
		if (fmtText[i] == 'n')
			Ans += ' ';
		else
			Ans += fmtText[i];
	}
	return Ans;
}
/*
int NextFmtPos(int FmtPos, LPCTSTR fmtText)
{
	for (unsigned int Ans=FmtPos+1; (fmtText[Ans] != 'n') && (fmtText[Ans] != '#') && (Ans < strlen(fmtText)); Ans++);
	return Ans;
}
*//*
void FormatText(CString & strInText, CString & strOutText, LPCTSTR fmtText)
{
	strOutText = MakeFormatText(fmtText);
	
	// Put numeric digits into appropriate places in format string
	int CurPos, FmtPos = -1;
	for (CurPos=0; CurPos<strInText.GetLength(); CurPos++)  {
		if (isdigit(strInText[CurPos])) {
			FmtPos = NextFmtPos(FmtPos, fmtText);
			if (FmtPos > (strOutText.GetLength()-1)) break;
			strOutText.SetAt(FmtPos, strInText[CurPos]);
		}
	}
}
/*
int NewSelPos(int OrgStart, CString &CurText, CString &FmtText)
{
	int i, Ans;
	CString tmpChar;

	if (OrgStart > CurText.GetLength()) OrgStart = CurText.GetLength();
	if (OrgStart < 0) OrgStart = CurText.GetLength();
	for (Ans=0, i=0; i<OrgStart; i++) {
		if (isdigit(CurText[i])) {
			tmpChar = FmtText.Mid(Ans, 1);
			while ((tmpChar != "#" && tmpChar != "n") && Ans < FmtText.GetLength()){
				Ans++;
				tmpChar = FmtText.Mid(Ans, 1);
			}
			Ans++;
		}
	}
	return Ans;
}
*/
int FormDate::Format()
{
	static bool IsRunning = false;
	CString		in, out;
	int			x1, x2;

	if (IsRunning) 
		return 0;
	IsRunning = true;

	GetSel(x1, x2);
	GetWindowText(in);
//	FormatText(in, out, "##/##/##nn");
	out = "12/12/12";
	if (out != in) 
	{	SetWindowText(out);
		SetSel(x1, x2);
	}
	IsRunning = false;
	return 1;
}

BOOL FormDate::PreTranslateMessage (MSG *pMsg)
{
//	if (pMsg->message == WM_KEYDOWN) 
//		if (HIWORD(pMsg->wParam) == EN_CHANGE)
//			Format();
	CString str;
#ifndef SHOW_SOURCE_ONLY
	GetWindowText(str);
	//TES 4/30/2008 - PLID 26475 - This may fire the EN_CHANGE event, but we don't want to flag ourselves as edited 
	// just because we formatted the text.  So, set m_bSelfEditing to TRUE, so we know that this isn't a user change.
	m_bSelfEditing = TRUE;
	if(str.Find("/",0)==4)
		if(format & WIDE)
			FormatItemText(this, str, "nnnn     nn    nn");//"##/##/##nn");
		else
			FormatItemText(this, str, "nnnn nn nn");//"##/##/##nn");
	else if(str.Find("/",0)==2)
		if(format & WIDE)
			FormatItemText(this, str, "nn    nn     nnnn");//"##/##/##nn");
		else
			FormatItemText(this, str, "nn nn nnnn");//"##/##/##nn");
		//TES 4/30/2008 - PLID 26475 - We're done now, any future changes are user changes.
	m_bSelfEditing = FALSE;
#endif
	return FormEdit::PreTranslateMessage(pMsg);
}

void FormDate::PrintOut(CDC *pDC)
{
	CFont* pFont, *pOldFont = NULL;
	CString value;

	/////////////////////////////////////////
	// Make a special exception for a number
	// of controls
	if ((id >= 397 && id <= 421) || id == 3549) {
		pFont = new CFont;
		//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
		CreateCompatiblePointFont(pFont, MINI_FONT_SIZE * 100, "Arial");
		pOldFont = (CFont *)pDC->SelectObject(pFont);			
	}

	GetWindowText(value);
	value.Replace("#", " ");
	value.Replace("/", " ");
	pDC->TextOut(
			(int)((double)x * PRINT_X_SCALE + PRINT_X_OFFSET), 
			(int)PRINT_Y_OFFSET - (int)((double)y * PRINT_Y_SCALE),
			value);

	if (pOldFont) {
		pDC->SelectObject(pOldFont);
		delete pFont;
	}
}

BOOL FormDate::Refresh(FormLayer* layer)
{
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

	// (j.jones 2007-06-22 09:31) - PLID 25665 - when initially refreshing,
	// reset m_bIsEdited to FALSE
	m_bIsEdited = FALSE;

	////////////////////////////////////////////
	// This will make the bottom three HCFA box
	// fonts small
	if ((id >= 397 && id <= 421) || id == 3549)
		SetFont(layer->m_pMiniFont, false);
	else
		SetFont(layer->m_pFont, false);

#ifdef _DEBUG
		if (GetAsyncKeyState(VK_CONTROL)) {
			tmpStr.Format("%d-%d", form, id);
			SetWindowText(tmpStr);
			return TRUE;
		}
#endif

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
						else {
							COleDateTime dt;
							dt.ParseDateTime(CString(tmpVar.bstrVal));
							SetDateValue(dt);
						}

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
		layer->m_rs->MoveFirst();
		// Commented out by Chris
		//if (format & GROUP && (!(format & CHECK)))// if we are in a chart
		{	if (value < layer->m_rs->GetRecordCount()) {
				layer->m_rs->Move(value);
			}
			else 
			{
				ShowEmptyControlSource(this, source);
				return FALSE;
			}
		}
		//else layer->m_rs.MoveFirst();

		if(layer->m_rs->State != adStateClosed && !layer->m_rs->eof && source != "") {
		
			_variant_t var = layer->m_rs->Fields->Item[_variant_t(source)]->Value;
			if (var.vt == VT_NULL) {
				ShowEmptyControlSource(this, source);
			}
			else {
#ifdef SHOW_SOURCE_ONLY
				SetWindowText(source);
#else
				SetDateValue(((COleDateTime)(var.date)));
#endif
			}
		}
		else {
			ShowEmptyControlSource(this, source);
		}
	}
	catch (_com_error)//again, leave it blank, we don't know what it is
	{
	}

#ifndef SHOW_SOURCE_ONLY
	Birthdate();
#endif
	//SetFont(layer->m_pFont,false);
	return TRUE;
}


void FormDate::Save(int iDocumentID)
{
	CString str, strValue;

	try {
		if(GetDateValue().GetStatus() != COleDateTime::valid) {
			strValue = "NULL";
		}
		else {
			strValue = "'" + FormatDateTimeForSql(GetDateValue(), dtoDate) + "'";
		}
		//Instead of saving the text, which could be anything, we'll get the date value and format it.
		long nAffected = 0;
		ExecuteSql(&nAffected, adCmdText, 
			"SET NOCOUNT OFF\r\n" // (a.walling 2011-05-27 12:37) - PLID 43866 - Explicitly set NOCOUNT
			"UPDATE FormHistoryT SET [Value] = %s WHERE DocumentID = %d and ControlID = %d",
			strValue, iDocumentID, id);
		if (nAffected == 0) {
			ExecuteSql("INSERT INTO FormHistoryT ( DocumentID, ControlID, [Value] ) VALUES ( %d, %d, %s )", iDocumentID, id, strValue);
		}
	}
	NxCatchAll("Error in saving historical document information");
}