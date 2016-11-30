#include "stdafx.h"
#include "ReconcileMedicationsUtils.h"
#include "ReconcileMedicationsDlg.h"
#include "AuditTrail.h"
#include "DecisionRuleUtils.h"

// (j.jones 2013-01-09 13:25) - PLID 54530 - created, and moved functions here from MainFrame

// (c.haag 2010-02-18 17:23) - PLID 37384 - Lets the user apply a new prescription to the current medications list
// (j.jones 2010-08-23 09:17) - PLID 40178 - takes in a NewCropGUID
// (j.jones 2011-05-02 15:24) - PLID 43350 - this now takes in a Sig for the current med
// (j.jones 2013-01-08 12:00) - PLID 47303 - added parent wnd
// (j.jones 2013-01-09 11:55) - PLID 54530 - no longer need the Sig nor strNewCropGUID
//TES 10/31/2013 - PLID 59251 - Added arNewCDSInterventions, any interventions triggered in this function will be added to it
BOOL ReconcileCurrentMedicationsWithOneNewPrescription(long nPatientID, long nNewPrescriptionID, OLE_COLOR clrBack, CWnd *pParentWnd, IN OUT CDWordArray &arNewCDSInterventions)
{
	// (j.jones 2013-01-09 13:28) - PLID 54530 - now we just call the function for multiple

	CArray<long, long> aryNewPrescriptionIDs;
	CStringArray astrNewCropRxGUIDs;
	if(nNewPrescriptionID != -1) {
		aryNewPrescriptionIDs.Add(nNewPrescriptionID);
	}
	else {
		ThrowNxException("ReconcileCurrentMedicationsWithOneNewPrescription called with no prescription ID!");
	}

	// (j.jones 2013-01-09 14:41) - PLID 54530 - nothing ever called this with a NewCropGUID,
	// but if it did, this code would suffice, but you would need to remove the above exception
	/*
	if(!strNewCropGUID.IsEmpty()) {
		astrNewCropRxGUIDs.Add(strNewCropGUID);
	}
	*/
	
	//TES 10/31/2013 - PLID 59251 - Pass in arNewCDSInterventions
	return ReconcileCurrentMedicationsWithMultipleNewPrescriptions(nPatientID, aryNewPrescriptionIDs, astrNewCropRxGUIDs, clrBack, pParentWnd, arNewCDSInterventions);
}

// (c.haag 2010-02-18 10:03) - PLID 37424 - Lets the user apply new prescriptions imported from our NewCrop
// integration to the current medications list
// (j.jones 2013-01-08 10:19) - PLID 47302 - added parent wnd
// (j.jones 2013-01-09 11:55) - PLID 54530 - added ability to add multiple prescriptions by prescription ID, not necessarily NewCrop IDs
//TES 10/31/2013 - PLID 59251 - Added arNewCDSInterventions, any interventions triggered in this function will be added to it
BOOL ReconcileCurrentMedicationsWithMultipleNewPrescriptions(long nPatientID, CArray<long, long> &aryNewPrescriptionIDs, const CStringArray& astrNewCropRxGUIDs, OLE_COLOR clrBack, CWnd *pParentWnd, IN OUT CDWordArray &arNewCDSInterventions)
{
	// (j.jones 2013-01-11 13:12) - PLID 54462 - check preferences here
	if(GetRemotePropertyInt("ReconcileNewRxWithCurMeds", 0, 0, "<None>", true) == 0) {
		return FALSE;
	}

	// (j.jones 2013-01-08 10:19) - PLID 47302 - added parent wnd, if one was not provided,
	// make mainframe be the parent
	if(pParentWnd == NULL) {
		pParentWnd = GetMainFrame();

		if(pParentWnd == NULL) {
			ThrowNxException("ReconcileCurrentMedicationsWithMultipleNewPrescriptions called with no valid MainFrame!");
		}
	}

	// (j.jones 2013-01-09 11:55) - PLID 54530 - added ability to add multiple prescriptions by prescription ID, not necessarily NewCrop IDs,
	// but we need one or the other
	if(aryNewPrescriptionIDs.GetSize() == 0 && astrNewCropRxGUIDs.GetSize() == 0) {
		//we have no new prescription IDs or NewCropIDs, so there is nothing to reconcile
		return FALSE;
	}
	else if(aryNewPrescriptionIDs.GetSize() > 0 && astrNewCropRxGUIDs.GetSize() > 0
		&& aryNewPrescriptionIDs.GetSize() != astrNewCropRxGUIDs.GetSize()) {
		//if we have both arrays filled, they must be equal, because we're only going to use one
		ThrowNxException("ReconcileCurrentMedicationsWithMultipleNewPrescriptions called with an uneven count of prescriptions!");
	}

	BOOL bDataChanged = FALSE;
	if(CReconcileMedicationsDlg::CanCurrentUserAccess()) {
		// (j.jones 2012-10-17 16:07) - PLID 51713 - check the patient's HasNoMeds status
		BOOL bCurHasNoMedsStatus = ReturnsRecordsParam("SELECT PersonID FROM PatientsT WHERE HasNoMeds = 1 AND PersonID = {INT}", nPatientID);
		CReconcileMedicationsDlg dlgReconcile(nPatientID, pParentWnd);

		BOOL bContinue = FALSE;
		if(aryNewPrescriptionIDs.GetSize() > 0) {
			dlgReconcile.AddPrescriptionByPMIDs(aryNewPrescriptionIDs);
			bContinue = TRUE;
		}
		else if(astrNewCropRxGUIDs.GetSize() > 0) {
			bContinue = (dlgReconcile.AddPrescriptionsByNewCropRxGUIDs(astrNewCropRxGUIDs) > 0);
		}
		if(bContinue) {
			dlgReconcile.AddCurrentMedicationsFromData();
			dlgReconcile.SetBackColor( clrBack );
			if (IDOK == dlgReconcile.DoModal()) {
				//CReconcileMedicationsDlg::CChangeArray& aRequestedChanges = dlgReconcile.GetRequestedChanges();
				// (s.dhole 2013-06-21 16:38) - PLID 55964
				CReconcileMedicationsDlg::CMergeMedicationArray& aRequestedChanges = dlgReconcile.GetRequestedChanges();
				if (aRequestedChanges.GetSize() > 0) {
					// Commit the changes to data
					CString strSqlBatch = BeginSqlBatch();
					long nAuditTransID = BeginAuditTransaction();
					try {
						BOOL bNewCropReconciliation = FALSE;
						for (int i = 0; i < aRequestedChanges.GetSize(); i++) {
							// (s.dhole 2013-06-21 16:38) - PLID 55964 Change class
							CReconcileMedicationsDlg::MergeMedication ch = aRequestedChanges[i];
							// (s.dhole 2013-06-21 16:38) - PLID 55964 addde merge
							if (ch.Action == ch.eAddMed || ch.Action == ch.eMergeCurMed) {
								// Add a current medication to the patient medications tab from a prescription
								// (j.jones 2010-08-23 09:17) - PLID 40178 - supported NewCropGUID
								CString strNewCropGUID = "NULL";
								if (!ch.strNewCropGUID.IsEmpty()) {
									strNewCropGUID.Format("'%s'", _Q(ch.strNewCropGUID));
								}
								// (c.haag 2010-09-15 11:00) - PLID 40215 - First check whether the medication already exists for the patient
								// as a discontinued value; if so, remove its discontinued status
								// (j.jones 2011-05-02 15:19) - PLID 43450 - supported the Sig
								//// (s.dhole 2013-06-21 11:08) - PLID 40178 If we merging a medication we jus update last update date
								// (j.armen 2013-06-27 17:10) - PLID 57359 - Idenitate CurrentPatientMedsT
								// (s.dhole 2013-11-26 17:33) - PLID 55964 change sql to update medication
								// (b.eyers 2015-12-18) - PLID 67749 - Save the start date if this is a new current med

								AddStatementToSqlBatch(strSqlBatch,
									"IF EXISTS (SELECT MedicationID FROM CurrentPatientMedsT WHERE MedicationID = %d AND PatientID = %d ) BEGIN \r\n "
									"UPDATE CurrentPatientMedsT SET Discontinued = 0, DiscontinuedDate = NULL, "
									"NewCropGUID = %s, Sig = '%s' , LastUpdateDate=GETDATE() "
									"WHERE MedicationID = %d AND PatientID = %d \r\n"
									"END \r\n"
									"IF NOT EXISTS(SELECT MedicationID FROM CurrentPatientMedsT WHERE MedicationID = %d AND PatientID = %d) BEGIN \r\n"
									"INSERT INTO CurrentPatientMedsT (PatientID, MedicationID, NewCropGUID, InputByUserID, Sig,LastUpdateDate, StartDate) "
									"  VALUES ( %d, %d, %s, %d, '%s',GETDATE(), '%s') \r\n"
									"END \r\n"
									, ch.nMedicationID, nPatientID
									, strNewCropGUID, _Q(ch.strPatientExplanation)
									, ch.nMedicationID, nPatientID
									, ch.nMedicationID, nPatientID,
									nPatientID, ch.nMedicationID, strNewCropGUID, GetCurrentUserID(), _Q(ch.strPatientExplanation), FormatDateTimeForSql(ch.dtStartDate, dtoDate));
								// (s.dhole 2013-06-18 12:12) - PLID  56927 - added Merge medication Audit
								if (ch.Action == ch.eMergeCurMed){
									if (ch.strPatientExplanation.CompareNoCase(ch.strCurrentSig)){
										CString strTempFrom = FormatString("%s Sig: %s ", ch.strName, ch.strCurrentSig);
										CString strTempTo = ch.strPatientExplanation;
										AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditTransID, aeiCurrentMedicationMerge, ch.nEMRDataID, strTempFrom, strTempTo, aepHigh, aetCreated);
									}
									if (ch.bIsActive != ch.bCurrentIsActive){
										CString strTempFrom = FormatString("%s: %s ", ch.strName, (ch.bCurrentIsActive ? "Active" : "<Discontinued>"));
										CString strTempTo = FormatString("%s", (ch.bIsActive ? "Active" : "<Discontinued>"));
										AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditTransID, aeiCurrentMedicationMerge, ch.nEMRDataID, strTempFrom, strTempTo, aepHigh, aetCreated);
									}

								}
								else{
									AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditTransID, aeiCurrentPatientMedsAdd, ch.nEMRDataID, "", ch.strName, aepHigh, aetCreated);
								}


								// (s.dhole 2013-06-21 11:13) - PLID  need audit for merge medication
								// (j.jones 2012-10-17 16:07) - PLID 51713 - since we added medications, clear the HasNoMeds status,
								// but only if it is currently set (passed into this function through bCurHasNoMedsStatus)
								if (bCurHasNoMedsStatus) {

									if (-1 == nAuditTransID) {
										nAuditTransID = BeginAuditTransaction();
									}
									AddStatementToSqlBatch(strSqlBatch, "UPDATE PatientsT SET HasNoMeds = 0 WHERE PersonID = %li", nPatientID);
									AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditTransID, aeiPatientHasNoMedicationsStatus, nPatientID, "'No Medications' Status Selected", "'No Medications' Status Cleared", aepMedium, aetChanged);

									//set the status to false, to reflect our change
									bCurHasNoMedsStatus = FALSE;
								}

								// (s.dhole 2013-10-30 15:21) - PLID 62149 save reconciliation action , Only if we are getting medication from NewCrop
								if (astrNewCropRxGUIDs.GetSize() > 0) {
									bNewCropReconciliation = TRUE;

								}
							}
							// (s.dhole 2013-06-21 16:39) - PLID 40178 
							else if (ch.Action == ch.eDeleteMed) {
								// Delete a current medication from the patient medications tab
								AddStatementToSqlBatch(strSqlBatch, "DELETE FROM CurrentPatientMedsT WHERE ID = %li", ch.nCurrentPatientMedsID);
								AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditTransID, aeiCurrentPatientMedsDeleted, ch.nCurrentEMRDataID, ch.strCurrentName, "<Deleted>", aepHigh, aetDeleted);
							}
							else
							{
								// do nothing
							}
						}
						// (s.dhole 2013-10-30 15:21) - PLID 62149 save reconciliation action , Only if we are getting medication from NewCrop
						if (bNewCropReconciliation){
							AddStatementToSqlBatch(strSqlBatch, "INSERT INTO PatientReconciliationT(PatientID, ReconciliationDesc )  VALUES(%li , 'NewCrop Medication Reconciliation') ", nPatientID);
						}

						ExecuteSqlBatch(strSqlBatch);
						CommitAuditTransaction(nAuditTransID);

						// Send a table checker
						CClient::RefreshTable(NetUtils::CurrentPatientMedsT, nPatientID);
						// Warn of discrepancies
						// (j.armen 2012-05-31 14:39) - PLID 50718 - Check if we are licensed using the helper function
						if(g_pLicense->HasEMR(CLicense::cflrSilent, TRUE) == 2) {
							WarnMedicationDiscrepanciesWithEMR(nPatientID);
						}
						// (c.haag 2010-09-21 13:43) - PLID 40610 - Create todo alarms for decisions
						//TES 10/31/2013 - PLID 59251 - Pass in arNewCDSInterventions
						UpdateDecisionRules(GetRemoteData(), nPatientID, arNewCDSInterventions);

						bDataChanged = TRUE;
					}
					catch (...) {
						RollbackAuditTransaction(nAuditTransID);
						throw;
					}

				} // if (aRequestedChanges.GetSize() > 0) {
				else {
					// If we get here, there were no changes to make
				}
			}
			else {
				// User changed their mind
			}
		} else {
			// No prescriptions were added
		}
	}
	else {
		// Preference is turned off, do nothing
	}
	return bDataChanged;
}