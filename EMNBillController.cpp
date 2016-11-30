#include "stdafx.h"
#include "EMNBillController.h"
#include "PatientView.h"
#include <foreach.h>
#include "BillingModuleDlg.h"

// (j.dinatale 2012-01-16 10:53) - PLID 47539 - will control billing from EMR

CEMNBillController::CEMNBillController(void) : CMessageOnlyWnd()
{
	Reset();
	m_BillingDlg = NULL;
}

BEGIN_MESSAGE_MAP(CEMNBillController, CMessageOnlyWnd)
	//{{AFX_MSG_MAP(CEMNBillController)
	ON_MESSAGE(NXM_POST_EDIT_BILL, OnPostEditBill)
	ON_MESSAGE(NXM_POST_CANCEL_BILL, OnPostCancelBill)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CEMNBillController::Initialize()
{
	try {
		g_propManager.BulkCache("EMNBillManager", propbitNumber,
			"(Username = '<None>') AND ("
			"Name = 'RequireEMRChargeResps' OR "
			"Name = 'DisplayChargeSplitDlgAlways' "
			")");

		// create the dialog
		return Create();
	} NxCatchAll(__FUNCTION__);

	return FALSE;
}

void CEMNBillController::Reset(long nNewEMRID /*= -1*/, long nNewPatientID /*= -1*/)
{
	m_nCurrEMNID = nNewEMRID;
	m_nPatientID = nNewPatientID;
	m_aryInsuredPartiesToBill.RemoveAll();
}

bool CEMNBillController::CanWeBill(long nEMNID)
{
	if (GetMainFrame()->IsBillingModuleOpen(true)) {
		return false;
	}

	CPatientView* pView = (CPatientView *)GetMainFrame()->GetOpenView(PATIENT_MODULE_NAME);
	//(e.lally 2012-03-29) PLID 48979 - The patient view may have destroyed the billing dlg, without telling us, if the module was closed.
	//	So we're no longer going to try to validate a billing dlg pointer that may be dangling (pointing to invalid memory).
	//	That left open the possibility for access violations, though hard to reproduce.
	//	Just ensure the patients module is open and get the accurate pointer from the patient view each time.
	if(!pView) {
		//if the patients module is closed, our billing pointer is definitely invalid,
		//so we must get it back!
		if(GetMainFrame()->FlipToModule(PATIENT_MODULE_NAME)) {
			pView = (CPatientView *)GetMainFrame()->GetOpenView(PATIENT_MODULE_NAME);
		}
		else {
			//We couldn't even open the patients module so we must fail
			return false;
		}
	}

	//(e.lally 2012-03-29) PLID 48979 - Always ensure we have an accurate pointer by getting it from the source each time.
	m_BillingDlg = pView->GetBillingDlg();

	return m_BillingDlg != NULL;
}

bool CEMNBillController::BillEntireEMN(long nEMNID, long nPatientID, long nInsuredPartyID)
{
	try{
		if(!CanWeBill(nEMNID)){
			// No, we can't!
			return false;
		}

		Reset(nEMNID, nPatientID);

		// Yes, we can!
		if(m_BillingDlg) {
			m_BillingDlg->m_pFinancialDlg = this;
			m_BillingDlg->m_nPatientID = nPatientID;

			// (j.jones 2011-06-30 09:00) - PLID 43770 - track if we are being created from an EMR
			// (j.gruber 2016-03-22 10:12) - PLID 68006 - Change bFromEMR to an Enum
			m_BillingDlg->OpenWithBillID(-1, BillEntryType::Bill, 1, BillFromType::EMR);
			m_BillingDlg->PostMessage(NXM_BILL_EMR, nEMNID, nInsuredPartyID);
			return true;
		}
	}NxCatchAll(__FUNCTION__);

	return false;
}

bool CEMNBillController::BillEMNToAssignedResps(long nEMNID, long nPatientID)
{
	try{
		if(!CanWeBill(nEMNID)){
			// No, we can't!
			return false;
		}

		if(HasUnassignedCharges(nEMNID)){
			// we shouldn't really end up here, but it is possible
			ASSERT(FALSE);
			return false;
		}

		Reset(nEMNID, nPatientID);
		// (j.dinatale 2012-01-23 10:44) - PLID 47539 - see if we have billed for each insured party
		ADODB::_RecordsetPtr rsInsInfo = CreateParamRecordset(
			"SELECT DISTINCT EMRChargeRespT.InsuredPartyID, CONVERT(BIT, CASE WHEN EXISTS ( "
			"SELECT TOP 1 1 FROM "
			"( "
			"	SELECT EMRChargesT.ID, EMRChargesT.EMRID, EMRChargeRespT.InsuredPartyID FROM "
			"	EMRChargesT "
			"	LEFT JOIN EMRChargeRespT ON EMRChargesT.ID = EMRChargeRespT.EMRChargeID "
			"	LEFT JOIN CPTCodeT ON EMRChargesT.ServiceID = CPTCodeT.ID "
			"	LEFT JOIN "
			"	( "
			"		SELECT BillsT.Deleted, BilledEMNsT.* FROM BilledEMNsT "
			"		LEFT JOIN BillsT ON BilledEMNsT.BillID = BillsT.ID "
			"		LEFT JOIN BillCorrectionsT ON BillsT.ID = BillCorrectionsT.OriginalBillID "
			"		WHERE BillCorrectionsT.OriginalBillID IS NULL AND BillsT.Deleted = 0 AND BillsT.EntryType = 1 "
			"	) BilledEMNsSubQ ON BilledEMNsSubQ.EMNID = EMRChargesT.EMRID "
			"		AND (EMRChargeRespT.EMRChargeID Is Null OR COALESCE(EMRChargeRespT.InsuredPartyID, -1) = COALESCE(BilledEMNsSubQ.InsuredPartyID, -1)) "
			"	WHERE EMRChargesT.Deleted = 0 AND (CPTCodeT.Billable Is Null OR CPTCodeT.Billable = 1) AND BilledEMNsSubQ.BillID IS NULL "
			") UnbilledEMNsSubQ WHERE UnbilledEMNsSubQ.EMRID = EMRChargesT.EMRID "
			"	AND (EMRChargeRespT.EMRChargeID Is Null OR COALESCE(EMRChargeRespT.InsuredPartyID, -1) = COALESCE(UnbilledEMNsSubQ.InsuredPartyID, -1)))  THEN 0 ELSE 1 END) AS BeenBilled "
			"FROM EMRChargesT "
			"INNER JOIN EMRChargeRespT ON EMRChargesT.ID = EMRChargeRespT.EMRChargeID " 
			"WHERE EMRChargesT.EMRID = {INT} AND EMRChargesT.Deleted = 0 ", nEMNID);

		int nCount = 0;
		while(!rsInsInfo->eof){
			long nInsuredPartyID = AdoFldLong(rsInsInfo, "InsuredPartyID", -1);
			BOOL bHasBeenBilled = AdoFldBool(rsInsInfo, "BeenBilled", FALSE);
			nCount++;	// (j.dinatale 2012-01-23 09:31) - PLID 47539 - need to keep track of how many insured parties we collected
	
			// (j.dinatale 2012-01-23 09:30) - PLID 47539 - need to filter out insured parties that are already billed
			if(!bHasBeenBilled){
				m_aryInsuredPartiesToBill.Add(nInsuredPartyID);
			}

			rsInsInfo->MoveNext();
		} 

		if(m_aryInsuredPartiesToBill.IsEmpty()){
			Reset();
			return true;
		}

		// (j.dinatale 2012-01-23 09:35) - PLID 47539 - nCount helps us determine if we ended up billing the entire thing to begin with (in case user cancels half way through a partial)
		// (j.dinatale 2012-01-17 17:50) - PLID 47539 - if we only have one resp, just bill the entire thing
		if(nCount == 1 && m_aryInsuredPartiesToBill.GetSize() == 1){
			return BillEntireEMN(nEMNID, nPatientID, (long)m_aryInsuredPartiesToBill.GetAt(0));
		}

		// Yes, we can!
		if(m_BillingDlg) {
			m_BillingDlg->m_pFinancialDlg = this;
			m_BillingDlg->m_nPatientID = nPatientID;

			long nInsuredPartyID = m_aryInsuredPartiesToBill.GetAt(0);
			m_aryInsuredPartiesToBill.RemoveAt(0);

			// (j.gruber 2016-03-22 10:12) - PLID 68006 - Change bFromEMR to an Enum
			m_BillingDlg->OpenWithBillID(-1, BillEntryType::Bill, 1, BillFromType::EMR);
			m_BillingDlg->PostMessage(NXM_BILL_EMR_FOR_INSUREDPARTY, m_nCurrEMNID, nInsuredPartyID);

			return true;
		}
	}NxCatchAll(__FUNCTION__);

	return false;
}

void CEMNBillController::SubscribeToEvents(CWnd *pWnd)
{
	if (!pWnd) {
		ASSERT(FALSE);
		return;
	}
	
	ASSERT(!m_eventSubscribers.count(pWnd));
	
	m_eventSubscribers.insert(pWnd);
}

void CEMNBillController::UnsubscribeFromEvents(CWnd *pWnd)
{
	m_eventSubscribers.erase(pWnd);
}

void CEMNBillController::PostEvent(UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	foreach(CWnd* pWnd, m_eventSubscribers) {
		pWnd->PostMessage(nMessage, wParam, lParam);
	}
}

LRESULT CEMNBillController::OnPostEditBill(WPARAM wParam, LPARAM lParam)
{
	try{
		if(!m_aryInsuredPartiesToBill.IsEmpty()){
			if(CanWeBill(m_nCurrEMNID)){
				// Yes, we can!
				if(m_BillingDlg) {
					m_BillingDlg->m_pFinancialDlg = this;
					m_BillingDlg->m_nPatientID = m_nPatientID;

					long nInsuredPartyID = m_aryInsuredPartiesToBill.GetAt(0);
					m_aryInsuredPartiesToBill.RemoveAt(0);

					// (j.jones 2011-06-30 09:00) - PLID 43770 - track if we are being created from an EMR
					// (j.gruber 2016-03-22 10:12) - PLID 68006 - Change bFromEMR to an Enum
					m_BillingDlg->OpenWithBillID(-1, BillEntryType::Bill, 1, BillFromType::EMR);
					m_BillingDlg->PostMessage(NXM_BILL_EMR_FOR_INSUREDPARTY, m_nCurrEMNID, nInsuredPartyID);
				}
			}
		}else{
			Reset();
			PostEvent(NXM_POST_EDIT_BILL, wParam, lParam);
		}
	}NxCatchAll(__FUNCTION__);
	return 0;
}

// (j.dinatale 2012-01-20 15:34) - PLID 47539 - handle when a bill was cancelled
LRESULT CEMNBillController::OnPostCancelBill(WPARAM wParam, LPARAM lParam)
{
	try{
		Reset();
		PostEvent(NXM_POST_CANCEL_BILL, wParam, lParam);
	}NxCatchAll(__FUNCTION__);
	return 0;
}

bool CEMNBillController::NeedToBillPartials(long nEMNID)
{
	BOOL bRequireEMRChargeResps = GetRemotePropertyInt("RequireEMRChargeResps", 0, 0, "<None>", true);

	if(bRequireEMRChargeResps){
		return true;
	}

	/*ADODB::_RecordsetPtr rsCounts = CreateParamRecordset(
		"SELECT "
		"(SELECT COUNT(*) FROM EMRChargesT "
		"INNER JOIN EMRChargeRespT ON EMRChargesT.ID = EMRChargeRespT.EMRChargeID "
		"WHERE EMRID = {INT} AND EMRChargesT.Deleted = 0) AS Assigned, "
		"(SELECT COUNT(*) FROM EMRChargesT "
		"LEFT JOIN EMRChargeRespT ON EMRChargesT.ID = EMRChargeRespT.EMRChargeID "
		"WHERE EMRID = {INT} AND EMRChargeID IS NULL AND EMRChargesT.Deleted = 0 "
		") AS Unassigned"
		, nEMNID, nEMNID);*/

	// (j.dinatale 2012-01-27 15:58) - PLID 47539 - Exclude non billable codes
	ADODB::_RecordsetPtr rsCounts = CreateParamRecordset(
		"SELECT "
		"(SELECT COUNT(*) FROM EMRChargesT "
		"INNER JOIN EMRChargeRespT ON EMRChargesT.ID = EMRChargeRespT.EMRChargeID "
		"LEFT JOIN CPTCodeT ON EMRChargesT.ServiceID = CPTCodeT.ID "
		"WHERE EMRID = {INT} AND EMRChargesT.Deleted = 0 "
		"AND (CPTCodeT.Billable Is Null OR CPTCodeT.Billable = 1) "
		") AS Assigned "
		, nEMNID);

	long nAssigned = 0;
	//long nUnassigned = 0;

	if(!rsCounts->eof){
		nAssigned = AdoFldLong(rsCounts, "Assigned", 0);
		//nUnassigned = AdoFldLong(rsCounts, "Unassigned", 0);
	}

	return nAssigned > 0;
}

bool CEMNBillController::HasUnassignedCharges(long nEMNID)
{
	// (j.dinatale 2012-01-19 10:18) - PLID 47539 - have to make sure we look at only products or billable codes only
	return !!(ReturnsRecordsParam(
		"SELECT TOP 1 1 FROM EMRChargesT "
		"LEFT JOIN EMRChargeRespT ON EMRChargesT.ID = EMRChargeRespT.EMRChargeID "
		"LEFT JOIN CPTCodeT ON EMRChargesT.ServiceID = CPTCodeT.ID "
		"WHERE EMRChargesT.EMRID = {INT} AND EMRChargeRespT.EMRChargeID IS NULL AND "
		"(CPTCodeT.Billable Is Null OR CPTCodeT.Billable = 1) AND EMRChargesT.Deleted = 0 ", nEMNID));
}

// (j.dinatale 2012-01-20 09:32) - PLID 47539 - need to have a method to check if there are inactive insurances assigned to EMR charges
bool CEMNBillController::HasInactiveInsuranceAssigned(long nEMNID, bool bExcludeBilled /*= true */)
{
	// (j.dinatale 2012-01-27 15:08) - PLID 47539 - need to make sure we exclude deleted and corrected bills
	if(bExcludeBilled){
		return !!(ReturnsRecordsParam(
			"SELECT TOP 1 1 FROM EMRChargesT "
			"LEFT JOIN EMRChargeRespT ON EMRChargesT.ID = EMRChargeRespT.EMRChargeID "
			"LEFT JOIN CPTCodeT ON EMRChargesT.ServiceID = CPTCodeT.ID "
			"LEFT JOIN InsuredPartyT ON EMRChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "
			"LEFT JOIN "
			"( "
			"	SELECT BillsT.Deleted, BilledEMNsT.* FROM BilledEMNsT "
			"	LEFT JOIN BillsT ON BilledEMNsT.BillID = BillsT.ID "
			"	LEFT JOIN BillCorrectionsT ON BillsT.ID = BillCorrectionsT.OriginalBillID "
			"	WHERE BillCorrectionsT.OriginalBillID IS NULL AND BillsT.Deleted = 0 AND BillsT.EntryType = 1 "
			") BilledEMNsT ON BilledEMNsT.EMNID = EMRChargesT.EMRID "
			"	AND (EMRChargeRespT.EMRChargeID Is Null OR COALESCE(EMRChargeRespT.InsuredPartyID, -1) = COALESCE(BilledEMNsT.InsuredPartyID, -1)) "
			"WHERE EMRChargesT.EMRID = {INT} AND InsuredPartyT.RespTypeID = -1 AND BilledEMNsT.BillID IS NULL AND "
			"(CPTCodeT.Billable Is Null OR CPTCodeT.Billable = 1) AND EMRChargesT.Deleted = 0 ", nEMNID));
	}else{
		return !!(ReturnsRecordsParam(
			"SELECT TOP 1 1 FROM EMRChargesT "
			"LEFT JOIN EMRChargeRespT ON EMRChargesT.ID = EMRChargeRespT.EMRChargeID "
			"LEFT JOIN CPTCodeT ON EMRChargesT.ServiceID = CPTCodeT.ID "
			"LEFT JOIN InsuredPartyT ON EMRChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "
			"WHERE EMRChargesT.EMRID = {INT} AND InsuredPartyT.RespTypeID = -1 AND "
			"(CPTCodeT.Billable Is Null OR CPTCodeT.Billable = 1) AND EMRChargesT.Deleted = 0 ", nEMNID));
	}
}

// (j.dinatale 2012-01-23 14:22) - PLID 47620
bool CEMNBillController::HasBeenFullyBilled(long nEMNID)
{
	if(!ReturnsRecordsParam("SELECT TOP 1 1 FROM EMRChargesT WHERE EMRID = {INT} AND Deleted = 0", nEMNID)){
		return false;
	}

	return !(ReturnsRecordsParam(
		"SELECT TOP 1 1 FROM ( "
		"	SELECT DISTINCT EMRChargesT.EMRID FROM "
		"	EMRChargesT "
		"	LEFT JOIN EMRChargeRespT ON EMRChargesT.ID = EMRChargeRespT.EMRChargeID "
		"	LEFT JOIN CPTCodeT ON EMRChargesT.ServiceID = CPTCodeT.ID "
		"	LEFT JOIN "
		"	( "
		"		SELECT BillsT.Deleted, BilledEMNsT.* FROM BilledEMNsT "
		"		LEFT JOIN BillsT ON BilledEMNsT.BillID = BillsT.ID "
		"		LEFT JOIN BillCorrectionsT ON BillsT.ID = BillCorrectionsT.OriginalBillID "
		"		WHERE BillCorrectionsT.OriginalBillID IS NULL AND BillsT.Deleted = 0 AND BillsT.EntryType = 1 "
		"	) BilledEMNsT ON BilledEMNsT.EMNID = EMRChargesT.EMRID "
		"		AND (EMRChargeRespT.EMRChargeID Is Null OR COALESCE(EMRChargeRespT.InsuredPartyID, -1) = COALESCE(BilledEMNsT.InsuredPartyID, -1)) "
		"	WHERE "
		"	(CPTCodeT.Billable Is Null OR CPTCodeT.Billable = 1) "
		"	AND EMRChargesT.Deleted = 0 "
		"	AND BilledEMNsT.BillID IS NULL "
		"	AND EMRChargesT.EMRID IN (SELECT EMRID FROM EMRChargesT INNER JOIN EMRChargeRespT ON EMRChargesT.ID = EMRChargeRespT.EMRChargeID) "
		"	UNION "
		"	SELECT DISTINCT EMRChargesT.EMRID FROM "
		"	EMRChargesT "
		"	LEFT JOIN EMRChargeRespT ON EMRChargesT.ID = EMRChargeRespT.EMRChargeID "
		"	LEFT JOIN CPTCodeT ON EMRChargesT.ServiceID = CPTCodeT.ID "
		"	LEFT JOIN "
		"	( "
		"		SELECT BillsT.Deleted, BilledEMNsT.* FROM BilledEMNsT "
		"		LEFT JOIN BillsT ON BilledEMNsT.BillID = BillsT.ID "
		"		LEFT JOIN BillCorrectionsT ON BillsT.ID = BillCorrectionsT.OriginalBillID "
		"		WHERE BillCorrectionsT.OriginalBillID IS NULL AND BillsT.Deleted = 0 AND BillsT.EntryType = 1 "
		"	) BilledEMNsT ON BilledEMNsT.EMNID = EMRChargesT.EMRID "
		"	WHERE "
		"	(CPTCodeT.Billable Is Null OR CPTCodeT.Billable = 1) "
		"	AND EMRChargesT.Deleted = 0 "
		"	AND EMRChargesT.EMRID NOT IN (SELECT EMRID FROM EMRChargesT INNER JOIN EMRChargeRespT ON EMRChargesT.ID = EMRChargeRespT.EMRChargeID)	AND BilledEMNsT.BillID IS NULL "
		"	AND BilledEMNsT.BillID IS NULL "
		") UnbilledEMNsQ "
		"WHERE UnbilledEMNsQ.EMRID = {INT}", nEMNID));
}