// EditAllergyListDlg.cpp : implementation file
//

#include "stdafx.h"
#include "patientDialog.h"
#include "EditAllergyListDlg.h"
#include "GlobalDataUtils.h"
#include "MultiSelectDlg.h"
#include "EmrUtils.h"
#include "AuditTrail.h"
#include "FirstDataBankAllergyImportDlg.h" // (b.savon 2012-08-08 13:53) - PLID 51734
#include "NxException.h" // (b.savon 2012-08-08 13:53) - PLID 51734
#include "PrescriptionUtilsNonAPI.h"	// (j.jones 2013-03-27 17:23) - PLID 55920 - we only need the non-API header here
#include "NxAPI.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;
using namespace NXDATALISTLib;

// (a.walling 2010-01-21 16:43) - PLID 37022 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.


/////////////////////////////////////////////////////////////////////////////
// CEditAllergyListDlg dialog
// (b.savon 2012-09-26 16:59) - PLID 52874 - Add column enum
enum AllergyListColumns{
	alcAllergyID = 0,
	alcAllergyName = 1,
	alcConfiguredInteraction = 2,
	alcInactive = 3,
	alcImportStatus = 4,
	alcConceptID = 5,
	alcConceptIDType = 6,
	alcFDBOutOfDate = 7, //TES 5/10/2013 - PLID 56631
};

CEditAllergyListDlg::CEditAllergyListDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEditAllergyListDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditAllergyListDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEditAllergyListDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditAllergyListDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_EDIT, m_btnEdit);
	DDX_Control(pDX, IDC_DELETE, m_btnDelete);
	DDX_Control(pDX, IDC_ADD, m_btnAdd);
	DDX_Control(pDX, IDC_EDIT_COMBO_BKG, m_nxcBack);
	DDX_Control(pDX, IDC_UPDATE_ALL_ALLERGIES, m_btnUpdateAllAllergies);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditAllergyListDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEditAllergyListDlg)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_EDIT, OnEdit)
	ON_BN_CLICKED(IDC_UPDATE_ALL_ALLERGIES, OnUpdateAllAllergies)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditAllergyListDlg message handlers

BEGIN_EVENTSINK_MAP(CEditAllergyListDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEditAllergyListDlg)
	ON_EVENT(CEditAllergyListDlg, IDC_EDIT_LIST, 10 /* EditingFinished */, OnEditingFinishedEditList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_I4)
	ON_EVENT(CEditAllergyListDlg, IDC_EDIT_LIST, 9 /* EditingFinishing */, OnEditingFinishingEditList, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CEditAllergyListDlg, IDC_EDIT_LIST, 5 /* LButtonUp */, OnLButtonUpEditList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEditAllergyListDlg, IDC_EDIT_LIST, 18 /* RequeryFinished */, OnRequeryFinishedEditList, VTS_I2)
	ON_EVENT(CEditAllergyListDlg, IDC_EDIT_LIST, 6 /* RButtonDown */, OnRButtonDownEditList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

// (b.savon 2012-07-24 14:14) - PLID 51734 - Add option to Import Allergy from FDB
void CEditAllergyListDlg::OnAdd() 
{
	try{
		// (j.fouts 2012-09-25 09:27) - PLID 52825 - Only allow importing if they have the FDB license
		if(g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent)) {
			enum {
				alAddFreeTextAllergy = -10,
				alAddFDBAllergy = -11,		
			};

			CMenu mnu;
			mnu.m_hMenu = CreatePopupMenu();
			long nIndex = 0;
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, alAddFDBAllergy, "Import Allergy");
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, alAddFreeTextAllergy, "Add Free Text Allergy");

			CRect rc;
			CWnd *pWnd = GetDlgItem(IDC_ADD);
			int nResult = 0;
			if (pWnd) {
				pWnd->GetWindowRect(&rc);
				nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, rc.right, rc.top, this, NULL);
			} else {
				CPoint pt;
				GetCursorPos(&pt);
				nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this, NULL);
			}

			switch(nResult) {

				case alAddFreeTextAllergy: {
					OnAddFreeTextAllergy();
					break;
				}

				case alAddFDBAllergy: {
					OnAddImportedFDBAllergy();
					break;
				}
			}
		}
		else
		{
			OnAddFreeTextAllergy();
		}
	}NxCatchAll(__FUNCTION__);	
}

// (b.savon 2012-08-08 13:48) - PLID 51734 - Handle importing a first data bank allergy.
void CEditAllergyListDlg::OnAddImportedFDBAllergy()
{
	try{

		CFirstDataBankAllergyImportDlg dlg(this);
		if( dlg.DoModal() == IDOK ){
			if( dlg.ShouldRequery() ){
				m_pAllergyList->Requery();
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2012-08-08 13:48) - PLID 51734 - Handle adding a free text allergy
void CEditAllergyListDlg::OnAddFreeTextAllergy()
{
	try {

		// (j.fouts 2012-09-25 09:27) - PLID 52825 - Only allow importing if they have the FDB license
		if(g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent)) {
			// (b.savon 2012-08-08 13:48) - PLID 51734 - Make sure the user knows this isn't recommended.
			if (IDYES == MsgBox(MB_YESNO, "In future versions of Practice, you will not be able to"
				" use a Free Text Allergy if you are using E-Prescribing.\n\nIt is recommended that you import a non-Free Text Allergy and use that instead.\n\n"
				"Would you like to CANCEL adding a Free Text Allergy and search for a non-Free Text Allergy instead?")) {
					OnAddImportedFDBAllergy();
					return;
			}
		}

		//Add a new row into the list and set it to be editable
		IRowSettingsPtr pRow;
		pRow = m_pAllergyList->GetRow(-1);

		long nNewID = NewNumber("AllergyT", "ID");
		// (b.savon 2012-09-26 16:59) - PLID 52874 - Replace hard coded with enum and add status
		pRow->PutValue(alcAllergyID, (long)nNewID);
		pRow->PutValue(alcAllergyName, _variant_t("Enter New Allergy"));
		pRow->PutValue(alcConfiguredInteraction, _variant_t("<None>"));
		// (c.haag 2007-04-09 14:56) - PLID 25504 - Inactive column
		pRow->PutValue(alcInactive, VARIANT_FALSE);
		pRow->PutValue(alcImportStatus, _variant_t("Free-Text Allergy"));
		// (b.savon 2012-08-15 14:49) - PLID 51703 - Default free text values
		pRow->PutValue(alcConceptID, (long)-1);
		pRow->PutValue(alcConceptIDType, (long)-1);
		//TES 5/10/2013 - PLID 56631 - Added an FDBOutOfDate column
		pRow->PutValue(alcFDBOutOfDate, g_cvarFalse);

		// (c.haag 2007-04-04 16:34) - PLID 25498 - Add the allergy
		ExecuteAddition(nNewID);
		m_pAllergyList->InsertRow(pRow, -1);
		m_pAllergyList->SetSelByColumn(alcAllergyID, nNewID);
		m_pAllergyList->StartEditing(m_pAllergyList->GetCurSel(), alcAllergyName);

	}NxCatchAll("Error in OnAdd");	
}

void CEditAllergyListDlg::OnDelete() 
{
	//check to see that something is selected
	long nCurSel = m_pAllergyList->GetCurSel();
	
	if (nCurSel != -1) {

		try{

			// (c.haag 2007-04-04 17:28) - PLID 25498 - Allergies are linked with EMR table items.
			// An EMR table item must have at least one active row. Given that, we cannot allow a user
			// to delete the last allergy.
			if (!HasMultipleActiveAllergies())
				return;

			// (b.savon 2012-09-26 17:02) - PLID 52874 - Replace with enum
			long nDelID = VarLong(m_pAllergyList->GetValue(nCurSel, alcAllergyID));

			// (c.haag 2007-04-04 16:50) - PLID 25498 - Do not delete if the allergy is in use by an EMR detail
			if (ReturnsRecords("SELECT 1 FROM EMRDetailTableDataT "
				"LEFT JOIN EmrDataT ON EmrDataT.ID = EMRDetailTableDataT.EMRDataID_Y "
				"WHERE "
				// The table data belongs to a detail that is not deleted
				"EmrDetailID IN (SELECT ID FROM EMRDetailsT WHERE Deleted = 0 AND EMRID IN (SELECT ID FROM EMRMasterT WHERE Deleted = 0)) "
				// The table data is not populated (regardless of column)
				// (z.manning 2008-12-10 16:00) - PLID 32389 - We now check for non-blank data in all column types
				// (a.walling 2009-04-03 10:44) - PLID 33831 - blank data is no longer saved to the database
				/*
				"	AND Len(EMRDetailTableDataT.Data) > 0 \r\n"
				"	AND ( \r\n"
				"		(ListType = 4 AND EMRDetailTableDataT.Data <> '0' AND EMRDetailTableDataT.Data <> '-1') \r\n" // dropdown
				"		OR (ListType = 5 AND EMRDetailTableDataT.Data <> '0' AND EMRDetailTableDataT.Data <> '-1') \r\n" // checkbox
				"	) \r\n"					
				*/
				// The table row belongs to this allergy
				"AND EMRDataID_X IN (SELECT EMRDataID FROM AllergyT WHERE ID = %d) ", nDelID))
			{
				MessageBox("You cannot delete this allergy because it is currently in use by patient medical records.");
				return;
			}

			// (z.manning 2009-03-17 14:20) - PLID 33242 - Don't allow them to delete this allergy
			// if it has somehow been involved in table spawning.
			if(IsDataIDUsedBySpawnedObject(FormatString("SELECT EmrDataID FROM AllergyT WHERE ID = %li",nDelID))) {
				MessageBox("This allergy is referenced by spawned objects on patient EMNs. It cannot be deleted.");
				return;
			}

			if (IDYES == MessageBox("Are you sure you want to delete this allergy?", "NexTech Practice", MB_YESNO|MB_ICONQUESTION)) {
	

				//check to see that there are no patients with that allergy

				_RecordsetPtr rs;
				rs = CreateRecordset("SELECT Count(ID) as nCount FROM PatientAllergyT WHERE AllergyID = %li", nDelID);
				long nCount  = AdoFldLong(rs, "nCount");
				
				if (nCount > 0 ) {
					//we can't delete
					CString str;
					str.Format("There are %li patients with this as an Allergy, you cannot delete this allergy",  nCount);
					MessageBox(str);
				}
				else {
					//we can delete it!
					// (c.haag 2007-04-04 16:35) - PLID 25498 - Delete the allergy
					ExecuteDeletion(nDelID);
					m_pAllergyList->RemoveRow(nCurSel);
				}
			}
			else {
				//they don't want to delete, so do nothing
			}
		}NxCatchAll("Error in OnDelete");
	}
	else {
		//they haven't selected anything
		MessageBox("Please select an Allergy to Delete");
	}

	
}

void CEditAllergyListDlg::OnEdit() 
{
	//check to see that something is selected

	long nCurSel = m_pAllergyList->GetCurSel();

	if (nCurSel != -1) {

		// (b.savon 2012-09-26 17:02) - PLID 52874 - replace with enum
		//make the datalist for that sel editable
		m_pAllergyList->StartEditing(nCurSel,alcAllergyName);
	}
	else {
		MessageBox("Please select an Allergy to edit");
	}
	
}

void CEditAllergyListDlg::OnEditingFinishedEditList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, long bCommit) 
{
	if (nRow != -1) {
		if (bCommit) {
			try {
				// (b.savon 2012-09-26 17:02) - PLID 52874 - replace with enums
				switch (nCol) {
				case 1: {
					long nID = VarLong(m_pAllergyList->GetValue(nRow, alcAllergyID));
					// (c.haag 2007-04-04 16:36) - PLID 24598 - Update the name
					ExecuteNameChange(nID, VarString(varNewValue));
					break;
				}
				case 3: {
					// (c.haag 2007-04-09 14:58) - PLID 25504 - Inactivate the allergy.
					// We've already done validation in OnEditingFinishing
					long nID = VarLong(m_pAllergyList->GetValue(nRow, alcAllergyID));
					ExecuteChangeActiveFlag(nID, (VarBool(varNewValue)) ? FALSE : TRUE);
					break;
				}
				default:
					break;
				}
			}
			NxCatchAll ("Error in OnEditingFinishedEditList");
		}
	}
}

void CEditAllergyListDlg::OnOK() 
{
	//I think all we have to do is close the dialog
	CDialog::OnOK();
}

BOOL CEditAllergyListDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-25 12:28) - PLID 29790 - NxIconified buttons
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnEdit.AutoSet(NXB_MODIFY);
		m_btnDelete.AutoSet(NXB_DELETE);
		m_btnOK.AutoSet(NXB_CLOSE);
		// (b.savon 2012-09-26 17:03) - PLID 52874 - Add nxcolor member and set as patient status
		m_nxcBack.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
		
		//Initialize the datalist
		m_pAllergyList = BindNxDataListCtrl(this, IDC_EDIT_LIST, GetRemoteData(), true);
	}
	NxCatchAll("Error in CEditAllergyListDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditAllergyListDlg::OnEditingFinishingEditList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{

	switch (nCol) {
		case 1:
			{
				if (*pbCommit) {
					// (b.savon 2012-09-26 17:03) - PLID 52874 - Replace with enums
					// (b.savon 2012-08-09 10:11) - PLID 51703 - Don't allow name changes to FDB allergies
					if( VarLong(m_pAllergyList->GetValue(nRow, alcConceptID), -1) != -1 &&
						VarLong(m_pAllergyList->GetValue(nRow, alcConceptIDType), -1) != -1 ){
						MsgBox("You may not edit an imported allergy.");
						*pbContinue = TRUE;
						*pbCommit = FALSE;
						return;
					}

					if (VarString(varOldValue, "").Compare(VarString(pvarNewValue, "")) != 0) {
						// (j.gruber 2007-08-07 15:47) - PLID 26975 - don't let them rename an allergy if there are things associated with it
						_RecordsetPtr rsCount = CreateParamRecordset("SELECT Count(*) as Count FROM EMRDetailTableDataT "
						"LEFT JOIN EmrDataT ON EmrDataT.ID = EMRDetailTableDataT.EMRDataID_Y "
						"WHERE "
						// The table data belongs to a detail that is not deleted
						"EmrDetailID IN (SELECT ID FROM EMRDetailsT WHERE Deleted = 0 AND EMRID IN (SELECT ID FROM EMRMasterT WHERE Deleted = 0)) "
						// The table data is not populated (regardless of column)
						// (z.manning 2008-12-10 16:00) - PLID 32389 - We now check for non-blank data in all column types
						// (a.walling 2009-04-03 10:44) - PLID 33831 - blank data is no longer saved to the database
						/*
						"	AND Len(EMRDetailTableDataT.Data) > 0 \r\n"
						"	AND ( \r\n"
						"		(ListType = 4 AND EMRDetailTableDataT.Data <> '0' AND EMRDetailTableDataT.Data <> '-1') \r\n" // dropdown
						"		OR (ListType = 5 AND EMRDetailTableDataT.Data <> '0' AND EMRDetailTableDataT.Data <> '-1') \r\n" // checkbox
						"	) \r\n"					
						*/
						// (b.savon 2012-09-26 17:04) - PLID 52874 - Replace with enum
						// The table row belongs to this allergy
						"AND EMRDataID_X IN (SELECT EMRDataID FROM AllergyT WHERE ID = {INT}) ", VarLong(m_pAllergyList->GetValue(nRow, alcAllergyID)));
						if (!rsCount->eof) {
							long nCount = AdoFldLong(rsCount, "Count", -1);

							if (nCount > 0) {
								CString strMsg;
								strMsg.Format("This allergy is in use by %li patient medical records.  You may not rename it", nCount);
								MsgBox(strMsg);
								*pbContinue = TRUE;
								*pbCommit = FALSE;
								return;
										
							}
						}
						// (b.savon 2012-09-26 17:04) - PLID 52874 - Replace with enum
						rsCount = CreateParamRecordset("SELECT Count(ID) as Count FROM PatientAllergyT WHERE AllergyID = {INT} ", VarLong(m_pAllergyList->GetValue(nRow, alcAllergyID)));
						if (!rsCount->eof) {
							long nCount = AdoFldLong(rsCount, "Count", -1);

							if (nCount > 0) {
								CString strMsg;
								strMsg.Format("This allergy is in use by %li patient(s).  You may not rename it", nCount);
								MsgBox(strMsg);
								*pbContinue = TRUE;
								*pbCommit = FALSE;
								return;
							}
						}					
					}
					else {

						//if they are exactly the same, there is no reason to do anything
						*pbContinue = TRUE;
						*pbCommit = FALSE;
						return;
					}

					CString strTemp = strUserEntered;
					strTemp.TrimLeft();
					strTemp.TrimRight();
					if (strTemp.IsEmpty()) {
						MessageBox("Please enter valid data");
						*pbCommit = FALSE;
						*pbContinue = FALSE;
					}
					// (b.savon 2012-09-26 17:05) - PLID 52874 - Replace with enum
					// (c.haag 2007-04-04 16:59) - PLID 25498 - Don't use the Name field in AllergyT anymore
					if(!IsRecordsetEmpty("SELECT Data FROM AllergyT "
							"INNER JOIN EmrDataT ON EmrDataT.ID = AllergyT.EmrDataID "
							"WHERE AllergyT.ID <> %li AND Data = '%s'",VarLong(m_pAllergyList->GetValue(nRow, alcAllergyID)),_Q(strTemp))) {
						MessageBox("The data you entered already exists in the list. Please enter a unique name.");
						*pbCommit = FALSE;
						*pbContinue = FALSE;
					}
				}
			}
		break;
		case 3:
			{
				// (c.haag 2007-04-09 15:05) - PLID 25504 - If they are inactivating the record, then we have to make
				// sure that at least one allergy is active. We are not allowed to have no active allergies in data.
				if (VarBool(*pvarNewValue)) {
					if (!HasMultipleActiveAllergies()) {
						*pbCommit = FALSE;
						*pbContinue = FALSE;
						return;
					}
				}

				// (c.haag 2007-04-09 15:08) - PLID 25504 - Confirm this with the user
				if (VarBool(*pvarNewValue)) {
					if (IDYES != MsgBox(MB_YESNO | MB_ICONQUESTION, "Are you sure you wish to inactivate this allergy?")) {
						*pbCommit = FALSE;
						*pbContinue = FALSE;
					}
				} else {
					if (IDYES != MsgBox(MB_YESNO | MB_ICONQUESTION, "Are you sure you wish to activate this allergy?")) {
						*pbCommit = FALSE;
						*pbContinue = FALSE;
					}
				}
				break;
			}
		default:
		break;
	}




	
}

void CEditAllergyListDlg::OnLButtonUpEditList(long nRow, short nCol, long x, long y, long nFlags) 
{
	switch(nCol) {
	case 2:
		if(nRow != -1) {
			// (a.walling 2006-11-27 15:11) - PLID 20179 - Don't open the interaction editor if user lacks permissions
			if(!CheckCurrentUserPermissions(bioPatientAllergies,sptDynamic1))
				return;

			// (b.savon 2012-09-26 17:06) - PLID 52874 - Replace with enums
			// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
			CMultiSelectDlg dlg(this, "DrugList");
			CDWordArray dwaDrugs;
			long nAllergyID = VarLong(m_pAllergyList->GetValue(nRow,alcAllergyID));
			_RecordsetPtr rsDrugs = CreateRecordset("SELECT DrugID FROM DrugAllergyT WHERE AllergyID = %li", nAllergyID);
			while(!rsDrugs->eof) {
				dwaDrugs.Add(AdoFldLong(rsDrugs, "DrugID"));
				rsDrugs->MoveNext();
			}
			rsDrugs->Close();
			dlg.PreSelect(dwaDrugs);
			// (c.haag 2007-02-02 17:38) - PLID 24561 - We now store medication names in EmrDataT.Data rather than DrugList.Name
			if(IDOK == dlg.Open("DrugList LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID", "", "DrugList.ID", "Data", "Select any drugs which would be contraindicated for this allergy.")) {
				dlg.FillArrayWithIDs(dwaDrugs);
				ExecuteSql("DELETE FROM DrugAllergyT WHERE AllergyID = %li", nAllergyID);
				for(int i = 0; i < dwaDrugs.GetSize(); i++) {
					ExecuteSql("INSERT INTO DrugAllergyT (DrugID, AllergyID) VALUES (%li, %d)", dwaDrugs[i], nAllergyID);
				}
				CString strDescription = dlg.GetMultiSelectString();
				if(strDescription.IsEmpty()) strDescription = "<None>";
				m_pAllergyList->PutValue(nRow,alcConfiguredInteraction, _bstr_t(strDescription));
			}
		}
		break;
	}

}

void CEditAllergyListDlg::OnRequeryFinishedEditList(short nFlags) 
{
	try{
		for(int i = 0; i < m_pAllergyList->GetRowCount(); i++) {
			// (b.savon 2012-09-26 17:08) - PLID 52874 - Replace with enum and added a recordset to the the query to determine
			// if the row should be colored if it's an imported allergy.  Also added an event handler
			long nAllergyID = VarLong(m_pAllergyList->GetValue(i,alcAllergyID));
			// (c.haag 2007-02-02 17:38) - PLID 24561 - We now store medication names in EmrDataT.Data rather than DrugList.Name
			//TES 5/9/2013 - PLID 56614 - Added FDBOutOfDate
			_RecordsetPtr prs = CreateParamRecordset(
				"SELECT	CASE WHEN ConceptID IS NOT NULL AND ConceptIDType IS NOT NULL THEN 1 ELSE 0 END AS IsFDBImportedAllergy, \r\n"
				"FDBOutOfDate "
				"FROM	AllergyT \r\n"
				"WHERE   ID = {INT} \r\n"
				"\r\n"
				"SELECT EMRDataT.Data AS Name FROM DrugList "
				"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
				"INNER JOIN DrugAllergyT ON DrugList.ID = DrugAllergyT.DrugID WHERE DrugAllergyT.AllergyID = {INT}", nAllergyID, nAllergyID);
			if( !prs->eof ){
				// Lets color all the non-imported FDB allergies gray
				if( AdoFldLong(prs, "IsFDBImportedAllergy", 0) == 1 ){
					//TES 5/9/2013 - PLID 56614 - Highlight the outdated codes
					if(AdoFldBool(prs, "FDBOutOfDate", 0)) {
						((NXDATALISTLib::IRowSettingsPtr)m_pAllergyList->GetRow(i))->PutBackColor(ERX_IMPORTED_OUTOFDATE_COLOR);
					}
					else {
						((NXDATALISTLib::IRowSettingsPtr)m_pAllergyList->GetRow(i))->PutBackColor(ERX_IMPORTED_COLOR);
					}
				}
				prs = prs->NextRecordset(NULL);
			}

			if(prs->eof) {
				m_pAllergyList->PutValue(i,alcConfiguredInteraction,_bstr_t("<None>"));
			}
			else {
				CString strDrugs;
				while(!prs->eof) {
					strDrugs += AdoFldString(prs, "Name") + ",";
					prs->MoveNext();
				}
				strDrugs.TrimRight(",");
				m_pAllergyList->PutValue(i,alcConfiguredInteraction,_bstr_t(strDrugs));
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CEditAllergyListDlg::ExecuteNameChange(long nAllergyID, const CString& strNewName)
{
	//
	// (c.haag 2007-04-04 16:07) - PLID 25498 - This function is called to change the name
	// of an allergy. We must update the EMR data only.
	//
	CWaitCursor wc;
	BEGIN_TRANS("ExecuteNameChange")

		long nEmrInfoID = GetActiveAllergiesInfoID();
		if (IsEMRInfoItemInUse(nEmrInfoID)) {
			nEmrInfoID = BranchAllergiesInfoItem();
		} else {
			TouchEMRInfoItem(nEmrInfoID); // (a.walling 2013-03-27 09:47) - PLID 55898 - Update EmrInfoT.ModifiedDate (and vicariously, the .Revision rowversion)
		}
		long nDataID = VarLong(GetTableField("AllergyT", "EMRDataID", "ID", nAllergyID));

		// Now do EMR auditing.
		CString strOldValue, strNewValue;
		_RecordsetPtr prs = CreateRecordset("SELECT EmrDataT.Data FROM EMRDataT WHERE EMRDataT.ID = %d", nDataID);
		strOldValue.Format("Item: 'Allergies' - Table Row: '%s'", AdoFldString(prs, "Data"));
		strNewValue.Format("Table Row: '%s'", strNewName);
		prs->Close();
		AuditEvent(-1, "", BeginNewAuditEvent(), aeiEMRItemTableItemName, nEmrInfoID, strOldValue, strNewValue, aepHigh, aetChanged);

		// Now update the system Allergies info item
		ExecuteSql("UPDATE EMRDataT SET Data = '%s' WHERE ID IN (SELECT EMRDataID FROM AllergyT WHERE ID = %d)", _Q(strNewName), nAllergyID);

	END_TRANS("ExecuteNameChange")

	// (c.haag 2007-04-04 16:14) - PLID 25498 - This function will pop up an exception message if,
	// for some unexpected reason, a record exists in AllergyT with a bad EmrDataID, or a record
	// exists in EmrDataT for the active Allergies item that does not correspond to AllergyT
	WarnEmrDataDiscrepanciesWithAllergyT();
}

void CEditAllergyListDlg::ExecuteAddition(long nAllergyID)
{
	//
	// (c.haag 2007-04-04 16:16) - PLID 25498 - This function is called to add an allergy to the
	// AllergyT table. We must update the EMR data as well.
	//
	CWaitCursor wc;
	BEGIN_TRANS("ExecuteAddition")

		long nEmrInfoID = GetActiveAllergiesInfoID();
		if (IsEMRInfoItemInUse(nEmrInfoID)) {
			nEmrInfoID = BranchAllergiesInfoItem();
		} else {
			TouchEMRInfoItem(nEmrInfoID); // (a.walling 2013-03-27 09:47) - PLID 55898 - Update EmrInfoT.ModifiedDate (and vicariously, the .Revision rowversion)
		}

		// Now do EMR auditing.
		CString strOldValue, strNewValue;
		strOldValue.Format("Item: 'Allergies'");
		strNewValue.Format("New Table Row: 'Enter New Allergy'");
		AuditEvent(-1, "", BeginNewAuditEvent(), aeiEMRItemTableItemCreated, nEmrInfoID, strOldValue, strNewValue, aepMedium, aetCreated);

		// (j.jones 2007-08-14 16:02) - PLID 27053 - added EMRDataGroupsT
		long nEMRDataGroupID = NewNumber("EMRDataGroupsT", "ID");
		ExecuteSql("INSERT INTO EMRDataGroupsT (ID) VALUES (%li)", nEMRDataGroupID);

		long nDataID = NewNumber("EMRDataT", "ID");

		// Now add the new allergy to the EMRDataT table
		ExecuteSql("INSERT INTO EMRDataT (ID, EMRInfoID, Data, SortOrder, ListType, EmrDataGroupID) "
			"SELECT %d, %d, 'Enter New Allergy', "
			"(SELECT COALESCE(Max(SortOrder),0) + 1 FROM EMRDataT WHERE EMRInfoID = %d), "
			"2, %li",
			nDataID, nEmrInfoID, nEmrInfoID, nEMRDataGroupID);

		// Now add the allergy to AllergyT
		ExecuteSql("INSERT INTO AllergyT (ID, EMRDataID) "
			"VALUES (%li, %d)", nAllergyID, nDataID);

	END_TRANS("ExecuteAddition")

	// (c.haag 2007-04-04 16:14) - PLID 25498 - This function will pop up an exception message if,
	// for some unexpected reason, a record exists in AllergyT with a bad EmrDataID, or a record
	// exists in EmrDataT for the active Allergies item that does not correspond to AllergyT
	WarnEmrDataDiscrepanciesWithAllergyT();
}

void CEditAllergyListDlg::ExecuteDeletion(long nAllergyID)
{
	//
	// (c.haag 2007-04-04 16:21) - PLID 25498 - This function is called to remove an allergy from
	// the allergy table. We must update the EMR data as well.
	//
	CWaitCursor wc;
	BEGIN_TRANS("ExecuteDeletion")

		long nEmrInfoID = GetActiveAllergiesInfoID();
		if (IsEMRInfoItemInUse(nEmrInfoID)) {
			nEmrInfoID = BranchAllergiesInfoItem();
		} else {
			TouchEMRInfoItem(nEmrInfoID); // (a.walling 2013-03-27 09:47) - PLID 55898 - Update EmrInfoT.ModifiedDate (and vicariously, the .Revision rowversion)
		}
		long nDataID = VarLong(GetTableField("AllergyT", "EMRDataID", "ID", nAllergyID));

		// Now do EMR auditing.
		CString strOldValue, strNewValue;
		_RecordsetPtr prs = CreateParamRecordset("SELECT EmrDataT.Data FROM EMRDataT WHERE EMRDataT.ID = {INT}", nDataID);
		strOldValue.Format("Item: 'Allergies'");
		strNewValue.Format("Deleted Table Row: '%s'", AdoFldString(prs, "Data"));
		prs->Close();
		AuditEvent(-1, "", BeginNewAuditEvent(), aeiEMRItemTableItemDeleted, nEmrInfoID, strOldValue, strNewValue, aepHigh, aetDeleted);

		// (z.manning 2013-03-11 12:58) - PLID 55554 - Moved the logic for deleting EMR data records to its own function
		CSqlFragment sqlDelete;
		GetDeleteEmrDataSql(CSqlFragment("{INT}", nDataID), sqlDelete);
		ExecuteParamSql(sqlDelete);

	END_TRANS("ExecuteDeletion")

	// (c.haag 2007-04-04 16:14) - PLID 25498 - This function will pop up an exception message if,
	// for some unexpected reason, a record exists in AllergyT with a bad EmrDataID, or a record
	// exists in EmrDataT for the active Allergies item that does not correspond to AllergyT
	WarnEmrDataDiscrepanciesWithAllergyT();
}

void CEditAllergyListDlg::ExecuteChangeActiveFlag(long nAllergyID, BOOL bActivate)
{
	//
	// (c.haag 2007-04-09 14:53) - PLID 25504 - This function is called to inactivate an allergy
	// in the AllergyT table. We must update the EMR data.
	//
	CWaitCursor wc;
	BEGIN_TRANS("ExecuteChangeActiveFlag")

		long nEmrInfoID = GetActiveAllergiesInfoID();
		if (IsEMRInfoItemInUse(nEmrInfoID)) {
			nEmrInfoID = BranchAllergiesInfoItem();
		} else {
			TouchEMRInfoItem(nEmrInfoID); // (a.walling 2013-03-27 09:47) - PLID 55898 - Update EmrInfoT.ModifiedDate (and vicariously, the .Revision rowversion)
		}
		long nDataID = VarLong(GetTableField("AllergyT", "EMRDataID", "ID", nAllergyID));

		// Now do EMR auditing.
		CString strOldValue, strNewValue;
		_RecordsetPtr prs = CreateRecordset("SELECT EmrDataT.Data FROM EMRDataT WHERE EMRDataT.ID = %d", nDataID);
		if (bActivate) {
			strOldValue.Format("Item: 'Allergies' - Table Row: %s (Inactive)", AdoFldString(prs, "Data"));
			strNewValue.Format("Table Row: '%s' (Active)", AdoFldString(prs, "Data"));
		} else {
			strOldValue.Format("Item: 'Allergies' - Table Row: %s (Active)", AdoFldString(prs, "Data"));
			strNewValue.Format("Table Row: '%s' (Inactive)", AdoFldString(prs, "Data"));
		}
		prs->Close();
		AuditEvent(-1, "", BeginNewAuditEvent(), aeiEMRItemTableItemInactive, nEmrInfoID, strOldValue, strNewValue, aepMedium, aetChanged);

		ExecuteSql("UPDATE EMRDataT SET Inactive = %d WHERE ID IN (SELECT EMRDataID FROM AllergyT WHERE ID = %d)", 
			(bActivate) ? 0 : 1, nAllergyID);

	END_TRANS("ExecuteChangeActiveFlag")

	// (c.haag 2007-04-09 14:54) - PLID 25504 - This function will pop up an exception message if,
	// for some unexpected reason, a record exists in AllergyT with a bad EmrDataID, or a record
	// exists in EmrDataT for the active Allergies item that does not correspond to AllergyT
	WarnEmrDataDiscrepanciesWithAllergyT();
}

BOOL CEditAllergyListDlg::HasMultipleActiveAllergies()
{
	// (c.haag 2007-04-04 17:23) - PLID 25498 - This function determines if there is exactly one
	// active allergy in Practice, and returns TRUE if there is not. If necessary, the allergy
	// list will also be refreshed.
	const int nActiveAllergies = GetRecordCount("SELECT AllergyT.ID FROM AllergyT INNER JOIN EmrDataT ON EmrDataT.ID = AllergyT.EmrDataID WHERE Inactive = 0");
	if (1 == nActiveAllergies) {
		MessageBox("You must have at least one active allergy in your database.", "NexTech Practice", MB_OK|MB_ICONINFORMATION);
		// It may be possible to get the previous warning if someone else was deleting allergies from another
		// computer. If the active allergy count and list count are inconsistent, refresh the list.
		const int nListRows = m_pAllergyList->GetRowCount();
		if (nActiveAllergies != nListRows) {
			MessageBox("The allergy list has been modified by other users. The list will now be refreshed.", "NexTech Practice", MB_OK|MB_ICONINFORMATION);
			m_pAllergyList->Requery();
		}
		return FALSE;
	} else {
		return TRUE;
	}
}

void CEditAllergyListDlg::OnUpdateAllAllergies()
{
	try
	{
		//TES 6/5/2013 - PLID 56631 - Make sure they have the FDB license
		// (z.manning 2015-11-12 09:11) - PLID 66714 - We decided a while back that having an FDB license was not 
		// required to update meds, so now we are doing the same for allergies.
		//if(!g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent)) {
		//	return;
		//}

		//TES 5/10/2013 - PLID 56631 - Call the API to update all linked allergies from FDB information
		CArray<NexTech_Accessor::_FDBUpdateAllergyInputPtr, NexTech_Accessor::_FDBUpdateAllergyInputPtr> aryAllergies;

		//TES 5/10/2013 - PLID 56631 - Create our SAFEARRAY to be passed to the UpdateAllergies function in the API
		Nx::SafeArray<IUnknown *> saryAllergies = Nx::SafeArray<IUnknown *>::From(aryAllergies);

		
		CWaitCursor cwait;

		//TES 5/10/2013 - PLID 56631 - Call the API to update the allergies and then convert the handed back SAFEARRAY to a CArray so we can do something useful with it.
		// Note that we pass in an empty array, this will tell the function to update all linked records
		NexTech_Accessor::_FDBUpdateAllergyResultsArrayPtr updateResults = GetAPI()->UpdateAllergies(GetAPISubkey(), GetAPILoginToken(), saryAllergies);

		//TES 5/10/2013 - PLID 56631 - Refresh the list to re-color the linked allergies
		m_pAllergyList->Requery();

		Nx::SafeArray<IUnknown *> saryAllergyResults(updateResults->FDBUpdateAllergyResults);

		int nSuccessCount = 0, nFailedCount = 0;
		foreach(NexTech_Accessor::_FDBUpdateAllergyOutputPtr pUpdateResult, saryAllergyResults) {
			if(pUpdateResult->Success) {
				nSuccessCount++;
			}
			else {
				nFailedCount++;
			}
		}

		//TES 5/10/2013 - PLID 56631 - Report success
		if(nFailedCount == 0) {
			if(nSuccessCount == 0) {
				MsgBox("All allergies were already up to date");
			}
			else {
				MsgBox("Successfully updated %i allergies", nSuccessCount);
			}
		}
		else {
			//TES 5/10/2013 - PLID 56631 - There is currently no case where any codes should fail to update, so raise an exception
			ThrowNxException("Failed to update %i allergies from FirstDataBank", nFailedCount);
		}


	}NxCatchAll(__FUNCTION__);
}

void CEditAllergyListDlg::OnRButtonDownEditList(long nRow, short nCol, long x, long y, long nFlags)
{
	try {
		m_pAllergyList->CurSel = nRow;
		if(nRow == -1) return;
		
		//TES 5/10/2013 - PLID 56631 - If the row we're on is linked to FDB, and is out of date, give a menu option to update the record from FDB
		if(m_pAllergyList->GetValue(nRow, alcConceptID).vt != VT_NULL)
		{
			if (VarBool(m_pAllergyList->GetValue(nRow, alcFDBOutOfDate), FALSE))
			{
				CNxMenu pMenu;
				pMenu.CreatePopupMenu();
				pMenu.AppendMenu(MF_ENABLED, 1, "Update Allergy Information From FirstDataBank");

				CPoint pt;
				GetCursorPos(&pt);
				int nCmd = pMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, pt.x, pt.y, this);
				if (nCmd == 1) {
					//TES 5/10/2013 - PLID 56631 - Call the API to update this allergy
					CArray<NexTech_Accessor::_FDBUpdateAllergyInputPtr, NexTech_Accessor::_FDBUpdateAllergyInputPtr> aryAllergies;
					NexTech_Accessor::_FDBUpdateAllergyInputPtr allergy(__uuidof(NexTech_Accessor::FDBUpdateAllergyInput));
					allergy->AllergyID = VarLong(m_pAllergyList->GetValue(nRow, alcAllergyID));
					aryAllergies.Add(allergy);

					//TES 5/10/2013 - PLID 56631 - Create our SAFEARRAY to be passed to the UpdateAllergies function in the API
					Nx::SafeArray<IUnknown *> saryAllergies = Nx::SafeArray<IUnknown *>::From(aryAllergies);

					CWaitCursor cwait;

					//TES 5/10/2013 - PLID 56631 - Call the API to update the allergies and then convert the handed back SAFEARRAY to a CArray so we can do something useful with it.
					NexTech_Accessor::_FDBUpdateAllergyResultsArrayPtr updateResults = GetAPI()->UpdateAllergies(GetAPISubkey(), GetAPILoginToken(), saryAllergies);

					Nx::SafeArray<IUnknown *> saryAllergyResults(updateResults->FDBUpdateAllergyResults);

					int nSuccessCount = 0, nFailedCount = 0;
					foreach(NexTech_Accessor::_FDBUpdateAllergyOutputPtr pUpdateResult, saryAllergyResults) {
						if (pUpdateResult->Success) {
							nSuccessCount++;
						}
						else {
							nFailedCount++;
						}
					}

					//TES 5/10/2013 - PLID 56631 - If we succeeded, just update the row color
					if (nFailedCount == 0) {
						((NXDATALISTLib::IRowSettingsPtr)m_pAllergyList->GetRow(nRow))->PutBackColor(ERX_IMPORTED_COLOR);
					}
					else {
						//TES 5/10/2013 - PLID 56631 - There is currently no case where any allergies should fail to update, so raise an exception
						ThrowNxException("Failed to update single allergy from FirstDataBank");
					}
				}
			}
		}
		
	}NxCatchAll(__FUNCTION__);
}