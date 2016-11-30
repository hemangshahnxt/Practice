// InputListDlg.cpp : implementation file
//

#include "stdafx.h"
#include "InputListDlg.h"

#include "GlobalDataUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CInputListDlg dialog


CInputListDlg::CInputListDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CInputListDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInputListDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_paryIdList = NULL;
	m_pstrTextList = NULL;
	m_strSql = "";
	m_bMultiSelect = TRUE;
}


void CInputListDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInputListDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInputListDlg, CDialog)
	//{{AFX_MSG_MAP(CInputListDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInputListDlg message handlers

void CInputListDlg::OnOK() 
{
	// Clear out the lists because we're about to add all the elements
	if (m_paryIdList) {
		m_paryIdList->RemoveAll();
	}
	if (m_pstrTextList) {
		(*m_pstrTextList) = "";
	}
	// Store the selected rows in our output variables
	LPDISPATCH lpDisp;
	for (long p = m_dlInputList->GetFirstSelEnum(); p; ) {
		m_dlInputList->GetNextSelEnum(&p, &lpDisp);
		if (lpDisp) {
			IRowSettingsPtr pRow(lpDisp);
			lpDisp->Release();
			if (m_paryIdList) {
				m_paryIdList->Add(VarLong(pRow->Value[0], -1));
			}
			if (m_pstrTextList) {
				(*m_pstrTextList) += VarString(pRow->Value[1], "") + ", ";
			}
		} else {
			ASSERT(FALSE); // The datalist's fault
			break;
		}
	}

	// The string list will end up with a terminating comma
	if (m_pstrTextList && (*m_pstrTextList).Right(2) == ", ") {
		(*m_pstrTextList).Delete((*m_pstrTextList).GetLength()-2, 2);
	}
	
	CDialog::OnOK();
}

int CInputListDlg::ZoomList(IN const CString &strSql, IN BOOL bMultiSelect, IN OUT CDWordArray &aryIdList, OUT CString *pstrTextListing /*= NULL*/)
{
	m_strSql = strSql;
	m_bMultiSelect = bMultiSelect;
	m_paryIdList = &aryIdList;
	m_pstrTextList = pstrTextListing;

	return DoModal();
}

BOOL CInputListDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	try {
		// (z.manning, 04/30/2008) - PLID 29845 - Set button styles
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// Get the datalist object
		m_dlInputList = BindNxDataListCtrl(this, IDC_INPUT_LIST, GetRemoteData(), false);
		
		// Set the from clause
		CString strFrom;
		strFrom.Format("(%s) T", m_strSql);
		m_dlInputList->FromClause = _bstr_t(strFrom);

		// Allow/disallow mutliple selections
		m_dlInputList->AllowMultiSelect = m_bMultiSelect;

		// Requery
		m_dlInputList->Requery();
	} NxCatchAllCall("CInputListDlg::OnInitDialog", {
		// Did we get a bad sql?  Remember it MUST be a wff select query that returns two fields, called ID and Name.
		EndDialog(IDCANCEL);
		return FALSE;
	});

	try {
		// Iterate through the whole datalist, unselecting items that are not in the list, selecting items that are
		if (m_paryIdList) {
			m_dlInputList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
			LPDISPATCH lpDisp;
			for (long p = m_dlInputList->GetFirstRowEnum(); p; ) {
				m_dlInputList->GetNextRowEnum(&p, &lpDisp);
				if (lpDisp) {
					IRowSettingsPtr pRow(lpDisp);
					lpDisp->Release();
					if (IsIDInArray(VarLong(pRow->Value[0], -1), m_paryIdList)) {
						pRow->Selected = TRUE;
					} else {
						pRow->Selected = FALSE;
					}
				} else {
					ASSERT(FALSE); // The datalist's fault
					break;
				}
			}
		}
	} NxCatchAllCall("CInputListDlg::OnInitDialog", {
		// Did we get a bad sql?  Remember it MUST be a wff select query that returns two fields, called ID and Name.
		EndDialog(IDCANCEL);
		return FALSE;
	});

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
