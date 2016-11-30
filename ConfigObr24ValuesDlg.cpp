// ConfigObr24ValuesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FinancialRc.h"
#include "ConfigObr24ValuesDlg.h"

//TES 7/30/2010 - PLID 39908 - Created
// CConfigObr24ValuesDlg dialog

IMPLEMENT_DYNAMIC(CConfigObr24ValuesDlg, CNxDialog)

CConfigObr24ValuesDlg::CConfigObr24ValuesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CConfigObr24ValuesDlg::IDD, pParent)
{

}

CConfigObr24ValuesDlg::~CConfigObr24ValuesDlg()
{
}

void CConfigObr24ValuesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ADD_OBR_24_VALUE, m_nxbAdd);
	DDX_Control(pDX, IDC_REMOVE_OBR_24_VALUE, m_nxbRemove);
	DDX_Control(pDX, IDOK, m_nxbOK);
	DDX_Control(pDX, IDCANCEL, m_nxbCancel);
}


BEGIN_MESSAGE_MAP(CConfigObr24ValuesDlg, CNxDialog)
	ON_BN_CLICKED(IDC_ADD_OBR_24_VALUE, &CConfigObr24ValuesDlg::OnAddObr24Value)
	ON_BN_CLICKED(IDC_REMOVE_OBR_24_VALUE, &CConfigObr24ValuesDlg::OnRemoveObr24Value)
END_MESSAGE_MAP()

using namespace NXDATALIST2Lib;
// CConfigObr24ValuesDlg message handlers
BOOL CConfigObr24ValuesDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();
	try {
		//TES 7/30/2010 - PLID 39908 - Set the caption.
		SetDlgItemText(IDC_CONFIG_OBR_24_VALUES_LABEL, "This dialog allows you to specify whether text segments in Lab Result messages should be combined.  Any messages with one of the specified values in the OBR-24 field will have its segments combined into a single result; all other messages will treat each segment as a separate result.  These values should only be changed when working with NexTech Technical Support.");

		//TES 7/30/2010 - PLID 39908 - AutoSet NxIconButtons.
		m_nxbAdd.AutoSet(NXB_NEW);
		m_nxbRemove.AutoSet(NXB_DELETE);
		m_nxbOK.AutoSet(NXB_OK);
		m_nxbCancel.AutoSet(NXB_CANCEL);

		//TES 7/30/2010 - PLID 39908 - Load the values we were given into the datalist.
		m_pValuesList = BindNxDataList2Ctrl(IDC_OBR_24_VALUES, false);
		for(int i = 0; i < m_saValues.GetSize(); i++) {
			IRowSettingsPtr pRow = m_pValuesList->GetNewRow();
			pRow->PutValue(0, _bstr_t(m_saValues[i]));
			m_pValuesList->AddRowAtEnd(pRow, NULL);
		}

	}NxCatchAll(__FUNCTION__);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
void CConfigObr24ValuesDlg::OnAddObr24Value()
{
	try {
		//TES 7/30/2010 - PLID 39908 - Loop until they add a valid value, or cancel.
		bool bAdded = false, bCancelled = false;
		while(!bAdded && !bCancelled) {
			CString strValue;
			if(IDOK != InputBox(this, "Please enter an OBR-24 value", strValue, "")) {
				bCancelled = true;
			}
			else {
				//TES 7/30/2010 - PLID 39908 - Note that we don't check for empty here; empty is a valid value.
				//TES 7/30/2010 - PLID 39908 - Does this value have a pipe?
				if(strValue.Find('|') != -1) {
					//TES 7/30/2010 - PLID 39908 - We use that for delimiting the values in data, plus it wouldn't work anyway
					// since it's a reserved character for HL7, so don't let them add it.
					MsgBox("The '|' character is reserved for use by HL7, and cannot be used in an OBR-24 field.");
				}
				else {
					//TES 7/30/2010 - PLID 39908 - Does this value already exist in the list?
					bool bFound = false;
					IRowSettingsPtr pRow = m_pValuesList->GetFirstRow();
					while(pRow && !bFound) {
						CString strExistingValue = VarString(pRow->GetValue(0));
						if(strExistingValue.CompareNoCase(strValue) == 0) {
							bFound = true;
						}
						pRow = pRow->GetNextRow();
					}
					if(bFound) {
						MsgBox("The value you entered is already in the list.");
					}
					else {
						//TES 7/30/2010 - PLID 39908 - Well, looks like it's valid, add it to the list.
						IRowSettingsPtr pRow = m_pValuesList->GetNewRow();
						pRow->PutValue(0, _bstr_t(strValue));
						m_pValuesList->AddRowAtEnd(pRow, NULL);
						bAdded = true;
					}
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CConfigObr24ValuesDlg::OnRemoveObr24Value()
{
	try {
		if(m_pValuesList->CurSel) {
			m_pValuesList->RemoveRow(m_pValuesList->CurSel);
		}
	}NxCatchAll(__FUNCTION__);
}
BEGIN_EVENTSINK_MAP(CConfigObr24ValuesDlg, CNxDialog)
	ON_EVENT(CConfigObr24ValuesDlg, IDC_OBR_24_VALUES, 2, CConfigObr24ValuesDlg::OnSelChangedObr24Values, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CConfigObr24ValuesDlg, IDC_OBR_24_VALUES, 9, CConfigObr24ValuesDlg::OnEditingFinishingObr24Values, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
END_EVENTSINK_MAP()

void CConfigObr24ValuesDlg::OnSelChangedObr24Values(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		if(lpNewSel) {
			m_nxbRemove.EnableWindow(TRUE);
		}
		else {
			m_nxbRemove.EnableWindow(FALSE);
		}
	}NxCatchAll(__FUNCTION__);
}

void CConfigObr24ValuesDlg::OnOK()
{
	try {
		//TES 7/30/2010 - PLID 39908 - Load the values into our CStringArray.  Also track the total length of the string, the database
		// only allows 1000 characters (which is way more than anyone would ever need).
		CString strValues;
		m_saValues.RemoveAll();
		IRowSettingsPtr pRow = m_pValuesList->GetFirstRow();
		while(pRow) {
			CString strValue = VarString(pRow->GetValue(0));
			strValues += strValue + "|";
			m_saValues.Add(strValue);
			pRow = pRow->GetNextRow();
		}
		if(strValues.GetLength() > 1000) {
			MsgBox("You have entered more values than are supported.  Please reduce the size of your list to a total of less than 1000 characters.");
			return;
		}

		CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__);
}

void CConfigObr24ValuesDlg::OnEditingFinishingObr24Values(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {
		if(*pbCommit) {
			//TES 8/11/2010 - PLID 39908 - Does it have a '|' in it?
			CString strNew = strUserEntered;
			if(strNew.Find("|") != -1) {
				MsgBox("The '|' character is reserved for use by HL7, and cannot be used in an OBR-24 field.");
				*pbCommit = FALSE;
				*pbContinue = FALSE;
				return;
			}
			//TES 7/30/2010 - PLID 39908 - Does this value already exist in the list?
			IRowSettingsPtr pRow(lpRow);
			bool bFound = false;
			IRowSettingsPtr p = m_pValuesList->GetFirstRow();
			while(p && !bFound) {
				if(p != pRow) {
					CString strExistingValue = VarString(p->GetValue(0));
					if(strExistingValue.CompareNoCase(strNew) == 0) {
						bFound = true;
					}
				}
				p = p->GetNextRow();
			}
			if(bFound) {
				MsgBox("The value you entered is already in the list.");
				*pbCommit = FALSE;
				*pbContinue = FALSE;
				return;
			}
		}
	}NxCatchAll(__FUNCTION__);
}
