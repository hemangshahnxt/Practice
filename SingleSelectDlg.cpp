// SingleSelectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SingleSelectDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CSingleSelectDlg dialog

enum SingleSelectColumns {

	sscID = 0,
	sscValue,
};

CSingleSelectDlg::CSingleSelectDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSingleSelectDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSingleSelectDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nSelectedID = -1;
	m_nPreSelectID = -1;
	m_varSelectedDisplayValue = g_cvarNull;
	m_bForceSelection = false;
	m_bCheckAdditionalFilter = FALSE;
	m_strCancelButtonText = "Cancel";
	m_eCancelButtonStyle = NXB_CANCEL;
	m_pastrValues = NULL; // (c.haag 2010-09-24 10:41) - PLID 40677
}


void CSingleSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSingleSelectDlg)
	DDX_Control(pDX, IDC_STATIC_DESCRIPTION, m_nxstaticDescription);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_SINGLE_SELECT_FILTER_CHECK, m_btnAdditionalFilterCheck);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSingleSelectDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSingleSelectDlg)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_SINGLE_SELECT_FILTER_CHECK, &CSingleSelectDlg::OnBnClickedSingleSelectFilterCheck)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSingleSelectDlg message handlers

BOOL CSingleSelectDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	if (m_dlList == NULL) {
		m_dlList = BindNxDataListCtrl(this, IDC_SINGLESELECT_LIST, GetRemoteData(), false);
	}

	// (z.manning, 04/30/2008) - PLID 29845 - Set button styles
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(m_eCancelButtonStyle);
	m_btnCancel.SetWindowText(m_strCancelButtonText);

	m_dlList->FromClause = (LPCTSTR)m_strFrom;

	IColumnSettingsPtr(m_dlList->GetColumn(sscID))->FieldName = (LPCTSTR)m_strIDField;
	IColumnSettingsPtr(m_dlList->GetColumn(sscValue))->FieldName = (LPCTSTR)m_strValueField;

	if(!m_strAdditionalWhere.IsEmpty()) {
		// (z.manning 2010-05-11 13:23) - PLID 37416 - We have an additional filter to show the checkbox
		m_btnAdditionalFilterCheck.EnableWindow();
		m_btnAdditionalFilterCheck.ShowWindow(SW_SHOW);
		m_btnAdditionalFilterCheck.SetWindowText(m_strFilterCheckDisplayText);
		m_btnAdditionalFilterCheck.SetCheck(m_bCheckAdditionalFilter ? BST_CHECKED : BST_UNCHECKED);
	}

	RequeryList();

	// (c.haag 2010-09-24 10:41) - PLID 40677 - How did pre-selecting used to even work with this code below
	// the TrySetSelByColumn???
	
	// (j.jones 2013-01-04 11:35) - PLID 49624 - This was never supposed to be here. The PreSelect always
	// cleared it when setting by ID, it didn't work right for setting by text. Without PreSelect in use,
	// nothing should be selected by default.
	/*
	else if (m_dlList->GetRowCount() > 0)
		m_dlList->PutCurSel(0);
	*/

	// (c.haag 2010-09-24 10:41) - PLID 40677 - Pre-select a text value for memory text-only selections
	if (NULL != m_pastrValues)
	{
		if (!m_strPreSelect.IsEmpty()) {
			m_dlList->TrySetSelByColumn(sscValue, _bstr_t(m_strPreSelect));
		}
	}
	else {
		m_dlList->TrySetSelByColumn(sscID, m_nPreSelectID);
	}
	
	GetDlgItem(IDC_STATIC_DESCRIPTION)->SetWindowText(m_strDescription);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE}
}

//TES 10/13/2008 - PLID 21093 - Added a parameter to tell the dialog to not let them leave the selection blank,
// they will have to either select a record, or Cancel.
HRESULT CSingleSelectDlg::Open(CString strFrom, CString strWhere,
		CString strIDField, CString strValueField, CString strDescription, bool bForceSelection /*= false*/)
{
	m_strFrom = strFrom;
	m_strWhere = strWhere;
	m_strIDField = strIDField;
	m_strValueField = strValueField;
	m_strDescription = strDescription;
	m_bForceSelection = bForceSelection;
	return DoModal();
}

// (c.haag 2010-09-24 10:41) - PLID 40677 - Support for memory values. The ID will be the index in the array.
HRESULT CSingleSelectDlg::Open(CStringArray* pastrValues, CString strDescription, bool bForceSelection /*= false*/)
{
	m_pastrValues = pastrValues;
	m_strDescription = strDescription;
	m_bForceSelection = bForceSelection;
	return DoModal();
}

long CSingleSelectDlg::GetSelectedID()
{
	return m_nSelectedID;
}

// (j.jones 2007-08-23 09:17) - PLID 27148 - added so we could get
// the display name, but a variant since we can't assume it's a string
_variant_t CSingleSelectDlg::GetSelectedDisplayValue()
{
	return m_varSelectedDisplayValue;
}

void CSingleSelectDlg::OnOK() 
{
	if (m_dlList->CurSel > -1) {
		m_nSelectedID = VarLong(m_dlList->GetValue(m_dlList->CurSel, sscID));
		// (j.jones 2007-08-23 09:17) - PLID 27148 - store the variant in the display column
		m_varSelectedDisplayValue = m_dlList->GetValue(m_dlList->CurSel, sscValue);
	}
	else {
		//TES 10/13/2008 - PLID 21093 - Our caller doesn't want to allow this, tell the user to make up their mind!
		if(m_bForceSelection) {
			//TES 8/5/2010 - PLID 39642 - Use the actual cancel button text (was hard-coded to "Cancel")
			MsgBox("Please select an option from the list, or choose %s", m_strCancelButtonText);
			return;
		}
		else {
			m_nSelectedID = -1;
		}
	}

	CNxDialog::OnOK();
}

void CSingleSelectDlg::PreSelect(long ID)
{
	m_nPreSelectID = ID;
}

// (c.haag 2010-09-24 10:41) - PLID 40677 - Support for pre-selecting a string (for memory values only)
void CSingleSelectDlg::PreSelect(const CString& str)
{
	m_strPreSelect = str;
}

// (z.manning 2010-05-11 13:02) - PLID 37416
void CSingleSelectDlg::OnBnClickedSingleSelectFilterCheck()
{
	try
	{
		_variant_t varCurSelID = g_cvarNull;
		long nCurSel = m_dlList->GetCurSel();
		if(nCurSel != -1) {
			varCurSelID = m_dlList->GetValue(nCurSel, sscID);
		}

		RequeryList();
		if(varCurSelID.vt != VT_NULL) {
			// (z.manning 2010-05-11 14:20) - PLID 37416 - Try to preserve the previous selection though it
			// may no longer be in the list which is fine.
			m_dlList->TrySetSelByColumn(sscID, varCurSelID);
		}

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2010-05-11 13:02) - PLID 37416 - Added a checkbox to the single select dialog to have 2
// different ways of filtering the list.
void CSingleSelectDlg::UseAdditionalFilter(const CString strAdditionalWhere, const CString strDisplayText, const BOOL bCheckedByDefault)
{
	m_strAdditionalWhere = strAdditionalWhere;
	m_strFilterCheckDisplayText = strDisplayText;
	m_bCheckAdditionalFilter = bCheckedByDefault;
}

// (z.manning 2010-05-11 14:14) - PLID 37416 - Moved the requery logic to its own function
void CSingleSelectDlg::RequeryList()
{
	// (c.haag 2010-09-24 10:41) - PLID 40677 - Populate the list from strings in memory
	if (NULL != m_pastrValues) 
	{
		m_dlList->Clear(); // Need to clear the list first
		for (long i=0; i < m_pastrValues->GetSize(); i++) {
			IRowSettingsPtr pRow = m_dlList->GetRow(-1);
			pRow->Value[sscID] = i;
			pRow->Value[sscValue] = _bstr_t(m_pastrValues->GetAt(i));
			m_dlList->AddRow(pRow);
		}
	}
	else {
		CString strWhere;
		if(!m_strWhere.IsEmpty()) {
			strWhere += "(" + m_strWhere + ")";
		}

		if(!m_strAdditionalWhere.IsEmpty() && m_btnAdditionalFilterCheck.GetCheck() == BST_CHECKED) {
			if(!strWhere.IsEmpty()) {
				strWhere += " AND ";
			}
			strWhere += "(" + m_strAdditionalWhere + ")";
		}
		m_dlList->PutWhereClause(_bstr_t(strWhere));
		m_dlList->Requery();
	}
}

// (z.manning 2010-05-11 14:43) - PLID 37416 - Added a function to set different text and style to the cancel button
void CSingleSelectDlg::SetCancelButtonStyle(const CString strText, const NXB_TYPE eStyle)
{
	m_strCancelButtonText = strText;
	m_eCancelButtonStyle = eStyle;
}
