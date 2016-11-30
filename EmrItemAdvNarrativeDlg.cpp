// EmrItemAdvNarrativeDlg.cpp : implementation file
//
// (c.haag 2007-03-29 17:07) - PLID 25423 - References to CEMNDetail::m_arMergeFields
// now use arrow symbols
// 

#include "stdafx.h"
#include <NxPracticeSharedLib/RichEditUtils.h>
#include "EmrItemAdvNarrativeDlg.h"
#include "EmrItemAdvListDlg.h"
#include "EmrItemAdvSliderDlg.h"
#include "EmrItemAdvTableDlg.h"
#include "EMN.h"
#include "EmrTextMacroDlg.h"
#include "EMRTopic.h"
#include "EMNDetail.h"

// (a.walling 2011-11-11 11:11) - PLID 46632 - WindowlessUtils - Various functions replaced with windowless-safe versions.

extern CPracticeApp theApp;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEmrItemAdvNarrativeDlg dialog


CEmrItemAdvNarrativeDlg::CEmrItemAdvNarrativeDlg(class CEMNDetail *pDetail)
	: CEmrItemAdvDlg(pDetail)
{
	//{{AFX_DATA_INIT(CEmrItemAdvNarrativeDlg)
		 m_RichEditCtrl = NULL;
	//}}AFX_DATA_INIT
	// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated
	//m_bAddingMergeFields = FALSE;
	// (c.haag 2007-08-15 16:34) - PLID 27084 - Garbage collection
	// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated
	//m_pstrRequestFormatRichTextCtrlResult = NULL;

	// (a.walling 2009-11-25 12:21) - PLID 36365 - We don't need a CString, just use a LPTSTR
	m_szRequestFormatBuffer = NULL;

	// (c.haag 2012-04-02) - PLID 49346
	m_bHasReflectedInitialState = FALSE;
}


CEmrItemAdvNarrativeDlg::~CEmrItemAdvNarrativeDlg()
{
	// (c.haag 2007-08-15 16:34) - PLID 27084 - Garbage collection
	// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated
	/*if (m_pstrRequestFormatRichTextCtrlResult) {
		delete m_pstrRequestFormatRichTextCtrlResult;
	}*/

	// (a.walling 2009-11-25 12:21) - PLID 36365 - We don't need a CString, just use a LPTSTR
	if (m_szRequestFormatBuffer) {
		delete[] m_szRequestFormatBuffer;
		m_szRequestFormatBuffer = NULL;
	}
}

BEGIN_MESSAGE_MAP(CEmrItemAdvNarrativeDlg, CEmrItemAdvDlg)
	//{{AFX_MSG_MAP(CEmrItemAdvNarrativeDlg)
	ON_WM_SHOWWINDOW()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// (z.manning, 04/14/2008) - PLID 29632 - Use the defined IDC rather than a hardcoded value
BEGIN_EVENTSINK_MAP(CEmrItemAdvNarrativeDlg, CEmrItemAdvDlg)
	ON_EVENT(CEmrItemAdvNarrativeDlg, RICH_TEXT_IDC, 1 /* TextChanged */, OnTextChangedRichTextCtrl, VTS_NONE)
	ON_EVENT(CEmrItemAdvNarrativeDlg, RICH_TEXT_IDC, 2 /* Link */, OnLinkRichTextCtrl, VTS_BSTR)
	ON_EVENT(CEmrItemAdvNarrativeDlg, RICH_TEXT_IDC, 4 /* RequestFormat */, OnRequestFormatRichTextCtrl, VTS_BSTR VTS_BSTR VTS_PBSTR)
	ON_EVENT(CEmrItemAdvNarrativeDlg, RICH_TEXT_IDC, 5 /* RightClickField */, OnRightClickFieldRichTextCtrl, VTS_BSTR VTS_BSTR VTS_I4 VTS_I4 VTS_I4)
	//ON_EVENT(CEmrItemAdvNarrativeDlg, RICH_TEXT_IDC, 6 /* RequestField */, OnRequestFieldRichTextCtrl, VTS_BSTR)
	//ON_EVENT(CEmrItemAdvNarrativeDlg, RICH_TEXT_IDC, 7 /* RequestAllFields */, OnRequestAllFieldsRichTextCtrl, VTS_NONE)
	//ON_EVENT(CEmrItemAdvNarrativeDlg, RICH_TEXT_IDC, 8 /* ResolvePendingMergeFieldValue */, OnResolvePendingMergeFieldValue, VTS_PDISPATCH)
	ON_EVENT(CEmrItemAdvNarrativeDlg, RICH_TEXT_IDC, 9 /* RequestLWMergeFieldData */, OnRequestLWMergeFieldData, VTS_NONE)
	ON_EVENT(CEmrItemAdvNarrativeDlg, RICH_TEXT_IDC, 10 /* RequestAllAvailableFields */, OnRequestAllAvailableFields, VTS_BOOL VTS_BOOL VTS_BOOL VTS_PVARIANT)
	ON_EVENT(CEmrItemAdvNarrativeDlg, RICH_TEXT_IDC, 13 /* RequestAvailableFieldsVersion */, OnRequestAvailableFieldsVersion, VTS_PI2)
	ON_EVENT(CEmrItemAdvNarrativeDlg, HTML_TEXT_IDC, 250, OnBeforeNavigate2HtmlEdit, VTS_DISPATCH VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CEmrItemAdvNarrativeDlg, HTML_TEXT_IDC, 252, OnNavigateComplete2HtmlEdit, VTS_DISPATCH VTS_PVARIANT)
	ON_EVENT(CEmrItemAdvNarrativeDlg, RICH_TEXT_IDC, 14 /* CheckValidIDs */, OnCheckValidIDsRichTextCtrl, VTS_I4 VTS_PBOOL VTS_I4 VTS_PBOOL)
	ON_EVENT(CEmrItemAdvNarrativeDlg, RICH_TEXT_IDC, 15 /* RightClickHtmlField */, OnRightClickHtmlFieldRichTextCtrl, VTS_BSTR VTS_BSTR VTS_I4 VTS_I4 VTS_BSTR)
	ON_EVENT(CEmrItemAdvNarrativeDlg, RICH_TEXT_IDC, 16 /* RightClickHtml */, OnRightClickHtmlRichTextCtrl, VTS_I4 VTS_I4 VTS_BOOL VTS_BOOL)
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEmrItemAdvNarrativeDlg message handlers

int CEmrItemAdvNarrativeDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	try {
		if (CEmrItemAdvDlg::OnCreate(lpCreateStruct) == -1)
			return -1;

		//TES 3/15/2010 - PLID 37757 - Check our detail's read only status
		DWORD dwDisabled = m_pDetail->GetReadOnly() ? WS_DISABLED : 0;
		
		// Add the label
		{
			// (b.cardillo 2012-05-02 20:28) - PLID 49255 - Use the detail label text with special modifiers for onscreen presentation
			CString strLabel = GetLabelText(TRUE);
			if (!strLabel.IsEmpty()) {
				strLabel.Replace("&", "&&");
				// (a.walling 2011-11-11 11:11) - PLID 46627 - Just create windows with an empty rect initially rather than a 1x1 rect
				m_wndLabel.CreateControl(strLabel, WS_VISIBLE | WS_GROUP | dwDisabled, CRect(0, 0, 0, 0), this, 0xffff);
				//m_wndLabel.ModifyStyleEx(0, WS_EX_TRANSPARENT); // (a.walling 2011-05-11 14:55) - PLID 43661 - Labels need WS_EX_TRANSPARENT exstyle
				// (a.walling 2011-11-11 11:11) - PLID 46621 - Unify font handing for emr items
				m_wndLabel->NativeFont = EmrFonts::GetTitleFont();
			}
		}

		// (c.haag 2012-04-02) - PLID 49346 - If the detail is not a classic NexTech RTF-formatted narrative, then we need to use a browser control
		//TES 5/14/2012 - PLID 49346 - This is all encapsulated inside the control now, so we don't need to worry about whether it's HTML or RTF.

		//TES 1/29/2008 - PLID 28673 - Use a #defined IDC, not a numeric literal.
		if (!m_wndRichEdit.CreateControl(_T("RICHTEXTEDITOR.RichTextEditorCtrl.1"), "EmrItemAdvNarrativeDlg", WS_CHILD|WS_VISIBLE, CRect(0,0,0,0), this, RICH_TEXT_IDC)) {
			DWORD dwErr = GetLastError();
			return -1;
		}

		if (m_wndRichEdit.GetSafeHwnd()) {
			//TES 7/11/2012 - PLID 49346 - Set WS_EX_CONTROLPARENT, otherwise MFC can get in an infinite loop
			m_wndRichEdit.ModifyStyleEx(WS_EX_NOPARENTNOTIFY, WS_EX_CONTROLPARENT);
		}

		m_RichEditCtrl = m_wndRichEdit.GetControlUnknown();

		// (a.walling 2009-11-17 13:58) - PLID 36365
		m_RichEditCtrl->UsePushFieldUpdates = VARIANT_TRUE;

		m_RichEditCtrl->DefaultFlags = "s";

		//Fill in any fields that have may have been supplied to us before our window was created.
		// (c.haag 2007-04-03 17:54) - PLID 25488 - We now use methods rather than direct accesses
		// to the merge fields in the detail object
		// (c.haag 2007-05-10 14:58) - PLID 25958 - The methods have changed such that we now use
		// a connectionless recordset object to do the work
		// (c.haag 2007-05-14 10:00) - PLID 25970 - Now that we share the detail's merge field list
		// with the control, there's no need to manually fill in values anymore

		// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated
		//m_RichEditCtrl->MergeFieldSet = m_pDetail->GetNarrativeFieldRecordset();

		/*
		m_RichEditCtrl->BeginAddMergeFields();
		ADODB::_RecordsetPtr prsNarrativeFields = m_pDetail->GetNarrativeFieldRecordset();
		if (prsNarrativeFields->GetRecordCount() > 0) {
			ADODB::FieldsPtr f = prsNarrativeFields->Fields;
			prsNarrativeFields->MoveFirst();
			while (!prsNarrativeFields->eof) {
				CString strProperties = "Linkable;";
				if(AdoFldBool(f, "Linkable")) strProperties += "True;"; else strProperties += "False;";
				strProperties += "IsSubField;";
				if(AdoFldBool(f, "IsSubField")) strProperties += "True;"; else strProperties += "False;";
				strProperties += "CaseSensitive;";
				if(AdoFldBool(f, "CaseSensitive")) strProperties += "True;"; else strProperties += "False;";
				if(AdoFldBool(f, "ValueIsValid")) {
					m_RichEditCtrl->AddFilledMergeFieldWithProperties(_bstr_t(AdoFldString(f, "Field")), _bstr_t(GetNarrativeValueField(prsNarrativeFields)), _bstr_t(strProperties));
				}
				else {
					m_RichEditCtrl->AddAvailableMergeFieldWithProperties(_bstr_t(AdoFldString(f, "Field")), _bstr_t(strProperties));
				}
				prsNarrativeFields->MoveNext();
			}
		}

		m_RichEditCtrl->EndAddMergeFields();
		*/

		//TES 3/15/2010 - PLID 37757 - Check our detail's read only status
		m_RichEditCtrl->ReadOnly = m_pDetail->GetReadOnly();
		//DRT 4/29/2008 - PLID 29771 - Set the background color to our dialog background color.
		m_RichEditCtrl->BackColor = GetBackgroundColor();
		m_wndRichEdit.ShowWindow(SW_SHOW);
		// (a.walling 2012-04-18 15:09) - PLID 50430 - Don't set focus on create
		//m_wndRichEdit.SetFocus();

		return 0;
	} NxCatchAll(__FUNCTION__);

	return -1;
}

// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated
/*
void CEmrItemAdvNarrativeDlg::SetNarrativeFieldRecordset(ADODB::_RecordsetPtr& prs)
{
	// (c.haag 2007-05-14 10:04) - PLID 25970 - If the detail's merge
	// field recordset object changed (not the content, but the object itself),
	// then we need to call this function to make sure that the form control
	// is up to date
	if (NULL != m_RichEditCtrl) {
		//m_RichEditCtrl->MergeFieldSet = prs;
	}
}
*/

BOOL CEmrItemAdvNarrativeDlg::RepositionControls(IN OUT CSize &szArea, BOOL bCalcOnly)
{
	CEmrItemAdvDlg::RepositionControls(szArea, bCalcOnly);
	// The caller is giving us a full window area so we have to adjust off the 
	// border to get what will be our client area.
	CSize szBorder;
	CSize szMergeBtn;
	CalcWindowBorderSize(this, &szBorder);
	// Adjust off the border
	szArea.cx -= szBorder.cx;
	szArea.cy -= szBorder.cy;
	
	const long cnMinEditWidth = 80;
	const long cnMinEditHeight = 25;
	long cnMergeBtnMargin = 5;

	// Make sure the merge status icon button reflects the state of the data,
	// because that will have a direct influence on our size calculations.
	UpdateStatusButtonAppearances();

	CClientDC dc(this);

	long nTopMargin = 3;
	long nIdealWidth = 668;
	if (IsControlValid(&m_wndLabel)) {
		CSize sz(LONG_MAX, LONG_MAX);
		CalcControlIdealDimensions(&dc, &m_wndLabel, sz);

		//first see if the label is too wide to be displayed
		if(sz.cx > szArea.cx) {

			//if so, find out how many lines we must create
			int nHeight = 1;
			while (nHeight < 10 && sz.cx / nHeight > szArea.cx) {
				nHeight++;
			}
			
			//now increase the height of the item to match
			if((sz.cx / (nHeight - 1)) > szArea.cx) {
				sz.cx = szArea.cx;
				sz.cy *= nHeight;
			}
		}

		nTopMargin += sz.cy;
		if (nIdealWidth < sz.cx) {
			nIdealWidth = sz.cx;
		}
		if (!bCalcOnly) {
			CRect rPrior, rLabel;
			GetControlChildRect(this, &m_wndLabel, &rPrior);

			m_wndLabel.MoveWindow(6, 2, sz.cx, sz.cy, FALSE);

			GetControlChildRect(this, &m_wndLabel, &rLabel);

			rPrior.InflateRect(2,2,2,2);
			rLabel.InflateRect(2,2,2,2);
			InvalidateRect(rPrior, TRUE);
			InvalidateRect(rLabel, TRUE);
		}
	}
	// (c.haag 2006-06-30 17:00) - PLID 19977 - We now calculate
	// merge button dimensions if either the merge or problem
	// button is visible
	if (IsMergeButtonVisible() || IsProblemButtonVisible()) {
		CSize sz(LONG_MAX, LONG_MAX);
		CalcControlIdealDimensions(&dc, m_pBtnMergeStatus, szMergeBtn);
	}
	else {
		szMergeBtn = CSize(0,0);
		cnMergeBtnMargin = 0;
	}

	long nIdealHeight = 325;

	if (bCalcOnly) {
		if (szArea.cx > nIdealWidth) {
			szArea.cx = nIdealWidth;
		}
		if (szArea.cy > nIdealHeight) {
			szArea.cy = nIdealHeight;
		}
	}
	
	// Make sure we're not smaller than the minimum
	long nMinWidth = 20 + cnMinEditWidth;
	long nMinHeight = nTopMargin + cnMinEditHeight + cnMergeBtnMargin + szMergeBtn.cy + 10;
	if (szArea.cx < nMinWidth) {
		szArea.cx = nMinWidth;
	}
	if (szArea.cy < nMinHeight) {
		szArea.cy = nMinHeight;
	}

	if (!bCalcOnly && m_wndRichEdit.m_hWnd) {
		m_wndRichEdit.MoveWindow(10, nTopMargin, szArea.cx - 20, szArea.cy - nTopMargin - szMergeBtn.cy - cnMergeBtnMargin - 10);
	}
	// (c.haag 2012-04-02) - PLID 49346 - Move the HTML editor window
	if (!bCalcOnly && IsWindow(m_wndHtmlEdit.GetSafeHwnd()))
	{
		m_wndHtmlEdit.MoveWindow(10, nTopMargin, szArea.cx - 20, szArea.cy - nTopMargin - szMergeBtn.cy - cnMergeBtnMargin - 10);
	}

	// (c.haag 2006-07-03 09:29) - PLID 19944 - We now use the variable x to calculate
	// the correct position of each iconic button
	int x = 10;
	if (!bCalcOnly && IsMergeButtonVisible()) {
		m_pBtnMergeStatus->MoveWindow(x, szArea.cy - szMergeBtn.cy - 10, szMergeBtn.cx, szMergeBtn.cy);
		x += szMergeBtn.cx + 3;
	}
	// (c.haag 2006-06-30 16:48) - PLID 19944 - Move the problem status button, too
	if (!bCalcOnly && IsProblemButtonVisible()) {
		m_pBtnProblemStatus->MoveWindow(x, szArea.cy - szMergeBtn.cy - 10, szMergeBtn.cx, szMergeBtn.cy);
	}

	BOOL bAns;
	if (szArea.cx >= nIdealWidth && szArea.cy >= nIdealHeight) {
		// Our ideal fits within the given area
		bAns = TRUE;
	} else {
		// Our ideal was too big for the given area
		bAns = FALSE;
	}

	// Return the new area, but adjust back on the border size since the caller wants window area
	szArea.cx += szBorder.cx;
	szArea.cy += szBorder.cy;

	return bAns;
}

void CEmrItemAdvNarrativeDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	try {
		CEmrItemAdvDlg::OnShowWindow(bShow, nStatus);
		
		// (a.walling 2012-05-16 15:25) - PLID 50430 - Don't set focus when showing the window!!
		//if (bShow) {
		//	// (z.manning, 04/14/2008) - PLID 29632 - Use the defined IDC rather than a hardcoded value
		//	// (c.haag 2012-04-02) - PLID 49346 - Test for valid controls
		//	if (IsWindow(m_wndRichEdit.GetSafeHwnd())) {
		//		GetDlgItem(RICH_TEXT_IDC)->SetFocus();
		//	} else if (IsWindow(m_wndHtmlEdit.GetSafeHwnd())) {
		//		GetDlgItem(HTML_TEXT_IDC)->SetFocus();
		//	}
		//}
	} NxCatchAll("Error in CEmrItemAdvNarrativeDlg::OnShowWindow");
}

void CEmrItemAdvNarrativeDlg::ReflectCurrentContent()
{
	CEmrItemAdvDlg::ReflectCurrentContent();

	if (NULL != m_RichEditCtrl) {
		// (c.haag 2007-05-14 11:37) - PLID 25970 - Now that we are the sole maintainer of the
		// merge field list, this function is depreciated in the context of adding fields to the
		// form control. However, we need to make sure the visible text is up to date.
		//m_RichEditCtrl->ParseMergeFields();
		// (a.walling 2009-11-19 15:08) - PLID 36365 - Push updates to the narrative
		m_pDetail->UpdateNarrativeFields();

		/*m_RichEditCtrl->BeginAddMergeFields();
		// (c.haag 2007-04-03 17:54) - PLID 25488 - We now use methods rather than direct accesses
		// to the merge fields in the detail object
		// (c.haag 2007-05-10 14:58) - PLID 25958 - The methods have changed such that we now use
		// a connectionless recordset object to do the work
		ADODB::_RecordsetPtr prsNarrativeFields = m_pDetail->GetNarrativeFieldRecordset();
		if (prsNarrativeFields->GetRecordCount() > 0) {
			ADODB::FieldsPtr f = prsNarrativeFields->Fields;
			prsNarrativeFields->MoveFirst();
			while (!prsNarrativeFields->eof) {
				CString strProperties = "Linkable;";
				if(AdoFldBool(f, "Linkable")) strProperties += "True;"; else strProperties += "False;";
				strProperties += "IsSubField;";
				if(AdoFldBool(f, "IsSubField")) strProperties += "True;"; else strProperties += "False;";
				strProperties += "IsCaseSensitive;";
				if(AdoFldBool(f, "CaseSensitive")) strProperties += "True;"; else strProperties += "False;";
				
				if(AdoFldBool(f, "ValueIsValid")) {
					m_RichEditCtrl->AddFilledMergeFieldWithProperties(_bstr_t(AdoFldString(f, "Field")), _bstr_t(GetNarrativeValueField(prsNarrativeFields)), _bstr_t(strProperties));
				}
				else {
					m_RichEditCtrl->AddAvailableMergeFieldWithProperties(_bstr_t(AdoFldString(f, "Field")), _bstr_t(strProperties));
				}
				prsNarrativeFields->MoveNext();
			}
		}

		m_RichEditCtrl->EndAddMergeFields();*/
	}
}

void CEmrItemAdvNarrativeDlg::ReflectCurrentState()
{
	CEmrItemAdvDlg::ReflectCurrentState();

	// (c.haag 2012-04-02) - PLID 49346 - Only do rich edit control reflection if we're a NexTech RTF-formatted narrative
	//TES 5/14/2012 - PLID 49346 - This is all encapsulated inside the control now, so we don't need to worry about whether it's HTML or RTF.
	if(m_RichEditCtrl) 
	{
		CString strPriorNarrative = (LPCTSTR)m_RichEditCtrl->RichText;
		CString strNewNarrative = AsString(m_pDetail->GetState());
		if (strNewNarrative != strPriorNarrative) {
			//TES 11/3/2005 - PLID 18222 - The act of setting the text may change the text, if it's of an old version.
			CString strInitialText = AsString(m_pDetail->GetState());
			m_RichEditCtrl->RichText = _bstr_t(AsString(m_pDetail->GetState()));
			// (z.manning 2009-05-18 14:31) - PLID 34286 - Due to recent changes in the way narrative fields are
			// loaded in rich text control, sometimes the rich text was getting set here, but not matching the
			// actual rich text because the fields hadn't been loaded yet in the control. I added a check here
			// to only bother updating the rich text if the version did in fact change.
			// (z.manning 2009-06-18 17:14) - PLID 34286 - The above comment isn't wrong, but the logic behind
			// the original fix for this item is because there are things other than different versions that
			// require the state to be set here if the rich text change when we set it in the rich text control.
			if(CString((LPCTSTR)m_RichEditCtrl->RichText) != strInitialText) {
				//TES 7/25/2007 - PLID 26793 - This was RequestStateChange(); however, that had the additional effect
				// of setting m_bModified to TRUE for this detail, and as you can see by looking at the code and comments
				// in CEMNDetail::EnsureEmrItemAdvDlg(), ReflectCurrentState() shouldn't result in m_bModified being set.
				//NOTE: Since m_bModified is not being set, that now means that the new text will not be saved (unless
				// the detail is otherwise modified).  However, that's OK, because the next time this narrative is loaded
				// this code will get called again, so the narrative will be in exactly the state next time it's loaded
				// as it is this time.
				m_pDetail->SetState(m_RichEditCtrl->RichText);

				// (z.manning, 04/01/2008) - PLID 29473 - If the text did change then we need to handle that
				// for things such as topics' completion status.
				if(m_pDetail != NULL && m_pDetail->m_pParentTopic != NULL) {
					m_pDetail->m_pParentTopic->HandleDetailStateChange(m_pDetail);
				}
			}
		}
		m_RichEditCtrl->PutAllowFieldInsert(TRUE);
		m_bHasReflectedInitialState = TRUE;
	}
}

//TES 3/23/2010 - PLID 37757 - This dialog doesn't maintain its own ReadOnly flag, so I changed the function name from
// SetReadOnly() to ReflectReadOnlyStatus()
void CEmrItemAdvNarrativeDlg::ReflectReadOnlyStatus(BOOL bReadOnly)
{
	//TES 3/15/2010 - PLID 37757 - The ReadOnly flag lives in the detail now
	//m_bReadOnly = bReadOnly;
	if(IsWindow(GetSafeHwnd())) {
		m_wndLabel.EnableWindow(!m_pDetail->GetReadOnly());
		// (c.haag 2012-04-02) - PLID 49346 - Check whether the rich text control is NULL
		if (NULL != m_RichEditCtrl) {
			m_RichEditCtrl->ReadOnly = m_pDetail->GetReadOnly();
		}
	}

	CEmrItemAdvDlg::ReflectReadOnlyStatus(bReadOnly);
}

void CEmrItemAdvNarrativeDlg::UpdateRichTextAppearance()
{
	//
	// (c.haag 2007-07-30 12:47) - PLID 26858 - This function ensures that the
	// rich text and the narrative merge fields are "in synchronization". You 
	// should call this function if this detail's narrative merge field list is
	// maintained by an EMN (rather than by the detail), and the merge field list
	// was modified.
	//
	// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated
	/*
	if(IsWindow(GetSafeHwnd())) {
		if(NULL != m_RichEditCtrl) {
			CString strRichTextBeforeRemoval = (LPCTSTR)m_RichEditCtrl->RichText;

			m_RichEditCtrl->ParseMergeFields();

			if (strRichTextBeforeRemoval != CString((LPCTSTR)m_RichEditCtrl->RichText)) {
				m_pDetail->RequestStateChange(m_RichEditCtrl->RichText);
			}
		}
	}
	*/	

	// (a.walling 2009-11-19 15:08) - PLID 36365 - Push updates to the narrative
	m_pDetail->UpdateNarrativeFields();
}

void CEmrItemAdvNarrativeDlg::OnTextChangedRichTextCtrl()
{
	try {
		m_pDetail->RequestStateChange(m_RichEditCtrl->RichText);
	} NxCatchAll("Error in CEmrItemAdvNarrativeDlg::OnTextChangedRichTextCtrl");
}

void CEmrItemAdvNarrativeDlg::OnLinkRichTextCtrl(LPCTSTR strMergeField)
{
	// (c.haag 2008-11-25 11:39) - PLID 31693 - Moved functionality to CEmrItemAdvNarrativeBase
	HandleLinkRichTextCtrl(strMergeField, m_pDetail, GetSafeHwnd(), NULL);
}

// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated massive amounts of recordset-related narrative code

// (a.walling 2009-11-17 11:38) - PLID 36365
// (a.walling 2010-05-27 09:25) - PLID 38910 - Parameter to force update
void CEmrItemAdvNarrativeDlg::UpdateNarrativeFields(COleSafeArray& saFieldValues, bool bForceUpdate)
{	
	if(IsWindow(GetSafeHwnd())) {
		if(NULL != m_RichEditCtrl) {
			m_pDetail->InvalidateLinkedDetailCache();

			long nFieldsRemoved = 0;
			long nFieldsUpdated = 0;
			m_RichEditCtrl->PushFieldUpdates(saFieldValues, &nFieldsRemoved, &nFieldsUpdated);

			if (bForceUpdate || (nFieldsRemoved != 0 || nFieldsUpdated != 0)) {
				m_pDetail->RequestStateChange(m_RichEditCtrl->RichText);
			}
		}
	}
}

// (c.haag 2013-04-26) - PLID 56450 - Ensure the HTML is XHTML compliant for HTML narratives
void CEmrItemAdvNarrativeDlg::ValidateHtml()
{
	if (IsWindow(GetSafeHwnd()) && NULL != m_RichEditCtrl)
	{
		// This will fire a TextChanged event if anything changed
		m_RichEditCtrl->ValidateRichText();
	}
}

void CEmrItemAdvNarrativeDlg::OnRequestFormatRichTextCtrl(LPCTSTR strField, LPCTSTR strFlags, LPCTSTR *pstrValue)
{
	try {
		// (c.haag 2007-04-03 17:55) - PLID 25488 - Use GetMergeField for faster, cleaner lookups
		// (c.haag 2007-05-10 15:00) - PLID 25958 - GetMergeField has been replaced with SeekToNarrativeField
		// now that we use recordsets instead of a map
		// (c.haag 2007-08-15 16:34) - PLID 27084 - I had hoped to depreciate this function with the new
		// shared recordset logic, but alas, we can't do it for backwards compatibility reasons. Before 
		// we used recordsets, we would pass a string from a struct object into pstrValue. The struct
		// object persisted in memory; so while it was a horrible design, it wasn't crashy. We don't have 
		// persistent string objects any longer, so the solution here is to allocate one string per
		// request, and do garbage collection with it.

		// (c.haag 2007-08-15 16:34) - PLID 27084 - Clean up the old string if we have one
		// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated
		/*
		if (m_pstrRequestFormatRichTextCtrlResult) {
			delete m_pstrRequestFormatRichTextCtrlResult;
			m_pstrRequestFormatRichTextCtrlResult = NULL;
		}
		*/

		//ADODB::_RecordsetPtr prs;
		// (j.jones 2008-04-04 12:24) - PLID 29547 - added strFlags as a parameter

		
		// (a.walling 2009-11-25 12:21) - PLID 36365 - We don't need a CString, just use a LPTSTR
		if (m_szRequestFormatBuffer) {
			delete[] m_szRequestFormatBuffer;
			m_szRequestFormatBuffer = NULL;
		}

		// (a.walling 2009-11-17 16:31) - PLID 36365
		NarrativeField nf;
		nf.strField = strField;
		nf.strFlags = strFlags;
		m_pDetail->GetValueForNarrative(nf);
		//*pstrValue = new TCHAR[(nf.strValue.GetLength() + 1) * sizeof(TCHAR)];
		m_szRequestFormatBuffer = new TCHAR[(nf.strValue.GetLength() + 1) * sizeof(TCHAR)];
		strcpy((LPTSTR)m_szRequestFormatBuffer, (LPCTSTR)nf.strValue);
		*pstrValue = m_szRequestFormatBuffer;

		// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated
		/*
		if (NULL != (prs = m_pDetail->SeekToNarrativeField(strField, strFlags))) {
			if(CString(strFlags).Find("s") != -1) {
				//We want the sentence form.
				// (c.haag 2007-03-29 17:07) - PLID 25423 - Calculate the sentence form,
				// but only once (I think strTmp was for debugging in the past)
				// (c.haag 2007-05-15 11:15) - We have to do a crazy memory leak-causing workaround to satisfy this crazy rich text callback that we'll soon get rid of
				CString* pstrTmp = new CString(GetNarrativeSentenceFormField(prs));
				*pstrValue = *pstrTmp;
				// (c.haag 2007-08-15 16:34) - PLID 27084 - Remember our allocated string for
				// garbage collection
				m_pstrRequestFormatRichTextCtrlResult = pstrTmp;
			}
			else {
				// (c.haag 2007-03-29 17:07) - PLID 25423 - Calculate the value,
				// but only once (I think strTmp was for debugging in the past)
				CString* pstrTmp = new CString(GetNarrativeValueField(prs));
				*pstrValue = *pstrTmp;
				// (c.haag 2007-08-15 16:34) - PLID 27084 - Remember our allocated string for
				// garbage collection
				m_pstrRequestFormatRichTextCtrlResult = pstrTmp;
			}
		}
		*/
	} NxCatchAll("Error in CEmrItemAdvNarrativeDlg::OnRequestFormatRichTextCtrl");
}

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_FORMAT_SENTENCE	41000
#define ID_UNFORMAT_SENTENCE	41001
#define ID_REMOVE_FIELD	41002

void CEmrItemAdvNarrativeDlg::OnRightClickFieldRichTextCtrl(LPCTSTR strField, LPCTSTR strFlags, long x, long y, long nIndex)
{
	try {

		int nCmdId = GetNarrativeRightClickFieldOption(strField, strFlags, x, y);
		if(nCmdId == ID_FORMAT_SENTENCE || nCmdId == ID_UNFORMAT_SENTENCE) {
			CString strNewFlags = strFlags;
			if(strNewFlags.Find("s") == -1) {
				strNewFlags += "s";
			}
			else {
				strNewFlags.Remove('s');
			}
			m_RichEditCtrl->SetMergeFieldFlags(nIndex, _bstr_t(strNewFlags));
			//TES 9/13/2007 - PLID 27377 - This detail has now changed.
			m_pDetail->RequestStateChange(m_RichEditCtrl->RichText);
		}
		else if(nCmdId == ID_REMOVE_FIELD) {
			m_RichEditCtrl->RemoveMergeFieldByIndex(nIndex);
			//TES 9/13/2007 - PLID 27377 - This detail has now changed.
			m_pDetail->RequestStateChange(m_RichEditCtrl->RichText);
		}
		
		
	} NxCatchAll("Error in CEmrItemAdvNarrativeDlg::OnRightClickFieldRichTextCtrl");
}

//TES 7/6/2012 - PLID 51419 - Split this out so the context menu function can be called by both RTF and HTML narratives.
int CEmrItemAdvNarrativeDlg::GetNarrativeRightClickFieldOption(const CString &strField, const CString &strFlags, long x, long y)
{

	//Track the menu synchronously, because nIndex isn't guaranteed to stay valid.
	// (a.walling 2011-11-30 08:39) - PLID 46625 - Use CNxMenu
	CNxMenu mnu;
	mnu.CreatePopupMenu();
	// (a.walling 2009-11-19 15:08) - PLID 36365 - Show sentence unless it's a generic field

	BOOL bShowSentenceOption = TRUE;
	if (m_pDetail->m_pParentTopic && m_pDetail->m_pParentTopic->GetParentEMN()) {
		CString strValueDummy;
		bool bIsValidDummy;
		if (m_pDetail->m_pParentTopic->GetParentEMN()->GetGenericMergeFieldValue(strField, strValueDummy, bIsValidDummy)) {
			// this is a generic merge field, no sentence form available
			bShowSentenceOption = FALSE;
		}
	}
		
	if(bShowSentenceOption) {
		if(CString(strFlags).Find("s") == -1) {
			mnu.AppendMenu(MF_BYPOSITION, ID_FORMAT_SENTENCE, "Show Sentence Format");
		}
		else {
			mnu.AppendMenu(MF_BYPOSITION, ID_UNFORMAT_SENTENCE, "Hide Sentence Format");
		}
	}

	mnu.AppendMenu(MF_BYPOSITION, ID_REMOVE_FIELD, "Remove Field");
	int nCmdID = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD|TPM_TOPALIGN, x, y, this, NULL);
	mnu.DestroyMenu();
	return nCmdID;
}

void CEmrItemAdvNarrativeDlg::OnRightClickHtmlFieldRichTextCtrl(LPCTSTR strField, LPCTSTR strFlags, long x, long y, LPCTSTR strNexGUID)
{
	try {
		//TES 7/6/2012 - PLID 51419 - Separate function for HTML narratives, which identify their fields differently.
		int nCmdID = GetNarrativeRightClickFieldOption(strField, strFlags, x, y);
		if(nCmdID == ID_FORMAT_SENTENCE || nCmdID == ID_UNFORMAT_SENTENCE) {
			CString strNewFlags = strFlags;
			if(strNewFlags.Find("s") == -1) {
				strNewFlags += "s";
			}
			else {
				strNewFlags.Remove('s');
			}
			m_RichEditCtrl->SetMergeFieldFlagsByNexGuid(strNexGUID, _bstr_t(strNewFlags));
			//TES 9/13/2007 - PLID 27377 - This detail has now changed.
			m_pDetail->RequestStateChange(m_RichEditCtrl->RichText);
		}
		else if(nCmdID == ID_REMOVE_FIELD) {
			m_RichEditCtrl->RemoveMergeFieldByNexGuid(strNexGUID);
			//TES 9/13/2007 - PLID 27377 - This detail has now changed.
			m_pDetail->RequestStateChange(m_RichEditCtrl->RichText);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2012-10-05 15:04) - PLID 52986 - Handle right clicking HTML narrative, when selection is not a field.
// (j.armen 2013-03-19 11:38) - PLID 53102 - Now also called for RTF Narratives
void CEmrItemAdvNarrativeDlg::OnRightClickHtmlRichTextCtrl(long x, long y, bool bCanCopy, bool bCanPaste)
{
	try
	{
		enum {cut = 1, copy, paste, macro};
		CNxMenu mnu;
		mnu.CreatePopupMenu();

		// Can cut when we can copy and are not read only
		mnu.AppendMenu((!m_pDetail->GetReadOnly() && bCanCopy) ? MF_BYPOSITION : (MF_BYPOSITION|MF_GRAYED), cut, "Cu&t");
		// Can copy when we can copy
		mnu.AppendMenu(bCanCopy ? MF_BYPOSITION : (MF_BYPOSITION|MF_GRAYED), copy, "&Copy");
		// Can paste when we can paste and are not read only
		mnu.AppendMenu((!m_pDetail->GetReadOnly() && bCanPaste) ? MF_BYPOSITION : (MF_BYPOSITION|MF_GRAYED), paste, "&Paste");
		mnu.AppendMenu(MF_BYPOSITION|MF_MENUBREAK);
		// Can paste macro text when we are not read only
		mnu.AppendMenu(!m_pDetail->GetReadOnly() ? MF_BYPOSITION : (MF_BYPOSITION|MF_GRAYED), macro, "Paste &Macro Text...");

		int nCmdID = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD|TPM_TOPALIGN, x, y, this, NULL);
		mnu.DestroyMenu();
		switch(nCmdID)
		{
			case cut:
				m_RichEditCtrl->Cut();
				break;
			case copy:
				m_RichEditCtrl->Copy();
				break;
			case paste:
				m_RichEditCtrl->Paste();
				break;
			case macro:
				CEmrTextMacroDlg dlg(this);
				if(IDOK == dlg.DoModal())
					m_RichEditCtrl->ReplaceSel(_bstr_t(dlg.m_strResultTextData), g_cvarTrue);
				break;
		}
	}NxCatchAll(__FUNCTION__);
}

// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated
/*
void CEMRItemAdvPopupWnd::BeginAddMergeFields()
{
	// (c.haag 2007-05-14 11:36) - PLID 25970 - The rich edit control is no longer
	// responsible for maintaining its own field list, so this function is depreciated.
	// However, we must maintain the Adding Merge Fields flag ourselves.
	*/
	/*if(m_RichEditCtrl) {
		m_RichEditCtrl->BeginAddMergeFields();
	}*/
	/*
	m_bAddingMergeFields = TRUE;
}
*/

// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated
/*
void CEMRItemAdvPopupWnd::EndAddMergeFields()
{
	// (c.haag 2007-05-14 11:36) - PLID 25970 - The rich edit control is no longer
	// responsible for maintaining its own field list, so while EndAddMergeFields
	// is depreciated, we still need to update the form content
	m_bAddingMergeFields = FALSE;
	if(m_RichEditCtrl) {
		//m_RichEditCtrl->EndAddMergeFields();
		m_RichEditCtrl->ParseMergeFields();
	}
}
*/

BOOL CEmrItemAdvNarrativeDlg::IsStateSet()
{
	if(m_RichEditCtrl) {
		return m_RichEditCtrl->IsNarrativeComplete();
	}
	else {
		//hmm....
		return FALSE;
	}
}

// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated massive amounts of recordset-related narrative code

// (j.jones 2008-01-14 09:44) - PLID 18709 - added OnRequestLWMergeFieldData
void CEmrItemAdvNarrativeDlg::OnRequestLWMergeFieldData()
{
	try {
		
		//when we get this message from the richedit control, we can assume
		//that the richedit control has set LWMergeDataNeeded = TRUE in the
		//narrative recordset for at least one record, possibly multiple

		//NarrativeRequestLWMergeFieldData will loop through the recordset,
		//grab all the merge fields we need to load, load them, populate
		//the recordset, then set LWMergeDataNeeded = FALSE

		// (a.walling 2009-11-19 15:08) - PLID 36365 - the control may have new fields and etc in its own internal state, so pass in the most up to date state
		NarrativeRequestLWMergeFieldData(m_pDetail, (LPCTSTR)m_RichEditCtrl->RichText);


		// (a.walling 2009-11-19 15:08) - PLID 36365 - Push updates to the narrative
		m_pDetail->UpdateNarrativeFields(NULL, (LPCTSTR)m_RichEditCtrl->RichText);

	} NxCatchAll("Error in CEmrItemAdvNarrativeDlg::OnRequestLWMergeFieldData");
}

//TES 9/16/2009 - PLID 35529 - Assigns this dialog to the given detail (only used for the global, "windowless" dialog)
void CEmrItemAdvNarrativeDlg::SetDetail(CEMNDetail *pDetail)
{
	m_pDetail = pDetail;
	// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated
	//m_RichEditCtrl->MergeFieldSet = m_pDetail->GetNarrativeFieldRecordset();
	ReflectCurrentState();
}

// (a.walling 2009-11-19 14:21) - PLID 36367 - Requesting list of various merge fields from the host
void CEmrItemAdvNarrativeDlg::OnRequestAllAvailableFields(VARIANT_BOOL bIncludeLetterWriting, VARIANT_BOOL bIncludeEMR, VARIANT_BOOL bIncludeListItems, VARIANT* psafearrayFields)
{
	try {
		COleSafeArray saFields;
		// (z.manning 2011-11-10 10:14) - PLID 46382 - Pass in 1 for the version here as we now consider 1 the pre-versioned version.
		m_pDetail->PopulateAvailableNarrativeFields(saFields, bIncludeLetterWriting != VARIANT_FALSE ? true : false, bIncludeEMR != VARIANT_FALSE ? true : false, bIncludeListItems != VARIANT_FALSE ? true : false);

		*psafearrayFields = saFields.Detach();
	} NxCatchAll("Error loading available narrative fields");
}

// (z.manning 2011-11-10 10:15) - PLID 46382
void CEmrItemAdvNarrativeDlg::OnRequestAvailableFieldsVersion(short* pnVersion)
{
	try
	{
		*pnVersion = NARRATIVE_AVAILABLE_FIELD_VERSION;
	}
	NxCatchAll(__FUNCTION__);
}

// (c.haag 2012-04-02) - PLID 49346 - Pre-navigation handler
void CEmrItemAdvNarrativeDlg::OnBeforeNavigate2HtmlEdit(LPDISPATCH pDisp, VARIANT* URL, VARIANT* Flags, VARIANT* TargetFrameName, VARIANT* PostData, VARIANT* Headers, BOOL* Cancel)
{
	try 
	{
		// Fordid any navigations except the initial one
		if (m_bHasReflectedInitialState || NULL == URL || VarString(URL) !=  "about:blank") 
		{
			*Cancel = TRUE;

			// See if we have a valid URL. If it's valid, it should be pointing to a detail. If it is, then pop it up.
			if (NULL != URL)	
			{
				CString strUrl = VarString(URL);
				if (strUrl.Left(18) == "nexemr://detailID/")
				{
					CString strDetailID = strUrl.Mid(18);
					CEMNDetail* pDetail = m_pDetail->GetParentEMN()->GetDetailByID(atol(strDetailID));
					if (NULL != pDetail) {
						HandleLinkRichTextCtrl(pDetail, m_pDetail, GetSafeHwnd(), NULL);
					} else {
						//If you get this to happen, then we know there is some way to get a narrative which contains a field
						//	which does not have a matching name anywhere on this EMN.
						AfxThrowNxException("Narrative could not popup the detail for this merge field.  It may have been removed from this EMN.");
					}
				}
			}			
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (c.haag 2012-04-02) - PLID 49346 - Post-navigation handler
void CEmrItemAdvNarrativeDlg::OnNavigateComplete2HtmlEdit(LPDISPATCH pDisp, VARIANT* URL)
{
	try 
	{
		// Once the navigation is done, we will assign
		if (!m_bHasReflectedInitialState)
		{
			SetHtmlText( AsString(m_pDetail->GetState()) );
			m_bHasReflectedInitialState = TRUE;
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (c.haag 2012-04-02) - PLID 49346 - This function will assign HTML to the browser control. This should only
// be used for HTML narratives.
// (j.armen 2012-07-18 14:28) - PLID 51605 - Fixed BSTR leak
void CEmrItemAdvNarrativeDlg::SetHtmlText(const CString& strHtml)
{
	IDispatchPtr p;
	HRESULT hr = m_pHtmlEdit->get_Document(&p);
	if (SUCCEEDED(hr)) {
		IHTMLDocument2Ptr pDoc;
		hr = p->QueryInterface(IID_IHTMLDocument2, (void**)&pDoc);
		if (SUCCEEDED(hr))
		{
			_variant_t varHtml = (LPCTSTR)strHtml;
			COleSafeArray saHtml;
			saHtml.CreateOneDim(VT_VARIANT, 1, &varHtml);
			varHtml.Detach(); // now owned by safe array
			hr = pDoc->write(saHtml.parray);
			if (!SUCCEEDED(hr))
			{
				ThrowNxException("Failed to write HTML to the document!");
			}
		} else {
			ThrowNxException("Failed to get the HTML document in SetHTMLText!");
		}
	} else {
		ThrowNxException("Failed to get the HTML document in SetHTMLText!");
	}
}

void CEmrItemAdvNarrativeDlg::OnCheckValidIDsRichTextCtrl(long nDetailID, BOOL *pbDetailIDExists, long nDataID, BOOL *pbDataIDExists)
{
	try {
		//TES 7/3/2012 - PLID 51357 - The control wants to know whether the DetailID and DataID it has actually exist on this EMN.
		CEMNDetail* pDetail = m_pDetail->GetParentEMN()->GetDetailByID(nDetailID);
		if(pDetail) {
			*pbDetailIDExists = TRUE;
			ListElement *ple = pDetail->GetListElementByID(nDataID);
			if(ple) {
				*pbDataIDExists = TRUE;
			}
			else {
				*pbDataIDExists = FALSE;
			}
		}
		else {
			*pbDetailIDExists = FALSE;
			*pbDataIDExists = FALSE;
		}
	}NxCatchAll(__FUNCTION__);
}