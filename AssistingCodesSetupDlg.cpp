// AssistingCodesSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AssistingCodesSetupDlg.h"
#include "InternationalUtils.h"

// (j.jones 2010-11-22 10:07) - PLID 41564 - created

// CAssistingCodesSetupDlg dialog

IMPLEMENT_DYNAMIC(CAssistingCodesSetupDlg, CNxDialog)

using namespace NXDATALIST2Lib;
using namespace ADODB;

enum UnitListColumns {

	ulcHours = 0,
	ulcMinutes,
	ulcUnits,
};

CAssistingCodesSetupDlg::CAssistingCodesSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CAssistingCodesSetupDlg::IDD, pParent)
{
	m_bUnitTableChanged = FALSE;
}

CAssistingCodesSetupDlg::~CAssistingCodesSetupDlg()
{
}

void CAssistingCodesSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BTN_ADD_NEW_ASSISTING_UNIT_OPTION, m_btnAdd);
	DDX_Control(pDX, IDC_BTN_DELETE_ASSISTING_UNIT_OPTION, m_btnDelete);
	DDX_Control(pDX, IDC_RADIO_BEGIN_END_TIMES, m_radioTimes);
	DDX_Control(pDX, IDC_RADIO_MINUTES, m_radioMinutes);
	DDX_Control(pDX, IDC_EDIT_ASSISTING_UNIT_FEE, m_editUnitFee);
}

BEGIN_MESSAGE_MAP(CAssistingCodesSetupDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDC_BTN_ADD_NEW_ASSISTING_UNIT_OPTION, OnBtnAddNewAssistingUnitOption)
	ON_BN_CLICKED(IDC_BTN_DELETE_ASSISTING_UNIT_OPTION, OnBtnDeleteAssistingUnitOption)
	ON_EN_KILLFOCUS(IDC_EDIT_ASSISTING_UNIT_FEE, OnEnKillfocusEditAssistingUnitFee)
END_MESSAGE_MAP()


// CAssistingCodesSetupDlg message handlers

BOOL CAssistingCodesSetupDlg::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnDelete.AutoSet(NXB_DELETE);

		//limit this to something reasonable, it is never going to be over $100.00 anyways
		m_editUnitFee.SetLimitText(10);

		g_propManager.CachePropertiesInBulk("CAssistingCodesSetupDlg-Number", propNumber,
				"(Username = '<None>' OR Username = '%s') AND ("
				"Name = 'OHIPAssistingCode_TimePrompt' "
				")",
				_Q(GetCurrentUserName()));

		g_propManager.CachePropertiesInBulk("CAssistingCodesSetupDlg-Text", propText,
				"(Username = '<None>' OR Username = '%s') AND ("
				"Name = 'OHIPAssistingCode_BasePrice' "
				")",
				_Q(GetCurrentUserName()));

		m_UnitList = BindNxDataList2Ctrl(IDC_ASSISTING_UNIT_TABLE, true);

		//default value is $11.58, as of October 2010
		CString strUnitFee = GetRemotePropertyText("OHIPAssistingCode_BasePrice", "$11.58", 0, "<None>", true);
		COleCurrency cyUnitFee;
		cyUnitFee.ParseCurrency(strUnitFee);
		strUnitFee = FormatCurrencyForInterface(cyUnitFee);
		GetDlgItem(IDC_EDIT_ASSISTING_UNIT_FEE)->SetWindowText(strUnitFee);

		long nTimePrompt = GetRemotePropertyInt("OHIPAssistingCode_TimePrompt", 0, 0, "<None>", true);
		//0 - times, 1 - minutes
		m_radioTimes.SetCheck(nTimePrompt == 0);
		m_radioMinutes.SetCheck(nTimePrompt == 1);

	}NxCatchAll(__FUNCTION__);

	return FALSE;
}

void CAssistingCodesSetupDlg::OnOK()
{
	try {

		CString strUnitFee;
		//this function will validate the fee, warn if it is invalid,
		//and replace the value on screen with what will eventually be
		//saved if not corrected
		if(!GetValidatedUnitFee(strUnitFee)) {
			//the user typed in an invalid fee, and they were warned about it
			return;
		}

		//require entries in the table
		if(m_UnitList->GetRowCount() == 0) {
			AfxMessageBox("There must be at least one entry in the time unit list.");
			return;
		}
		
		//save the unit table
		if(m_bUnitTableChanged) {

			CString strSqlBatch;
			CNxParamSqlArray aryParams;

			//clear the existing values
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM AssistingCodesSetupT");

			IRowSettingsPtr pRow = m_UnitList->GetFirstRow();
			while(pRow) {

				long nHours = VarLong(pRow->GetValue(ulcHours), 0);
				long nMinutes = VarLong(pRow->GetValue(ulcMinutes), 0);
				long nUnits = VarLong(pRow->GetValue(ulcUnits), 0);

				long nTotalMinutes = (nHours * 60) + nMinutes;
				AddParamStatementToSqlBatch(strSqlBatch, aryParams, "INSERT INTO AssistingCodesSetupT (StartAfterMinute, Units) "
					"VALUES ({INT}, {INT})", nTotalMinutes, nUnits);

				pRow = pRow->GetNextRow();
			}

			ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);
		}		

		SetRemotePropertyText("OHIPAssistingCode_BasePrice", strUnitFee, 0, "<None>");

		//0 - times, 1 - minutes
		long nTimePrompt = 0;
		if(m_radioMinutes.GetCheck()) {
			nTimePrompt = 1;
		}		
		SetRemotePropertyInt("OHIPAssistingCode_TimePrompt", nTimePrompt, 0, "<None>");

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}

void CAssistingCodesSetupDlg::OnBtnAddNewAssistingUnitOption()
{
	try {

		//add a new entry of a reasonable size
		//if the list is empty, add 0:00, 1 unit,
		//otherwise just add a 60 minute & 1 unit increment
		//to the highest value

		long nHours = 0;
		long nMinutes = 0;
		long nUnits = 1;
		if(m_UnitList->GetRowCount() != 0) {
			//there are entries, as there should be,
			//so grab the last row
			IRowSettingsPtr pRow = m_UnitList->GetLastRow();
			nHours = VarLong(pRow->GetValue(ulcHours), 0);
			nMinutes = VarLong(pRow->GetValue(ulcMinutes), 0);
			nUnits = VarLong(pRow->GetValue(ulcUnits), 0);

			//add 1 unit
			nUnits++;

			//add 1 hour
			nHours++;

			BOOL bValidTime = FALSE;
			//should be impossible for this to be invalid, but check anyways
			while(!bValidTime) {

				//assume valid until proven otherwise
				bValidTime = TRUE;

				IRowSettingsPtr pFindRow = m_UnitList->GetFirstRow();
				while(pFindRow) {
					long nCheckHours = VarLong(pFindRow->GetValue(ulcHours), 0);
					long nCheckMinutes = VarLong(pFindRow->GetValue(ulcMinutes), 0);
					if(nCheckHours == nHours && nCheckMinutes == nMinutes) {
						//this time already exists
						bValidTime = FALSE;
					}
					pFindRow = pFindRow->GetNextRow();
				}

				if(!bValidTime) {
					//increment by 1 hour
					nHours++;
				}
			}
		}

		//create the new row
		IRowSettingsPtr pRow = m_UnitList->GetNewRow();
		pRow->PutValue(ulcHours, nHours);
		pRow->PutValue(ulcMinutes, nMinutes);
		pRow->PutValue(ulcUnits, nUnits);
		m_UnitList->AddRowSorted(pRow, NULL);

		//start editing the row
		m_UnitList->StartEditing(pRow, ulcHours);

		m_bUnitTableChanged = TRUE;

	}NxCatchAll(__FUNCTION__);
}

void CAssistingCodesSetupDlg::OnBtnDeleteAssistingUnitOption()
{
	try {

		IRowSettingsPtr pRow = m_UnitList->GetCurSel();
		if(pRow == NULL) {
			AfxMessageBox("You must first select an entry from the time unit list to delete.");
		}
		else if(MessageBox("Are you sure you wish to delete this time unit entry?", "Practice", MB_ICONQUESTION|MB_YESNO) == IDNO) {
			return;
		}

		//remove the row
		m_UnitList->RemoveRow(pRow);

		m_bUnitTableChanged = TRUE;

	}NxCatchAll(__FUNCTION__);
}

void CAssistingCodesSetupDlg::OnEnKillfocusEditAssistingUnitFee()
{
	try {

		CString strFormattedUnitFee;
		//this function will validate the fee, warn if it is invalid,
		//and replace the value on screen with what will eventually be
		//saved if not corrected
		GetValidatedUnitFee(strFormattedUnitFee);

	} NxCatchAll(__FUNCTION__);
}

//this function will validate the fee, warn if it is invalid,
//and replace the value on screen with what will eventually be
//saved if not corrected, returns TRUE if the fee is valid,
//returns FALSE if a warning was given
BOOL CAssistingCodesSetupDlg::GetValidatedUnitFee(OUT CString &strFormattedUnitFee)
{
	//ensure that the currency is displayed as it will be saved

	GetDlgItemText(IDC_EDIT_ASSISTING_UNIT_FEE, strFormattedUnitFee);
	COleCurrency cyUnitFee;
	if(!cyUnitFee.ParseCurrency(strFormattedUnitFee) || cyUnitFee.GetStatus() == COleCurrency::invalid) {
		cyUnitFee = COleCurrency(0,0);
		strFormattedUnitFee = FormatCurrencyForInterface(cyUnitFee);
		GetDlgItem(IDC_EDIT_ASSISTING_UNIT_FEE)->SetWindowText(strFormattedUnitFee);
		AfxMessageBox("Please enter a valid unit fee.");
		return FALSE;
	}

	strFormattedUnitFee = FormatCurrencyForInterface(cyUnitFee);		
	
	if(strFormattedUnitFee.GetLength() > 10) {
		cyUnitFee = COleCurrency(0,0);
		strFormattedUnitFee = FormatCurrencyForInterface(cyUnitFee);
		GetDlgItem(IDC_EDIT_ASSISTING_UNIT_FEE)->SetWindowText(strFormattedUnitFee);
		AfxMessageBox("The fee you have entered is too large. Please enter a smaller unit fee.");
		return FALSE;
	}

	GetDlgItem(IDC_EDIT_ASSISTING_UNIT_FEE)->SetWindowText(strFormattedUnitFee);

	return TRUE;
}

BEGIN_EVENTSINK_MAP(CAssistingCodesSetupDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CAssistingCodesSetupDlg)
	ON_EVENT(CAssistingCodesSetupDlg, IDC_ASSISTING_UNIT_TABLE, 9 /* EditingFinishing */, OnEditingFinishingUnitList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CAssistingCodesSetupDlg, IDC_ASSISTING_UNIT_TABLE, 10 /* EditingFinished */, OnEditingFinishedUnitList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CAssistingCodesSetupDlg, IDC_ASSISTING_UNIT_TABLE, 6 /* RButtonDown */, OnRButtonDownUnitList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CAssistingCodesSetupDlg::OnEditingFinishingUnitList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow = lpRow;
		if(pRow == NULL) {
			return;
		}

		switch(nCol) {
			case ulcHours: {

				if(pvarNewValue->vt != VT_I4) {					
					*pbCommit = FALSE;
					AfxMessageBox("You must enter a valid value for hours.");
					return;
				}

				long nHours = VarLong(pvarNewValue);

				//limit to a maximum of 1000
				if(nHours > 1000) {					
					*pbCommit = FALSE;
					AfxMessageBox("The value for hours cannot be greater than 1000.");
					return;
				}
				else if(nHours < 0) {
					*pbCommit = FALSE;
					AfxMessageBox("The value for hours cannot be less than zero.");
					return;
				}

				//now validate that it doesn't already exist
				long nMinutes = VarLong(pRow->GetValue(ulcMinutes), 0);

				IRowSettingsPtr pFindRow = m_UnitList->GetFirstRow();
				while(pFindRow) {
					if(pFindRow != pRow) {
						long nCheckHours = VarLong(pFindRow->GetValue(ulcHours), 0);
						long nCheckMinutes = VarLong(pFindRow->GetValue(ulcMinutes), 0);
						if(nCheckHours == nHours && nCheckMinutes == nMinutes) {
							*pbCommit = FALSE;
							AfxMessageBox("An entry for this length of time already exists in the list.");
							return;
						}
					}
					pFindRow = pFindRow->GetNextRow();
				}

				break;
				}

			case ulcMinutes: {

				if(pvarNewValue->vt != VT_I4) {					
					*pbCommit = FALSE;
					AfxMessageBox("You must enter a valid value for hours.");
					return;
				}

				long nMinutes = VarLong(pvarNewValue);

				if(nMinutes > 59) {
					*pbCommit = FALSE;
					AfxMessageBox("The value for minutes cannot be greater than 59.");
					return;
				}
				else if(nMinutes < 0) {
					*pbCommit = FALSE;
					AfxMessageBox("The value for minutes cannot be less than zero.");
					return;
				}

				//now validate that it doesn't already exist
				long nHours = VarLong(pRow->GetValue(ulcHours), 0);

				IRowSettingsPtr pFindRow = m_UnitList->GetFirstRow();
				while(pFindRow) {
					if(pFindRow != pRow) {
						long nCheckHours = VarLong(pFindRow->GetValue(ulcHours), 0);
						long nCheckMinutes = VarLong(pFindRow->GetValue(ulcMinutes), 0);
						if(nCheckHours == nHours && nCheckMinutes == nMinutes) {
							*pbCommit = FALSE;
							AfxMessageBox("An entry for this length of time already exists in the list.");
							return;
						}
					}
					pFindRow = pFindRow->GetNextRow();
				}

				break;
				}

			case ulcUnits: {

				if(pvarNewValue->vt != VT_I4) {					
					*pbCommit = FALSE;
					AfxMessageBox("You must enter a valid unit value.");
					return;
				}

				long nUnits = VarLong(pvarNewValue);

				//limit to a maximum of 1000
				if(nUnits > 1000) {			
					*pbCommit = FALSE;
					AfxMessageBox("The value for units cannot be greater than 1000.");
					return;
				}
				//technically, it should never be zero, but there's no real reason to disallow it
				else if(nUnits < 0) {
					*pbCommit = FALSE;
					AfxMessageBox("The value for units cannot be less than zero.");
					return;
				}

				break;
			}				
		}

	}NxCatchAll(__FUNCTION__);
}

void CAssistingCodesSetupDlg::OnEditingFinishedUnitList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow = lpRow;
		if(pRow == NULL) {
			return;
		}

		if(!bCommit) {
			return;
		}

		//every field in this table is VT_I4, and technically non-nullable at that
		if(VarLong(varOldValue, 0) != VarLong(varNewValue, 0)) {
			m_bUnitTableChanged = TRUE;
		}

		m_UnitList->Sort();

	}NxCatchAll(__FUNCTION__);
}

void CAssistingCodesSetupDlg::OnRButtonDownUnitList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		m_UnitList->PutCurSel(pRow);

		enum MenuItem
		{
			miDelete = 1,
		};

		CMenu mnu;
		mnu.CreatePopupMenu();
		mnu.AppendMenu(MF_ENABLED, miDelete, "&Delete This Unit Option");

		CPoint pt;
		GetCursorPos(&pt);
		int nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD, pt.x, pt.y, this);

		switch(nResult)
		{
			case miDelete:
			{
				OnBtnDeleteAssistingUnitOption();
			}
			default:
				break;
		}	

	}NxCatchAll(__FUNCTION__);
}