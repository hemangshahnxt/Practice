// FormControl.cpp: implementation of the FormControl class.
//	This is the base class for all form control classes.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FormControl.h"
#include "FormEdit.h"
#include "FormLine.h"
#include "FormDate.h"
#include "FormCheck.h"
#include "FormQuery.h"
#include "InternationalUtils.h"
#include "FormLayer.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// (a.walling 2014-03-13 09:01) - PLID 61359 - Show the source in debug mode, otherwise blank
bool FormControl::ShouldShowEmptyControlSource(CWnd* pWnd, const CString& sourceText)
{
#ifdef _DEBUG
	return true;

	// You can set one or more sourceText values to break on
	/*
	static const char* breakOnTheseSources[] = {
		"WhichCodes"
	};
	for (int i = 0; i < _countof(breakOnTheseSources); ++i) {
		if (0 == sourceText.CompareNoCase(breakOnTheseSources[i])) {
			AfxDebugBreak();
		}
	}
	*/

#else
	return false;
#endif
}

// (a.walling 2014-03-13 09:01) - PLID 61359 - places that called SetWindowText with source #ifdef _DEBUG and "" if not now just use ShowEmptyControlSource

IMPLEMENT_DYNAMIC(FormControl, CObject) 

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FormControl::FormControl()
{
	m_pOldFont = NULL;
	// (j.jones 2007-06-22 09:31) - PLID 25665 - track whether the control is edited
	m_bIsEdited = FALSE;

	//TES 4/30/2008 - PLID 26475 - Need to track whether we change our contents ourselves (meaning they haven't really changed).
	m_bSelfEditing = FALSE;
}

FormControl::FormControl(FormControl *control)
{
	format = control->format;
	x = control->x;
	y = control->y;
	width = control->width;
	height = control->height;
	id = control->id;
	value = control->value;
	source = control->source;
	nID = control->nID;
	form = control->form;
	color = control->color;
	m_pOldFont = NULL;

	// (j.jones 2007-06-22 09:31) - PLID 25665 - track whether the control is edited
	m_bIsEdited = control->m_bIsEdited;

	//TES 4/30/2008 - PLID 26475 - Need to track whether we change our contents ourselves (meaning they haven't really changed).
	m_bSelfEditing = FALSE;
}

FormControl::~FormControl()
{

}

FormControl *FormControl::CreateControl(FormLayer *layer, long nSetupGroupID /* = -1*/)
{
	FormEdit		*pEdit;
	FormDate		*pDate;
	FormCheck		*pCheck;
	CString			formsql,
					datasql,
					tmpStr;
	CWaitCursor		wait;
	RECT			rect;
//	int				temp,
	int				flags;//,
//					size;
//	static int		i = 187;	//can be anything, as long as no local resource id's match
	COleVariant		tmpVar;

	// (j.jones 2007-06-22 09:31) - PLID 25665 - track whether the control is edited
	m_bIsEdited = FALSE;

	// TEMP by chris
	if (format & EDIT) {
		pEdit = new FormEdit(this);
		rect.left = x;
		rect.right = x + width;
		rect.top = y;
		rect.bottom = y + height;
		flags = WS_VISIBLE | WS_CHILD | ES_LEFT;
		if (format & READONLY)
			flags |= ES_READONLY;
		else flags |= WS_TABSTOP;
		if (format & RIGHTALIGN)
			flags |= ES_RIGHT;
		pEdit->Create (flags, rect, layer->m_pWnd);
		m_pOldFont = pEdit->GetFont();
		pEdit->Refresh(layer);
		return pEdit;
	}
	else if (format & DATE)
	{	pDate = new FormDate(this);
		pDate->Create (layer->m_pWnd, pDate->nID);
		m_pOldFont = pDate->GetFont();
		pDate->Refresh(layer);
		return pDate;
	}


	if (format & GROUP && (!(format & CHECK)))// if we are in a chart
	{	
		if (!(layer->m_rs->bof && layer->m_rs->bof)) {
			layer->m_rs->MoveLast();
			layer->m_rs->MoveFirst();
		}

		/*if (value < layer->m_rs->GetRecordCount())
			layer->m_rs->PutAbsolutePosition(value);
		else 
		{*///	formRS.MoveNext();

			//return NULL;
			if (!layer->m_rs->eof) {
				layer->m_rs->MoveLast();
				layer->m_rs->MoveNext();
			}
		//}
	}
	else if (!layer->m_rs->eof) { layer->m_rs->MoveFirst(); }

	if (format & EDIT)
	{	pEdit = new FormEdit(this);
		rect.left = x;
		rect.right = x + width;
		rect.top = y;
		rect.bottom = y + height;
		flags = WS_VISIBLE | WS_CHILD | ES_LEFT;
		if (format & READONLY)
			flags |= ES_READONLY;
		else flags |= WS_TABSTOP;
		pEdit->Create (flags, rect, layer->m_pWnd);

		m_pOldFont = pEdit->GetFont();

		if ((id >= 397 && id <= 421) || id == 3549)
			pEdit->SetFont(layer->m_pMiniFont, false);
		else
			pEdit->SetFont(layer->m_pFont, false);

		try
		{	
			if(layer->m_rs->State != ADODB::adStateClosed && !layer->m_rs->eof && source != "") {

				tmpVar = layer->m_rs->Fields->Item[_variant_t(source)]->Value;
				switch (tmpVar.vt)
				{	case VT_BSTR:					
						pEdit->SetWindowText ((CString)(char *)tmpVar.bstrVal);
						break;
					case VT_CY:
						pEdit->SetWindowText(FormatCurrencyForInterface(VarCurrency(tmpVar)));
						break;
					case VT_I2:
					case VT_UI1:
						tmpStr.Format("%i", VarByte(tmpVar));
						pEdit->SetWindowText (tmpStr);
						break;
					case VT_I4:
						tmpStr.Format("%li", tmpVar.lVal);
						pEdit->SetWindowText (tmpStr);
						break;
					case VT_R4:
						tmpStr.Format("%g", tmpVar.fltVal);
						pEdit->SetWindowText (tmpStr);
						break;
					case VT_R8:
						tmpStr.Format("%g", tmpVar.dblVal);
						pEdit->SetWindowText (tmpStr);
						break;
					default: 
						//this could be an error, but could be a blank box.  Just leaving it blank is the best solution EITHER way.
						ShowEmptyControlSource(pEdit, source);
						break;
				}
			}
			else {
				ShowEmptyControlSource(pEdit, source);
			}
		}
		//this could be an error, but could be a blank box.  Just leaving it blank is the best solution EITHER way.
		catch (CException *e)
		{	
			ShowEmptyControlSource(pEdit, source);
			e->Delete();
		}
		return pEdit;
	}
	else if (format & STATIC)
	{	pEdit = new FormEdit(this);
		rect.left = x;
		rect.right = x + width;
		rect.top = y;
		rect.bottom = y + height;
		if (!pEdit->Create (WS_VISIBLE | WS_CHILD | ES_LEFT | ES_READONLY, rect, layer->m_pWnd))
		{	AfxMessageBox ("Error creating text box, you may continue, but please contact Nextech.");
			return NULL;
		}
		m_pOldFont = pEdit->GetFont();

		if (format & ITALIC)
			pEdit->SetFont(layer->m_pItalicFont, false);
		else pEdit->SetFont(layer->m_pStaticFont,false);

#ifdef _DEBUG
		if (GetAsyncKeyState(VK_CONTROL)) {
			CString tmpStr;
			tmpStr.Format("%d-%d", form, id);
			pEdit->SetWindowText(tmpStr);
			return pEdit;
		}
#endif
		pEdit->SetWindowText (source);

		return pEdit;
	}
	else if (format & LINE)
		return new FormLine(this);
	else if (format & DATE)
	{	pDate = new FormDate(this);
		pDate->Create (layer->m_pWnd, pDate->nID);
		m_pOldFont = pDate->GetFont();
		try
		{
			if(layer->m_rs->State != ADODB::adStateClosed && !layer->m_rs->eof && source != "") {
				COleVariant var = layer->m_rs->Fields->Item[_variant_t(source)]->Value;
				if (var.vt != VT_NULL) {
					pDate->SetDateValue(((COleDateTime)(var.date)));				
				}			
				else {
					ShowEmptyControlSource(pDate, source);
				}
			}
			else {
				ShowEmptyControlSource(pDate, source);
			}
		}
		catch (CException *e)//again, leave it blank, we don't know what it is
		{	e->Delete();
		}
		pDate->Birthdate();
		pDate->SetFont(layer->m_pFont,false);
		return pDate;
	}
	else if (format & CHECK)
	{	pCheck = new FormCheck(this);
		pCheck->Create(layer->m_pWnd, nSetupGroupID);
		pCheck->Refresh(layer);
/*		try
		{
			if (format & IGNORESOURCE) {
				pCheck->SetCheck(value);
			}
			else {
				COleVariant var = layer->m_rs.GetFieldValue (source);
				if (var.vt == VT_BSTR)
					var = (long)atoi(var.pcVal);
				pCheck->SetCheck(var.lVal == value);
			}
		}
		catch (CDaoException *e)
		{	e->Delete(); //again. leave it blank
		}*/
		return pCheck;
	}
	return NULL;
}

void FormControl::ResetFont()
{
}

void FormControl::SetEdited(BOOL bEdited)
{
	//TES 4/30/2008 - PLID 26475 - If we are changing our own text, we don't want to flag ourselves as "edited"
	if(!m_bSelfEditing) {
		m_bIsEdited = bEdited;
	}
}