// BillClaimFieldsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BillClaimFieldsDlg.h"

// CBillClaimFieldsDlg dialog

// (j.jones 2013-08-13 11:12) - PLID 57902 - created
// (a.walling 2016-03-10 14:51) - PLID 68561 - UB04 Enhancements - all UB handling in CUB04AdditionalFieldsDlg

using namespace NXDATALIST2Lib;

enum ClaimFieldColumns {

	cfcFieldID = 0,
	cfcFieldName,
	cfcValue,
};

//all possible fields on this dialog
enum FieldType {

	eftInvalid = 0,
	eftHCFABox8,
	eftHCFABox9b,
	eftHCFABox9c,
	eftHCFABox10d,
	eftHCFABox11bQual,
	eftHCFABox11b,

	// (a.walling 2016-03-09 16:21) - PLID 68561 - UB04 Enhancements - remove old UI and codebehind for UB04 boxes 31-36
};

IMPLEMENT_DYNAMIC(CBillClaimFieldsDlg, CNxDialog)

CBillClaimFieldsDlg::CBillClaimFieldsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CBillClaimFieldsDlg::IDD, pParent)
{
	m_bReadOnly = false;
	m_bUseNewForm = false;
	m_bChanged = false;
}

CBillClaimFieldsDlg::~CBillClaimFieldsDlg()
{
}

void CBillClaimFieldsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CBillClaimFieldsDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MESSAGE_MAP()

// CBillClaimFieldsDlg message handlers

BOOL CBillClaimFieldsDlg::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_ClaimFieldList = BindNxDataList2Ctrl(IDC_CLAIM_FIELD_LIST, false);

		//fill our list with fields for this form type
		SetWindowText("Additional HCFA Claim Fields");
		AddHCFAFields();

		if(m_bReadOnly) {
			m_ClaimFieldList->PutReadOnly(VARIANT_TRUE);
		}

	}NxCatchAll(__FUNCTION__);

	return TRUE;	// return TRUE unless you set the focus to a control
					// EXCEPTION: OCX Property Pages should return FALSE
}

void CBillClaimFieldsDlg::AddHCFAFields()
{
	if(m_bUseNewForm) {
		//some of these fields are on the new HCFA, and not the old one,
		//and others have their names changed
		AddFieldRow(eftHCFABox8, "HCFA Box 8", m_strHCFABox8);
		AddFieldRow(eftHCFABox9b, "HCFA Box 9b", m_strHCFABox9b);
		AddFieldRow(eftHCFABox9c, "HCFA Box 9c", m_strHCFABox9c);
		AddFieldRow(eftHCFABox10d, "HCFA Box 10d: Claim Codes", m_strHCFABox10d);
		AddFieldRow(eftHCFABox11bQual, "HCFA Box 11b Qualifier", m_strHCFABox11bQual);
		AddFieldRow(eftHCFABox11b, "HCFA Box 11b", m_strHCFABox11b);
	}
	else {
		//this field is on both forms but has no special name on the old one
		AddFieldRow(eftHCFABox10d, "HCFA Box 10d", m_strHCFABox10d);
	}
}

void CBillClaimFieldsDlg::AddFieldRow(FieldType eFieldType, CString strFieldName, CString strValue)
{
	IRowSettingsPtr pRow = m_ClaimFieldList->GetNewRow();
	pRow->PutValue(cfcFieldID, (long)eFieldType);
	pRow->PutValue(cfcFieldName, (LPCTSTR)strFieldName);
	pRow->PutValue(cfcValue, (LPCTSTR)strValue);
	m_ClaimFieldList->AddRowAtEnd(pRow, NULL);
}

BEGIN_EVENTSINK_MAP(CBillClaimFieldsDlg, CNxDialog)
ON_EVENT(CBillClaimFieldsDlg, IDC_CLAIM_FIELD_LIST, 9, OnEditingFinishingClaimFieldList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
ON_EVENT(CBillClaimFieldsDlg, IDC_CLAIM_FIELD_LIST, 10, OnEditingFinishedClaimFieldList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
END_EVENTSINK_MAP()

void CBillClaimFieldsDlg::OnEditingFinishingClaimFieldList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		if(nCol == cfcValue) {

			if(strUserEntered == "") {
				//blank is fine
				return;
			}

			long nLengthMax = -1;

			FieldType eFieldID = (FieldType)VarLong(pRow->GetValue(cfcFieldID));
			switch(eFieldID) {
				case eftHCFABox8:
				case eftHCFABox9b:
				case eftHCFABox9c:
				case eftHCFABox10d:
				case eftHCFABox11b:
					nLengthMax = 50;
					break;
				case eftHCFABox11bQual:
					nLengthMax = 2;
					break;
				default:
					//what value is this?
					ASSERT(FALSE);
					break;
			}

			CString strEntered = strUserEntered;
			if(nLengthMax != -1 && strEntered.GetLength() > nLengthMax) {
				*pbContinue = FALSE;
				CString str;
				str.Format("The value you entered is too long. Please enter a value that is %li characters or less.", nLengthMax);
				AfxMessageBox(str);
			}
		}

	}NxCatchAll(__FUNCTION__);
}

void CBillClaimFieldsDlg::OnEditingFinishedClaimFieldList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {

		if(!bCommit) {
			return;
		}

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		if(nCol == cfcValue) {

			//update the member variable with their changes
			FieldType eFieldID = (FieldType)VarLong(pRow->GetValue(cfcFieldID));
			CString strNewValue = VarString(varNewValue, "");
			switch(eFieldID) {
				case eftHCFABox8:
					m_strHCFABox8 = strNewValue;
					break;
				case eftHCFABox9b:
					m_strHCFABox9b = strNewValue;
					break;
				case eftHCFABox9c:
					m_strHCFABox9c = strNewValue;
					break;
				case eftHCFABox10d:
					m_strHCFABox10d = strNewValue;
					break;
				case eftHCFABox11bQual:
					m_strHCFABox11bQual = strNewValue;
					break;
				case eftHCFABox11b:
					m_strHCFABox11b = strNewValue;
					break;
				// (a.walling 2016-03-09 16:21) - PLID 68561 - UB04 Enhancements - remove old UI and codebehind for UB04 boxes 31-36
				default:
					//what value is this?
					ASSERT(FALSE);
					break;
			}

			m_bChanged = true;
		}

	}NxCatchAll(__FUNCTION__);
}

void CBillClaimFieldsDlg::OnOK()
{
	try {

		//the member variables were updated with their changes once they finished editing

		//Warn if they enter an 11b Qualifer or 11b number, and not the opposite,
		//but only if these fields are actually displayed, and editable.
		//Continue warning even if they didn't change anything this time, because
		//they really should not have done this.
		if(m_bUseNewForm && !m_bReadOnly) {
			if(m_strHCFABox11bQual.IsEmpty() != m_strHCFABox11b.IsEmpty()) {
				if(IDNO == MessageBox("The Box 11b qualifier and number should either both be blank, or both filled.\n\n"
					"Are you sure you wish to save this invalid data?",
					"Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
					return;
				}
			}
		}

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}

void CBillClaimFieldsDlg::OnCancel()
{
	try {

		if(m_bChanged) {
			if(IDNO == MessageBox("You have made changes to these fields. If you cancel, your changes will not be saved.\n\n"
				"Are you sure you wish to cancel your changes?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				
				return;
			}
		}

		CNxDialog::OnCancel();

	}NxCatchAll(__FUNCTION__);
}
