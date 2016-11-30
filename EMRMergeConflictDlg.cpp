// EMRMergeConflictDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EMRMergeConflictDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CEMRMergeConflictDlg dialog


CEMRMergeConflictDlg::CEMRMergeConflictDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMRMergeConflictDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEMRMergeConflictDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nSelectedDetailID = -1;
}


void CEMRMergeConflictDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMRMergeConflictDlg)
	DDX_Control(pDX, IDC_STATIC_MERGECONFLICT_DESCRIPTION, m_nxstaticMergeconflictDescription);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEMRMergeConflictDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMRMergeConflictDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMRMergeConflictDlg message handlers

BOOL CEMRMergeConflictDlg::OnInitDialog() 
{
	try {
		//TES 8/11/2005 - It's possible that there is only one detail here (if we were given two or more details with the same data).
		//If that's the case, just return that item.
		if(m_adwDetailIDs.GetSize() == 1) {
			m_nSelectedDetailID = m_adwDetailIDs.GetAt(0);
			// (z.manning 2010-01-14 10:22) - PLID 36879 - Call EndDialog here instead of OnOK or
			// else we'll get assertions since CNxDialog::OnInitDialog hasn't been called yet.
			//CDialog::OnOK();
			EndDialog(IDOK);
			return TRUE;
		}

		CNxDialog::OnInitDialog();

		// (c.haag 2008-05-19 16:47) - PLID 29806 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		
		// (c.haag 2004-10-05 10:57) - PLID 14242 - Assign the title text and
		// list of collections involved in this conflict.
		CString str;
		str.Format("The field '%s' exists in multiple EMN's that are included in the merge. Please select which Detail to include merge data from.", m_strFieldName);
		SetDlgItemText(IDC_STATIC_MERGECONFLICT_DESCRIPTION, str);
		m_dlDetails = BindNxDataListCtrl(this,IDC_LIST_COLLECTION_CONFLICTS,GetRemoteData(),false);

		for (long i=0; i < m_astrDetails.GetSize(); i++)
		{
			IRowSettingsPtr pRow = m_dlDetails->GetRow(-1);
			pRow->Value[0] = (long)m_adwDetailIDs[i];
			pRow->Value[1] = _bstr_t(m_astrDetails[i]);
			pRow->Value[2] = _bstr_t(m_astrFormattedData[i]);
			m_dlDetails->AddRow(pRow);
		}
		m_dlDetails->Sort();
		GetDlgItem(IDOK)->EnableWindow(FALSE);
	} NxCatchAll("Error in CEMRMergeConflictDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEMRMergeConflictDlg::SetFieldName(const CString& strFieldName)
{
	m_strFieldName = strFieldName;
}

void CEMRMergeConflictDlg::AddDetail(const CString& strCollectionName,
										 const CString& strFormattedData,
										 long nDetailID)
{
	//TES 8/11/2005 - Only add it if it hasn't already been added.
	for(int i = 0; i < m_astrFormattedData.GetSize(); i++) {
		if(m_astrFormattedData.GetAt(i) == strFormattedData) return;
	}

	m_astrDetails.Add(strCollectionName);
	m_adwDetailIDs.Add(nDetailID);
	if (-1 != strFormattedData.Find("NXIMAGE")) {
		m_astrFormattedData.Add("<Image Data>");
	} else if (-1 != strFormattedData.Find("NXRTF")) {
		m_astrFormattedData.Add("<Document Data>");
	} else {
		m_astrFormattedData.Add(strFormattedData);
	}
}

BEGIN_EVENTSINK_MAP(CEMRMergeConflictDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEMRMergeConflictDlg)
	ON_EVENT(CEMRMergeConflictDlg, IDC_LIST_COLLECTION_CONFLICTS, 3 /* DblClickCell */, OnDblClickCellListDetailConflicts, VTS_I4 VTS_I2)
	ON_EVENT(CEMRMergeConflictDlg, IDC_LIST_COLLECTION_CONFLICTS, 2 /* SelChanged */, OnSelChangedListDetailConflicts, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEMRMergeConflictDlg::OnDblClickCellListDetailConflicts(long nRowIndex, short nColIndex) 
{
	if (nRowIndex < 0) return;
	m_nSelectedDetailID = m_dlDetails->Value[nRowIndex][0];
	GetDlgItem(IDOK)->EnableWindow(TRUE);
	PostMessage(WM_COMMAND, IDOK);
}

long CEMRMergeConflictDlg::GetSelectedDetailID()
{
	return m_nSelectedDetailID;
}

void CEMRMergeConflictDlg::OnSelChangedListDetailConflicts(long nNewSel) 
{
	if (nNewSel < 0) return;
	m_nSelectedDetailID = m_dlDetails->Value[nNewSel][0];	
	GetDlgItem(IDOK)->EnableWindow(TRUE);
}
