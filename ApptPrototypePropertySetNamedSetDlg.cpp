// ApptPrototypePropertySetNamedSetDlg.cpp : implementation file
// (b.cardillo 2011-02-22 16:25) - PLID 40419 - Admin UI for managing Appointment Prototypes
//

#include "stdafx.h"
#include "Practice.h"
#include "ApptPrototypePropertySetNamedSetDlg.h"
#include "MultiSelectDlg.h"

using namespace ADODB;
using namespace NXDATALIST2Lib;

// CApptPrototypePropertySetNamedSetDlg dialog

IMPLEMENT_DYNAMIC(CApptPrototypePropertySetNamedSetDlg, CNxDialog)

CApptPrototypePropertySetNamedSetDlg::CApptPrototypePropertySetNamedSetDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CApptPrototypePropertySetNamedSetDlg::IDD, pParent)
{
	m_bInitialized = FALSE;
	m_pDataSource = NULL;
	m_nNextNegativeID = -1;
}

CApptPrototypePropertySetNamedSetDlg::~CApptPrototypePropertySetNamedSetDlg()
{
}

void CApptPrototypePropertySetNamedSetDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ADD_BTN, m_btnAdd);
	DDX_Control(pDX, IDC_DELETE_BTN, m_btnDelete);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CApptPrototypePropertySetNamedSetDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CApptPrototypePropertySetNamedSetDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_ADD_BTN, &CApptPrototypePropertySetNamedSetDlg::OnBnClickedAddBtn)
	ON_BN_CLICKED(IDC_DELETE_BTN, &CApptPrototypePropertySetNamedSetDlg::OnBnClickedDeleteBtn)
END_MESSAGE_MAP()


namespace ENamedSetListColumns {
	enum _Enum {
		ID,
		Name,
		DetailsText,
	};
};
// CApptPrototypePropertySetNamedSetDlg message handlers

CString CApptPrototypePropertySetNamedSetDlg::CalculateDetailsText(const CMap<LONG,LONG,BYTE,BYTE> &details)
{
	if (!details.IsEmpty()) {
		int nExpectedLength = 0;
		CStringSortedArrayNoCase sorted;
		for (POSITION pos = details.GetStartPosition(); pos != 0; ) {
			LONG detailID;
			BYTE dontcare;
			details.GetNextAssoc(pos, detailID, dontcare);
			// Add the name of this one
			{
				CMap<LONG, LONG, CPossibleDetail, CPossibleDetail &>::CPair *pPair = m_mapAllPossibleDetails.PLookup(detailID);
				if (pPair != NULL) {
					CString &str = pPair->value.m_strName;
					sorted.Insert(str);
					nExpectedLength += str.GetLength() + 2;
				} else {
					// This should only happen if our map is incomplete somehow, which could either mean the caller 
					// gave us a bad query with which to populate our map, or the data no longer has an entry for 
					// one of the details the caller passed us.  All we can do is report it as unknown.
					sorted.Insert(_T("{Unknown}"));
					nExpectedLength += 11;
				}
			}
		}
		// Now join the array elements into a comma-delimited string and return it
		{
			CString ans;
			ans.Preallocate(nExpectedLength);
			int countAllButLast = sorted.GetSize() - 1;
			for (int i=0; i<countAllButLast; i++) {
				ans += sorted.GetAt(i) + _T(", ");
			}
			ans += sorted.GetAt(countAllButLast);
			return ans;
		}
	} else {
		return _T("{None}");
	}
}

BOOL CApptPrototypePropertySetNamedSetDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		// Set button styles
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnDelete.AutoSet(NXB_DELETE);
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// Load all possible detail values
		{
			m_mapAllPossibleDetails.RemoveAll();
			_RecordsetPtr prs;
			prs = CreateParamRecordset(GetRemoteData(), m_strAllPossibleDetailsSql);
			FieldsPtr flds = prs->GetFields();
			FieldPtr fldID = flds->GetItem(_T("ID"));
			FieldPtr fldName = flds->GetItem(_T("Name"));
			FieldPtr fldInactive = flds->GetItem(_T("Inactive"));
			while (!prs->eof) {
				LONG id = AdoFldLong(fldID);
				m_mapAllPossibleDetails.SetAt(id, CPossibleDetail(id, AdoFldString(fldName), AdoFldBool(fldInactive)));
				prs->MoveNext();
			}
		}

		// Fill our dl2 with the list of sets
		NXDATALIST2Lib::_DNxDataListPtr pdl = GetDlgItem(IDC_NAMEDSET_LIST)->GetControlUnknown();
		pdl->GetColumn(ENamedSetListColumns::DetailsText)->PutColumnTitle(AsBstr(m_strFriendlySetName + _T("(s)")));
		pdl->Clear();
		for (INT_PTR i=0, nCount=m_LocalCopy.GetSize(); i<nCount; i++) {
			CApptPrototypePropertySetNamedSet &set = m_LocalCopy.GetAt(i);
			NXDATALIST2Lib::IRowSettingsPtr pRow = pdl->GetNewRow();
			pRow->PutValue(ENamedSetListColumns::ID, set.m_nID);
			pRow->PutValue(ENamedSetListColumns::Name, AsVariant(set.m_strName));
			pRow->PutValue(ENamedSetListColumns::DetailsText, AsVariant(CalculateDetailsText(set.m_mapdwDetailIDs)));
			pdl->AddRowSorted(pRow, NULL);
		}

		// Give a friendly title
		SetWindowText(_T("Edit ") + m_strFriendlySetName + _T(" Sets"));

		// Select none and disable the delete button (because nothing is selected)
		pdl->PutCurSel(NULL);
		GetDlgItem(IDC_DELETE_BTN)->EnableWindow(FALSE);
	} NxCatchAllCall(__FUNCTION__, { EndDialog(IDCANCEL); return FALSE; });

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CApptPrototypePropertySetNamedSetDlg::OnBnClickedOk()
{
	// Save the changes back to the caller's data source
	m_pDataSource->Copy(m_LocalCopy);
	
	// And close with an IDOK result
	OnOK();
}

LONG CApptPrototypePropertySetNamedSetDlg::GetNextNegativeID()
{
	return m_nNextNegativeID;
}

INT_PTR CApptPrototypePropertySetNamedSetDlg::DoModal(const CString &strFriendlySetName, CArray<CApptPrototypePropertySetNamedSet, CApptPrototypePropertySetNamedSet &> &dataSource, LONG nNextNegativeID, const CString &strAllPossibleDetailsSql)
{
	// Make a local copy for editing
	m_LocalCopy.Copy(dataSource);
	// Remember the callers so we can write back to it if the user commits
	m_pDataSource = &dataSource;
	// Remember the query to be used for loading the list of possible detail entries for this set
	m_strAllPossibleDetailsSql = strAllPossibleDetailsSql;
	// Remember the user-friendly description of the kind of set we're editing
	m_strFriendlySetName = strFriendlySetName;
	// Initialize to the next negative ID given by the caller so we can track newly added sets uniquely
	m_nNextNegativeID = nNextNegativeID;
	
	// Do modal now that we've initialized properly
	m_bInitialized = TRUE;
	return DoModal();
}

INT_PTR CApptPrototypePropertySetNamedSetDlg::DoModal()
{
	if (m_bInitialized) {
		return CNxDialog::DoModal();
	} else {
		ThrowNxException(_T("Dialog cannot open because it has not been given initialization properties."));
	}
}
BEGIN_EVENTSINK_MAP(CApptPrototypePropertySetNamedSetDlg, CNxDialog)
	ON_EVENT(CApptPrototypePropertySetNamedSetDlg, IDC_NAMEDSET_LIST, 19, CApptPrototypePropertySetNamedSetDlg::LeftClickNamedsetList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CApptPrototypePropertySetNamedSetDlg, IDC_NAMEDSET_LIST, 2, CApptPrototypePropertySetNamedSetDlg::SelChangedNamedsetList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CApptPrototypePropertySetNamedSetDlg, IDC_NAMEDSET_LIST, 9, CApptPrototypePropertySetNamedSetDlg::EditingFinishingNamedsetList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CApptPrototypePropertySetNamedSetDlg, IDC_NAMEDSET_LIST, 10, CApptPrototypePropertySetNamedSetDlg::EditingFinishedNamedsetList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
END_EVENTSINK_MAP()

void CApptPrototypePropertySetNamedSetDlg::LeftClickNamedsetList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	if (lpRow != NULL && nCol == ENamedSetListColumns::DetailsText) {
		CApptPrototypePropertySetNamedSet &set = FindAssociatedNamedSet(lpRow);
		
		// Get a sorted list of all possible details, only including the inactive ones if they are already selected
		CStringSortedArrayNoCase names;
		CArray<LONG,LONG> ids;
		{
			for (CMap<LONG, LONG, CPossibleDetail, CPossibleDetail &>::CPair *pPair = m_mapAllPossibleDetails.PGetFirstAssoc(); pPair != NULL; pPair = m_mapAllPossibleDetails.PGetNextAssoc(pPair)) {
				if (!pPair->value.m_bInactive || set.m_mapdwDetailIDs.PLookup(pPair->key) != NULL) {
					int nInsertAtIndex;
					// It'd be pretty unusual to have duplicate names (because right now we only support 
					// resourcesets and aptpurposesets which don't allow duplicates) but technically we 
					// can handle it, so just ignore the return value (which tells us whether it was in 
					// the list already or not).
					names.FindNearestGreater(pPair->value.m_strName, nInsertAtIndex);
					// Insert into both lists at that position
					names.InsertAt(nInsertAtIndex, pPair->value.m_strName);
					ids.InsertAt(nInsertAtIndex, pPair->key);
				}
			}
		}
		// Figure out which ones are to be selected
		CVariantArray selected;
		{
			for (INT_PTR i=0, nCount=ids.GetSize(); i<nCount; i++) {
				if (set.m_mapdwDetailIDs.PLookup(ids.GetAt(i)) != NULL) {
					// Inexplicably the CMultiSelectDlg uses string IDs when opened with a stringarray, yet 
					// doesn't offer CStringArray typed PreSelect() function, so we use the CVariantArray 
					// typed function.
					selected.Add(AsVariant(AsString((long)i)));
				}
			}
		}

		// Now prompt the user to update the selections
		// (j.armen 2012-06-08 09:32) - PLID 49607 - Input for this multiselect dlg isn't cacheable, so let's set a manual cache name
		CMultiSelectDlg dlg(this, "CApptPrototypePropertySetNamedSetDlg::LeftClickNamedsetList");
		dlg.PreSelect(selected);
		if (dlg.OpenWithStringArray(names, _T("Please select the ") + m_strFriendlySetName + _T("s for this set"), 0) == IDOK) {
			// Store the user's new selection(s) back to our set
			dlg.FillArrayWithIDs(selected);
			set.m_mapdwDetailIDs.RemoveAll();
			for (INT_PTR i=0, nCount=selected.GetSize(); i<nCount; i++) {
				set.m_mapdwDetailIDs.SetAt(ids.GetAt(selected.GetAt(i)), 0);
			}
			// And reflect the new selection(s) on screen
			IRowSettingsPtr pRow = lpRow;
			pRow->PutValue(ENamedSetListColumns::DetailsText, AsVariant(CalculateDetailsText(set.m_mapdwDetailIDs)));
		}
	}
}

void CApptPrototypePropertySetNamedSetDlg::SelChangedNamedsetList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		GetDlgItem(IDC_DELETE_BTN)->EnableWindow((lpNewSel != NULL) ? TRUE : FALSE);
	} NxCatchAll(__FUNCTION__);
}

CApptPrototypePropertySetNamedSet &CApptPrototypePropertySetNamedSetDlg::FindAssociatedNamedSet(LPDISPATCH lpRow)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = lpRow;
	LONG nCurID = VarLong(pRow->GetValue(ENamedSetListColumns::ID));
	BOOL found = FALSE;
	for (INT_PTR i=0, nCount=m_LocalCopy.GetSize(); i<nCount; i++) {
		CApptPrototypePropertySetNamedSet &set = m_LocalCopy.GetAt(i);
		if (set.m_nID == nCurID) {
			return set;
		}
	}
	ThrowNxException(_T("Could not find set associated with currently selected row."));
}

void CApptPrototypePropertySetNamedSetDlg::OnBnClickedAddBtn()
{
	try {
		NXDATALIST2Lib::_DNxDataListPtr pdl = GetDlgItem(IDC_NAMEDSET_LIST)->GetControlUnknown();
		// Try to find a unique new name
		_variant_t varNewName;
		CString strNewName;
		{
			for (LONG n=1; n<10000; n++) {
				strNewName.Format(_T("New %li"), n);
				varNewName = AsVariant(strNewName);
				if (pdl->FindByColumn(ENamedSetListColumns::Name, varNewName, NULL, VARIANT_FALSE) == NULL) {
					break;
				}
			}
		}
		// Add an invalid entry to the array
		INT_PTR index = m_LocalCopy.Add(CApptPrototypePropertySetNamedSet(0, strNewName));

		// Add this entry to the datalist2
		CApptPrototypePropertySetNamedSet &set = m_LocalCopy[index];
		NXDATALIST2Lib::IRowSettingsPtr pRow = pdl->GetNewRow();
		pRow->PutValue(ENamedSetListColumns::ID, m_nNextNegativeID);
		pRow->PutValue(ENamedSetListColumns::Name, varNewName);
		pRow->PutValue(ENamedSetListColumns::DetailsText, AsVariant(CalculateDetailsText(set.m_mapdwDetailIDs)));
		pRow = pdl->AddRowSorted(pRow, NULL);

		// Now that we've successfully added to memory and on screen, store the valid (but pseudo) ID in memory remember that this ID is now in use
		set.m_nID = m_nNextNegativeID;
		m_nNextNegativeID--;

		// Make sure it's selected, and start editing it since the user is obviously going to want to rename it
		pdl->PutCurSel(pRow);
		GetDlgItem(IDC_DELETE_BTN)->EnableWindow(TRUE);
		pdl->StartEditing(pRow, ENamedSetListColumns::Name);
	} NxCatchAll(__FUNCTION__);
}

void CApptPrototypePropertySetNamedSetDlg::OnBnClickedDeleteBtn()
{
	try {
		NXDATALIST2Lib::_DNxDataListPtr pdl = GetDlgItem(IDC_NAMEDSET_LIST)->GetControlUnknown();
		pdl->StopEditing(VARIANT_FALSE);
		NXDATALIST2Lib::IRowSettingsPtr pCurRow = pdl->GetCurSel();
		if (pCurRow != NULL) {
			LONG id = VarLong(pCurRow->GetValue(ENamedSetListColumns::ID));
			BOOL foundAndRemoved = FALSE;
			for (INT_PTR i=0, nCount=m_LocalCopy.GetSize(); i<nCount; i++) {
				if (m_LocalCopy.GetAt(i).m_nID == id) {
					m_LocalCopy.RemoveAt(i);
					foundAndRemoved = TRUE;
					break;
				}
			}
			if (foundAndRemoved) {
				pdl->RemoveRow(pCurRow);
				// Reflect the correct enabled state for the delete button
				GetDlgItem(IDC_DELETE_BTN)->EnableWindow(pdl->GetCurSel() != NULL);
			} else {
				ThrowNxException(_T("Could not find the current row in memory."));
			}
		} else {
			MessageBox(_T("Please select the ") + m_strFriendlySetName + _T(" you want to delete."), NULL, MB_OK|MB_ICONEXCLAMATION);
		}
	} NxCatchAll(__FUNCTION__);
}

void CApptPrototypePropertySetNamedSetDlg::EditingFinishingNamedsetList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	if (pbCommit != NULL && (*pbCommit) && pbContinue != NULL && (*pbContinue)) {
		try {
			if (lpRow != NULL) {
				if (nCol == ENamedSetListColumns::Name) {
					// Validate the new name
					CString strNewValue = strUserEntered;
					strNewValue.Trim();
					{
						// Make sure the name is not empty
						if (strNewValue.IsEmpty()) {
							MessageBox(_T("Please enter a friendly name for this set."), _T("Invalid name"), MB_OK|MB_ICONEXCLAMATION);
							*pbContinue = FALSE;
							return;
						}

						// Make sure the name is not too long
						if (strNewValue.GetLength() > 200) {
							MessageBox(_T("The friendly name you entered is too long.  Please enter a friendly name 200 characters or shorter."), _T("Invalid name"), MB_OK|MB_ICONEXCLAMATION);
							*pbContinue = FALSE;
							return;
						}

						// Make sure the name isn't already in use
						NXDATALIST2Lib::IRowSettingsPtr pRow = lpRow;
						LONG nCurID = VarLong(pRow->GetValue(ENamedSetListColumns::ID));
						for (INT_PTR i=0, nCount=m_LocalCopy.GetSize(); i<nCount; i++) {
							CApptPrototypePropertySetNamedSet &set = m_LocalCopy[i];
							if (set.m_nID != nCurID) {
								if (set.m_strName.CompareNoCase(strNewValue) == 0) {
									MessageBox(_T("The friendly name you entered is already in use by another set.  Please enter a unique friendly name."), _T("Invalid name"), MB_OK|MB_ICONEXCLAMATION);
									*pbContinue = FALSE;
									return;
								}
							}
						}

						// Reflect the new value we interpret it to be
						VariantClear(pvarNewValue);
						(*pvarNewValue) = AsVariant(strNewValue).Detach();
					}
				} else {
					ThrowNxException(_T("Cannot edit a cell outside the Name column."));
				}
			} else {
				ThrowNxException(_T("Cannot edit non-row."));
			}
		} NxCatchAllCall(__FUNCTION__, { *pbContinue = FALSE; });
	}
}

void CApptPrototypePropertySetNamedSetDlg::EditingFinishedNamedsetList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	if (bCommit) {
		try {
			if (lpRow != NULL) {
				if (nCol == ENamedSetListColumns::Name) {
					// Save the change in our class-local memory
					FindAssociatedNamedSet(lpRow).m_strName = VarString(varNewValue);
				} else {
					ThrowNxException(_T("Cannot finish editing a cell outside the Name column."));
				}
			} else {
				ThrowNxException(_T("Cannot finish editing non-row."));
			}
		} NxCatchAll(__FUNCTION__);
	}
}
