// CCHITReportsConfigOptionsDlg.cpp : implementation file
// (d.thompson 2010-01-20) - PLID 36927
//

#include "stdafx.h"
#include "Practice.h"
#include "CCHITReportsConfigOptionsDlg.h"
#include "CCHITReportInfo.h" // (r.gonet 06/12/2013) - PLID 55151
using namespace NXDATALIST2Lib;
using namespace ADODB;

enum eItemColumns {
	eicID = 0,
	eicMasterID,
	eicName,
	eicType,
	eicAutoAlpha,
	eicFlipTable,
};

enum eDataColumns {
	edcID = 0,
	edcData,
	edcEMRDataGroupID,
	edcSortOrder,
	edcListType,
	edcListSortType,
};

// (j.gruber 2010-09-21 13:52) - PLID 40617 - category list columns
enum CatListColumns {
	clcID,
	clcName,
};

// CCCHITReportsConfigOptionsDlg dialog
IMPLEMENT_DYNAMIC(CCCHITReportsConfigOptionsDlg, CNxDialog)

CCCHITReportsConfigOptionsDlg::CCCHITReportsConfigOptionsDlg(CWnd* pParent, CString strReportName, CCHITReportConfigType crctType)
	: CNxDialog(CCCHITReportsConfigOptionsDlg::IDD, pParent)
{
	m_strReportName = strReportName;
	m_crctType = crctType;
}

CCCHITReportsConfigOptionsDlg::~CCCHITReportsConfigOptionsDlg()
{
}

void CCCHITReportsConfigOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_STATIC_BOTTOM_LABEL, m_stBottomLabel);
}


BEGIN_MESSAGE_MAP(CCCHITReportsConfigOptionsDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CCCHITReportsConfigOptionsDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CCCHITReportsConfigOptionsDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CCCHITReportsConfigOptionsDlg message handlers
BOOL CCCHITReportsConfigOptionsDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		//Setup controls
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_pItemList = BindNxDataList2Ctrl(IDC_CCHIT_EMR_ITEM_LIST, false);
		// (j.gruber 2010-09-14 09:22) - PLID 40514 - added a second item list
		m_pItemList2 = BindNxDataList2Ctrl(IDC_CCHIT_EMR_ITEM_LIST2, false);
		m_pDataList = BindNxDataList2Ctrl(IDC_CCHIT_EMR_DATA_LIST, false);
		// (j.gruber 2010-09-21 13:53) - PLID 40617 - category list
		m_pCatList = BindNxDataList2Ctrl(IDC_CCHIT_CAT_LIST, true);

		
		CString strItemListWhereClause;
		// (j.jones 2014-02-05 10:04) - PLID 60530 - if sliders are allowed, change the where clause
		if(m_crctType == crctEMRDataGroupOrSlider) {
			strItemListWhereClause.Format("Inactive = 0 AND EmrInfoT.ID > 0 AND DataType IN (%li, %li, %li, %li) AND DataSubType <> %li",
				eitSingleList, eitMultiList, eitSlider, eitTable, eistGenericTable);
		}
		else {
			strItemListWhereClause.Format("Inactive = 0 AND EmrInfoT.ID > 0 AND DataType IN (%li, %li, %li) AND DataSubType <> %li",
				eitSingleList, eitMultiList, eitTable, eistGenericTable);
		}
		m_pItemList->PutWhereClause((LPCTSTR)strItemListWhereClause);
		m_pItemList2->PutWhereClause((LPCTSTR)strItemListWhereClause);

		m_pItemList->Requery();
		// (j.jones 2014-02-05 10:07) - PLID 60530 - don't requery the second item list unless we need it
		if(m_crctType == crctEMRMultiItems) {
			m_pItemList2->Requery();
		}

		switch (m_crctType) {
			
			// (j.gruber 2010-09-10 15:00) - PLID 40487 - created types
			case crctEMRDataGroup: 
				{
					//Attempt to load the property.
					_RecordsetPtr prs = CreateParamRecordset("SELECT EMRInfoT.ID, ConfigRT.IntParam FROM ConfigRT "
						"INNER JOIN EMRDataT ON ConfigRT.IntParam = EMRDataT.EMRDataGroupID "
						"INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID "
						"INNER JOIN EMRInfoMasterT ON EmrInfoT.ID = EmrInfoMasterT.ActiveEmrInfoID "
						"WHERE ConfigRT.Name = {STRING}", FormatString("CCHITReportInfo_%s", m_strReportName));
					if(!prs->eof) {
						_variant_t varInfoID = prs->Fields->Item["ID"]->Value;
						_variant_t varDataGroupID = prs->Fields->Item["IntParam"]->Value;

						IRowSettingsPtr pItemRow = m_pItemList->SetSelByColumn(eicID, varInfoID);
						if(pItemRow != NULL) {
							//Load the data element list
							LoadDataElements();

							//Try to pick our group ID
							IRowSettingsPtr pFoundRow = m_pDataList->SetSelByColumn(edcEMRDataGroupID, varDataGroupID);
							//And make sure it's visible
							if(pFoundRow != NULL) {
								m_pDataList->EnsureRowInView(pFoundRow);
							}
							

							//It's irrelevant to us on loading if that succeeds or not.  If it doesn't, they'll have to repick.
						}
						else {
							//No longer exists, maybe it was inactivated or something?  Oh well, they'll have to repick.
						}
					}
				}
				break;

			case crctEMRItem:
				{
					//first, hide the bottom list
					HideBottomBox();

					//now select it
					_RecordsetPtr prs = CreateParamRecordset("SELECT EMRInfoT.ID "
						" FROM ConfigRT "
						"INNER JOIN EMRInfoMasterT ON ConfigRT.IntParam = EMRInfoMasterT.ID "
						"INNER JOIN EMRInfoT ON EMRInfoMasterT.ActiveEMRInfoID = EMRInfoT.ID "						
						"WHERE ConfigRT.Name = {STRING}", FormatString("CCHITReportInfo_%s", m_strReportName));
					if(!prs->eof) {
						_variant_t varInfoID = prs->Fields->Item["ID"]->Value;				
						m_pItemList->SetSelByColumn(eicID, varInfoID);						
					}
				}
			break;

			case crctEMRMultiItems:
				{
					//first, show the bottom item list
					ShowSecondItemList();

					//now select the first box
					_RecordsetPtr prs = CreateParamRecordset("SELECT EMRInfoT.ID "
						" FROM ConfigRT "
						"INNER JOIN EMRInfoMasterT ON ConfigRT.IntParam = EMRInfoMasterT.ID "
						"INNER JOIN EMRInfoT ON EMRInfoMasterT.ActiveEMRInfoID = EMRInfoT.ID "						
						"WHERE ConfigRT.Name = {STRING};"

						"SELECT EMRInfoT.ID "
						" FROM ConfigRT "
						"INNER JOIN EMRInfoMasterT ON ConfigRT.IntParam = EMRInfoMasterT.ID "
						"INNER JOIN EMRInfoT ON EMRInfoMasterT.ActiveEMRInfoID = EMRInfoT.ID "						
						"WHERE ConfigRT.Name = {STRING};"						
						
						, FormatString("CCHITReportInfo_%s", m_strReportName), FormatString("CCHITReportInfo_%s2", m_strReportName));
					if(!prs->eof) {
						_variant_t varInfoID = prs->Fields->Item["ID"]->Value;				
						m_pItemList->SetSelByColumn(eicID, varInfoID);						
					}

					prs = prs->NextRecordset(0);

					//now select the second box					
					if(!prs->eof) {
						_variant_t varInfoID = prs->Fields->Item["ID"]->Value;				
						m_pItemList2->SetSelByColumn(eicID, varInfoID);						
					}
				}
			break;

			case crctMailsentCat:
				{
					//first, hide show the category list
					ShowCatList();

					//now select it
					_RecordsetPtr prs = CreateParamRecordset("SELECT NoteCatsF.ID "
						" FROM ConfigRT "
						"INNER JOIN NoteCatsF ON ConfigRT.IntParam = NoteCatsF.ID "						
						"WHERE ConfigRT.Name = {STRING}", FormatString("CCHITReportInfo_%s", m_strReportName));
					if(!prs->eof) {
						_variant_t varInfoID = prs->Fields->Item["ID"]->Value;				
						m_pCatList->SetSelByColumn(clcID, varInfoID);						
					}
				}		
				break;

			// (j.jones 2014-02-05 09:49) - PLID 60530 - vitals signs allow sliders in addition to emr data groups
			case crctEMRDataGroupOrSlider: {
					//Attempt to load the property.
					_RecordsetPtr prs = CreateParamRecordset("SELECT EMRInfoT.ID, ConfigRT.IntParam FROM ConfigRT "
						"INNER JOIN EMRDataT ON ConfigRT.IntParam = EMRDataT.EMRDataGroupID "
						"INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID "
						"INNER JOIN EMRInfoMasterT ON EmrInfoT.ID = EmrInfoMasterT.ActiveEmrInfoID "
						"WHERE ConfigRT.Name = {STRING}; \r\n"
						""
						"SELECT EMRInfoT.ID "
						"FROM ConfigRT "
						"INNER JOIN EMRInfoMasterT ON ConfigRT.IntParam = EMRInfoMasterT.ID "
						"INNER JOIN EMRInfoT ON EMRInfoMasterT.ActiveEMRInfoID = EMRInfoT.ID "						
						"WHERE EMRInfoT.DataType = {CONST_INT} AND ConfigRT.Name = {STRING};",
						FormatString("CCHITReportInfo_%s", m_strReportName),
						eitSlider, FormatString("CCHITReportInfo_%s2", m_strReportName));
					
					bool bFirstRecordsetIsEmpty = true;
					if(!prs->eof) {
						bFirstRecordsetIsEmpty = false;
						_variant_t varInfoID = prs->Fields->Item["ID"]->Value;
						_variant_t varDataGroupID = prs->Fields->Item["IntParam"]->Value;

						IRowSettingsPtr pItemRow = m_pItemList->SetSelByColumn(eicID, varInfoID);
						if(pItemRow != NULL) {
							//Load the data element list
							LoadDataElements();

							//Try to pick our group ID
							IRowSettingsPtr pFoundRow = m_pDataList->SetSelByColumn(edcEMRDataGroupID, varDataGroupID);
							//And make sure it's visible
							if(pFoundRow != NULL) {
								m_pDataList->EnsureRowInView(pFoundRow);
							}
							

							//It's irrelevant to us on loading if that succeeds or not.  If it doesn't, they'll have to repick.
						}
					}

					prs = prs->NextRecordset(0);

					//now select the info items
					if(!prs->eof) {

						//it should not be possible for two items to be selected
						ASSERT(bFirstRecordsetIsEmpty);

						_variant_t varInfoID = prs->Fields->Item["ID"]->Value;				
						m_pItemList->SetSelByColumn(eicID, varInfoID);
						
						//LoadDataElements should disable the bottom datalist for us
						LoadDataElements();
					}
				}
				break;
				
		}

	} NxCatchAll("Error in OnInitDialog");

	return TRUE;
}

// (j.gruber 2010-09-14 09:29) - PLID 40514 - Show the second item box
void CCCHITReportsConfigOptionsDlg::ShowSecondItemList() {

	GetDlgItem(IDC_CCHIT_EMR_ITEM_LIST2)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_CCHIT_EMR_DATA_LIST)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_STATIC_BOTTOM_LABEL)->ShowWindow(SW_SHOW);
	m_stBottomLabel.SetWindowTextA("Please select an EMR Item to fulfill the secondary report criteria.");
}

// (j.gruber 2010-09-10 15:00) - PLID 40487 - created for
void CCCHITReportsConfigOptionsDlg::HideBottomBox() {

	CRect rcTopBox, rcBottomBox;

	GetDlgItem(IDC_STATIC_BOTTOM_LABEL)->ShowWindow(SW_HIDE);
					
	GetDlgItem(IDC_CCHIT_EMR_ITEM_LIST)->GetWindowRect(rcTopBox);
	ScreenToClient(rcTopBox);

	GetDlgItem(IDC_CCHIT_EMR_DATA_LIST)->GetWindowRect(rcBottomBox);
	ScreenToClient(rcBottomBox);

	//set the bottom of the top box = to the bottom of the bottom box
	rcTopBox.bottom = rcBottomBox.bottom;

	//now move it
	GetDlgItem(IDC_CCHIT_EMR_ITEM_LIST)->MoveWindow(rcTopBox);

	GetControlPositions();
}

// (j.gruber 2010-09-21 13:56) - PLID 40617 - show the category list
void CCCHITReportsConfigOptionsDlg::ShowCatList() {

	CRect rcTopBox, rcBottomBox;

	SetDlgItemText(IDC_STATIC, "Please choose a category to fulfill the report criteria.");
	GetDlgItem(IDC_STATIC_BOTTOM_LABEL)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_CCHIT_EMR_ITEM_LIST)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_CCHIT_CAT_LIST)->ShowWindow(SW_SHOW);
					
	GetDlgItem(IDC_CCHIT_CAT_LIST)->GetWindowRect(rcTopBox);
	ScreenToClient(rcTopBox);

	GetDlgItem(IDC_CCHIT_EMR_DATA_LIST)->GetWindowRect(rcBottomBox);
	ScreenToClient(rcBottomBox);

	//set the bottom of the top box = to the bottom of the bottom box
	rcTopBox.bottom = rcBottomBox.bottom;

	GetDlgItem(IDC_CCHIT_EMR_DATA_LIST)->ShowWindow(SW_HIDE);

	//now move it
	GetDlgItem(IDC_CCHIT_CAT_LIST)->MoveWindow(rcTopBox);

	GetControlPositions();
}

void CCCHITReportsConfigOptionsDlg::OnBnClickedOk()
{
	try {
		// (j.gruber 2010-09-10 16:02) - PLID 40487 - check the status
		// (j.gruber 2010-09-14 09:31) - PLID 40514 - 2 save values
		long nSaveValue = -1;
		long nSaveValue2 = -1;
		switch (m_crctType) {
			case crctEMRDataGroup:
				{
					IRowSettingsPtr pRow = m_pDataList->CurSel;
					if(pRow == NULL) {
						AfxMessageBox("You must select a data element before pressing OK.");
						return;
					}

					//If we have one, save the EMRDataGroupID
					nSaveValue = VarLong(pRow->GetValue(edcEMRDataGroupID));
				}
			break;

			case crctEMRItem:
				{
					IRowSettingsPtr pRow = m_pItemList->CurSel;
					if(pRow == NULL) {
						AfxMessageBox("You must select a EMN item before pressing OK.");
						return;
					}

					//If we have one, save the EMRDataGroupID
					nSaveValue = VarLong(pRow->GetValue(eicMasterID));
				}
			break;

			case crctEMRMultiItems:
				{
					IRowSettingsPtr pRow = m_pItemList->CurSel;
					IRowSettingsPtr pRow2 = m_pItemList2->CurSel;
					
					if(pRow == NULL || pRow2 == NULL) {
						AfxMessageBox("You must select a EMN item from both boxes before pressing OK.");
						return;
					}

					//If we have one, save the EMRDataGroupID
					nSaveValue = VarLong(pRow->GetValue(eicMasterID));
					nSaveValue2 = VarLong(pRow2->GetValue(eicMasterID));
				}
			break;

			// (j.gruber 2010-09-21 13:57) - PLID 40617
			case crctMailsentCat:
				{
					IRowSettingsPtr pRow = m_pCatList->CurSel;
					if(pRow == NULL) {
						AfxMessageBox("You must select a category before pressing OK.");
						return;
					}

					//If we have one, save the EMRDataGroupID
					nSaveValue = VarLong(pRow->GetValue(clcID));
				}
			break;

			// (j.jones 2014-02-05 09:49) - PLID 60530 - vitals signs allow sliders in addition to emr data groups
			case crctEMRDataGroupOrSlider: {
					IRowSettingsPtr pItemRow = m_pItemList->CurSel;
					if(pItemRow == NULL) {
						AfxMessageBox("You must select an EMR item before pressing OK.");
						return;
					}

					short nDataType = VarShort(pItemRow->GetValue(eicType));
					if(nDataType == eitSlider) {
						//save the MasterInfoID
						nSaveValue2 = VarLong(pItemRow->GetValue(eicMasterID));
						nSaveValue = -1;
					}
					else {
						IRowSettingsPtr pDataRow = m_pDataList->CurSel;
						if(pDataRow == NULL) {
							AfxMessageBox("You must select a data element before pressing OK.");
							return;
						}

						//If we have one, save the EMRDataGroupID
						nSaveValue = VarLong(pDataRow->GetValue(edcEMRDataGroupID));
						nSaveValue2 = -1;
					}			
				}
			break;
		}

		// (j.jones 2014-02-05 09:49) - PLID 60530 - vitals signs allow sliders in addition to emr data groups
		if(m_crctType == crctEMRDataGroupOrSlider &&
			((nSaveValue != -1 && nSaveValue2 != -1) || (nSaveValue == -1 && nSaveValue2 == -1))) {

			//this should not be possible, these should be mutually exclusive
			ASSERT(FALSE);
		}

		SetRemotePropertyInt("CCHITReportInfo_" + m_strReportName, nSaveValue, 0, "<None>");
		
		if (m_crctType == crctEMRMultiItems || m_crctType == crctEMRDataGroupOrSlider) {
			SetRemotePropertyInt("CCHITReportInfo_" + m_strReportName + "2", nSaveValue2, 0, "<None>");
		}

		CDialog::OnOK();


	} NxCatchAll(__FUNCTION__);
}

void CCCHITReportsConfigOptionsDlg::OnBnClickedCancel()
{
	CDialog::OnCancel();
}
BEGIN_EVENTSINK_MAP(CCCHITReportsConfigOptionsDlg, CNxDialog)
	ON_EVENT(CCCHITReportsConfigOptionsDlg, IDC_CCHIT_EMR_ITEM_LIST, 2, CCCHITReportsConfigOptionsDlg::SelChangedCchitEmrItemList, VTS_DISPATCH VTS_DISPATCH)
END_EVENTSINK_MAP()

void CCCHITReportsConfigOptionsDlg::SelChangedCchitEmrItemList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		LoadDataElements();

	} NxCatchAll(__FUNCTION__);
}

void CCCHITReportsConfigOptionsDlg::LoadDataElements()
{
	IRowSettingsPtr pItemRow = m_pItemList->CurSel;

	//First control enabled status
	// (j.jones 2014-02-05 09:49) - PLID 60530 - if a slider, don't load the bottom datalist
	if(pItemRow == NULL || VarShort(pItemRow->GetValue(eicType)) == eitSlider) {
		GetDlgItem(IDC_STATIC_BOTTOM_LABEL)->EnableWindow(FALSE);
		m_pDataList->Clear();
		m_pDataList->PutEnabled(VARIANT_FALSE);
		//And just quit, we have nothing more to do.
		return;
	}
	else {
		GetDlgItem(IDC_STATIC_BOTTOM_LABEL)->EnableWindow(TRUE);
		m_pDataList->PutEnabled(VARIANT_TRUE);
	}

	//Now load the data element list appropriately
	long nInfoID = VarLong(pItemRow->GetValue(eicID));
	CString strWhere;
	strWhere.Format("EMRDataT.Inactive = 0 AND EMRDataT.IsLabel = 0 AND EMRDataT.EMRInfoID = %li", nInfoID);
	m_pDataList->WhereClause = _bstr_t(strWhere);
	m_pDataList->Requery();

	//When we're showing a table type, we have a ListType column that states if it's a Row or Column
	short nDataType = VarShort(pItemRow->GetValue(eicType));
	IColumnSettingsPtr pCol = m_pDataList->GetColumn(edcListType);
	if(nDataType == 7) {
		//Tables, show it
		pCol->PutColumnStyle(csVisible|csWidthPercent);
		pCol->PutStoredWidth(33);

		//While we're here, also set the combo text in case of table flipping
		IColumnSettingsPtr pColListType = m_pDataList->GetColumn(edcListType);
		if(VarBool(pItemRow->GetValue(eicFlipTable))) {
			//It is flipped.  Reverse our descriptors.
			pColListType->ComboSource = _bstr_t("SELECT 1, 'List' UNION SELECT 2, 'Column' UNION SELECT 3, 'Row - Text' UNION SELECT 4, 'Row - Dropdown' UNION SELECT 5, 'Row - Checkbox' UNION SELECT 6, 'Row - Linked'");
		}
		else {
			//Not flipped, as normal
			pColListType->ComboSource = _bstr_t("SELECT 1, 'List' UNION SELECT 2, 'Row' UNION SELECT 3, 'Column - Text' UNION SELECT 4, 'Column - Dropdown' UNION SELECT 5, 'Column - Checkbox' UNION SELECT 6, 'Column - Linked'");
		}
	}
	else {
		//List, don't show it
		pCol->PutColumnStyle(csVisible|csFixedWidth);
		pCol->PutStoredWidth(0);
	}

	//Fix the sort order appropriately.  In all cases 'edcListSortType' will be first.  This is the type -- either
	//	1 (list item), 2 (row), or 3 (column).
	//Otherwise, if AutoAlpha is turned on for the item, sort by edcName second.  If it's not, sort by edcSortOrder
	IColumnSettingsPtr pColName = m_pDataList->GetColumn(edcData);
	IColumnSettingsPtr pColSort = m_pDataList->GetColumn(edcSortOrder);
	if(VarBool(pItemRow->GetValue(eicAutoAlpha))) {
		pColName->SortPriority = 1;
		pColSort->SortPriority = 2;
	}
	else {
		pColName->SortPriority = 2;
		pColSort->SortPriority = 1;
	}
}
