// FramesOptionsDlg.cpp : implementation file
//

// (j.gruber 2010-06-24 09:56) - PLID 39314 - created
#include "stdafx.h"
#include "Practice.h"
#include "FramesOptionsDlg.h"
#include "FramesData.h"

enum CategoryListColumns {
	clcID = 0,
	clcName,
};

enum TrackStatus {
	tsNotTrackable = 0,
	tsTrackOrders,
	tsTrackQuantity,
};

// CFramesOptionsDlg dialog

IMPLEMENT_DYNAMIC(CFramesOptionsDlg, CNxDialog)

CFramesOptionsDlg::CFramesOptionsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CFramesOptionsDlg::IDD, pParent)
{

}

CFramesOptionsDlg::~CFramesOptionsDlg()
{
}

void CFramesOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FR_OP_RADIO_NOT_TRACKABLE_ITEM, m_radioNotTrackable);
	DDX_Control(pDX, IDC_FR_OP_RADIO_TRACK_ORDERS_ITEM, m_radioTrackOrders);
	DDX_Control(pDX, IDC_FR_OP_RADIO_TRACK_QUANTITY_ITEM, m_radioTrackQuantity);
	DDX_Control(pDX, IDC_FR_OP_TAXABLE2, m_taxable2);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_FR_OP_TAXABLE, m_taxable);
	DDX_Control(pDX, IDC_FR_OP_BILLABLE, m_billable);			
}


BEGIN_MESSAGE_MAP(CFramesOptionsDlg, CNxDialog)
	/*ON_BN_CLICKED(IDC_FR_OP_RADIO_NOT_TRACKABLE_ITEM, OnRadioNotTrackableItem)
	ON_BN_CLICKED(IDC_FR_OP_RADIO_TRACK_ORDERS_ITEM, OnRadioTrackOrdersItem)
	ON_BN_CLICKED(IDC_FR_OP_RADIO_TRACK_QUANTITY_ITEM, OnRadioTrackQuantityItem)	*/	
	ON_BN_CLICKED(IDC_FR_OP_BILLABLE, &CFramesOptionsDlg::OnChangeBillable)		
END_MESSAGE_MAP()


// CFramesOptionsDlg message handlers

BOOL CFramesOptionsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_pCatList = BindNxDataList2Ctrl(IDC_FR_OP_CAT_LIST, true);

		// (z.manning 2010-06-23 19:13) - PLID 39311 - No need to do this as these are cached in InvFramesDlg
		/*g_propManager.CachePropertiesInBulk("InvFramesOptions", propNumber,
				"(Username = '<None>' OR Username = '%s') AND ("
				"Name = 'InvFrameOpBillable' OR "
				"Name = 'InvFrameOpTaxable1' OR "
				"Name = 'InvFrameOpTaxable2' OR "
				"Name = 'InvFrameOpTracking' OR "
				"Name = 'InvFrameOpCategory' "				
				")",
				_Q(GetCurrentUserName()));*/

		long nBillable = GetRemotePropertyInt("InvFrameOpBillable", 1, 0, "<None>", true);
		long nTaxable1 = GetRemotePropertyInt("InvFrameOpTaxable1", 0, 0, "<None>", true);
		long nTaxable2= GetRemotePropertyInt("InvFrameOpTaxable2", 0, 0, "<None>", true);
		long nTracking = GetRemotePropertyInt("InvFrameOpTracking", tsTrackQuantity, 0, "<None>", true);
		

		if (nBillable == 1) {
			CheckDlgButton(IDC_FR_OP_BILLABLE, 1);
		}
		else {
			CheckDlgButton(IDC_FR_OP_BILLABLE, 0);
		}

		if (nTaxable1 == 1) {
			CheckDlgButton(IDC_FR_OP_TAXABLE, 1);
		}
		else {
			CheckDlgButton(IDC_FR_OP_TAXABLE, 0);
		}

		if (nTaxable2 == 1) {
			CheckDlgButton(IDC_FR_OP_TAXABLE2, 1);
		}
		else {
			CheckDlgButton(IDC_FR_OP_TAXABLE2, 0);
		}

		if (nTracking == tsNotTrackable) {
			CheckDlgButton(IDC_FR_OP_RADIO_NOT_TRACKABLE_ITEM, 1);
			CheckDlgButton(IDC_FR_OP_RADIO_TRACK_ORDERS_ITEM, 0);
			CheckDlgButton(IDC_FR_OP_RADIO_TRACK_QUANTITY_ITEM, 0);
		}
		else if (nTracking == tsTrackOrders) {
			CheckDlgButton(IDC_FR_OP_RADIO_NOT_TRACKABLE_ITEM, 0);
			CheckDlgButton(IDC_FR_OP_RADIO_TRACK_ORDERS_ITEM, 1);
			CheckDlgButton(IDC_FR_OP_RADIO_TRACK_QUANTITY_ITEM, 0);
		}
		else if (nTracking == tsTrackQuantity) {
			CheckDlgButton(IDC_FR_OP_RADIO_NOT_TRACKABLE_ITEM, 0);
			CheckDlgButton(IDC_FR_OP_RADIO_TRACK_ORDERS_ITEM, 0);
			CheckDlgButton(IDC_FR_OP_RADIO_TRACK_QUANTITY_ITEM, 1);
		}
		else {
			//default to not tracked
			CheckDlgButton(IDC_FR_OP_RADIO_NOT_TRACKABLE_ITEM, 1);
			CheckDlgButton(IDC_FR_OP_RADIO_TRACK_ORDERS_ITEM, 0);
			CheckDlgButton(IDC_FR_OP_RADIO_TRACK_QUANTITY_ITEM, 0);
		}	

		BOOL bShow;
		if (nBillable == 1)
			bShow = TRUE;
		else bShow = FALSE;
		m_taxable.EnableWindow(bShow);
		m_taxable2.EnableWindow(bShow);	
		if (!bShow) {
			m_taxable.SetCheck(0);
			m_taxable2.SetCheck(0);
		}

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}



void CFramesOptionsDlg::OnChangeBillable() 
{
	try {
		BOOL bShow;
		if (m_billable.GetCheck())
			bShow = TRUE;
		else bShow = FALSE;
		m_taxable.EnableWindow(bShow);
		m_taxable2.EnableWindow(bShow);	
		if (!bShow) {
			m_taxable.SetCheck(0);
			m_taxable2.SetCheck(0);
		}
	}NxCatchAll(__FUNCTION__);
}


void CFramesOptionsDlg::OnOK(){

	try {

		SetRemotePropertyInt("InvFrameOpBillable", IsDlgButtonChecked(IDC_FR_OP_BILLABLE) ? 1 : 0, 0, "<None>");
		//only set the tax if billable is checked
		if ( IsDlgButtonChecked(IDC_FR_OP_BILLABLE) ) {
			SetRemotePropertyInt("InvFrameOpTaxable1", IsDlgButtonChecked(IDC_FR_OP_TAXABLE) ? 1 : 0, 0, "<None>");
			SetRemotePropertyInt("InvFrameOpTaxable2", IsDlgButtonChecked(IDC_FR_OP_TAXABLE2) ? 1 : 0, 0, "<None>");
		}
		else {
			SetRemotePropertyInt("InvFrameOpTaxable1", 0, 0, "<None>");
			SetRemotePropertyInt("InvFrameOpTaxable2", 0, 0, "<None>");
		}

		long nTracking = tsNotTrackable;
		if (IsDlgButtonChecked(IDC_FR_OP_RADIO_TRACK_ORDERS_ITEM)) {
			nTracking = tsTrackOrders;
		}
		else if (IsDlgButtonChecked(IDC_FR_OP_RADIO_TRACK_QUANTITY_ITEM)) {
			nTracking = tsTrackQuantity;
		}	
		SetRemotePropertyInt("InvFrameOpTracking", nTracking, 0, "<None>");
		
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCatList->CurSel;
		long nCategoryID = -1;
		if (pRow) {
			nCategoryID = VarLong(pRow->GetValue(clcID), -1);				
		}
		SetRemotePropertyInt("InvFrameOpCategory", nCategoryID, 0, "<None>");		

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}

void CFramesOptionsDlg::OnCancel(){
	try {
		CNxDialog::OnCancel();
	}NxCatchAll(__FUNCTION__);
}


BEGIN_EVENTSINK_MAP(CFramesOptionsDlg, CNxDialog)
ON_EVENT(CFramesOptionsDlg, IDC_FR_OP_CAT_LIST, 18, CFramesOptionsDlg::RequeryFinishedFrOpCatList, VTS_I2)
END_EVENTSINK_MAP()

void CFramesOptionsDlg::RequeryFinishedFrOpCatList(short nFlags)
{
	try {

		long nCategoryID = GetRemotePropertyInt("InvFrameOpCategory", fpcvCollection, 0, "<None>", true);

		//add our extra options
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCatList->GetNewRow();
		NXDATALIST2Lib::IRowSettingsPtr pRowTemp = m_pCatList->GetFirstRow();
		if (pRow) {
			pRow->PutValue(clcID, (long)fpcvNone);
			pRow->PutValue(clcName, _variant_t("<No Category>"));
			m_pCatList->AddRowBefore(pRow, pRowTemp);			
		}

		pRow = m_pCatList->GetNewRow();
		if (pRow) {
			pRow->PutValue(clcID, (long)fpcvCollection);
			pRow->PutValue(clcName, _variant_t("<Frame Collection>"));
			m_pCatList->AddRowBefore(pRow, pRowTemp);			
		}

		pRow = m_pCatList->GetNewRow();
		if (pRow) {
			pRow->PutValue(clcID, (long)fpcvManufacturer);
			pRow->PutValue(clcName, _variant_t("<Frame Manufacturer>"));
			m_pCatList->AddRowBefore(pRow, pRowTemp);			
		}

		pRow = m_pCatList->GetNewRow();
		if (pRow) {
			pRow->PutValue(clcID, (long)fpcvBrand);
			pRow->PutValue(clcName, _variant_t("<Frame Brand>"));
			m_pCatList->AddRowBefore(pRow, pRowTemp);		
		}

		pRow = m_pCatList->GetNewRow();
		if (pRow) {
			pRow->PutValue(clcID, (long)fpcvGroup);
			pRow->PutValue(clcName, _variant_t("<Frame Product Group Name>"));
			m_pCatList->AddRowBefore(pRow, pRowTemp);			
		}

		//now set the selction
		m_pCatList->SetSelByColumn(clcID, nCategoryID);

	}NxCatchAll(__FUNCTION__);
}
