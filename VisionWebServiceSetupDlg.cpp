// VisionWebServiceSetupDlg.cpp : implementation file

#include "stdafx.h"
#include "Practice.h"
#include "VisionWebServiceSetupDlg.h"

using namespace ADODB;
using namespace NXDATALIST2Lib;

// (s.dhole 2010-10-15 14:57) - PLID 41281 visionWeb Web service Url configration and Ref id setup
// CVisionWebServiceSetupDlg dialog

IMPLEMENT_DYNAMIC(CVisionWebServiceSetupDlg, CNxDialog)

CVisionWebServiceSetupDlg::CVisionWebServiceSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CVisionWebServiceSetupDlg::IDD, pParent)
{

}



CVisionWebServiceSetupDlg::~CVisionWebServiceSetupDlg()
{
}

void CVisionWebServiceSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_VSIONWEB_SETUP_OK, m_SaveSetupBtn);
	DDX_Control(pDX, IDC_BTN_VSIONWEB_SETUP_CANCEL , m_CancelBtn);
}


BEGIN_MESSAGE_MAP(CVisionWebServiceSetupDlg, CNxDialog)
	
	ON_BN_CLICKED(IDC_BTN_VSIONWEB_SETUP_OK, &CVisionWebServiceSetupDlg::OnBtnClickedSave)
	ON_BN_CLICKED(IDC_BTN_VSIONWEB_SETUP_CANCEL, &CVisionWebServiceSetupDlg::OnBtnClickedCancel)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CVisionWebServiceSetupDlg, CNxDialog)
ON_EVENT(CVisionWebServiceSetupDlg, IDC_VISIONWEB_SERVICE_URL_LIST, 9, CVisionWebServiceSetupDlg::EditingFinishingVisionwebServiceUrlList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
END_EVENTSINK_MAP()


// CVisionWebServiceSetupDlg message handlers
BOOL CVisionWebServiceSetupDlg::OnInitDialog()
{
	try{
		CNxDialog::OnInitDialog();
		m_SaveSetupBtn.AutoSet(NXB_OK);
		m_CancelBtn.AutoSet(NXB_CANCEL);
		m_VisionWebSetupUrlList= BindNxDataList2Ctrl(IDC_VISIONWEB_SERVICE_URL_LIST,false);
		g_propManager.BulkCache("VisionWebSetup",propbitText, FormatString(
		"(Username = '<None>') AND ("
		"Name = '%s' OR "
		"Name = '%s' OR "
		"Name = '%s' OR "
		"Name = '%s' OR "
		"Name = '%s' OR "
		"Name = '%s' OR "
		"Name = '%s' OR "
		"Name = '%s' OR "
		"Name = '%s' " 
		")", 
		VISIONWEBSERVICE_USER_ACCOUNT_URL,VISIONWEBSERVICE_CHANGE_PASSWORD_URL,VISIONWEBSERVICE_ORDER_URL,
		VISIONWEBSERVICE_ORDER_ERROR_DETAIL_URL,VISIONWEBSERVICE_ORDER_STATUS_URL,VISIONWEBSERVICE_CATALOG_URL,
		VISIONWEBSERVICE_REFID,VISIONWEBSERVICE_ERROR_URL,VISIONWEBSERVICE_SUPPLIER_URL));

		AddRowToList(VISIONWEBSERVICE_USER_ACCOUNT_URL);
		AddRowToList(VISIONWEBSERVICE_CHANGE_PASSWORD_URL);
		AddRowToList(VISIONWEBSERVICE_ORDER_URL);
		AddRowToList(VISIONWEBSERVICE_ORDER_ERROR_DETAIL_URL);
		AddRowToList(VISIONWEBSERVICE_ORDER_STATUS_URL);
		AddRowToList(VISIONWEBSERVICE_CATALOG_URL);
		AddRowToList(VISIONWEBSERVICE_REFID);
		AddRowToList(VISIONWEBSERVICE_ERROR_URL);
		AddRowToList(VISIONWEBSERVICE_SUPPLIER_URL);
	}NxCatchAll("Error in CVisionWebServiceSetupDlg::OnInitDialog");
	return FALSE;
}

// adding each row into datalist
void CVisionWebServiceSetupDlg::AddRowToList(LPCTSTR urlType)
{ 
	try{
		IRowSettingsPtr pNewRow = m_VisionWebSetupUrlList->GetNewRow();
		pNewRow->PutValue(vwURLType, _variant_t(urlType) );
		pNewRow->PutValue(vwUrl ,_variant_t( GetRemotePropertyText(urlType, "", 0, "<None>", true)));
		m_VisionWebSetupUrlList->AddRowAtEnd(pNewRow,NULL);
	}NxCatchAll("Error in CVisionWebServiceSetupDlg::AddRowToList");
}

// this is predefine data, non of data lenght is more than 255 char, not adding any prompt  
void CVisionWebServiceSetupDlg::EditingFinishingVisionwebServiceUrlList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try{
		IRowSettingsPtr pRow(lpRow);
		CString  strUrlInfo =VarString(*pvarNewValue,"");
		if (strUrlInfo.GetLength() !=255) 
		{
			strUrlInfo.Left(255); 	 
		}
	}NxCatchAll("Error in CVisionWebServiceSetupDlg::EditingFinishingVisionwebServiceUrlList");
}


void CVisionWebServiceSetupDlg::OnBtnClickedSave()
{
	try{
		Save();
		CNxDialog::OnOK();
	}NxCatchAll("Error in CVisionWebServiceSetupDlg::OnBtnClickedSave");
}

void CVisionWebServiceSetupDlg::OnBtnClickedCancel()
{
	try{
		CNxDialog::OnCancel();  
	}NxCatchAll("Error in CVisionWebServiceSetupDlg::OnBtnClickedCancel");
}

void CVisionWebServiceSetupDlg::Save()
{
	try{
		CString strField,strValue;
		IRowSettingsPtr pRow = m_VisionWebSetupUrlList->GetFirstRow();
		while (pRow )	
		{
			strField =VarString(pRow->GetValue(vwURLType),"");   
			strValue =VarString(pRow->GetValue(vwUrl),"");   
			strValue.Left(255); 
			// has to have field name
			if (strField!="") 
				SetRemotePropertyText(strField,strValue,0,"<None>" );
			pRow = pRow->GetNextRow(); 
		}
	}NxCatchAll("Error in CVisionWebServiceSetupDlg::Save");
}
