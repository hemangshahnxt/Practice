// ReferralOrderEntryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PatientsRc.h"
#include "ReferralOrderEntryDlg.h"
#include "AuditTrail.h"
#include "InternationalUtils.h"
#include "GlobalReportUtils.h"
#include "Reports.h"

using namespace NXDATALIST2Lib;

// (a.walling 2010-01-21 16:43) - PLID 37026 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



// (z.manning 2009-05-06 12:39) - PLID 28530
void AuditReferralOrderChange(const ReferralOrder &reforderOld, const ReferralOrder &reforderNew, BOOL bNewlyCreated = FALSE)
{
	CAuditTransaction audit;

	CString strPatientName = GetExistingPatientName(reforderNew.nPatientID);

	if(bNewlyCreated) {
		AuditEvent(reforderNew.nPatientID, strPatientName, audit, aeiReferralOrderCreated, reforderNew.nID, ""
			, FormatString("Referral to %s on %s",reforderNew.strReferToName,FormatDateTimeForInterface(reforderNew.dtDate,0,dtoDate))
			, aepMedium, aetCreated);
	}

	if(reforderOld.dtDate.GetStatus() != COleDateTime::valid || reforderOld.dtDate != reforderNew.dtDate) {
		CString strOld;
		if(reforderOld.dtDate.GetStatus() == COleDateTime::valid) {
			strOld = FormatDateTimeForInterface(reforderOld.dtDate,0,dtoDate);
		}
		CString strNew = FormatDateTimeForInterface(reforderNew.dtDate,0,dtoDate);

		AuditEvent(reforderNew.nPatientID, strPatientName, audit, aeiReferralOrderDate, reforderNew.nID, strOld, strNew, aepMedium, aetChanged);
	}

	if(reforderOld.strReferToName != reforderNew.strReferToName) {
		AuditEvent(reforderNew.nPatientID, strPatientName, audit, aeiReferralOrderReferTo, reforderNew.nID, reforderOld.strReferToName, reforderNew.strReferToName, aepMedium, aetChanged);
	}

	if(reforderOld.strReferredByName != reforderNew.strReferredByName) {
		AuditEvent(reforderNew.nPatientID, strPatientName, audit, aeiReferralOrderReferredBy, reforderNew.nID, reforderOld.strReferredByName, reforderNew.strReferredByName, aepMedium, aetChanged);
	}

	if(reforderOld.strReason != reforderNew.strReason) {
		AuditEvent(reforderNew.nPatientID, strPatientName, audit, aeiReferralOrderReason, reforderNew.nID, reforderOld.strReason, reforderNew.strReason, aepMedium, aetChanged);
	}

	audit.Commit();
}

// (z.manning 2009-05-06 12:44) - PLID 28530
void AuditNewReferralOrder(const ReferralOrder &reforderNew)
{
	ReferralOrder reforderBlank;
	AuditReferralOrderChange(reforderBlank, reforderNew, TRUE);
}

// (z.manning 2009-05-12 12:41) - PLID 34219
void PrintReferralOrder(const long nReferralOrderID, BOOL bPreview)
{
	CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(662)]);
	infReport.nExtraID = nReferralOrderID;

	CPrintInfo prInfo;
	if(!bPreview) {
		CPrintDialog* dlg;
		dlg = new CPrintDialog(FALSE);
		prInfo.m_bPreview = false;
		prInfo.m_bDirect = false;
		prInfo.m_bDocObject = false;
		if(prInfo.m_pPD != NULL) {
			delete prInfo.m_pPD;
		}
		prInfo.m_pPD = dlg;
	}

	RunReport(&infReport, bPreview, GetMainFrame(), "Referral Order", &prInfo);
}

// CReferralOrderEntryDlg dialog
// (z.manning 2009-05-05 09:48) - PLID 28529 - Created

IMPLEMENT_DYNAMIC(CReferralOrderEntryDlg, CNxDialog)

CReferralOrderEntryDlg::CReferralOrderEntryDlg(const long nPatientID, CWnd* pParent)
	: CNxDialog(CReferralOrderEntryDlg::IDD, pParent)
{
	m_nPatientID = nPatientID;
	m_varOrderSetID.vt = VT_NULL;
	m_bCloseParent = FALSE;
	m_nDefaultRefPhysID = -1;
	m_bPreventPreview = FALSE;
}

CReferralOrderEntryDlg::~CReferralOrderEntryDlg()
{
}

void CReferralOrderEntryDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_REFERRAL_ORDER_ENTRY_BACKGROUND, m_nxcolor);
	DDX_Control(pDX, IDC_REFERRAL_ORDER_ENTRY_TEXT, m_nxstaticHeader);
	DDX_Control(pDX, IDC_REFERRING_PHYS_TEXT, m_nxstaticRefPhysText);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_REFERRAL_ORDER_REASON, m_nxeditReason);
	DDX_Control(pDX, IDC_REFERRAL_ORDER_DATE, m_dtpDate);
	DDX_Control(pDX, IDC_PRINT_REFERRAL_ORDER, m_btnPrint);
	DDX_Control(pDX, IDC_PREVIEW_REFERRAL_ORDER, m_btnPreview);
}


BEGIN_MESSAGE_MAP(CReferralOrderEntryDlg, CNxDialog)
	ON_BN_CLICKED(IDC_PREVIEW_REFERRAL_ORDER, &CReferralOrderEntryDlg::OnBnClickedPreviewReferralOrder)
	ON_BN_CLICKED(IDC_PRINT_REFERRAL_ORDER, &CReferralOrderEntryDlg::OnBnClickedPrintReferralOrder)
END_MESSAGE_MAP()


// CReferralOrderEntryDlg message handlers

BOOL CReferralOrderEntryDlg::OnInitDialog()
{
	try
	{
		CNxDialog::OnInitDialog();

		// (j.jones 2010-08-27 11:41) - PLID 40167 - added bulk caching
		g_propManager.CachePropertiesInBulk("CReferralOrderEntryDlg", propNumber,
				"(Username = '<None>' OR Username = '%s') AND ("
				"Name = 'ReferralOrder_RequireReason' "
				")",
				_Q(GetCurrentUserName()));

		m_pdlReferTo = BindNxDataList2Ctrl(IDC_REFER_TO_LIST, true);

		m_pdlReferredBy = BindNxDataList2Ctrl(IDC_REFERRED_BY_LIST, false);
		m_pdlReferredBy->Requery();

		m_nxcolor.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnPrint.AutoSet(NXB_PRINT);
		m_btnPreview.AutoSet(NXB_PRINT_PREV);
		m_nxeditReason.SetLimitText(255);

		m_nxstaticHeader.SetWindowText(FormatString("Referral order for patient: %s - %li"
			, GetExistingPatientName(m_nPatientID), GetExistingPatientUserDefinedID(m_nPatientID)));

		if(!IsNew()) {
			m_nxeditReason.SetWindowText(m_OriginalOrder.strReason);
			m_dtpDate.SetValue(m_OriginalOrder.dtDate);
		}

		if(m_bPreventPreview) {
			m_btnPreview.ShowWindow(SW_HIDE);
			m_btnPreview.EnableWindow(FALSE);
		}

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

BOOL CReferralOrderEntryDlg::IsNew()
{
	return (m_OriginalOrder.nID == -1);
}
BEGIN_EVENTSINK_MAP(CReferralOrderEntryDlg, CNxDialog)
	ON_EVENT(CReferralOrderEntryDlg, IDC_REFER_TO_LIST, 18, CReferralOrderEntryDlg::RequeryFinishedReferToList, VTS_I2)
	ON_EVENT(CReferralOrderEntryDlg, IDC_REFERRED_BY_LIST, 18, CReferralOrderEntryDlg::RequeryFinishedReferredByList, VTS_I2)
	ON_EVENT(CReferralOrderEntryDlg, IDC_REFER_TO_LIST, 16, CReferralOrderEntryDlg::SelChosenReferToList, VTS_DISPATCH)
END_EVENTSINK_MAP()

void CReferralOrderEntryDlg::RequeryFinishedReferToList(short nFlags)
{
	try
	{
		HideDatalistRowsBasedOnBooleanColumn(m_pdlReferTo, rtcInactive);

		if(!IsNew()) {
			m_pdlReferTo->SetSelByColumn(rtcID, m_OriginalOrder.nReferToID);
			UpdateRefPhysText();
		}
		else if(m_nDefaultRefPhysID != -1) {
			// (z.manning 2009-05-13 12:31) - PLID 28554 - If this is a new referral order and we
			// have a default ref phys ID, then set that as the selection.
			m_pdlReferTo->SetSelByColumn(rtcID, m_nDefaultRefPhysID);
			m_nDefaultRefPhysID = -1;
		}

	}NxCatchAll(__FUNCTION__);
}

void CReferralOrderEntryDlg::RequeryFinishedReferredByList(short nFlags)
{
	try
	{
		HideDatalistRowsBasedOnBooleanColumn(m_pdlReferredBy, rbcInactive);
		
		IRowSettingsPtr pRow = m_pdlReferredBy->GetNewRow();
		pRow->PutValue(rbcID, g_cvarNull);
		pRow->PutValue(rbcName, "< No Referring Doctor >");
		pRow->PutValue(rbcInactive, g_cvarFalse);
		m_pdlReferredBy->AddRowBefore(pRow, m_pdlReferredBy->GetFirstRow());

		// (z.manning 2009-05-05 15:47) - If this is a new referral order then default
		// the referring phys to the patient's G1 provider
		if(IsNew()) {
			ADODB::_RecordsetPtr prs = CreateParamRecordset(
				"SELECT MainPhysician FROM PatientsT \r\n"
				"INNER JOIN PersonT Provider ON PatientsT.MainPhysician = Provider.ID \r\n"
				"WHERE PatientsT.PersonID = {INT} AND Provider.Archived = 0 \r\n"
				, m_nPatientID);
			if(!prs->eof) {
				m_pdlReferredBy->SetSelByColumn(rbcID, AdoFldLong(prs->GetFields(), "MainPhysician"));
			}
		}
		else {
			m_pdlReferredBy->SetSelByColumn(rbcID, m_OriginalOrder.varReferredByID);
		}

	}NxCatchAll(__FUNCTION__);
}

void CReferralOrderEntryDlg::UpdateRefPhysText()
{
	IRowSettingsPtr pRow = m_pdlReferTo->GetCurSel();
	if(pRow == NULL) {
		m_nxstaticRefPhysText.SetWindowText("");
	}
	else {
		CString strText;
		strText += VarString(pRow->GetValue(rtcAddress1), "") + "\r\n";
		CString strCity = VarString(pRow->GetValue(rtcCity), "");
		if(!strCity.IsEmpty()) {
			strText += strCity + ", ";
		}
		strText += VarString(pRow->GetValue(rtcState), "") + ' ';
		strText += VarString(pRow->GetValue(rtcZip), "") + "\r\n";

		CString strWorkPhone = VarString(pRow->GetValue(rtcWorkPhone), "");
		if(!strWorkPhone.IsEmpty()) {
			strText += "Work Phone: " + strWorkPhone + "\r\n";
		}
		CString strSpecialty = VarString(pRow->GetValue(rtcSpecialty), "");
		if(!strSpecialty.IsEmpty()) {
			strText += "Specialty: " + strSpecialty + "\r\n";
		}

		m_nxstaticRefPhysText.SetWindowText(strText);
	}
}
void CReferralOrderEntryDlg::SelChosenReferToList(LPDISPATCH lpRow)
{
	try
	{
		UpdateRefPhysText();

	}NxCatchAll(__FUNCTION__);
}

void CReferralOrderEntryDlg::OnOK()
{
	try
	{
		if(!Save()) {
			return;
		}

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}


BOOL CReferralOrderEntryDlg::Save()
{
	ReferralOrder reforderNew;
	reforderNew.nID = m_OriginalOrder.nID;
	reforderNew.nPatientID = m_nPatientID;

	IRowSettingsPtr prowReferTo = m_pdlReferTo->GetCurSel();
	if(prowReferTo == NULL) {
		MessageBox("You must select a doctor to 'refer to.'");
		return FALSE;
	}
	reforderNew.nReferToID = VarLong(prowReferTo->GetValue(rtcID));

	reforderNew.dtDate = m_dtpDate.GetValue();
	if(reforderNew.dtDate.GetStatus() != COleDateTime::valid || reforderNew.dtDate.GetYear() < 1753) {
		MessageBox("Please select a valid date");
		return FALSE;
	}
	reforderNew.dtDate.SetDate(reforderNew.dtDate.GetYear(), reforderNew.dtDate.GetMonth(), reforderNew.dtDate.GetDay());

	reforderNew.varReferredByID = g_cvarNull;
	IRowSettingsPtr prowReferredBy = m_pdlReferredBy->GetCurSel();
	if(prowReferredBy != NULL) {
		reforderNew.varReferredByID = prowReferredBy->GetValue(rbcID);
		if(prowReferredBy->GetValue(rbcID).vt == VT_I4) {
			reforderNew.strReferredByName = VarString(prowReferredBy->GetValue(rbcName), "");
		}
	}

	m_nxeditReason.GetWindowText(reforderNew.strReason);

	// (j.jones 2010-08-27 11:29) - PLID 40167 - check the preference if this is required,
	// and warn if the order is new or the reason changed (don't warn if we just opened it
	// and didn't change anything)
	if(GetRemotePropertyInt("ReferralOrder_RequireReason", 0, 0, "<None>", true) == 1
		&& (IsNew() || m_OriginalOrder.strReason != reforderNew.strReason)) {
		CString strCheckReason = reforderNew.strReason;
		strCheckReason.Replace("\r", "");
		strCheckReason.Replace("\n", "");
		strCheckReason.TrimLeft();
		strCheckReason.TrimRight();
		if(strCheckReason.IsEmpty()) {
			MessageBox("You must enter in a Reason for this order.", "Practice", MB_ICONINFORMATION|MB_OK);
			return FALSE;
		}
	}

	// (z.manning 2009-05-06 14:09) - PLID 28530 - Needed for auditing
	reforderNew.strReferToName = VarString(prowReferTo->GetValue(rtcName), "");

	if(IsNew())
	{
		// (z.manning 2009-05-08 09:55) - PLID 28554 - Added OrderSetID
		ADODB::_RecordsetPtr prs = CreateParamRecordset(
			"SET NOCOUNT ON \r\n"
			"INSERT INTO ReferralOrdersT (Date, PatientID, ReferToID, ReferredByID, Reason, InputDate, InputUserID, OrderSetID) \r\n"
			"VALUES ({OLEDATETIME}, {INT}, {INT}, {VT_I4}, {STRING}, GetDate(), {INT}, {VT_I4}) \r\n"
			"SET NOCOUNT OFF \r\n"
			"SELECT convert(int, SCOPE_IDENTITY()) AS NewID \r\n"
			, reforderNew.dtDate, m_nPatientID, reforderNew.nReferToID, reforderNew.varReferredByID, reforderNew.strReason, GetCurrentUserID(), m_varOrderSetID);

		reforderNew.nID = AdoFldLong(prs->GetFields(), "NewID");

		// (z.manning 2009-05-06 12:57) - PLID 28530 - Audit
		AuditNewReferralOrder(reforderNew);
	}
	else
	{
		ExecuteParamSql(
			"UPDATE ReferralOrdersT SET \r\n"
			"	Date = {OLEDATETIME}, \r\n"
			"	PatientID = {INT}, \r\n"
			"	ReferToID = {INT}, \r\n"
			"	ReferredByID = {VT_I4}, \r\n"
			"	Reason = {STRING} \r\n"
			"WHERE ID = {INT} \r\n"
			, reforderNew.dtDate, m_nPatientID, reforderNew.nReferToID, reforderNew.varReferredByID, reforderNew.strReason
			, m_OriginalOrder.nID);

		// (z.manning 2009-05-06 12:57) - PLID 28530 - Audit
		AuditReferralOrderChange(m_OriginalOrder, reforderNew);
	}

	m_OriginalOrder = reforderNew;

	return TRUE;
}

int CReferralOrderEntryDlg::EditExistingReferralOrder(const long nReferralOrderID)
{
	if(nReferralOrderID == -1) {
		ASSERT(FALSE);
		return IDCANCEL;
	}

	ADODB::_RecordsetPtr prs = CreateParamRecordset(
		"SELECT Date, ReferToID, ReferredByID, Reason, PatientID \r\n"
		"	, RefPhysPerson.[Last] + ', ' + RefPhysPerson.[First] + ' ' + RefPhysPerson.[Middle] + ' ' + RefPhysPerson.[Title] AS RefPhysName \r\n"
		"	, ProviderPerson.[Last] + ', ' + ProviderPerson.[First] + ' ' + ProviderPerson.[Middle] + ' ' + ProviderPerson.[Title] AS ProviderName \r\n"
		"FROM ReferralOrdersT \r\n"
		"LEFT JOIN PersonT RefPhysPerson ON ReferToID = RefPhysPerson.ID \r\n"
		"LEFT JOIN PersonT ProviderPerson ON ReferredByID = ProviderPerson.ID \r\n"
		"WHERE ReferralOrdersT.ID = {INT} \r\n"
		, nReferralOrderID);
	if(prs->eof) {
		return IDCANCEL;
	}

	ReferralOrder reforder;
	reforder.nID = nReferralOrderID;
	reforder.dtDate = AdoFldDateTime(prs->GetFields(), "Date");
	reforder.nReferToID = AdoFldLong(prs->GetFields(), "ReferToID");
	reforder.varReferredByID = prs->GetFields()->GetItem("ReferredByID")->Value;
	reforder.strReason = AdoFldString(prs->GetFields(), "Reason", "");
	reforder.nPatientID = AdoFldLong(prs->GetFields(), "PatientID");
	reforder.strReferToName = AdoFldString(prs->GetFields(), "RefPhysName", "");
	reforder.strReferredByName = AdoFldString(prs->GetFields(), "ProviderName", "");
	
	return EditExistingReferralOrder(reforder);
}

int CReferralOrderEntryDlg::EditExistingReferralOrder(const ReferralOrder &referralorder)
{
	ASSERT(referralorder.nID != -1);
	m_OriginalOrder = referralorder;
	return DoModal();
}

// (z.manning 2009-05-08 09:39) - PLID 28554 - Sets the order set ID if this order is part of an order set.
void CReferralOrderEntryDlg::SetOrderSetID(const long nOrderSetID)
{
	m_varOrderSetID = nOrderSetID;
}

// (z.manning 2009-05-12 15:29) - PLID 34219
void CReferralOrderEntryDlg::OnBnClickedPreviewReferralOrder()
{
	try
	{
		if(!Save()) {
			return;
		}
		CNxDialog::OnOK();
		m_bCloseParent = TRUE;
		PrintReferralOrder(m_OriginalOrder.nID, TRUE);

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2009-05-12 15:29) - PLID 34219
void CReferralOrderEntryDlg::OnBnClickedPrintReferralOrder()
{
	try
	{
		if(!Save()) {
			return;
		}
		CNxDialog::OnOK();
		PrintReferralOrder(m_OriginalOrder.nID, FALSE);

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2009-05-13 12:29) - PLID 28554 - Use this to auto-select a ref phys when the dialog loads.
void CReferralOrderEntryDlg::SetDefaultRefPhysID(const long nRefPhysID)
{
	m_nDefaultRefPhysID = nRefPhysID;
}
