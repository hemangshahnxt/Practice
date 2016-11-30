// LabEditDiagnosisLinkDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "LabEditDiagnosisLinkDlg.h"
#include "afxdialogex.h"


// CLabEditDiagnosisLinkDlg dialog

// (j.gruber 2016-02-10 12:25) - PLID 68155 - Create a setup dialog to link Diagnosis Codes with Microscopic descriptions

#define UNSELECTED_DIAGNOSIS_FILTER -1
#define EXISTING_DESCRIPTIONS RGB(53, 195, 116)
#define NO_DESCRIPTIONS RGB(0,0,0)

enum DiagnosisListColumns
{
	dlcID = 0,
	dlcDiagCode,
	dlcDescription,
	dlcHasDescription,  // (j.gruber 2016-02-10 16:10) - PLID 68226 - Color the rows in the diagnosis list if they already have a microscopic description (or more) associated
};

enum MicroscopicDescriptionListColumns
{
	mdlcID = 0,
	mdlcDescription,
};

IMPLEMENT_DYNAMIC(CLabEditDiagnosisLinkDlg, CNxDialog)

CLabEditDiagnosisLinkDlg::CLabEditDiagnosisLinkDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(IDD_LABS_EDIT_DIAGNOSIS_LINK_DLG, pParent)
{
	m_bAdvancedSetup = FALSE;

	m_bIsFiltered = FALSE;	

}

CLabEditDiagnosisLinkDlg::~CLabEditDiagnosisLinkDlg()
{
}

void CLabEditDiagnosisLinkDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDCANCEL, m_btnClose);
	DDX_Control(pDX, IDC_APPLY_LINK, m_btnApply);
	DDX_Control(pDX, IDC_APPEND_LINK, m_btnInsert);
	DDX_Control(pDX, IDC_SELECT_ONE_DIAGNOSIS, m_btnSelectOneDiagnosis);
	DDX_Control(pDX, IDC_SELECT_ALL_DIAGNOSIS, m_btnSelectAllDiagnosis);
	DDX_Control(pDX, IDC_UNSELECT_ONE_DIAGNOSIS, m_btnUnSelectOneDiagnosis);
	DDX_Control(pDX, IDC_UNSELECT_ALL_DIAGNOSIS, m_btnUnSelectAllDiagnosis);
	DDX_Control(pDX, IDC_SELECT_ONE_DESCRIPTION, m_btnSelectOneDescription);
	DDX_Control(pDX, IDC_SELECT_ALL_DESCRIPTION, m_btnSelectAllDescription);
	DDX_Control(pDX, IDC_UNSELECT_ONE_DESCRIPTION, m_btnUnSelectOneDescription);
	DDX_Control(pDX, IDC_UNSELECT_ALL_DESCRIPTION, m_btnUnSelectAllDescription);
}


BEGIN_MESSAGE_MAP(CLabEditDiagnosisLinkDlg, CNxDialog)
	ON_BN_CLICKED(IDC_SELECT_ONE_DIAGNOSIS, &CLabEditDiagnosisLinkDlg::OnBnClickedSelectOneDiagnosis)
	ON_BN_CLICKED(IDC_SELECT_ALL_DIAGNOSIS, &CLabEditDiagnosisLinkDlg::OnBnClickedSelectAllDiagnosis)
	ON_BN_CLICKED(IDC_UNSELECT_ONE_DIAGNOSIS, &CLabEditDiagnosisLinkDlg::OnBnClickedUnselectOneDiagnosis)
	ON_BN_CLICKED(IDC_UNSELECT_ALL_DIAGNOSIS, &CLabEditDiagnosisLinkDlg::OnBnClickedUnselectAllDiagnosis)
	ON_BN_CLICKED(IDC_SELECT_ONE_DESCRIPTION, &CLabEditDiagnosisLinkDlg::OnBnClickedSelectOneDescription)
	ON_BN_CLICKED(IDC_SELECT_ALL_DESCRIPTION, &CLabEditDiagnosisLinkDlg::OnBnClickedSelectAllDescription)
	ON_BN_CLICKED(IDC_UNSELECT_ONE_DESCRIPTION, &CLabEditDiagnosisLinkDlg::OnBnClickedUnselectOneDescription)
	ON_BN_CLICKED(IDC_UNSELECT_ALL_DESCRIPTION, &CLabEditDiagnosisLinkDlg::OnBnClickedUnselectAllDescription)
	ON_BN_CLICKED(IDC_APPLY_LINK, &CLabEditDiagnosisLinkDlg::OnBnClickedApplyLink)
	ON_BN_CLICKED(IDC_TOGGLE_SETUP, &CLabEditDiagnosisLinkDlg::OnBnClickedToggleSetup)
	ON_EN_CHANGE(IDC_DESCRIPTION_SEARCH, &CLabEditDiagnosisLinkDlg::OnEnChangeDescriptionSearch)
	ON_BN_CLICKED(IDC_APPEND_LINK, &CLabEditDiagnosisLinkDlg::OnBnClickedAppendLink)
END_MESSAGE_MAP()


// CLabEditDiagnosisLinkDlg message handlers
BOOL CLabEditDiagnosisLinkDlg::OnInitDialog()
{
	try
	{
		CNxDialog::OnInitDialog();

		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnApply.AutoSet(NXB_MODIFY);
		m_btnInsert.AutoSet(NXB_MODIFY); // (j.gruber 2016-02-12 16:21) - PLID 68248 - On the advanced setup, have 2 buttons instead of one, Replace and Insert.  

		m_btnSelectOneDiagnosis.AutoSet(NXB_RIGHT);
		m_btnSelectAllDiagnosis.AutoSet(NXB_RRIGHT);
		m_btnUnSelectOneDiagnosis.AutoSet(NXB_LEFT);
		m_btnUnSelectAllDiagnosis.AutoSet(NXB_LLEFT);

		m_btnSelectOneDescription.AutoSet(NXB_RIGHT);
		m_btnSelectAllDescription.AutoSet(NXB_RRIGHT);
		m_btnUnSelectOneDescription.AutoSet(NXB_LEFT);
		m_btnUnSelectAllDescription.AutoSet(NXB_LLEFT);

		m_pDiagnosisFilterList = BindNxDataList2Ctrl(IDC_LABDIAGNOSIS_COMBO, false);
		m_pDiagnosisAvailableList = BindNxDataList2Ctrl(IDC_LABDIAGNOSIS_AVAILABLE, false);
		m_pDiagnosisSelectedList = BindNxDataList2Ctrl(IDC_LABDIAGNOSIS_SELECTED, false);

		// (j.gruber 2016-02-10 16:10) - PLID 68226 - Color the rows in the diagnosis list if they already have a microscopic description (or more) associated
		CString strFrom = "LabDiagnosisT LEFT JOIN DiagCodes ON LabDiagnosisT.DiagID = DiagCodes.ID "
			" LEFT JOIN (SELECT LabDiagnosisID FROM LabClinicalDiagnosisLinkT GROUP BY LabDiagnosisID) DiagLinkQ "
			" ON LabDiagnosisT.ID = DiagLinkQ.LabDiagnosisID " ;

		m_pDiagnosisFilterList->FromClause = _bstr_t(strFrom);
		m_pDiagnosisAvailableList->FromClause = _bstr_t(strFrom);

		m_pDiagnosisFilterList->Requery();
		m_pDiagnosisAvailableList->Requery();			

		m_pDescriptionAvailableList = BindNxDataList2Ctrl(IDC_LABCLINICALDIAGNOSIS_AVAILABLE, false);
		m_pDescriptionSelectedList = BindNxDataList2Ctrl(IDC_LABCLINICALDIAGNOSIS_SELECTED, false);

		LoadAvailableDescriptionList();

		DefaultMovingWindows();

		PositionBoxes();
					

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}



BEGIN_EVENTSINK_MAP(CLabEditDiagnosisLinkDlg, CNxDialog)
ON_EVENT(CLabEditDiagnosisLinkDlg, IDC_LABDIAGNOSIS_COMBO, 16, CLabEditDiagnosisLinkDlg::SelChosenLabdiagnosisCombo, VTS_DISPATCH)
ON_EVENT(CLabEditDiagnosisLinkDlg, IDC_LABDIAGNOSIS_COMBO, 18, CLabEditDiagnosisLinkDlg::RequeryFinishedLabdiagnosisCombo, VTS_I2)
ON_EVENT(CLabEditDiagnosisLinkDlg, IDC_LABDIAGNOSIS_AVAILABLE, 3, CLabEditDiagnosisLinkDlg::DblClickCellLabdiagnosisAvailable, VTS_DISPATCH VTS_I2)
ON_EVENT(CLabEditDiagnosisLinkDlg, IDC_LABDIAGNOSIS_SELECTED, 3, CLabEditDiagnosisLinkDlg::DblClickCellLabdiagnosisSelected, VTS_DISPATCH VTS_I2)
ON_EVENT(CLabEditDiagnosisLinkDlg, IDC_LABCLINICALDIAGNOSIS_AVAILABLE, 3, CLabEditDiagnosisLinkDlg::DblClickCellLabclinicaldiagnosisAvailable, VTS_DISPATCH VTS_I2)
ON_EVENT(CLabEditDiagnosisLinkDlg, IDC_LABCLINICALDIAGNOSIS_SELECTED, 3, CLabEditDiagnosisLinkDlg::DblClickCellLabclinicaldiagnosisSelected, VTS_DISPATCH VTS_I2)
ON_EVENT(CLabEditDiagnosisLinkDlg, IDC_LABDIAGNOSIS_AVAILABLE, 18, CLabEditDiagnosisLinkDlg::RequeryFinishedLabdiagnosisAvailable, VTS_I2)
ON_EVENT(CLabEditDiagnosisLinkDlg, IDC_LABDIAGNOSIS_COMBO, 1 /* SelChanging */, CLabEditDiagnosisLinkDlg::SelChangingLabdiagnosisCombo, VTS_DISPATCH VTS_PDISPATCH)
END_EVENTSINK_MAP()

#pragma region Datalists_Message_Handlers
/*Datalist Message Handlers*/

void CLabEditDiagnosisLinkDlg::SelChangingLabdiagnosisCombo(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}
	NxCatchAll(__FUNCTION__);
}


void CLabEditDiagnosisLinkDlg::SelChosenLabdiagnosisCombo(LPDISPATCH lpRow)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow)
		{ 
			// (j.gruber 2016-02-10 12:25) - PLID 68155 - call our helper function to do the work
			SetSelLabDiagnosisCombo(pRow);			
		}

	}NxCatchAll(__FUNCTION__)
}


void CLabEditDiagnosisLinkDlg::RequeryFinishedLabdiagnosisCombo(short nFlags)
{
	try {

		ColorRows(m_pDiagnosisFilterList);

		// (j.gruber 2016-02-10 12:25) - PLID 68155 - load the no selection row
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDiagnosisFilterList->GetNewRow();
		if (pRow)
		{
			pRow->PutValue(dlcID, UNSELECTED_DIAGNOSIS_FILTER);
			pRow->PutValue(dlcDiagCode, _variant_t(""));
			pRow->PutValue(dlcDescription, _variant_t("<No Selection>"));
			pRow->PutValue(dlcHasDescription, 0);
			m_pDiagnosisFilterList->AddRowBefore(pRow, m_pDiagnosisFilterList->GetFirstRow());


			//set the selection to be no row
			m_pDiagnosisFilterList->CurSel = pRow;

			//disable buttons
			EnableWindows(FALSE);

		}

	}NxCatchAll(__FUNCTION__)
}

// (j.gruber 2016-02-10 15:34) - PLID 68226 - need to color
void CLabEditDiagnosisLinkDlg::RequeryFinishedLabdiagnosisAvailable(short nFlags)
{
	try {
		ColorRows(m_pDiagnosisAvailableList);
	}NxCatchAll(__FUNCTION__)
}


void CLabEditDiagnosisLinkDlg::DblClickCellLabdiagnosisAvailable(LPDISPATCH lpRow, short nColIndex)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow)
		{
			// (j.gruber 2016-02-10 12:25) - PLID 68155 - call our helper function to do the work
			SelectOneDiagnosis(pRow);
		}


	}NxCatchAll(__FUNCTION__)
}


void CLabEditDiagnosisLinkDlg::DblClickCellLabdiagnosisSelected(LPDISPATCH lpRow, short nColIndex)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow)
		{
			// (j.gruber 2016-02-10 12:25) - PLID 68155 - call our helper function to do the work
			UnSelectOneDiagnosis(pRow);
		}


	}NxCatchAll(__FUNCTION__)
}


void CLabEditDiagnosisLinkDlg::DblClickCellLabclinicaldiagnosisAvailable(LPDISPATCH lpRow, short nColIndex)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow)
		{
			// (j.gruber 2016-02-10 12:25) - PLID 68155 - call our helper function to do the work
			SelectOneDescription(pRow);
		}


	}NxCatchAll(__FUNCTION__)
}


void CLabEditDiagnosisLinkDlg::DblClickCellLabclinicaldiagnosisSelected(LPDISPATCH lpRow, short nColIndex)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow)
		{
			// (j.gruber 2016-02-10 12:25) - PLID 68155 - call our helper function to do the work
			UnSelectOneDescription(pRow);
		}

	}NxCatchAll(__FUNCTION__)
}
#pragma endregion 

#pragma region Button_Message_Handlers
/*Button Message Handlers*/
void CLabEditDiagnosisLinkDlg::OnBnClickedSelectOneDiagnosis()
{
	try {
		//get the current selection
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDiagnosisAvailableList->CurSel;
		if (pRow)
		{
			// (j.gruber 2016-02-10 12:25) - PLID 68155 - call our helper function to do the work
			SelectOneDiagnosis(pRow);
		}

	}NxCatchAll(__FUNCTION__)
}


void CLabEditDiagnosisLinkDlg::OnBnClickedSelectAllDiagnosis()
{
	try {
		// (j.gruber 2016-02-10 12:25) - PLID 68155 - call our helper function to do the work
		SelectAllDiagnosis();
	}NxCatchAll(__FUNCTION__)
}


void CLabEditDiagnosisLinkDlg::OnBnClickedUnselectOneDiagnosis()
{
	try {
		//get the current selection
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDiagnosisSelectedList->CurSel;
		if (pRow)
		{
			// (j.gruber 2016-02-10 12:25) - PLID 68155 - call our helper function to do the work
			UnSelectOneDiagnosis(pRow);
		}

	}NxCatchAll(__FUNCTION__)
}


void CLabEditDiagnosisLinkDlg::OnBnClickedUnselectAllDiagnosis()
{
	try {
		// (j.gruber 2016-02-10 12:25) - PLID 68155 - call our helper function to do the work
		UnSelectAllDiagnosis();
	}NxCatchAll(__FUNCTION__)
}


void CLabEditDiagnosisLinkDlg::OnBnClickedSelectOneDescription()
{
	try {
		//get the current selection
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDescriptionAvailableList->CurSel;
		if (pRow)
		{
			SelectOneDescription(pRow);
		}

	}NxCatchAll(__FUNCTION__)
}


void CLabEditDiagnosisLinkDlg::OnBnClickedSelectAllDescription()
{
	try {
		// (j.gruber 2016-02-10 12:25) - PLID 68155 - call our helper function to do the work
		SelectAllDescription();
	}NxCatchAll(__FUNCTION__)
}


void CLabEditDiagnosisLinkDlg::OnBnClickedUnselectOneDescription()
{
	try {
		//get the current selection
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDescriptionSelectedList->CurSel;
		if (pRow)
		{
			// (j.gruber 2016-02-10 12:25) - PLID 68155 - call our helper function to do the work
			UnSelectOneDescription(pRow);
		}
		

	}NxCatchAll(__FUNCTION__)
}


void CLabEditDiagnosisLinkDlg::OnBnClickedUnselectAllDescription()
{
	try {
		// (j.gruber 2016-02-10 12:25) - PLID 68155 - call our helper function to do the work
		UnSelectAllDescription();
	}NxCatchAll(__FUNCTION__)
}



void CLabEditDiagnosisLinkDlg::OnBnClickedApplyLink()
{
	try {

		if (m_bAdvancedSetup)
		{
			// (j.gruber 2016-02-10 12:25) - PLID 68155 - warn to make sure they know they are overwriting things
			if (!WarnOnApply())
			{
				return;
			}
		}			

		//save the link
		// (j.gruber 2016-02-12 11:48) - PLID 68248 - delete from the apply button
		LinkSelectedRecords(TRUE);

		// (j.gruber 2016-02-10 16:23) - PLID 68226 - Color the rows in the diagnosis list if they already have a microscopic description (or more) associated
		SetDiagnosisHasDescription(m_pDescriptionSelectedList->GetRowCount() == 0 ? FALSE : TRUE);
		ColorRows(m_pDiagnosisSelectedList);
		ColorRows(m_pDiagnosisFilterList);
		ColorRows(m_pDiagnosisAvailableList);

	}NxCatchAll(__FUNCTION__)
}

// (j.gruber 2016-02-10 12:25) - PLID 68225 - Message handler for the Advanced/Simple setup button
void CLabEditDiagnosisLinkDlg::OnBnClickedToggleSetup()
{
	try
	{
		//reset the selections
		
		
		if (m_bAdvancedSetup)
		{
			//if we are already on advanced, we are going to simple
			m_bAdvancedSetup = FALSE;
			SetDlgItemText(IDC_TOGGLE_SETUP, "Advanced Setup");
			SetDlgItemText(IDC_APPLY_LINK, "Apply");
			EnableWindows(FALSE);
			SetSelLabDiagnosisCombo(UNSELECTED_DIAGNOSIS_FILTER);
		}
		else {
			//if we are already on simple, we are going to advanced
			m_bAdvancedSetup = TRUE;
			SetDlgItemText(IDC_TOGGLE_SETUP, "Simple Setup");
			SetDlgItemText(IDC_APPLY_LINK, "Replace");
			EnableWindows(TRUE);
		}
		PositionBoxes();	
		UnSelectAllDiagnosis();
		UnSelectAllDescription();

		
		SetDlgItemText(IDC_DESCRIPTION_SEARCH, "");
		m_bIsFiltered = FALSE;
		
	}NxCatchAll(__FUNCTION__)
}

// (j.gruber 2016-02-12 11:47) - PLID 68248 - On the advanced setup, have 2 buttons instead of one, apply and append.  The apply button behaves as it currently does, replacing what was previously there.  The append button is add only
void CLabEditDiagnosisLinkDlg::OnBnClickedAppendLink()
{
	try
	{
		
		//no warning from the append button since we aren't replacing anything

		//save the link
		// (j.gruber 2016-02-12 11:48) - PLID 68248 - do not delete from the append button
		LinkSelectedRecords(FALSE);

		// (j.gruber 2016-02-10 16:23) - PLID 68226 - Color the rows in the diagnosis list if they already have a microscopic description (or more) associated
		SetDiagnosisHasDescription(m_pDescriptionSelectedList->GetRowCount() == 0 ? FALSE : TRUE);
		ColorRows(m_pDiagnosisSelectedList);
		ColorRows(m_pDiagnosisFilterList);
		ColorRows(m_pDiagnosisAvailableList);

	}NxCatchAll(__FUNCTION__)
}


#pragma endregion 

#pragma region Arrow_Button_Helper_Functions
/*helper functions*/

// (j.gruber 2016-02-10 12:25) - PLID 68155 - Moves one diagnosis code from the Available list to the Selected list
void CLabEditDiagnosisLinkDlg::SelectOneDiagnosis(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	try {
		if (pRow)
		{
			//make sure its currently selected
			m_pDiagnosisAvailableList->CurSel = pRow;

			//copy the row from one list to the other
			m_pDiagnosisSelectedList->TakeCurrentRowAddSorted(m_pDiagnosisAvailableList, NULL);

			// (j.gruber 2016-02-12 12:48) - PLID 68248 - since we are enabling/disabling out append, might as well do both if nothing is selected for diagnosis
			EnableSaveButtons();
			
		}
	}NxCatchAll(__FUNCTION__)
}

// (j.gruber 2016-02-10 12:25) - PLID 68155 - Moves all diagnosis codes from the Available list to the Selected list
void CLabEditDiagnosisLinkDlg::SelectAllDiagnosis()
{
	try {
		m_pDiagnosisSelectedList->TakeAllRows(m_pDiagnosisAvailableList);

		// (j.gruber 2016-02-12 12:48) - PLID 68248 - since we are enabling/disabling out append, might as well do both if nothing is selected for diagnosis
		EnableSaveButtons();
		
		
	}NxCatchAll(__FUNCTION__)	

}

// (j.gruber 2016-02-10 12:25) - PLID 68155 - Moves one diagnosis code from the Selected list to the Available list
void CLabEditDiagnosisLinkDlg::UnSelectOneDiagnosis(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	try {
		
		if (pRow) {
			//make sure its currently selected
			m_pDiagnosisSelectedList->CurSel = pRow;
		
			//copy the row from one list to the other
			m_pDiagnosisAvailableList->TakeCurrentRowAddSorted(m_pDiagnosisSelectedList, NULL);	

			// (j.gruber 2016-02-12 12:48) - PLID 68248 - since we are enabling/disabling out append, might as well do both if nothing is selected for diagnosis
			EnableSaveButtons();
			
		}

	}NxCatchAll(__FUNCTION__)

}

// (j.gruber 2016-02-10 12:25) - PLID 68155 - Moves all diagnosis codes from the Selected list to the Available list
void CLabEditDiagnosisLinkDlg::UnSelectAllDiagnosis()
{
	try {	

		m_pDiagnosisAvailableList->TakeAllRows(m_pDiagnosisSelectedList);		

		// (j.gruber 2016-02-12 12:48) - PLID 68248 - since we are enabling/disabling out append, might as well do both if nothing is selected for diagnosis
		EnableSaveButtons();

	}NxCatchAll(__FUNCTION__)

}

// (j.gruber 2016-02-10 12:25) - PLID 68155 - Moves one descripion from the Available list to the Selected list
void CLabEditDiagnosisLinkDlg::SelectOneDescription(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	try {
		if (pRow)
		{
			// (j.gruber 2016-02-10 09:40) - PLID 68157 - Remove the row from the map
			long nID = VarLong(pRow->GetValue(mdlcID));
			RemoveRowFromMap(nID);			

			//make sure its currently selected
			m_pDescriptionAvailableList->CurSel = pRow;

			//copy the row from one list to the other
			m_pDescriptionSelectedList->TakeCurrentRowAddSorted(m_pDescriptionAvailableList, NULL);

			// (j.gruber 2016-02-12 11:48) - PLID 68248 - enable the button
			EnableSaveButtons();
			
		}

	}NxCatchAll(__FUNCTION__)

}


// (j.gruber 2016-02-10 12:25) - PLID 68155 - Moves all descripions from the Available list to the Selected list
void CLabEditDiagnosisLinkDlg::SelectAllDescription()
{
	try {
		// (j.gruber 2016-02-10 08:56) - PLID 68157 - if we are filtered, we can't remove everything from the map
		if (m_bIsFiltered)
		{
			//run through the list and remove any rows that are moving over to selected
			NXDATALIST2Lib::IRowSettingsPtr pRowLoop = m_pDescriptionAvailableList->GetFirstRow();
			while (pRowLoop)
			{
				long nID = VarLong(pRowLoop->GetValue(mdlcID));
				RemoveRowFromMap(nID);
				pRowLoop = pRowLoop->GetNextRow();
			}
		}
		else {
			//clear the map
			m_mapAvailList.clear();
		}
		m_pDescriptionSelectedList->TakeAllRows(m_pDescriptionAvailableList);

		// (j.gruber 2016-02-12 11:48) - PLID 68248 - enable the button
		EnableSaveButtons();
		

	}NxCatchAll(__FUNCTION__)

}

// (j.gruber 2016-02-10 12:25) - PLID 68155 - Moves one descripion from the Selected list to the Available list
void CLabEditDiagnosisLinkDlg::UnSelectOneDescription(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	try {

		if (pRow)
		{
			// (j.gruber 2016-02-10 09:42) - PLID 68157 - add the row into our available map
			m_mapAvailList.insert(std::make_pair(VarLong(pRow->GetValue(mdlcID)), pRow));		

			//make sure its currently selected
			m_pDescriptionSelectedList->CurSel = pRow;

			//copy the row from one list to the other
			m_pDescriptionAvailableList->TakeCurrentRowAddSorted(m_pDescriptionSelectedList, NULL);

			// (j.gruber 2016-02-10 09:42) - PLID 68157  - refilter
			if (m_bIsFiltered)
			{
				CString strFilter;
				GetDlgItemText(IDC_DESCRIPTION_SEARCH, strFilter);
				FilterAvailableDescriptionList(strFilter);
			}
		}

		// (j.gruber 2016-02-12 11:48) - PLID 68248 - if the selected list is empty, disable the button
		EnableSaveButtons();
			

	}NxCatchAll(__FUNCTION__)

}

// (j.gruber 2016-02-10 12:25) - PLID 68155 - Moves all descripions from the selected list to the available list
void CLabEditDiagnosisLinkDlg::UnSelectAllDescription()
{
	try {	

		BeginWaitCursor();

		// (j.gruber 2016-02-10 09:42) - PLID 68157 - add everything from selected into our available map
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDescriptionSelectedList->GetFirstRow();
		while (pRow)
		{
			m_mapAvailList.insert(std::make_pair(VarLong(pRow->GetValue(mdlcID)), pRow));
			pRow = pRow->GetNextRow();
		}	

		m_pDescriptionAvailableList->TakeAllRows(m_pDescriptionSelectedList);

		// (j.gruber 2016-02-10 09:42) - PLID 68157 - refilter
		if (m_bIsFiltered)
		{
			CString strFilter;
			GetDlgItemText(IDC_DESCRIPTION_SEARCH, strFilter);
			FilterAvailableDescriptionList(strFilter);
		}

		EndWaitCursor();

		// (j.gruber 2016-02-12 11:48) - PLID 68248 -  disable the button
		EnableSaveButtons();
		

	}NxCatchAll(__FUNCTION__)

}


#pragma endregion 

#pragma region Window_Position_Functions
// (j.gruber 2016-02-10 12:25) - PLID 68225 - Save the default layout of the buttons
void CLabEditDiagnosisLinkDlg::DefaultMovingWindows()
{
	GetDlgItem(IDC_LABCLINICALDIAGNOSIS_AVAILABLE)->GetWindowRect(m_rectDescriptionAvail);
	ScreenToClient(m_rectDescriptionAvail);
	GetDlgItem(IDC_LABCLINICALDIAGNOSIS_SELECTED)->GetWindowRect(m_rectDescriptionSelected);
	ScreenToClient(m_rectDescriptionSelected);
	GetDlgItem(IDC_SELECT_ONE_DESCRIPTION)->GetWindowRect(m_rectSelectOneDescription);
	ScreenToClient(m_rectSelectOneDescription);
	GetDlgItem(IDC_SELECT_ALL_DESCRIPTION)->GetWindowRect(m_rectSelectAllDescription);
	ScreenToClient(m_rectSelectAllDescription);
	GetDlgItem(IDC_UNSELECT_ONE_DESCRIPTION)->GetWindowRect(m_rectUnSelectOneDescription);
	ScreenToClient(m_rectUnSelectOneDescription);
	GetDlgItem(IDC_UNSELECT_ALL_DESCRIPTION)->GetWindowRect(m_rectUnSelectAllDescription);
	ScreenToClient(m_rectUnSelectAllDescription);
	GetDlgItem(IDC_DESCRIPTION_SEARCH)->GetWindowRect(m_rectDescriptionSearch);
	ScreenToClient(m_rectDescriptionSearch);
	GetDlgItem(IDCANCEL)->GetWindowRect(m_rectClose);
	ScreenToClient(m_rectClose);
	GetDlgItem(IDC_APPLY_LINK)->GetWindowRect(m_rectApply);
	ScreenToClient(m_rectApply);
	GetDlgItem(IDC_TOGGLE_SETUP)->GetWindowRect(m_rectToggle);
	ScreenToClient(m_rectToggle);
	GetWindowRect(m_rectDlg);

	GetDlgItem(IDC_LABDIAGNOSIS_AVAILABLE)->GetWindowRect(m_rectDiagAvail);
	ScreenToClient(m_rectDiagAvail);
	GetDlgItem(IDC_LABDIAGNOSIS_SELECTED)->GetWindowRect(m_rectDiagSelected);
	ScreenToClient(m_rectDiagSelected);
	GetDlgItem(IDC_SELECT_ONE_DIAGNOSIS)->GetWindowRect(m_rectDiagSelectOne);
	ScreenToClient(m_rectDiagSelectOne);
	GetDlgItem(IDC_SELECT_ALL_DIAGNOSIS)->GetWindowRect(m_rectDiagSelectAll);
	ScreenToClient(m_rectDiagSelectAll);
	GetDlgItem(IDC_UNSELECT_ONE_DIAGNOSIS)->GetWindowRect(m_rectDiagUnSelectOne);
	ScreenToClient(m_rectDiagUnSelectOne);
	GetDlgItem(IDC_UNSELECT_ALL_DIAGNOSIS)->GetWindowRect(m_rectDiagUnSelectAll);
	ScreenToClient(m_rectDiagUnSelectAll);	

	GetDlgItem(IDC_AVAILABLE_LABEL)->GetWindowRect(m_rectAvailableLabel);
	ScreenToClient(m_rectAvailableLabel);
	GetDlgItem(IDC_LAB_SELECTED_LABEL)->GetWindowRect(m_rectSelectedLabel);
	ScreenToClient(m_rectSelectedLabel);

	// (j.gruber 2016-02-12 11:48) - PLID 68248 - add m_rectAppend
	GetDlgItem(IDC_APPEND_LINK)->GetWindowRect(m_rectAppend);
	ScreenToClient(m_rectAppend);


}

// (j.gruber 2016-02-10 12:25) - PLID 68225 - Moves the windows around depending on whether we are in advanced mode or not
void CLabEditDiagnosisLinkDlg::PositionBoxes()
{
	try {
		if (m_bAdvancedSetup)
		{ 
			CRect rectDescriptionAvail, rectDescriptionSelected, rectSelectOneDescription, rectSelectAllDescription,
				rectUnSelectOneDescription, rectUnSelectAllDescription, rectDescriptionSearch, rectClose, rectApply, rectToggle, rectDlg,
				rectDiagAvail, rectDiagSelected, rectDiagSelectOne, rectDiagSelectAll, rectDiagUnSelectOne, rectDiagUnSelectAll,
				rectAvailableLabel, rectSelectedLabel, rectAppend;

			//set our rects to be the same as their member counterparts
			rectDescriptionAvail = m_rectDescriptionAvail;
			rectDescriptionSelected = m_rectDescriptionSelected;
			rectSelectOneDescription = m_rectSelectOneDescription;
			rectSelectAllDescription = m_rectSelectAllDescription;
			rectUnSelectOneDescription = m_rectUnSelectOneDescription;
			rectUnSelectAllDescription = m_rectUnSelectAllDescription;
			rectDescriptionSearch = m_rectDescriptionSearch;
			rectClose = m_rectClose;
			rectApply = m_rectApply;
			rectToggle = m_rectToggle;
			rectDlg = m_rectDlg;
			rectDiagAvail = m_rectDiagAvail;
			rectDiagSelected = m_rectDiagSelected;
			rectDiagSelectOne = m_rectDiagSelectOne;
			rectDiagSelectAll = m_rectDiagSelectAll;
			rectDiagUnSelectOne = m_rectDiagUnSelectOne;
			rectDiagUnSelectAll = m_rectDiagUnSelectAll;
			rectAvailableLabel = m_rectAvailableLabel;
			rectSelectedLabel = m_rectSelectedLabel;
			rectAppend = m_rectAppend; // (j.gruber 2016-02-12 11:48) - PLID 68248 - add m_rectAppend

			

			//we need to account for the hiding of the combo box
			CRect rectDiagnosisFilterLabel, rectDiagnosisAvail;
			GetDlgItem(IDC_LAB_FILTER_LABEL)->GetWindowRect(rectDiagnosisFilterLabel);
			ScreenToClient(rectDiagnosisFilterLabel);

		    //now subtract the height of the diagnosis boxes from the top of the filter to know how far to move everything up
			long nDiff = m_rectAvailableLabel.top - rectDiagnosisFilterLabel.top;

			//now account for the difference
			rectAvailableLabel.top -= nDiff;
			rectAvailableLabel.bottom -= nDiff;
			rectSelectedLabel.top -= nDiff;
			rectSelectedLabel.bottom -= nDiff;
			rectDescriptionAvail.top -= nDiff;
			rectDescriptionAvail.bottom -= nDiff;
			rectDescriptionSelected.top -= nDiff;
			rectDescriptionSelected.bottom -= nDiff;
			rectSelectOneDescription.top -= nDiff;
			rectSelectOneDescription.bottom -= nDiff;
			rectSelectAllDescription.top -= nDiff;
			rectSelectAllDescription.bottom -= nDiff;
			rectUnSelectOneDescription.top -= nDiff;
			rectUnSelectOneDescription.bottom -= nDiff;
			rectUnSelectAllDescription.top -= nDiff;
			rectUnSelectAllDescription.bottom -= nDiff;
			rectDescriptionSearch.top -= nDiff;
			rectDescriptionSearch.bottom -= nDiff;
			rectClose.top -= nDiff;
			rectClose.bottom -= nDiff;
			rectApply.top -= nDiff;
			rectApply.bottom -= nDiff;
			rectToggle.top -= nDiff;
			rectToggle.bottom -= nDiff;
			rectDlg.bottom -= nDiff;
			rectDiagAvail.top -= nDiff;
			rectDiagAvail.bottom -= nDiff;
			rectDiagSelected.top -= nDiff;
			rectDiagSelected.bottom -= nDiff;
			rectDiagSelectOne.top -= nDiff;
			rectDiagSelectOne.bottom -= nDiff;
			rectDiagSelectAll.top -= nDiff;
			rectDiagSelectAll.bottom -= nDiff;
			rectDiagUnSelectOne.top -= nDiff;
			rectDiagUnSelectOne.bottom -= nDiff;
			rectDiagUnSelectAll.top -= nDiff;
			rectDiagUnSelectAll.bottom -= nDiff;
			rectAppend.top -= nDiff;
			rectAppend.bottom -= nDiff;
						
			MoveWindow(rectDlg);
			GetDlgItem(IDC_AVAILABLE_LABEL)->MoveWindow(rectAvailableLabel);
			GetDlgItem(IDC_LAB_SELECTED_LABEL)->MoveWindow(rectSelectedLabel);
			GetDlgItem(IDC_LABCLINICALDIAGNOSIS_AVAILABLE)->MoveWindow(rectDescriptionAvail);
			GetDlgItem(IDC_LABCLINICALDIAGNOSIS_SELECTED)->MoveWindow(rectDescriptionSelected);
			GetDlgItem(IDC_SELECT_ONE_DESCRIPTION)->MoveWindow(rectSelectOneDescription);
			GetDlgItem(IDC_SELECT_ALL_DESCRIPTION)->MoveWindow(rectSelectAllDescription);
			GetDlgItem(IDC_UNSELECT_ONE_DESCRIPTION)->MoveWindow(rectUnSelectOneDescription);
			GetDlgItem(IDC_UNSELECT_ALL_DESCRIPTION)->MoveWindow(rectUnSelectAllDescription);
			GetDlgItem(IDC_DESCRIPTION_SEARCH)->MoveWindow(rectDescriptionSearch);
			GetDlgItem(IDCANCEL)->MoveWindow(rectClose);
			GetDlgItem(IDC_APPLY_LINK)->MoveWindow(rectApply);
			GetDlgItem(IDC_TOGGLE_SETUP)->MoveWindow(rectToggle);		
			GetDlgItem(IDC_LABDIAGNOSIS_AVAILABLE)->MoveWindow(rectDiagAvail);
			GetDlgItem(IDC_LABDIAGNOSIS_SELECTED)->MoveWindow(rectDiagSelected);
			GetDlgItem(IDC_SELECT_ONE_DIAGNOSIS)->MoveWindow(rectDiagSelectOne);
			GetDlgItem(IDC_SELECT_ALL_DIAGNOSIS)->MoveWindow(rectDiagSelectAll);
			GetDlgItem(IDC_UNSELECT_ONE_DIAGNOSIS)->MoveWindow(rectDiagUnSelectOne);
			GetDlgItem(IDC_UNSELECT_ALL_DIAGNOSIS)->MoveWindow(rectDiagUnSelectAll);
			GetDlgItem(IDC_APPEND_LINK)->MoveWindow(rectAppend); // (j.gruber 2016-02-12 11:48) - PLID 68248 - add rectAppend

			//now show the diagnosis stuff
			GetDlgItem(IDC_LABDIAGNOSIS_AVAILABLE)->ShowWindow(TRUE);
			GetDlgItem(IDC_LABDIAGNOSIS_SELECTED)->ShowWindow(TRUE);
			GetDlgItem(IDC_SELECT_ONE_DIAGNOSIS)->ShowWindow(TRUE);
			GetDlgItem(IDC_SELECT_ALL_DIAGNOSIS)->ShowWindow(TRUE);
			GetDlgItem(IDC_UNSELECT_ONE_DIAGNOSIS)->ShowWindow(TRUE);
			GetDlgItem(IDC_UNSELECT_ALL_DIAGNOSIS)->ShowWindow(TRUE);

			//hide the drop down box and label
			m_pDiagnosisFilterList->Enabled = FALSE;
			GetDlgItem(IDC_LABDIAGNOSIS_COMBO)->ShowWindow(FALSE);	
			GetDlgItem(IDC_LAB_FILTER_LABEL)->ShowWindow(FALSE);

			// (j.gruber 2016-02-12 11:48) - PLID 68248 - show the append button
			GetDlgItem(IDC_APPEND_LINK)->ShowWindow(TRUE);
			
		}
		else
		{
			
			//get the dimensions to move the search box
			CRect rectToMoveDescriptionTo;
			rectToMoveDescriptionTo.top = m_rectDiagAvail.top - m_rectDescriptionSearch.Height() - 2;
			rectToMoveDescriptionTo.bottom = m_rectDiagAvail.top -  2;
			rectToMoveDescriptionTo.left = m_rectDescriptionSearch.left;
			rectToMoveDescriptionTo.right = m_rectDescriptionSearch.right;

			//move the labels
			CRect rectToMoveAvailLabel = m_rectAvailableLabel;
			rectToMoveAvailLabel.top = rectToMoveDescriptionTo.top - m_rectAvailableLabel.Height() - 2;
			rectToMoveAvailLabel.bottom = rectToMoveDescriptionTo.top - 2;

			CRect rectToMoveSelectedLabel = m_rectSelectedLabel;
			rectToMoveSelectedLabel.top = m_rectDiagSelected.top - m_rectSelectedLabel.Height() - 2;
			rectToMoveSelectedLabel.bottom = m_rectDiagSelected.top - 2;

			//get the rect to move the apply button to			
			long nDiff = m_rectDescriptionAvail.top - m_rectDiagAvail.top;
			
			CRect rectToMoveApplyTo;
			
			rectToMoveApplyTo = m_rectApply;
			rectToMoveApplyTo.top = m_rectApply.top - nDiff;
			rectToMoveApplyTo.bottom = m_rectApply.bottom - nDiff;
			// (j.gruber 2016-02-12 11:48) - PLID 68248 - need to center it also on the dialog
			long nApplyLeft = m_rectDlg.Width() / 2 - (m_rectApply.Width() / 2);
			rectToMoveApplyTo.left = nApplyLeft;
			rectToMoveApplyTo.right = nApplyLeft + m_rectApply.Width();			


			//and the close button
			CRect rectToMoveCloseTo;
			rectToMoveCloseTo = m_rectClose;
			rectToMoveCloseTo.top = m_rectClose.top - nDiff;
			rectToMoveCloseTo.bottom = m_rectClose.bottom - nDiff;

			//and the toggle
			CRect rectToMoveToggleTo;
			rectToMoveToggleTo = m_rectToggle;
			rectToMoveToggleTo.top = m_rectToggle.top - nDiff;
			rectToMoveToggleTo.bottom = m_rectToggle.bottom - nDiff;

			//and the bottom of the dialog
			CRect rectToMoveDlg = m_rectDlg;
			rectToMoveDlg.bottom = m_rectDlg.bottom - nDiff;						

			//now start moving
			GetDlgItem(IDC_AVAILABLE_LABEL)->MoveWindow(rectToMoveAvailLabel);
			GetDlgItem(IDC_LAB_SELECTED_LABEL)->MoveWindow(rectToMoveSelectedLabel);
			GetDlgItem(IDC_LABCLINICALDIAGNOSIS_AVAILABLE)->MoveWindow(m_rectDiagAvail);
			GetDlgItem(IDC_LABCLINICALDIAGNOSIS_SELECTED)->MoveWindow(m_rectDiagSelected);
			GetDlgItem(IDC_SELECT_ONE_DESCRIPTION)->MoveWindow(m_rectDiagSelectOne);
			GetDlgItem(IDC_SELECT_ALL_DESCRIPTION)->MoveWindow(m_rectDiagSelectAll);
			GetDlgItem(IDC_UNSELECT_ONE_DESCRIPTION)->MoveWindow(m_rectDiagUnSelectOne);
			GetDlgItem(IDC_UNSELECT_ALL_DESCRIPTION)->MoveWindow(m_rectDiagUnSelectAll);
			GetDlgItem(IDC_DESCRIPTION_SEARCH)->MoveWindow(rectToMoveDescriptionTo);
			GetDlgItem(IDC_APPLY_LINK)->MoveWindow(rectToMoveApplyTo);
			GetDlgItem(IDCANCEL)->MoveWindow(rectToMoveCloseTo);
			GetDlgItem(IDC_TOGGLE_SETUP)->MoveWindow(rectToMoveToggleTo);
			GetDlgItem(IDC_APPEND_LINK)->MoveWindow(m_rectAppend);  // (j.gruber 2016-02-12 11:48) - PLID 68248 - add m_rectAppend
			MoveWindow(rectToMoveDlg);

			//now hide the diagnosis stuff
			GetDlgItem(IDC_LABDIAGNOSIS_AVAILABLE)->ShowWindow(FALSE);
			GetDlgItem(IDC_LABDIAGNOSIS_SELECTED)->ShowWindow(FALSE);
			GetDlgItem(IDC_SELECT_ONE_DIAGNOSIS)->ShowWindow(FALSE);
			GetDlgItem(IDC_SELECT_ALL_DIAGNOSIS)->ShowWindow(FALSE);
			GetDlgItem(IDC_UNSELECT_ONE_DIAGNOSIS)->ShowWindow(FALSE);
			GetDlgItem(IDC_UNSELECT_ALL_DIAGNOSIS)->ShowWindow(FALSE);

			//show the drop down box and label
			m_pDiagnosisFilterList->Enabled = TRUE;
			GetDlgItem(IDC_LABDIAGNOSIS_COMBO)->ShowWindow(TRUE);
			GetDlgItem(IDC_LAB_FILTER_LABEL)->ShowWindow(TRUE);

			// (j.gruber 2016-02-12 11:48) - PLID 68248 - hide the append button
			GetDlgItem(IDC_APPEND_LINK)->ShowWindow(FALSE);


		}

		CenterWindow();

	}NxCatchAll(__FUNCTION__)

}

#pragma endregion

#pragma region Datalist_Helper_Functions
// (j.gruber 2016-02-10 12:25) - PLID 68155 - takes a diagnosis filter list row that needs to be set to the current selection and loads the description boxes accordingly
void CLabEditDiagnosisLinkDlg::SetSelLabDiagnosisCombo(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	try {
		if (pRow)
		{
			//set the selection to this row
			m_pDiagnosisFilterList->CurSel = pRow;

			//clear the description list
			UnSelectAllDescription();

			BeginWaitCursor();

			//turn off drawing on the datalist while we are doing this
			m_pDescriptionAvailableList->SetRedraw(VARIANT_FALSE);

			long nDiagID = VarLong(pRow->GetValue(dlcID));
			if (nDiagID != UNSELECTED_DIAGNOSIS_FILTER)
			{
				//load the description list with the miroscopic descriptions for this diagnosis
				ADODB::_RecordsetPtr rsDescription = CreateParamRecordset("SELECT LabClinicalDiagnosisID "
					" FROM LabClinicalDiagnosisLinkT  "
					" WHERE LabDiagnosisID = {INT} ", nDiagID);
				
				//are there records?
				//if we are filtered, undo it to put all rows back in the list
				BOOL bReset = FALSE;
				if (!rsDescription->eof && m_bIsFiltered)
				{
					ResetAvailableDescriptionList();
					bReset = TRUE;
				}

				//now loop since we have all our records
				while (!rsDescription->eof)
				{
					long nDescriptionID = AdoFldLong(rsDescription->Fields, "LabClinicalDiagnosisID");

					//find the row in the map. since we might be filtered
					NXDATALIST2Lib::IRowSettingsPtr pRowFound = m_pDescriptionAvailableList->FindByColumn(mdlcID, nDescriptionID, NULL, FALSE);
					if (pRowFound)
					{						
						SelectOneDescription(pRowFound);

						//let's unset the cursel on the available list because it colors oddly otherwise
						m_pDescriptionAvailableList->CurSel = NULL;
					}
					rsDescription->MoveNext();
				}

				if (bReset)
				{					
					CString strFilter;
					GetDlgItemText(IDC_DESCRIPTION_SEARCH, strFilter);
					FilterAvailableDescriptionList(strFilter);
				}

			}

			m_pDescriptionAvailableList->SetRedraw(VARIANT_TRUE);

			EndWaitCursor();

			//set our boxes appropriately
			EnableWindows(nDiagID != UNSELECTED_DIAGNOSIS_FILTER);

			//if they chose -1, we already cleared the list, so there is nothing else to do
		}

	}NxCatchAll(__FUNCTION__)

}

// (j.gruber 2016-02-10 12:25) - PLID 68155 - takes an ID of a LabDiagnosisT record that needs to be the current selection in the diagnosis filter list
void CLabEditDiagnosisLinkDlg::SetSelLabDiagnosisCombo(long nID)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDiagnosisFilterList->FindByColumn(dlcID, nID, NULL, FALSE);
		if (pRow)
		{
			// (j.gruber 2016-02-10 12:25) - PLID 68155 - call our overload
			SetSelLabDiagnosisCombo(pRow);
		}

	}NxCatchAll(__FUNCTION__)


}

// (j.gruber 2016-02-10 12:25) - PLID 68155 - Disables buttons depending on parameter passed in
void CLabEditDiagnosisLinkDlg::EnableWindows(BOOL bEnabled)
{
	int nEnabled = SW_HIDE;
	if (bEnabled)
	{
		nEnabled = SW_SHOW;
	}

	GetDlgItem(IDC_SELECT_ONE_DESCRIPTION)->EnableWindow(nEnabled);
	GetDlgItem(IDC_SELECT_ALL_DESCRIPTION)->EnableWindow(nEnabled);
	GetDlgItem(IDC_UNSELECT_ONE_DESCRIPTION)->EnableWindow(nEnabled);
	GetDlgItem(IDC_UNSELECT_ALL_DESCRIPTION)->EnableWindow(nEnabled);
	
	GetDlgItem(IDC_APPLY_LINK)->EnableWindow(nEnabled);
	GetDlgItem(IDC_APPEND_LINK)->EnableWindow(nEnabled);  // (j.gruber 2016-02-12 12:56) - PLID 68248
	GetDlgItem(IDC_DESCRIPTION_SEARCH)->EnableWindow(nEnabled);

	m_pDescriptionAvailableList->Enabled = bEnabled;
	m_pDescriptionSelectedList->Enabled = bEnabled;
}

// (j.gruber 2016-02-10 12:25) - PLID 68248 - Enables/Disables the save buttons based on what is in the lists
void CLabEditDiagnosisLinkDlg::EnableSaveButtons()
{
	try
	{
		if (m_bAdvancedSetup)
		{
			//we are showing all datalists
			if (m_pDiagnosisSelectedList->GetRowCount() == 0)
			{
				//disable both buttons
				GetDlgItem(IDC_APPEND_LINK)->EnableWindow(SW_HIDE);
				GetDlgItem(IDC_APPLY_LINK)->EnableWindow(SW_HIDE);
			}
			else {
				//does description have any rows
				if (m_pDescriptionSelectedList->GetRowCount() == 0)
				{
					//only disable append
					GetDlgItem(IDC_APPEND_LINK)->EnableWindow(SW_HIDE);
					GetDlgItem(IDC_APPLY_LINK)->EnableWindow(SW_SHOW);

				}
				else
				{
					//both buttons are enabled
					GetDlgItem(IDC_APPEND_LINK)->EnableWindow(SW_SHOW);
					GetDlgItem(IDC_APPLY_LINK)->EnableWindow(SW_SHOW);
				}
			}

		}
		else {

			//check the filter list
			NXDATALIST2Lib::IRowSettingsPtr pCurSel = m_pDiagnosisFilterList->CurSel;
			long nID = -1;
			if (pCurSel)
			{
				nID = VarLong(pCurSel->GetValue(dlcID));
			}

			//does description have any rows
			if (m_pDescriptionSelectedList->GetRowCount() == 0)
			{
				if (nID != -1)
				{
					GetDlgItem(IDC_APPLY_LINK)->EnableWindow(SW_SHOW);
				}
				else
				{
					GetDlgItem(IDC_APPLY_LINK)->EnableWindow(SW_HIDE);
				}

			}
			else
			{
				//both buttons are enabled
				GetDlgItem(IDC_APPLY_LINK)->EnableWindow(SW_SHOW);
			}
		}
	
	
	}NxCatchAll(__FUNCTION__)
}


//this function sets the selected diagnosis rows as having a description (or not) depending on parameter passed in
void CLabEditDiagnosisLinkDlg::SetDiagnosisHasDescription(BOOL bSet)
{
	try
	{
		if (m_bAdvancedSetup)
		{
			//loop through the selected datalist and set the column appropriately
			NXDATALIST2Lib::IRowSettingsPtr pRowLoop = m_pDiagnosisSelectedList->GetFirstRow();
			while (pRowLoop)
			{
				pRowLoop->PutValue(dlcHasDescription, bSet ? 1 : 0);
				
				//also set the selection on the filter list
				long nID = VarLong(pRowLoop->GetValue(dlcID));
				NXDATALIST2Lib::IRowSettingsPtr pRowFilter = m_pDiagnosisFilterList->FindByColumn(dlcID, nID, NULL, FALSE);
				if (pRowFilter)
				{
					pRowFilter->PutValue(dlcHasDescription, bSet ? 1 : 0);
				}

				pRowLoop = pRowLoop->GetNextRow();
			}
		}
		else {

			//first set the filter list
			NXDATALIST2Lib::IRowSettingsPtr pCurSel = m_pDiagnosisFilterList->CurSel;
			if (pCurSel)
			{
				//set the has description
				pCurSel->PutValue(dlcHasDescription, bSet ? 1 : 0);

				long nID = VarLong(pCurSel->GetValue(dlcID));
				//find it in the available list
				NXDATALIST2Lib::IRowSettingsPtr pRowFound = m_pDiagnosisAvailableList->FindByColumn(dlcID, nID, NULL, FALSE);
				if (pRowFound)
				{
					pRowFound->PutValue(dlcHasDescription, bSet ? 1 : 0);
				}
			}

		}
		

	}NxCatchAll(__FUNCTION__)


}
void CLabEditDiagnosisLinkDlg::ColorRows(NXDATALIST2Lib::_DNxDataListPtr pDataList)
{
	try
	{
		// (j.gruber 2016-02-10 15:34) - PLID 68226 - loop through the list and color if the already have descriptions
		NXDATALIST2Lib::IRowSettingsPtr pRowLoop = pDataList->GetFirstRow();
		while (pRowLoop)
		{
			long nHasDescription = VarLong(pRowLoop->GetValue(dlcHasDescription));
			if (nHasDescription == 1)
			{
				//color the row
				pRowLoop->ForeColor = EXISTING_DESCRIPTIONS;
			}
			else {
				pRowLoop->ForeColor = NO_DESCRIPTIONS;
			}
			pRowLoop = pRowLoop->GetNextRow();
		}

	}NxCatchAll(__FUNCTION__)

}

#pragma endregion

#pragma region Search_Message_Handlers
// (j.gruber 2016-02-10 12:40) - PLID 68157 - Message handler when the search box changes
void CLabEditDiagnosisLinkDlg::OnEnChangeDescriptionSearch()
{
	try
	{
		CString strEntry;
		GetDlgItemText(IDC_DESCRIPTION_SEARCH, strEntry);

		if (strEntry.GetLength() >= 3)
		{
			FilterAvailableDescriptionList(strEntry);
			m_bIsFiltered = TRUE;
		}
		else {
			if (m_bIsFiltered) {
				ResetAvailableDescriptionList();
				m_bIsFiltered = FALSE;
			}
		}
	}NxCatchAll(__FUNCTION__)

}
#pragma endregion

#pragma region Search_Helper_Functions
/*Filter Helper Functions*/
// (j.gruber 2016-02-10 12:40) - PLID 68157 - called when the search box reaches more than 3 characters, filter the available description list for the text in the box
void CLabEditDiagnosisLinkDlg::FilterAvailableDescriptionList(CiString strFilter)
{
	BeginWaitCursor();

	// (j.gruber 2014-03-10 14:41) - PLID 61201 - take out leading spaces
	strFilter = strFilter.TrimLeft();

	//first clear the list
	m_pDescriptionAvailableList->Clear();

	//loop through our array of rows to filter for our string
	for each(MicroscopicDescription pItem in m_mapAvailList)
	{
		NXDATALIST2Lib::IRowSettingsPtr pRow = pItem.second;

		if (pRow)
		{
			//check the description column
			CiString cisValue = VarString(pRow->GetValue(mdlcDescription));
			if (cisValue.Find(strFilter) != -1)
			{
				AddRowFromMap(pRow);
			}
		}
	}

	EndWaitCursor();
}

// (j.gruber 2016-02-10 12:40) - PLID 68157 - adds a row to the available description list from our map
void CLabEditDiagnosisLinkDlg::AddRowFromMap(NXDATALIST2Lib::IRowSettingsPtr pRowFromMap)
{
	NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pDescriptionAvailableList->GetNewRow();
	for (int i = 0; i < m_pDescriptionAvailableList->GetColumnCount(); i++)
	{
		pNewRow->PutValue(i, pRowFromMap->GetValue(i));
	}
	m_pDescriptionAvailableList->AddRowSorted(pNewRow, NULL);
}



// (j.gruber 2016-02-10 12:40) - PLID 68157 - resets the available description list with the items in the map
void CLabEditDiagnosisLinkDlg::ResetAvailableDescriptionList()
{
	BeginWaitCursor();

	m_pDescriptionAvailableList->Clear();

	for each(MicroscopicDescription pItem in m_mapAvailList)
	{
		NXDATALIST2Lib::IRowSettingsPtr pRow = pItem.second;
		if (pRow)
		{
			AddRowFromMap(pRow);
		}
	}

	EndWaitCursor();

}

// (j.gruber 2016-02-10 12:40) - PLID 68157 - helper function to remove items from the map
void CLabEditDiagnosisLinkDlg::RemoveRowFromMap(long nID)
{
	MicroscopicDescriptionMapIterator itFind = m_mapAvailList.find(nID);
	if (itFind != m_mapAvailList.end())
	{
		//we found it, now remove it
		m_mapAvailList.erase(itFind);
	}
}

// (j.gruber 2016-02-10 12:26) - PLID 68157 - Load the Available Description List and our map
void CLabEditDiagnosisLinkDlg::LoadAvailableDescriptionList()
{
	try
	{
		ADODB::_RecordsetPtr rsDescriptions = CreateRecordsetStd(" SELECT ID, Description FROM LabClinicalDiagnosisT ");
		while (!rsDescriptions->eof)
		{
			//add the row to our list
			NXDATALIST2Lib::IRowSettingsPtr pRowAdded = m_pDescriptionAvailableList->GetNewRow();
			long nID = AdoFldLong(rsDescriptions->Fields, "ID");
			pRowAdded->PutValue(mdlcID, nID);
			pRowAdded->PutValue(mdlcDescription, _variant_t(AdoFldString(rsDescriptions->Fields, "Description")));
			m_pDescriptionAvailableList->AddRowSorted(pRowAdded, NULL);

			//insert into our map
			m_mapAvailList.insert(std::make_pair(nID, pRowAdded));

			rsDescriptions->MoveNext();

		}

	}NxCatchAll(__FUNCTION__)

}
#pragma endregion 

#pragma region Saving_Functions
/*Saving Functions*/

// (j.gruber 2016-02-10 12:25) - PLID 68155 - save the link to data
// (j.gruber 2016-02-12 11:48) - PLID 68248 - added option to delete all or not
void CLabEditDiagnosisLinkDlg::LinkSelectedRecords(BOOL bDeleteAll)
{
	try
	{
		CSqlFragment sqlCommit(" SET NOCOUNT ON; \r\n "
			" SET XACT_ABORT ON \r\n"
			" BEGIN TRAN \r\n");

		//get our Microscopic description list IDs
		NXDATALIST2Lib::IRowSettingsPtr pRowDescription = m_pDescriptionSelectedList->GetFirstRow();
		CArray<long, long> aryLabDescriptions;
		while (pRowDescription)
		{
			long nDescID = VarLong(pRowDescription->GetValue(mdlcID));
			aryLabDescriptions.Add(nDescID);

			pRowDescription = pRowDescription->GetNextRow();
		}

		if (m_bAdvancedSetup)
		{
			//run through our diagnosis code list and create our delete statements
			NXDATALIST2Lib::IRowSettingsPtr pRowLoop = m_pDiagnosisSelectedList->GetFirstRow();
			while (pRowLoop)
			{
				long nLabDiagnosisID = VarLong(pRowLoop->GetValue(dlcID));

				// (j.gruber 2016-02-12 11:48) - PLID 68248 - added option to delete or not
				if (bDeleteAll)
				{				
					//first delete the existing stuff
					sqlCommit += CSqlFragment(" DELETE FROM LabClinicalDiagnosisLinkT WHERE LabDiagnosisID = {INT} \r\n", nLabDiagnosisID);
				}
				else {
					sqlCommit += CSqlFragment(" DELETE FROM LabClinicalDiagnosisLinkT WHERE LabDiagnosisID = {INT} AND LabClinicalDiagnosisID IN ({INTARRAY}) \r\n", nLabDiagnosisID, aryLabDescriptions);
				}

				//now add the microscopic descriptions
				for (int i = 0; i < aryLabDescriptions.GetSize(); i++)
				{
					sqlCommit += CSqlFragment(" INSERT INTO LabClinicalDiagnosisLinkT (LabDiagnosisID, LabClinicalDiagnosisID) VALUES ({INT}, {INT}) \r\n", nLabDiagnosisID, aryLabDescriptions.GetAt(i));
				}

				pRowLoop = pRowLoop->GetNextRow();
			}
		}
		else {

			//we aren't in advanced mode, just save with one diagnosis and no warning
			NXDATALIST2Lib::IRowSettingsPtr pRowDiag = m_pDiagnosisFilterList->CurSel;
			if (pRowDiag)
			{
				long nLabDiagnosisID = VarLong(pRowDiag->GetValue(dlcID));

				//first delete the existing stuff
				sqlCommit += CSqlFragment(" DELETE FROM LabClinicalDiagnosisLinkT WHERE LabDiagnosisID = {INT} \r\n", nLabDiagnosisID);

				//now add the microscopic descriptions
				for (int i = 0; i < aryLabDescriptions.GetSize(); i++)
				{
					sqlCommit += CSqlFragment(" INSERT INTO LabClinicalDiagnosisLinkT (LabDiagnosisID, LabClinicalDiagnosisID) VALUES ({INT}, {INT}) \r\n", nLabDiagnosisID, aryLabDescriptions.GetAt(i));
				}
			}
		}

		//now commit
		sqlCommit += CSqlFragment(" COMMIT TRAN \r\n");

		//now commit it
		ADODB::_RecordsetPtr rsCommit = CreateParamRecordset(sqlCommit);


	}NxCatchAll(__FUNCTION__)

}

// (j.gruber 2016-02-10 12:25) - PLID 68155 - called when clicking the apply button when in advanced mode
// warns if you are overwriting existing links
BOOL CLabEditDiagnosisLinkDlg::WarnOnApply()
{
	//first check to see if any of the diagnosis codes already have microscopic descriptions
	CArray<long, long> aryLabDiagIDs;
	NXDATALIST2Lib::IRowSettingsPtr pLoopRow = m_pDiagnosisSelectedList->GetFirstRow();
	while (pLoopRow)
	{
		aryLabDiagIDs.Add(VarLong(pLoopRow->GetValue(dlcID)));
		pLoopRow = pLoopRow->GetNextRow();
	}

	CString strDiagnoses = "";

	long nCount = 0;
	long nCountDescriptions = m_pDescriptionSelectedList->GetRowCount();

	//call our recordset to see if these diagnoses already have descriptions linked to them
	ADODB::_RecordsetPtr rsCheck = CreateParamRecordset("SELECT LabDiagnosisID, Description FROM LabClinicalDiagnosisLinkT "
		" INNER JOIN LabDiagnosisT ON LabClinicalDiagnosisLinkT.LabDiagnosisID = LabDiagnosisT.ID "
		" WHERE LabDiagnosisID IN ({INTARRAY}) "
		" GROUP BY LabDiagnosisID, Description ", aryLabDiagIDs);
	while (!rsCheck->eof)
	{
		strDiagnoses += AdoFldString(rsCheck->Fields, "Description") + "\r\n";
		rsCheck->MoveNext();
		nCount++;
	}

	if (!strDiagnoses.IsEmpty())
	{
		CString strDiagFormat, strDiagFormat2, strDiagFormat3;
		if (nCount == 1)
		{
			strDiagFormat = "diagnosis has";
			strDiagFormat2 = "this";			
		}
		else {
			strDiagFormat = "diagnoses have";
			strDiagFormat2 = "these";
			
		}

		if (nCountDescriptions == 0)
		{
			strDiagFormat3 = "have no linked microscopic descriptions";
		}
		else if (nCountDescriptions == 1)
		{
			strDiagFormat3 = "be the selected microscopic description";
		}
		else
		{
			strDiagFormat3 = "be the selected microscopic descriptions";
		}

		if (IDNO == MsgBox(MB_YESNO, "The following %s at least one microscopic description linked.  By continuing, %s will be changed to %s.\r\n\r\n"
			"%s"
			"\r\nAre you sure you wish to continue?", strDiagFormat, strDiagFormat2, strDiagFormat3, strDiagnoses))
		{
			return FALSE;
		}
	}

	//if we got here, there aren't any diagnoses already linked
	return TRUE;
}
#pragma endregion



