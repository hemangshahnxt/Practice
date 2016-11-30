// SuperBillApplyPromptDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SuperBillApplyPromptDlg.h"
#include "DateTimeUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CSuperBillApplyPromptDlg dialog


CSuperBillApplyPromptDlg::CSuperBillApplyPromptDlg(CWnd* pParent)
	: CNxDialog(CSuperBillApplyPromptDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSuperBillApplyPromptDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSuperBillApplyPromptDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSuperBillApplyPromptDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_INCLUDE_APPLIES, m_checkIncludeApplies);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSuperBillApplyPromptDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSuperBillApplyPromptDlg)
	ON_BN_CLICKED(IDC_INCLUDE_APPLIES, OnIncludeApplies)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSuperBillApplyPromptDlg message handlers

BOOL CSuperBillApplyPromptDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	m_nFinalID = -1;	//none chosen

	try {
		// (c.haag 2008-05-02 10:11) - PLID 29879 - NxIconify the Close button
		m_btnOK.AutoSet(NXB_CLOSE);
		m_listIDs = BindNxDataListCtrl(IDC_SUPERBILL_LIST, false);

	} NxCatchAll("Error initializing superbill list");

	//OnIncludeApplies handles requerying and determining the WHERE clause anyways, so just call it
	OnIncludeApplies();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSuperBillApplyPromptDlg::OnIncludeApplies() 
{

	try {
		//save the current selection
		long nCurrent = -1;
		if(m_listIDs->GetCurSel() != -1)
			nCurrent = m_listIDs->GetValue(m_listIDs->GetCurSel(), 0);

		//change the where clause and requery
		CString sql;

		if(IsDlgButtonChecked(IDC_INCLUDE_APPLIES)) {
			//all superbills
			sql.Format("PrintedSuperbillsT.PatientID = %li AND PrintedSuperbillsT.Void = 0", m_nPatientID);
		}
		else {
			//don't include already-applied superbill ids
			//DRT 3/24/2004 - PLID 11586 - Added a filter to the IN clause to filter for just the current patient, no sense selecting a bunch of records that cannot apply here.
			sql.Format("PrintedSuperbillsT.PatientID = %li AND PrintedSuperbillsT.Void = 0 AND SavedID NOT IN (SELECT SuperbillID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE SuperbillID IS NOT NULL AND LineItemT.Deleted = 0 AND LineItemT.Type = 10 AND LineItemT.PatientID = %li)", m_nPatientID, m_nPatientID);
		}

		//add in the date filter if that property is set
		CString sqlDate;
		if(GetRemotePropertyInt("ApplySuperbillUseDate",0,0,"<None>",true)==1) {	//default to off
			//they are using a date
			// (j.jones 2009-01-06 15:16) - PLID 32215 - the default is now the current date
			COleDateTime dtNow = COleDateTime::GetCurrentTime();
			COleDateTime dt = GetRemotePropertyDateTime("ApplySuperbillDate", &dtNow, 0, "<None>", false);

			sqlDate.Format(" AND PrintedSuperbillsT.PrintedOn >= '%s'", FormatDateTimeForSql(dt, dtoDate));
			sql += sqlDate;
		}

		m_listIDs->PutWhereClause(_bstr_t(sql));

		//and refresh the list
		m_listIDs->Requery();

		//and now insert a row of <None Selected>
		IRowSettingsPtr pRow;
		pRow = m_listIDs->GetRow(-1);

		pRow->PutValue(0, long(-1));
		pRow->PutValue(1, _bstr_t("<None Selected>"));

		//add the row to the list
		m_listIDs->AddRow(pRow);

		//reset the selection
		m_listIDs->TrySetSelByColumn(0, long(nCurrent));
	} NxCatchAll("Error Requerying the Superbill List");
}

void CSuperBillApplyPromptDlg::OnOK() 
{
	try {
		//DRT 3/24/2004 - PLID 11586 - This is sort of a workaround, but they've already dismissed
		//	the dialog, so we should not wait any longer on a requery to finish.
		if(m_listIDs->IsRequerying() == VARIANT_TRUE) {
			m_listIDs->CancelRequery();
		}

		if(m_listIDs->GetCurSel() == -1)
			m_nFinalID = -1;	//none chosen
		else
			m_nFinalID = m_listIDs->GetValue(m_listIDs->GetCurSel(), 0);

	} NxCatchAll("Error saving selection in OnOK()");

	CDialog::OnOK();
}

void CSuperBillApplyPromptDlg::OnCancel() 
{
	//there is no good way to cancel the billing dialog from opening (from the way we're firing the dialog right now) - so just pretend they hit ok
	//	this only fires if they hit the escape key - this is also questionable behavior (rather nonstandard), but right now it's the best I've got
	OnOK();
}

BEGIN_EVENTSINK_MAP(CSuperBillApplyPromptDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CSuperBillApplyPromptDlg)
	ON_EVENT(CSuperBillApplyPromptDlg, IDC_SUPERBILL_LIST, 18 /* RequeryFinished */, OnRequeryFinishedSuperbillList, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CSuperBillApplyPromptDlg::OnRequeryFinishedSuperbillList(short nFlags) 
{
	try {
		//DRT 6/27/2007 - PLID 25014 - If the preference is enabled, automatically attempt to select a superbill ID
		//	if there is only 1, and we don't already have something selected.
		//Note:  Bob and I discussed the datalist / TrySetSelFinished / RequeryFinished functionality here.  The behavior is:
		//	At the time that OnRequeryFinished is fired, TrySetSel will either a) Have successfully selected a value, if
		//	there is a possible value to select (returned success in the TrySetSelFinished handler), or b) failed to select
		//	a value.  In the case of (b), it is possible that the TrySetSelFinished message was not fired until AFTER
		//	the OnRequeryFinished message was fired.  However, nothing is being selected due to that message, so this
		//	code is safe to work with the list.
		if(GetRemotePropertyInt("ApplySuperbillAutoApplyIfOne", 1, 0, GetCurrentUserName(), true)) {
			//First get a row count.  If there aren't exactly 2 (<No Selection> and a legit value), then we can't do anything.
			if(m_listIDs->GetRowCount() == 2) {
				//See what is already selected.  If it's a real value, then we don't need to do anything.  If it's
				//	the "no selection" row, then we can continue with our auto-selection code.
				long nCurSel = m_listIDs->CurSel;
				long nID = -1;
				if(nCurSel != -1) {
					nID = VarLong(m_listIDs->GetValue(nCurSel, 0), -1);
				}

				if(nID != -1) {
					//There is a legitimate superbill ID already selected.  We don't want to change that
				}
				else {
					//No superbill is selected, and there is only 1 in the list.  Let's select it!
					for(int i = 0; i < 2; i++) {
						long nID = VarLong(m_listIDs->GetValue(i, 0), -1);
						if(nID != -1)
							m_listIDs->CurSel = i;
					}
				}
			}
			else {
				//There are more or less than exactly 2 rows.  Cannot do anything
			}
		}
		else {
			//Preference is disabled, they do not want to auto-select the ID
		}

	} NxCatchAll("Error in OnRequeryFinishedSuperbillList");
}
