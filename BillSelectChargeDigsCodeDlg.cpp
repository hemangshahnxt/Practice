// BillSelectChargeDigsCodeDlg.cpp : implementation file
//
// (s.dhole 2011-05-16 14:44) - PLID 33666 New Dlg to assign Digs to Line Item
#include "stdafx.h"
#include "Practice.h"
#include "BillingRc.h"
#include "BillSelectChargeDigsCodeDlg.h"
#include "DiagSearchUtils.h" 
#include "DiagCodeInfo.h"


// CBillSelectChargeDigsCodeDlg dialog
using namespace NXDATALIST2Lib;
// (j.gruber 2014-02-25 13:20) - PLID 61028 - add our ICD10 codes while we are here

enum DiagCodeSelectedListColumns {
	dcslcIsSelected= 0,
	dcslcID,
	dcslcICD9ID,	
	dcslcICD9Code,
	dcslcICD9Desc,
	dcslcICD10ID,	
	dcslcICD10Code,
	dcslcICD10Desc,
	dcslcOrderIndex,
};


IMPLEMENT_DYNAMIC(CBillSelectChargeDigsCodeDlg, CNxDialog)

CBillSelectChargeDigsCodeDlg::CBillSelectChargeDigsCodeDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CBillSelectChargeDigsCodeDlg::IDD, pParent)
{

}

CBillSelectChargeDigsCodeDlg::~CBillSelectChargeDigsCodeDlg()
{
}

void CBillSelectChargeDigsCodeDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	
	
}


BEGIN_MESSAGE_MAP(CBillSelectChargeDigsCodeDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, OnOk)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MESSAGE_MAP()

BOOL CBillSelectChargeDigsCodeDlg::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);		
		SetDlgItemText(IDC_DIG_CODE_LABEL, FormatString("Choose Diagnosis Codes to Link to %s" , m_strCaption )  ) ;
		

		m_DiagCodeListSelected = BindNxDataList2Ctrl(IDC_DIAG_CODE_SELECTED_LIST, false);

		CArray<long, long> aryIndexes;
				
		// (j.gruber 2014-02-25 13:16) - PLID 61028 - new structure						
		for(int i=0; i<m_arypAllDiagCodes.GetSize(); i++) {					
			DiagCodeInfoPtr pInfo = (DiagCodeInfoPtr)m_arypAllDiagCodes.GetAt(i);
			if(pInfo) {
				NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_DiagCodeListSelected->GetNewRow();				

				//is it in our selected map
				CChargeWhichCodePair pair = CChargeWhichCodePair(pInfo->nDiagCode9ID, pInfo->nDiagCode10ID);
				CChargeWhichCodesIterator itFind = m_mapSelectedCodes->find(pair);
				if (itFind != m_mapSelectedCodes->end()) 
				{
					//its selected
					pNewRow->PutValue(dcslcIsSelected, g_cvarTrue);
				}
				else {
					pNewRow->PutValue(dcslcIsSelected, g_cvarFalse);
				}
				
				pNewRow->PutValue(dcslcID, pInfo->nID);
				// (j.gruber 2014-02-25 13:27) - PLID 61028 - change to 9 and 10
				pNewRow->PutValue(dcslcICD9ID, pInfo->nDiagCode9ID);				
				pNewRow->PutValue(dcslcICD9Code, _bstr_t(pInfo->strDiagCode9Code));
				pNewRow->PutValue(dcslcICD9Desc, _bstr_t(pInfo->strDiagCode9Desc));
				pNewRow->PutValue(dcslcICD10ID, pInfo->nDiagCode10ID);				
				pNewRow->PutValue(dcslcICD10Code, _bstr_t(pInfo->strDiagCode10Code));
				pNewRow->PutValue(dcslcICD10Desc, _bstr_t(pInfo->strDiagCode10Desc));
				pNewRow->PutValue(dcslcOrderIndex, pInfo->nOrderIndex);
				m_DiagCodeListSelected->AddRowSorted(pNewRow, NULL);
				
			}			
		}
		// (j.gruber 2014-02-25 13:28) - PLID 61028 - not sure why this was here to begin with, but not needed anymore
		/*m_strSelectedDiagCodes="";
		ClearDiagCodeArray();*/

		// (j.gruber 2014-02-26 13:05) - PLID 61028 - only show depending on ICD9/10
		IColumnSettingsPtr pCol = m_DiagCodeListSelected->GetColumn(dcslcICD9Code);
		long nWidth9 = pCol->GetStoredWidth();
		pCol = m_DiagCodeListSelected->GetColumn(dcslcICD10Code);
		long nWidth10 = pCol->GetStoredWidth();
		DiagSearchUtils::SizeDiagnosisListColumnsBySearchPreference(m_DiagCodeListSelected, dcslcICD9Code, dcslcICD10Code, nWidth9, nWidth10,  "", "", dcslcICD9Desc, dcslcICD10Desc, true, false, true);

	}NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// (j.gruber 2014-02-25 13:36) - PLID 61028
DiagCodeInfoPtr CBillSelectChargeDigsCodeDlg::FindDiagCodeByPair(CChargeWhichCodePair pair)
{
	for(int i = 0; i < m_arypAllDiagCodes.GetSize(); i++) 
	{
		DiagCodeInfoPtr pDiag = m_arypAllDiagCodes.GetAt(i);
		if (pDiag->nDiagCode9ID == pair.first
			&& pDiag->nDiagCode10ID == pair.second)
		{
			return pDiag;
		}
		
	}

	ThrowNxException("Error in CBillSelectChargeDigsCodeDlg::FindDiagCodeByPair - could not find pair %li, %li", pair.first, pair.second);
}

void CBillSelectChargeDigsCodeDlg::OnOk()
{
	try {
		if(m_bReadOnly) {
			//leave now, do nothing else
			CNxDialog::OnOK();
			return;
		}
		
		// (j.gruber 2014-02-25 13:29) - PLID 61028 - fill our new structure

		//first clear our existing map
		m_mapSelectedCodes->clear();

		//now fill it back up
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_DiagCodeListSelected->GetFirstRow();		
		while(pRow) {
			if (VarBool(pRow->GetValue(dcslcIsSelected)))
			{	
				long nICD9ID = VarLong(pRow->GetValue(dcslcICD9ID), -1);
				long nICD10ID = VarLong(pRow->GetValue(dcslcICD10ID), -1);
				CChargeWhichCodePair pair = CChargeWhichCodePair(nICD9ID, nICD10ID);
				DiagCodeInfoPtr pDiag = FindDiagCodeByPair(pair);

				m_mapSelectedCodes->insert(std::make_pair(pair, pDiag));
			}
			pRow = pRow->GetNextRow();
		}

		CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__);
}

void CBillSelectChargeDigsCodeDlg::OnCancel()
{
	try {
		CNxDialog::OnCancel();

	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2014-02-25 13:36) - PLID 61028 - no longer needed
/*void CBillSelectChargeDigsCodeDlg::ClearDiagCodeArray()
{
	try {

		for(int i=0; i<m_arypAllDiagCodes.GetSize(); i++) {
			DiagCodeInfo *pDiag = (DiagCodeInfo*)m_arypAllDiagCodes.GetAt(i);
			delete pDiag;
		}
		m_arypAllDiagCodes.RemoveAll();

	}NxCatchAll("Error in CBillSelectChargeDigsCodeDlg::ClearDiagCodeArray");
}*/

BEGIN_EVENTSINK_MAP(CBillSelectChargeDigsCodeDlg, CNxDialog)
	ON_EVENT(CBillSelectChargeDigsCodeDlg, IDC_DIAG_CODE_SELECTED_LIST, 9, CBillSelectChargeDigsCodeDlg::EditingFinishingDiagCodeSelectedList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
END_EVENTSINK_MAP()

void CBillSelectChargeDigsCodeDlg::EditingFinishingDiagCodeSelectedList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	// (j.gruber 2014-02-25 13:36) - PLID 61028 - need error handling
	try
	{
		if(nCol == dcslcIsSelected && VarBool(pvarNewValue) == TRUE) {
			IRowSettingsPtr pRow= m_DiagCodeListSelected->GetFirstRow();
			long nCount = 0;
			while(pRow) {
				if(VarBool(pRow->GetValue(dcslcIsSelected)) == TRUE) {
					nCount++;
				}

				// (j.jones 2013-08-09 14:43) - PLID 57299 - reworded this warning to be somewhat more generic,
				// as the new HCFA also supports 12 diagnosis codes, but the limitation of linking 4 remains
				if(nCount >= 4) {
					AfxMessageBox("No more than 4 diagnosis codes can be linked with one charge. ");
					*pbCommit = FALSE;
					return;
				}

				pRow = pRow->GetNextRow();
			}
		}
	}NxCatchAll(__FUNCTION__);
}
