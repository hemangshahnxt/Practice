// EditEMNOtherProvidersDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "EditEMNOtherProvidersDlg.h"
#include "EMRRc.h"
#include "EditComboBox.h"
#include "EmrColors.h"
#include "EMNProvider.h"

// (j.gruber 2009-06-16 16:01) - PLID 33688 - created for
enum ProviderTypeListColumns {
	ptlcProviderID = 0,
	ptlcTypeID = 1,
	ptlcIsNew = 2,
};

// CEditEMNOtherProvidersDlg dialog

IMPLEMENT_DYNAMIC(CEditEMNOtherProvidersDlg, CNxDialog)

CEditEMNOtherProvidersDlg::CEditEMNOtherProvidersDlg(
	CArray<EMNProvider*, EMNProvider*> *paryProviders,
	CWnd* pParent /*=NULL*/)
	: CNxDialog(CEditEMNOtherProvidersDlg::IDD, pParent)
{

	m_paryProviders = paryProviders;
	m_bWarnIfCancel = FALSE;
	
}

CEditEMNOtherProvidersDlg::~CEditEMNOtherProvidersDlg()
{
}

void CEditEMNOtherProvidersDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_OTHER_PROVS_COLOR, m_color);
	DDX_Control(pDX, IDC_ADD_EMR_OTHER_PROV, m_btnAdd);
	DDX_Control(pDX, IDC_REMOVE_EMR_OTHER_PROV, m_btnRemove);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	
}


BEGIN_MESSAGE_MAP(CEditEMNOtherProvidersDlg, CNxDialog)
	ON_BN_CLICKED(IDC_ADD_EMR_OTHER_PROV, &CEditEMNOtherProvidersDlg::OnBnClickedAddEmrOtherProv)
	ON_BN_CLICKED(IDC_REMOVE_EMR_OTHER_PROV, &CEditEMNOtherProvidersDlg::OnBnClickedRemoveEmrOtherProv)
	ON_BN_CLICKED(IDOK, &CEditEMNOtherProvidersDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CEditEMNOtherProvidersDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CEditEMNOtherProvidersDlg message handlers
BOOL CEditEMNOtherProvidersDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnRemove.AutoSet(NXB_DELETE);
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// (a.walling 2012-05-31 14:49) - PLID 50719 - EmrColors
		COLORREF bkgColor = EmrColors::Topic::PatientBackground();

		m_color.SetColor(bkgColor);

		//bind the list
		m_pProviderList = BindNxDataList2Ctrl(IDC_EMN_OTHER_PROV_LIST, true);

		//Load the list with our array that got passed in
		LoadListFromArray();
	}NxCatchAll("Error in CEditEMNOtherProvidersDlg::OnInitDialog()");
	return TRUE;
}


void CEditEMNOtherProvidersDlg::LoadListFromArray() 
{
	try {
		//loop through our array and add the values
		_variant_t varTrue(VARIANT_TRUE, VT_BOOL);
		_variant_t varFalse(VARIANT_FALSE, VT_BOOL);
		for (int i = 0; i < m_paryProviders->GetSize(); i++) {

			EMNProvider *pProv = m_paryProviders->GetAt(i);

			NXDATALIST2Lib::IRowSettingsPtr pRow;
			pRow = m_pProviderList->GetNewRow();		

			CString strProvID = AsString(pProv->nID) + '-' + pProv->strName;
			pRow->PutValue(ptlcProviderID, _variant_t(strProvID));

			CString strTypeID = AsString(pProv->nTypeID) + '-' + pProv->strTypeName;
			pRow->PutValue(ptlcTypeID, _variant_t(strTypeID));		
			

			if (pProv->bIsNew) {
				pRow->PutValue(ptlcIsNew, varTrue);
			}
			else {
				pRow->PutValue(ptlcIsNew, varFalse);
			}

			m_pProviderList->AddRowAtEnd(pRow, NULL);
					
		}
	}NxCatchAll("Error in CEditEMNOtherProvidersDlg::LoadListFromArray()");
}

void CEditEMNOtherProvidersDlg::OnBnClickedAddEmrOtherProv()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pProviderList->GetNewRow();

		_variant_t varTrue(VARIANT_TRUE, VT_BOOL);
		pRow->PutValue(ptlcIsNew, varTrue);

		m_pProviderList->AddRowAtEnd(pRow, NULL);
		m_pProviderList->StartEditing(pRow, ptlcProviderID);
	}NxCatchAll("Error in CEditEMNOtherProvidersDlg::OnBnClickedAddEmrOtherProv()");
}

void CEditEMNOtherProvidersDlg::OnBnClickedRemoveEmrOtherProv()
{
	try {
		//get the id
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pProviderList->CurSel;
		if (pRow == NULL) {
			MessageBox("Please choose a provider to remove");
			return;
		}	

		m_pProviderList->RemoveRow(pRow);
	}NxCatchAll("Error in CEditEMNOtherProvidersDlg::OnBnClickedRemoveEmrOtherProv()");
}

void CEditEMNOtherProvidersDlg::OnBnClickedOk()
{

	try {
		//first run through the list and make sure everything has a value
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pProviderList->GetFirstRow();

		BOOL bValidated = TRUE;
		while (pRow) {

			if (pRow->GetValue(ptlcProviderID).vt != VT_BSTR) {
				bValidated = FALSE;
			}

			if (pRow->GetValue(ptlcTypeID).vt != VT_BSTR) {
				bValidated = FALSE;
			}

			pRow = pRow->GetNextRow();
		}

		if (!bValidated) {
			MsgBox("Please make sure all rows have both a provider and type selected.");
			return;
		}

		//also make sure there is only one provider per type in the list
		pRow = m_pProviderList->GetFirstRow();
		CMap<CString, LPCTSTR, CString, LPCTSTR> mapProvIDs;
		while (pRow) {
			CString strTemp;
			CString strProvID = VarString(pRow->GetValue(ptlcProviderID), "");
			if (mapProvIDs.Lookup(strProvID, strTemp)) {
				bValidated = FALSE;
			}

			mapProvIDs.SetAt(strProvID, strProvID);

			pRow = pRow->GetNextRow();
		}

		if (!bValidated) {
			MsgBox("A Provider may only be selected once.  Please correct this.");
			return;
		}


		//clear everything from the current list and make a new list
		for (int i = m_paryProviders->GetSize() - 1; i >= 0 ; i--) {

			EMNProvider * pProv = m_paryProviders->GetAt(i);
			m_paryProviders->RemoveAt(i);

			delete pProv;
		}

		//now create a new list
		pRow = m_pProviderList->GetFirstRow();

		while (pRow) {

			EMNProvider *pProv = new EMNProvider();
			pProv->bIsNew = VarBool(pRow->GetValue(ptlcIsNew));

			CString strValue = VarString (pRow->GetValue(ptlcProviderID));
			long nResult = strValue.Find("-");
			ASSERT(nResult != -1);
			long nID = atoi(strValue.Left(nResult));
			CString strProvName = strValue.Right(strValue.GetLength() - (nResult + 1));

			pProv->nID = nID;
			pProv->strName = strProvName;

			//now for the type
			strValue = VarString (pRow->GetValue(ptlcTypeID));
			nResult = strValue.Find("-");
			ASSERT(nResult != -1);
			nID = atoi(strValue.Left(nResult));
			CString strTypeName = strValue.Right(strValue.GetLength() - (nResult + 1));

			pProv->nTypeID = nID;
			pProv->strTypeName = strTypeName;

			// (j.jones 2011-04-28 14:39) - PLID 43122 - added FloatEMRData (defaults to true)
			pProv->bFloatEMRData = VarBool(GetTableField("ProvidersT", "FloatEMRData", "PersonID", pProv->nID), TRUE);
			
			m_paryProviders->Add(pProv);

			pRow = pRow->GetNextRow();
		}
			
		OnOK();
	}NxCatchAll("CEditEMNOtherProvidersDlg::OnBnClickedOk()");
}

void CEditEMNOtherProvidersDlg::OnBnClickedCancel()
{
	OnCancel();
}
BEGIN_EVENTSINK_MAP(CEditEMNOtherProvidersDlg, CNxDialog)
	ON_EVENT(CEditEMNOtherProvidersDlg, IDC_EMN_OTHER_PROV_LIST, 9, CEditEMNOtherProvidersDlg::EditingFinishingEmnOtherProvList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
END_EVENTSINK_MAP()

void CEditEMNOtherProvidersDlg::EditingFinishingEmnOtherProvList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {

			switch (nCol) {

				case ptlcProviderID:
				case ptlcTypeID:

					if (pvarNewValue->vt == VT_EMPTY || VarString(pvarNewValue, "").IsEmpty()) {
						
						MsgBox("Please select a value");

						*pbCommit = FALSE;
						//let the out, they might not have any types
						//*pbContinue = FALSE;
						
						//start editing again
						//m_pProviderList->StartEditing(pRow, nCol);
					}
				break;
			}
		}			
	}NxCatchAll("CEditEMNOtherProvidersDlg::EditingFinishingEmnOtherProvList");
}

