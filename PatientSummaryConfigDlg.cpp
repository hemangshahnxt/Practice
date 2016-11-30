// PatientSummaryConfigDlg.cpp : implementation file
//

// (j.gruber 2010-06-15 14:28) - PLID  39174  - Created

#include "stdafx.h"
#include "Practice.h"
#include "PatientSummaryConfigDlg.h"
#include "PatientsRc.h"

enum ConfigListColumns {
	clcName = 0,
	clcBase,
	clcSort,
	clcShow,
};

// CPatientSummaryConfigDlg dialog

IMPLEMENT_DYNAMIC(CPatientSummaryConfigDlg, CNxDialog)

CPatientSummaryConfigDlg::CPatientSummaryConfigDlg(CMap<long, long, CString, LPCTSTR> *pmapCustomFields, CWnd* pParent /*=NULL*/)
	: CNxDialog(CPatientSummaryConfigDlg::IDD, pParent)
{
	m_pmapCustomFields = pmapCustomFields;

}

CPatientSummaryConfigDlg::~CPatientSummaryConfigDlg()
{
}

void CPatientSummaryConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PSC_MOVE_ONE_LEFT, m_btnMoveOneLeft);
	DDX_Control(pDX, IDC_PSC_MOVE_ONE_RIGHT, m_btnMoveOneRight);
	DDX_Control(pDX, IDC_PSC_LEFT_MOVE_DOWN, m_btnLeftMoveDown);
	DDX_Control(pDX, IDC_PSC_LEFT_MOVE_UP, m_btnLeftMoveUp);
	DDX_Control(pDX, IDC_PSC_RIGHT_MOVE_DOWN, m_btnRightMoveDown);
	DDX_Control(pDX, IDC_PSC_RIGHT_MOVE_UP, m_btnRightMoveUp);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_ST_DESCRIPTION, m_stDesc);
}


BEGIN_MESSAGE_MAP(CPatientSummaryConfigDlg, CNxDialog)
	ON_BN_CLICKED(IDC_PSC_LEFT_MOVE_UP, &CPatientSummaryConfigDlg::OnBnClickedPscLeftMoveUp)
	ON_BN_CLICKED(IDC_PSC_LEFT_MOVE_DOWN, &CPatientSummaryConfigDlg::OnBnClickedPscLeftMoveDown)
	ON_BN_CLICKED(IDC_PSC_MOVE_ONE_RIGHT, &CPatientSummaryConfigDlg::OnBnClickedPscMoveOneRight)
	ON_BN_CLICKED(IDC_PSC_MOVE_ONE_LEFT, &CPatientSummaryConfigDlg::OnBnClickedPscMoveOneLeft)
	ON_BN_CLICKED(IDC_PSC_RIGHT_MOVE_UP, &CPatientSummaryConfigDlg::OnBnClickedPscRightMoveUp)
	ON_BN_CLICKED(IDC_PSC_RIGHT_MOVE_DOWN, &CPatientSummaryConfigDlg::OnBnClickedPscRightMoveDown)
	ON_BN_CLICKED(IDOK, &CPatientSummaryConfigDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CPatientSummaryConfigDlg message handlers

void CPatientSummaryConfigDlg::OnBnClickedPscLeftMoveUp()
{
	try {

		MoveRowUp(m_pLeftList);
		UpdateButtons();

	}NxCatchAll(__FUNCTION__);
}

void CPatientSummaryConfigDlg::OnBnClickedPscLeftMoveDown()
{
	try {

		MoveRowDown(m_pLeftList);
		UpdateButtons();

	}NxCatchAll(__FUNCTION__);
}

void CPatientSummaryConfigDlg::OnBnClickedPscMoveOneRight()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pLeftList->CurSel;
		if (pRow) {
			NXDATALIST2Lib::IRowSettingsPtr pRowChecked;
			NXDATALIST2Lib::IRowSettingsPtr pRowLoop = m_pRightList->GetFirstRow();
			while (pRowLoop) {
				if (VarBool(pRowLoop->GetValue(clcShow), FALSE)) {
					pRowChecked = pRowLoop;
				}
				pRowLoop = pRowLoop->GetNextRow();
			}			

			if (pRowChecked) {
				m_pLeftList->RemoveRow(pRow);
				if (pRowChecked == m_pRightList->GetLastRow()) {
					m_pRightList->AddRowAtEnd(pRow, NULL);
				}
				else {
					m_pRightList->AddRowBefore(pRow, pRowChecked->GetNextRow());
				}
			}
			else {
				m_pLeftList->RemoveRow(pRow);
				m_pRightList->AddRowBefore(pRow, m_pRightList->GetFirstRow());
			}
			m_pRightList->EnsureRowInView(pRow);
			FixSorts();
			UpdateButtons();
		}

	}NxCatchAll(__FUNCTION__);
}

void CPatientSummaryConfigDlg::UpdateButtons()
{

	//check the left list
	NXDATALIST2Lib::IRowSettingsPtr pCurSel = m_pLeftList->CurSel;
	if (pCurSel == m_pLeftList->GetFirstRow()) {
		m_btnLeftMoveUp.EnableWindow(FALSE);
		m_btnLeftMoveDown.EnableWindow(TRUE);
	}
	else if (pCurSel == m_pLeftList->GetLastRow()) {
		m_btnLeftMoveUp.EnableWindow(TRUE);
		m_btnLeftMoveDown.EnableWindow(FALSE);
	}
	else if (pCurSel == NULL) {
		m_btnLeftMoveUp.EnableWindow(FALSE);
		m_btnLeftMoveDown.EnableWindow(FALSE);
	}
	else {
		m_btnLeftMoveUp.EnableWindow(TRUE);
		m_btnLeftMoveDown.EnableWindow(TRUE);
	}

	//now the right list
	pCurSel = m_pRightList->CurSel;
	if (pCurSel == m_pRightList->GetFirstRow()) {
		m_btnRightMoveUp.EnableWindow(FALSE);
		m_btnRightMoveDown.EnableWindow(TRUE);
	}
	else if (pCurSel == m_pRightList->GetLastRow()) {
		m_btnRightMoveUp.EnableWindow(TRUE);
		m_btnRightMoveDown.EnableWindow(FALSE);
	}
	else if (pCurSel == NULL) {
		m_btnRightMoveUp.EnableWindow(FALSE);
		m_btnRightMoveDown.EnableWindow(FALSE);
	}
	else {
		m_btnRightMoveUp.EnableWindow(TRUE);
		m_btnRightMoveDown.EnableWindow(TRUE);
	}

	if (m_pLeftList->GetRowCount() <= 1) {
		m_btnLeftMoveUp.EnableWindow(FALSE);
		m_btnLeftMoveDown.EnableWindow(FALSE);
	}

	if (m_pRightList->GetRowCount() <= 1) {
		m_btnRightMoveUp.EnableWindow(FALSE);
		m_btnRightMoveDown.EnableWindow(FALSE);
	}
	

	/*if (m_pLeftList->GetRowCount() == 0) {
		m_btnMoveOneRight.EnableWindow(FALSE);
		m_btnMoveOneLeft.EnableWindow(TRUE);
	}
	else {
		m_btnMoveOneRight.EnableWindow(TRUE);
		m_btnMoveOneLeft.EnableWindow(TRUE);
	}

	if (m_pRightList->GetRowCount() == 0) {
		m_btnMoveOneRight.EnableWindow(TRUE);
		m_btnMoveOneLeft.EnableWindow(FALSE);
	}*/

}


void CPatientSummaryConfigDlg::OnBnClickedPscMoveOneLeft()
{
	try {		
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pRightList->CurSel;
		if (pRow) {
			
			//find the last row that is checked
			NXDATALIST2Lib::IRowSettingsPtr pRowChecked;
			NXDATALIST2Lib::IRowSettingsPtr pRowLoop = m_pLeftList->GetFirstRow();
			while (pRowLoop) {
				if (VarBool(pRowLoop->GetValue(clcShow), FALSE)) {
					pRowChecked = pRowLoop;
				}
				pRowLoop = pRowLoop->GetNextRow();
			}			

			if (pRowChecked) {
				m_pRightList->RemoveRow(pRow);
				if (pRowChecked == m_pLeftList->GetLastRow()) {
					m_pLeftList->AddRowAtEnd(pRow, NULL);
				}
				else {
					m_pLeftList->AddRowBefore(pRow, pRowChecked->GetNextRow());
				}
			}
			else {
				m_pRightList->RemoveRow(pRow);
				m_pLeftList->AddRowBefore(pRow, m_pLeftList->GetFirstRow());
			}
			m_pLeftList->EnsureRowInView(pRow);
			FixSorts();
			UpdateButtons();
		}
	}NxCatchAll(__FUNCTION__);
}

void CPatientSummaryConfigDlg::OnBnClickedPscRightMoveUp()
{
	try {

		MoveRowUp(m_pRightList);
		UpdateButtons();

	}NxCatchAll(__FUNCTION__);
}

void CPatientSummaryConfigDlg::OnBnClickedPscRightMoveDown()
{
	try {

		MoveRowDown(m_pRightList);
		UpdateButtons();

	}NxCatchAll(__FUNCTION__);
}

BOOL CPatientSummaryConfigDlg::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();
		m_btnMoveOneLeft.AutoSet(NXB_LEFT);
		m_btnMoveOneRight.AutoSet(NXB_RIGHT);

		m_btnLeftMoveDown.AutoSet(NXB_DOWN);
		m_btnLeftMoveUp.AutoSet(NXB_UP);

		m_btnRightMoveDown.AutoSet(NXB_DOWN);
		m_btnRightMoveUp.AutoSet(NXB_UP);

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_stDesc.SetWindowText("To make a field show in the top box of the Patient Summary, check the checkbox next to its name.  Move items between the two boxes below in order to have them show on the right or the left of the Patient Summary.  You can rearrange the order of items by using the up and down arrows on the left for the left side of the Patient Summary and the up and down arrows on the right for the right side of the Patient Summary.");
	
		//configrt cache called by caller
		
		m_pLeftList = BindNxDataList2Ctrl(IDC_PAT_SUM_CONFIG_LEFT, false);
		m_pRightList = BindNxDataList2Ctrl(IDC_PAT_SUM_CONFIG_RIGHT, false);

		LoadConfigValues();
		UpdateButtons();

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

CString CPatientSummaryConfigDlg::GetCustomTitle(long nField)
{
	CString strTitle;
	if (m_pmapCustomFields->Lookup(nField, strTitle)) {
		return strTitle;
	}
	else {
		ASSERT(FALSE);
		return "";
	}
}

void CPatientSummaryConfigDlg::SetConfigInfo(CString strBase, CString strName)
{
	CPatSummaryConfigType cfgType;

	cfgType.m_strConfigName = strBase;
	cfgType.m_bCheck = GetRemotePropertyInt(strBase + "Check", 0, 0, GetCurrentUserName(), true);
	cfgType.m_nList = GetRemotePropertyInt(strBase + "List", 1, 0, GetCurrentUserName(), true);
	cfgType.m_nSortOrder = GetRemotePropertyInt(strBase + "Sort", 1, 0, GetCurrentUserName(), true);
	
	NXDATALIST2Lib::_DNxDataListPtr pList;
	if (cfgType.m_nList == 1) {
		pList = m_pLeftList;
	}
	else {
		pList = m_pRightList;
	}

	NXDATALIST2Lib::IRowSettingsPtr pRow = pList->GetNewRow();
	if (pRow) {
		pRow->PutValue(clcName, variant_t(strName));
		pRow->PutValue(clcBase, variant_t(strBase));
		pRow->PutValue(clcSort, cfgType.m_nSortOrder);
		if (cfgType.m_bCheck) {
			pRow->PutValue(clcShow, g_cvarTrue);
		}
		else {
			pRow->PutValue(clcShow, g_cvarFalse);
		}
		pList->AddRowSorted(pRow, NULL);
	}
}

void CPatientSummaryConfigDlg::LoadConfigValues() 
{
	SetConfigInfo("PatSumPatInfo", "Patient Information");
	SetConfigInfo("PatSumEmergencyContactInfo", "Emergency Contact Information");
	SetConfigInfo("PatSumCustom1", GetCustomTitle(1));
	SetConfigInfo("PatSumCustom2", GetCustomTitle(2));
	SetConfigInfo("PatSumCustom3", GetCustomTitle(3));
	SetConfigInfo("PatSumCustom4", GetCustomTitle(4));
	SetConfigInfo("PatSumPatNotes", "Patient Notes");
	SetConfigInfo("PatSumMainPhysician", "Main Physician");
	SetConfigInfo("PatSumPatCoord", "Patient Coordinator");
	SetConfigInfo("PatSumFirstContactDate", "First Contact Date");
	SetConfigInfo("PatSumEnteredBy", "Entered By");
	// (j.jones 2012-02-01 17:16) - PLID 46145 - added global periods
	SetConfigInfo("PatSumGlobalPeriods", "Global Periods");
	SetConfigInfo("PatSumEmploymentInfo", "Employment Infomation");
	SetConfigInfo("PatSumRace", "Race");
	SetConfigInfo("PatSumEthnicity", "Ethnicity");
	SetConfigInfo("PatSumLanguage", "Language");
	SetConfigInfo("PatSumDiagCodes", "Diagnosis Codes");
	SetConfigInfo("PatSumCurIllDate", "Current Illness Date");
	SetConfigInfo("PatSumPatType", "Patient Type");
	SetConfigInfo("PatSumLocation", "Location");
	SetConfigInfo("PatSumRefInfo", "Referring Information");
	SetConfigInfo("PatSumWarning", "Patient Warning");
	SetConfigInfo("PatSumLastModified", "Last Modified Date");
	SetConfigInfo("PatSumCustomText1", GetCustomTitle(11));
	SetConfigInfo("PatSumCustomText2", GetCustomTitle(12));
	SetConfigInfo("PatSumCustomText3", GetCustomTitle(13));
	SetConfigInfo("PatSumCustomText4", GetCustomTitle(14));
	SetConfigInfo("PatSumCustomText5", GetCustomTitle(15));
	SetConfigInfo("PatSumCustomText6", GetCustomTitle(16));
	SetConfigInfo("PatSumCustomText7", GetCustomTitle(90));
	SetConfigInfo("PatSumCustomText8", GetCustomTitle(91));
	SetConfigInfo("PatSumCustomText9", GetCustomTitle(92));
	SetConfigInfo("PatSumCustomText10", GetCustomTitle(93));
	SetConfigInfo("PatSumCustomText11", GetCustomTitle(94));
	SetConfigInfo("PatSumCustomText12", GetCustomTitle(95));
	SetConfigInfo("PatSumCustomNote", GetCustomTitle(17));
	SetConfigInfo("PatSumCustomList1", GetCustomTitle(21));
	SetConfigInfo("PatSumCustomList2", GetCustomTitle(22));
	SetConfigInfo("PatSumCustomList3", GetCustomTitle(23));
	SetConfigInfo("PatSumCustomList4", GetCustomTitle(24));
	SetConfigInfo("PatSumCustomList5", GetCustomTitle(25));
	SetConfigInfo("PatSumCustomList6", GetCustomTitle(26));
	SetConfigInfo("PatSumCustomContact", GetCustomTitle(31));
	SetConfigInfo("PatSumCustomCheck1", GetCustomTitle(41));
	SetConfigInfo("PatSumCustomCheck2", GetCustomTitle(42));
	SetConfigInfo("PatSumCustomCheck3", GetCustomTitle(43));
	SetConfigInfo("PatSumCustomCheck4", GetCustomTitle(44));
	SetConfigInfo("PatSumCustomCheck5", GetCustomTitle(45));
	SetConfigInfo("PatSumCustomCheck6", GetCustomTitle(46));
	SetConfigInfo("PatSumCustomDate1", GetCustomTitle(51));
	SetConfigInfo("PatSumCustomDate2", GetCustomTitle(52));
	SetConfigInfo("PatSumCustomDate3", GetCustomTitle(53));
	SetConfigInfo("PatSumCustomDate4", GetCustomTitle(54));
	SetConfigInfo("PatSumInsPartyInfo", "Insurance Information");
	SetConfigInfo("PatSumCurMeds", "Current Medications");
	SetConfigInfo("PatSumAllergies", "Allergies");
	SetConfigInfo("PatSumLastApptInfo", "Last Appointment Information");
	SetConfigInfo("PatSumLastDiagsBilled", "Last Diagnosis Codes Billed");
	SetConfigInfo("PatSumPatBal", "Patient Balance");
	SetConfigInfo("PatSumInsBal", "Insurance Balance");

	//now that we have everything loaded, make sure to fix the sorts
	FixSorts();

}

void CPatientSummaryConfigDlg::FixSorts(){

	//loop through each list and put the sort one after the other
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pLeftList->GetFirstRow();
	long nSort = 1;
	while (pRow) {
		pRow->PutValue(clcSort, (long)nSort);
		nSort++;
		pRow = pRow->GetNextRow();
	}

	pRow = m_pRightList->GetFirstRow();
	nSort = 1;
	while (pRow) {
		pRow->PutValue(clcSort, (long)nSort);
		nSort++;
		pRow = pRow->GetNextRow();
	}
}

void CPatientSummaryConfigDlg::MoveRowUp(NXDATALIST2Lib::_DNxDataListPtr pList)
{
	//get the current selection from the list
	NXDATALIST2Lib::IRowSettingsPtr pRow = pList->CurSel;
	if (pRow) {

		//we want to move this one up and the one below it down
		NXDATALIST2Lib::IRowSettingsPtr pRowAbove = pRow->GetPreviousRow();

		if (pRowAbove) {

			long nCurrentSort = VarLong(pRow->GetValue(clcSort));
			long nPrevSort = VarLong(pRowAbove->GetValue(clcSort));

			pRow->PutValue(clcSort, nPrevSort);
			pRowAbove->PutValue(clcSort, nCurrentSort);
			
			pList->Sort();
			pList->EnsureRowInView(pRow);

			FixSorts();
		}
	}
}

void CPatientSummaryConfigDlg::MoveRowDown(NXDATALIST2Lib::_DNxDataListPtr pList)
{
	//get the current selection from the list
	NXDATALIST2Lib::IRowSettingsPtr pRow = pList->CurSel;
	if (pRow) {

		//we want to move this one up and the one below it down
		NXDATALIST2Lib::IRowSettingsPtr pRowBelow = pRow->GetNextRow();

		if (pRowBelow) {

			long nCurrentSort = VarLong(pRow->GetValue(clcSort));
			long nAfterSort = VarLong(pRowBelow->GetValue(clcSort));

			pRow->PutValue(clcSort, nAfterSort);
			pRowBelow->PutValue(clcSort, nCurrentSort);			

			pList->Sort();
			pList->EnsureRowInView(pRow);

			FixSorts();
		}
	}
}
void CPatientSummaryConfigDlg::OnBnClickedOk()
{
	try {

		FixSorts();

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pLeftList->GetFirstRow();
		while (pRow) {

			CString strBase = VarString(pRow->GetValue(clcBase), "");
			BOOL bCheck = VarBool(pRow->GetValue(clcShow), g_cvarFalse);
			long nSort = VarLong(pRow->GetValue(clcSort), 0);

			SetRemotePropertyInt(strBase + "Check", bCheck ? 1 : 0, 0, GetCurrentUserName());
			SetRemotePropertyInt(strBase + "List", 1, 0, GetCurrentUserName());
			SetRemotePropertyInt(strBase + "Sort", nSort, 0, GetCurrentUserName());

			pRow = pRow->GetNextRow();
		}

		pRow = m_pRightList->GetFirstRow();
		while (pRow) {

			CString strBase = VarString(pRow->GetValue(clcBase), "");
			BOOL bCheck = VarBool(pRow->GetValue(clcShow), g_cvarFalse);
			long nSort = VarLong(pRow->GetValue(clcSort), 0);

			SetRemotePropertyInt(strBase + "Check", bCheck ? 1 : 0, 0, GetCurrentUserName());
			SetRemotePropertyInt(strBase + "List", 2, 0, GetCurrentUserName());
			SetRemotePropertyInt(strBase + "Sort", nSort, 0, GetCurrentUserName());

			pRow = pRow->GetNextRow();
		}

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}
BEGIN_EVENTSINK_MAP(CPatientSummaryConfigDlg, CNxDialog)
	ON_EVENT(CPatientSummaryConfigDlg, IDC_PAT_SUM_CONFIG_LEFT, 2, CPatientSummaryConfigDlg::SelChangedPatSumConfigLeft, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CPatientSummaryConfigDlg, IDC_PAT_SUM_CONFIG_RIGHT, 2, CPatientSummaryConfigDlg::SelChangedPatSumConfigRight, VTS_DISPATCH VTS_DISPATCH)
END_EVENTSINK_MAP()

void CPatientSummaryConfigDlg::SelChangedPatSumConfigLeft(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		UpdateButtons();
	}NxCatchAll(__FUNCTION__);
}

void CPatientSummaryConfigDlg::SelChangedPatSumConfigRight(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		UpdateButtons();
	}NxCatchAll(__FUNCTION__);
}
