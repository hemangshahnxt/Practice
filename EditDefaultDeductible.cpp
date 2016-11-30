// EditDefaultDeductible.cpp : implementation file
//Created for PLID 50636 by r.wilson 8/13/2012

#include "stdafx.h"
#include "Practice.h"
#include "EditDefaultDeductible.h"
#include "PatientsRc.h"
#include "AFXTEMPL.H"
#include "GlobalFinancialUtils.h"

using namespace ADODB;


//(8/23/2012 r.wilson) PLID 50636 - Columns for the insurance drop down
enum InsuranceCoCols {
	iccID = 0,							//InsuranceCo ID
	iccDefaultTotalDeductible,			
	iccDefaultTotalOOP,
	iccPerPayGroup,						
	iccName,
};


//(8/23/2012 r.wilson) PLID 50636 - Columns for the Paygroups data list (grid)
enum PayGroupListCols{
	pgcID = 0,						//ID for from table 'InsuranceCoPayGroupsDefaultsT'
	pgcPayGroupID,					
	pgcPayGroupName,
	pgcCoInsurance,
	pgcCopayMoney,
	pgcCopayPercentage,
	pgcTotalDeductible,
	pgcTotalOOP,
};


using namespace NXDATALIST2Lib;

// CEditDefaultDeductible dialog

IMPLEMENT_DYNAMIC(CEditDefaultDeductible, CNxDialog)

CEditDefaultDeductible::CEditDefaultDeductible(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEditDefaultDeductible::IDD, pParent)
{
	m_nInsuranceCoID = -1;	
}

CEditDefaultDeductible::~CEditDefaultDeductible()
{
}

void CEditDefaultDeductible::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_ALL_PAY_GROUPS_RADIO, m_radioAllPaygroups);
	DDX_Control(pDX, IDC_INDIVIDUAL_PAY_GROUPS_RADIO, m_radioIndividualPaygroups);
	DDX_Control(pDX, IDC_OK_BUTTON, m_nxibOK);
	DDX_Control(pDX, IDC_CANCEL_BUTTON, m_nxibCancel);
	DDX_Control(pDX, IDC_TOTAL_DEDUCTIBLE_EDIT, m_nxeditTotalDeductible);
	DDX_Control(pDX, IDC_TOTAL_OOP_EDIT, m_nxeditTotalOOP);
	DDX_Control(pDX, IDC_NXCOLORCTRL1, m_nxcTop);
	DDX_Control(pDX, IDC_NXCOLORCTRL_BOTTOM, m_nxcBottom);
}


BEGIN_MESSAGE_MAP(CEditDefaultDeductible, CNxDialog)
	ON_BN_CLICKED(IDC_ALL_PAY_GROUPS_RADIO, &CEditDefaultDeductible::OnBnClickedAllPayGroupsRadio)
	ON_BN_CLICKED(IDC_INDIVIDUAL_PAY_GROUPS_RADIO, &CEditDefaultDeductible::OnBnClickedIndividualPayGroupsRadio)
	ON_BN_CLICKED(IDC_OK_BUTTON, &CEditDefaultDeductible::OnBnClickedOkButton)
	ON_BN_CLICKED(IDC_CANCEL_BUTTON, &CEditDefaultDeductible::OnBnClickedCancelButton)
	ON_EN_KILLFOCUS(IDC_TOTAL_DEDUCTIBLE_EDIT, &CEditDefaultDeductible::OnEnKillfocusTotalDeductibleEdit)
	ON_EN_KILLFOCUS(IDC_TOTAL_OOP_EDIT, &CEditDefaultDeductible::OnEnKillfocusTotalOopEdit)
END_MESSAGE_MAP()


// CEditDefaultDeductible message handlers


BOOL CEditDefaultDeductible::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try
	{				
		m_bNeedsToSave = FALSE;

		//(8/23/2012 r.wilson) PLID 50636 - set back ground color as the same as the parent dialogs color
		m_nxcTop.SetColor(m_nColor);
		m_nxcBottom.SetColor(m_nColor);

		//(8/23/2012 r.wilson) PLID 50636 - Set flag because we are about to be in an initializing phase
		m_bDialogInitializing = TRUE;

		m_nxibOK.AutoSet(NXB_OK);
		m_nxibCancel.AutoSet(NXB_CANCEL);

		m_InsuranceCo =	BindNxDataList2Ctrl(IDC_INSURANCE_CO_DATALIST,false);
		m_PayGroupsList = BindNxDataList2Ctrl(IDC_PAY_GROUP_LIST, false);	

	
		m_InsuranceCo->Requery();

		/*
		//(8/23/2012 r.wilson) PLID 50636 - Make sure that insurance companies exist in the system
		if(m_InsuranceCo->GetRowCount() < 1){
			//Need to pop up a message telling user that they should add insurance companies before coming in here			
			MsgBox("There are no insurance companies in your system. \nPlease add at least one insurance company before continuing.");
			this->CloseWindow();
		}
		*/

		//(8/23/2012 r.wilson) PLID 50636 - Select whatever insuranceCo ID was passed in (if any)
		m_InsuranceCo->SetSelByColumn(iccID, (long) m_nInsuranceCoID);

		SetMemberVariables();
		//RadioButtonLogic();
		UpdatePayGroupsList();
		RadioButtonLogic();
				
	}NxCatchAll(__FUNCTION__);
	
	return TRUE;
}

//(8/23/2012 r.wilson) PLID 50636 - This gets fired to make the paygroups datalist to update based
//									on the selected Insurance company from the drop down
void CEditDefaultDeductible::UpdatePayGroupsList()
{

	IRowSettingsPtr pRow = m_InsuranceCo->GetCurSel();
	//(8/23/2012 r.wilson) PLID 50636 - Make sure the selected row is not null
	if(!pRow)
	{
		return;
	}

	//(8/23/2012 r.wilson) PLID 50636 - Get selected insurancecoId and store it in a member variable
	m_nInsuranceCoID = VarLong(pRow->GetValue(iccID));

	//(8/23/2012 r.wilson) PLID 50636 - Build new where clause
	//CString strWhereClause = "";
	CString strFromClause = "";
	strFromClause.Format(   " (SELECT InsuranceCoPayGroupsDefaultsT.ID AS InsuranceCoPayGroupsDefaultsTID , ServicePaygroupsT.ID AS ServicePaygroupsTID, ServicePaygroupsT.Name AS Name, "							
							"		 CopayMoney, CopayPercentage, CoInsurance, TotalDeductible, TotalOOP "
							" FROM ServicePaygroupsT "
							" LEFT JOIN (SELECT ID, PayGroupID, CopayMoney, CopayPercentage, CoInsurance, TotalDeductible, TotalOOP " 
							"				FROM InsuranceCoPayGroupsDefaultsT  "
							"				WHERE InsuranceCoID = %li ) "
							" AS InsuranceCoPayGroupsDefaultsT ON ServicePaygroupsT.ID = InsuranceCoPayGroupsDefaultsT.PaygroupID ) AS Qry", m_nInsuranceCoID);

	//strWhereClause.Format("InsuranceCoID = %li OR InsuranceCoID IS NULL",m_nInsuranceCoID);

	//(8/23/2012 r.wilson) PLID 50636 - Requery datalist
	m_PayGroupsList->PutFromClause(_bstr_t(strFromClause));
	m_PayGroupsList->Requery();


}

//(8/23/2012 r.wilson) PLID 50636 - This function does the following based on the member varible m_bPerPayGroup:
//				-Toggles the enable value for some text boxes and some columns
//				-Changes col color based on whether they are enabled
//				-Updates radio buttons to make sure only one is checked at a time
void CEditDefaultDeductible::RadioButtonLogic()
{
	CWnd* pEdit_TotalDeductible = GetDlgItem( IDC_TOTAL_DEDUCTIBLE_EDIT );
	CWnd* pEdit_OOP = GetDlgItem( IDC_TOTAL_OOP_EDIT );	

	pEdit_TotalDeductible->EnableWindow(!m_bPerPayGroup);
	pEdit_OOP->EnableWindow(!m_bPerPayGroup);

	//(8/23/2012 r.wilson) PLID 50636 - Update radio buttons
	m_radioAllPaygroups.SetCheck(!m_bPerPayGroup);
	m_radioIndividualPaygroups.SetCheck(m_bPerPayGroup);	
	
	FormatDataListCols();


}

void CEditDefaultDeductible::OnBnClickedAllPayGroupsRadio()
{
	try{
		//(8/23/2012 r.wilson) PLID 50636 - If nothing changed then nothin needs to be done
		if(m_bPerPayGroup == FALSE){
			return;
		}
		
		//(8/23/2012 r.wilson) PLID 50636 - Update radio button and member varable
		m_radioAllPaygroups.SetCheck(TRUE);
		m_bPerPayGroup = FALSE;
		m_bNeedsToSave = TRUE;

		//(8/23/2012 r.wilson) PLID 50636 - Call this function to reflect changes on the screen
		RadioButtonLogic();
	}NxCatchAll(__FUNCTION__);
}

void CEditDefaultDeductible::OnBnClickedIndividualPayGroupsRadio()
{
	try{
		//(8/23/2012 r.wilson) PLID 50636 - If nothing changed then nothin needs to be done
		if(m_bPerPayGroup == TRUE){
			return;
		}

		//(8/23/2012 r.wilson) PLID 50636 - Update radio button and member varable
		m_radioIndividualPaygroups.SetCheck(TRUE);
		m_bPerPayGroup = TRUE;
		m_bNeedsToSave = TRUE;

		//(8/23/2012 r.wilson) PLID 50636 - Call this function to reflect changes on the screen
		RadioButtonLogic();
	}NxCatchAll(__FUNCTION__);
}
BEGIN_EVENTSINK_MAP(CEditDefaultDeductible, CNxDialog)
	ON_EVENT(CEditDefaultDeductible, IDC_INSURANCE_CO_DATALIST, 2, CEditDefaultDeductible::SelChangedInsuranceCoDatalist, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CEditDefaultDeductible, IDC_PAY_GROUP_LIST, 9, CEditDefaultDeductible::EditingFinishingPayGroupList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CEditDefaultDeductible, IDC_PAY_GROUP_LIST, 10, CEditDefaultDeductible::EditingFinishedPayGroupList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
ON_EVENT(CEditDefaultDeductible, IDC_PAY_GROUP_LIST, 8, CEditDefaultDeductible::EditingStartingPayGroupList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
ON_EVENT(CEditDefaultDeductible, IDC_INSURANCE_CO_DATALIST, 1, CEditDefaultDeductible::SelChangingInsuranceCoDatalist, VTS_DISPATCH VTS_PDISPATCH)
ON_EVENT(CEditDefaultDeductible, IDC_PAY_GROUP_LIST, 18, CEditDefaultDeductible::RequeryFinishedPayGroupList, VTS_I2)
END_EVENTSINK_MAP()

void CEditDefaultDeductible::SelChangedInsuranceCoDatalist(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try
	{	
		
		IRowSettingsPtr pRowOld(lpOldSel);
		IRowSettingsPtr pRowNew(lpNewSel);		

		if(!pRowOld || !pRowNew){
			return;
		}

		if(!Save()){
			//(8/23/2012 r.wilson) PLID 50636 - if save failed then don't change insurance companies
			m_InsuranceCo->SetSelByColumn(iccID,(long) m_nInsuranceCoID);
			return;
		}	

		long nInsCoId = VarLong(pRowOld->GetValue(iccID));
		long nInsCoIdNew = VarLong(pRowNew->GetValue(iccID));

		m_nInsuranceCoID = VarLong(pRowNew->GetValue(iccID));
		SetMemberVariables();
		UpdatePayGroupsList();	
		RadioButtonLogic();

	}NxCatchAll(__FUNCTION__);
}

void CEditDefaultDeductible::SetMemberVariables()
{
	
		m_bPerPayGroup = VarBool(m_InsuranceCo->GetCurSel()->GetValue(iccPerPayGroup));
		COleCurrency cyDefaultTotalDeductible = VarCurrency(m_InsuranceCo->GetCurSel()->GetValue(iccDefaultTotalDeductible),COleCurrency(0,0));
		COleCurrency cyDefaultTotalOOP = VarCurrency(m_InsuranceCo->GetCurSel()->GetValue(iccDefaultTotalOOP), COleCurrency(0,0));
		
		_variant_t vtTmp = m_InsuranceCo->GetCurSel()->GetValue(iccDefaultTotalDeductible);

		if(cyDefaultTotalDeductible != COleCurrency(0,0)){		
			m_strDefaultTotalDeductible.Format("%s", AsString(vtTmp));
		}
		else{
			m_strDefaultTotalDeductible = "";
		}
		
		vtTmp = m_InsuranceCo->GetCurSel()->GetValue(iccDefaultTotalOOP);
		if(cyDefaultTotalOOP != COleCurrency(0,0)){		
			m_strDefaultTotalOOP.Format("%s", AsString(vtTmp));
		}
		else{
			m_strDefaultTotalOOP = "";
		}

		//(8/23/2012 r.wilson) PLID 50636 - Update display		
		m_nxeditTotalDeductible.SetWindowTextA(m_strDefaultTotalDeductible);
		m_nxeditTotalOOP.SetWindowTextA(m_strDefaultTotalOOP);

}


void CEditDefaultDeductible::EditingFinishingPayGroupList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		CString strEntered = strUserEntered;

		_variant_t varCopay;
		_variant_t varPercent;

		varCopay = pRow->GetValue(pgcCopayMoney);
		varPercent = pRow->GetValue(pgcCopayPercentage);

		if(*pbContinue && *pbCommit && pRow)
		{
			switch(nCol)
			{
				case pgcCoInsurance: 
					{
						if(strEntered.IsEmpty()){
							*pvarNewValue = g_cvarNull;
							return;
						}

						if(!IsNumeric(strEntered)){
							*pvarNewValue = g_cvarNull;
							MsgBox("Please enter a valid percentage value.");
							*pbContinue = FALSE;
							*pbCommit = FALSE;
							return;
						}
						
						long nPercent = atoi(strEntered);
						if (nPercent < 0 || nPercent > 100) {
							*pvarNewValue = g_cvarNull;
							MsgBox("Please enter a valid percentage value.");
							*pbContinue = FALSE;
							*pbCommit = FALSE;
							return;
						}
					}
					break;

				case pgcCopayMoney:
					{
						// (b.spivey, March 26, 2015) - PLID 56838 - Don't allow them to save a value in here if there's one in the other column
						//we can't have money if the percent is already filled in
						if (varPercent.vt != VT_NULL && varPercent.vt != VT_EMPTY && (!strEntered.IsEmpty())) {
							MessageBox("You cannot enter both a copay currency amount and a copay percent amount.");
							*pvarNewValue = g_cvarNull;
							return;
						}

						if (strEntered.IsEmpty()) {
							//put a null value in
							*pvarNewValue = g_cvarNull;
							return;
						}

						//check to make sure its a valid currency
						COleCurrency cyAmt = ParseCurrencyFromInterface(strEntered);
						if (cyAmt.GetStatus() != COleCurrency::valid) {
							*pvarNewValue = g_cvarNull;
							MsgBox("Please enter a valid currency.");
							*pbContinue = FALSE;
							*pbCommit = FALSE;
							return;
						}

						if (cyAmt < COleCurrency(0,0)) {
							*pvarNewValue = g_cvarNull;
							MsgBox("Please enter a currency greater than 0.");
							*pbContinue = FALSE;
							*pbCommit = FALSE;
							return;
						}						
					}
					break;

				case pgcCopayPercentage:
					{			
						// (b.spivey, March 26, 2015) - PLID 56838 - Don't allow them to save a value in here if there's one in the other column
						//we can't have percent if the money is already filled in
						if (varCopay.vt != VT_NULL && varCopay.vt != VT_EMPTY && (!strEntered.IsEmpty())) {
							MessageBox("You cannot enter both a copay currency amount and a copay percent amount.");
							*pvarNewValue = g_cvarNull;
							return;
						}

						if (strEntered.IsEmpty()) {
							//put a null value in
							*pvarNewValue = g_cvarNull;
							return;
						}

						//check to make sure its a number
						if (!IsNumeric(strEntered)) {
							*pvarNewValue = g_cvarNull;
							MsgBox("Please enter a valid percentage value.");
							*pbContinue = FALSE;
							*pbCommit = FALSE;
							return;
						}

						long nPercent = atoi(strEntered);
						if (nPercent < 0 || nPercent > 100) {
							*pvarNewValue = g_cvarNull;
							MsgBox("Please enter a valid percentage value.");
							*pbContinue = FALSE;
							*pbCommit = FALSE;
							return;
						}
					}
					break;

				case pgcTotalDeductible:
					{
						if (strEntered.IsEmpty()) {
							//put a null value in
							*pvarNewValue = g_cvarNull;
							return;
						}

						//check to make sure its a valid currency
						COleCurrency cyAmt = ParseCurrencyFromInterface(strEntered);
						if (cyAmt.GetStatus() != COleCurrency::valid) {
							*pvarNewValue = g_cvarNull;
							MsgBox("Please enter a valid currency.");
							*pbContinue = FALSE;
							*pbCommit = FALSE;
							return;
						}

						if (cyAmt < COleCurrency(0,0)) {
							*pvarNewValue = g_cvarNull;
							MsgBox("Please enter a currency greater than 0.");
							*pbContinue = FALSE;
							*pbCommit = FALSE;
							return;
						}
					}
					break;

					case pgcTotalOOP:
					{
						if (strEntered.IsEmpty()) {
							//put a null value in
							*pvarNewValue = g_cvarNull;
							return;
						}

						//check to make sure its a valid currency
						COleCurrency cyAmt = ParseCurrencyFromInterface(strEntered);
						if (cyAmt.GetStatus() != COleCurrency::valid) {
							*pvarNewValue = g_cvarNull;
							MsgBox("Please enter a valid currency.");
							*pbContinue = FALSE;
							*pbCommit = FALSE;
							return;
						}

						if (cyAmt < COleCurrency(0,0)) {
							*pvarNewValue = g_cvarNull;
							MsgBox("Please enter a currency greater than 0.");
							*pbContinue = FALSE;
							*pbCommit = FALSE;
							return;
						}
					}
					break;
			}
		}

	}NxCatchAll(__FUNCTION__);
}

void CEditDefaultDeductible::EditingFinishedPayGroupList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try
	{		
		IRowSettingsPtr pRow(lpRow);
		if(!pRow || bCommit == FALSE){
			return;
		}
		
		_variant_t varTmp = pRow->GetValue(nCol);
		CString strEmptyTest = AsString(pRow->GetValue(nCol));
		COleCurrency cyTmp = ParseCurrencyFromInterface(AsString(varTmp));
		varTmp = cyTmp;		
		varTmp.vt = VT_CY;
		
		if(!strEmptyTest.IsEmpty())
		{
			//(8/23/2012 r.wilson) PLID 50636 - Adds money formatting to the appropriate paygroup columns in the data list
			switch(nCol)
			{
				case pgcCopayMoney:					
						pRow->PutValue(nCol, varTmp);				
					break;
				
				case pgcTotalDeductible:				
					pRow->PutValue(nCol, varTmp);
					break;
					
				case pgcTotalOOP:				
					pRow->PutValue(nCol, varTmp);
					break;			
			}
		}

		_variant_t vtCopayMoney, vtCopayPercentage, vtCoInsurance, vtTotalDeductible, vtTotalOOP;

		vtCopayMoney = pRow->GetValue(pgcCopayMoney);
		vtCopayPercentage = pRow->GetValue(pgcCopayPercentage);
		vtCoInsurance = pRow->GetValue(pgcCoInsurance);
		vtTotalDeductible = pRow->GetValue(pgcTotalDeductible);
		vtTotalOOP = pRow->GetValue(pgcTotalOOP);

		// (j.jones 2015-11-05 11:39) - PLID 63866 - ensured CopayMoney is rounded
		if (vtCopayMoney.vt == VT_CY) {
			COleCurrency cyCopay = VarCurrency(vtCopayMoney);
			RoundCurrency(cyCopay);
			vtCopayMoney = _variant_t(cyCopay);
		}

		if(vtTotalDeductible.vt == VT_EMPTY )
		{
			vtTotalDeductible = g_cvarNull;			
		}
		if(vtTotalOOP.vt == VT_EMPTY)
		{			
			vtTotalOOP = g_cvarNull;
		}

		long nPaygroupID = VarLong(pRow->GetValue(pgcPayGroupID));			
		long nInsCoPaygroupsDefaultID = -1;

		if(VarLong(pRow->GetValue(pgcID),-1) == -1)
		{
			//(8/23/2012 r.wilson) PLID 50636 - if we couldn't pull out an id then we assume that we need to add a NEW entry
			 _RecordsetPtr rs = CreateParamRecordset(
				 "SET NOCOUNT ON \r\n"
				 " INSERT INTO InsuranceCoPayGroupsDefaultsT " 
				" ([InsuranceCoID],[PaygroupID],[CopayMoney],[CopayPercentage],[CoInsurance],[TotalDeductible],[TotalOOP]) "
				 "VALUES ({INT}, {INT}, {VT_CY}, {VT_I4}, {VT_I4}, {VT_CY}, {VT_CY}); \r\n "
				 " SET NOCOUNT OFF \r\n"
				 " SELECT CONVERT(INT, SCOPE_IDENTITY()) AS ID",
				 m_nInsuranceCoID, nPaygroupID, vtCopayMoney, vtCopayPercentage, vtCoInsurance, vtTotalDeductible, vtTotalOOP);

			  if(!rs->eof){					 
				  nInsCoPaygroupsDefaultID = AdoFldLong(rs,"ID");
				  pRow->PutValue(pgcID, (long) nInsCoPaygroupsDefaultID);				  
			  }
		}
		else
		{
			//(8/23/2012 r.wilson) PLID 50636 - Update existing values
			_RecordsetPtr rs = CreateParamRecordset(
				" UPDATE	InsuranceCoPayGroupsDefaultsT "
				" SET "
				"		CopayMoney = {VT_CY}, CopayPercentage = {VT_I4}, "
				"		CoInsurance = {VT_I4}, TotalDeductible = {VT_CY}, TotalOOP = {VT_CY}"
				" WHERE "
				"		InsuranceCoID = {INT} AND PayGroupID = {INT}",
				vtCopayMoney,vtCopayPercentage, vtCoInsurance,vtTotalDeductible, vtTotalOOP
				,m_nInsuranceCoID, nPaygroupID);							
		}	

	}NxCatchAll(__FUNCTION__);
}

//(8/23/2012 r.wilson) PLID 50636 - save function
/*		NOTE:   
			This save function is not responsible for the Paygroups datalist (grid) saving features.

*/
BOOLEAN CEditDefaultDeductible::Save()
{
   CSqlFragment sqlFrag;

   _variant_t vtDefaultDeduct;
   _variant_t vtDefaultTotalOOp;

	//(8/23/2012 r.wilson) PLID 50636 - The variant will be null or VT_CY
	COleCurrency cyAmt = ParseCurrencyFromInterface(m_strDefaultTotalDeductible);
	if (cyAmt.GetStatus() != COleCurrency::valid) {
		vtDefaultDeduct = g_cvarNull;					
	}
	else{
		vtDefaultDeduct = cyAmt;
	}
	
	//(8/23/2012 r.wilson) PLID 50636 - recycle cyAmt ...
	//	The variant will be null or VT_CY
	cyAmt = ParseCurrencyFromInterface(m_strDefaultTotalOOP);
	if (cyAmt.GetStatus() != COleCurrency::valid) {
		vtDefaultTotalOOp = g_cvarNull;					
	}
	else{
		vtDefaultTotalOOp = cyAmt;
	}
	

	//(8/23/2012 r.wilson) PLID 50636 - Update values in the insurance company drop down
	IRowSettingsPtr pRow = m_InsuranceCo->FindByColumn(iccID, (long) m_nInsuranceCoID, m_InsuranceCo->GetFirstRow(), FALSE);
	pRow->PutValue(iccDefaultTotalDeductible, vtDefaultDeduct);
	pRow->PutValue(iccDefaultTotalOOP, vtDefaultTotalOOp);
	pRow->PutValue(iccPerPayGroup, m_bPerPayGroup ? g_cvarTrue : g_cvarFalse);

	
	sqlFrag += CSqlFragment("Update InsuranceCoT SET DefaultTotalDeductible = {VT_CY}, DefaultTotalOOP = {VT_CY}, DefaultDeductiblePerPayGroup = {VT_BOOL} WHERE PersonID = {INT}",
		vtDefaultDeduct, vtDefaultTotalOOp, m_bPerPayGroup ? g_cvarTrue : g_cvarFalse, m_nInsuranceCoID);

   ExecuteParamSql(GetRemoteData(), sqlFrag);
   m_bNeedsToSave = FALSE;
   
   return TRUE;

}

//(8/23/2012 r.wilson) PLID 50636 - Ok Button clicked
void CEditDefaultDeductible::OnBnClickedOkButton()
{
	try
	{		
		//(8/23/2012 r.wilson) PLID 50636 - Save then exit
		Save();
		CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__);
}

//(8/23/2012 r.wilson) PLID 50636 - Cancel Button Clicked
void CEditDefaultDeductible::OnBnClickedCancelButton()
{
	try
	{
		//(8/23/2012 r.wilson) PLID		50636 - As user if they really want to exit	
		if(m_bNeedsToSave == TRUE){
			if(IDNO == MessageBox("Are you sure you want to exit without saving your changes?","Practice",MB_ICONQUESTION|MB_YESNO)){
				return;					
			}
		}


		CNxDialog::OnCancel();
	}NxCatchAll(__FUNCTION__);
}

//(8/23/2012 r.wilson) PLID 50636 - formats Total Deductible edit box to money format
void CEditDefaultDeductible::OnEnKillfocusTotalDeductibleEdit()
{
	try
	{		
		BOOL bErrorsFound = FALSE;
		CString strRawText;
		CString strTmp = "";
		m_nxeditTotalDeductible.GetWindowTextA(	strRawText);
		
		//(8/23/2012 r.wilson) PLID 50636 - if the string is empty don't format
		if(strRawText.IsEmpty()){
			m_strDefaultTotalDeductible = "";
			return;
		}

		COleCurrency cyTotalDeduct = ParseCurrencyFromInterface(strRawText);

		//(8/23/2012 r.wilson) PLID 50636 - if not valid currency
		if(cyTotalDeduct.GetStatus() != COleCurrency::valid){
			MsgBox("Please enter a valid currency.");
			bErrorsFound = TRUE;
		}

		//(8/23/2012 r.wilson) PLID 50636 - if currency is less that zero 
		else if(cyTotalDeduct < COleCurrency(0,0))
		{
			MsgBox("Please enter a currency greater than 0.");
			bErrorsFound = TRUE;
		}

		//(8/23/2012 r.wilson) PLID 50636 - if errors found change text back to old text and give it focus
		if(bErrorsFound == TRUE){
			m_nxeditTotalDeductible.SetWindowTextA(m_strDefaultTotalDeductible);			
			m_nxeditTotalDeductible.SetFocus();
		}

		//(8/23/2012 r.wilson) PLID 50636 - Format text to have $ and such
		else{
			_variant_t vtTmp(cyTotalDeduct);			
			m_strDefaultTotalDeductible = FormatCurrencyForInterface(vtTmp.cyVal);
			m_nxeditTotalDeductible.SetWindowTextA(m_strDefaultTotalDeductible);
		}

		m_bNeedsToSave = TRUE;
		

	}NxCatchAll(__FUNCTION__)
}

//(8/23/2012 r.wilson) PLID 50636 - formats the Total OOP edit box to money format
void CEditDefaultDeductible::OnEnKillfocusTotalOopEdit()
{
	try
	{		
		BOOL bErrorsFound = FALSE;
		CString strRawText;
		CString strTmp = "";
		m_nxeditTotalOOP.GetWindowTextA(strRawText);
		
		//(8/23/2012 r.wilson) PLID 50636 - if the string is empty don't format
		if(strRawText.IsEmpty()){
			m_strDefaultTotalOOP = "";
			return;
		}

		COleCurrency cyTotalOOP = ParseCurrencyFromInterface(strRawText);

		//(8/23/2012 r.wilson) PLID 50636 - if not valid currency
		if(cyTotalOOP.GetStatus() != COleCurrency::valid){
			MsgBox("Please enter a valid currency.");
			bErrorsFound = TRUE;
		}

		//(8/23/2012 r.wilson) PLID 50636 - if currency is less that zero 
		else if(cyTotalOOP < COleCurrency(0,0))
		{
			MsgBox("Please enter a currency greater than 0.");
			bErrorsFound = TRUE;
		}


		//(8/23/2012 r.wilson) PLID 50636 - if errors found change text back to old text and give it focus
		if(bErrorsFound == TRUE){
			m_nxeditTotalOOP.SetWindowTextA(m_strDefaultTotalOOP);
			m_nxeditTotalOOP.SetFocus();
		}

		//(8/23/2012 r.wilson) PLID 50636 - Format text to have $ and such
		else{
			_variant_t vtTmp(cyTotalOOP);									
			m_strDefaultTotalOOP = FormatCurrencyForInterface(vtTmp.cyVal);
			m_nxeditTotalOOP.SetWindowTextA(m_strDefaultTotalOOP);
		}

		m_bNeedsToSave = TRUE;

	}NxCatchAll(__FUNCTION__);
}

//(r.wilson 9/6/2012) PLID 50636 
void CEditDefaultDeductible::EditingStartingPayGroupList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		_variant_t varCopay;
		_variant_t varPercent;

		varCopay = pRow->GetValue(pgcCopayMoney);
		varPercent = pRow->GetValue(pgcCopayPercentage);
		// (b.spivey, March 26, 2015) - PLID 56838 - Don't let them start editing if a value in the opposite column exists. 
		switch (nCol) {

			case pgcCopayMoney:
				//we can't have money if the percent is already filled in
				if (varPercent.vt != VT_NULL && varPercent.vt != VT_EMPTY && (varCopay.vt == VT_NULL || varCopay.vt == VT_EMPTY)) {
					MessageBox("You cannot enter both a copay currency amount and a copay percent amount.");
					*pbContinue = FALSE;
				}
				break;

			case pgcCopayPercentage:
				//we can't have percent if the money is already filled in
				if (varCopay.vt != VT_NULL && varCopay.vt != VT_EMPTY && (varPercent.vt == VT_NULL || varPercent.vt == VT_EMPTY)) {
					MessageBox("You cannot enter both a copay currency amount and a copay percent amount.");
					*pbContinue = FALSE;
				}
				break;
		}

		if ((nCol != pgcTotalDeductible && nCol != pgcTotalOOP) || m_bPerPayGroup == FALSE){
			return; 
		}

		CString strPayGroupName = VarString(pRow->GetValue(pgcPayGroupName), _T(""));
		CString strCopayGroup = "";

		if (strPayGroupName.CompareNoCase(_T("Copay")) == 0)
		{
			*pbContinue = FALSE;
			if (nCol == pgcTotalDeductible){
				AfxMessageBox("\"Total Deductible\" cannot be used on the Copay pay group.");
			}
			if (nCol == pgcTotalOOP){
				AfxMessageBox("\"Total Out of Pocket\" cannot be used on the Copay pay group.");
			}

		}
		
		BOOL bError = FALSE;

		if (bError){
			MessageBox("You cannot enter both a copay currency amount and a copay percent amount.");
			*pbContinue = FALSE;
		}
	
		
	}NxCatchAll(__FUNCTION__);
}

//(r.wilson 9/6/2012) PLID 50636 
void CEditDefaultDeductible::SelChangingInsuranceCoDatalist(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try
	{
		IRowSettingsPtr pRowOld(lpOldSel);
		IRowSettingsPtr pRowNew(*lppNewSel);

		if(!pRowOld || !pRowNew){
			return;
		}

		long nInsCoId = VarLong(pRowOld->GetValue(iccID));
		long nInsCoIdNew = VarLong(pRowNew->GetValue(iccID));
		
		if(nInsCoId == nInsCoIdNew)
		{
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

		if(m_bNeedsToSave == TRUE){
			if(IDNO == MessageBox("Any changes made to the previous insurance company will be saved.\n"
			"Do you still wish to switch insurance companies?","Practice",MB_ICONQUESTION|MB_YESNO))
			{	
				SafeSetCOMPointer(lppNewSel, lpOldSel);
			}
		}
		
	}NxCatchAll(__FUNCTION__);

}

//(8/23/2012 r.wilson) PLID 50636 - If Insurance company is setting defaults Per Paygroups then enable these colomns on the datalist
	//									Else disable them
	//									The 2 edit control's enabled flag is always opposite of the 2 columns on the datalist
void CEditDefaultDeductible::FormatDataListCols()
{
	IColumnSettingsPtr pCol_TotalDeductible = m_PayGroupsList->GetColumn(pgcTotalDeductible);
		IColumnSettingsPtr pCol_OOP = m_PayGroupsList->GetColumn(pgcTotalOOP);

		IRowSettingsPtr pRowCopay = m_PayGroupsList->SearchByColumn(pgcPayGroupName,_T("Copay"), m_PayGroupsList->GetFirstRow() ,FALSE);

		if(m_bPerPayGroup == FALSE){

		if(pRowCopay){
			OLE_COLOR colorTmp = RGB(255,255,255);
			pRowCopay->PutCellBackColor(pgcTotalDeductible, colorTmp);
			pRowCopay->PutCellBackColor(pgcTotalOOP, colorTmp);
		}

		pCol_TotalDeductible->ColumnStyle = csVisible|csFixedWidth;
		pCol_TotalDeductible->PutStoredWidth(0);
		pCol_OOP->ColumnStyle = csVisible|csFixedWidth;
		pCol_OOP->PutStoredWidth(0);
	}
	else {
		
		if(pRowCopay){
			OLE_COLOR colorTmp = RGB(205,201,201);
			pRowCopay->PutCellBackColor(pgcTotalDeductible, colorTmp);
			pRowCopay->PutCellBackColor(pgcTotalOOP, colorTmp);
		}
		
		pCol_TotalDeductible->ColumnStyle = csVisible|csWidthAuto|csEditable;
		pCol_OOP->ColumnStyle = csVisible|csWidthAuto|csEditable;
	}
}

void CEditDefaultDeductible::RequeryFinishedPayGroupList(short nFlags)
{
	try
	{
		FormatDataListCols();
	}NxCatchAll(__FUNCTION__);
}
