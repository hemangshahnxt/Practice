// NDCDefSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "NDCDefSetupDlg.h"
#include "AuditTrail.h"

// (j.dinatale 2012-06-12 11:07) - PLID 32795 - Created

namespace NDCDefUnitTypeList{
	enum NDCUnitTypeListCols{
		ID = 0,
		Name = 1,
	};
};


// CNDCDefSetupDlg dialog

IMPLEMENT_DYNAMIC(CNDCDefSetupDlg, CDialog)

CNDCDefSetupDlg::CNDCDefSetupDlg(long nServiceID, bool bFromInventoryMod /*= false*/, CWnd* pParent /*=NULL*/)
	: CNxDialog(CNDCDefSetupDlg::IDD, pParent), m_nServiceID(nServiceID), m_bFromInvModule(bFromInventoryMod)
{

}

CNDCDefSetupDlg::~CNDCDefSetupDlg()
{
}

void CNDCDefSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_NDC_DEF_SETUP_BACKCOLOR, m_nxcolorBackground);
}


BEGIN_MESSAGE_MAP(CNDCDefSetupDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CNDCDefSetupDlg::OnBnClickedOk)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CNDCDefSetupDlg, CNxDialog)
	ON_EVENT(CNDCDefSetupDlg, IDC_NDC_DEF_UNIT_TYPE, 1, CNDCDefSetupDlg::SelChangingNdcDefUnitType, VTS_DISPATCH VTS_PDISPATCH)
END_EVENTSINK_MAP()

// CNDCDefSetupDlg message handlers
BOOL CNDCDefSetupDlg::OnInitDialog() 
{
	try{
		CNxDialog::OnInitDialog();

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		if(m_bFromInvModule){
			m_nxcolorBackground.SetColor(GetNxColor(GNC_INVENTORY, 0));
		}else{
			m_nxcolorBackground.SetColor(GetNxColor(GNC_ADMIN, 0));
		}

		m_dlUnitType = BindNxDataList2Ctrl(IDC_NDC_DEF_UNIT_TYPE, false);
		SetupUnitTypeList();
		RestrictFieldLengths();

		LoadInfo();
	}NxCatchAll(__FUNCTION__);

	return FALSE;
}

void CNDCDefSetupDlg::SetupUnitTypeList()
{
	if(m_dlUnitType){
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlUnitType->GetNewRow();
		if(pRow){
			pRow->PutValue(NDCDefUnitTypeList::ID, _bstr_t(""));
			pRow->PutValue(NDCDefUnitTypeList::Name, _bstr_t("<None>"));
			m_dlUnitType->AddRowAtEnd(pRow, NULL);

			m_dlUnitType->CurSel = pRow;
		}

		pRow = m_dlUnitType->GetNewRow();
		if(pRow){
			pRow->PutValue(NDCDefUnitTypeList::ID, _bstr_t("F2"));
			pRow->PutValue(NDCDefUnitTypeList::Name, _bstr_t("F2 - International Unit"));
			m_dlUnitType->AddRowAtEnd(pRow, NULL);
		}

		pRow = m_dlUnitType->GetNewRow();
		if(pRow){
			pRow->PutValue(NDCDefUnitTypeList::ID, _bstr_t("GR"));
			pRow->PutValue(NDCDefUnitTypeList::Name, _bstr_t("GR - Gram"));
			m_dlUnitType->AddRowAtEnd(pRow, NULL);
		}

		pRow = m_dlUnitType->GetNewRow();
		if(pRow){
			pRow->PutValue(NDCDefUnitTypeList::ID, _bstr_t("ME"));
			pRow->PutValue(NDCDefUnitTypeList::Name, _bstr_t("ME - Milligram"));
			m_dlUnitType->AddRowAtEnd(pRow, NULL);
		}

		pRow = m_dlUnitType->GetNewRow();
		if(pRow){
			pRow->PutValue(NDCDefUnitTypeList::ID, _bstr_t("ML"));
			pRow->PutValue(NDCDefUnitTypeList::Name, _bstr_t("ML - Milliliter"));
			m_dlUnitType->AddRowAtEnd(pRow, NULL);
		}

		pRow = m_dlUnitType->GetNewRow();
		if(pRow){
			pRow->PutValue(NDCDefUnitTypeList::ID, _bstr_t("UN"));
			pRow->PutValue(NDCDefUnitTypeList::Name, _bstr_t("UN - Unit"));
			m_dlUnitType->AddRowAtEnd(pRow, NULL);
		}
	}
}

void CNDCDefSetupDlg::RestrictFieldLengths()
{
	((CEdit *)GetDlgItem(IDC_NDC_DEF_CODE))->SetLimitText(50);
	((CEdit *)GetDlgItem(IDC_NDC_DEF_QUANTITY))->SetLimitText(15);
	((CEdit *)GetDlgItem(IDC_NDC_DEF_UNIT_PRICE))->SetLimitText(15);
}

void CNDCDefSetupDlg::LoadInfo()
{
	ADODB::_RecordsetPtr rsNDCInfo = CreateParamRecordset(
		"SELECT DefNDCCode, DefNDCQty, DefNDCUnitType, DefNDCUnitPrice FROM ServiceT WHERE ID = {INT}", m_nServiceID);

	if(!rsNDCInfo->eof){
		SetDlgItemText(IDC_NDC_DEF_CODE, AdoFldString(rsNDCInfo, "DefNDCCode", ""));
		SetDlgItemText(IDC_NDC_DEF_QUANTITY, AsString(AdoFldDouble(rsNDCInfo, "DefNDCQty", 0.0)));
		SetDlgItemText(IDC_NDC_DEF_UNIT_PRICE, FormatCurrencyForInterface(AdoFldCurrency(rsNDCInfo, "DefNDCUnitPrice", g_ccyZero)));

		CString strUnitType = AdoFldString(rsNDCInfo, "DefNDCUnitType", "");
		m_dlUnitType->SetSelByColumn(NDCDefUnitTypeList::ID, _bstr_t(strUnitType));
	}else{
		SetDlgItemText(IDC_NDC_DEF_CODE, "");
		SetDlgItemText(IDC_NDC_DEF_QUANTITY, "0");
		SetDlgItemText(IDC_NDC_DEF_UNIT_PRICE, FormatCurrencyForInterface(g_ccyZero));

		m_dlUnitType->SetSelByColumn(NDCDefUnitTypeList::ID, _bstr_t(""));
	}
}

void CNDCDefSetupDlg::OnBnClickedOk()
{
	try{
		CString strCode, strUnitType;
		double dblQty;
		COleCurrency cyUnitPrice;

		GetDlgItemText(IDC_NDC_DEF_CODE, strCode);

		{
			CString strTemp = strCode;
			bool bWrongSizedNDCCodes = false;
			bool bNonNumberNDCCodes = false;
			strTemp.Replace("-","");

			//see if the length is not 11 digits
			if(strTemp.GetLength() != 11) {
				bWrongSizedNDCCodes = true;
			}

			//confirm each character is number
			CString strNumbersOnly = strTemp.SpanIncluding("0123456789");
			if(strTemp.GetLength() != strNumbersOnly.GetLength()) {
				bNonNumberNDCCodes = true;
			}

			if(bNonNumberNDCCodes || bWrongSizedNDCCodes) {
				CString strWarn = "The following potential problem(s) were found with the NDC Code:\n\n";

				if(bNonNumberNDCCodes) {
					strWarn += "The code has a non-numeric value in it. A typical NDC Code only contains numbers and dashes.\r\n";
				}
				if(bWrongSizedNDCCodes) {
					strWarn += "The code is not 11 digits long. A typical NDC code is 11 digits, excluding dashes.\r\n";
				}

				strWarn += "\r\nAre you sure you wish to continue with these changes?";

				if(IDNO == MessageBox(strWarn, "Practice", MB_YESNO|MB_ICONEXCLAMATION)) {
					return;
				}
			}
		}

		if(m_dlUnitType){
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlUnitType->CurSel;
			if(pRow){
				strUnitType = VarString(pRow->GetValue(NDCDefUnitTypeList::ID), "");
			}
		}

		{
			CString strValue;
			GetDlgItemText(IDC_NDC_DEF_QUANTITY, strValue);

			dblQty = 0.0;
			if(!strValue.IsEmpty()){
				dblQty = atof(strValue);
				if(dblQty < 0.0){
					dblQty = 0.0;
				}
			}
		}

		{
			CString strValue;
			GetDlgItemText(IDC_NDC_DEF_UNIT_PRICE, strValue);

			if(!cyUnitPrice.ParseCurrency(strValue) || cyUnitPrice < g_ccyZero){
				cyUnitPrice = g_ccyZero;
			}
		}

		ADODB::_RecordsetPtr rsNDCInfo;

		if(m_bFromInvModule){
			rsNDCInfo = CreateParamRecordset(
				"SELECT Name, 'Product' AS Code, '' AS SubCode, DefNDCCode, DefNDCQty, DefNDCUnitType, DefNDCUnitPrice "
				"FROM ServiceT WHERE ServiceT.ID = {INT}", m_nServiceID);
		}else{
			rsNDCInfo = CreateParamRecordset(
				"SELECT Name, Code, SubCode, DefNDCCode, DefNDCQty, DefNDCUnitType, DefNDCUnitPrice "
				"FROM ServiceT "
				"LEFT JOIN CptCodeT ON ServiceT.ID = CPTCodeT.ID "
				"WHERE ServiceT.ID = {INT}", m_nServiceID);
		}

		CString strOldCode = "";
		CString strOldUnitType = "";
		COleCurrency cyOldUnitPrice = g_ccyZero;
		double dblOldQty = 0.0;
		CString strCodeName = "";
		CString strCPTCode = "";
		CString strCPTSubCode = "";

		if(!rsNDCInfo->eof){
			strOldCode = AdoFldString(rsNDCInfo, "DefNDCCode", "");
			strOldUnitType = AdoFldString(rsNDCInfo, "DefNDCUnitType", "");
			cyOldUnitPrice = AdoFldCurrency(rsNDCInfo, "DefNDCUnitPrice", g_ccyZero);
			dblOldQty = AdoFldDouble(rsNDCInfo, "DefNDCQty", 0.0);
			strCPTCode = AdoFldString(rsNDCInfo, "Code", "");
			strCPTSubCode = AdoFldString(rsNDCInfo, "SubCode", "");
			strCodeName = AdoFldString(rsNDCInfo, "Name", "");
		}
		rsNDCInfo->Close();

		strCPTCode.TrimLeft();
		strCPTCode.TrimRight();
		strCPTSubCode.TrimLeft();
		strCPTSubCode.TrimRight();
		strCodeName.TrimLeft();
		strCodeName.TrimRight();

		if(!strCPTSubCode.IsEmpty()){
			strCPTCode += " - " + strCPTSubCode;
		}

		ExecuteParamSql(
			"UPDATE ServiceT SET DefNDCCode = {STRING}, "
			"DefNDCQty = {DOUBLE}, "
			"DefNDCUnitType = {STRING}, "
			"DefNDCUnitPrice = {OLECURRENCY} "
			"WHERE ID = {INT}", strCode, dblQty, strUnitType, cyUnitPrice, m_nServiceID);

		CAuditTransaction auditTrans;
		if(strOldCode.Compare(strCode)){
			AuditEvent(-1, strCodeName, auditTrans, aeiDefNDCCode, m_nServiceID, strCPTCode + ": " + (strOldCode.IsEmpty() ? "<None>" : strOldCode), (strCode.IsEmpty() ? "<None>" : strCode), aepLow, aetChanged);
		}

		if(cyOldUnitPrice != cyUnitPrice){
			AuditEvent(-1, strCodeName, auditTrans, aeiDefUnitPrice, m_nServiceID, strCPTCode + ": " + FormatCurrencyForInterface(cyOldUnitPrice), FormatCurrencyForInterface(cyUnitPrice), aepLow, aetChanged);
		}

		if(strOldUnitType.Compare(strUnitType)){
			AuditEvent(-1, strCodeName, auditTrans, aeiDefUnitType, m_nServiceID, strCPTCode + ": " + strOldUnitType, strUnitType, aepLow, aetChanged);
		}

		if(dblOldQty != dblQty){
			AuditEvent(-1, strCodeName, auditTrans, aeiDefQty, m_nServiceID, strCPTCode + ": " + AsString(dblOldQty), AsString(dblQty), aepLow, aetChanged);
		}

		auditTrans.Commit();
	}NxCatchAll(__FUNCTION__);

	CNxDialog::OnOK();
}

void CNDCDefSetupDlg::SelChangingNdcDefUnitType(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}

BOOL CNDCDefSetupDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	try{
		int nID = LOWORD(wParam);
		switch (HIWORD(wParam)){
			case EN_KILLFOCUS:
				{
					switch(nID){
						case IDC_NDC_DEF_CODE:
							{
								CString strValue;
								GetDlgItemText(IDC_NDC_DEF_CODE, strValue);
								strValue = strValue.Trim();
								SetDlgItemText(IDC_NDC_DEF_CODE, strValue);
							}
							break;
						case IDC_NDC_DEF_QUANTITY:
							{
								CString strValue;
								GetDlgItemText(IDC_NDC_DEF_QUANTITY, strValue);

								double dblQty = 0.0;
								if(!strValue.IsEmpty()){
									dblQty = atof(strValue);
									if(dblQty < 0.0){
										dblQty = 0.0;
									}
								}

								SetDlgItemText(IDC_NDC_DEF_QUANTITY, AsString(dblQty));
							}
							break;
						case IDC_NDC_DEF_UNIT_PRICE:
							{
								CString strValue;
								GetDlgItemText(IDC_NDC_DEF_UNIT_PRICE, strValue);
								COleCurrency cyUnitPrice;
								
								if(!cyUnitPrice.ParseCurrency(strValue) || cyUnitPrice < g_ccyZero){
									cyUnitPrice = g_ccyZero;
								}

								SetDlgItemText(IDC_NDC_DEF_UNIT_PRICE, FormatCurrencyForInterface(cyUnitPrice));
							}
							break;
						default:
							break;
					}
				}
				break;
			default:
				break;
		}
	}NxCatchAll(__FUNCTION__);

	return CNxDialog::OnCommand(wParam, lParam);
}