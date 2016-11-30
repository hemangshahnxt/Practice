// ConversionRateByDateConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "marketingRc.h"
#include "ConversionRateByDateConfigDlg.h"
#include "GlobalDrawingUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CConversionRateByDateConfigDlg dialog

enum NewConsultColumns
{
	nccID=0,
	nccInclude,
	nccType,
};

enum ExistingConsultColumns
{
	eccID=0,
	eccInclude,
	eccType,

};

enum AllConsultColumns
{
	accID=0,
	accInclude,
	accType,

};
enum ProcedureColumns
{
	pcID=0,
	pcInclude,
	pcType,
};

CConversionRateByDateConfigDlg::CConversionRateByDateConfigDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CConversionRateByDateConfigDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConversionRateByDateConfigDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	//(e.lally 2009-09-14) PLID 35527 - Initialize
	m_bSplitConsults = FALSE;
}


void CConversionRateByDateConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConversionRateByDateConfigDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_NEW_CONSULT_LABEL, m_nxeditNewConsultLabel);
	DDX_Control(pDX, IDC_EXISTING_CONSULT_LABEL, m_nxeditExistingConsultLabel);
	DDX_Control(pDX, IDC_SURGERY_LABEL, m_nxeditSurgeryLabel);
	DDX_Control(pDX, IDC_ALL_CONSULT_LABEL, m_nxeditAllConsultLabel);
	DDX_Control(pDX, IDC_NEW_LABEL, m_nxstaticNewLabel);
	DDX_Control(pDX, IDC_ALL_LABEL, m_nxstaticAllLabel);
	DDX_Control(pDX, IDC_EXISTING_LABEL, m_nxstaticExistingLabel);
	DDX_Control(pDX, IDC_CONS1_LABEL, m_nxstaticCons1Label);
	DDX_Control(pDX, IDC_CONS_LABEL, m_nxstaticConsLabel);
	DDX_Control(pDX, IDC_CONS2_LABEL, m_nxstaticCons2Label);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_ALL_CONSULTS, m_radioAllConsults);
	DDX_Control(pDX, IDC_SPLIT_CONSULTS, m_radioSplitConsults);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConversionRateByDateConfigDlg, CNxDialog)
	//{{AFX_MSG_MAP(CConversionRateByDateConfigDlg)
	ON_BN_CLICKED(IDC_SPLIT_CONSULTS, OnSplitConsults)
	ON_BN_CLICKED(IDC_ALL_CONSULTS, OnAllConsults)
	ON_WM_CTLCOLOR()
	ON_EN_KILLFOCUS(IDC_NEW_CONSULT_LABEL, OnEnKillfocusNewConsultLabel)
	ON_EN_KILLFOCUS(IDC_EXISTING_CONSULT_LABEL, OnEnKillfocusExistingConsultLabel)
	ON_EN_KILLFOCUS(IDC_ALL_CONSULT_LABEL, OnEnKillfocusAllConsultLabel)
	ON_EN_KILLFOCUS(IDC_SURGERY_LABEL, OnEnKillfocusSurgeryLabel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CConversionRateByDateConfigDlg, CNxDialog)
	ON_EVENT(CConversionRateByDateConfigDlg, IDC_APTTYPE_SURGERY_LIST, 10, OnEditingFinishedApttypeSurgeryList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CConversionRateByDateConfigDlg, IDC_APTTYPE_NEW_CONSULT_LIST, 10, OnEditingFinishedApttypeNewConsultList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CConversionRateByDateConfigDlg, IDC_APTTYPE_EXISTING_CONSULT_LIST, 10, OnEditingFinishedApttypeExistingConsultList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CConversionRateByDateConfigDlg, IDC_APTTYPE_CONSULT_LIST, 10, OnEditingFinishedApttypeAllConsultList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConversionRateByDateConfigDlg message handlers

BOOL CConversionRateByDateConfigDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-29 12:04) - PLID 29824 - NxIconify the buttons
		//(e.lally 2009-09-14) PLID 35527 - Made the OK button a Close button. Removed the cancel button.
		m_btnOK.AutoSet(NXB_CLOSE);
		
		//initialize the datalists!!
		m_pNewConsultList = BindNxDataListCtrl(this, IDC_APTTYPE_NEW_CONSULT_LIST, GetRemoteData(), FALSE);
		m_pExistingConsultList = BindNxDataListCtrl(this, IDC_APTTYPE_EXISTING_CONSULT_LIST, GetRemoteData(), FALSE);
		m_pAllConsultList = BindNxDataListCtrl(this, IDC_APTTYPE_CONSULT_LIST, GetRemoteData(), FALSE);
		m_pSurgeryList = BindNxDataListCtrl(this, IDC_APTTYPE_SURGERY_LIST, GetRemoteData(), FALSE);

		m_brush.CreateSolidBrush(PaletteColor(0x00C8FFFF));

		//(e.lally 2009-09-14) PLID 35527 - Separated the split consult vs single lists/labels into separate preferences.
			//Just load from one place, track the settings separately for this dlg until we save
		m_bSplitConsults = GetRemotePropertyInt("CRGSplitConsults", 1, 0, "<None>", TRUE);
		m_strSplitConsultIDs = GetRemotePropertyText("CRGConsultList", "", 0, "<None>", TRUE);
		m_strSingleConsultIDs = GetRemotePropertyText("CRGSingleConsultList", "", 0, "<None>", TRUE);
		m_strSplitConsultLabels = GetRemotePropertyText("CRGConsultLabels", "New Consult---Existing Consult", 0, "<None>", TRUE);
		m_strSingleConsultLabel = GetRemotePropertyText("CRGSingleConsultLabels", "Consults", 0, "<None>", TRUE);

		m_strSurgeryIDs = GetRemotePropertyText("CRGSurgeryList", "", 0, "<None>", TRUE);
		m_strSurgeryLabel = GetRemotePropertyText("CRGSurgeryLabel", "Procedures", 0, "<None>", TRUE);

		m_nxeditNewConsultLabel.SetLimitText(125);			//Approx 1/2 Max length of ConfigRT.TextParam
		m_nxeditExistingConsultLabel.SetLimitText(125);		//Approx 1/2 Max length of ConfigRT.TextParam
		m_nxeditAllConsultLabel.SetLimitText(255);			//Max length of ConfigRT.TextParam
		m_nxeditAllConsultLabel.SetLimitText(255);			//Max length of ConfigRT.TextParam

		Refresh();
	}
	NxCatchAll("Error in CConversionRateByDateConfigDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CConversionRateByDateConfigDlg::Refresh() {

	//figure out what to show on this here dialog
	//(e.lally 2009-09-14) PLID 35527 - Use the member variables
	if (m_bSplitConsults) {
		
		CheckDlgButton(IDC_SPLIT_CONSULTS, TRUE);
		CheckDlgButton(IDC_ALL_CONSULTS, FALSE);


		GetDlgItem(IDC_APTTYPE_NEW_CONSULT_LIST)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_APTTYPE_EXISTING_CONSULT_LIST)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_APTTYPE_CONSULT_LIST)->ShowWindow(SW_HIDE);

		GetDlgItem(IDC_NEW_LABEL)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_EXISTING_LABEL)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_ALL_LABEL)->ShowWindow(SW_HIDE);

		GetDlgItem(IDC_CONS1_LABEL)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_CONS2_LABEL)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_CONS_LABEL)->ShowWindow(SW_HIDE);

		GetDlgItem(IDC_NEW_CONSULT_LABEL)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_EXISTING_CONSULT_LABEL)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_ALL_CONSULT_LABEL)->ShowWindow(SW_HIDE);
		

		m_pNewConsultList->Requery();
		m_pExistingConsultList->Requery();

		CString strNewIDList, strExistingIDList;
		
		long nResult = m_strSplitConsultIDs.Find("---");
		if (nResult == -1) {
			//that means we came from having just one list, just check the new list
			strNewIDList = m_strSplitConsultIDs;
			strExistingIDList = "";
		}
		else {
			strNewIDList = m_strSplitConsultIDs.Left(nResult);
			strExistingIDList = m_strSplitConsultIDs.Right(m_strSplitConsultIDs.GetLength() - (nResult + 3));
		}

		CheckDataList(m_pNewConsultList, strNewIDList);
		CheckDataList(m_pExistingConsultList, strExistingIDList);

		nResult = m_strSplitConsultLabels.Find("---");
		if (nResult == -1) {
			//(e.lally 2009-09-14) PLID 35527 - Eventually, this should no longer be possible.
			//we came from one list
			SetDlgItemText(IDC_NEW_CONSULT_LABEL, m_strSplitConsultLabels);
			SetDlgItemText(IDC_EXISTING_CONSULT_LABEL, "");
		}
		else {
			SetDlgItemText(IDC_NEW_CONSULT_LABEL, m_strSplitConsultLabels.Left(nResult));
			SetDlgItemText(IDC_EXISTING_CONSULT_LABEL, m_strSplitConsultLabels.Right(m_strSplitConsultLabels.GetLength() - (nResult + 3)));
		}

	}
	else {

		CheckDlgButton(IDC_SPLIT_CONSULTS, FALSE);
		CheckDlgButton(IDC_ALL_CONSULTS, TRUE);

		GetDlgItem(IDC_APTTYPE_NEW_CONSULT_LIST)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_APTTYPE_EXISTING_CONSULT_LIST)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_APTTYPE_CONSULT_LIST)->ShowWindow(SW_SHOW);

		GetDlgItem(IDC_NEW_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EXISTING_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ALL_LABEL)->ShowWindow(SW_SHOW);

		GetDlgItem(IDC_NEW_CONSULT_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EXISTING_CONSULT_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ALL_CONSULT_LABEL)->ShowWindow(SW_SHOW);

		GetDlgItem(IDC_CONS1_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_CONS2_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_CONS_LABEL)->ShowWindow(SW_SHOW);

		m_pAllConsultList->Requery();

		//check everything that we can
		m_strSingleConsultIDs.Replace(")---(", ",");
		CheckDataList(m_pAllConsultList, m_strSingleConsultIDs);

		//just set it to be the first part and they can change it to be whatever they want
		long nResult = m_strSingleConsultLabel.Find("---");
		if (nResult == -1) {
			SetDlgItemText(IDC_ALL_CONSULT_LABEL, m_strSingleConsultLabel);
		}
		else {
			//(e.lally 2009-09-14) PLID 35527 - Eventually, this should no longer be possible.
			SetDlgItemText(IDC_ALL_CONSULT_LABEL, m_strSingleConsultLabel.Left(nResult));
		}
		
	}


	//onto the surgery list
	m_pSurgeryList->Requery();
	CheckDataList(m_pSurgeryList, m_strSurgeryIDs);
	SetDlgItemText(IDC_SURGERY_LABEL, m_strSurgeryLabel);

}


void CConversionRateByDateConfigDlg::CheckDataList(NXDATALISTLib::_DNxDataListPtr pDataList, CString strChecks) {

	if (strChecks.IsEmpty()) {

		//rockon, we are done, we have nothing to do!!
		return;
	}


	//get rid of the parenthesis
	strChecks.TrimLeft("(");
	strChecks.TrimRight(")");

	//loop through the list until we have all the numbers
	CString strChecksTmp = strChecks;
	long nResult;
	long nIDtoCheck;
	IRowSettingsPtr pRow;


	const _variant_t varTrue(VARIANT_TRUE, VT_BOOL);
	const _variant_t varFalse(VARIANT_FALSE, VT_BOOL);


	//first, loop through all the rows and set them all to FALSE
	for (int i = 0; i < pDataList->GetRowCount(); i++) {
		pRow = pDataList->GetRow(i);
		pRow->PutValue(1, varFalse);
	}

	//TES 2004-01-30: You know, Visual Studio isn't just giving these warnings for its health.
	nResult = strChecksTmp.Find(",");

	if (nResult == -1) {

		//there is only one item in the list

		//make sure it isn't empty
		if (!strChecksTmp.IsEmpty()) {
			nIDtoCheck = nIDtoCheck = atoi(strChecksTmp);
			pRow = pDataList->GetRow(pDataList->FindByColumn(0, nIDtoCheck, 0, FALSE));
			pRow->PutValue(1, varTrue);
		}
	}
	else {

		//now set the ones that need to be true, to true
		while ((!strChecksTmp.IsEmpty()) && (nResult != -1)) {

 			nResult = strChecksTmp.Find(",");

			if (nResult == -1) {
				//there must be only one left
				nIDtoCheck = atoi(strChecksTmp);
			}
			else {
				nIDtoCheck = atoi(strChecksTmp.Left(nResult));
			}


			//find the row in the datalist
			pRow = pDataList->GetRow(pDataList->FindByColumn(0, nIDtoCheck, 0, FALSE));
			pRow->PutValue(1, varTrue);

			//alrighty, we are done, move on
			strChecksTmp = strChecksTmp.Right(strChecksTmp.GetLength() - (nResult + 1));

		}
	}

}

void CConversionRateByDateConfigDlg::OnCancel() 
{
	//(e.lally 2009-09-14) PLID 35527 - Don't need to save the original values anymore.
	try{
		if(!IsValidSetup(TRUE)){
			return;
		}
		CNxDialog::OnCancel();
	}NxCatchAll(__FUNCTION__);
}

CString CConversionRateByDateConfigDlg::GenerateSaveString(NXDATALISTLib::_DNxDataListPtr pDataList)  {

	CString strReturn = "(";

	//loop through the datalist and see which ones are checked

	for (int i = 0; i < pDataList->GetRowCount(); i++) {

		if (VarBool(pDataList->GetValue(i, 1))) {

			strReturn += AsString(VarLong(pDataList->GetValue(i,0))) + ",";
		}
	}

	//take off the last ,
	strReturn.TrimRight(",");

	//append the ending )
	strReturn += ")";

	return strReturn;
}


BOOL CConversionRateByDateConfigDlg::IsDataListChecked(NXDATALISTLib::_DNxDataListPtr pDataList) {

	//(e.lally 2009-09-14) PLID 35527 - This code does not look very efficient, but I am leaving it as is.
	//Let the caller handle the message to the user.
	for (int i = 0; i < pDataList->GetRowCount(); i++) {
		if (VarBool(pDataList->GetValue(i, 1))) {
			return TRUE;
		}
	}
	return FALSE;
}

BOOL IsAnyEntryCheckedInBothLists(_DNxDataList *lpdl1, short nIDColumnIndex1, short nCheckboxColumnIndex1, _DNxDataList *lpdl2, short nIDColumnIndex2, short nCheckboxColumnIndex2)
{
	_DNxDataListPtr pdl1(lpdl1);
	_DNxDataListPtr pdl2(lpdl2);

	// Loop through list 1.  For any checked item, see if it's also checked in list 2.
	for (long p = pdl1->GetFirstRowEnum(); p; ) {
		LPDISPATCH lpDisp = NULL;
		pdl1->GetNextRowEnum(&p, &lpDisp);
		if (lpDisp) {
			IRowSettingsPtr pRow(lpDisp);
			lpDisp->Release();
			if (AsBool(pRow->GetValue(nCheckboxColumnIndex1))) {
				// It's checked in list 1.  See if it's checked in list 2.
				_variant_t varID = pRow->GetValue(nIDColumnIndex1);
				long nSearchStart = 0;
				while (true) {
					// Nope, it wasn't checked in the second list
					long nNextFound = pdl2->FindByColumn(nIDColumnIndex2, varID, nSearchStart, VARIANT_FALSE);
					if (nNextFound != sriNoRow && nNextFound >= nSearchStart) {
						// Found the row, see if it's checked
						if (AsBool(pdl2->GetValue(nNextFound, nCheckboxColumnIndex2))) {
							// Yep, it's checked in both lists
							return TRUE;
						}
						// Keep searching, starting immediately AFTER this one
						nSearchStart = nNextFound + 1;
					} else {
						// No more to find
						break;
					}
				}
			}
		}
	}

	// If we made it here, there were no elements in list 1 that were also checked in list 2.
	return FALSE;
}

void CConversionRateByDateConfigDlg::SaveConsultTypeLists() 
{
	//(e.lally 2009-11-13) PLID 35527 - We don't want to save "bad" setups, so check the validity before committing the changes
	//The user won't be allowed to leave this setup with an invalid setup, so this just prevents data from getting messed up with
	//an abnormal termination of the program in the middle of changing the settings.
	if(!IsValidSetup(FALSE)){
		return;
	}
	//(e.lally 2009-09-14) PLID 35527 - try to save our settings
	if (m_bSplitConsults) {

		CString strNew, strExisting;

		strNew = GenerateSaveString(m_pNewConsultList);
		strExisting = GenerateSaveString(m_pExistingConsultList);
		m_strSplitConsultIDs = strNew + "---" + strExisting;
		SetRemotePropertyText("CRGConsultList", m_strSplitConsultIDs, 0, "<None>");

	}
	else {

		//(e.lally 2009-09-14) PLID 35527 - Use our member variables
		m_strSingleConsultIDs = GenerateSaveString(m_pAllConsultList);
		SetRemotePropertyText("CRGSingleConsultList", m_strSingleConsultIDs, 0, "<None>");

	}

	
	//(e.lally 2009-09-14) PLID 35527 - Use our member variables
	m_strSurgeryIDs = GenerateSaveString(m_pSurgeryList);
	SetRemotePropertyText("CRGSurgeryList", m_strSurgeryIDs, 0, "<None>");
}

void CConversionRateByDateConfigDlg::SaveProcedureTypeList() 
{
	//(e.lally 2009-11-13) PLID 35527 - We don't want to save "bad" setups, so check the validity before committing the changes
	//The user won't be allowed to leave this setup with an invalid setup, so this just prevents data from getting messed up with
	//an abnormal termination of the program in the middle of changing the settings.
	if(!IsValidSetup(FALSE)){
		return;
	}
	
	//(e.lally 2009-09-14) PLID 35527 - Use our member variables
	m_strSurgeryIDs = GenerateSaveString(m_pSurgeryList);
	SetRemotePropertyText("CRGSurgeryList", m_strSurgeryIDs, 0, "<None>");
}

void CConversionRateByDateConfigDlg::SaveConsultLabels() 
{
	//(e.lally 2009-11-13) PLID 35527 - We don't want to save "bad" setups, so check the validity before committing the changes
	//The user won't be allowed to leave this setup with an invalid setup, so this just prevents data from getting messed up with
	//an abnormal termination of the program in the middle of changing the settings.
	if(!IsValidSetup(FALSE)){
		return;
	}
	//(e.lally 2009-09-14) PLID 35527 - try to save our settings
	if (m_bSplitConsults) {

		//set the labels
		CString strCons1Label, strCons2Label;
		GetDlgItemText(IDC_NEW_CONSULT_LABEL, strCons1Label);
		GetDlgItemText(IDC_EXISTING_CONSULT_LABEL, strCons2Label);
		m_strSplitConsultLabels = strCons1Label + "---" + strCons2Label;
		SetRemotePropertyText("CRGConsultLabels", m_strSplitConsultLabels, 0, "<None>");

	}
	else {
		GetDlgItemText(IDC_ALL_CONSULT_LABEL, m_strSingleConsultLabel);
		SetRemotePropertyText("CRGSingleConsultLabels", m_strSingleConsultLabel, 0, "<None>");

	}
}

void CConversionRateByDateConfigDlg::SaveProcedureLabel() 
{
	//(e.lally 2009-11-13) PLID 35527 - We don't want to save "bad" setups, so check the validity before committing the changes
	//The user won't be allowed to leave this setup with an invalid setup, so this just prevents data from getting messed up with
	//an abnormal termination of the program in the middle of changing the settings.
	if(!IsValidSetup(FALSE)){
		return;
	}
	//(e.lally 2009-09-14) PLID 35527 - Save the procedure label changes
	GetDlgItemText(IDC_SURGERY_LABEL, m_strSurgeryLabel);
	SetRemotePropertyText("CRGSurgeryLabel", m_strSurgeryLabel, 0, "<None>");
}

BOOL CConversionRateByDateConfigDlg::IsValidSetup(BOOL bWarnUser)
{
	if (m_bSplitConsults) {
		if ((!IsDataListChecked(m_pNewConsultList))) {
			if(bWarnUser){
				//(e.lally 2009-09-14) PLID 35527 - Give a more informative message to the user
				MessageBox("Please select at least one item to use as the New Consult type list", "Practice", MB_OK);
			}
			return FALSE;
		}

		if (!IsDataListChecked(m_pExistingConsultList)) {
			if(bWarnUser){
				//(e.lally 2009-09-14) PLID 35527 - Give a more informative message to the user
				MessageBox("Please select at least one item to use as the Existing Consult type list", "Practice", MB_OK);
			}
			return FALSE;
		}
		if (!IsDataListChecked(m_pSurgeryList)) {
			if(bWarnUser){
				//(e.lally 2009-09-14) PLID 35527 - Give a more informative message to the user
				MessageBox("Please select at least one item to use as the Procedure type list", "Practice", MB_OK);
			}
			return FALSE;
		}

		// (b.cardillo 2005-10-03 13:30) - PLID 17626 - Give a warning if there are any types 
		// that are selected in BOTH lists.
		if (IsAnyEntryCheckedInBothLists(m_pNewConsultList, 0, 1, m_pExistingConsultList, 0, 1)) {
			if(bWarnUser){
				if (MessageBox(
					"A consult type is checked in both the 'New Consults' and 'Existing Consults' lists.  "
					"If you proceed, any appointment with this type will be counted as both 'New' AND "
					"'Existing'.\r\n\r\nClick OK to proceed anyway, or Cancel to review the settings.", 
					NULL, MB_OKCANCEL|MB_ICONEXCLAMATION) != IDOK)
				{
					return FALSE;
				}
			}
		}
	}
	else {

		if (!IsDataListChecked(m_pAllConsultList)) {
			if(bWarnUser){
				//(e.lally 2009-09-14) PLID 35527 - Give a more informative message to the user
				MessageBox("Please select at least one item to use as the Consult type list", "Practice", MB_OK);
			}
			return FALSE;
		}
		if (!IsDataListChecked(m_pSurgeryList)) {
			if(bWarnUser){
				//(e.lally 2009-09-14) PLID 35527 - Give a more informative message to the user
				MessageBox("Please select at least one item to use as the Procedure type list", "Practice", MB_OK);
			}
			return FALSE;
		}
	}
	return TRUE;
}

void CConversionRateByDateConfigDlg::OnOK() 
{
	//(e.lally 2009-09-14) PLID 35527
	try {
		if(!IsValidSetup(TRUE)){
			return;
		}

		CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__);
}

void CConversionRateByDateConfigDlg::OnSplitConsults() 
{
	//(e.lally 2009-09-14) PLID 35527 - Save changes if the settings are valid, otherwise stop loading the new settings.
	try {
		if(!IsValidSetup(TRUE)){
			m_radioSplitConsults.SetCheck(FALSE);
			//reset back to other radio button
			m_radioAllConsults.SetCheck(TRUE);
			return;
		}
		m_bSplitConsults = TRUE;
		SetRemotePropertyInt("CRGSplitConsults", m_bSplitConsults, 0, "<None>");
		Refresh();
	}NxCatchAll(__FUNCTION__);
}

void CConversionRateByDateConfigDlg::OnAllConsults() 
{
	//(e.lally 2009-09-14) PLID 35527 - Save changes if the settings are valid, otherwise stop loading the new settings.
	try {
		if(!IsValidSetup(TRUE)){
			m_radioSplitConsults.SetCheck(TRUE);
			m_radioAllConsults.SetCheck(FALSE);
			return;
		}
		m_bSplitConsults = FALSE;
		SetRemotePropertyInt("CRGSplitConsults", m_bSplitConsults, 0, "<None>");
		Refresh();
	}NxCatchAll(__FUNCTION__);
}

HBRUSH CConversionRateByDateConfigDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	/*
	HBRUSH hbr = CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (nCtlColor == CTLCOLOR_STATIC)
	{
		extern CPracticeApp theApp;
		pDC->SelectPalette(&theApp.m_palette, FALSE);
		pDC->RealizePalette();
		pDC->SetBkColor(PaletteColor(0x00C8FFFF));
		return m_brush;
	}

	return hbr;
	*/

	// (a.walling 2008-04-02 09:13) - PLID 29497 - Handle new NxColor's non-solid backgrounds
	HANDLE_GENERIC_TRANSPARENT_CTL_COLOR();
}

//(e.lally 2009-11-13) PLID 35527
void CConversionRateByDateConfigDlg::OnEditingFinishedApttypeSurgeryList(long nRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try{
		if(nCol == pcInclude){
			SaveProcedureTypeList();
		}

	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2009-11-13) PLID 35527
void CConversionRateByDateConfigDlg::OnEditingFinishedApttypeNewConsultList(long nRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try{
		if(nCol == nccInclude){
			SaveConsultTypeLists();
		}

	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2009-11-13) PLID 35527
void CConversionRateByDateConfigDlg::OnEditingFinishedApttypeExistingConsultList(long nRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try{
		if(nCol == eccInclude){
			SaveConsultTypeLists();
		}

	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2009-11-13) PLID 35527
void CConversionRateByDateConfigDlg::OnEditingFinishedApttypeAllConsultList(long nRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try{
		if(nCol == accInclude){
			SaveConsultTypeLists();
		}

	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2009-11-13) PLID 35527
void CConversionRateByDateConfigDlg::OnEnKillfocusNewConsultLabel()
{
	try {
		SaveConsultLabels();

	} NxCatchAll(__FUNCTION__);
}

//(e.lally 2009-11-13) PLID 35527
void CConversionRateByDateConfigDlg::OnEnKillfocusExistingConsultLabel()
{
	try {
		SaveConsultLabels();

	} NxCatchAll(__FUNCTION__);
}

//(e.lally 2009-11-13) PLID 35527
void CConversionRateByDateConfigDlg::OnEnKillfocusAllConsultLabel()
{
	try {
		SaveConsultLabels();

	} NxCatchAll(__FUNCTION__);
}

//(e.lally 2009-11-13) PLID 35527
void CConversionRateByDateConfigDlg::OnEnKillfocusSurgeryLabel()
{
	try {
		SaveProcedureLabel();
	} NxCatchAll(__FUNCTION__);
}