// CustomRecordActionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "administratorRc.h"
#include "CustomRecordActionDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CEmrActionDlg dialog

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_REMOVE_CPT	39000
#define ID_REMOVE_DIAG	39001



CCustomRecordActionDlg::CCustomRecordActionDlg(CWnd* pParent)
	: CNxDialog(CCustomRecordActionDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCustomRecordActionDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CCustomRecordActionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCustomRecordActionDlg)
	DDX_Control(pDX, IDC_ACTION_CAPTION, m_nxstaticActionCaption);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCustomRecordActionDlg, CNxDialog)
	//{{AFX_MSG_MAP(CCustomRecordActionDlg)
	ON_COMMAND(ID_REMOVE_CPT, OnRemoveCpt)
	ON_COMMAND(ID_REMOVE_DIAG, OnRemoveDiag)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCustomRecordActionDlg message handlers

using namespace ADODB;

BOOL CCustomRecordActionDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		CString strCaption;
		_RecordsetPtr rsSourceName = CreateRecordset("SELECT %s FROM %s WHERE %s = %li", GetEmrActionObjectSourceNameField(m_SourceType), GetEmrActionObjectSourceTable(m_SourceType), GetEmrActionObjectSourceIdField(m_SourceType), m_nSourceID);
		if(rsSourceName->eof) {
			MsgBox("Attempted to load Action dialog for invalid object!");
			CDialog::OnOK();
			return TRUE;
		}
		strCaption.Format("When the %s \"%s\" is added to a custom record, automatically add the following items:", GetEmrActionObjectName(m_SourceType, FALSE), VarString(rsSourceName->Fields->Item[0L]->Value, ""));
		SetDlgItemText(IDC_ACTION_CAPTION, strCaption);

		// (z.manning, 04/30/2008) - PLID 29852 - Set button styles
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_pCptCombo = BindNxDataListCtrl(IDC_ACTION_CPT_COMBO);
		m_pCptList = BindNxDataListCtrl(IDC_ACTION_CPT_LIST, false);
		CString strWhere;
		strWhere.Format("ServiceT.ID IN (SELECT DestID FROM EmrActionsT WHERE SourceType = %li AND SourceID = %li AND DestType = %li AND Deleted = 0)", 
			m_SourceType, m_nSourceID, eaoCpt);
		m_pCptList->WhereClause = _bstr_t(strWhere);
		m_pCptList->Requery();

		m_pDiagCombo = BindNxDataListCtrl(IDC_ACTION_DIAG_COMBO);
		m_pDiagList = BindNxDataListCtrl(IDC_ACTION_DIAG_LIST, false);
		// (r.gonet 03/02/2014) - PLID 61131 - Removed support for ICD-10
		// (b.savon 2014-07-14 13:15) - PLID 62706 - Deprecate the old Diag DestType and handle the changes Practice wide
		strWhere.Format("DiagCodes.ID IN (SELECT DiagCodeID_ICD9 FROM EmrActionsT INNER JOIN EmrActionDiagnosisDataT ON EmrActionsT.ID = EmrActionDiagnosisDataT.EmrActionID WHERE SourceType = %li AND SourceID = %li AND DestType = %li AND Deleted = 0) AND DiagCodes.ICD10 = 0", 
			m_SourceType, m_nSourceID, eaoDiagnosis/*eaoDiag*/);
		m_pDiagList->WhereClause = _bstr_t(strWhere);
		m_pDiagList->Requery();
	}NxCatchAll("Error in CEmrActionDlg::OnInitDialog()");


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCustomRecordActionDlg::OnOK() 
{
	try {
		//Delete any existing records.
		ExecuteSql("UPDATE EmrActionsT SET Deleted = 1 WHERE SourceType = %li AND SourceID = %li", m_SourceType, m_nSourceID);

		//Loop through CPT Codes, add.
		long p = m_pCptList->GetFirstRowEnum();
		LPDISPATCH pDisp = NULL;
		
		while (p) {
			m_pCptList->GetNextRowEnum(&p, &pDisp);
			IRowSettingsPtr pRow(pDisp);
			pDisp->Release();
			//DRT 1/18/2007 - PLID 24181 - Custom records do not have the ability to use the new charge-action data, so
			//	just leave it at the defaults.  But the records must exist.
			CString strSql;
			strSql += FormatString("INSERT INTO EmrActionsT (SourceType, SourceID, DestType, DestID) VALUES (%li, %li, %li, %li);\r\n",
				m_SourceType, m_nSourceID, eaoCpt, VarLong(pRow->GetValue(0)));
			strSql += FormatString("INSERT INTO EmrActionChargeDataT (ActionID) (SELECT @@identity);");
			ExecuteSqlStd(strSql);
		}

		//Loop through Diag Codes, add.
		p = m_pDiagList->GetFirstRowEnum();
		pDisp = NULL;
		while (p) {
			m_pDiagList->GetNextRowEnum(&p, &pDisp);
			pDisp->Release();
			IRowSettingsPtr pRow(pDisp);
			// (b.savon 2014-07-14 13:18) - PLID 62706 - Deprecate the old Diag DestType and handle the changes Practice wide
			ExecuteSql(
				"INSERT INTO EmrActionsT (SourceType, SourceID, DestType, DestID) VALUES (%li, %li, %li, %li) "
				"DECLARE @EmrActionID INT; SET @EmrActionID = (SELECT @@IDENTITY); "
				"INSERT INTO EmrActionDiagnosisDataT (EmrActionID, DiagCodeID_ICD9, DiagCodeID_ICD10) VALUES (@EmrActionID, %li, NULL) ",
				m_SourceType, m_nSourceID, eaoDiagnosis/*eaoDiag*/, -1, VarLong(pRow->GetValue(0)));
		}

		CDialog::OnOK();
	}NxCatchAll("Error in CEmrActionDlg::OnOK()");
}

void CCustomRecordActionDlg::OnCancel() 
{
	CDialog::OnCancel();
}

BEGIN_EVENTSINK_MAP(CCustomRecordActionDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEmrActionDlg)
	ON_EVENT(CCustomRecordActionDlg, IDC_ACTION_CPT_COMBO, 16 /* SelChosen */, OnSelChosenActionCptCombo, VTS_I4)
	ON_EVENT(CCustomRecordActionDlg, IDC_ACTION_DIAG_COMBO, 16 /* SelChosen */, OnSelChosenActionDiagCombo, VTS_I4)
	ON_EVENT(CCustomRecordActionDlg, IDC_ACTION_CPT_LIST, 7 /* RButtonUp */, OnRButtonUpActionCptList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CCustomRecordActionDlg, IDC_ACTION_DIAG_LIST, 7 /* RButtonUp */, OnRButtonUpActionDiagList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CCustomRecordActionDlg::OnSelChosenActionCptCombo(long nRow) 
{
	if(nRow != -1 && m_pCptList->FindByColumn(0, m_pCptCombo->GetValue(nRow,0), 0, true) == -1)
		m_pCptList->AddRow(m_pCptCombo->GetRow(nRow));

	//now clear out the selection
	m_pCptCombo->CurSel = -1;
}

void CCustomRecordActionDlg::OnSelChosenActionDiagCombo(long nRow) 
{
	if(nRow != -1 && m_pDiagList->FindByColumn(0, m_pDiagCombo->GetValue(nRow,0), 0, true) == -1)
		m_pDiagList->AddRow(m_pDiagCombo->GetRow(nRow));

	//now clear out the selection
	m_pDiagCombo->CurSel = -1;
}

void CCustomRecordActionDlg::OnRButtonUpActionCptList(long nRow, short nCol, long x, long y, long nFlags) 
{
	if(nRow != -1) {
		m_pCptList->CurSel = nRow;
		CMenu mPopup;
		mPopup.CreatePopupMenu();
		mPopup.InsertMenu(0, MF_BYPOSITION, ID_REMOVE_CPT, "Remove");

		CPoint pt;
		GetCursorPos(&pt);
		mPopup.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);
	}	
}

void CCustomRecordActionDlg::OnRButtonUpActionDiagList(long nRow, short nCol, long x, long y, long nFlags) 
{
	if(nRow != -1) {
		m_pDiagList->CurSel = nRow;
		CMenu mPopup;
		mPopup.CreatePopupMenu();
		mPopup.InsertMenu(0, MF_BYPOSITION, ID_REMOVE_DIAG, "Remove");

		CPoint pt;
		GetCursorPos(&pt);
		mPopup.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);
	}
}

void CCustomRecordActionDlg::OnRemoveCpt()
{
	m_pCptList->RemoveRow(m_pCptList->CurSel);
}

void CCustomRecordActionDlg::OnRemoveDiag()
{
	m_pDiagList->RemoveRow(m_pDiagList->CurSel);
}
