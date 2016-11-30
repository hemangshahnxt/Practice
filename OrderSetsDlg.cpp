// OrderSetsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PracticeRc.h"
#include "OrderSetsDlg.h"
#include "LabsSetupDlg.h"
#include "SelectDlg.h"
#include "LabEntryDlg.h"
#include "PrescriptionEditDlg.h"
#include "ReferralOrderEntryDlg.h"
#include "MultiSelectDlg.h"
#include "MedicationSelectDlg.h"
#include "AuditTrail.h"
#include "PrescriptionUtilsAPI.h"	// (j.jones 2013-03-27 17:25) - PLID 55920 - we really do need the API version here

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums and related code

using namespace NXDATALIST2Lib;

// (a.walling 2010-01-21 16:43) - PLID 37025 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



// COrderSetsDlg dialog
// (z.manning 2009-05-07 16:16) - PLID 28554 - Created

#define ORDER_SET_TYPE_TEXT		"Order Set"
#define MEDICATION_TYPE_TEXT	"Medication"
#define LAB_TYPE_TEXT			"Lab"
#define REF_ORDER_TYPE_TEXT		"Referral Order"

BOOL DeleteOrderSet(const long nOrderSetID, const long nPatientID, CWnd *pwndParent)
{
	int nMsgBoxResult = pwndParent->MessageBox(
		"Deleting an order set will only remove the order set but not any of its components.\r\n\r\n"
		"Are you sure you want to delete this order set?", NULL, MB_ICONQUESTION|MB_YESNO);
	if(nMsgBoxResult != IDYES) {
		return FALSE;
	}

	ExecuteParamSql(
		"UPDATE LabsT SET OrderSetID = NULL WHERE OrderSetID = {INT}; \r\n"
		"UPDATE PatientMedications SET OrderSetID = NULL WHERE OrderSetID = {INT}; \r\n"
		"UPDATE ReferralOrdersT SET OrderSetID = NULL WHERE OrderSetID = {INT}; \r\n"
		"DELETE FROM OrderSetsT WHERE ID = {INT}; \r\n"
		, nOrderSetID, nOrderSetID, nOrderSetID, nOrderSetID);

	AuditEvent(nPatientID, GetExistingPatientName(nPatientID), BeginNewAuditEvent(GetCurrentUserName())
		, aeiPatientOrderSetDeleted, nOrderSetID, "", "<Deleted>", aepLow);

	return TRUE;
}

IMPLEMENT_DYNAMIC(COrderSetsDlg, CNxDialog)

COrderSetsDlg::COrderSetsDlg(const long nPatientID, CWnd* pParent)
	: CNxDialog(COrderSetsDlg::IDD, pParent), m_nPatientID(nPatientID)
{
	m_bNeedToCloseDlg = FALSE; // (c.haag 2010-07-16 10:55) - PLID 34338 - Because the main frame
											// now manages lab entry dialogs, we need to track whether we need to
											// close or not with a member variable
	m_bHasLabDataChanged = FALSE; // And also track whether data changed
}

COrderSetsDlg::~COrderSetsDlg()
{
}

void COrderSetsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDCANCEL, m_btnClose);
	DDX_Control(pDX, IDC_NEW_ORDER_SET, m_btnNewOrderSet);
	DDX_Control(pDX, IDC_ORDER_SET_BACKGROUND, m_nxcolor);
}


BEGIN_MESSAGE_MAP(COrderSetsDlg, CNxDialog)
	ON_BN_CLICKED(IDC_NEW_ORDER_SET, &COrderSetsDlg::OnBnClickedNewOrderSet)
	ON_MESSAGE(NXM_LAB_ENTRY_DLG_CLOSED, OnLabEntryDlgClosed)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(COrderSetsDlg, CNxDialog)
	ON_EVENT(COrderSetsDlg, IDC_ORDER_SET_LIST, 18, COrderSetsDlg::RequeryFinishedOrderSetList, VTS_I2)
	ON_EVENT(COrderSetsDlg, IDC_ORDER_SET_LIST, 19, COrderSetsDlg::LeftClickOrderSetList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(COrderSetsDlg, IDC_ORDER_SET_LIST, 6, COrderSetsDlg::RButtonDownOrderSetList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()


// COrderSetsDlg message handlers

BOOL COrderSetsDlg::OnInitDialog()
{
	try
	{
		CNxDialog::OnInitDialog();

		m_pdlOrderSets = BindNxDataList2Ctrl(IDC_ORDER_SET_LIST, false);
		// (z.manning 2009-05-08 10:06) - This one query should auto-load the tree structure for all of
		// the current patient's order sets.
		// (z.manning 2009-06-18 11:27) - 28554 - Note: I converted the ntext fields to nvarchar(4000) here
		// because this query fails in SQL 2000 otherwise. If any of these fields happen to be longer than
		// 4,000 characters, then we're just going to have to live with not displaying it all. I don't think
		// that either of these fields should have been ntext to begin with (nor should any fields).
		//TES 11/10/2009 - PLID 36260 - Replaced AnatomySide with AnatomyQualifierID
		//TES 12/8/2009 - PLID 36512 - Restored AnatomySide
		// (z.manning 2010-04-30 15:38) - PLID 37553 - We now have a view to pull anatomic location
		//TES 8/5/2011 - PLID 44901 - Filter on permissioned locations
		m_pdlOrderSets->PutFromClause(_bstr_t(FormatString("\r\n"
			"-- Order sets \r\n"
			"(SELECT OrderSetsT.ID AS OrderSetID, NULL AS ParentID, NULL AS ObjectID, '%s' AS Type, \r\n"
			"	OrderSetTemplatesT.Name AS Description, '' AS Instructions, InputDate \r\n"
			"FROM OrderSetsT \r\n"
			"LEFT JOIN OrderSetTemplatesT ON OrderSetsT.OrderSetTemplateID = OrderSetTemplatesT.ID \r\n"
			"WHERE PatientID = %li \r\n"
			"UNION ALL \r\n"
			"-- Medications \r\n"
			"SELECT NULL AS OrderSetID, OrderSetID AS ParentID, PatientMedications.ID AS ObjectID, '%s' AS Type, \r\n"
			"	DrugList.DrugName AS Description, CONVERT(nvarchar(4000), PatientInstructions) AS Instructions, NULL AS InputDate \r\n"
			"FROM PatientMedications \r\n"
			"LEFT JOIN DrugList ON MedicationID = DrugList.ID \r\n"
			"WHERE PatientID = %li AND OrderSetID IS NOT NULL AND PatientMedications.Deleted = 0 \r\n"
			"UNION ALL \r\n"
			"-- Labs \r\n"
			"SELECT NULL AS OrderSetID, OrderSetID AS ParentID, LabsT.ID AS ObjectID, '%s' AS Type, \r\n"
			"	FormNumberTextID + ' - ' + CASE WHEN LabsT.Specimen IS NULL THEN '' ELSE LabsT.Specimen END + ' - ' +  \r\n"
			"		CASE WHEN LabsT.Type = %i THEN \r\n"
			"			LabAnatomicLocationQ.AnatomicLocation \r\n"
			"			ELSE LabsT.ToBeOrdered END AS Description, CONVERT(nvarchar(4000), LabsT.Instructions) AS Instructions, NULL AS InputDate \r\n"
			"FROM LabsT \r\n"
			"LEFT JOIN LabAnatomicLocationQ ON LabsT.ID = LabAnatomicLocationQ.LabID \r\n"
			"WHERE OrderSetID IS NOT NULL AND PatientID = %li AND LabsT.Deleted = 0 AND %s\r\n"
			"UNION ALL \r\n"
			"-- Referral orders \r\n"
			"SELECT NULL AS OrderSetID, OrderSetID AS ParentID, ReferralOrdersT.ID AS ObjectID, '%s' AS Type, \r\n"
			"	RefPhys.Last + ', ' + RefPhys.First + ' ' + RefPhys.Middle + ' ' + RefPhys.Title AS Description, Reason AS Instructions, NULL AS InputDate \r\n"
			"FROM ReferralOrdersT \r\n"
			"LEFT JOIN PersonT RefPhys ON ReferToID = RefPhys.ID \r\n"
			"WHERE PatientID = %li AND OrderSetID IS NOT NULL \r\n"
			") OrderSetQ \r\n"
			, ORDER_SET_TYPE_TEXT, m_nPatientID, MEDICATION_TYPE_TEXT, m_nPatientID
			, LAB_TYPE_TEXT, ltBiopsy, m_nPatientID, GetAllowedLocationClause("LabsT.LocationID"), REF_ORDER_TYPE_TEXT, m_nPatientID)));
		m_pdlOrderSets->Requery();

		m_nxcolor.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
		SetWindowText(FormatString("Order Sets for %s (%li)", GetExistingPatientName(m_nPatientID), GetExistingPatientUserDefinedID(m_nPatientID)));
		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnNewOrderSet.AutoSet(NXB_NEW);

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}
void COrderSetsDlg::OnBnClickedNewOrderSet()
{
	try
	{
		// (z.manning 2009-05-08 10:05) - Open the generic select dialog to allow them to choose an order set template.
		CSelectDlg dlgSelect(this);
		dlgSelect.m_strTitle = "Select Order Set";
		dlgSelect.m_strCaption = "Please select an order set...";
		dlgSelect.m_strFromClause = "OrderSetTemplatesT";
		int nIdColumnArrayIndex;
		DatalistColumn dcID;
		dcID.strField = "OrderSetTemplatesT.ID";
		dcID.strTitle = "ID";
		dcID.nWidth = 0;
		dcID.nStyle = csVisible|csFixedWidth;
		dcID.nSortPriority = -1;
		dcID.bSortAsc = TRUE;
		nIdColumnArrayIndex = dlgSelect.m_arColumns.Add(dcID);
		DatalistColumn dcName;
		dcName.strField = "Name";
		dcName.strTitle = "Order Set";
		dcName.nWidth = -1;
		dcName.nStyle = csVisible|csWidthAuto;
		dcName.nSortPriority = 0;
		dcName.bSortAsc = TRUE;
		dlgSelect.m_arColumns.Add(dcName);

		if (dlgSelect.DoModal() == IDOK)
		{
			// The user made a selection
			const long nOrderSetTemplateID = VarLong(dlgSelect.m_arSelectedValues.GetAt(nIdColumnArrayIndex));

			// (z.manning 2009-05-08 10:04) - Create the order set and get the new ID.
			ADODB::_RecordsetPtr prsInsert = CreateParamRecordset(
				"SET NOCOUNT ON \r\n"
				"INSERT INTO OrderSetsT (PatientID, OrderSetTemplateID) VALUES ({INT}, {INT}) \r\n"
				"SET NOCOUNT OFF \r\n"
				"SELECT convert(int, SCOPE_IDENTITY()) AS OrderSetID \r\n"
				, m_nPatientID, nOrderSetTemplateID);
			const long nOrderSetID = AdoFldLong(prsInsert->GetFields(), "OrderSetID");
			prsInsert->Close();

			// (z.manning 2009-05-08 10:04) - Get all the components for this order set template.
			// (z.manning 2009-05-14 10:23) - PLID 29912 - Also include the name/description of the
			// order set element in this query.
			ADODB::_RecordsetPtr prsObjects = CreateParamRecordset(
				"--Medications \r\n"
				"SELECT MedicationID, DrugList.DrugName \r\n"
				"FROM OrderSetTemplateMedicationsT \r\n"
				"LEFT JOIN DrugList ON MedicationID = DrugList.ID \r\n"
				"WHERE OrderSetTemplateID = {INT}; \r\n"
				"--Labs \r\n"
				"SELECT LabProcedureID, LabProceduresT.Name AS LabName, OrderSetTemplateLabsT.ToBeOrdered \r\n"
				"FROM OrderSetTemplateLabsT \r\n"
				"LEFT JOIN LabProceduresT ON LabProcedureID = LabProceduresT.ID \r\n"
				"WHERE OrderSetTemplateID = {INT}; \r\n"
				"--Referral orders \r\n"
				"SELECT ReferringPhyID, RefPhys.Last + ', ' + RefPhys.First + ' ' + RefPhys.Middle + ' ' + RefPhys.Title AS RefPhysName \r\n"
				"FROM OrderSetTemplateReferralsT \r\n"
				"LEFT JOIN PersonT RefPhys ON ReferringPhyID = RefPhys.ID \r\n"
				"WHERE OrderSetTemplateID = {INT}; \r\n"
				, nOrderSetTemplateID, nOrderSetTemplateID, nOrderSetTemplateID);

			// (z.manning 2009-05-14 14:28) - Ensure permissions
			BOOL bLabPermission = (GetCurrentUserPermissions(bioPatientLabs) & SPT____C_______ANDPASS) != 0;
			BOOL bRefOrderPermission = (GetCurrentUserPermissions(bioReferralOrders) & SPT____C_______ANDPASS) != 0;
			BOOL bMedPermission = (GetCurrentUserPermissions(bioPatientMedication) & SPT____C_______ANDPASS) != 0;

			// (z.manning 2009-05-14 10:22) - PLID 29912 - We need to give users to option to select
			// what components of the order set template they want to include in the patient order set.
			// So let's go through all the different types and load them in the multi select dialog.
			// I added a character to the ID values for each type so we'll know which ID is why type
			// later on in this function.
			CString strSource;
			if(bMedPermission) {
				for(; !prsObjects->eof; prsObjects->MoveNext()) {
					const long nMedicationID = AdoFldLong(prsObjects->GetFields(), "MedicationID");
					CString strMedID = "M" + AsString(nMedicationID);
					CString strMedName = AdoFldString(prsObjects->GetFields(), "DrugName", "");
					strMedName.Replace(';', '_');
					strSource += strMedID + ';' + "Medication - " + strMedName + ';';
				}
			}
			prsObjects = prsObjects->NextRecordset(NULL);
			if(bLabPermission && g_pLicense->CheckForLicense(CLicense::lcLabs,CLicense::cflrSilent)) {
				for(; !prsObjects->eof; prsObjects->MoveNext()) {
					const long nLabProcedureID = AdoFldLong(prsObjects->GetFields(), "LabProcedureID");
					CString strToBeOrdered = AdoFldString(prsObjects->GetFields(), "ToBeOrdered", "");
					CString strLabID = "L" + AsString(nLabProcedureID) + '_' + strToBeOrdered;
					CString strLabName = AdoFldString(prsObjects->GetFields(), "LabName", "");
					strLabName.Replace(';', '_');
					strSource += strLabID + ';' + "Lab - " + strLabName + " - " + strToBeOrdered + ';';
				}
			}
			prsObjects = prsObjects->NextRecordset(NULL);
			if(bRefOrderPermission) {
				for(; !prsObjects->eof; prsObjects->MoveNext()) {
					const long nRefPhysID = AdoFldLong(prsObjects->GetFields(), "ReferringPhyID");
					CString strRefID = "R" + AsString(nRefPhysID);
					CString strRefPhysName = AdoFldString(prsObjects->GetFields(), "RefPhysName", "");
					strRefPhysName.Replace(';', '_');
					strSource += strRefID + ';' + "Referral - " + strRefPhysName + ';';
				}
			}
			prsObjects->Close();

			// (z.manning 2009-05-14 15:06) - Do we have any components?
			if(strSource.IsEmpty()) {
				MessageBox("This order set is empty or you do not have access to any of its components.");
				ExecuteParamSql("DELETE FROM OrderSetsT WHERE ID = {INT}", nOrderSetID);
				return;
			}

			// (z.manning 2009-05-14 10:30) - PLID 29912 - Now open the multi select dialog and let them 
			// select the individual components they want to include in this patient order set.
			// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
			CMultiSelectDlg dlgMultiSelect(this, "OrderSetsT");
			dlgMultiSelect.m_bPreSelectAll = TRUE;
			CVariantArray aryvarIDsToSkip, aryvarSelectedIDs;
			if(IDOK == dlgMultiSelect.OpenWithDelimitedComboSource(_bstr_t(strSource), aryvarIDsToSkip, "Select the components you want to include in the order set.")) {
				dlgMultiSelect.FillArrayWithIDs(aryvarSelectedIDs);
			}
			else {
				// (z.manning 2009-05-14 10:15) - PLID 29912 - Silently cancel this order set
				ExecuteParamSql("DELETE FROM OrderSetsT WHERE ID = {INT}", nOrderSetID);
				return;
			}

			BOOL bSomethingInOrderSet = FALSE;
			BOOL bNeedToCloseDialog = FALSE;

			// (z.manning 2009-05-08 09:59) - Go through all the elements in the order set template
			for(int nElementIndex = 0; nElementIndex < aryvarSelectedIDs.GetSize(); nElementIndex++)
			{
				// (z.manning 2009-05-14 11:02) - PLID 29912 - Get the selected IDs from the multi select
				// dialog and then get the first character of that ID to determine what the component
				// type is.
				CString strID = VarString(aryvarSelectedIDs.GetAt(nElementIndex));
				char chType = strID.GetAt(0);
				strID.Delete(0, 1);
				CString strExtraText;
				int nUnderscore = strID.Find('_');
				if(nUnderscore != -1) {
					strExtraText = strID.Mid(nUnderscore + 1);
					strID.Delete(nUnderscore, strID.GetLength() - nUnderscore);
				}
				const long nObjectID = atol(strID);

				switch(chType)
				{
					case 'M': // Medication
					{
						if(AddMedicationToOrderSet(nOrderSetID, nObjectID)) {
							bSomethingInOrderSet = TRUE;
						}
					}
					break;

					case 'L': // Lab
					{
						if(AddLabToOrderSet(nOrderSetID, nObjectID, strExtraText, bNeedToCloseDialog)) {
							bSomethingInOrderSet = TRUE;
						}
					}
					break;

					case 'R': // Referral Order
					{
						if(AddReferralOrderToOrderSet(nOrderSetID, nObjectID, bNeedToCloseDialog)) {
							bSomethingInOrderSet = TRUE;
						}
					}
					break;

					default:
						AfxThrowNxException("Unexpected order set element type");
					break;
				}
			}

			if(bSomethingInOrderSet) {
				AuditEvent(m_nPatientID, GetExistingPatientName(m_nPatientID), BeginNewAuditEvent(GetCurrentUserName()),
					aeiPatientOrderSetCreated, nOrderSetID, "", "<Created>", aepLow);
			}

			if(bNeedToCloseDialog) {
				EndDialog(IDCANCEL);
			}
			else if(bSomethingInOrderSet) {
				m_pdlOrderSets->Requery();
			}
			else {
				// (z.manning 2009-05-08 10:04) - If this order set is empty then there's no point to
				// it existing.
				MessageBox("There are no components in this order set so it will not be created.");
				ExecuteParamSql("DELETE FROM OrderSetsT WHERE ID = {INT}", nOrderSetID);
			}
		}

	}NxCatchAll(__FUNCTION__);
}

void COrderSetsDlg::RequeryFinishedOrderSetList(short nFlags)
{
	try
	{
		// (z.manning 2009-05-13 12:37) - Expand all the rows by default
		for(IRowSettingsPtr pRow = m_pdlOrderSets->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
		{
			pRow->PutExpanded(VARIANT_TRUE);

			if(IsOrderSetRow(pRow)) {
				pRow->PutBackColor(RGB(235,235,255));
			}
		}

	}NxCatchAll(__FUNCTION__);
}

void COrderSetsDlg::LeftClickOrderSetList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		long nOrderSetID = -1;
		if(IsOrderSetRow(pRow)) {
			nOrderSetID = VarLong(pRow->GetValue(oscOrderSetID));
		}
		else {
			IRowSettingsPtr pParentRow = pRow->GetParentRow();
			if(pParentRow != NULL) {
				if(IsOrderSetRow(pParentRow)) {
					nOrderSetID = VarLong(pParentRow->GetValue(oscOrderSetID));
				}
			}
		}

		const long nObjectID = VarLong(pRow->GetValue(oscObjectID), -1);

		switch(nCol)
		{
			case oscDescription:
				CString strType = VarString(pRow->GetValue(oscType), "");
				if(strType == MEDICATION_TYPE_TEXT) {
					if(CheckCurrentUserPermissions(bioPatientMedication, sptWrite)) {
						CPrescriptionEditDlg dlgMed(this);
						dlgMed.SetOrderSetID(nOrderSetID);
						// (b.savon 2013-03-18 17:27) - PLID 55477
						// (j.jones 2012-11-16 11:34) - PLID 53765 - this now returns an enum of whether we edited or deleted a prescription
						// (j.fouts 2013-03-12 10:15) - PLID 52973 - Seperated Loading a prescription out from the prescription Edit Dlg
						PrescriptionInfo rxInformation = LoadFullPrescription(nObjectID);

						// (j.jones 2016-01-06 15:40) - PLID 67823 - filled bIsNewRx
						PrescriptionDialogReturnValue epdrvReturn = (PrescriptionDialogReturnValue)dlgMed.EditPrescription(false, rxInformation);
						//check whether we deleted, or really made a change
						if(epdrvReturn == epdrvDeleteRx || dlgMed.GetChangesMade()) {
							m_pdlOrderSets->Requery();
						}
					}
				}
				if(strType == LAB_TYPE_TEXT) {
					if(CheckCurrentUserPermissions(bioPatientLabs, sptWrite)) {
						// (c.haag 2010-07-16 9:51) - PLID 34338 - New way of opening labs. Legacy code commented out.						
						int result = GetMainFrame()->OpenLab(this, -1, -1, ltInvalid, nObjectID, nOrderSetID, "", -1, FALSE, TRUE, GetSafeHwnd());
						if (m_bNeedToCloseDlg) {
							EndDialog(IDCANCEL);
						}
						else if (m_bHasLabDataChanged) {
							m_pdlOrderSets->Requery();
						}
						m_bHasLabDataChanged = FALSE;
						/*
						//TES 11/25/2009 - PLID 36193 - We set the initial lab ID now, since the dialog may have multiple labs on it
						dlgLab.SetInitialLabID(nObjectID);
						dlgLab.SetOrderSetID(nOrderSetID);
						dlgLab.DoModal();
						if(dlgLab.HasOpenedReport()) {
							EndDialog(IDCANCEL);
						}
						else if(dlgLab.HasDataChanged()) {
							m_pdlOrderSets->Requery();
						}
						*/
					}
				}
				if(strType == REF_ORDER_TYPE_TEXT) {
					if(CheckCurrentUserPermissions(bioReferralOrders, sptWrite)) {
						CReferralOrderEntryDlg dlgRef(m_nPatientID, this);
						dlgRef.SetOrderSetID(nOrderSetID);
						if(IDOK == dlgRef.EditExistingReferralOrder(nObjectID)) {
							if(dlgRef.m_bCloseParent) {
								EndDialog(IDCANCEL);
							}
							else {
								m_pdlOrderSets->Requery();
							}
						}
					}
				}
				break;
		}

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2009-05-14 15:43) - PLID 34269
void COrderSetsDlg::RButtonDownOrderSetList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		m_pdlOrderSets->PutCurSel(pRow);

		CMenu mnu;
		mnu.CreatePopupMenu();

		enum MenuItems {
			miAddMed = 1,
			miAddLab,
			miAddRefOrder,
			miDelete,
			miRemoveFromOrderSet,
		};

		// (z.manning 2009-05-14 16:17) - PLID 34269 - Only order set rows have a menu option, at least for now.
		long nObjectID = -1;
		if(IsOrderSetRow(pRow))
		{
			nObjectID = VarLong(pRow->GetValue(oscOrderSetID));
			// (z.manning 2009-05-14 16:26) - PLID 34269 - Give them the option to add any order set
			// components to an existing order set.
			mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, miAddMed, "Add &Medication to Order Set");
			if(g_pLicense->CheckForLicense(CLicense::lcLabs, CLicense::cflrSilent)) {
				mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, miAddLab, "Add &Lab to Order Set");
			}
			mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, miAddRefOrder, "Add &Referral Order to Order Set");
			mnu.AppendMenu(MF_SEPARATOR);
			mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, miDelete, "&Delete Order Set");
		}
		else
		{
			nObjectID = VarLong(pRow->GetValue(oscObjectID));
			mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, miRemoveFromOrderSet, "&Remove from Order Set");
		}



		CPoint pt;
		GetCursorPos(&pt);
		switch(mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this))
		{
			case miAddMed:
			{
				// (z.manning 2009-05-14 16:26) - PLID 34269 - Add a new medication
				CMedicationSelectDlg dlgSelectMed(this);
				dlgSelectMed.SetPatientID(m_nPatientID);
				if(dlgSelectMed.DoModal() == IDOK) {
					if(AddMedicationToOrderSet(nObjectID, dlgSelectMed.m_nMedicationID)) {
						m_pdlOrderSets->Requery();
					}
				}
			}
			break;

			case miAddLab:
			{
				// (z.manning 2009-05-14 16:26) - PLID 34269 - Add a new lab
				CSelectDlg dlgSelect(this);
				dlgSelect.m_strTitle = "Select Lab Procedure";
				dlgSelect.m_strCaption = "Please select a lab procedure...";
				dlgSelect.m_strFromClause = "LabProceduresT";
				dlgSelect.m_strWhereClause = "Inactive = 0";
				int nIdColumnArrayIndex;
				DatalistColumn dcID;
				dcID.strField = "ID";
				dcID.strTitle = "ID";
				dcID.nWidth = 0;
				dcID.nStyle = csVisible|csFixedWidth;
				dcID.nSortPriority = -1;
				dcID.bSortAsc = TRUE;
				nIdColumnArrayIndex = dlgSelect.m_arColumns.Add(dcID);
				DatalistColumn dcName;
				dcName.strField = "Name";
				dcName.strTitle = "Lab Procedure";
				dcName.nWidth = -1;
				dcName.nStyle = csVisible|csWidthAuto;
				dcName.nSortPriority = 0;
				dcName.bSortAsc = TRUE;
				dlgSelect.m_arColumns.Add(dcName);
				if(dlgSelect.DoModal() == IDOK) {
					const long nLabProcID = VarLong(dlgSelect.m_arSelectedValues.GetAt(nIdColumnArrayIndex));
					BOOL bNeedToCloseDialog = FALSE;
					if(AddLabToOrderSet(nObjectID, nLabProcID, "", bNeedToCloseDialog)) {
						if(bNeedToCloseDialog) {
							EndDialog(IDCANCEL);
						}
						else {
							m_pdlOrderSets->Requery();
						}
					}
				}
			}
			break;

			case miAddRefOrder:
			{
				// (z.manning 2009-05-14 16:26) - PLID 34269 - Add a new referral order
				BOOL bNeedToCloseDialog = FALSE;
				if(AddReferralOrderToOrderSet(nObjectID, -1, bNeedToCloseDialog)) {
					if(bNeedToCloseDialog) {
						EndDialog(IDCANCEL);
					}
					else {
						m_pdlOrderSets->Requery();
					}
				}
			}
			break;

			case miDelete:
				if(DeleteOrderSet(nObjectID, m_nPatientID, this)) {
					m_pdlOrderSets->Requery();
				}
			break;

			case miRemoveFromOrderSet:
			{
				CString strType = VarString(pRow->GetValue(oscType), "");
				if(strType == MEDICATION_TYPE_TEXT) {
					ExecuteParamSql("UPDATE PatientMedications SET OrderSetID = NULL WHERE ID = {INT}", nObjectID);
				}
				else if(strType == LAB_TYPE_TEXT) {
					ExecuteParamSql("UPDATE LabsT SET OrderSetID = NULL WHERE ID = {INT}", nObjectID);
				}
				else if(strType == REF_ORDER_TYPE_TEXT) {
					ExecuteParamSql("UPDATE ReferralOrdersT SET OrderSetID = NULL WHERE ID = {INT}", nObjectID);
				}
				else {
					ASSERT(FALSE);
					return;
				}
				m_pdlOrderSets->RemoveRow(pRow);
			}
			break;
		}

	}NxCatchAll(__FUNCTION__);
}

BOOL COrderSetsDlg::IsOrderSetRow(IRowSettingsPtr pRow)
{
	return (pRow->GetValue(oscOrderSetID).vt == VT_I4);
}

BOOL COrderSetsDlg::AddMedicationToOrderSet(const long nOrderSetID, const long nMedicationID)
{
	if(CheckCurrentUserPermissions(bioPatientMedication, sptCreate))
	{
		CPrescriptionEditDlg dlgMed(this);
		dlgMed.SetOrderSetID(nOrderSetID);

		// (b.savon 2014-08-26 10:38) - PLID 63401 - Spawning a prescription from EMR does not request eligibility or check for formulary.
		long nInsuranceID = -1;
		if (g_pLicense->HasEPrescribing(CLicense::cflrSilent) == CLicense::eptSureScripts){
			nInsuranceID = CheckExistingFormularyData(this, m_nPatientID);
		}
		// (b.savon 2013-03-18 17:13) - PLID 55477
		// (j.jones 2012-11-16 11:34) - PLID 53765 - this now returns an enum of whether we edited or deleted a prescription
		// (j.fouts 2013-03-12 10:15) - PLID 52973 - Seperated Loading a prescription out from the prescription Edit Dlg
		// (b.savon 2014-08-26 10:38) - PLID 63401 - Pass the insurance id
		NexTech_Accessor::_UpdatePresQueueResultsPtr pResults = SaveNewPrescription(nMedicationID, FALSE, NULL, m_nPatientID, -1, GetCurrentLocationID(), COleDateTime::GetTickCount(), -1, 0, SourceActionInfo(), -1, -1, nInsuranceID);

		if( pResults->PrescriptionsAdded == NULL ){
			ThrowNxException("Unable to Add Prescription in OrderSetsDlg::AddMedicationToOrderSet!");
		}

		Nx::SafeArray<IUnknown*> saryPrescriptions(pResults->PrescriptionsAdded);

		if(saryPrescriptions.GetCount() == 0)
		{
			ThrowNxException("Attempted to create a new prescription, but no prescription was returned."); 
		}

		// (b.savon 2013-03-19 17:05) - PLID 55477 - Use struct
		PrescriptionInfo rxInformation;
		NexTech_Accessor::_QueuePrescriptionPtr pPrescription = saryPrescriptions[0];
		rxInformation.pPrescription = pPrescription;
		rxInformation.erxUserRole = pResults->UserRole;
		rxInformation.saryPrescribers = pResults->Prescriber;
		rxInformation.sarySupervisors = pResults->Supervisor;
		rxInformation.saryNurseStaff = pResults->NurseStaff;

		// (j.jones 2013-11-25 09:55) - PLID 59772 - for new prescriptions we pass in the drug interactions info
		DrugInteractionInfo drugInteractionInfo;
		if(pResults->DrugDrugInteracts) {
			drugInteractionInfo.saryDrugDrugInteracts = pResults->DrugDrugInteracts;
		}
		if(pResults->DrugAllergyInteracts) {
			drugInteractionInfo.saryDrugAllergyInteracts = pResults->DrugAllergyInteracts;
		}
		if(pResults->DrugDiagnosisInteracts) {
			drugInteractionInfo.saryDrugDiagnosisInteracts = pResults->DrugDiagnosisInteracts;
		}

		// (j.jones 2016-01-06 15:40) - PLID 67823 - filled bIsNewRx
		PrescriptionDialogReturnValue epdrvReturn = (PrescriptionDialogReturnValue)dlgMed.EditPrescription(true, rxInformation, &drugInteractionInfo);
		if(epdrvReturn != epdrvDeleteRx) {
			// (z.manning 2009-05-14 12:10) - Prescriptions don't have table checkers, but let's at
			// least update the current screen if the user is in the medictions tab.
			if(GetMainFrame() != NULL) {
				CNxTabView *pView = (CNxTabView*)GetMainFrame()->GetOpenView(PATIENT_MODULE_NAME);
				if(pView != NULL) {
					if(pView->GetActiveTab() == PatientsModule::MedicationTab) {
						pView->UpdateView();
					}
				}
			}

			return TRUE;
		}
	}

	return FALSE;
}

BOOL COrderSetsDlg::AddLabToOrderSet(const long nOrderSetID, const long nLabProcedureID, const CString &strToBeOrdered, OUT BOOL &bNeedToCloseDialog)
{
	if(CheckCurrentUserPermissions(bioPatientLabs, sptCreate))
	{
		// (c.haag 2010-07-16 9:51) - PLID 34338 - New way of opening labs. Legacy code commented out.
		// Table checkers will also be available with related changes.
		//TES 8/5/2011 - PLID 44908 - Track whether they commit the lab.
		UINT nReturn = GetMainFrame()->OpenLab(this, m_nPatientID, nLabProcedureID, ltInvalid, -1, nOrderSetID, strToBeOrdered, -1, FALSE, TRUE, GetSafeHwnd());
		bNeedToCloseDialog = m_bNeedToCloseDlg;

		//TES 8/5/2011 - PLID 44908 - Now return the correct value (this was always returning FALSE before)
		return (nReturn != IDCANCEL);

		/*
		dlgLab.SetPatientID(m_nPatientID);
		dlgLab.SetLabProcedureID(nLabProcedureID);
		dlgLab.SetOrderSetID(nOrderSetID);
		dlgLab.SetDefaultToBeOrdered(strToBeOrdered);
		dlgLab.DoModal();
		if(dlgLab.HasOpenedReport()) {
			bNeedToCloseDialog = TRUE;
		}
		if(dlgLab.HasDataChanged())
		{
			// (z.manning 2009-05-14 12:10) - Labs don't have table checkers, but let's at
			// least update the current screen if the user is in the labs tab.
			if(GetMainFrame() != NULL) {
				CNxTabView *pView = (CNxTabView*)GetMainFrame()->GetOpenView(PATIENT_MODULE_NAME);
				if(pView != NULL) {
					if(pView->GetActiveTab() == CPatientView::LabTab) {
						pView->UpdateView();
					}
				}
			}

			return TRUE;
		}*/
	}

	return FALSE;
}

BOOL COrderSetsDlg::AddReferralOrderToOrderSet(const long nOrderSetID, const long nRefPhysID, OUT BOOL &bNeedToCloseDialog)
{
	if(CheckCurrentUserPermissions(bioReferralOrders, sptCreate))
	{
		CReferralOrderEntryDlg dlgReferral(m_nPatientID, this);
		dlgReferral.SetOrderSetID(nOrderSetID);
		dlgReferral.SetDefaultRefPhysID(nRefPhysID);
		if(dlgReferral.DoModal() == IDOK) {
			if(dlgReferral.m_bCloseParent) {
				bNeedToCloseDialog = TRUE;
			}
			return TRUE;
		}
	}

	return FALSE;
}

// (c.haag 2010-07-15 17:23) - PLID 34338 - This message is received when a lab entry dialog is closed
// and the lab was opened from this specific dialog.
LRESULT COrderSetsDlg::OnLabEntryDlgClosed(WPARAM wParam, LPARAM lParam)
{
	try {
		CLabEntryDlg *pDlg = (CLabEntryDlg *)lParam;
		if (pDlg) {
			if (pDlg->HasOpenedReport()) {
				m_bNeedToCloseDlg = TRUE;
			}
			m_bHasLabDataChanged = pDlg->HasDataChanged();
		}
	}
	NxCatchAll(__FUNCTION__)
	return 0;
}
