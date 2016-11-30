// CaseHistorySelectDetailDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CaseHistorySelectDetailDlg.h"
#include "CaseHistoryDlg.h"
#include "BarCode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum CPTCodeColumns {
	chsddCptCodeID = 0,
	chsddCptCodeName = 3,
	chsddCptCodeAmount = 4,
	chsddCptCodeIsLinkedToProducts = 5, //TES 7/16/2008 - PLID 27983
	chsddCptCodeBarcode = 6,	// (j.jones 2011-06-15 10:36) - PLID 24507 - added barcode column, it's not visible, but it is scannable
};

// (j.jones 2008-05-20 17:53) - PLID 29249 - added IsSerialized column, also named these enums
enum ProductColumns {
	chsddProductID = 0,
	chsddProductCategory = 1,
	chsddProductSupplier = 2,
	chsddProductName = 3,
	chsddProductBarcode = 4,	// (j.jones 2008-09-08 12:11) - PLID 15345 - added barcode column, it's visible for products
	chsddProductAmount = 5,
	chsddProductCost = 6,
	chsddIsSerialized = 7,
};

enum PersonColumns {
	chsddPersonID = 0,
	chsddPersonName = 1,
	chsddPersonCost = 2,
	chsddPersonType = 3,
};

enum PreferenceCardColumns {
	chsddPreferenceCardID = 0,
	chsddPreferenceCardName,
};

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CCaseHistorySelectDetailDlg dialog


CCaseHistorySelectDetailDlg::CCaseHistorySelectDetailDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CCaseHistorySelectDetailDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCaseHistorySelectDetailDlg)
		m_nPreferenceCardID = -1;
		m_bIsSerializedProduct = FALSE;
		m_bIsLinkedToProducts = FALSE;
		m_bCaseHistoryIsCompleted = FALSE;
	//}}AFX_DATA_INIT
}


void CCaseHistorySelectDetailDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCaseHistorySelectDetailDlg)
	DDX_Control(pDX, IDC_PRODUCT_RADIO, m_btnProduct);
	DDX_Control(pDX, IDC_CPTCODE_RADIO, m_btnCpt);
	DDX_Control(pDX, IDC_PERSON_RADIO, m_btnPerson);
	DDX_Control(pDX, IDC_PREFERENCE_CARD_RADIO, m_btnPreferenceCard);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCaseHistorySelectDetailDlg, CNxDialog)
	//{{AFX_MSG_MAP(CCaseHistorySelectDetailDlg)
	ON_BN_CLICKED(IDC_CPTCODE_RADIO, OnCptcodeRadio)
	ON_BN_CLICKED(IDC_PRODUCT_RADIO, OnProductRadio)
	ON_BN_CLICKED(IDC_PERSON_RADIO, OnPersonRadio)
	ON_BN_CLICKED(IDC_PREFERENCE_CARD_RADIO, OnPreferenceCardRadio)
	ON_MESSAGE(WM_BARCODE_SCAN, OnBarcodeScan)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCaseHistorySelectDetailDlg message handlers

BOOL CCaseHistorySelectDetailDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-05-01 10:14) - PLID 29863 - NxIconify buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_dlProductCombo = BindNxDataListCtrl(this, IDC_PRODUCT_COMBO, GetRemoteData(), true);
		IColumnSettingsPtr(m_dlProductCombo->GetColumn(0))->PutBackColor(GetNxColor(GNC_PRODUCT,-1));
		IColumnSettingsPtr(m_dlProductCombo->GetColumn(1))->PutBackColor(GetNxColor(GNC_PRODUCT,-1));
		IColumnSettingsPtr(m_dlProductCombo->GetColumn(2))->PutBackColor(GetNxColor(GNC_PRODUCT,-1));
		IColumnSettingsPtr(m_dlProductCombo->GetColumn(3))->PutBackColor(GetNxColor(GNC_PRODUCT,-1));
		IColumnSettingsPtr(m_dlProductCombo->GetColumn(4))->PutBackColor(GetNxColor(GNC_PRODUCT,-1));
		IColumnSettingsPtr(m_dlProductCombo->GetColumn(5))->PutBackColor(GetNxColor(GNC_PRODUCT,-1));
		IColumnSettingsPtr(m_dlProductCombo->GetColumn(6))->PutBackColor(GetNxColor(GNC_PRODUCT,-1));
		
		CheckDlgButton(IDC_PRODUCT_RADIO, 1);
		CheckDlgButton(IDC_CPTCODE_RADIO, 0);
		CheckDlgButton(IDC_PERSON_RADIO, 0);
		GetDlgItem(IDC_PRODUCT_COMBO)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_CPTCODE_COMBO)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_PERSON_COMBO)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_PREFERENCE_CARD_ADD_COMBO)->ShowWindow(SW_HIDE);

		GetMainFrame()->RegisterForBarcodeScan(this);
	}
	NxCatchAll("Error in CCaseHistorySelectDetailDlg::OnInitDialog");

	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// (j.jones 2011-06-15 10:44) - PLID 24507 - added OnDestroy
void CCaseHistorySelectDetailDlg::OnDestroy()
{
	try {
		
		GetMainFrame()->UnregisterForBarcodeScan(this);

	}NxCatchAll(__FUNCTION__);

	// (a.walling 2012-01-26 13:24) - PLID 47814 - Need to call base class when handling OnDestroy!
	__super::OnDestroy();
}

void CCaseHistorySelectDetailDlg::OnCptcodeRadio() 
{
	try {

		if (m_dlCptCodeCombo == NULL) {

			// (j.jones 2011-06-15 10:55) - PLID 24507 - moved this code to a new function
			InitializeCPTCombo();
		}
		GetDlgItem(IDC_PRODUCT_COMBO)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_CPTCODE_COMBO)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_PERSON_COMBO)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_PREFERENCE_CARD_ADD_COMBO)->ShowWindow(SW_HIDE);

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-06-15 10:55) - PLID 24507 - added InitializeCPTCombo
void CCaseHistorySelectDetailDlg::InitializeCPTCombo()
{
	if(m_dlCptCodeCombo == NULL) {
		m_dlCptCodeCombo = BindNxDataListCtrl(this, IDC_CPTCODE_COMBO, GetRemoteData(), false);
		if(m_bCaseHistoryIsCompleted) {
			//TES 7/16/2008 - PLID 27983 - If we're on a completed case history, don't show CPT Codes that are linked to products.
			// (j.jones 2011-03-28 17:00) - PLID 42575 - ignore non-billable CPT codes
			// (j.jones 2011-07-21 08:51) - PLID 44645 - fixed poor SQL syntax
			m_dlCptCodeCombo->WhereClause = _bstr_t("ServiceT.Active = 1 AND CPTCodeT.Billable = 1 AND LinkedServicesQ.CptID Is Null");
		}
		m_dlCptCodeCombo->Requery();
		IColumnSettingsPtr(m_dlCptCodeCombo->GetColumn(0))->PutBackColor(GetNxColor(GNC_CPT_CODE,-1));
		IColumnSettingsPtr(m_dlCptCodeCombo->GetColumn(1))->PutBackColor(GetNxColor(GNC_CPT_CODE,-1));
		IColumnSettingsPtr(m_dlCptCodeCombo->GetColumn(2))->PutBackColor(GetNxColor(GNC_CPT_CODE,-1));
		IColumnSettingsPtr(m_dlCptCodeCombo->GetColumn(3))->PutBackColor(GetNxColor(GNC_CPT_CODE,-1));
		IColumnSettingsPtr(m_dlCptCodeCombo->GetColumn(4))->PutBackColor(GetNxColor(GNC_CPT_CODE,-1));
		IColumnSettingsPtr(m_dlCptCodeCombo->GetColumn(chsddCptCodeIsLinkedToProducts))->PutBackColor(GetNxColor(GNC_CPT_CODE,-1));
		IColumnSettingsPtr(m_dlCptCodeCombo->GetColumn(chsddCptCodeBarcode))->PutBackColor(GetNxColor(GNC_CPT_CODE,-1));
	}
}

void CCaseHistorySelectDetailDlg::OnProductRadio() 
{
	if (m_dlProductCombo == NULL) {
		m_dlProductCombo = BindNxDataListCtrl(this, IDC_PRODUCT_COMBO, GetRemoteData(), true);
		IColumnSettingsPtr(m_dlProductCombo->GetColumn(0))->PutBackColor(GetNxColor(GNC_PRODUCT,-1));
		IColumnSettingsPtr(m_dlProductCombo->GetColumn(1))->PutBackColor(GetNxColor(GNC_PRODUCT,-1));
		IColumnSettingsPtr(m_dlProductCombo->GetColumn(2))->PutBackColor(GetNxColor(GNC_PRODUCT,-1));
		IColumnSettingsPtr(m_dlProductCombo->GetColumn(3))->PutBackColor(GetNxColor(GNC_PRODUCT,-1));
		IColumnSettingsPtr(m_dlProductCombo->GetColumn(4))->PutBackColor(GetNxColor(GNC_PRODUCT,-1));
		IColumnSettingsPtr(m_dlProductCombo->GetColumn(5))->PutBackColor(GetNxColor(GNC_PRODUCT,-1));
	}
	GetDlgItem(IDC_PRODUCT_COMBO)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_CPTCODE_COMBO)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_PERSON_COMBO)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_PREFERENCE_CARD_ADD_COMBO)->ShowWindow(SW_HIDE);
}

void CCaseHistorySelectDetailDlg::OnPersonRadio() 
{
	if (m_dlPersonCombo == NULL) {
		m_dlPersonCombo = BindNxDataListCtrl(this, IDC_PERSON_COMBO, GetRemoteData(), true);
		IColumnSettingsPtr(m_dlPersonCombo->GetColumn(chsddPersonID))->PutBackColor(GetNxColor(GNC_PERSONNEL,-1));
		IColumnSettingsPtr(m_dlPersonCombo->GetColumn(chsddPersonName))->PutBackColor(GetNxColor(GNC_PERSONNEL,-1));
		IColumnSettingsPtr(m_dlPersonCombo->GetColumn(chsddPersonCost))->PutBackColor(GetNxColor(GNC_PERSONNEL,-1));
		IColumnSettingsPtr(m_dlPersonCombo->GetColumn(chsddPersonType))->PutBackColor(GetNxColor(GNC_PERSONNEL,-1));

		// (j.jones 2008-07-01 09:34) - PLID 18744 - hide the cost column if they do not have
		// permission to view the personnel cost
		if(!(GetCurrentUserPermissions(bioContactsDefaultCost) & sptRead)) {
			IColumnSettingsPtr(m_dlPersonCombo->GetColumn(chsddPersonCost))->PutStoredWidth(0);
			IColumnSettingsPtr(m_dlPersonCombo->GetColumn(chsddPersonCost))->ColumnStyle = csFixedWidth|csVisible;
		}
	}
	GetDlgItem(IDC_PRODUCT_COMBO)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_CPTCODE_COMBO)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_PERSON_COMBO)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_PREFERENCE_CARD_ADD_COMBO)->ShowWindow(SW_HIDE);
}

// (j.jones 2009-08-19 16:32) - PLID 35124 - changed to use Preference Cards, not surgeries
void CCaseHistorySelectDetailDlg::OnPreferenceCardRadio() 
{
	if (m_dlPreferenceCardCombo == NULL) {
		m_dlPreferenceCardCombo = BindNxDataListCtrl(this, IDC_PREFERENCE_CARD_ADD_COMBO, GetRemoteData(), true);
		//use the color purple (not the movie), just halfway between blue and red
		//it's really light, but just colored enough to not be white
		IColumnSettingsPtr(m_dlPreferenceCardCombo->GetColumn(0))->PutBackColor(RGB(242,237,237));
		IColumnSettingsPtr(m_dlPreferenceCardCombo->GetColumn(1))->PutBackColor(RGB(242,237,237));
		IColumnSettingsPtr(m_dlPreferenceCardCombo->GetColumn(2))->PutBackColor(RGB(242,237,237));
		IColumnSettingsPtr(m_dlPreferenceCardCombo->GetColumn(3))->PutBackColor(RGB(242,237,237));
	}
	GetDlgItem(IDC_PRODUCT_COMBO)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_CPTCODE_COMBO)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_PERSON_COMBO)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_PREFERENCE_CARD_ADD_COMBO)->ShowWindow(SW_SHOW);
}

void CCaseHistorySelectDetailDlg::OnOK() 
{
	try {

		if (IsDlgButtonChecked(IDC_PRODUCT_RADIO)) {
			long nCurSel = m_dlProductCombo->GetCurSel();
			if (nCurSel != -1) {
				m_nID = VarLong(m_dlProductCombo->GetValue(nCurSel, chsddProductID));
				m_strName = VarString(m_dlProductCombo->GetValue(nCurSel, chsddProductName));
				m_nType = chditProduct;
				m_dblQuantity = 1.0;
				m_cyAmount = VarCurrency(m_dlProductCombo->GetValue(nCurSel, chsddProductAmount));
				m_cyCost = VarCurrency(m_dlProductCombo->GetValue(nCurSel, chsddProductCost));
				m_bBillable = true;
				// (j.jones 2009-08-19 15:30) - PLID 35124 - removed PayToPractice
				//m_bPayToPractice = true;
				// (j.jones 2008-05-20 17:55) - PLID 29249 - pull the serialized status from the combo
				m_bIsSerializedProduct = VarBool(m_dlProductCombo->GetValue(nCurSel, chsddIsSerialized), FALSE);
				m_bIsLinkedToProducts = FALSE;
				CDialog::OnOK();
				return;
			}
		} else if (IsDlgButtonChecked(IDC_CPTCODE_RADIO)) {
			long nCurSel = m_dlCptCodeCombo->GetCurSel();
			if (nCurSel != -1) {
				m_nID = VarLong(m_dlCptCodeCombo->GetValue(nCurSel, chsddCptCodeID));
				m_strName = VarString(m_dlCptCodeCombo->GetValue(nCurSel, chsddCptCodeName));
				m_nType = chditCptCode;
				m_dblQuantity = 1.0;
				m_cyAmount = VarCurrency(m_dlCptCodeCombo->GetValue(nCurSel, chsddCptCodeAmount));
				m_cyCost = COleCurrency(0,0);
				m_bBillable = true;
				// (j.jones 2009-08-19 15:30) - PLID 35124 - removed PayToPractice
				//m_bPayToPractice = true;
				m_bIsSerializedProduct = FALSE;
				//TES 7/16/2008 - PLID 27983 - Pull from the combo whether it's linked to products.
				m_bIsLinkedToProducts = VarBool(m_dlCptCodeCombo->GetValue(nCurSel, chsddCptCodeIsLinkedToProducts));
				CDialog::OnOK();
				return;
			}
		} else if (IsDlgButtonChecked(IDC_PERSON_RADIO)) {
			long nCurSel = m_dlPersonCombo->GetCurSel();
			if (nCurSel != -1) {
				m_nID = VarLong(m_dlPersonCombo->GetValue(nCurSel, chsddPersonID));
				m_strName = VarString(m_dlPersonCombo->GetValue(nCurSel, chsddPersonName));
				m_nType = chditPerson;
				m_dblQuantity = 1.0;
				m_cyAmount = COleCurrency(0,0);
				m_cyCost = VarCurrency(m_dlPersonCombo->GetValue(nCurSel, chsddPersonCost),COleCurrency(0,0));
				m_bBillable = false;
				// (j.jones 2009-08-19 15:30) - PLID 35124 - removed PayToPractice
				//m_bPayToPractice = true;
				m_bIsSerializedProduct = FALSE;
				m_bIsLinkedToProducts = FALSE;
				CDialog::OnOK();
				return;
			}
		} else if (IsDlgButtonChecked(IDC_PREFERENCE_CARD_RADIO)) {
			long nCurSel = m_dlPreferenceCardCombo->GetCurSel();
			if (nCurSel != -1) {
				m_nPreferenceCardID = VarLong(m_dlPreferenceCardCombo->GetValue(nCurSel, chsddPreferenceCardID));
				CDialog::OnOK();
				return;
			}
		}
		
		// If we made it here, nothing was selected
		MessageBox("Please select a Product, Service Code, Person, or Preference Card.","Practice",MB_ICONINFORMATION|MB_OK);

	} NxCatchAll("CCaseHistorySelectDetailDlg::OnOK");
}

BEGIN_EVENTSINK_MAP(CCaseHistorySelectDetailDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CCaseHistorySelectDetailDlg)
	ON_EVENT(CCaseHistorySelectDetailDlg, IDC_PREFERENCE_CARD_ADD_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedPreferenceCardCombo, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CCaseHistorySelectDetailDlg::OnRequeryFinishedPreferenceCardCombo(short nFlags) 
{
	try {

		//get the provider names that use this surgery, and update the description
		for(int i=0;i<m_dlPreferenceCardCombo->GetRowCount();i++) {

			CString str;
			// (j.jones 2009-08-19 16:31) - PLID 35124 - this is now PreferenceCardProvidersT
			str.Format("BEGIN "
					" "
					"declare @names nvarchar(1000) "
					" "
					"SELECT @names = coalesce(@names + ' / ','') + coalesce(Last + ', ','') + coalesce(First + ' ','')  + coalesce(Middle + ' ','') + coalesce(Title,'') "
					"FROM PersonT INNER JOIN PreferenceCardProvidersT ON PersonT.ID = PreferenceCardProvidersT.ProviderID "
					"WHERE PreferenceCardID = %li ORDER BY PersonT.Last, PersonT.First, PersonT.Middle "
					" "
					"SELECT coalesce(@names,'') AS ProviderNames "
					"END",VarLong(m_dlPreferenceCardCombo->GetValue(i, chsddPreferenceCardID)));

			ADODB::_RecordsetPtr rs = GetRemoteData()->Execute(_bstr_t(str), NULL, ADODB::adCmdText);
			if(!rs->eof) {
				CString strProvNames = AdoFldString(rs, "ProviderNames","");
				if(strProvNames.GetLength() > 0) {
					CString strPrefCardName = VarString(m_dlPreferenceCardCombo->GetValue(i,chsddPreferenceCardName),"");
					str.Format("%s (%s)",strPrefCardName,strProvNames);
					m_dlPreferenceCardCombo->PutValue(i, chsddPreferenceCardName,_bstr_t(str));
				}
			}
			rs->Close();
		}

	}NxCatchAll("Error generating preference card provider list.");	
}

// (j.jones 2011-06-15 10:47) - PLID 24507 - added OnBarcodeScan
LRESULT CCaseHistorySelectDetailDlg::OnBarcodeScan(WPARAM wParam, LPARAM lParam)
{
	try {

		//Need to convert this correctly from a bstr
		CString strBarcode = (LPCTSTR)_bstr_t((BSTR)lParam);
		
		//first see if this barcode exists in the product list
		long nProductRow = m_dlProductCombo->FindByColumn(chsddProductBarcode, _bstr_t(strBarcode), 0, VARIANT_TRUE);
		if(nProductRow != -1) {
			//select the product radio button
			CheckDlgButton(IDC_PRODUCT_RADIO, 1);
			CheckDlgButton(IDC_CPTCODE_RADIO, 0);
			CheckDlgButton(IDC_PERSON_RADIO, 0);
			CheckDlgButton(IDC_PREFERENCE_CARD_RADIO, 0);
			OnProductRadio();
			return 0;
		}

		//try the CPT list
		if (m_dlCptCodeCombo == NULL) {
			InitializeCPTCombo();
		}

		long nCPTRow = m_dlCptCodeCombo->FindByColumn(chsddCptCodeBarcode, _bstr_t(strBarcode), 0, VARIANT_TRUE);
		if(nCPTRow != -1) {
			//select the cpt radio button
			CheckDlgButton(IDC_PRODUCT_RADIO, 0);
			CheckDlgButton(IDC_CPTCODE_RADIO, 1);
			CheckDlgButton(IDC_PERSON_RADIO, 0);
			CheckDlgButton(IDC_PREFERENCE_CARD_RADIO, 0);
			OnCptcodeRadio();
			return 0;
		}

	}NxCatchAll(__FUNCTION__);
	
	return 0;
}