// BoldFieldPickDlg.cpp : implementation file
//

// (j.gruber 2010-06-02 16:33) - PLID 38538 - created

#include "stdafx.h"
#include "Practice.h"
#include "BoldFieldPickDlg.h"
#include "FinancialRc.h"
#include "DateTimeUtils.h"
#include "InternationalUtils.h"



enum BoldListColumns {
	tlcCode = 0,
	tlcDescription,
	tlcSelection,
	tlcValue,
	tlcValueCode,
};


// CBoldFieldPickDlg dialog

IMPLEMENT_DYNAMIC(CBoldFieldPickDlg, CNxDialog)

CBoldFieldPickDlg::CBoldFieldPickDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CBoldFieldPickDlg::IDD, pParent)
{

}

CBoldFieldPickDlg::~CBoldFieldPickDlg()
{
}

void CBoldFieldPickDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BOLD_DESCRIPTION_STATIC, m_stDescription);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CBoldFieldPickDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CBoldFieldPickDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CBoldFieldPickDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CBoldFieldPickDlg message handlers
BOOL CBoldFieldPickDlg::OnInitDialog() 
{
	
	try {
		CNxDialog::OnInitDialog();

		//bind the list
		m_pList = BindNxDataList2Ctrl(IDC_BOLD_FIELD_LIST, false);

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		CString strDesc;
		strDesc.Format("There are multiple EMN items and/or values "
			"that correspond to the following BOLD information. "
			"Please choose the EMN and/or values to pull the information "
			"from for each code listed.");
		m_stDescription.SetWindowTextA(strDesc);
		
		CString strTitle;
		strTitle.Format("Choose BOLD Values for %s: %s", m_strPatientName, m_strVisitType);
		this->SetWindowTextA(strTitle);

		//load the list from the array
		LoadList();

	}NxCatchAll(__FUNCTION__);
	return TRUE;
}



void CBoldFieldPickDlg::LoadList() 
{

	try {

		for (int i = 0; i < m_paryBoldList->GetSize(); i++) {

			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->GetNewRow();

			CString strCode = m_paryBoldList->GetAt(i)->strCode;
			CString strCodeDesc = m_paryBoldList->GetAt(i)->strCodeDesc;

			pRow->PutValue(tlcCode, _variant_t(strCode));
			pRow->PutValue(tlcDescription, _variant_t(strCodeDesc));
			CArray<BOLDItem*, BOLDItem*> *aryItems = m_paryBoldList->GetAt(i)->aryItems;

			CString strComboSource;
			
			//now fill our description column
			for (int j = 0; j < aryItems->GetSize(); j++) {

				CString strRow = aryItems->GetAt(j)->strRow;
				long nItemID = aryItems->GetAt(j)->nItemID;

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

			if (aryItems->GetSize() == 1) {

				//check to see if we need to add this
				
				if (aryItems->GetAt(0)->paryDetails->GetSize() != 1 && (!m_paryBoldList->GetAt(i)->bAllowMultiples)) {
					//add it
					m_pList->AddRowAtEnd(pRow, NULL);					

					//we only have one, so set it as the selection
					pRow->PutValue(tlcSelection, aryItems->GetAt(0)->nItemID);				
					pfs->PutEditable(VARIANT_FALSE);
					pRow->PutRefCellFormatOverride(tlcSelection, pfs);
					LoadValuesList(pRow, aryItems->GetAt(0)->nItemID);
				}						
				
			}
			else {
				m_pList->AddRowAtEnd(pRow, NULL);
			}
		}

	}NxCatchAll(__FUNCTION__);
}

void CBoldFieldPickDlg::LoadValuesList(LPDISPATCH lpRow, long nRowItemID)
{
	try {

		//loop through the list and look for this itemID for this code, then load the values
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {

			CString strRowCode = VarString(pRow->GetValue(tlcCode));
		
			for (int i = 0; i < m_paryBoldList->GetSize(); i++) {

				CString strCode = m_paryBoldList->GetAt(i)->strCode;
				BOOL bAllowMultiples = m_paryBoldList->GetAt(i)->bAllowMultiples;

				
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

						CArray<BOLDItem*, BOLDItem*> *aryItems = m_paryBoldList->GetAt(i)->aryItems;

						CString strComboSource;
					
						
						//now find our ItemID
						for (int j = 0; j < aryItems->GetSize(); j++) {

							long nItemID = aryItems->GetAt(j)->nItemID;

							if (nItemID == nRowItemID) {

								//fill the values

								NXDATALIST2Lib::IFormatSettingsPtr pfs(__uuidof(NXDATALIST2Lib::FormatSettings));
								pfs->PutDataType(VT_BSTR);
								pfs->PutFieldType(NXDATALIST2Lib::cftComboSimple);								
								pfs->PutConnection(_variant_t((LPDISPATCH)NULL)); //we're going to let this combo use Practice's connection
								

								for (int k = 0; k <  aryItems->GetAt(j)->paryDetails->GetSize(); k++) {

									CString strValue =  aryItems->GetAt(j)->paryDetails->GetAt(k)->strSelection;
									CString strSelectCode = aryItems->GetAt(j)->paryDetails->GetAt(k)->strSelectCode;

									//get rid of any semicolon's that may exist
									strValue.Remove(';');
									strSelectCode.Remove(';');

									CString strTemp;
									if (strSelectCode.IsEmpty()) {
										strTemp.Format("%s;%s;", strValue, strValue);
									}
									else {
										strTemp.Format("%s;%s;", strSelectCode, strValue);
									}

									strComboSource += strTemp;			

									pfs->PutEditable(VARIANT_TRUE);
								}	
								pfs->PutComboSource(_bstr_t(strComboSource));

								if (aryItems->GetAt(j)->paryDetails->GetSize() == 1) {

									//there is only 1, just fill it
									pfs->PutEditable(VARIANT_FALSE);
									CString strValue = aryItems->GetAt(j)->paryDetails->GetAt(0)->strSelection;
									CString strValueCode = aryItems->GetAt(j)->paryDetails->GetAt(0)->strSelectCode;
									pRow->PutValue(tlcValue, _variant_t(strValue));
									pRow->PutValue(tlcValueCode, _variant_t(strValueCode));
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

void CBoldFieldPickDlg::OnBnClickedOk()
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
			if (IDNO == MsgBox(MB_YESNO, "Not all selections have been filled out. This request will not include the fields that still need a selection made, are you sure you want to continue?")) {
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
				CString strCode = VarString(pRow->GetValue(tlcCode), "");
				CString strValueCode;
				_variant_t varValueCode = pRow->GetValue(tlcValueCode);
				if (varValueCode.vt == VT_BSTR) {
					strValueCode = VarString(varValueCode);
				}
				if (strValue == "<Multiple Values Allowed>") {
					//look up the itemID
					strValue = AsString(pRow->GetValue(tlcSelection));
				}
				else if (!strValueCode.IsEmpty()) {
					strValue = strValueCode;
				}
				m_pmapReturnValues->SetAt(strCode, strValue);
			}
			pRow = pRow->GetNextRow();
		}

		CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__);

}

void CBoldFieldPickDlg::OnBnClickedCancel()
{
	try {
		if (IDNO == MsgBox(MB_YESNO, "There are selections that must be made before this request can be sent to BOLD, by cancelling, this request will not include any fields that need a selection made, are you sure you want to continue?")) {
			return;
		}
		CNxDialog::OnCancel();
	}NxCatchAll(__FUNCTION__);
}
BEGIN_EVENTSINK_MAP(CBoldFieldPickDlg, CDialog)
	ON_EVENT(CBoldFieldPickDlg, IDC_BOLD_FIELD_LIST, 10, CBoldFieldPickDlg::EditingFinishedBoldFieldList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
END_EVENTSINK_MAP()

void CBoldFieldPickDlg::EditingFinishedBoldFieldList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
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
