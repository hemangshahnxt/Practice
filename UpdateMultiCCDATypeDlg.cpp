// UpdateMultiCCDATypeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "UpdateMultiCCDATypeDlg.h"
#include "CPTAdditionalInfoDlg.h" // to get enum CCDAProcedureType

// a.walling says I've created a false dependency on CCPTAdditionalInfoDlg...
/* 
a.walling says:
A better solution is to move that enum into an existing or new header file nearby 
other related definitions. If this is saved to data, then it should rarely (if ever) 
change, so it is not that big of a deal to put it into one of the big headers (like 
globalfinancialutils for example) but it might warrant its own .h file. You don't 
need to worry about creating too many .h files! See if there are other related 
definitions around, and if so try to keep them together -- this may involve moving 
around some of the enum definitions from other .h or local .cpp files.
*/

enum CCDATypeListColumns{
	ccdatlcID = 0,
	ccdatlcName,
};

enum CodeListColumns{
	clcID = 0,
	clcCode,
	clcCodeDesc,
	clcCurrentType,
};

// (r.goldschmidt 2014-05-06 11:34) - PLID 61789 - Created
// CUpdateMultiCCDATypeDlg dialog

IMPLEMENT_DYNAMIC(CUpdateMultiCCDATypeDlg, CNxDialog)

CUpdateMultiCCDATypeDlg::CUpdateMultiCCDATypeDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CUpdateMultiCCDATypeDlg::IDD, pParent)
{}

CUpdateMultiCCDATypeDlg::~CUpdateMultiCCDATypeDlg()
{}

void CUpdateMultiCCDATypeDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_REMOVE_ALL, m_btnRemoveAll);
	DDX_Control(pDX, IDC_REMOVE, m_btnRemove);
	DDX_Control(pDX, IDC_ADD, m_btnAdd);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);

}

BEGIN_MESSAGE_MAP(CUpdateMultiCCDATypeDlg, CNxDialog)
	ON_BN_CLICKED(IDCANCEL, &CUpdateMultiCCDATypeDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &CUpdateMultiCCDATypeDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_REMOVE_ALL, &CUpdateMultiCCDATypeDlg::OnBnClickedRemoveAll)
	ON_BN_CLICKED(IDC_REMOVE, &CUpdateMultiCCDATypeDlg::OnBnClickedRemove)
	ON_BN_CLICKED(IDC_ADD, &CUpdateMultiCCDATypeDlg::OnBnClickedAdd)
END_MESSAGE_MAP()

BOOL CUpdateMultiCCDATypeDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try{

		//setup the NxIconButtons
		m_btnAdd.AutoSet(NXB_DOWN);
		m_btnRemove.AutoSet(NXB_UP);
		m_btnRemoveAll.AutoSet(NXB_UUP);
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CLOSE);

		//setup the datalists
		m_pUnselected = BindNxDataList2Ctrl(IDC_UNSELECTED_CPTCODE_LIST, GetRemoteData(), true);
		// the effective query that builds IDC_UNSELECTED_CPTCODE_LIST (as of r.goldschmidt 2014-05-06 11:34):
		/*	
			" "
			"SELECT CPTCodeT.ID, "
			"	CPTCodeT.Code, "
			"	ServiceT.Name, "
			"	CASE WHEN CPTCodeT.CCDAType = 1 THEN 'Procedure' "
			"		WHEN CPTCodeT.CCDAType = 2 THEN 'Observation' "
			"		WHEN CPTCodeT.CCDAType = 3 THEN 'Act' "
			"		WHEN CPTCodeT.CCDAType = 4 THEN 'Encounter' "
			"		ELSE '' END AS CCDAType "
			"FROM CPTCodeT "
			"INNER JOIN ServiceT ON CPTCodeT.ID = ServiceT.ID "
			" " 
		*/

		m_pSelected = BindNxDataList2Ctrl(IDC_SELECTED_CPTCODE_LIST, false);
		m_pCCDAType = BindNxDataList2Ctrl(IDC_CCDA_TYPE_LIST, false);
		{
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCCDAType->GetNewRow();

			pRow->PutValue(ccdatlcID, (long)cptInvalidType);
			pRow->PutValue(ccdatlcName, _bstr_t("<No Selection>"));
			m_pCCDAType->AddRowAtEnd(pRow, NULL);

			pRow = m_pCCDAType->GetNewRow();
			pRow->PutValue(ccdatlcID, (long)cptProcedure);
			pRow->PutValue(ccdatlcName, _bstr_t("Procedure"));
			m_pCCDAType->AddRowAtEnd(pRow, NULL);

			pRow = m_pCCDAType->GetNewRow();
			pRow->PutValue(ccdatlcID, (long)cptObservation);
			pRow->PutValue(ccdatlcName, _bstr_t("Observation"));
			m_pCCDAType->AddRowAtEnd(pRow, NULL);

			pRow = m_pCCDAType->GetNewRow();
			pRow->PutValue(ccdatlcID, (long)cptAct);
			pRow->PutValue(ccdatlcName, _bstr_t("Act"));
			m_pCCDAType->AddRowAtEnd(pRow, NULL);

			pRow = m_pCCDAType->GetNewRow();
			pRow->PutValue(ccdatlcID, (long)cptEncounter);
			pRow->PutValue(ccdatlcName, _bstr_t("Encounter"));
			m_pCCDAType->AddRowAtEnd(pRow, NULL);
		}
		
		m_pCCDAType->SetSelByColumn(ccdatlcID, (long)cptInvalidType);

	}NxCatchAll(__FUNCTION__);

	return TRUE;

}


// CUpdateMultiCCDATypeDlg message handlers

// Close dialog, don't do anything else
void CUpdateMultiCCDATypeDlg::OnBnClickedCancel()
{
	try{
		CNxDialog::OnCancel();
	}NxCatchAll(__FUNCTION__);
}

// Update the CPT Code list. Leave dialog open for further changes if desired.
void CUpdateMultiCCDATypeDlg::OnBnClickedOk()
{
	try{
		// make sure something is in the selected service code list
		long nSelRows = m_pSelected->GetRowCount();
		if (nSelRows == 0) {
			MsgBox("You must have at least 1 item selected to update.");
			return;
		}

		// ID of CCDA type to update to
		CCDAProcedureType eCCDAType = (CCDAProcedureType)VarLong(m_pCCDAType->GetCurSel()->GetValue(ccdatlcID), -1);
		// Name of CCDA type to update to
		CString strCCDATypeToUpdate = (eCCDAType == cptInvalidType) ? "" : VarString(m_pCCDAType->GetCurSel()->GetValue(ccdatlcName), "");

		// confirm that the user wishes to update
		if (MsgBox(MB_YESNO, "This action will set %s as the default type\nfor the %li service codes in the selected list.\n\nAre you sure you wish to do this?",
			eCCDAType == cptInvalidType ? "<No Selection>" : strCCDATypeToUpdate, nSelRows) == IDNO)
			return;

		// green light to update CPTCodesT

		// array of CPTCodeT.ID values that get CCDAType updated
		CArray<long> aryIDsToUpdate;

		// iterate through selected list to build array of IDs 
		//  and update Current Type value for codes in selected list
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSelected->GetFirstRow();
		while (pRow) {
			aryIDsToUpdate.Add(VarLong(pRow->GetValue(clcID))); // add ID to array of IDs that need updating
			pRow->PutValue(clcCurrentType, bstr_t(strCCDATypeToUpdate)); // put new value in Current Type field
			pRow = pRow->GetNextRow();
		}

		//run the update query
		ExecuteParamSql(" "
			"UPDATE CPTCodeT "
			"	SET CCDAType = {VT_I4} "
			"	WHERE ID IN ({INTARRAY}) "
			" ",
			eCCDAType == cptInvalidType ? g_cvarNull : (long)eCCDAType,
			aryIDsToUpdate);

		MsgBox("The selected service codes have been updated to use the new type.");

	}NxCatchAll(__FUNCTION__);

}

// Move all items in Selected Service Code List back to Unselected Service Code List
void CUpdateMultiCCDATypeDlg::OnBnClickedRemoveAll()
{
	try{
		m_pUnselected->TakeAllRows(m_pSelected);
	}NxCatchAll(__FUNCTION__);
}

// Move row(s) selected in Selected Service Code List back to Unselected Service Code List
void CUpdateMultiCCDATypeDlg::OnBnClickedRemove()
{
	try{
		m_pUnselected->TakeCurrentRowAddSorted(m_pSelected, NULL);
	}NxCatchAll(__FUNCTION__);
}

// Move row(s) selected in Unselected Service Code List to Selected Service Code List
void CUpdateMultiCCDATypeDlg::OnBnClickedAdd()
{
	try{
		m_pSelected->TakeCurrentRowAddSorted(m_pUnselected, NULL);
	}NxCatchAll(__FUNCTION__);
}


BEGIN_EVENTSINK_MAP(CUpdateMultiCCDATypeDlg, CNxDialog)
	ON_EVENT(CUpdateMultiCCDATypeDlg, IDC_UNSELECTED_CPTCODE_LIST, 3, CUpdateMultiCCDATypeDlg::DblClickCellUnselectedList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CUpdateMultiCCDATypeDlg, IDC_SELECTED_CPTCODE_LIST, 3, CUpdateMultiCCDATypeDlg::DblClickCellSelectedList, VTS_DISPATCH VTS_I2)
END_EVENTSINK_MAP()

// Move double clicked row to Selected Service Code List
void CUpdateMultiCCDATypeDlg::DblClickCellUnselectedList(LPDISPATCH lpRow, short nColIndex)
{
	try{
		m_pSelected->TakeCurrentRowAddSorted(m_pUnselected, NULL);
	}NxCatchAll(__FUNCTION__);
}

// Move double clicked row to Unselected Service Code List
void CUpdateMultiCCDATypeDlg::DblClickCellSelectedList(LPDISPATCH lpRow, short nColIndex)
{
	try{
		m_pUnselected->TakeCurrentRowAddSorted(m_pSelected, NULL);
	}NxCatchAll(__FUNCTION__);
}
