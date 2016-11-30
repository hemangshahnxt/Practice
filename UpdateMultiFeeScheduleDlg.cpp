// UpdateMultiFeeScheduleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "UpdateMultiFeeScheduleDlg.h"
#include "AuditTrail.h"

using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37026 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



/////////////////////////////////////////////////////////////////////////////
// CUpdateMultiFeeScheduleDlg dialog


CUpdateMultiFeeScheduleDlg::CUpdateMultiFeeScheduleDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CUpdateMultiFeeScheduleDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUpdateMultiFeeScheduleDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_nCurrentID = -1;
	m_nBasedOnID = -1;
	m_strCurrentSchedule = "";
	m_nPercent = 100;
	m_bUpdatingProducts = false;
}


void CUpdateMultiFeeScheduleDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUpdateMultiFeeScheduleDlg)
	DDX_Control(pDX, IDC_RADIO_FEES_ONLY, m_radioFeesOnly);
	DDX_Control(pDX, IDC_RADIO_ALLOWABLES_ONLY, m_radioAllowablesOnly);
	DDX_Control(pDX, IDC_RADIO_UPDATE_BOTH, m_radioUpdateBoth);
	DDX_Control(pDX, IDC_PERCENTAGE, m_nxeditPercentage);
	DDX_Control(pDX, IDC_FEE_NAME, m_nxstaticFeeName);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUpdateMultiFeeScheduleDlg, CNxDialog)
	//{{AFX_MSG_MAP(CUpdateMultiFeeScheduleDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUpdateMultiFeeScheduleDlg message handlers

BOOL CUpdateMultiFeeScheduleDlg::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();

		m_listGroups = BindNxDataListCtrl(this, IDC_SCHEDULE_LIST, GetRemoteData(), true);
		
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// (j.jones 2013-04-11 16:09) - PLID 56221 - update the window text to make it clear
		// we're updating only for services or products, never both at the same time
		// (d.lange 2015-10-15 11:42) - PLID 67117 - Renamed to fee schedule
		if(m_bUpdatingProducts) {
			SetWindowText("Update Fee Schedule for Inventory Items");
		}
		else {
			SetWindowText("Update Fee Schedule for Service Codes");
		}


		// (j.jones 2008-09-02 17:17) - PLID 24064 - load up the last setting for the fees/allowables update
		long nUpdateMultiFees = GetRemotePropertyInt("UpdateMultiFees", 0, 0, GetCurrentUserName(), true);
		//0 - fees only, 1 - allowables only, 2 - both
		if(nUpdateMultiFees == 2) {	//both
			m_radioUpdateBoth.SetCheck(TRUE);
		}
		else if(nUpdateMultiFees == 1) {	//allowables only
			m_radioAllowablesOnly.SetCheck(TRUE);
		}
		else {	//fees only
			m_radioFeesOnly.SetCheck(TRUE);
		}

		// (j.jones 2007-02-27 08:59) - PLID 24116 - add row for "standard fees"
		NXDATALISTLib::IRowSettingsPtr pRow = m_listGroups->GetRow(-1);
		pRow->PutValue(0, (long)-2);
		pRow->PutValue(1, _bstr_t(" <Standard Fees>"));
		m_listGroups->AddRow(pRow);

		//set the static caption to the m_strCurrentSchedule
		SetDlgItemText(IDC_FEE_NAME, m_strCurrentSchedule);
		//fill in the default %
		SetDlgItemInt(IDC_PERCENTAGE, m_nPercent);

		//try setting the default group to the current one
		m_listGroups->TrySetSelByColumn(0, long(m_nCurrentID));


	}NxCatchAll("Error in CUpdateMultiFeeScheduleDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CUpdateMultiFeeScheduleDlg::OnOK() 
{
	try {
		long nCurSel = m_listGroups->GetCurSel();
		if(nCurSel == -1) {
			MsgBox("Please select a fee schedule.");
			return;
		}

		//must have a percent > 0 and < 1001 (for safety)
		m_nPercent = GetDlgItemInt(IDC_PERCENTAGE);

		if(m_nPercent == 0) {
			//Josh and I (DRT) decided to disallow this option.  It is safer to require they delete the group
			//and re-create it to achieve $0 status than to give the option where someone might accidentally
			//wipe everything out.
			MsgBox("You cannot update to a percentage of 0.  Please enter a non-zero percentage and try again.");
			return;
		}

		if(m_nPercent < 0) {
			MsgBox("You cannot update a percentage to a negative value.  Please enter a positive percentage and try again.");
			return;
		}

		if(m_nPercent > 1000) {
			MsgBox("You cannot automatically update to a percentage > 1000.  If you really need to update to a number that high, update by 1000, "
				"and then run another update.");
			return;
		}

		// (j.jones 2008-09-02 17:21) - PLID 24064 - check the setting for the fees/allowables update
		long nUpdateMultiFees = 0;
		CString strUpdateType = m_bUpdatingProducts ? "prices" : "fees";
		//0 - fees only, 1 - allowables only, 2 - both
		if(m_radioUpdateBoth.GetCheck()) {
			nUpdateMultiFees = 2;
			strUpdateType = m_bUpdatingProducts ? "prices and allowables" : "fees and allowables";
		}
		else if(m_radioAllowablesOnly.GetCheck()) {
			nUpdateMultiFees = 1;
			strUpdateType = "allowables";
		}

		//fill in the BasedOnID.  This will be used for comparisons and the update
		// (j.jones 2007-02-27 09:00) - PLID 24116 - may be -2 for Standard Fees
		m_nBasedOnID = VarLong(m_listGroups->GetValue(nCurSel, 0), -1);

		//if we are updating from the standard fees, but have a setting that updates allowables,
		//warn that only fees can be updated
		if(m_nBasedOnID == -2) {
			if(nUpdateMultiFees == 2) {
				if(MessageBox("Updating a fee schedule from the standard fees cannot update allowables. "
					"If you continue, only the new fees will be updated. Allowables will be unchanged.\n\n"
					"Are you sure you wish to continue?",
					"Practice", MB_ICONQUESTION|MB_YESNO) == IDNO) {
					return;
				}
			}
			else if(nUpdateMultiFees == 1) {
				AfxMessageBox("Updating a fee schedule from the standard fees cannot update allowables. Please select 'Update Fees' instead, or cancel.");
				return;
			}
		}

		//since this is a bit confusing, we need to setup a warning message that tells them EXACTLY 
		//(in english) what is going
		CString strMsg;

		if(m_nBasedOnID == m_nCurrentID) {
			//We are updating ourself.  Give an appropriate message
			// (j.jones 2013-04-11 16:09) - PLID 56221 - make it clear whether we're for services
			// or for products, we would never update both at the same time
			if(m_bUpdatingProducts) {
				strMsg.Format("You are about to update the %s for the inventory items in the fee schedule '%s'. "
					"All product %s in this schedule will be updated to %li percent of their previous value. "
					"For example, a $100 amount will be updated to $%li.\n\n"
					"Are you sure you wish to do this?",
					strUpdateType, m_strCurrentSchedule, strUpdateType, m_nPercent, m_nPercent);
			}
			else {
				strMsg.Format("You are about to update the %s for the service codes in the fee schedule '%s'. "
					"All service code %s in this schedule will be updated to %li percent of their previous value. "
					"For example, a $100 amount will be updated to $%li.\n\n"
					"Are you sure you wish to do this?",
					strUpdateType, m_strCurrentSchedule, strUpdateType, m_nPercent, m_nPercent);
			}
		}
		else if(m_nBasedOnID == -2) {
			// (j.jones 2007-02-27 09:01) - PLID 24116 - tweak the message to say we are updating from the standard fees
			//We are updating ourself.  Give an appropriate message
			// (j.jones 2013-04-11 16:09) - PLID 56221 - make it clear whether we're for services
			// or for products, we would never update both at the same time
			if(m_bUpdatingProducts) {
				strMsg.Format("You are about to update the inventory items in the fee schedule '%s'. "
					"All product prices in this schedule will be updated by copying the standard prices and %s "
					"the prices to %li percent of their original value. "
					"For example, a $100 amount will be updated to $%li.\n\n"
					"Are you sure you wish to do this?", 
					m_strCurrentSchedule, ((m_nPercent >= 100) ? "increasing" : "decreasing"), m_nPercent, m_nPercent);
			}
			else {
				strMsg.Format("You are about to update the service codes in the fee schedule '%s'. "
					"All service code fees in this schedule will be updated by copying the standard fees and %s "
					"the fees to %li percent of their original value. "
					"For example, a $100 amount will be updated to $%li.\n\n"
					"Are you sure you wish to do this?", 
					m_strCurrentSchedule, ((m_nPercent >= 100) ? "increasing" : "decreasing"), m_nPercent, m_nPercent);
			}
		}
		else {
			//normal from-fee-schedule message
			// (j.jones 2013-04-11 16:09) - PLID 56221 - make it clear whether we're for services
			// or for products, we would never update both at the same time
			if(m_bUpdatingProducts) {
				strMsg.Format("You are about to update the %s for the inventory items in the fee schedule '%s'. "
					"All product %s in this schedule will be updated by copying the data from the schedule '%s' and %s "
					"the amounts to %li percent of their original value. "
					"For example, a $100 amount will be updated to $%li.\n\n"
					"Are you sure you wish to do this?", 
					strUpdateType, m_strCurrentSchedule, strUpdateType,
					VarString(m_listGroups->GetValue(nCurSel, 1), ""), ((m_nPercent >= 100) ? "increasing" : "decreasing"),
					m_nPercent, m_nPercent);
			}
			else {
				strMsg.Format("You are about to update the %s for the service codes in the fee schedule '%s'. "
					"All service code %s in this schedule will be updated by copying the data from the schedule '%s' and %s "
					"the amounts to %li percent of their original value. "
					"For example, a $100 amount will be updated to $%li.\n\n"
					"Are you sure you wish to do this?", 
					strUpdateType, m_strCurrentSchedule, strUpdateType,
					VarString(m_listGroups->GetValue(nCurSel, 1), ""), ((m_nPercent >= 100) ? "increasing" : "decreasing"),
					m_nPercent, m_nPercent);
			}
		}

		// (j.jones 2009-02-16 09:13) - PLID 32698 - converted from MsgBox to MessageBox so it wouldn't try
		// to process % tokens
		if(MessageBox(strMsg, "Practice", MB_ICONQUESTION|MB_YESNO) == IDNO) {
			return;
		}

		// (j.jones 2013-04-11 16:09) - PLID 56221 - Every insert or update we do will
		// inner join on ServiceT and either CPTCodeT or ProductT. Define that table now.
		CString strInnerJoin = "INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID ";
		if(m_bUpdatingProducts) {
			strInnerJoin = "INNER JOIN ProductT ON ServiceT.ID = ProductT.ID ";
		}

		//Now we're through the message nonsense, update our table with the new, correct values
		if(m_nBasedOnID == m_nCurrentID) {
			//we are updating our own items - very simple query
			CString sql;

			// (j.jones 2008-09-02 17:31) - PLID 24064 - update Price, Allowable, or both, based on our setting
			if(nUpdateMultiFees == 2) { //fees & allowables
				sql.Format("UPDATE MultiFeeItemsT SET "
					"MultiFeeItemsT.Price = Round(convert(money, (MultiFeeItemsT.Price * %g)), 2), "
					"MultiFeeItemsT.Allowable = Round(convert(money, (MultiFeeItemsT.Allowable * %g)), 2) "
					"FROM MultiFeeItemsT "
					"INNER JOIN ServiceT ON MultiFeeItemsT.ServiceID = ServiceT.ID "
					"%s "
					"WHERE MultiFeeItemsT.FeeGroupID = %li",
					double(m_nPercent)/100.0, double(m_nPercent)/100.0,
					strInnerJoin, m_nCurrentID);
			}
			else if(nUpdateMultiFees == 1) { //allowables only
				sql.Format("UPDATE MultiFeeItemsT SET MultiFeeItemsT.Allowable = Round(convert(money, (MultiFeeItemsT.Allowable * %g)), 2) "
					"FROM MultiFeeItemsT "
					"INNER JOIN ServiceT ON MultiFeeItemsT.ServiceID = ServiceT.ID "
					"%s "
					"WHERE MultiFeeItemsT.FeeGroupID = %li",
					double(m_nPercent)/100.0,
					strInnerJoin, m_nCurrentID);
			}
			else { //fees only
				sql.Format("UPDATE MultiFeeItemsT SET MultiFeeItemsT.Price = Round(convert(money, (MultiFeeItemsT.Price * %g)), 2) "
					"FROM MultiFeeItemsT "
					"INNER JOIN ServiceT ON MultiFeeItemsT.ServiceID = ServiceT.ID "
					"%s "
					"WHERE MultiFeeItemsT.FeeGroupID = %li", double(m_nPercent)/100.0,
					strInnerJoin, m_nCurrentID);
			}

			ExecuteSql("%s", sql);
		}
		else if(m_nBasedOnID == -2) {
			// (j.jones 2007-02-27 09:09) - PLID 24116 - supported updating from standard fees
			CString strSqlBatch;
			//1)  Find all services that do not exist in m_nCurrent.  These are items that currently have no fees, but
			//		should have fees when we're done.  For these we want to insert into the m_nCurrent group.
			AddStatementToSqlBatch(strSqlBatch, "INSERT INTO MultiFeeItemsT (FeeGroupID, ServiceID, Price) "
				"(SELECT %li, ServiceT.ID, Round(convert(money, (Price * %g)), 2) AS Price "
				"FROM ServiceT "
				"%s "
				"WHERE ServiceT.ID NOT IN (SELECT ServiceID FROM MultiFeeItemsT WHERE FeeGroupID = %li))", 
				m_nCurrentID, double(m_nPercent)/100.0,
				strInnerJoin, m_nCurrentID);

			//2)  Find all items that exist in m_nCurrent. These are items that currently have a price (presumably), but will be updated
			//		with the m_nBasedOn group values.
			AddStatementToSqlBatch(strSqlBatch, "UPDATE MultiFeeItemsT SET MultiFeeItemsT.Price = Round(convert(money, ("
				"ServiceT.Price * %g)), 2) "
				"FROM MultiFeeItemsT "
				"INNER JOIN ServiceT ON MultiFeeItemsT.ServiceID = ServiceT.ID "
				"%s "
				"WHERE MultiFeeItemsT.FeeGroupID = %li", 
				double(m_nPercent)/100.0,
				strInnerJoin, m_nCurrentID);

			ExecuteSqlBatch(strSqlBatch);
		}
		else {

			// (j.jones 2008-09-02 17:31) - PLID 24064 - update Price, Allowable, or both, based on our setting
			CString strSqlBatch;

			if(nUpdateMultiFees == 2) { //fees & allowables

				//1)  Find all items in m_nCurrent that do not exist in m_nBasedOnID, and delete them entirely.
				AddStatementToSqlBatch(strSqlBatch, "DELETE MultiFeeItemsT "
					"FROM MultiFeeItemsT "
					"INNER JOIN ServiceT ON MultiFeeItemsT.ServiceID = ServiceT.ID "
					"%s "
					"WHERE MultiFeeItemsT.FeeGroupID = %li", strInnerJoin, m_nCurrentID);

				//2)  Find all items that exist in m_nBasedOnID. For these we want to insert into the m_nCurrent group.
				AddStatementToSqlBatch(strSqlBatch, "INSERT INTO MultiFeeItemsT (FeeGroupID, ServiceID, Price, Allowable) "
					"SELECT %li, MultiFeeItemsT.ServiceID, Round(convert(money, (MultiFeeItemsT.Price * %g)), 2) AS Price, "
					"Round(convert(money, (MultiFeeItemsT.Allowable * %g)), 2) AS Allowable "
					"FROM MultiFeeItemsT "
					"INNER JOIN ServiceT ON MultiFeeItemsT.ServiceID = ServiceT.ID "
					"%s "
					"WHERE MultiFeeItemsT.FeeGroupID = %li", 
					m_nCurrentID, double(m_nPercent)/100.0, double(m_nPercent)/100.0,
					strInnerJoin, m_nBasedOnID);

				//3)  Find all items that exist in both m_nCurrent and m_nBasedOn.  These are items that currently have a price, but will be updated
				//		with the m_nBasedOn group values.
				//a) fees
				AddStatementToSqlBatch(strSqlBatch, "UPDATE MultiFeeItemsT SET MultiFeeItemsT.Price = Round(convert(money, ("
					"(SELECT MFIT.Price FROM MultiFeeItemsT MFIT WHERE MFIT.ServiceID = MultiFeeItemsT.ServiceID AND MFIT.FeeGroupID = %li) * %g)), 2) "
					"FROM MultiFeeItemsT "
					"INNER JOIN ServiceT ON MultiFeeItemsT.ServiceID = ServiceT.ID "
					"%s "
					"WHERE MultiFeeItemsT.FeeGroupID = %li AND MultiFeeItemsT.ServiceID IN ("
					"SELECT MultiFeeItemsT.ServiceID FROM MultiFeeItemsT WHERE MultiFeeItemsT.FeeGroupID = %li AND MultiFeeItemsT.Price IS NOT NULL)", 
					m_nBasedOnID, double(m_nPercent)/100.0,
					strInnerJoin,
					m_nCurrentID, m_nBasedOnID);
				//b) allowables
				AddStatementToSqlBatch(strSqlBatch, "UPDATE MultiFeeItemsT SET MultiFeeItemsT.Allowable = Round(convert(money, ("
					"(SELECT MFIT.Allowable FROM MultiFeeItemsT MFIT WHERE MFIT.ServiceID = MultiFeeItemsT.ServiceID AND MFIT.FeeGroupID = %li) * %g)), 2) "
					"FROM MultiFeeItemsT "
					"INNER JOIN ServiceT ON MultiFeeItemsT.ServiceID = ServiceT.ID "
					"%s "
					"WHERE MultiFeeItemsT.FeeGroupID = %li AND MultiFeeItemsT.ServiceID IN ("
					"SELECT MultiFeeItemsT.ServiceID FROM MultiFeeItemsT WHERE MultiFeeItemsT.FeeGroupID = %li AND MultiFeeItemsT.Allowable IS NOT NULL)", 
					m_nBasedOnID, double(m_nPercent)/100.0,
					strInnerJoin,
					m_nCurrentID, m_nBasedOnID);
			}
			else if(nUpdateMultiFees == 1) { //allowables only

				//1)  Find all items in m_nCurrent that do not exist in m_nBasedOnID.  These are items that currently have an allowable, but should not
				//		have an allowable when we're finished.  Update the allowable to NULL if they have a fee, delete entirely otherwise.
				//a)  Update Allowable to NULL
				AddStatementToSqlBatch(strSqlBatch, "UPDATE MultiFeeItemsT SET MultiFeeItemsT.Allowable = NULL "
					"FROM MultiFeeItemsT "
					"INNER JOIN ServiceT ON MultiFeeItemsT.ServiceID = ServiceT.ID "
					"%s "
					"WHERE MultiFeeItemsT.FeeGroupID = %li AND MultiFeeItemsT.Price IS NOT NULL AND MultiFeeItemsT.ServiceID IN ("
					"SELECT MultiFeeItemsT.ServiceID FROM MultiFeeItemsT WHERE MultiFeeItemsT.FeeGroupID = %li AND MultiFeeItemsT.ServiceID NOT IN "
					"(SELECT MultiFeeItemsT.ServiceID FROM MultiFeeItemsT WHERE MultiFeeItemsT.FeeGroupID = %li))",
					strInnerJoin,
					m_nCurrentID, m_nCurrentID, m_nBasedOnID);

				//b)  Delete records with no prices
				AddStatementToSqlBatch(strSqlBatch, "DELETE MultiFeeItemsT "
					"FROM MultiFeeItemsT "
					"INNER JOIN ServiceT ON MultiFeeItemsT.ServiceID = ServiceT.ID "
					"%s "
					"WHERE MultiFeeItemsT.FeeGroupID = %li AND MultiFeeItemsT.Price IS NULL AND MultiFeeItemsT.ServiceID IN ("
					"SELECT MultiFeeItemsT.ServiceID FROM MultiFeeItemsT WHERE MultiFeeItemsT.FeeGroupID = %li AND MultiFeeItemsT.ServiceID NOT IN "
					"(SELECT MultiFeeItemsT.ServiceID FROM MultiFeeItemsT WHERE MultiFeeItemsT.FeeGroupID = %li))",
					strInnerJoin,
					m_nCurrentID, m_nCurrentID, m_nBasedOnID);

				//2)  Find all items that do not exist in m_nCurrent but do exist in m_nBasedOnID.  These are items that currently have no allowables, but
				//		should have allowables when we're done.  For these we want to insert into the m_nCurrent group.
				AddStatementToSqlBatch(strSqlBatch, "INSERT INTO MultiFeeItemsT (FeeGroupID, ServiceID, Allowable) "
					"(SELECT %li, MultiFeeItemsT.ServiceID, Round(convert(money, (MultiFeeItemsT.Allowable * %g)), 2) AS Allowable "
					"FROM MultiFeeItemsT "
					"INNER JOIN ServiceT ON MultiFeeItemsT.ServiceID = ServiceT.ID "
					"%s "
					"WHERE MultiFeeItemsT.FeeGroupID = %li AND MultiFeeItemsT.ServiceID IN ( "
					"SELECT MultiFeeItemsT.ServiceID FROM MultiFeeItemsT WHERE MultiFeeItemsT.FeeGroupID = %li AND MultiFeeItemsT.ServiceID NOT IN ( "
					"SELECT MultiFeeItemsT.ServiceID FROM MultiFeeItemsT WHERE MultiFeeItemsT.FeeGroupID = %li)))", 
					m_nCurrentID, double(m_nPercent)/100.0,
					strInnerJoin,
					m_nBasedOnID, m_nBasedOnID, m_nCurrentID);

				//3)  Find all items that exist in both m_nCurrent and m_nBasedOn.  These are items that currently have an allowable, but will be updated
				//		with the m_nBasedOn group values.
				AddStatementToSqlBatch(strSqlBatch, "UPDATE MultiFeeItemsT SET MultiFeeItemsT.Allowable = Round(convert(money, ("
					"(SELECT MFIT.Allowable FROM MultiFeeItemsT MFIT WHERE MFIT.ServiceID = MultiFeeItemsT.ServiceID AND MFIT.FeeGroupID = %li) * %g)), 2) "
					"FROM MultiFeeItemsT "
					"INNER JOIN ServiceT ON MultiFeeItemsT.ServiceID = ServiceT.ID "
					"%s "
					"WHERE MultiFeeItemsT.FeeGroupID = %li AND MultiFeeItemsT.ServiceID IN ("
					"SELECT MultiFeeItemsT.ServiceID FROM MultiFeeItemsT WHERE MultiFeeItemsT.FeeGroupID = %li AND MultiFeeItemsT.Allowable IS NOT NULL)", 
					m_nBasedOnID, double(m_nPercent)/100.0,
					strInnerJoin,
					m_nCurrentID, m_nBasedOnID);
			}
			else { //fees only

				//1)  Find all items in m_nCurrent that do not exist in m_nBasedOnID.  These are items that currently have a fee, but should not
				//		have a fee when we're finished.  Update the price to NULL if they have an allowable, delete entirely otherwise.
				//a)  Update Price to NULL
				AddStatementToSqlBatch(strSqlBatch, "UPDATE MultiFeeItemsT SET MultiFeeItemsT.Price = NULL "
					"FROM MultiFeeItemsT "
					"INNER JOIN ServiceT ON MultiFeeItemsT.ServiceID = ServiceT.ID "
					"%s "
					"WHERE MultiFeeItemsT.FeeGroupID = %li AND MultiFeeItemsT.Allowable IS NOT NULL AND MultiFeeItemsT.ServiceID IN ("
					"SELECT MultiFeeItemsT.ServiceID FROM MultiFeeItemsT WHERE MultiFeeItemsT.FeeGroupID = %li AND MultiFeeItemsT.ServiceID NOT IN "
					"(SELECT MultiFeeItemsT.ServiceID FROM MultiFeeItemsT WHERE MultiFeeItemsT.FeeGroupID = %li))",
					strInnerJoin,
					m_nCurrentID, m_nCurrentID, m_nBasedOnID);

				//b)  Delete records with no allowables
				AddStatementToSqlBatch(strSqlBatch, "DELETE FROM MultiFeeItemsT "
					"FROM MultiFeeItemsT "
					"INNER JOIN ServiceT ON MultiFeeItemsT.ServiceID = ServiceT.ID "
					"%s "
					"WHERE MultiFeeItemsT.FeeGroupID = %li AND MultiFeeItemsT.Allowable IS NULL AND MultiFeeItemsT.ServiceID IN ("
					"SELECT MultiFeeItemsT.ServiceID FROM MultiFeeItemsT WHERE MultiFeeItemsT.FeeGroupID = %li AND MultiFeeItemsT.ServiceID NOT IN "
					"(SELECT MultiFeeItemsT.ServiceID FROM MultiFeeItemsT WHERE MultiFeeItemsT.FeeGroupID = %li))",
					strInnerJoin,
					m_nCurrentID, m_nCurrentID, m_nBasedOnID);

				//2)  Find all items that do not exist in m_nCurrent but do exist in m_nBasedOnID.  These are items that currently have no fees, but
				//		should have fees when we're done.  For these we want to insert into the m_nCurrent group.
				AddStatementToSqlBatch(strSqlBatch, "INSERT INTO MultiFeeItemsT (FeeGroupID, ServiceID, Price) "
					"(SELECT %li, MultiFeeItemsT.ServiceID, Round(convert(money, (MultiFeeItemsT.Price * %g)), 2) AS Price "
					"FROM MultiFeeItemsT "
					"INNER JOIN ServiceT ON MultiFeeItemsT.ServiceID = ServiceT.ID "
					"%s "
					"WHERE MultiFeeItemsT.FeeGroupID = %li AND MultiFeeItemsT.ServiceID IN ( "
					"SELECT MultiFeeItemsT.ServiceID FROM MultiFeeItemsT WHERE MultiFeeItemsT.FeeGroupID = %li AND MultiFeeItemsT.ServiceID NOT IN ( "
					"SELECT MultiFeeItemsT.ServiceID FROM MultiFeeItemsT WHERE MultiFeeItemsT.FeeGroupID = %li)))", 
					m_nCurrentID, double(m_nPercent)/100.0,
					strInnerJoin,
					m_nBasedOnID, m_nBasedOnID, m_nCurrentID);

				//3)  Find all items that exist in both m_nCurrent and m_nBasedOn.  These are items that currently have a price, but will be updated
				//		with the m_nBasedOn group values.
				AddStatementToSqlBatch(strSqlBatch, "UPDATE MultiFeeItemsT SET MultiFeeItemsT.Price = Round(convert(money, ("
					"(SELECT MFIT.Price FROM MultiFeeItemsT MFIT WHERE MFIT.ServiceID = MultiFeeItemsT.ServiceID AND MultiFeeItemsT.FeeGroupID = %li) * %g)), 2) "
					"FROM MultiFeeItemsT "
					"INNER JOIN ServiceT ON MultiFeeItemsT.ServiceID = ServiceT.ID "
					"%s "
					"WHERE MultiFeeItemsT.FeeGroupID = %li AND MultiFeeItemsT.ServiceID IN ("
					"SELECT MultiFeeItemsT.ServiceID FROM MultiFeeItemsT WHERE MultiFeeItemsT.FeeGroupID = %li AND MultiFeeItemsT.Price IS NOT NULL)", 
					m_nBasedOnID, double(m_nPercent)/100.0,
					strInnerJoin,
					m_nCurrentID, m_nBasedOnID);
			}

			ExecuteSqlBatch(strSqlBatch);
		}

		//for auditing
		// (j.jones 2008-09-02 17:51) - PLID 24064 - updated the audit description to track whether we
		// updated the Price, Allowable, or both
		// (j.jones 2013-04-11 16:29) - PLID 56221 - added indicator of whether they updated services or products
		CString strNew;
		if(m_nBasedOnID == m_nCurrentID) {
			strNew.Format("Updated %s %s for group '%s' to %li percent of its previous value.", m_bUpdatingProducts ? "inventory item" : "service code", strUpdateType, m_strCurrentSchedule, m_nPercent);
		}
		else {
			strNew.Format("Updated %s %s group '%s' to %li percent of the group '%s's' value.", m_bUpdatingProducts ? "inventory item" : "service code", strUpdateType, m_strCurrentSchedule, m_nPercent, VarString(m_listGroups->GetValue(nCurSel, 1), ""));
		}

		long nAuditID = BeginNewAuditEvent();
		AuditEvent(-1, "", nAuditID, aeiUpdateMultiFees, -1, "", strNew, aepHigh, aetChanged);

		// (j.jones 2008-09-02 17:20) - PLID 24064 - save the setting for the fees/allowables update
		SetRemotePropertyInt("UpdateMultiFees", nUpdateMultiFees, 0, GetCurrentUserName());

	} NxCatchAll("Error in CUpdateMultiFeeScheduleDlg::OnOK()");

	CDialog::OnOK();
}

void CUpdateMultiFeeScheduleDlg::OnCancel() 
{
	//cancel, do nothing

	CDialog::OnCancel();
}
