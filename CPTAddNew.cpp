// CPTAddNew.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "CPTCodes.h"
#include "CPTAddNew.h"
#include "GlobalFinancialUtils.h"
#include "NxStandard.h"
#include "PracProps.h"
#include "InternationalUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 10:28) - PLID 28000 - Need to specify namespace
using namespace ADODB;
using namespace NXDATALIST2Lib;

// (j.jones 2008-11-12 16:12) - PLID 30702 - added procedure list enum
enum ProcedureListColumns {

	plcID = 0,
	plcName,
};

/////////////////////////////////////////////////////////////////////////////
// CCPTAddNew dialog

CCPTAddNew::CCPTAddNew(CWnd* pParent)
	: CNxDialog(CCPTAddNew::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCPTAddNew)
		m_pParent = pParent;
		m_nID = -1;
		m_cyPrice = COleCurrency(0,0);
	//}}AFX_DATA_INIT
}


void CCPTAddNew::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCPTAddNew)
	DDX_Control(pDX, IDC_TAXABLE_2, m_Taxable2);
	DDX_Control(pDX, IDC_TAXABLE_1, m_Taxable1);
	DDX_Control(pDX, IDC_CODE, m_nxeditCode);
	DDX_Control(pDX, IDC_SUBCODE, m_nxeditSubcode);
	DDX_Control(pDX, IDC_DESC, m_nxeditDesc);
	DDX_Control(pDX, IDC_FEE, m_nxeditFee);
	DDX_Control(pDX, IDC_COST, m_nxeditCost);
	DDX_Control(pDX, IDC_TYPE_OF_SERVICE, m_nxeditTypeOfService);
	DDX_Control(pDX, ID_OK_BTN, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCPTAddNew, CNxDialog)
	//{{AFX_MSG_MAP(CCPTAddNew)
	ON_BN_CLICKED(ID_OK_BTN, OnOkBtn)
	ON_EN_KILLFOCUS(IDC_FEE, OnKillfocusFee)
	ON_EN_KILLFOCUS(IDC_COST, OnKillfocusCost)
	ON_BN_CLICKED(IDC_TAXABLE_1, OnTaxable1)
	ON_BN_CLICKED(IDC_TAXABLE_2, OnTaxable2)
	ON_EN_KILLFOCUS(IDC_TYPE_OF_SERVICE, OnKillfocusTypeOfService)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCPTAddNew message handlers

void CCPTAddNew::OnOK() 
{
	
}

void CCPTAddNew::OnOkBtn() 
{
	try {

		CString code,
				subcode,
				desc,
				fee,
				tos,
				str,
				strCost;

		long	nNewID, tax1, tax2;

		GetDlgItemText (IDC_CODE, code);

		//replace any quotes that get entered, they shouldn't be here
		//I'm allowing quotes again because I don't know why they aren't there -JMM
		//code.Replace("\'", "");
		//code.Replace("\"", "");
		code.TrimLeft();
		code.TrimRight();

		
		if (code.IsEmpty()) {
			MessageBox ("Please enter valid data");
			return;
		}
		else {
			str = code;
			code.Format("%s", str);
		}

		if(m_Taxable1.GetCheck())
			tax1 = 1;
		else
			tax1 = 0;

		if(m_Taxable2.GetCheck())
			tax2 = 1;
		else
			tax2 = 0;

		GetDlgItemText (IDC_SUBCODE, subcode);
		//why take out the quotes, leave them in - JMM
		//subcode.Replace("\'", "");
		//subcode.Replace("\"", "");

		if (subcode.IsEmpty()) subcode = " ";
		else {
			str = subcode;
			subcode.Format ("%s", str);
		}

		GetDlgItemText(IDC_TYPE_OF_SERVICE, tos);
		tos.TrimRight();

		GetDlgItemText (IDC_DESC, desc);
		if (desc.IsEmpty()) desc = "";
		else {
			str = desc;
			desc.Format ("%s", str);
		}

		GetDlgItemText (IDC_FEE, fee);

		if(!IsValidCurrencyText(fee)) {
			MsgBox("Please fill correct information in the 'Standard Fee' box.");
			return;
		}

		COleCurrency cyFee = ParseCurrencyFromInterface(fee);

		if(cyFee < COleCurrency(0,0)) {
			MsgBox("Practice does not allow a negative amount for the standard fee.");
			return;
		}

		if(cyFee > COleCurrency(100000000,0)) {
			CString str;
			str.Format("Practice does not allow an amount greater than %s.",FormatCurrencyForInterface(COleCurrency(100000000,0),TRUE,TRUE));
			MsgBox(str);
			return;
		}
		// (s.dhole 2012-03-02 17:55) - PLID 47399 Add new cost
		GetDlgItemText (IDC_COST, strCost);

		if(!IsValidCurrencyText(strCost)) {
			MsgBox("Please fill correct information in the 'Default Cost' box.");
			return;
		}

		//TES 1/18/2012 - PLID 54700 - This was using fee and not strCost, thus the Cost field was not getting validated.
		COleCurrency cyCost = ParseCurrencyFromInterface(strCost);

		if(cyCost < COleCurrency(0,0)) {
			MsgBox("Practice does not allow a negative amount for the default cost.");
			return;
		}

		if(cyCost > COleCurrency(100000000,0)) {
			CString str;
			str.Format("Practice does not allow an amount greater than %s.",FormatCurrencyForInterface(COleCurrency(100000000,0),TRUE,TRUE));
			MsgBox(str);
			return;
		}

		

		CString sql;
		_RecordsetPtr rs;

		EnsureRemoteData();
		rs = CreateRecordset("SELECT * from CPTCodeT WHERE Code = '%s' AND SubCode = '%s'", _Q(code), _Q(subcode));
		if (!rs->eof) {
			MessageBox("This Service Code, Subcode combination already exists.\nChange one of the codes to save it.");
			return;
		}
		rs->Close();

		// (j.jones 2008-11-12 16:15) - PLID 30702 - get the procedure ID, if they selected one
		long nProcedureID = -1;
		IRowSettingsPtr pProcRow = m_ProcedureList->GetCurSel();
		if(pProcRow) {
			nProcedureID = VarLong(pProcRow->GetValue(plcID), -1);
		}
		CString strProcedureID = "NULL";
		if(nProcedureID != -1) {
			strProcedureID.Format("%li", nProcedureID);
		}

		nNewID = NewNumber("ServiceT","ID");
		// (j.jones 2008-11-12 16:28) - PLID 30702 - save the ProcedureID
		// (s.dhole 2012-03-02 17:55) - PLID 47399 Add new cost
		// (j.gruber 2012-12-04 11:35) - PLID 48566 - 
		//TES 1/18/2012 - PLID 53837 - Made the cost field internationally compliant
		ExecuteSql("INSERT INTO ServiceT (ID, Name, Price, Taxable1, Taxable2, ProcedureID) VALUES (%li, '%s', Convert(money, '%s'), %li, %li, %s);", nNewID, _Q(desc), _Q(FormatCurrencyForSql(cyFee)), tax1, tax2, strProcedureID);
		ExecuteSql("INSERT INTO CPTCodeT (ID, Code, SubCode, TypeOfService,ServiceDefaultCost) VALUES (%li, '%s', '%s', '%s',Convert(money, '%s'));", nNewID, _Q(code), _Q(subcode), _Q(tos) , _Q(FormatCurrencyForSql(cyCost)));
		ExecuteParamSql("INSERT INTO ServiceLocationInfoT (ServiceID, LocationID) \r\n"
							"SELECT {INT}, ID FROM LocationsT WHERE Managed = 1 "
						, nNewID);	
		

		//save all the data in the member variables so that the calling function can put them into the datalist
		m_nID = nNewID;
		strName = desc;
		strCode = code;
		strSubCode = subcode;
		// (j.jones 2010-01-11 09:08) - PLID 24054 - exposed the price as a currency, not a string
		m_cyPrice = cyFee;

		CDialog::OnOK();

	}NxCatchAll("Error in CCPTAddNew::OnOkBtn()");

	//DRT 7/7/03 - Moved the OnOK inside the try...catch.  If it fails, we need to stay in the dialog
	//		and not close it out.

}

BOOL CCPTAddNew::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();
		
		// (z.manning, 04/30/2008) - PLID 29852 - Set button styles
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// (j.jones 2008-11-12 16:10) - PLID 30702 - added procedure list
		m_ProcedureList = BindNxDataList2Ctrl(IDC_CPT_PROCEDURE_LIST);

		//add a "no procedure" row
		IRowSettingsPtr pRow = m_ProcedureList->GetNewRow();
		pRow->PutValue(plcID, (long)-1);
		pRow->PutValue(plcName, (LPCTSTR)" <No Procedure Selected>");
		m_ProcedureList->AddRowSorted(pRow, NULL);

		GetDlgItem(IDC_CODE)->SetFocus();
		
		SetDlgItemText(IDC_FEE, FormatCurrencyForInterface(COleCurrency(0,0)));
		SetDlgItemText(IDC_COST, FormatCurrencyForInterface(COleCurrency(0,0)));

		((CNxEdit*)GetDlgItem(IDC_CODE))->SetLimitText(50);
		((CNxEdit*)GetDlgItem(IDC_SUBCODE))->SetLimitText(50);
		((CNxEdit*)GetDlgItem(IDC_DESC))->SetLimitText(255);
		((CNxEdit*)GetDlgItem(IDC_TYPE_OF_SERVICE))->SetLimitText(2);

	}NxCatchAll("Error in CCPTAddNew::OnInitDialog");
	
	return TRUE;
}

void CCPTAddNew::OnKillfocusFee() 
{
	try {
	
		GetDlgItemText(IDC_FEE, str);
		if (str.IsEmpty()) str = "0";
		COleCurrency cyTmp = ParseCurrencyFromInterface(str);
		if(cyTmp.GetStatus()==COleCurrency::invalid) {
			AfxMessageBox("An invalid currency was entered as the standard fee.\n"
			"Please correct this.");
			CNxEdit *pFee = (CNxEdit *)GetDlgItem(IDC_FEE);
			if (pFee) {
				pFee->SetWindowText(FormatCurrencyForInterface(COleCurrency(0,0)));
				pFee->SetFocus();
				pFee->SetSel(0, -1, TRUE);
			}
		} else {
			cy = ParseCurrencyFromInterface(str);
			SetDlgItemText(IDC_FEE, FormatCurrencyForInterface(cy));	
		}
	}NxCatchAll("Error parsing fee.");
}


// (s.dhole 2012-03-02 17:59) - PLID 47399
void CCPTAddNew::OnKillfocusCost() 
{
	try {
	
		GetDlgItemText(IDC_COST , str);
		if (str.IsEmpty()) str = "0";
		COleCurrency cyTmp = ParseCurrencyFromInterface(str);
		if(cyTmp.GetStatus()==COleCurrency::invalid) {
			AfxMessageBox("An invalid currency was entered as the default cost.\n"
			"Please correct this.");
			CNxEdit *pFee = (CNxEdit *)GetDlgItem(IDC_COST);
			if (pFee) {
				pFee->SetWindowText(FormatCurrencyForInterface(COleCurrency(0,0)));
				pFee->SetFocus();
				pFee->SetSel(0, -1, TRUE);
			}
		} else {
			cy = ParseCurrencyFromInterface(str);
			SetDlgItemText(IDC_COST, FormatCurrencyForInterface(cy));	
		}
	}NxCatchAll("Error parsing cost.");
}

void CCPTAddNew::OnTaxable1() 
{
}

void CCPTAddNew::OnTaxable2() 
{
}

void CCPTAddNew::OnCancel() 
{
	CDialog::OnCancel();
}

void CCPTAddNew::OnKillfocusTypeOfService() 
{
	CString str;
	GetDlgItemText(IDC_TYPE_OF_SERVICE, str);

	if(str.GetLength() > 2) {
		//2 is max length of a type of service code
		MsgBox("The text you have entered is too long.  It will be truncated to 2 characters.");
		str = str.Left(2);
		SetDlgItemText(IDC_TYPE_OF_SERVICE, str);
	}
}
