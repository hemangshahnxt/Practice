// EligibilityResponseFilteringConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EligibilityResponseFilteringConfigDlg.h"

// CEligibilityResponseFilteringConfigDlg dialog

// (j.jones 2010-03-25 17:40) - PLID 37905 - created

using namespace NXDATALIST2Lib;
using namespace ADODB;

//all 6 lists are identical
enum ListColumns {

	lcID = 0,
	lcDescription,
	lcFilterExcluded,
};

IMPLEMENT_DYNAMIC(CEligibilityResponseFilteringConfigDlg, CNxDialog)

CEligibilityResponseFilteringConfigDlg::CEligibilityResponseFilteringConfigDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEligibilityResponseFilteringConfigDlg::IDD, pParent)
{

}

CEligibilityResponseFilteringConfigDlg::~CEligibilityResponseFilteringConfigDlg()
{

}

void CEligibilityResponseFilteringConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BTN_SELECT_ONE_TOS, m_btnSelectOneServiceType);
	DDX_Control(pDX, IDC_BTN_SELECT_ALL_TOS, m_btnSelectAllServiceType);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ONE_TOS, m_btnUnselectOneServiceType);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ALL_TOS, m_btnUnselectAllServiceType);
	DDX_Control(pDX, IDC_BTN_SELECT_ONE_COVERAGE, m_btnSelectOneCoverageLevel);
	DDX_Control(pDX, IDC_BTN_SELECT_ALL_COVERAGE, m_btnSelectAllCoverageLevel);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ONE_COVERAGE, m_btnUnselectOneCoverageLevel);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ALL_COVERAGE, m_btnUnselectAllCoverageLevel);
	DDX_Control(pDX, IDC_BTN_SELECT_ONE_BENEFIT, m_btnSelectOneBenefitType);
	DDX_Control(pDX, IDC_BTN_SELECT_ALL_BENEFIT, m_btnSelectAllBenefitTypes);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ONE_BENEFIT, m_btnUnselectOneBenefitType);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ALL_BENEFIT, m_btnUnselectAllBenefitType);
	DDX_Control(pDX, IDC_CHECK_EXCLUDE_FROM_OUTPUT_FILE, m_checkExcludeFromOutputFile);
}


BEGIN_MESSAGE_MAP(CEligibilityResponseFilteringConfigDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_BTN_SELECT_ONE_TOS, OnBtnSelectOneTos)
	ON_BN_CLICKED(IDC_BTN_SELECT_ALL_TOS, OnBtnSelectAllTos)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ONE_TOS, OnBtnUnselectOneTos)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ALL_TOS, OnBtnUnselectAllTos)
	ON_BN_CLICKED(IDC_BTN_SELECT_ONE_COVERAGE, OnBtnSelectOneCoverage)
	ON_BN_CLICKED(IDC_BTN_SELECT_ALL_COVERAGE, OnBtnSelectAllCoverage)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ONE_COVERAGE, OnBtnUnselectOneCoverage)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ALL_COVERAGE, OnBtnUnselectAllCoverage)
	ON_BN_CLICKED(IDC_BTN_SELECT_ONE_BENEFIT, OnBtnSelectOneBenefit)
	ON_BN_CLICKED(IDC_BTN_SELECT_ALL_BENEFIT, OnBtnSelectAllBenefit)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ONE_BENEFIT, OnBtnUnselectOneBenefit)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ALL_BENEFIT, OnBtnUnselectAllBenefit)	
END_MESSAGE_MAP()


// CEligibilityResponseFilteringConfigDlg message handlers

BOOL CEligibilityResponseFilteringConfigDlg::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();

		// (j.jones 2010-04-19 10:07) - PLID 38202 - added preference caching
		g_propManager.CachePropertiesInBulk("CEligibilityResponseFilteringConfigDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND Name IN ( \r\n"
			"	'Eligibility_ExcludeFromOutputFile' \r\n"
			")"
			, _Q(GetCurrentUserName()));	

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnSelectOneServiceType.AutoSet(NXB_DOWN);
		m_btnSelectAllServiceType.AutoSet(NXB_DDOWN);
		m_btnUnselectOneServiceType.AutoSet(NXB_UP);
		m_btnUnselectAllServiceType.AutoSet(NXB_UUP);
		m_btnSelectOneCoverageLevel.AutoSet(NXB_DOWN);
		m_btnSelectAllCoverageLevel.AutoSet(NXB_DDOWN);
		m_btnUnselectOneCoverageLevel.AutoSet(NXB_UP);
		m_btnUnselectAllCoverageLevel.AutoSet(NXB_UUP);
		m_btnSelectOneBenefitType.AutoSet(NXB_DOWN);
		m_btnSelectAllBenefitTypes.AutoSet(NXB_DDOWN);
		m_btnUnselectOneBenefitType.AutoSet(NXB_UP);
		m_btnUnselectAllBenefitType.AutoSet(NXB_UUP);

		m_UnselectedServiceTypeList = BindNxDataList2Ctrl(IDC_UNSELECTED_SERVICETYPE_LIST);
		m_SelectedServiceTypeList = BindNxDataList2Ctrl(IDC_SELECTED_SERVICETYPE_LIST);
		m_UnselectedCoverageLevelList = BindNxDataList2Ctrl(IDC_UNSELECTED_COVERAGELEVEL_LIST);
		m_SelectedCoverageLevelList = BindNxDataList2Ctrl(IDC_SELECTED_COVERAGELEVEL_LIST);
		m_UnselectedBenefitTypeList = BindNxDataList2Ctrl(IDC_UNSELECTED_BENEFITTYPE_LIST);
		m_SelectedBenefitTypeList = BindNxDataList2Ctrl(IDC_SELECTED_BENEFITTYPE_LIST);

		// (j.jones 2010-04-19 09:56) - PLID 38202 - added option to exclude from the output file
		m_checkExcludeFromOutputFile.SetCheck(GetRemotePropertyInt("Eligibility_ExcludeFromOutputFile", 0, 0, "<None>", true) == 1);

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CEligibilityResponseFilteringConfigDlg::OnOK()
{
	try {

		//disallow saving unless there is at least one object in each selected list
		if(m_SelectedServiceTypeList->GetRowCount() == 0
			|| m_SelectedCoverageLevelList->GetRowCount() == 0
			|| m_SelectedBenefitTypeList->GetRowCount() == 0) {

			AfxMessageBox("Each list of Included items must have at least one item selected in which to filter on.");
			return;
		}

		CString strSqlBatch;
		CNxParamSqlArray aryParams;

		CWaitCursor pWait;

		//something changed if the FilterExcluded column is mismatched

		{
			IRowSettingsPtr pRow = m_UnselectedServiceTypeList->GetFirstRow();
			while(pRow) {
				if(!VarBool(pRow->GetValue(lcFilterExcluded))) {
					//a non-excluded record is in the excluded list
					long nID = VarLong(pRow->GetValue(lcID));
					AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE EligibilityDataReferenceT SET FilterExcluded = {INT} "
						"WHERE ID = {INT}", 1, nID);
				}
				pRow = pRow->GetNextRow();
			}
		}

		{
			IRowSettingsPtr pRow = m_UnselectedCoverageLevelList->GetFirstRow();
			while(pRow) {
				if(!VarBool(pRow->GetValue(lcFilterExcluded))) {
					//a non-excluded record is in the excluded list
					long nID = VarLong(pRow->GetValue(lcID));
					AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE EligibilityDataReferenceT SET FilterExcluded = {INT} "
						"WHERE ID = {INT}", 1, nID);
				}
				pRow = pRow->GetNextRow();
			}
		}

		{
			IRowSettingsPtr pRow = m_UnselectedBenefitTypeList->GetFirstRow();
			while(pRow) {
				if(!VarBool(pRow->GetValue(lcFilterExcluded))) {
					//a non-excluded record is in the excluded list
					long nID = VarLong(pRow->GetValue(lcID));
					AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE EligibilityDataReferenceT SET FilterExcluded = {INT} "
						"WHERE ID = {INT}", 1, nID);
				}
				pRow = pRow->GetNextRow();
			}
		}

		{
			IRowSettingsPtr pRow = m_SelectedServiceTypeList->GetFirstRow();
			while(pRow) {
				if(VarBool(pRow->GetValue(lcFilterExcluded))) {
					//an excluded record is in the included list
					long nID = VarLong(pRow->GetValue(lcID));
					AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE EligibilityDataReferenceT SET FilterExcluded = {INT} "
						"WHERE ID = {INT}", 0, nID);
				}
				pRow = pRow->GetNextRow();
			}
		}

		{
			IRowSettingsPtr pRow = m_SelectedCoverageLevelList->GetFirstRow();
			while(pRow) {
				if(VarBool(pRow->GetValue(lcFilterExcluded))) {
					//an excluded record is in the included list
					long nID = VarLong(pRow->GetValue(lcID));
					AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE EligibilityDataReferenceT SET FilterExcluded = {INT} "
						"WHERE ID = {INT}", 0, nID);
				}
				pRow = pRow->GetNextRow();
			}
		}

		{
			IRowSettingsPtr pRow = m_SelectedBenefitTypeList->GetFirstRow();
			while(pRow) {
				if(VarBool(pRow->GetValue(lcFilterExcluded))) {
					//an excluded record is in the included list
					long nID = VarLong(pRow->GetValue(lcID));
					AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE EligibilityDataReferenceT SET FilterExcluded = {INT} "
						"WHERE ID = {INT}", 0, nID);
				}
				pRow = pRow->GetNextRow();
			}
		}

		if(!strSqlBatch.IsEmpty()) {
			ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);
		}

		// (j.jones 2010-04-19 09:56) - PLID 38202 - added option to exclude from the output file
		SetRemotePropertyInt("Eligibility_ExcludeFromOutputFile", m_checkExcludeFromOutputFile.GetCheck() ? 1 : 0, 0, "<None>");

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}

void CEligibilityResponseFilteringConfigDlg::OnCancel()
{
	try {

		BOOL bChanged = FALSE;

		//see if anything changed by seeing if the FilterExcluded column is mismatched,
		//warn if anything did change

		{
			IRowSettingsPtr pRow = m_UnselectedServiceTypeList->GetFirstRow();
			while(pRow != NULL && !bChanged) {
				if(!VarBool(pRow->GetValue(lcFilterExcluded))) {
					//a non-excluded record is in the excluded list
					bChanged = TRUE;
				}
				pRow = pRow->GetNextRow();
			}
		}

		if(!bChanged) {
			IRowSettingsPtr pRow = m_UnselectedCoverageLevelList->GetFirstRow();
			while(pRow != NULL && !bChanged) {
				if(!VarBool(pRow->GetValue(lcFilterExcluded))) {
					//a non-excluded record is in the excluded list
					bChanged = TRUE;
				}
				pRow = pRow->GetNextRow();
			}
		}

		if(!bChanged) {
			IRowSettingsPtr pRow = m_UnselectedBenefitTypeList->GetFirstRow();
			while(pRow != NULL && !bChanged) {
				if(!VarBool(pRow->GetValue(lcFilterExcluded))) {
					//a non-excluded record is in the excluded list
					bChanged = TRUE;
				}
				pRow = pRow->GetNextRow();
			}
		}

		if(!bChanged) {
			IRowSettingsPtr pRow = m_SelectedServiceTypeList->GetFirstRow();
			while(pRow != NULL && !bChanged) {
				if(VarBool(pRow->GetValue(lcFilterExcluded))) {
					//an excluded record is in the included list
					bChanged = TRUE;
				}
				pRow = pRow->GetNextRow();
			}
		}

		if(!bChanged) {
			IRowSettingsPtr pRow = m_SelectedCoverageLevelList->GetFirstRow();
			while(pRow != NULL && !bChanged) {
				if(VarBool(pRow->GetValue(lcFilterExcluded))) {
					//an excluded record is in the included list
					bChanged = TRUE;
				}
				pRow = pRow->GetNextRow();
			}
		}

		if(!bChanged) {
			IRowSettingsPtr pRow = m_SelectedBenefitTypeList->GetFirstRow();
			while(pRow != NULL && !bChanged) {
				if(VarBool(pRow->GetValue(lcFilterExcluded))) {
					//an excluded record is in the included list
					bChanged = TRUE;
				}
				pRow = pRow->GetNextRow();
			}
		}

		if(bChanged) {
			if(IDNO == MessageBox("You have made changes to your filters. These changes will be lost if you cancel.\n\n"
				"Are you sure you wish to cancel?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				return;
			}
		}

		CNxDialog::OnCancel();

	}NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CEligibilityResponseFilteringConfigDlg, CNxDialog)
	ON_EVENT(CEligibilityResponseFilteringConfigDlg, IDC_UNSELECTED_SERVICETYPE_LIST, 3, OnDblClickCellUnselectedServicetypeList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CEligibilityResponseFilteringConfigDlg, IDC_SELECTED_SERVICETYPE_LIST, 3, OnDblClickCellSelectedServicetypeList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CEligibilityResponseFilteringConfigDlg, IDC_UNSELECTED_COVERAGELEVEL_LIST, 3, OnDblClickCellUnselectedCoveragelevelList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CEligibilityResponseFilteringConfigDlg, IDC_SELECTED_COVERAGELEVEL_LIST, 3, OnDblClickCellSelectedCoveragelevelList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CEligibilityResponseFilteringConfigDlg, IDC_UNSELECTED_BENEFITTYPE_LIST, 3, OnDblClickCellUnselectedBenefittypeList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CEligibilityResponseFilteringConfigDlg, IDC_SELECTED_BENEFITTYPE_LIST, 3, OnDblClickCellSelectedBenefittypeList, VTS_DISPATCH VTS_I2)
END_EVENTSINK_MAP()

void CEligibilityResponseFilteringConfigDlg::OnDblClickCellUnselectedServicetypeList(LPDISPATCH lpRow, short nColIndex)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		m_UnselectedServiceTypeList->PutCurSel(pRow);
		OnBtnSelectOneTos();

	}NxCatchAll(__FUNCTION__);
}

void CEligibilityResponseFilteringConfigDlg::OnDblClickCellSelectedServicetypeList(LPDISPATCH lpRow, short nColIndex)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		m_SelectedServiceTypeList->PutCurSel(pRow);
		OnBtnUnselectOneTos();

	}NxCatchAll(__FUNCTION__);
}

void CEligibilityResponseFilteringConfigDlg::OnDblClickCellUnselectedCoveragelevelList(LPDISPATCH lpRow, short nColIndex)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		m_UnselectedCoverageLevelList->PutCurSel(pRow);
		OnBtnSelectOneCoverage();

	}NxCatchAll(__FUNCTION__);
}

void CEligibilityResponseFilteringConfigDlg::OnDblClickCellSelectedCoveragelevelList(LPDISPATCH lpRow, short nColIndex)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		m_SelectedCoverageLevelList->PutCurSel(pRow);
		OnBtnUnselectOneCoverage();

	}NxCatchAll(__FUNCTION__);
}

void CEligibilityResponseFilteringConfigDlg::OnDblClickCellUnselectedBenefittypeList(LPDISPATCH lpRow, short nColIndex)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		m_UnselectedBenefitTypeList->PutCurSel(pRow);
		OnBtnSelectOneBenefit();

	}NxCatchAll(__FUNCTION__);
}

void CEligibilityResponseFilteringConfigDlg::OnDblClickCellSelectedBenefittypeList(LPDISPATCH lpRow, short nColIndex)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		m_SelectedBenefitTypeList->PutCurSel(pRow);
		OnBtnUnselectOneBenefit();

	}NxCatchAll(__FUNCTION__);
}

void CEligibilityResponseFilteringConfigDlg::OnBtnSelectOneTos()
{
	try {

		m_SelectedServiceTypeList->TakeCurrentRowAddSorted(m_UnselectedServiceTypeList, NULL);

	}NxCatchAll(__FUNCTION__);
}

void CEligibilityResponseFilteringConfigDlg::OnBtnSelectAllTos()
{
	try {

		m_SelectedServiceTypeList->TakeAllRows(m_UnselectedServiceTypeList);

	}NxCatchAll(__FUNCTION__);
}

void CEligibilityResponseFilteringConfigDlg::OnBtnUnselectOneTos()
{
	try {

		m_UnselectedServiceTypeList->TakeCurrentRowAddSorted(m_SelectedServiceTypeList, NULL);

	}NxCatchAll(__FUNCTION__);
}

void CEligibilityResponseFilteringConfigDlg::OnBtnUnselectAllTos()
{
	try {

		m_UnselectedServiceTypeList->TakeAllRows(m_SelectedServiceTypeList);

	}NxCatchAll(__FUNCTION__);
}

void CEligibilityResponseFilteringConfigDlg::OnBtnSelectOneCoverage()
{
	try {

		m_SelectedCoverageLevelList->TakeCurrentRowAddSorted(m_UnselectedCoverageLevelList, NULL);

	}NxCatchAll(__FUNCTION__);
}

void CEligibilityResponseFilteringConfigDlg::OnBtnSelectAllCoverage()
{
	try {

		m_SelectedCoverageLevelList->TakeAllRows(m_UnselectedCoverageLevelList);

	}NxCatchAll(__FUNCTION__);
}

void CEligibilityResponseFilteringConfigDlg::OnBtnUnselectOneCoverage()
{
	try {

		m_UnselectedCoverageLevelList->TakeCurrentRowAddSorted(m_SelectedCoverageLevelList, NULL);

	}NxCatchAll(__FUNCTION__);
}

void CEligibilityResponseFilteringConfigDlg::OnBtnUnselectAllCoverage()
{
	try {

		m_UnselectedCoverageLevelList->TakeAllRows(m_SelectedCoverageLevelList);

	}NxCatchAll(__FUNCTION__);
}

void CEligibilityResponseFilteringConfigDlg::OnBtnSelectOneBenefit()
{
	try {

		m_SelectedBenefitTypeList->TakeCurrentRowAddSorted(m_UnselectedBenefitTypeList, NULL);

	}NxCatchAll(__FUNCTION__);
}

void CEligibilityResponseFilteringConfigDlg::OnBtnSelectAllBenefit()
{
	try {

		m_SelectedBenefitTypeList->TakeAllRows(m_UnselectedBenefitTypeList);

	}NxCatchAll(__FUNCTION__);
}

void CEligibilityResponseFilteringConfigDlg::OnBtnUnselectOneBenefit()
{
	try {

		m_UnselectedBenefitTypeList->TakeCurrentRowAddSorted(m_SelectedBenefitTypeList, NULL);

	}NxCatchAll(__FUNCTION__);
}

void CEligibilityResponseFilteringConfigDlg::OnBtnUnselectAllBenefit()
{
	try {

		m_UnselectedBenefitTypeList->TakeAllRows(m_SelectedBenefitTypeList);

	}NxCatchAll(__FUNCTION__);
}