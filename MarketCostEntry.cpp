#include "stdafx.h"

#include "MarketCostEntry.h"
#include "pracProps.h"
#include "marketutils.h"
#include "globalFinancialUtils.h"
#include "AuditTrail.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "SelectContactDlg.h"
#include "MarketingRc.h"
#include "multiselectdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;

using namespace NXDATALIST2Lib;

// (a.walling 2010-01-21 16:43) - PLID 37024 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.


/////////////////////////////////////////////////////////////////////////////
// CMarketCostEntry dialog


CMarketCostEntry::CMarketCostEntry(CWnd* pParent)
	: CNxDialog(CMarketCostEntry::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMarketCostEntry)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_bActive = false;
	m_bIsNew = false;

	m_pReferralSubDlg = NULL;
	m_rcMultiProcedureLabel = CRect(0,0,0,0);
}


void CMarketCostEntry::DoDataExchange(CDataExchange* pDX)
{
	// (z.manning, 05/12/2008) - PLID 29702 - Converted notes from rich text to edit control
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMarketCostEntry)
	DDX_Control(pDX, IDC_DELETE, m_deleteButton);
	DDX_Control(pDX, IDCANCEL, m_cancelButton);
	DDX_Control(pDX, IDOK, m_okButton);
	DDX_Control(pDX, IDC_DTPAID, m_dtPaid);
	DDX_Control(pDX, IDC_FROM, m_dtFrom);
	DDX_Control(pDX, IDC_TO, m_dtTo);
	DDX_Control(pDX, IDC_COST, m_nxeditCost);
	DDX_Control(pDX, IDC_RECIEVER, m_nxeditReciever);
	DDX_Control(pDX, IDC_NUMBER, m_nxeditNumber);
	DDX_Control(pDX, IDC_REFERRAL_AREA, m_btnReferralArea);
	DDX_Control(pDX, IDC_NOTES, m_nxeditNotes);
	DDX_Control(pDX, IDC_MULTI_PROCEDURE_EXP_LABEL, m_labelMultiProcedure);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMarketCostEntry, CNxDialog)
	//{{AFX_MSG_MAP(CMarketCostEntry)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_EN_KILLFOCUS(IDC_COST, OnKillfocusCost)
	ON_BN_CLICKED(IDC_SELECT_CONTACT, OnSelectContact)
	//}}AFX_MSG_MAP
	ON_WM_LBUTTONDOWN()
	ON_WM_PAINT()
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMarketCostEntry message handlers

BOOL CMarketCostEntry::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		m_okButton.AutoSet(NXB_OK);
		m_cancelButton.AutoSet(NXB_CANCEL);
		m_deleteButton.AutoSet(NXB_DELETE);

		// (c.haag 2009-02-16 09:15) - PLID 33100 - Added a procedure dropdown.
		// For consistency, the locations dropdown has been converted to a datalist 2.
		m_dlLocations = BindNxDataList2Ctrl(IDC_COST_LOCATION);
		m_dlProcedures = BindNxDataList2Ctrl(IDC_COMBO_EXP_PROCEDURES, false);

		// (c.haag 2009-02-16 09:55) - PLID 33100 - Get the multi-procedure label rectangle
		{
			CWnd* pWnd = GetDlgItem(IDC_MULTI_PROCEDURE_EXP_LABEL);
			if (IsWindow(pWnd->GetSafeHwnd())) {
				pWnd->GetWindowRect(m_rcMultiProcedureLabel);
				ScreenToClient(&m_rcMultiProcedureLabel);
				// Hide the static text that was there by default
				pWnd->ShowWindow(SW_HIDE);
			}
		}

		//DRT 4/21/2006 - PLID 20232 - Implemented new datalist2 based referral tree.
		{
			m_pReferralSubDlg = new CReferralSubDlg(this);
			m_pReferralSubDlg->Create(IDD_REFERRAL_SUBDIALOG, this);

			CRect rc;
			GetDlgItem(IDC_REFERRAL_AREA)->GetWindowRect(&rc);
			ScreenToClient(&rc);
			m_pReferralSubDlg->MoveWindow(rc);
		}

		if (m_id)
		{
			_RecordsetPtr	rs;
			EnsureRemoteData();

			try
			{
				// (c.haag 2009-02-16 10:41) - PLID 33100 - We now load from MarketingCostProcedureT
				// (a.wilson 2012-6-25) PLID 50378 - if the source is inactive then we need to check the show incative box.
				rs = CreateParamRecordset("SELECT Amount, PaidTo, RefNumber, Description, ReferralSourceQ.PersonID AS ReferralID, "
					"EffectiveFrom, EffectiveTo, DatePaid, LocationID, ReferralSourceQ.Archived "
					"FROM MarketingCostsT "
					"INNER JOIN (SELECT ReferralSourceT.*, PersonT.Archived FROM ReferralSourceT INNER JOIN PersonT ON PersonT.ID = ReferralSourceT.PersonID) "
					"ReferralSourceQ ON MarketingCostsT.ReferralSource = ReferralSourceQ.PersonID "
					"WHERE ID = {INT};\r\n"
					"SELECT ProcedureID FROM MarketingCostProcedureT "
					"INNER JOIN ProcedureT ON ProcedureT.ID = MarketingCostProcedureT.ProcedureID "
					"WHERE MarketingCostID = {INT} "
					"ORDER BY Name"
					,m_id, m_id);
				ASSERT(!rs->eof);
				if(!rs->eof) {
					COleCurrency cost = rs->Fields->Item["Amount"]->Value;
					RoundCurrency(cost);
					SetDlgItemText (IDC_COST, FormatCurrencyForInterface(cost));
					m_strOldAmt = FormatCurrencyForInterface(cost);
					SetDlgItemVar(IDC_RECIEVER, rs->Fields->Item["PaidTo"]->Value, true, true);
					SetDlgItemVar(IDC_NUMBER,	rs->Fields->Item["RefNumber"]->Value, true, true);
					SetDlgItemVar(IDC_NOTES,	rs->Fields->Item["Description"]->Value, true, true);
					m_dtFrom.SetValue(	rs->Fields->Item["EffectiveFrom"]->Value);
					m_dtTo.SetValue(	rs->Fields->Item["EffectiveTo"]->Value);
					m_dtPaid.SetValue(	rs->Fields->Item["DatePaid"]->Value);
					m_strOldDatePaid = FormatDateTimeForSql(COleDateTime(rs->Fields->Item["DatePaid"]->Value), dtoDate);

					long nLocation = AdoFldLong(rs, "LocationID", -1);
					if(nLocation != -1) {
						m_dlLocations->SetSelByColumn(0, nLocation);
					}

					//DRT 4/21/2006 - PLID 20232 - Select the referral on the new referral subdialog.
					// (a.wilson 2012-6-26) PLID 50378 - added defaulted param for handling inactives.
					long nReferralID = AdoFldLong(rs, "ReferralID");
					m_pReferralSubDlg->SelectReferralID(nReferralID, AdoFldBool(rs, "Archived"));


					// (c.haag 2009-02-16 10:42) - PLID 33100 - Load procedure info
					rs = rs->NextRecordset(NULL);
					m_anProcedureIDs.RemoveAll();
					while (!rs->eof) {
						m_anProcedureIDs.Add(AdoFldLong(rs, "ProcedureID"));
						rs->MoveNext();
					}
					// Preserve these ID's for auditing purposes
					m_anOldProcedureIDs.Copy(m_anProcedureIDs);

					// Add to the procedure dropdown filter
					if (m_anProcedureIDs.GetSize() > 0) {
						CString strWhere = (LPCTSTR)m_dlProcedures->WhereClause;
						strWhere += FormatString(" OR ProcedureT.ID IN (%s)", ArrayAsString(m_anProcedureIDs));
						m_dlProcedures->WhereClause = _bstr_t(strWhere);
					}

				} // if(!rs->eof) {
				rs->Close();
			}NxCatchAll("Could not load cost data");
		}
		else
		{
			COleVariant now = COleDateTime::GetCurrentTime();
			m_dtFrom.SetValue(now);
			m_dtTo.SetValue(now);
			m_dtPaid.SetValue(now);
			SetDlgItemText(IDC_COST, FormatCurrencyForInterface(COleCurrency(0, 0)));
		}
		GetDlgItem(IDC_COST)->SetFocus();	

		// (c.haag 2009-02-16 09:41) - PLID 33100 - Add sentinel procedure options
		{
			m_dlProcedures->Requery();

			IRowSettingsPtr pRow = m_dlProcedures->GetNewRow();
			pRow->PutValue(0,-2L);
			pRow->PutValue(1," {Multiple Procedures}");
			m_dlProcedures->AddRowBefore(pRow, m_dlProcedures->GetFirstRow());

			pRow = m_dlProcedures->GetNewRow();
			pRow->PutValue(0,-1L);
			pRow->PutValue(1," {No Procedure}");
			m_dlProcedures->AddRowBefore(pRow, m_dlProcedures->GetFirstRow());

			// By default, no procedure is selected
			m_dlProcedures->SetSelByColumn(0, -1L);

			// Now update the procedure info area of the dialog
			DisplayProcedureInfo();
		}

		// (c.haag 2009-02-16 09:24) - PLID 33100 - Converted to datalist 2.
		IRowSettingsPtr pRow = m_dlLocations->GetNewRow();
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, " {No Location}");
		m_dlLocations->AddRowBefore(pRow, m_dlLocations->GetFirstRow());

		//if this item is new, hide the delete button
		if(m_bIsNew) {
			((CWnd*)GetDlgItem(IDC_DELETE))->EnableWindow(FALSE);
		}

	} NxCatchAll("Error in OnInitDialog()");

	return TRUE;
}

int CMarketCostEntry::DoModal(int setID/*=0*/) 
{
	if (m_bActive)
		return IDCANCEL;
	
	m_bActive = true;	//workaround for datalist sending multiple clicks before we go modal
	m_id = setID;
	long result = CNxDialog::DoModal();
	m_bActive = false;
	return result;
}
void CMarketCostEntry::OnOK() 
{
	CString sql, 
			amount, 
			paidTo, 
			number, 
			description, 
			datePaid, 
			from, 
			to, 
			source;

	long sourceID;

	GetDlgItemText(IDC_COST,	amount);
	amount = FormatCurrencyForSql(ParseCurrencyFromInterface(amount));
	GetDlgItemText(IDC_RECIEVER,paidTo);
	paidTo = paidTo;
	GetDlgItemText(IDC_NUMBER,	number);
	GetDlgItemText(IDC_NOTES,	description);
	description = description;
	// (a.wilson 2012-5-14) PLID 50378 - update to handle inactives.
	sourceID = m_pReferralSubDlg->GetSelectedReferralID(true);
	GetDateText(m_dtFrom, from);
	GetDateText(m_dtTo, to);
	GetDateText(m_dtPaid, datePaid);

	CString strLocation;
	IRowSettingsPtr pLocationRow = m_dlLocations->CurSel;
	if(NULL == pLocationRow) {
		strLocation = "NULL";
	}
	else {
		strLocation.Format("%li", VarLong(pLocationRow->GetValue(0L)));
	}

	if(COleDateTime(m_dtTo.GetValue().date) < COleDateTime(m_dtFrom.GetValue().date)) {
		AfxMessageBox("Your 'Effective To' date is before your 'Effective From' date. Please correct these dates.");
		return;
	}

	if (amount == "" || sourceID == -1)
	{	AfxMessageBox("Amount and Referral Source are required");
		return;
	}

	COleDateTime dtTemp;
	dtTemp.ParseDateTime("01/01/1753");

	if((COleDateTime)m_dtFrom.GetValue() < dtTemp || (COleDateTime)m_dtTo.GetValue() < dtTemp || (COleDateTime)m_dtPaid.GetValue() < dtTemp) {
		MsgBox("You cannot enter a date before 1753.");
		return;
	}

	amount.Replace(",", "");

	BOOL bIsNew = (m_id > 0) ? FALSE : TRUE;// (c.haag 2009-12-17 10:22) - PLID 33133
	// (c.haag 2009-02-16 10:48) - PLID 33100 - Save procedures, too
	long nSaveID = (m_id > 0) ? m_id : NewNumber("MarketingCostsT", "ID");

	if (m_id) {
		sql.Format ("UPDATE MarketingCostsT SET Amount = Convert(money,'%s'), PaidTo = \'%s\', RefNumber = \'%s\', "
			"Description = \'%s\', ReferralSource = %i, " 
			"DatePaid = %s, EffectiveFrom = %s, EffectiveTo = %s, LocationID = %s "
			"WHERE ID = %i;\r\n"
			"DELETE FROM MarketingCostProcedureT WHERE MarketingCostID = %d\r\n"
			,_Q(amount), _Q(paidTo), _Q(number), _Q(description), sourceID, datePaid, from, to, strLocation, nSaveID
			,nSaveID
			);
	}
	else {
		sql.Format("INSERT INTO MarketingCostsT (Amount, PaidTo, RefNumber, "
			"Description, ReferralSource, DatePaid, EffectiveFrom, EffectiveTo, ID, LocationID) "
			"SELECT Convert(money,'%s'), \'%s\', \'%s\', \'%s\', %i, %s, %s, %s, %i, %s;\r\n"
			,_Q(amount), _Q(paidTo), _Q(number), _Q(description), sourceID, datePaid, from, to, nSaveID, strLocation);
	}

	if (m_anProcedureIDs.GetSize() > 0) {
		sql += FormatString("INSERT INTO MarketingCostProcedureT (MarketingCostID, ProcedureID) "
			"SELECT %d, ID FROM ProcedureT WHERE ID IN (%s);\r\n"
			,nSaveID, ArrayAsString(m_anProcedureIDs));
	}
	
	try	{
		ExecuteSql("%s", sql);

		//auditing various things
		// (c.haag 2009-12-17 10:21) - PLID 33133 - Different fork for auditing
		if (bIsNew) {
			// We already have a MarketCostDelete enumeration; lets make one for aeiMarketCostCreate. Put the cost, date and
			// procedures in it.
			long nAuditID = BeginNewAuditEvent();
			if(nAuditID != -1) {
				CString strDesc;
				amount = FormatCurrencyForInterface(ParseCurrencyFromSql(amount));
				CString strProc = GetProcedureNameString(m_anProcedureIDs);
				if (strProc.IsEmpty()) { strProc = "(none)"; }
				strDesc.Format("Paid On: %s   Amount: %s   Procedures: %s", datePaid, amount, strProc);
				// (a.wilson 2012-5-14) PLID 50378 - update to handle inactives.
				AuditEvent(-1, m_pReferralSubDlg->GetSelectedReferralName(true), nAuditID, aeiMarketCostCreate, -1, "", strDesc, aepMedium, aetCreated);
			}
		}
		else {
			if(m_strOldAmt != "") {
				amount = FormatCurrencyForInterface(ParseCurrencyFromSql(amount));
				if(m_strOldAmt != amount) {
					long nAuditID = -1;
					nAuditID = BeginNewAuditEvent();
					// (a.wilson 2012-5-14) PLID 50378 - update to handle inactives.
					if(nAuditID != -1)
						AuditEvent(-1, m_pReferralSubDlg->GetSelectedReferralName(true), nAuditID, aeiMarketCostAmount, -1, m_strOldAmt, amount, aepMedium, aetChanged);
				}
			}

			// (c.haag 2009-12-17 10:08) - PLID 33133 - m_strOldDatePaid should not be an empty string; but make sure we don't
			// stick it with an "Invalid DateTime" value if it is
			if (m_strOldDatePaid != "") {
				COleDateTime dt;
				dt.ParseDateTime(m_strOldDatePaid);
				m_strOldDatePaid = "'" + FormatDateTimeForSql(dt, dtoDate) + "'";
			} else {			
				m_strOldDatePaid = "''"; // Still needs to be in single-quotes for consistency
			}
			if(m_strOldDatePaid != datePaid) {
				long nAuditID = -1;
				nAuditID = BeginNewAuditEvent();
				// (a.wilson 2012-5-14) PLID 50378 - update to handle inactives.
				if(nAuditID != -1)
					AuditEvent(-1, m_pReferralSubDlg->GetSelectedReferralName(true), nAuditID, aeiMarketCostDate, -1, m_strOldDatePaid, datePaid, aepMedium, aetChanged);
			}

			// (c.haag 2009-02-16 11:17) - PLID 33100 - Audit changes in procedure names. Keep in mind that if a
			// new cost is created with no procedures, there's no need to audit. Both arrays would be empty. If a
			// new cost is created with a procedure, it will be audited because m_anOldProcedureIDs is empty.
			if (!AreArrayContentsMatched(m_anOldProcedureIDs, m_anProcedureIDs)) {
				CString strOldProc = GetProcedureNameString(m_anOldProcedureIDs);
				CString strNewProc = GetProcedureNameString(m_anProcedureIDs);
				long nAuditID = -1;
				nAuditID = BeginNewAuditEvent();
				// (a.wilson 2012-5-14) PLID 50378 - update to handle inactives.
				if(nAuditID != -1)
					AuditEvent(-1, m_pReferralSubDlg->GetSelectedReferralName(true), nAuditID, aeiMarketCostProcedures, -1, strOldProc, strNewProc, aepMedium, aetChanged);
			}
		}
	}
	NxCatchAll("Could not save cost info");
	CDialog::OnOK();
}
void CMarketCostEntry::OnDelete() 
{
	if (m_id)
	{	if (IDYES != AfxMessageBox("This item has been previously saved, and will be permanently deleted, are you sure?", MB_YESNO))
			return;
		
//		EnsurePractice();
		try {
			// (c.haag 2009-02-16 11:31) - PLID 33100 - Delete from related tables, too
			ExecuteSql(
				"DELETE FROM MarketingCostProcedureT WHERE MarketingCostID = %d;\r\n"
				"DELETE FROM MarketingCostsT WHERE ID = %i", m_id, m_id);

			//auditing
			CString desc;
			GetDlgItemText(IDC_NOTES,	desc);
			long nAuditID = -1;
			nAuditID = BeginNewAuditEvent();
			// (a.wilson 2012-5-14) PLID 50378 - update to handle inactives.
			if(nAuditID != -1)
				AuditEvent(-1, m_pReferralSubDlg->GetSelectedReferralName(true), nAuditID, aeiMarketCostDelete, -1, desc, "<Deleted>", aepMedium, aetDeleted);
		}
		NxCatchAll("Could not delete");
	}
	CDialog::OnOK();
}

void CMarketCostEntry::OnCancel()
{
	CDialog::OnCancel();
}

void CMarketCostEntry::OnKillfocusCost() 
{
	COleCurrency tmpCur;
	CString value;

	GetDlgItemText (IDC_COST, value);
	COleCurrency cy = ParseCurrencyFromInterface(value);
	if(value.IsEmpty() || !IsValidCurrencyText(value) || cy.GetStatus() == COleCurrency::invalid) {
		MsgBox("Please enter a valid currency amount. \nInvalid Cost will be set to $0.00");
		value = "0.00";
	}

	tmpCur = ParseCurrencyFromInterface(value);
	if (tmpCur.GetStatus() == 0)
	{	RoundCurrency(tmpCur);
		SetDlgItemText (IDC_COST, FormatCurrencyForInterface(tmpCur));
	}
	else SetDlgItemText (IDC_COST, FormatCurrencyForInterface(COleCurrency(0, 0)));
}

void CMarketCostEntry::OnSelectContact() 
{
	CSelectContactDlg dlg(this);
	if(dlg.DoModal() == IDOK) {
		CString str, strMiddle, strCompany;

		//we don't want to put a bunch of spaces between things
		if(!dlg.m_strMiddle.IsEmpty())
			strMiddle.Format(" %s", dlg.m_strMiddle);

		if(!dlg.m_strCompany.IsEmpty())
			strCompany.Format(" - %s", dlg.m_strCompany);

		str.Format("%s%s %s%s", dlg.m_strFirst, strMiddle, dlg.m_strLast, strCompany);

		SetDlgItemText(IDC_RECIEVER, str);
	}
}

BEGIN_EVENTSINK_MAP(CMarketCostEntry, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CMarketCostEntry)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CMarketCostEntry, IDC_COST_LOCATION, 1, CMarketCostEntry::SelChangingCostLocation, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CMarketCostEntry, IDC_COST_LOCATION, 16, CMarketCostEntry::SelChosenCostLocation, VTS_DISPATCH)
	ON_EVENT(CMarketCostEntry, IDC_COMBO_EXP_PROCEDURES, 1, CMarketCostEntry::SelChangingComboExpProcedures, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CMarketCostEntry, IDC_COMBO_EXP_PROCEDURES, 16, CMarketCostEntry::SelChosenComboExpProcedures, VTS_DISPATCH)
END_EVENTSINK_MAP()

BOOL CMarketCostEntry::DestroyWindow() 
{
	try {
		if(m_pReferralSubDlg) {
			m_pReferralSubDlg->DestroyWindow();
			delete m_pReferralSubDlg;
			m_pReferralSubDlg = NULL;
		}
	} NxCatchAll("Error in OnDestroy()");

	return CNxDialog::DestroyWindow();
}

void CMarketCostEntry::SelChangingCostLocation(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			// (c.haag 2009-02-16 09:50) - PLID 33100 - Don't let them select nothing, change it back to the old row
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}
	NxCatchAll("Error in CMarketCostEntry::SelChangingCostLocation");
}

void CMarketCostEntry::SelChosenCostLocation(LPDISPATCH lpRow)
{
	// (c.haag 2009-02-16 09:21) - PLID 33100 - Converted from legacy code
	try {
		IRowSettingsPtr pRow(lpRow);
		if (NULL != pRow && VarLong(pRow->GetValue(0L)) == -1) {
			m_dlLocations->CurSel = NULL;
		}
	}
	NxCatchAll("Error in CMarketCostEntry::SelChosenCostLocation");
}

void CMarketCostEntry::SelChangingComboExpProcedures(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			// (c.haag 2009-02-16 09:50) - PLID 33100 - Don't let them select nothing, change it back to the old row
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}
	NxCatchAll("Error in CMarketCostEntry::SelChangingComboExpProcedures");
}

void CMarketCostEntry::SelChosenComboExpProcedures(LPDISPATCH lpRow)
{
	// (c.haag 2009-02-16 09:49) - PLID 33100 - The user changed the procedure dropdown
	try {
		IRowSettingsPtr pRow(m_dlProcedures->CurSel);
		if (NULL != pRow) {
			long nID = VarLong(pRow->GetValue(0L));
			//if the ID is -2, it's the "multiple" row
			if (nID == -2) {
				m_dlProcedures->CurSel = NULL;
				OnMultiProcedure();			
			}
			//if the ID is -1, it's the "none selected" row
			else if (nID == -1) {
				m_anProcedureIDs.RemoveAll();
			}
			else{
				m_anProcedureIDs.RemoveAll();
				m_anProcedureIDs.Add(nID);
			}
		}
		else {
			m_anProcedureIDs.RemoveAll();
		}
	}
	NxCatchAll("Error in CMarketCostEntry::SelChosenComboExpProcedures");
}

void CMarketCostEntry::OnLButtonDown(UINT nFlags, CPoint point)
{
	try {
		CNxDialog::OnLButtonDown(nFlags, point);

		if (!GetDlgItem(IDC_COMBO_EXP_PROCEDURES)->IsWindowVisible()) {
			if (m_rcMultiProcedureLabel.PtInRect(point)) {
				OnMultiProcedure();
			}	
		}
	}
	NxCatchAll("Error in CMarketCostEntry::OnLButtonDown");
}

// (c.haag 2009-02-16 10:05) - PLID 33100 - Handles when a user want to select multiple procedures
void CMarketCostEntry::OnMultiProcedure()
{
	try {
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "ProcedureT");
		CString strWhere = "(Inactive = 0 AND MasterProcedureID IS NULL)";
		// Make sure the current selection is always in the list
		if (m_anProcedureIDs.GetSize() > 0) {
			strWhere += FormatString(" OR ProcedureT.ID IN (%s)", ArrayAsString(m_anProcedureIDs));
		}
		// Make sure any original selections are always in the list
		if (m_anOldProcedureIDs.GetSize() > 0) {
			strWhere += FormatString(" OR ProcedureT.ID IN (%s)", ArrayAsString(m_anOldProcedureIDs));
		}

		dlg.PreSelect(m_anProcedureIDs);
		dlg.m_strNameColTitle = "Procedure";
		if (IDOK == dlg.Open("ProcedureT", strWhere, 
			"ProcedureT.ID", "ProcedureT.Name", "Select Procedures"))
		{
			dlg.FillArrayWithIDs(m_anProcedureIDs);
		}
		DisplayProcedureInfo();
	}
	NxCatchAll("Error in CMarketCostEntry::OnMultiProcedure()");
}

// (c.haag 2009-02-16 10:04) - PLID 33100 - Updates the procedure region on the dialog
void CMarketCostEntry::DisplayProcedureInfo()
{
	try {
		//if we only have 1 item, select it in the datalist, don't bother setting this all up
		if(m_anProcedureIDs.GetSize() <= 1) {
			GetDlgItem(IDC_COMBO_EXP_PROCEDURES)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_MULTI_PROCEDURE_EXP_LABEL)->ShowWindow(SW_HIDE);

			if(m_anProcedureIDs.GetSize() == 1) {

				// First, wait for the procedure dropdown to finish populating. Then we
				// can use it as a source for getting names
				m_dlProcedures->WaitForRequery(dlPatienceLevelWaitIndefinitely);

				if (NULL == m_dlProcedures->SetSelByColumn(0, m_anProcedureIDs[0])) {
					_RecordsetPtr prs = CreateParamRecordset("SELECT Name FROM ProcedureT WHERE ID = {INT}", m_anProcedureIDs[0]);
					if (!prs->eof) {
						IRowSettingsPtr pRow = m_dlProcedures->GetNewRow();
						pRow->PutValue(0,m_anProcedureIDs[0]);
						pRow->PutValue(1,prs->Fields->Item["Name"]->Value);
						m_dlProcedures->AddRowSorted(pRow, NULL);
						m_dlProcedures->SetSelByColumn(0, m_anProcedureIDs[0]);
					} else {
						// It was deleted. Nothing we can do.
					}
				}
			}
			return;
		}

		GetDlgItem(IDC_COMBO_EXP_PROCEDURES)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_MULTI_PROCEDURE_EXP_LABEL)->ShowWindow(SW_SHOW);

		// Populate the label string
		m_strProcedureList = GetProcedureNameString(m_anProcedureIDs);
		InvalidateRect(m_rcMultiProcedureLabel);
	}
	NxCatchAll("Error in CMarketCostEntry::DisplayProcedureInfo()");
}

// (c.haag 2009-02-16 10:05) - PLID 33100 - Returns a procedure name
CString CMarketCostEntry::GetProcedureName(long nID)
{
	// First, wait for the procedure dropdown to finish populating. Then we
	// can use it as a source for getting names
	m_dlProcedures->WaitForRequery(dlPatienceLevelWaitIndefinitely);

	// Now search for the name by ID
	IRowSettingsPtr pRow = m_dlProcedures->FindByColumn(0, nID, NULL, VARIANT_FALSE);
	if (NULL != pRow) {
		return VarString(pRow->GetValue(1));
	} else {
		// Could not find it. Go to plan B.
		_RecordsetPtr prs = CreateParamRecordset("SELECT Name FROM ProcedureT WHERE ID = {INT}", nID);
		if (!prs->eof) {
			return AdoFldString(prs, "Name");
		}
	}
	return ""; // Could not find in data or in the list
}

// (c.haag 2009-02-16 11:00) - PLID 33100 - Returns a string of procedure names
CString CMarketCostEntry::GetProcedureNameString(CArray<long,long>& anProcedureIDs)
{
	CString strLabel;
	int i;
	for (i=0; i < anProcedureIDs.GetSize(); i++) {
		strLabel += GetProcedureName(anProcedureIDs[i]) + ", ";
	}
	strLabel.TrimRight(", ");
	return strLabel;
}

void CMarketCostEntry::OnPaint()
{
	// (c.haag 2009-02-16 10:22) - PLID 33100 - Paint the hyperlink
	CPaintDC dc(this); // device context for painting
	if (m_labelMultiProcedure.IsWindowVisible()) {
		DrawTextOnDialog(this, &dc, m_rcMultiProcedureLabel, m_strProcedureList, dtsHyperlink, false, DT_LEFT, true, false, 0);
	}
}

BOOL CMarketCostEntry::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// (c.haag 2009-02-16 10:23) - PLID 33100 - Update the mouse cursor over the static label
	if (m_labelMultiProcedure.IsWindowVisible()) {
		SetCursor(GetLinkCursor());
	}
	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}
