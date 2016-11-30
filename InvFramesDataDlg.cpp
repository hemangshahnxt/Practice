// InvFramesDataDlg.cpp : implementation file
//

#include "stdafx.h"
#include "InventoryRc.h"
#include "InvFramesDataDlg.h"
#include "FramesData.h"
#include "AuditTrail.h"
#include "CategorySelectDlg.h"
#include "GlobalFinancialUtils.h"

// CInvFramesDataDlg dialog
// (z.manning 2010-06-21 09:59) - PLID 39257 - Created

IMPLEMENT_DYNAMIC(CInvFramesDataDlg, CNxDialog)
// (s.dhole 2012-04-30 11:11) - PLID 466662 Added bIsNewFrame
CInvFramesDataDlg::CInvFramesDataDlg(const long nProductID,const BOOL bIsNewFrame, BOOL bEditable /*= TRUE*/, CWnd* pParent /*=NULL*/)
	: CNxDialog(CInvFramesDataDlg::IDD, pParent)
	, m_nProductID(nProductID),m_bIsNewFrame(bIsNewFrame) 
{
	// (j.gruber 2010-06-23 17:08) - PLID 39323
	m_bIsEditable = bEditable;
	m_nDefaultCategoryID = -1;
}

CInvFramesDataDlg::~CInvFramesDataDlg()
{
}

void CInvFramesDataDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FRAMES_STYLE, m_nxeditStyle);
	DDX_Control(pDX, IDC_FRAMES_COLOR, m_nxeditColor);
	DDX_Control(pDX, IDC_FRAMES_COLOR_CODE, m_nxeditColorCode);
	DDX_Control(pDX, IDC_FRAMES_LENS_COLOR, m_nxeditLensColor);
	DDX_Control(pDX, IDC_FRAMES_EYE, m_nxeditEye);
	DDX_Control(pDX, IDC_FRAMES_BRIDGE, m_nxeditBridge);
	DDX_Control(pDX, IDC_FRAMES_TEMPLE, m_nxeditTemple);
	DDX_Control(pDX, IDC_FRAMES_DBL, m_nxeditDBL);
	DDX_Control(pDX, IDC_FRAMES_A, m_nxeditA);
	DDX_Control(pDX, IDC_FRAMES_B, m_nxeditB);
	DDX_Control(pDX, IDC_FRAMES_ED, m_nxeditED);
	DDX_Control(pDX, IDC_FRAMES_CIRCUMFERENCE, m_nxeditCircumference);
	DDX_Control(pDX, IDC_FRAMES_EDANGLE, m_nxeditEDAngle);
	DDX_Control(pDX, IDC_FRAMES_FRONT_PRICE, m_nxeditFrontPrice);
	DDX_Control(pDX, IDC_FRAMES_HALF_TEMPLES_PRICE, m_nxeditHalfTemplesPrice);
	DDX_Control(pDX, IDC_FRAMES_TEMPLES_PRICE, m_nxeditTemplesPrice);
	DDX_Control(pDX, IDC_FRAMES_COMPLETE_PRICE, m_nxeditCompletePrice);
	DDX_Control(pDX, IDC_FRAMES_MANUFACTURER, m_nxeditManufacturer);
	DDX_Control(pDX, IDC_FRAMES_BRAND, m_nxeditBrand);
	DDX_Control(pDX, IDC_FRAMES_COLLECTION, m_nxeditCollection);
	DDX_Control(pDX, IDC_FRAMES_GENDER, m_nxeditGender);
	DDX_Control(pDX, IDC_FRAMES_AGE_GROUP, m_nxeditAgeGroup);
	DDX_Control(pDX, IDC_FRAMES_ACTIVE_STATUS, m_nxeditActiveStatus);
	DDX_Control(pDX, IDC_FRAMES_PRODUCT_GROUP_NAME, m_nxeditProductGroupName);
	DDX_Control(pDX, IDC_FRAMES_RIM_TYPE, m_nxeditRimType);
	DDX_Control(pDX, IDC_FRAMES_MATERIAL, m_nxeditMaterial);
	DDX_Control(pDX, IDC_FRAMES_FRAME_SHAPE, m_nxeditFrameShape);
	DDX_Control(pDX, IDC_FRAMES_COUNTRY, m_nxeditCountry);
	DDX_Control(pDX, IDC_FRAMES_SKU, m_nxeditSKU);
	DDX_Control(pDX, IDC_FRAMES_YEAR_INTRODUCED, m_nxeditYearIntroduced);	
	DDX_Control(pDX, IDOK, m_btnOK);	
	DDX_Control(pDX, IDC_FRAME_DATA_CANCEL_BTN, m_btnCancel);	
	DDX_Control(pDX, IDCANCEL, m_btnClose);	
	DDX_Control(pDX, IDC_BTN_FRAME_CATEGORY_PICKER, m_btnPickCategory);
	DDX_Control(pDX, IDC_BTN_FRAME_CATEGORY_REMOVE, m_btnRemoveCategory);
}


BEGIN_MESSAGE_MAP(CInvFramesDataDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CInvFramesDataDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_FRAME_DATA_CANCEL_BTN, &CInvFramesDataDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDCANCEL, &CInvFramesDataDlg::OnBnClickedClose)
	ON_EN_KILLFOCUS(IDC_FRAMES_FRONT_PRICE, &CInvFramesDataDlg::OnEnKillfocusFramesFrontPrice)
	ON_EN_KILLFOCUS(IDC_FRAMES_HALF_TEMPLES_PRICE, &CInvFramesDataDlg::OnEnKillfocusFramesHalfTemplesPrice)
	ON_EN_KILLFOCUS(IDC_FRAMES_TEMPLES_PRICE, &CInvFramesDataDlg::OnEnKillfocusFramesTemplesPrice)
	ON_EN_KILLFOCUS(IDC_FRAMES_COMPLETE_PRICE, &CInvFramesDataDlg::OnEnKillfocusFramesCompletePrice)
	ON_EN_KILLFOCUS(IDC_FRAMES_STYLE, &CInvFramesDataDlg::OnEnKillfocusFramesStyle)
	ON_EN_KILLFOCUS(IDC_FRAMES_COLLECTION, &CInvFramesDataDlg::OnEnKillfocusFramesCollection)
	ON_EN_KILLFOCUS(IDC_FRAMES_COLOR, &CInvFramesDataDlg::OnEnKillfocusFramesColor)
	ON_EN_KILLFOCUS(IDC_FRAMES_COLOR_CODE, &CInvFramesDataDlg::OnEnKillfocusFramesColorCode)
	ON_EN_KILLFOCUS(IDC_FRAMES_EYE, &CInvFramesDataDlg::OnEnKillfocusFramesEye)
	ON_EN_KILLFOCUS(IDC_FRAMES_BRIDGE, &CInvFramesDataDlg::OnEnKillfocusFramesBridge)
	ON_EN_KILLFOCUS(IDC_FRAMES_TEMPLE, &CInvFramesDataDlg::OnEnKillfocusFramesTemple)
	ON_BN_CLICKED(IDC_BTN_FRAME_CATEGORY_PICKER, &CInvFramesDataDlg::OnCategoryPicker)
	ON_BN_CLICKED(IDC_BTN_FRAME_CATEGORY_REMOVE, &CInvFramesDataDlg::OnCategoryRemove)
END_MESSAGE_MAP()


// CInvFramesDataDlg message handlers

// (j.gruber 2010-06-23 17:08) - PLID 39323
void CInvFramesDataDlg::SetControls() 
{	
	m_nxeditStyle.SetReadOnly(!m_bIsEditable);
	m_nxeditColor.SetReadOnly(!m_bIsEditable);
	m_nxeditColorCode.SetReadOnly(!m_bIsEditable);
	m_nxeditLensColor.SetReadOnly(!m_bIsEditable);
	m_nxeditEye.SetReadOnly(!m_bIsEditable);
	m_nxeditBridge.SetReadOnly(!m_bIsEditable);
	m_nxeditTemple.SetReadOnly(!m_bIsEditable);
	m_nxeditDBL.SetReadOnly(!m_bIsEditable);
	m_nxeditA.SetReadOnly(!m_bIsEditable);
	m_nxeditB.SetReadOnly(!m_bIsEditable);
	m_nxeditED.SetReadOnly(!m_bIsEditable);
	m_nxeditCircumference.SetReadOnly(!m_bIsEditable);
	m_nxeditEDAngle.SetReadOnly(!m_bIsEditable);
	m_nxeditFrontPrice.SetReadOnly(!m_bIsEditable);
	m_nxeditHalfTemplesPrice.SetReadOnly(!m_bIsEditable);
	m_nxeditTemplesPrice.SetReadOnly(!m_bIsEditable);
	m_nxeditCompletePrice.SetReadOnly(!m_bIsEditable);
	m_nxeditManufacturer.SetReadOnly(!m_bIsEditable);
	m_nxeditBrand.SetReadOnly(!m_bIsEditable);
	m_nxeditCollection.SetReadOnly(!m_bIsEditable);
	m_nxeditGender.SetReadOnly(!m_bIsEditable);
	m_nxeditAgeGroup.SetReadOnly(!m_bIsEditable);
	m_nxeditActiveStatus.SetReadOnly(!m_bIsEditable);
	m_nxeditProductGroupName.SetReadOnly(!m_bIsEditable);
	m_nxeditRimType.SetReadOnly(!m_bIsEditable);
	m_nxeditMaterial.SetReadOnly(!m_bIsEditable);
	m_nxeditFrameShape.SetReadOnly(!m_bIsEditable);
	m_nxeditCountry.SetReadOnly(!m_bIsEditable);
	m_nxeditSKU.SetReadOnly(!m_bIsEditable);
	m_nxeditYearIntroduced.SetReadOnly(!m_bIsEditable);

	if (m_bIsEditable) {
		GetDlgItem(IDC_FRAME_DATA_CANCEL_BTN)->ShowWindow(SW_SHOW);
		GetDlgItem(IDOK)->ShowWindow(SW_SHOW);
		GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);
	}
	else {
		GetDlgItem(IDC_FRAME_DATA_CANCEL_BTN)->ShowWindow(SW_HIDE);
		GetDlgItem(IDOK)->ShowWindow(SW_HIDE);
		GetDlgItem(IDCANCEL)->ShowWindow(SW_SHOW);
	}
	
	m_nxeditStyle.SetLimitText(50);
	m_nxeditColor.SetLimitText(50);
	m_nxeditColorCode.SetLimitText(20);
	m_nxeditLensColor.SetLimitText(50);
	m_nxeditEye.SetLimitText(3);
	m_nxeditBridge.SetLimitText(10);
	m_nxeditTemple.SetLimitText(10);
	m_nxeditDBL.SetLimitText(10);
	m_nxeditA.SetLimitText(5);
	m_nxeditB.SetLimitText(5);
	m_nxeditED.SetLimitText(5);
	m_nxeditCircumference.SetLimitText(6);
	m_nxeditEDAngle.SetLimitText(6);
//	m_nxeditFrontPrice.SetLimitText(!m_bIsEditable);
	//m_nxeditHalfTemplesPrice.SetLimitText(!m_bIsEditable);
	//m_nxeditTemplesPrice.SetLimitText(!m_bIsEditable);
	//m_nxeditCompletePrice.SetLimitText(!m_bIsEditable);
	m_nxeditManufacturer.SetLimitText(50);
	m_nxeditBrand.SetLimitText(50);
	m_nxeditCollection.SetLimitText(50);
	m_nxeditGender.SetLimitText(10);
	m_nxeditAgeGroup.SetLimitText(15);
	m_nxeditActiveStatus.SetLimitText(1);
	m_nxeditProductGroupName.SetLimitText(15);
	m_nxeditRimType.SetLimitText(25);
	m_nxeditMaterial.SetLimitText(50);
	m_nxeditFrameShape.SetLimitText(15);
	m_nxeditCountry.SetLimitText(30);
	m_nxeditSKU.SetLimitText(30);
	m_nxeditYearIntroduced.SetLimitText(4);

}

BOOL CInvFramesDataDlg::OnInitDialog()
{
	try
	{
		CNxDialog::OnInitDialog();

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnClose.AutoSet(NXB_CLOSE);
		// (s.dhole 2012-03-22 10:29) - PLID 46662 new Category selection 
		m_btnPickCategory.AutoSet(NXB_MODIFY);
		m_btnRemoveCategory.AutoSet(NXB_DELETE);
		// (s.dhole 2012-03-15 17:45) - PLID 46662 
		m_btnRemoveCategory.EnableWindow(FALSE);
		SetControls();
		// (s.dhole 2012-03-22 10:29) - PLID 46662 Hide Category if we edit product 
		if (m_bIsNewFrame==FALSE)
		{
			GetDlgItem(IDC_STATIC_CATEGORY)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_NEW_FRAME_CATEGORY)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_BTN_FRAME_CATEGORY_PICKER)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_BTN_FRAME_CATEGORY_REMOVE)->ShowWindow(SW_HIDE);
		}
		if (m_nProductID != -1){
			Load();
			SetDlgItemText(IDC_FRAMES_PRODUCT_NAME,m_strFinalProductName);
		}
		else{
		//nothing
		}
	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

#define SET_FRAMES_FIELD(strFieldName, nxedit) nxedit.SetWindowText(frames.GetOutputByDataField(strFieldName))

void CInvFramesDataDlg::Load()
{
	CFramesData frames;
	if(frames.LoadFromProductID(m_nProductID))
	{
		SET_FRAMES_FIELD("StyleName", m_nxeditStyle);
		SET_FRAMES_FIELD("ColorDescription", m_nxeditColor);
		SET_FRAMES_FIELD("ColorCode", m_nxeditColorCode);
		SET_FRAMES_FIELD("LensColor", m_nxeditLensColor);
		SET_FRAMES_FIELD("Eye", m_nxeditEye);
		SET_FRAMES_FIELD("Bridge", m_nxeditBridge);
		SET_FRAMES_FIELD("Temple", m_nxeditTemple);
		SET_FRAMES_FIELD("DBL", m_nxeditDBL);
		SET_FRAMES_FIELD("A", m_nxeditA);
		SET_FRAMES_FIELD("B", m_nxeditB);
		SET_FRAMES_FIELD("ED", m_nxeditED);
		SET_FRAMES_FIELD("Circumference", m_nxeditCircumference);
		SET_FRAMES_FIELD("EDAngle", m_nxeditEDAngle);
		SET_FRAMES_FIELD("FrontPrice", m_nxeditFrontPrice);
		SET_FRAMES_FIELD("HalfTemplesPrice", m_nxeditHalfTemplesPrice);
		SET_FRAMES_FIELD("TemplesPrice", m_nxeditTemplesPrice);
		SET_FRAMES_FIELD("CompletePrice", m_nxeditCompletePrice);
		SET_FRAMES_FIELD("ManufacturerName", m_nxeditManufacturer);
		SET_FRAMES_FIELD("BrandName", m_nxeditBrand);
		SET_FRAMES_FIELD("CollectionName", m_nxeditCollection);
		SET_FRAMES_FIELD("GenderType", m_nxeditGender);
		SET_FRAMES_FIELD("AgeGroup", m_nxeditAgeGroup);
		SET_FRAMES_FIELD("ActiveStatus", m_nxeditActiveStatus);
		SET_FRAMES_FIELD("ProductGroupName", m_nxeditProductGroupName);
		SET_FRAMES_FIELD("RimType", m_nxeditRimType);
		SET_FRAMES_FIELD("Material", m_nxeditMaterial);
		SET_FRAMES_FIELD("FrameShape", m_nxeditFrameShape);
		SET_FRAMES_FIELD("Country", m_nxeditCountry);
		SET_FRAMES_FIELD("SKU", m_nxeditSKU);
		SET_FRAMES_FIELD("YearIntroduced", m_nxeditYearIntroduced);
	}
	// (s.dhole 2012-04-30 10:26) - PLID 46662 show this message onlyon edit
	else  if (m_bIsNewFrame ==FALSE){
		MessageBox(FormatString("Frames data not found for product ID (%li).",m_nProductID), NULL, MB_OK|MB_ICONERROR);
		EndDialog(IDCANCEL);
	}
	else
	{
	// nothing
	}
}

// (s.dhole 2012-03-22 10:29) - PLID 46662 Build  frame insert sql statment
CString CInvFramesDataDlg::GetFrameDataInsertSql(IN BOOL bIsCatalog ,const CString strNewFrameDataID)
{
	CFramesData  framesData ;
	CString strFieldSql, strValueSql,strTemp;
	
	COleCurrency cyTemp = COleCurrency(0,0); 
	m_nxeditStyle.GetWindowTextA(strTemp);
	framesData.SetFieldByvarValue("StyleName", variant_t(strTemp));
	m_nxeditColor.GetWindowTextA(strTemp);
	framesData.SetFieldByvarValue ("ColorDescription", variant_t(strTemp));
	m_nxeditColorCode.GetWindowTextA(strTemp);
	framesData.SetFieldByvarValue("ColorCode", variant_t(strTemp));
	m_nxeditLensColor.GetWindowTextA(strTemp);
	framesData.SetFieldByvarValue("LensColor", variant_t(strTemp));
	m_nxeditEye.GetWindowTextA(strTemp);
	framesData.SetFieldByvarValue("Eye", variant_t(strTemp));
	m_nxeditBridge.GetWindowTextA(strTemp);
	framesData.SetFieldByvarValue("Bridge", variant_t(strTemp));
	m_nxeditTemple.GetWindowTextA(strTemp);
	framesData.SetFieldByvarValue("Temple", variant_t(strTemp));
	m_nxeditDBL.GetWindowTextA(strTemp);
	framesData.SetFieldByvarValue("DBL", variant_t(strTemp));
	m_nxeditA.GetWindowTextA(strTemp);
	framesData.SetFieldByvarValue("A", variant_t(strTemp));
	m_nxeditB.GetWindowTextA(strTemp);
	framesData.SetFieldByvarValue("B", variant_t(strTemp));
	m_nxeditED.GetWindowTextA(strTemp);
	framesData.SetFieldByvarValue("ED", variant_t(strTemp));
	m_nxeditCircumference.GetWindowTextA(strTemp);
	framesData.SetFieldByvarValue("Circumference", variant_t(strTemp));
	m_nxeditEDAngle.GetWindowTextA(strTemp);
	framesData.SetFieldByvarValue("EDAngle", variant_t(strTemp));
	m_nxeditFrontPrice.GetWindowTextA(strTemp);
	if(!strTemp.IsEmpty()){
		cyTemp.ParseCurrency(strTemp, 0);
	}
	else{
		cyTemp =COleCurrency(0,0);
	}
	framesData.SetFieldByvarValue("FrontPrice", variant_t(cyTemp));
	m_nxeditHalfTemplesPrice.GetWindowTextA(strTemp);
	
	if(!strTemp.IsEmpty()){
		cyTemp.ParseCurrency(strTemp, 0);
	}
	else{
		cyTemp =COleCurrency(0,0);
	}
	framesData.SetFieldByvarValue("HalfTemplesPrice", variant_t(cyTemp));

	m_nxeditTemplesPrice.GetWindowTextA(strTemp);
	if(!strTemp.IsEmpty()){
		cyTemp.ParseCurrency(strTemp, 0);
	}
	else{
		cyTemp =COleCurrency(0,0);
	}

	framesData.SetFieldByvarValue("TemplesPrice", variant_t(cyTemp));

	m_nxeditCompletePrice.GetWindowTextA(strTemp);
	if(!strTemp.IsEmpty()){
		cyTemp.ParseCurrency(strTemp, 0);
	}
	else{
		cyTemp =COleCurrency(0,0);
	}

	framesData.SetFieldByvarValue("CompletePrice", variant_t(cyTemp));
	m_nxeditManufacturer.GetWindowTextA(strTemp);
	framesData.SetFieldByvarValue("ManufacturerName", variant_t(strTemp));
	m_nxeditBrand.GetWindowTextA(strTemp);
	framesData.SetFieldByvarValue("BrandName", variant_t(strTemp));
	m_nxeditCollection.GetWindowTextA(strTemp);
	framesData.SetFieldByvarValue("CollectionName", variant_t(strTemp));
	m_nxeditGender.GetWindowTextA(strTemp);
	framesData.SetFieldByvarValue("GenderType", variant_t(strTemp));
	m_nxeditAgeGroup.GetWindowTextA(strTemp);
	framesData.SetFieldByvarValue("AgeGroup", variant_t(strTemp));
	m_nxeditActiveStatus.GetWindowTextA(strTemp);
	framesData.SetFieldByvarValue("ActiveStatus", variant_t(strTemp));
	m_nxeditProductGroupName.GetWindowTextA(strTemp);
	framesData.SetFieldByvarValue("ProductGroupName", variant_t(strTemp));
	m_nxeditRimType.GetWindowTextA(strTemp);
	framesData.SetFieldByvarValue("RimType", variant_t(strTemp));
	m_nxeditMaterial.GetWindowTextA(strTemp);
	framesData.SetFieldByvarValue("Material", variant_t(strTemp));
	m_nxeditFrameShape.GetWindowTextA(strTemp);
	framesData.SetFieldByvarValue("FrameShape", variant_t(strTemp));
	m_nxeditCountry.GetWindowTextA(strTemp);
	framesData.SetFieldByvarValue("Country", variant_t(strTemp));
	m_nxeditSKU.GetWindowTextA(strTemp);
	framesData.SetFieldByvarValue("SKU", variant_t(strTemp));
	m_nxeditYearIntroduced.GetWindowTextA(strTemp);
	framesData.SetFieldByvarValue("YearIntroduced", variant_t(strTemp));
	framesData.SetFieldByvarValue("StyleNew", g_cvarFalse );
	framesData.SetFieldByvarValue("ChangedPrice", g_cvarFalse);
	// Now generate sql field and value and we have to add FPC id which is unique 
	framesData.GetFieldsAndValuesList(bIsCatalog,"'NXT' + Cast( "+ strNewFrameDataID + " AS VARCHAR(50))" , strFieldSql, strValueSql);
	return FormatString("INSERT INTO FramesDataT (%s) VALUES (%s  );", strFieldSql, strValueSql);
}


// (j.gruber 2010-06-23 17:08) - PLID 39323
void CInvFramesDataDlg::Save()
{

	CFramesData frames;
	CString strTemp;
	if(frames.LoadFromProductID(m_nProductID))
	{
		m_nxeditStyle.GetWindowTextA(strTemp);
		frames.SetFieldByDataField("StyleName", strTemp);
		m_nxeditColor.GetWindowTextA(strTemp);
		frames.SetFieldByDataField("ColorDescription", strTemp);
		m_nxeditColorCode.GetWindowTextA(strTemp);
		frames.SetFieldByDataField("ColorCode", strTemp);
		m_nxeditLensColor.GetWindowTextA(strTemp);
		frames.SetFieldByDataField("LensColor", strTemp);
		m_nxeditEye.GetWindowTextA(strTemp);
		frames.SetFieldByDataField("Eye", strTemp);
		m_nxeditBridge.GetWindowTextA(strTemp);
		frames.SetFieldByDataField("Bridge", strTemp);
		m_nxeditTemple.GetWindowTextA(strTemp);
		frames.SetFieldByDataField("Temple", strTemp);
		m_nxeditDBL.GetWindowTextA(strTemp);
		frames.SetFieldByDataField("DBL", strTemp);
		m_nxeditA.GetWindowTextA(strTemp);
		frames.SetFieldByDataField("A", strTemp);
		m_nxeditB.GetWindowTextA(strTemp);
		frames.SetFieldByDataField("B", strTemp);
		m_nxeditED.GetWindowTextA(strTemp);
		frames.SetFieldByDataField("ED", strTemp);
		m_nxeditCircumference.GetWindowTextA(strTemp);
		frames.SetFieldByDataField("Circumference", strTemp);
		m_nxeditEDAngle.GetWindowTextA(strTemp);
		frames.SetFieldByDataField("EDAngle", strTemp);
		
		m_nxeditFrontPrice.GetWindowTextA(strTemp);
		if (atoi(strTemp) < 0) {
			strTemp = "0";
		}
		frames.SetFieldByDataField("FrontPrice", strTemp);
		long a =atoi(strTemp) ;
		m_nxeditHalfTemplesPrice.GetWindowTextA(strTemp);
		if (atoi(strTemp) < 0) {
			strTemp = "0";
		}
		frames.SetFieldByDataField("HalfTemplesPrice", strTemp);
		
		m_nxeditTemplesPrice.GetWindowTextA(strTemp);
		if (atoi(strTemp) < 0) {
			strTemp = "0";
		}
		frames.SetFieldByDataField("TemplesPrice", strTemp);
		
		m_nxeditCompletePrice.GetWindowTextA(strTemp);
		if (atoi(strTemp) < 0) {
			strTemp = "0";
		}
		frames.SetFieldByDataField("CompletePrice", strTemp);
		
		m_nxeditManufacturer.GetWindowTextA(strTemp);
		frames.SetFieldByDataField("ManufacturerName", strTemp);
		m_nxeditBrand.GetWindowTextA(strTemp);
		frames.SetFieldByDataField("BrandName", strTemp);
		m_nxeditCollection.GetWindowTextA(strTemp);
		frames.SetFieldByDataField("CollectionName", strTemp);
		m_nxeditGender.GetWindowTextA(strTemp);
		frames.SetFieldByDataField("GenderType", strTemp);
		m_nxeditAgeGroup.GetWindowTextA(strTemp);
		frames.SetFieldByDataField("AgeGroup", strTemp);
		m_nxeditActiveStatus.GetWindowTextA(strTemp);
		frames.SetFieldByDataField("ActiveStatus", strTemp);
		m_nxeditProductGroupName.GetWindowTextA(strTemp);
		frames.SetFieldByDataField("ProductGroupName", strTemp);
		m_nxeditRimType.GetWindowTextA(strTemp);
		frames.SetFieldByDataField("RimType", strTemp);
		m_nxeditMaterial.GetWindowTextA(strTemp);
		frames.SetFieldByDataField("Material", strTemp);
		m_nxeditFrameShape.GetWindowTextA(strTemp);
		frames.SetFieldByDataField("FrameShape", strTemp);
		m_nxeditCountry.GetWindowTextA(strTemp);
		frames.SetFieldByDataField("Country", strTemp);
		m_nxeditSKU.GetWindowTextA(strTemp);
		frames.SetFieldByDataField("SKU", strTemp);
		m_nxeditYearIntroduced.GetWindowTextA(strTemp);
		frames.SetFieldByDataField("YearIntroduced", strTemp);

		frames.ApplyFieldsToData(m_nProductID);
	}

}

// (j.gruber 2010-06-23 17:08) - PLID 39323
void CInvFramesDataDlg::OnBnClickedOk()
{
	try {
		if (m_nProductID != -1) {
			Save();
		}
		else {
			// (s.dhole 2012-03-23 14:31) - PLID 46662	
			CString strProductName ;
			GetDlgItemText(IDC_FRAMES_PRODUCT_NAME  ,strProductName );
			//Check product name if we are adding new one

			// Will prmpt only for "CollectionName , StyleName, ColorDescription, ColorCode"  not to  Eye-Bridge-Temple
			if (strProductName== "" && m_nProductID == -1 && m_bIsNewFrame == TRUE){	
				AfxMessageBox ("You must enter one of the following item.\r\nCollection Name\r\nStyle Name\r\nColor Description\r\nColor Code");
				return;
			}
			strProductName  = GetFrameName();
			SetDlgItemText(IDC_FRAMES_PRODUCT_NAME  ,strProductName );
			m_nProductID = SaveNewFrame();
		}
		if (m_nProductID >-1)
			CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);

}

// (s.dhole 2012-03-15 17:45) - PLID 46662 save new frame data
// this function does not have parameterized query skice it depend on existing function
long CInvFramesDataDlg::SaveNewFrame()
{
	long nNewProductID = -1;
	try {
		CString strSqlBatch = BeginSqlBatch();
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nNewProductID INT");
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nNewFramesDataID INT");
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nSupplierID INT"); 
		CString strBaseSql = strSqlBatch;
		CString strProductName ;
		GetDlgItemText(IDC_FRAMES_PRODUCT_NAME  ,strProductName );
		AddStatementToSqlBatch(strSqlBatch, "SET @nNewProductID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM ServiceT)");
		long nUserID = GetCurrentUserID();
		// Frames are not taxabled
		AddCreateFrameProductSqlToBatch(strSqlBatch, "@nNewProductID", strProductName, TRUE, FALSE, FALSE,  nUserID);
		AddStatementToSqlBatch(strSqlBatch, "SET @nNewFramesDataID = (Select (ISNull((Select max(ID)   From FramesDataT  ),0) +1 ))");
		AddStatementToSqlBatch(strSqlBatch, GetFrameDataInsertSql(FALSE ,"@nNewFramesDataID"));
		AddStatementToSqlBatch(strSqlBatch, "SET @nNewFramesDataID = (SELECT CONVERT(int, SCOPE_IDENTITY()))");
		AddStatementToSqlBatch(strSqlBatch, 
			"UPDATE ProductT \r\n"
			"SET FramesDataID = @nNewFramesDataID \r\n"
			"WHERE ProductT.ID = @nNewProductID ");
		ADODB::_RecordsetPtr prsCreateProduct = CreateRecordsetStd(
		"SET XACT_ABORT ON \r\n"
		"SET NOCOUNT ON \r\n"
		"BEGIN TRAN \r\n" + strSqlBatch + "\r\nCOMMIT TRAN \r\n"
		"SET NOCOUNT OFF \r\n"
		"SELECT ID AS NewProductID \r\n"
		"FROM ServiceT \r\n"
		"WHERE ServiceT.ID = @nNewProductID \r\n");
		
		if(!prsCreateProduct->eof) {
			nNewProductID = AdoFldLong(prsCreateProduct->GetFields(), "NewProductID");
			m_strFinalProductName = strProductName;
			long nAuditID = -1;
			nAuditID = BeginNewAuditEvent();
			if(nAuditID != -1){
				AuditEvent(-1, "", nAuditID, aeiProductCreate, nNewProductID, "", strProductName, aepMedium, aetCreated);
			}

			// (j.jones 2015-03-03 14:40) - PLID 65111 - save the product categories via the API, if they picked any
			if (m_aryCategoryIDs.size() > 0) {
				std::vector<long> aryServiceIDs;
				aryServiceIDs.push_back(nNewProductID);
				UpdateServiceCategories(aryServiceIDs, m_aryCategoryIDs, m_nDefaultCategoryID, true);
			}

			// (b.eyers 2015-07-10) - PLID 24060 - newly created frames needs to send a tablechecker
			CClient::RefreshTable(NetUtils::Products, nNewProductID);

			return nNewProductID;	
		}
		else {
			ThrowNxException("InvFramesDataDlg::SaveNewFrame - eof error when trying to get new product ID");
		}
		return nNewProductID;	
	}NxCatchAll(__FUNCTION__);
	return nNewProductID;	
}

// (s.dhole 2012-03-15 17:45) - PLID 46662 
// (j.jones 2015-03-03 14:40) - PLID 65111 - categories are no longer part of this function
void  CInvFramesDataDlg::AddCreateFrameProductSqlToBatch(IN OUT CString &strSqlBatch, const CString strProductID, CString strName, BOOL bBillable, BOOL bTaxable1, BOOL bTaxable2, long nUserID)
{
	AddStatementToSqlBatch(strSqlBatch, 
		"INSERT INTO ServiceT (ID, Name, Taxable1, Taxable2) SELECT %s, \'%s\', %i, %i"
		, strProductID, _Q(strName), bTaxable1 ? 1 : 0, bTaxable2 ? 1 : 0);
	AddStatementToSqlBatch(strSqlBatch, "INSERT INTO ProductT (ID) SELECT %s", strProductID);

	AddStatementToSqlBatch(strSqlBatch, 
		"INSERT INTO ProductResponsibilityT (ProductID, UserID, LocationID) \r\n"
		"SELECT %s, %i, ID FROM LocationsT WHERE Managed = 1 AND Active = 1 AND TypeID = 1"
		, strProductID, nUserID, GetCurrentLocation());

	//We decided to make the Billable and Trackable statuses apply to all (managed) locations in the new item dlg
	//and users can change it after the save.
	AddStatementToSqlBatch(strSqlBatch, 
		"INSERT INTO ProductLocationInfoT (ProductID, LocationID, TrackableStatus, Billable) \r\n"
		"SELECT %s, ID, %li, %li FROM LocationsT WHERE Managed = 1 AND TypeID = 1"
		, strProductID, 2 /*tsTrackQuantity*/, bBillable ? 1 : 0);	

	// (j.gruber 2012-12-04 11:42) - PLID 48566
	AddStatementToSqlBatch(strSqlBatch, "INSERT INTO ServiceLocationInfoT (ServiceID, LocationID) \r\n"
							"SELECT %s, ID FROM LocationsT WHERE Managed = 1 "
						, strProductID);	
}
// (s.dhole 2012-03-15 17:45) - PLID 46662  construct  Product name
// Product name CollectionName , StyleName, ColorDescription, ColorCode, Eye-Bridge-Temple
CString  CInvFramesDataDlg::GetFrameName()
{
	CString strProductName, str;
	GetDlgItemText(IDC_FRAMES_COLLECTION  ,str);
	if(!str.IsEmpty()) {
		strProductName += str + ", ";
	}
	else {
		//nothing
	}
	
	GetDlgItemText(IDC_FRAMES_STYLE ,str);
	if(!str.IsEmpty()) {
		strProductName += str + ", ";
	}else {
		//nothing
	}

	GetDlgItemText(IDC_FRAMES_COLOR ,str);
	if(!str.IsEmpty()) {
		strProductName += str + ", ";
	}else {
		//nothing
	}

	GetDlgItemText(IDC_FRAMES_COLOR_CODE ,str);
	if(!str.IsEmpty()) {
		strProductName += str + ", ";
	}else {
		//nothing
	}
	CString strSize1, strSize2 , strSize3;
	GetDlgItemText(IDC_FRAMES_EYE ,strSize1);
	GetDlgItemText(IDC_FRAMES_BRIDGE ,strSize2);
	GetDlgItemText(IDC_FRAMES_TEMPLE ,strSize3);
	// (s.dhole 2012-06-06 17:45) - PLID 46662 Fis format string
	CString strSize = FormatString("%s-%s-%s", strSize1, strSize2 , strSize3);
	if(strSize.SpanIncluding("-") == strSize) {
		str  = "";
	}
	else {
		str   = strSize;
	}
	if(!str.IsEmpty()) {
		strProductName += str + ", ";
	}
	strProductName.TrimRight(", ");
	return strProductName;
}
// (j.gruber 2010-06-23 17:08) - PLID 39323
void CInvFramesDataDlg::OnBnClickedCancel()
{
	try {
		CNxDialog::OnCancel();		
	}NxCatchAll(__FUNCTION__);
}

void CInvFramesDataDlg::OnBnClickedClose()
{
	try {
		CNxDialog::OnCancel();		
	}NxCatchAll(__FUNCTION__);
}

void CInvFramesDataDlg::OnEnKillfocusFramesFrontPrice()
{
	try {
		CString strCy;
		GetDlgItemText(IDC_FRAMES_FRONT_PRICE, strCy);
		COleCurrency cy = ParseCurrencyFromInterface(strCy);
		if (cy.GetStatus() == COleCurrency::invalid) {
			cy = COleCurrency(0,0);
		}

		SetDlgItemText(IDC_FRAMES_FRONT_PRICE, FormatCurrencyForInterface(cy));
	}NxCatchAll(__FUNCTION__);
}

void CInvFramesDataDlg::OnEnKillfocusFramesHalfTemplesPrice()
{
	try {
		CString strCy;
		GetDlgItemText(IDC_FRAMES_HALF_TEMPLES_PRICE, strCy);
		COleCurrency cy = ParseCurrencyFromInterface(strCy);
		if (cy.GetStatus() == COleCurrency::invalid) {
			cy = COleCurrency(0,0);
		}
		SetDlgItemText(IDC_FRAMES_HALF_TEMPLES_PRICE, FormatCurrencyForInterface(cy));
	}NxCatchAll(__FUNCTION__);
}

void CInvFramesDataDlg::OnEnKillfocusFramesTemplesPrice()
{
	try {
		CString strCy;
		GetDlgItemText(IDC_FRAMES_TEMPLES_PRICE, strCy);
		COleCurrency cy = ParseCurrencyFromInterface(strCy);
		if (cy.GetStatus() == COleCurrency::invalid) {
			cy = COleCurrency(0,0);
		}
		SetDlgItemText(IDC_FRAMES_TEMPLES_PRICE, FormatCurrencyForInterface(cy));
	}NxCatchAll(__FUNCTION__);
}

void CInvFramesDataDlg::OnEnKillfocusFramesCompletePrice()
{
	try {
		CString strCy;
		GetDlgItemText(IDC_FRAMES_COMPLETE_PRICE, strCy);
		COleCurrency cy = ParseCurrencyFromInterface(strCy);
		if (cy.GetStatus() == COleCurrency::invalid) {
			cy = COleCurrency(0,0);
		}
		SetDlgItemText(IDC_FRAMES_COMPLETE_PRICE, FormatCurrencyForInterface(cy));
	}NxCatchAll(__FUNCTION__);
}
// (s.dhole 2012-03-15 17:45) - PLID 46662 
//s.dhole 2012-06-06 PLID 46662  change as style 
void CInvFramesDataDlg::OnEnKillfocusFramesStyle()
{
try {
		if  (m_nProductID<0 && m_bIsNewFrame ==TRUE) {
			CString  strProductName  = GetFrameName();
			SetDlgItemText(IDC_FRAMES_PRODUCT_NAME  ,strProductName );
		}
		else{
		//nothing
		}
	}NxCatchAll(__FUNCTION__);
}
// (s.dhole 2012-03-15 17:45) - PLID 46662 
void CInvFramesDataDlg::OnEnKillfocusFramesCollection()
{
	try {
		if  (m_nProductID<0 && m_bIsNewFrame ==TRUE) 
		{
			CString  strProductName  = GetFrameName();
			SetDlgItemText(IDC_FRAMES_PRODUCT_NAME  ,strProductName );
		}
		else{
		//nothing
		}
	}NxCatchAll(__FUNCTION__);
}

// (s.dhole 2012-03-15 17:45) - PLID 46662 
void CInvFramesDataDlg::OnEnKillfocusFramesColor()
{
	try {
		if  (m_nProductID<0 && m_bIsNewFrame ==TRUE){
			CString  strProductName  = GetFrameName();
			SetDlgItemText(IDC_FRAMES_PRODUCT_NAME  ,strProductName );
		}
		else{
		//nothing
		}
	}NxCatchAll(__FUNCTION__);
}
// (s.dhole 2012-03-15 17:45) - PLID 46662 
void CInvFramesDataDlg::OnEnKillfocusFramesColorCode()
{
	try {
		if  (m_nProductID<0 && m_bIsNewFrame ==TRUE){
			CString  strProductName  = GetFrameName();
			SetDlgItemText(IDC_FRAMES_PRODUCT_NAME  ,strProductName );
		}
		else{
		//nothing
		}
	}NxCatchAll(__FUNCTION__);
}
// (s.dhole 2012-03-15 17:45) - PLID 46662 
void CInvFramesDataDlg::OnEnKillfocusFramesEye()
{
	try {
		if  (m_nProductID<0 && m_bIsNewFrame ==TRUE  ){
			CString  strProductName  = GetFrameName();
			SetDlgItemText(IDC_FRAMES_PRODUCT_NAME  ,strProductName );
		}else{
		//nothing
		}
	}NxCatchAll(__FUNCTION__);
}
// (s.dhole 2012-03-15 17:45) - PLID 46662 
void CInvFramesDataDlg::OnEnKillfocusFramesBridge()
{
	try {
		if  (m_nProductID<0 && m_bIsNewFrame ==TRUE){
			CString  strProductName  = GetFrameName();
			SetDlgItemText(IDC_FRAMES_PRODUCT_NAME  ,strProductName );
		}
	}NxCatchAll(__FUNCTION__);
}
// (s.dhole 2012-03-15 17:45) - PLID 46662 
void CInvFramesDataDlg::OnEnKillfocusFramesTemple()
{
	try {
		if  (m_nProductID<0 && m_bIsNewFrame ==TRUE) {
			CString  strProductName  = GetFrameName();
			SetDlgItemText(IDC_FRAMES_PRODUCT_NAME  ,strProductName );
		}
		else{
		//nothing
		}
	}NxCatchAll(__FUNCTION__);
}

// (s.dhole 2012-03-15 17:45) - PLID 46662 
void CInvFramesDataDlg::OnCategoryPicker()
{
	//
	// You may now choose a category for the new
	// inventory item here
	//
	try {

		// (j.jones 2015-03-03 16:40) - PLID 65111 - products can now have multiple categories

		// (j.jones 2015-02-27 16:22) - PLID 64962 - added bAllowMultiSelect, true for frames
		// (j.jones 2015-03-02 15:36) - PLID 64970 - added strItemType
		CCategorySelectDlg dlg(this, true, "frame");

		// (j.jones 2015-03-02 08:55) - PLID 64962 - this dialog supports multiple categories,
		// but frames do not
		if (m_aryCategoryIDs.size() > 0) {
			dlg.m_aryInitialCategoryIDs.insert(dlg.m_aryInitialCategoryIDs.end(), m_aryCategoryIDs.begin(), m_aryCategoryIDs.end());
			dlg.m_nInitialDefaultCategoryID = m_nDefaultCategoryID;
		}

		// (j.jones 2015-03-02 10:18) - PLID 64962 - this now is just an OK/Cancel dialog
		if (IDOK == dlg.DoModal()) {	// Greater than zero means the user picked a valid category

			// (j.jones 2015-03-03 16:40) - PLID 65111 - products can now have multiple categories
			m_aryCategoryIDs.clear();
			m_aryCategoryIDs.insert(m_aryCategoryIDs.end(), dlg.m_arySelectedCategoryIDs.begin(), dlg.m_arySelectedCategoryIDs.end());
			m_nDefaultCategoryID = dlg.m_nSelectedDefaultCategoryID;

			CString strCategoryNames;
			LoadServiceCategories(m_aryCategoryIDs, m_nDefaultCategoryID, strCategoryNames);

			SetDlgItemText(IDC_NEW_FRAME_CATEGORY, strCategoryNames);
			m_btnRemoveCategory.EnableWindow(m_aryCategoryIDs.size() > 0);
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (s.dhole 2012-03-15 17:45) - PLID 46662 
void CInvFramesDataDlg::OnCategoryRemove()
{
	try {
		
		// (j.jones 2015-03-03 16:46) - PLID 65111 - products can now have multiple categories
		m_aryCategoryIDs.clear();
		m_nDefaultCategoryID = -1;
		SetDlgItemText(IDC_NEW_FRAME_CATEGORY, "");
		m_btnRemoveCategory.EnableWindow(FALSE);
	}
	NxCatchAll(__FUNCTION__);
}
