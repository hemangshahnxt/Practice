// EMRSelectMedicationsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EMRSelectMedicationsDlg.h"
#include "PrescriptionUtilsAPI.h"	// (j.jones 2013-04-03 16:47) - PLID 56080 - needs the API header because this does use the accessor
#include "FirstDataBankUtils.h"

// (j.jones 2013-03-04 17:32) - PLID 55376 - created

// CEMRSelectMedicationsDlg dialog

using namespace NXDATALIST2Lib;

#define ID_REMOVE_MEDICATION 677592015

IMPLEMENT_DYNAMIC(CEMRSelectMedicationsDlg, CNxDialog)

CEMRSelectMedicationsDlg::CEMRSelectMedicationsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMRSelectMedicationsDlg::IDD, pParent)
{

}

CEMRSelectMedicationsDlg::~CEMRSelectMedicationsDlg()
{
}

void CEMRSelectMedicationsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_CHECK_INCLUDE_FREE_TEXT_MEDS_ITEMS, m_checkIncludeFreeTextMeds);
	DDX_Control(pDX, IDC_ABOUT_EMRSELECT_MEDICATION_COLORS, m_icoAboutEMRSelectMedsColors); 
}

BEGIN_MESSAGE_MAP(CEMRSelectMedicationsDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, OnOk)
	ON_COMMAND(ID_REMOVE_MEDICATION, OnRemoveMedication)
	ON_BN_CLICKED(IDC_CHECK_INCLUDE_FREE_TEXT_MEDS_ITEMS, OnCheckIncludeFreeTextMedsItems)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CEMRSelectMedicationsDlg, CNxDialog)
	ON_EVENT(CEMRSelectMedicationsDlg, IDC_NXDL_MED_SEARCH_ADD, 16, CEMRSelectMedicationsDlg::SelChosenNxdlMedSearchAdd, VTS_DISPATCH)
	ON_EVENT(CEMRSelectMedicationsDlg, IDC_SELECT_MEDICATION_LIST, 6, CEMRSelectMedicationsDlg::RButtonDownSelectMedicationList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

// CEMRSelectMedicationsDlg message handlers

BOOL CEMRSelectMedicationsDlg::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// (j.jones 2016-01-25 15:29) - PLID 67999 - cached IncludeFreeTextFDBSearchResults
		g_propManager.CachePropertiesInBulk("EmrActionDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'IncludeFreeTextFDBSearchResults'" \
			")",
			_Q(GetCurrentUserName()));

		// (j.jones 2016-01-25 15:45) - PLID 67999 - if they do not have FDB, don't show the free text medication search option
		if (!g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent)) {
			//no FDB? no checkbox, and always include free-text meds, because that's all you've got
			m_checkIncludeFreeTextMeds.SetCheck(TRUE);
			m_checkIncludeFreeTextMeds.ShowWindow(SW_HIDE);
			// (b.eyers 2016-02-090 - PLID 67979 - added an icon for information about med search color, hide if no fdb
			m_icoAboutEMRSelectMedsColors.ShowWindow(SW_HIDE);
		}
		else {
			// (j.jones 2016-01-25 15:31) - PLID 67999 - added option to include free text meds in the medication search
			long nIncludeFreeTextFDBSearchResults = GetRemotePropertyInt("IncludeFreeTextFDBSearchResults", 0, 0, GetCurrentUserName(), true);
			//use the prescription option, since spawned meds are always prescriptions
			m_checkIncludeFreeTextMeds.SetCheck(nIncludeFreeTextFDBSearchResults & INCLUDEFREETEXT_PRESCRIPTIONS);

			// (b.eyers 2016-02-090 - PLID 67979 - added an icon for information about med search color
			CString strMedsToolTipText = "All medications with a salmon background are imported and are checked for interactions. \r\n"
				"All medications with a red background have changed since being imported, and must be updated before being used on new prescriptions. \r\n"
				"Using free text medications (white background) rather than those from the imported lists (salmon background) will result in a lack of interaction warnings.";
			m_icoAboutEMRSelectMedsColors.LoadToolTipIcon(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDB_QUESTION_MARK), strMedsToolTipText, false, false, false);
			m_icoAboutEMRSelectMedsColors.EnableClickOverride();
		}

		// (b.savon 2015-12-30 13:29) - PLID 67759
		// (j.jones 2016-01-25 15:43) - PLID 67999 - moved the bind to ResetMedicationSearchProvider
		ResetMedicationSearchProvider();

		m_List = BindNxDataList2Ctrl(IDC_SELECT_MEDICATION_LIST, false);

	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEMRSelectMedicationsDlg::OnOk()
{
	try {

		m_arySelectedMeds.RemoveAll();

		CWaitCursor pWait;

		// (j.jones 2013-04-04 12:55) - PLID 56080 - First find how many meds.
		// are selected. If the count is above 2,000, stop and yell at the user.
		{
			long nCountToImport = m_List->GetRowCount();

			//Sanity check. If they check off more than 2,000 meds., tell them they are insane.
			if(nCountToImport > 2000) {
				CString strWarn;
				strWarn.Format("You have selected %li medications to add to this EMR item. You may not select more than 2000 medications at a time.\n\n"
					"Please reduce the number of selected medications to be less than 2000.", nCountToImport);
				MessageBox(strWarn, "Practice", MB_ICONEXCLAMATION|MB_OK);
				return;
			}
		}

		//now get all the selected meds
		IRowSettingsPtr pRow = m_List->GetFirstRow();
		while(pRow) {

			IdName idNameElement;
			idNameElement.nID = VarLong(pRow->GetValue(mrcMedicationID), -1);
			idNameElement.strName = VarString(pRow->GetValue(mrcMedicationName));
			m_arySelectedMeds.Add(idNameElement);

			pRow = pRow->GetNextRow();
		}

		if(m_arySelectedMeds.GetSize() == 0) {
			MessageBox("No medications are selected.", "Practice", MB_ICONINFORMATION|MB_YESNO);
			return;
		}

		CNxDialog::OnOK();

	} NxCatchAll(__FUNCTION__);
}

// (b.savon 2015-12-30 13:29) - PLID 67759
void CEMRSelectMedicationsDlg::SelChosenNxdlMedSearchAdd(LPDISPATCH lpRow)
{
	try {
		AddMedicationFromSearch(lpRow);

	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2015-12-30 13:29) - PLID 67759
void CEMRSelectMedicationsDlg::AddMedicationFromSearch(LPDISPATCH lpRow)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

	if (pRow != NULL) {

		if (VarLong(pRow->GetValue(mrcMedicationID)) == NO_RESULTS_ROW) {
			return;
		}

		if (m_List->FindByColumn(mrcMedicationName, pRow->GetValue(mrcMedicationName), NULL, VARIANT_FALSE) != NULL) {
			return;
		}

		//Pull Values from selection
		long nMedicationID = VarLong(pRow->GetValue(mrcMedicationID), -1);
		long nFDBID = VarLong(pRow->GetValue(mrcFirstDataBankID), -1);
		CString strMedname = VarString(pRow->GetValue(mrcMedicationName), "");
		BOOL bFirstDataBankOutOfDate = VarBool(pRow->GetValue(mrcFDBOutOfDate), FALSE) && nFDBID > 0;

		//If the medicationid is -1 (i.e. DrugList.ID), we need to import it from FDB
		long nNewDrugListID = nMedicationID;
		if (nMedicationID == -1) {
			ImportMedication(nFDBID, strMedname, nNewDrugListID);
		}

		// Update Med List
		AddMedicationToList(nNewDrugListID, strMedname, nFDBID, bFirstDataBankOutOfDate);
	}
}

// (b.savon 2015-12-30 13:29) - PLID 67759
void CEMRSelectMedicationsDlg::AddMedicationToList(const long &nDrugListID, const CString& strMedName, const long &nFDBID, const BOOL &bFDBOutOfDate)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRowToAdd = m_List->GetNewRow();
		pRowToAdd->PutValue(mrcMedicationID, nDrugListID);
		pRowToAdd->PutValue(mrcMedicationName, AsBstr(strMedName));
		pRowToAdd->PutValue(mrcFirstDataBankID, nFDBID);
		pRowToAdd->PutValue(mrcFDBOutOfDate, bFDBOutOfDate && nFDBID > 0 ? VARIANT_TRUE : VARIANT_FALSE);

		if (nFDBID != -1) {
			if (nFDBID > 0 && bFDBOutOfDate) {
				pRowToAdd->PutBackColor(ERX_IMPORTED_OUTOFDATE_COLOR);
			}
			else {
				pRowToAdd->PutBackColor(ERX_IMPORTED_COLOR);
			}
		}

		m_List->AddRowSorted(pRowToAdd, NULL);
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2015-12-30 13:29) - PLID 67759
void CEMRSelectMedicationsDlg::RButtonDownSelectMedicationList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow != NULL) {

			m_List->PutCurSel(pRow);

			CMenu mPopup;
			mPopup.CreatePopupMenu();

			mPopup.AppendMenu(MF_BYPOSITION, ID_REMOVE_MEDICATION, "&Remove");

			CPoint pt;
			GetCursorPos(&pt);
			mPopup.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this);
		}
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2015-12-30 13:29) - PLID 67759
void CEMRSelectMedicationsDlg::OnRemoveMedication()
{
	try
	{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_List->GetCurSel();

		if (pRow != NULL) {
			m_List->RemoveRow(pRow);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2016-01-25 15:42) - PLID 67999 - added checkbox to include free text meds in the medication search
void CEMRSelectMedicationsDlg::OnCheckIncludeFreeTextMedsItems()
{
	try {

		//make sure they know what they are doing
		if (m_checkIncludeFreeTextMeds.GetCheck()) {

			if (!ConfirmFreeTextSearchWarning(this, false)) {
				//they changed their minds
				m_checkIncludeFreeTextMeds.SetCheck(FALSE);
				return;
			}

			//there are some weird focus issues here where the messagebox takes away focus
			//and the datalist placeholder text can't figure out where focus is,
			//so set the focus back to the checkbox
			m_checkIncludeFreeTextMeds.SetFocus();
		}

		//reflect their choice in the search results
		ResetMedicationSearchProvider();

		//we do NOT update the preference default here

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2016-01-25 15:43) - PLID 67999 - resets the medication search provider based
// on the value of the 'include free text' checkbox
void CEMRSelectMedicationsDlg::ResetMedicationSearchProvider()
{
	try {

		bool bIncludeFDBMedsOnly = m_checkIncludeFreeTextMeds.GetCheck() ? false : true;

		//force this to false if they don't have FDB
		if (!g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent)) {
			bIncludeFDBMedsOnly = false;
		}

		//re-bind the control to the new provider
		m_nxdlMedicationSearch = BindMedicationSearchListCtrl(this, IDC_NXDL_MED_SEARCH_ADD, GetRemoteData(), false, bIncludeFDBMedsOnly);

	}NxCatchAll(__FUNCTION__);
}