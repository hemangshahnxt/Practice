// TopsFieldPickDlg.cpp : implementation file
//
// (j.gruber 2009-11-20 17:09) - PLID 36139 - created for

#include "stdafx.h"
#include "Practice.h"
#include "TopsFieldPickDlg.h"
#include "FinancialRc.h"
#include "InternationalUtils.h"


enum TopsListColumns {
	tlcCode = 0,
	tlcDescription,
	tlcSelection,
	tlcValue,
};

// CTopsFieldPickDlg dialog

IMPLEMENT_DYNAMIC(CTopsFieldPickDlg, CNxDialog)

CTopsFieldPickDlg::CTopsFieldPickDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CTopsFieldPickDlg::IDD, pParent)
{

}

CTopsFieldPickDlg::~CTopsFieldPickDlg()
{
}

void CTopsFieldPickDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DESCRIPTION_STATIC, m_stDescription);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CTopsFieldPickDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CTopsFieldPickDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CTopsFieldPickDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CTopsFieldPickDlg message handlers
BOOL CTopsFieldPickDlg::OnInitDialog() 
{
	
	try {
		CNxDialog::OnInitDialog();

		//bind the list
		m_pList = BindNxDataList2Ctrl(IDC_TOPS_FIELD_LIST, false);

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		CString strDesc;
		strDesc.Format("There are multiple EMN items and/or values "
			"that correspond to the following TOPS information. "
			"Please choose the EMN and/or values to pull the information "
			"from for each code listed for.");
		m_stDescription.SetWindowTextA(strDesc);
		
		CString strTitle;
		strTitle.Format("Choose TOPS Values for %s on %s", m_strPatientName, FormatDateTimeForInterface(m_dtProcDate));
		this->SetWindowTextA(strTitle);

		//load the list from the array
		LoadList();

	}NxCatchAll(__FUNCTION__);
	return TRUE;
}



void CTopsFieldPickDlg::LoadList() 
{

	try {

		for (int i = 0; i < m_paryTopsList->GetSize(); i++) {

			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->GetNewRow();

			CString strCode = m_paryTopsList->GetAt(i)->strCode;
			CString strCodeDesc = m_paryTopsList->GetAt(i)->strCodeDesc;

			pRow->PutValue(tlcCode, _variant_t(strCode));
			pRow->PutValue(tlcDescription, _variant_t(strCodeDesc));
			CArray<TOPSField*, TOPSField*> *aryTOPS = m_paryTopsList->GetAt(i)->aryTOPS;

			CString strComboSource;
			
			//now fill our description column
			for (int j = 0; j < aryTOPS->GetSize(); j++) {

				CString strRow = aryTOPS->GetAt(j)->strRow;
				long nItemID = aryTOPS->GetAt(j)->nItemID;

				//get rid of any semicolon's that may exist
				strRow.Remove(';');

				CString strTemp;
				strTemp.Format("%li;%s;", nItemID, strRow);

				strComboSource += strTemp;			

			}

			NXDATALIST2Lib::IFormatSettingsPtr pfs(__uuidof(NXDATALIST2Lib::FormatSettings));
			pfs->PutDataType(VT_I4);
			pfs->PutFieldType(NXDATALIST2Lib::cftComboSimple);
			pfs->PutEditable(VARIANT_TRUE);
			pfs->PutConnection(_variant_t((LPDISPATCH)NULL)); //we're going to let this combo use Practice's connection
			pfs->PutComboSource(_bstr_t(strComboSource));

			pRow->PutRefCellFormatOverride(tlcSelection, pfs);

			if (aryTOPS->GetSize() == 1) {

				//check to see if we need to add this
				//CStringArray strAry = aryTOPS->GetAt(0)->arySelections;
				if (aryTOPS->GetAt(0)->parySelections->GetSize() != 1 && (!m_paryTopsList->GetAt(i)->bAllowMultiples)) {
					//add it
					m_pList->AddRowAtEnd(pRow, NULL);					

					//we only have one, so set it as the selection
					pRow->PutValue(tlcSelection, aryTOPS->GetAt(0)->nItemID);				
					pfs->PutEditable(VARIANT_FALSE);
					pRow->PutRefCellFormatOverride(tlcSelection, pfs);
					LoadValuesList(pRow, aryTOPS->GetAt(0)->nItemID);
				}						
				
			}
			else {
				m_pList->AddRowAtEnd(pRow, NULL);
			}
		}

	}NxCatchAll(__FUNCTION__);
}


BEGIN_EVENTSINK_MAP(CTopsFieldPickDlg, CNxDialog)
ON_EVENT(CTopsFieldPickDlg, IDC_TOPS_FIELD_LIST, 10, CTopsFieldPickDlg::EditingFinishedTopsFieldList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
END_EVENTSINK_MAP()


void CTopsFieldPickDlg::LoadValuesList(LPDISPATCH lpRow, long nRowItemID)
{
	try {

		//loop through the list and look for this itemID for this code, then load the values
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {

			CString strRowCode = VarString(pRow->GetValue(tlcCode));
		
			for (int i = 0; i < m_paryTopsList->GetSize(); i++) {

				CString strCode = m_paryTopsList->GetAt(i)->strCode;
				BOOL bAllowMultiples = m_paryTopsList->GetAt(i)->bAllowMultiples;

				
				if (strCode  == strRowCode) {
					if (bAllowMultiples) {
						CString strComboSource = "<Multiple Values Allowed>;<Multiple Values Allowed>;";

						NXDATALIST2Lib::IFormatSettingsPtr pfs(__uuidof(NXDATALIST2Lib::FormatSettings));
						pfs->PutDataType(VT_BSTR);
						pfs->PutFieldType(NXDATALIST2Lib::cftComboSimple);					
						pfs->PutEditable(VARIANT_FALSE);
						pfs->PutConnection(_variant_t((LPDISPATCH)NULL)); //we're going to let this combo use Practice's connection
						pfs->PutComboSource(_bstr_t(strComboSource));				

						pRow->PutRefCellFormatOverride(tlcValue, pfs);

						pRow->PutValue(tlcValue, variant_t("<Multiple Values Allowed>"));
					}
					else {

						CArray<TOPSField*, TOPSField*> *aryTOPS = m_paryTopsList->GetAt(i)->aryTOPS;

						CString strComboSource;
					
						
						//now find our ItemID
						for (int j = 0; j < aryTOPS->GetSize(); j++) {

							long nItemID = aryTOPS->GetAt(j)->nItemID;

							if (nItemID == nRowItemID) {

								//fill the values

								NXDATALIST2Lib::IFormatSettingsPtr pfs(__uuidof(NXDATALIST2Lib::FormatSettings));
								pfs->PutDataType(VT_BSTR);
								pfs->PutFieldType(NXDATALIST2Lib::cftComboSimple);								
								pfs->PutConnection(_variant_t((LPDISPATCH)NULL)); //we're going to let this combo use Practice's connection
								

								for (int k = 0; k <  aryTOPS->GetAt(j)->parySelections->GetSize(); k++) {

									CString strValue =  aryTOPS->GetAt(j)->parySelections->GetAt(k)->strSelection;

									//get rid of any semicolon's that may exist
									strValue.Remove(';');

									CString strTemp;
									strTemp.Format("%s;%s;", strValue, strValue);

									strComboSource += strTemp;			

									pfs->PutEditable(VARIANT_TRUE);
								}	
								pfs->PutComboSource(_bstr_t(strComboSource));

								if (aryTOPS->GetAt(j)->parySelections->GetSize() == 1) {

									//there is only 1, just fill it
									pfs->PutEditable(VARIANT_FALSE);
									CString strValue = aryTOPS->GetAt(j)->parySelections->GetAt(0)->strSelection;
									pRow->PutValue(tlcValue, _variant_t(strValue));
								}
							
								
								pRow->PutRefCellFormatOverride(tlcValue, pfs);
							}
						}
					}					
				}
			}		
		}
	}NxCatchAll(__FUNCTION__);
}

void CTopsFieldPickDlg::EditingFinishedTopsFieldList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {

		if (bCommit) {
		
			switch (nCol) {

				case tlcSelection:

					if (varNewValue.vt == VT_I4) {
					
						LoadValuesList(lpRow, VarLong(varNewValue));
					}
				break;
			}
		}

	}NxCatchAll(__FUNCTION__);
}

void CTopsFieldPickDlg::OnBnClickedOk()
{
	try {
		//make sure everything is filled in

		//to do this, we just need to loop though the values and make sure something exists
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->GetFirstRow();

		BOOL bValidated = TRUE;
		while (pRow) {

			_variant_t varValue = pRow->GetValue(tlcValue);
			if (varValue.vt != VT_BSTR) {
				bValidated = FALSE;
			}
			pRow = pRow->GetNextRow();
		}

		if (!bValidated) {
			if (IDNO == MsgBox(MB_YESNO, "Not all selections have been filled out. This request will not include the TOPS fields that still need a selection made, are you sure you want to continue?")) {
//				MsgBox("Please choose a value for every applicable row before clicking OK");
				return;
			}
		}

		pRow = m_pList->GetFirstRow();
		while (pRow) {

			//only add to the map if there is a valid value
			_variant_t varValue = pRow->GetValue(tlcValue);
			if (varValue.vt == VT_BSTR) {
				CString strValue = VarString(pRow->GetValue(tlcValue));
				CString strCode = VarString(pRow->GetValue(tlcCode));
				if (strValue == "<Multiple Values Allowed>") {
					//look up the itemID
					strValue = AsString(pRow->GetValue(tlcSelection));
				}
				m_pmapReturnValues->SetAt(strCode, strValue);
			}
			pRow = pRow->GetNextRow();
		}

		CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__);

}

void CTopsFieldPickDlg::OnBnClickedCancel()
{
	try {
		if (IDNO == MsgBox(MB_YESNO, "There are selections that must be made before this request can be sent to TOPS, by cancelling, this request will not include any TOPS fields that need a selection made, are you sure you want to continue?")) {
			return;
		}
		CNxDialog::OnCancel();
	}NxCatchAll(__FUNCTION__);
}
