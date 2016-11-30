// InvFrameSelection.cpp : implementation file
//
// (s.dhole 2012-04-17 09:24) - PLID 49734  New dialog, to select frames from a Framdata
#include "stdafx.h"
#include "Practice.h"
#include "InvFrameSelection.h"
#include "InventoryRc.h"


// CInvFrameSelection dialog

IMPLEMENT_DYNAMIC(CInvFrameSelection, CNxDialog)
using namespace ADODB;
using namespace NXDATALIST2Lib;
CInvFrameSelection::CInvFrameSelection(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInvFrameSelection::IDD, pParent)
{
	m_nFrameDataID =-1;
}

CInvFrameSelection::~CInvFrameSelection()
{
}

void CInvFrameSelection::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, ID_BTN_FRAMES_SELECT_OK, m_nxbFrameOK);
	DDX_Control(pDX, ID_BTN_FRAMES_SELECT_CANCEL, m_nxbFrameCancel);
	DDX_Control(pDX, IDC_BTN_FRAMES_SELECT_SERACH, m_nxbFrameSearch);

 	
}


BEGIN_MESSAGE_MAP(CInvFrameSelection, CNxDialog)
	ON_BN_CLICKED(ID_BTN_FRAMES_SELECT_OK, &CInvFrameSelection::OnBnClickedSelectFrameOk)
	ON_BN_CLICKED(ID_BTN_FRAMES_SELECT_CANCEL, &CInvFrameSelection::OnBnClickedSelectFrameCancel)
	ON_BN_CLICKED(IDC_BTN_FRAMES_SELECT_SERACH, &CInvFrameSelection::OnBnClickedSelectFrameSerach)
	ON_BN_CLICKED(IDC_CHK_FRAME_STYLE, &CInvFrameSelection::OnSelectStyle)
	ON_BN_CLICKED(IDC_CHK_FRAME_MANUFATURER, &CInvFrameSelection::OnSelectManufacturer)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CInvFrameSelection, CNxDialog)
	ON_EVENT(CInvFrameSelection, IDC_FRAME_SELECT_LIST, 18, CInvFrameSelection::OnRequeryFinishedFrameList, VTS_I2)
	ON_EVENT(CInvFrameSelection, IDC_FRAMES_SELECT_COMBO_MANUFACTURER, 16, CInvFrameSelection::OnSelChosenManufactuer, VTS_DISPATCH)
	ON_EVENT(CInvFrameSelection, IDC_FRAMES_SELECT_COMBO_MANUFACTURER, 1, CInvFrameSelection::SelChangingManufactuer, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CInvFrameSelection, IDC_FRAMES_SELECT_COMBO_STYLE, 16, CInvFrameSelection::OnSelChosenStyle, VTS_DISPATCH)
	ON_EVENT(CInvFrameSelection, IDC_FRAMES_SELECT_COMBO_STYLE, 1, CInvFrameSelection::SelChangingStyle, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CInvFrameSelection, IDC_FRAMES_SELECT_COMBO_COLLECTION, 16, CInvFrameSelection::OnSelChosenCollection, VTS_DISPATCH)
	ON_EVENT(CInvFrameSelection, IDC_FRAMES_SELECT_COMBO_COLLECTION, 1, CInvFrameSelection::SelChangingCollection, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CInvFrameSelection, IDC_FRAMES_SELECT_COMBO_BRAND, 16, CInvFrameSelection::OnSelChosenBrand, VTS_DISPATCH)
	ON_EVENT(CInvFrameSelection, IDC_FRAMES_SELECT_COMBO_BRAND, 1, CInvFrameSelection::SelChangingBrand, VTS_DISPATCH VTS_PDISPATCH)
END_EVENTSINK_MAP()

// CInvFrameSelection message handlers
BOOL CInvFrameSelection::OnInitDialog()
{
	CNxDialog::OnInitDialog();
	try {
		m_nxbFrameOK.AutoSet(NXB_OK);
		m_nxbFrameCancel.AutoSet(NXB_CANCEL);
		m_nxbFrameSearch.AutoSet(NXB_REFRESH);
		m_pFrameManufacturer = BindNxDataList2Ctrl(IDC_FRAMES_SELECT_COMBO_MANUFACTURER, true);
		m_pFrameStyle = BindNxDataList2Ctrl(IDC_FRAMES_SELECT_COMBO_STYLE, true);
		m_pFrameFrameCollection = BindNxDataList2Ctrl(IDC_FRAMES_SELECT_COMBO_COLLECTION, true);
		m_pFrameBrand = BindNxDataList2Ctrl(IDC_FRAMES_SELECT_COMBO_BRAND, true);
		m_pFrameList = BindNxDataList2Ctrl(IDC_FRAME_SELECT_LIST, false);
		
		// (s.dhole 2012-05-31 18:39) - PLID 50730
		m_pFrameManufacturer->CurSel = NULL;
		m_pFrameStyle->CurSel = NULL;
		m_pFrameFrameCollection->FindByColumn( 0,_bstr_t(" <ALL>"),NULL,g_cvarTrue );  
		m_pFrameBrand->FindByColumn( 0,_bstr_t(" <ALL>"),NULL,g_cvarTrue );  
		GetDlgItem(IDC_BTN_FRAMES_SELECT_SERACH)->EnableWindow(FALSE);
		CheckDlgButton(IDC_CHK_FRAME_MANUFATURER, BST_CHECKED);
		CheckDlgButton(IDC_CHK_FRAME_STYLE, BST_CHECKED);
	}NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}



void CInvFrameSelection::OnRequeryFinishedFrameList(short nFlags)
{
	try {
		if (m_pFrameList->GetRowCount()<=0){
			m_nxbFrameOK.EnableWindow(FALSE);
			m_nFrameDataID =-1;
			// (s.dhole 2012-05-31 18:39) - PLID 50730
			SetDlgItemText(IDC_LBL_FRAMES_COUNT, FormatString("Matching Frames: %s" , FormatDoubleForInterface(0,0)  ));
		}
		else{
			m_nxbFrameOK.EnableWindow(TRUE);
			m_pFrameList->HighlightVisible =TRUE;
			m_pFrameList->FindByColumn(0,m_nFrameDataID ,NULL,TRUE); 
			// (s.dhole 2012-05-31 18:39) - PLID 50730
			SetDlgItemText(IDC_LBL_FRAMES_COUNT, FormatString("Matching Frames: %s" , FormatDoubleForInterface(m_pFrameList->GetRowCount(),0)  ));
		}
	}NxCatchAll(__FUNCTION__);
}


void CInvFrameSelection::SelChangingManufactuer(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}


void CInvFrameSelection::OnSelChosenManufactuer(LPDISPATCH lpRow)
{
	try {
			IRowSettingsPtr pRow(lpRow);
			// (s.dhole 2012-05-31 18:39) - PLID 50730
			if (pRow)
				GetDlgItem(IDC_BTN_FRAMES_SELECT_SERACH)->EnableWindow(TRUE);
			
	}NxCatchAll(__FUNCTION__);
}


void CInvFrameSelection::SelChangingStyle(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}

void CInvFrameSelection::OnSelChosenStyle(LPDISPATCH lpRow)
{
	try {
			IRowSettingsPtr pRow(lpRow);
			// (s.dhole 2012-05-31 18:39) - PLID 50730
			if (pRow)
				GetDlgItem(IDC_BTN_FRAMES_SELECT_SERACH)->EnableWindow(TRUE);
			
	}NxCatchAll(__FUNCTION__);
}

void CInvFrameSelection::SelChangingCollection(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}

void CInvFrameSelection::OnSelChosenCollection(LPDISPATCH lpRow)
{
	try {
			IRowSettingsPtr pRow(lpRow);
			if (!pRow)
				return;
			
			
	}NxCatchAll(__FUNCTION__);
}

void CInvFrameSelection::SelChangingBrand(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}

void CInvFrameSelection::OnSelChosenBrand(LPDISPATCH lpRow)
{
	try {
			IRowSettingsPtr pRow(lpRow);
			if (!pRow)
				return;
			
	}NxCatchAll(__FUNCTION__);
}


void CInvFrameSelection::OnBnClickedSelectFrameSerach()
{
	try{
		ApplyFilter ();
	}NxCatchAll(__FUNCTION__);


}


void CInvFrameSelection::OnBnClickedSelectFrameOk()
{
	try{
		IRowSettingsPtr pRow = m_pFrameList ->CurSel;
		if (pRow ) {
			// (s.dhole 2012-04-25 13:12) - PLID 49734 We should kill process if user cselect frmae
			m_nFrameDataID =  VarLong( pRow->GetValue(0));   
			if(m_pFrameList->IsRequerying()){
				m_pFrameList->CancelRequery(); 
			}
		}
		else{
			m_nFrameDataID=-1;
			AfxMessageBox("Please select a frame");
			return;
		}
				
	}NxCatchAll(__FUNCTION__);

	CNxDialog::OnOK();
}

void CInvFrameSelection::OnBnClickedSelectFrameCancel()
{
	try{
	// (s.dhole 2012-04-25 13:12) - PLID 49734 We should kill process if user cselect frmae
		if(m_pFrameList->IsRequerying()){
			m_pFrameList->CancelRequery(); 
		}
	}NxCatchAll(__FUNCTION__);

	CNxDialog::OnCancel();
}

// (s.dhole 2012-05-31 18:39) - PLID 50730
void CInvFrameSelection::ApplyFilter()
{
	try{
		CWaitCursor wc;
	
		CString strWhere = "(FramesDataT.IsCatalog = 1 OR FramesDataT.FPC Is Null)" ;
		IRowSettingsPtr pRow ;
		if  (IsDlgButtonChecked(IDC_CHK_FRAME_MANUFATURER)){
			pRow = m_pFrameManufacturer ->CurSel;
			if (pRow ) {
				CString sTemp= VarString(  pRow->GetValue(0),"");
				strWhere += FormatString( " AND ManufacturerName = '%s' " ,_Q(sTemp ));
			}
			else{
				strWhere += FormatString( " AND ManufacturerName IS NULL " );
			}
		}
		 if  (IsDlgButtonChecked(IDC_CHK_FRAME_STYLE)){
			pRow = m_pFrameStyle ->CurSel;
			if (pRow ) {
				CString sTemp= VarString(  pRow->GetValue(0),"");
				strWhere += FormatString( " AND StyleName = '%s' " ,_Q(sTemp ));
				
			}
			else{
				strWhere += FormatString( " AND StyleName IS NULL " );
			}
		}

		pRow = m_pFrameFrameCollection ->CurSel;
		if (pRow ) {
			CString sTemp= VarString(  pRow->GetValue(0),"");
			if (!sTemp.IsEmpty() && sTemp.CompareNoCase(" <ALL>")!=0 )
			{
				strWhere += FormatString( " AND CollectionName = '%s' " ,_Q(sTemp) );
			}
		}

		pRow = m_pFrameBrand ->CurSel;
		if (pRow ) {
			CString sTemp= VarString(  pRow->GetValue(0),"");
			if (!sTemp.IsEmpty() && sTemp.CompareNoCase(" <ALL>")!=0 )
			{
				strWhere += FormatString( " AND BrandName = '%s' " ,_Q(sTemp) );
			}
		}
	/*	if(m_pFrameList->IsRequerying()){
				m_pFrameList->WaitForRequery(dlPatienceLevelTerminate);
		}
		*/
		m_pFrameList->Clear();
		m_pFrameList->WhereClause=bstr_t(strWhere);
		m_pFrameList->Requery();  
		// (s.dhole 2012-05-31 18:39) - PLID 50730
		SetDlgItemText(IDC_LBL_FRAMES_COUNT, "Matching Frames: Loading...");
	}NxCatchAll(__FUNCTION__);

}

// (s.dhole 2012-05-31 18:39) - PLID 50730
void CInvFrameSelection::OnSelectStyle()
{
	try {
		if  (IsDlgButtonChecked(IDC_CHK_FRAME_STYLE)){
			GetDlgItem(IDC_FRAMES_SELECT_COMBO_STYLE)->EnableWindow(TRUE);
		}
		else{
			GetDlgItem(IDC_FRAMES_SELECT_COMBO_STYLE)->EnableWindow(FALSE);
		}
		if  (!IsDlgButtonChecked(IDC_CHK_FRAME_STYLE) && !IsDlgButtonChecked(IDC_CHK_FRAME_MANUFATURER) ){
			GetDlgItem(IDC_BTN_FRAMES_SELECT_SERACH)->EnableWindow(FALSE);
			GetDlgItem(IDC_FRAMES_SELECT_COMBO_BRAND)->EnableWindow(FALSE);
			GetDlgItem(IDC_FRAMES_SELECT_COMBO_COLLECTION)->EnableWindow(FALSE);
		}
		else
		{
			GetDlgItem(IDC_BTN_FRAMES_SELECT_SERACH)->EnableWindow(TRUE);
			GetDlgItem(IDC_FRAMES_SELECT_COMBO_BRAND)->EnableWindow(TRUE);
			GetDlgItem(IDC_FRAMES_SELECT_COMBO_COLLECTION)->EnableWindow(TRUE);
		}
		

	}NxCatchAll(__FUNCTION__);
}

// (s.dhole 2012-05-31 18:39) - PLID 50730
void CInvFrameSelection::OnSelectManufacturer()
{
	try {
		if  (IsDlgButtonChecked(IDC_CHK_FRAME_MANUFATURER)){
			GetDlgItem(IDC_FRAMES_SELECT_COMBO_MANUFACTURER)->EnableWindow(TRUE);
		}
		else{
			GetDlgItem(IDC_FRAMES_SELECT_COMBO_MANUFACTURER)->EnableWindow(FALSE);
		}

		if  (!IsDlgButtonChecked(IDC_CHK_FRAME_STYLE) && !IsDlgButtonChecked(IDC_CHK_FRAME_MANUFATURER) ){
			GetDlgItem(IDC_FRAMES_SELECT_COMBO_BRAND)->EnableWindow(FALSE);
			GetDlgItem(IDC_FRAMES_SELECT_COMBO_COLLECTION)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_FRAMES_SELECT_SERACH)->EnableWindow(FALSE);
		}
		else
		{
			GetDlgItem(IDC_BTN_FRAMES_SELECT_SERACH)->EnableWindow(TRUE);
			GetDlgItem(IDC_FRAMES_SELECT_COMBO_BRAND)->EnableWindow(TRUE);
			GetDlgItem(IDC_FRAMES_SELECT_COMBO_COLLECTION)->EnableWindow(TRUE);
		}
	}NxCatchAll(__FUNCTION__);
}
