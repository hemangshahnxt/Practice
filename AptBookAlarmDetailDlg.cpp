// AptBookAlarmDetailDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AptBookAlarmDetailDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CAptBookAlarmDetailDlg dialog


CAptBookAlarmDetailDlg::CAptBookAlarmDetailDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CAptBookAlarmDetailDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAptBookAlarmDetailDlg)
		m_AptTypeID = -1;
		m_AptPurposeID = -1;
	//}}AFX_DATA_INIT
}


void CAptBookAlarmDetailDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAptBookAlarmDetailDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAptBookAlarmDetailDlg, CNxDialog)
	//{{AFX_MSG_MAP(CAptBookAlarmDetailDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAptBookAlarmDetailDlg message handlers

BOOL CAptBookAlarmDetailDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {

		// (z.manning, 04/30/2008) - PLID 29845 - Set button styles
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_AptTypeList = BindNxDataListCtrl(this,IDC_CHOOSE_TYPE,GetRemoteData(),true);
		m_AptPurposeList = BindNxDataListCtrl(this,IDC_CHOOSE_PURPOSE,GetRemoteData(),true);

		IRowSettingsPtr pRow = m_AptTypeList->GetRow(-1);
		pRow->PutValue(0,(long)-1);
		pRow->PutValue(1,"<All Types>");
		m_AptTypeList->AddRow(pRow);		

		pRow = m_AptPurposeList->GetRow(-1);
		pRow->PutValue(0,(long)-1);
		pRow->PutValue(1,"<All Purposes>");
		m_AptPurposeList->AddRow(pRow);

		if(m_AptTypeID == -1) {
			m_AptTypeList->SetSelByColumn(0, (long)-1);
		}
		else {
			if(m_AptTypeList->SetSelByColumn(0,m_AptTypeID) == -1) {
				//Maybe an inactive type.
				_RecordsetPtr rsType = CreateRecordset("SELECT Name FROM AptTypeT WHERE ID = %li", m_AptTypeID);
				if(!rsType->eof) {
					m_AptTypeList->PutComboBoxText(_bstr_t(AdoFldString(rsType, "Name")));
				}
			}
		}

		if(m_AptPurposeID == -1)
			m_AptPurposeList->SetSelByColumn(0, (long)-1);
		else {
			// (c.haag 2008-12-18 12:55) - PLID 32539 - If we didn't find the procedure, it may be inactive. Add
			// the inactive row.
			if (sriNoRow == m_AptPurposeList->SetSelByColumn(0,m_AptPurposeID) && m_AptPurposeID > 0) {
				_RecordsetPtr prs = CreateParamRecordset("SELECT Name FROM ProcedureT WHERE ID = {INT}", m_AptPurposeID);
				if (!prs->eof) {
					pRow = m_AptPurposeList->GetRow(-1);
					pRow->PutValue(0,m_AptPurposeID);
					pRow->PutValue(1,prs->Fields->Item["Name"]->Value);
					m_AptPurposeList->AddRow(pRow);
					m_AptPurposeList->Sort();
					m_AptPurposeList->SetSelByColumn(0, m_AptPurposeID);
				} else {
					// It was deleted. Nothing we can do.
				}
			}
		}

	}NxCatchAll("Error loading alarm details.");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CAptBookAlarmDetailDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CAptBookAlarmDetailDlg)
	ON_EVENT(CAptBookAlarmDetailDlg, IDC_CHOOSE_TYPE, 16 /* SelChosen */, OnSelChosenChooseType, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CAptBookAlarmDetailDlg::OnSelChosenChooseType(long nRow) 
{
	try {

		//filter the type list to only types of the selected purpose

		long nAptTypeID = -1;
		if(nRow != -1) {
			nAptTypeID = VarLong(m_AptTypeList->GetValue(nRow,0),-1);
		}

		CString strPurposeFilter = "";
		
		if(nAptTypeID != -1) {
			// (c.haag 2008-12-18 12:55) - PLID 32376 - Filter out inactive procedures
			strPurposeFilter.Format(
				"AptPurposeT.ID IN (SELECT AptPurposeTypeT.AptPurposeID FROM AptPurposeTypeT WHERE AptPurposeTypeT.AptTypeID = %li) "
				"AND AptPurposeT.ID NOT IN (SELECT ID FROM ProcedureT WHERE Inactive = 1) ", 
				nAptTypeID);
		}
		m_AptPurposeList->WhereClause = _bstr_t(strPurposeFilter);
		m_AptPurposeList->Requery();

		IRowSettingsPtr pRow = m_AptPurposeList->GetRow(-1);
		pRow->PutValue(0,(long)-1);
		pRow->PutValue(1,"<All Purposes>");
		m_AptPurposeList->AddRow(pRow);

	}NxCatchAll("Error filtering purpose list.");
}

void CAptBookAlarmDetailDlg::OnOK() 
{
	try {

		if(m_AptTypeList->CurSel == -1) {
			if(m_AptTypeList->GetComboBoxText() == _bstr_t("") || m_AptPurposeList->CurSel == -1) {
				AfxMessageBox("Please select an item in each list.");
				return;
			}
			//The type must be the same inactive one we originally loaded.
			m_AptPurposeID = VarLong(m_AptPurposeList->GetValue(m_AptPurposeList->GetCurSel(),0),-1);
		}
		else {
			if(m_AptTypeList->GetCurSel() == -1 || m_AptPurposeList->GetCurSel() == -1) {
				AfxMessageBox("Please select an item in each list.");
				return;
			}

			m_AptTypeID = VarLong(m_AptTypeList->GetValue(m_AptTypeList->GetCurSel(),0),-1);
			m_AptPurposeID = VarLong(m_AptPurposeList->GetValue(m_AptPurposeList->GetCurSel(),0),-1);
		}

	}NxCatchAll("Error setting detail information.");
	
	CDialog::OnOK();
}
