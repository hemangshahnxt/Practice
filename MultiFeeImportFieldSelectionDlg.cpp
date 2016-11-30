// MultiFeeImportFieldSelectionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "MultiFeeImportFieldSelectionDlg.h"
#include "SoapUtils.h"
#include "AdministratorRc.h"

#define MULTIFEE_IMPORT_TIMER 20123

// CMultiFeeImportFieldSelectionDlg dialog
// (j.gruber 2009-10-27 13:56) - PLID 35632 - created for

IMPLEMENT_DYNAMIC(CMultiFeeImportFieldSelectionDlg, CNxDialog)

CMultiFeeImportFieldSelectionDlg::CMultiFeeImportFieldSelectionDlg(CString strFileName, BOOL bUseAllowable, CString strCodeField, CString strFeeField, CString strAllowableField, CWnd* pParent /*=NULL*/)
	: CNxDialog(CMultiFeeImportFieldSelectionDlg::IDD, pParent)
{
	m_strFileName = strFileName;
	m_bUseAllowable = bUseAllowable;
	m_strCodeField = strCodeField;
	m_strFeeField = strFeeField;
	m_strAllowableField = strAllowableField;
}

CMultiFeeImportFieldSelectionDlg::~CMultiFeeImportFieldSelectionDlg()
{
}

void CMultiFeeImportFieldSelectionDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_STATIC_INSTRUCTIONS, m_nxstInstructions);
	CNxDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMultiFeeImportFieldSelectionDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CMultiFeeImportFieldSelectionDlg::OnBnClickedOk)
	ON_WM_TIMER()
END_MESSAGE_MAP()



BOOL CMultiFeeImportFieldSelectionDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		//bind the datalists
		m_pCodeField = BindNxDataList2Ctrl(IDC_MULTIFEE_CODE_FIELD, false);
		m_pFeeField = BindNxDataList2Ctrl(IDC_MULTIFEE_FEE_FIELD, false);
		m_pAllowableField = BindNxDataList2Ctrl(IDC_MULTIFEE_ALLOWABLE_FIELD, false);

		//see if allowable is in use
		if (!m_bUseAllowable) {
			GetDlgItem(IDC_MULTIFEE_ALLOWABLE_FIELD)->EnableWindow(FALSE);
			m_nxstInstructions.SetWindowText("Please choose the values in the XML file representing the service code and service code fee fields.");
		}
		else {
			GetDlgItem(IDC_MULTIFEE_ALLOWABLE_FIELD)->EnableWindow(TRUE);
			m_nxstInstructions.SetWindowText("Please choose the values in the XML file representing the service code, service code fee and allowable fields.");
		}

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		//set a time to start the load so that the dialog will pop up
		SetTimer(MULTIFEE_IMPORT_TIMER, 500, NULL);	

	}NxCatchAll(__FUNCTION__);
	return TRUE;
}


void CMultiFeeImportFieldSelectionDlg::OnTimer(UINT nIDEvent) 
{

	try {
		
		if (nIDEvent == MULTIFEE_IMPORT_TIMER) {
			KillTimer(MULTIFEE_IMPORT_TIMER);
			ProcessFieldList();
		}

		CNxDialog::OnTimer(nIDEvent);
	}NxCatchAll(__FUNCTION__);
}

void CMultiFeeImportFieldSelectionDlg::InitLoad(NXDATALIST2Lib::_DNxDataListPtr pList, long nID)
{
	//first clear the list
	pList->Clear();

	NXDATALIST2Lib::IRowSettingsPtr pRow = pList->GetNewRow();
	if (pRow) {
		pRow->PutValue(0, _variant_t("Loading..."));

		pList->AddRowAtEnd(pRow, NULL);
		
		GetDlgItem(nID)->RedrawWindow();

		pList->SetRedraw(FALSE);
	}

}

void CMultiFeeImportFieldSelectionDlg::ProcessFieldList()
{
	try {		

		CWaitCursor pWait;
		//set the lists to be loading
		InitLoad(m_pCodeField, IDC_MULTIFEE_CODE_FIELD);
		InitLoad(m_pFeeField, IDC_MULTIFEE_FEE_FIELD);
		if (m_bUseAllowable) {
			InitLoad(m_pAllowableField, IDC_MULTIFEE_ALLOWABLE_FIELD);
		}
		

		//loadup the file		
		MSXML2::IXMLDOMDocumentPtr pXMLDoc(__uuidof(MSXML2::DOMDocument60));
		if (pXMLDoc->load(_variant_t(m_strFileName)) == VARIANT_TRUE) {

			//now we have to get the root object
			MSXML2::IXMLDOMNodeListPtr pRootNodes = pXMLDoc->GetchildNodes();
			for (int i = 0; i < pRootNodes->Getlength(); i++) {
				
				CStringArray aryStr;
				MSXML2::IXMLDOMNodePtr pRootNode = pRootNodes->Getitem(i);
				if (pRootNode) {

					//now get all the children of the root
					ParseChildren(pRootNode, NULL);					
					
				}			
			}		
		}

		//copy the rows from the code list to the others
		CopyList(m_pCodeField, m_pFeeField);
		if (m_bUseAllowable) {
			CopyList(m_pCodeField, m_pAllowableField);
		}

		//now get rid of the loading, etc
		FinalizeLoading(m_pCodeField, m_strCodeField);
		FinalizeLoading(m_pFeeField, m_strFeeField);
		if (m_bUseAllowable) {
			FinalizeLoading(m_pAllowableField, m_strAllowableField);
		}

	}NxCatchAll(__FUNCTION__);
	
}

void CMultiFeeImportFieldSelectionDlg::FinalizeLoading(NXDATALIST2Lib::_DNxDataListPtr pList, CString strFieldPath) 
{

	//remove the loading.. row
	NXDATALIST2Lib::IRowSettingsPtr pRow;
	pRow = pList->FindByColumn(0, "Loading...", NULL, FALSE);

	if (pRow) {
		pList->RemoveRow(pRow);

		pList->SetRedraw(TRUE);
	}
	else {
		ASSERT(FALSE);
	}

	if (!strFieldPath.IsEmpty()) {
		LoadDefaults(pList, strFieldPath);
	}
	
}


void CMultiFeeImportFieldSelectionDlg::LoadDefaults(NXDATALIST2Lib::_DNxDataListPtr pList, CString strFieldPath) 
{
	//first find the last entry
	CString strTemp = strFieldPath;
	
	long nResult = strTemp.Find("->");
	while (nResult != -1) {
		strTemp = strTemp.Right(strTemp.GetLength() - (nResult + 2));
		nResult = strTemp.Find("->");
	}
	
	//we now have the last node of the path, so let's find the row it goes with
	NXDATALIST2Lib::IRowSettingsPtr pRow, pParentRow;
	pRow = pList->FindByColumn(0, _variant_t(strTemp), NULL, FALSE);
	
	long nCount = 0;
	while (pRow) {
		CStringArray aryRow;
		
		aryRow.Add(VarString(pRow->GetValue(0), ""));
		
		pParentRow = pRow->GetParentRow();
		while (pParentRow) {
			aryRow.Add(VarString(pParentRow->GetValue(0), ""));
			pParentRow = pParentRow->GetParentRow();
		}

		//now we have to reverse it
		CString strRowPath;
		for (int i = aryRow.GetSize() -1; i>= 0; i--) {
			strRowPath += aryRow.GetAt(i) + "->";
		}

		//take off the last ->
		strRowPath.TrimRight("->");

		//now compare
		if (strRowPath != strFieldPath) {
			//start again from this row
			pRow = pList->FindByColumn(0, _variant_t(strTemp), pRow, FALSE);
			nCount++;
			//just in case we get stuck where the field list is somehow screwed up, but is finding a row,
			//bomb out if we have gone around too many times
			if (nCount >= pList->GetRowCount()) {
				return;
			}

		}
		else {
			pList->CurSel = pRow;
			return;
		}
		
	}

	//if we got here, nothing matched, so we aren't setting anything
}

void CMultiFeeImportFieldSelectionDlg::CopyList(NXDATALIST2Lib::_DNxDataListPtr pList1, NXDATALIST2Lib::_DNxDataListPtr pList2) {

	//loop through list 1 and copy it to list2
	NXDATALIST2Lib::IRowSettingsPtr pRowCopyFrom = pList1->GetFirstRow();
	NXDATALIST2Lib::IRowSettingsPtr pRowCopyTo = NULL;

	while (pRowCopyFrom) {

		//don't copy the loading row
		if (VarString(pRowCopyFrom->GetValue(0), "") != "Loading...") {

			pRowCopyTo = pList2->GetNewRow();

			if (pRowCopyTo) {

				pRowCopyTo = pRowCopyFrom;
				pList2->AddRowAtEnd(pRowCopyTo, pRowCopyTo->GetParentRow());
			}
		}

		pRowCopyFrom = pRowCopyFrom->GetNextRow();
	}

}
void CMultiFeeImportFieldSelectionDlg::ParseChildren(MSXML2::IXMLDOMNodePtr pRootNode, NXDATALIST2Lib::IRowSettingsPtr pParentRow) {

	CString strRootName = (LPCTSTR) pRootNode->GetnodeName();

	MSXML2::IXMLDOMNodeListPtr pChildList = pRootNode->GetchildNodes();
	if (pChildList) {

		if(pChildList->length == 0) {
			//do nothing
		}
		else {			

			for (int i = 0; i < pChildList->Getlength(); i++) {

				MSXML2::IXMLDOMNodePtr pChild = pChildList->Getitem(i);
				if (pChild) {

					//don't include text nodes here because they just show #text
					if (pChild->nodeType == MSXML2::NODE_ELEMENT) {
						CString strChildName = VarString(pChild->GetnodeName(), "");

						//create a row for us
						NXDATALIST2Lib::IRowSettingsPtr pRow;
						pRow = EnsureDataListRow(strChildName, pParentRow);

						//now get all the children for this		
						ParseChildren(pChild, pRow);			
					}
				}
				
			}		
			
		}		
	}
	else {
		//this is an error condition
		ThrowNxException("Error parsing XML File");		
	}
}	

NXDATALIST2Lib::IRowSettingsPtr CMultiFeeImportFieldSelectionDlg::EnsureDataListRow(CString strRowName, NXDATALIST2Lib::IRowSettingsPtr pParentRow) 
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCodeField->FindByColumn(0, _variant_t(strRowName), pParentRow, FALSE);

	if (pRow) {
		//it exists, return it
		return pRow;
	}
	else {
		//we need to create it
		pRow = m_pCodeField->GetNewRow();
		pRow->PutValue(0, _variant_t(strRowName));
		m_pCodeField->AddRowAtEnd(pRow, pParentRow);		
		return pRow;
	}
}



// CMultiFeeImportFieldSelectionDlg message handlers
BEGIN_EVENTSINK_MAP(CMultiFeeImportFieldSelectionDlg, CDialog)
	ON_EVENT(CMultiFeeImportFieldSelectionDlg, IDC_MULTIFEE_CODE_FIELD, 1, CMultiFeeImportFieldSelectionDlg::SelChangingMultifeeCodeField, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CMultiFeeImportFieldSelectionDlg, IDC_MULTIFEE_FEE_FIELD, 1, CMultiFeeImportFieldSelectionDlg::SelChangingMultifeeFeeField, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CMultiFeeImportFieldSelectionDlg, IDC_MULTIFEE_ALLOWABLE_FIELD, 1, CMultiFeeImportFieldSelectionDlg::SelChangingMultifeeAllowableField, VTS_DISPATCH VTS_PDISPATCH)
END_EVENTSINK_MAP()

void CMultiFeeImportFieldSelectionDlg::SelChangingMultifeeCodeField(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}

void CMultiFeeImportFieldSelectionDlg::SelChangingMultifeeFeeField(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}

void CMultiFeeImportFieldSelectionDlg::SelChangingMultifeeAllowableField(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}

BOOL CMultiFeeImportFieldSelectionDlg::SaveFieldPath(NXDATALIST2Lib::_DNxDataListPtr pList, CString &strField, CString strDescriptor) 
{
	CStringArray aryTemp;

	//clear the field we are saving to
	strField = "";

	NXDATALIST2Lib::IRowSettingsPtr pRow = pList->CurSel;

	if (pRow == NULL) {
		MsgBox("Please choose a row from the %s list", strDescriptor);
		return FALSE;
	}

	while (pRow) {

		aryTemp.Add(VarString(pRow->GetValue(0)));

		pRow = pRow->GetParentRow();
	}

	//now we have to reverse it
	
	for (int i = aryTemp.GetSize() - 1; i >=0; i--) {
		strField += aryTemp.GetAt(i) + "->";		
	}

	//remove  the last one
	strField.TrimRight("->");
	return TRUE;
	
}

void CMultiFeeImportFieldSelectionDlg::OnBnClickedOk()
{
	try {

		if (SaveFieldPath(m_pCodeField, m_strCodeField, "Service Code")) {
			if (SaveFieldPath(m_pFeeField, m_strFeeField, "Fee")) {
				if (m_bUseAllowable) {
					if (SaveFieldPath(m_pAllowableField, m_strAllowableField, "Allowable")) {
						m_strParentNode = GetPathParent(m_strCodeField, m_strFeeField, m_strAllowableField);
						if (m_strParentNode.IsEmpty()) {
							MsgBox("The fields must have a common parent among them.  Please pick fields with at least one common parent.");						
						}
						else {
							CNxDialog::OnOK();
						}
					}
				}
				else {
					m_strParentNode = GetPathParent(m_strCodeField, m_strFeeField, m_strAllowableField);
					if (m_strParentNode.IsEmpty()) {
						MsgBox("The fields must have a common parent among them.  Please pick fields with at least one common parent.");						
					}
					else {
						CNxDialog::OnOK();
					}
				}
			}
		}

	}NxCatchAll(__FUNCTION__);
}

CString CMultiFeeImportFieldSelectionDlg::GetPathParent(CString strCodePath, CString strFeePath, CString strAllowablePath) {

	CString strReturn;

	//compare the code and fee paths first
	BOOL bContinue = TRUE;
	for (int i = 0; i < strCodePath.GetLength(); i++) {
		if (bContinue) {
			if (strCodePath.GetAt(i) == strFeePath.GetAt(i)) {
				strReturn += strCodePath.GetAt(i);
			}
			else {
				//stop
				bContinue = FALSE;
				break;				
			}
		}
	}


	//make sure there is a -> in the path
	long nResult = strReturn.Find("->");
	if (nResult == -1) {
		//we don't have a full section
		return "";
	}	

	//if we didn't end in a -> we need to trim to it
	if (strReturn.Right(2) != "->") {
		long nLastResult = strReturn.Find("->");		
		while (nLastResult != -1) {
			nResult = nLastResult;
			nLastResult = strReturn.Find("->", nLastResult+1);
		}
		strReturn = strReturn.Left(nResult);
	}

	//now make sure that allowable starts with the same string, if applicable
	if (m_bUseAllowable) {
		nResult = strAllowablePath.Find(strReturn);
		if (nResult != 0) {
			return "";
			
		}
	}
	

	//we are good
	return strReturn;

}