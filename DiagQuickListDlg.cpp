// DiagQuickListDlg.cpp : implementation file
// (c.haag 2014-02-21) - PLID 60931 - Initial implementation
//

#include "stdafx.h"
#include "DiagQuickListDlg.h"
#include "DiagQuickListCopyFromUserDlg.h"
#include "DiagSearchUtils.h"
#include "DiagQuickListUtils.h"
#include "DiagCodeSearchDlg.h"
#include "AsyncDiagSearchQuery.h"
#include "NexCodeDlg.h"
#include "NxAPI.h"
#include "NexGem.h"

#define SHARED_BACK_COLOR	RGB(192,192,192)

using namespace NXDATALIST2Lib;

enum UserListColumns
{
	ulcID = 0,
	ulcChecked,
	ulcUsername,
};

enum QuickListColumns
{
	qlcID = 0,
	qlcChecked,
	qlcIsShared,
	qlcICD9ID,
	qlcICD9CodeNumber,
	qlcICD9Description,
	qlcICD10ID,
	qlcICD10CodeNumber,
	qlcICD10Description,
};

// (r.farnworth 2014-07-18 12:40) - PLID 62967
struct CDiagQuickListDlg::DiagnosisCodeCommit{
	CArray<NexTech_Accessor::_DiagnosisCodeCommitPtr, NexTech_Accessor::_DiagnosisCodeCommitPtr> aryMultiMatch;
};

// CDiagQuickListDlg dialog

IMPLEMENT_DYNAMIC(CDiagQuickListDlg, CNxDialog)

CDiagQuickListDlg::CDiagQuickListDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CDiagQuickListDlg::IDD, pParent),
	m_pDiagCodeCommitMultiMatch(new DiagnosisCodeCommit())
{
	m_bIsEMRTemplate = FALSE;
	__super::SetMinSize(450, 400);
	m_pDiagCodeCommitMultiMatch->aryMultiMatch.RemoveAll(); // (r.farnworth 2014-07-18 12:40) - PLID 62967
}

CDiagQuickListDlg::~CDiagQuickListDlg()
{
}

// Requeries the QuickList, and optionally, the shared user list
void CDiagQuickListDlg::Requery(BOOL bRequerySharedUserList)
{
	NexTech_Accessor::_DiagQuickListPtr quickList = DiagQuickListUtils::GetDiagQuickListForCurrentUser();

	// Populate the shared user list
	if (bRequerySharedUserList)
	{
		Nx::SafeArray<IUnknown *> saryUsersWithQuickLists(quickList->UsersWithQuickLists);
		m_UserList->Clear();
		foreach(NexTech_Accessor::_UserPtr pUser, saryUsersWithQuickLists)
		{
			IRowSettingsPtr pRow = m_UserList->GetNewRow();
			pRow->Value[ulcID] = pUser->ID;
			pRow->Value[ulcChecked] = g_cvarFalse;
			pRow->Value[ulcUsername] = pUser->username;
			m_UserList->AddRowAtEnd(pRow, NULL);
		}

		// Check boxes for users whose QuickList items we are sharing
		Nx::SafeArray<BSTR> sarySharedUserIDs(quickList->sharedUserIDs);
		foreach(_bstr_t userID, sarySharedUserIDs)
		{
			IRowSettingsPtr pUserRow = m_UserList->FindByColumn(ulcID, userID, NULL, VARIANT_FALSE);
			if (NULL != pUserRow)
			{
				pUserRow->Value[ulcChecked] = g_cvarTrue;
			}
			else
			{
				ThrowNxException("Failed to find user %s in the QuickList user list!", (LPCTSTR)userID);
			}
		}
	}

	// Remember the selected items
	struct Crosswalk
	{
		_variant_t ICD9ID;
		_variant_t ICD10ID;
	};
	CArray<Crosswalk,Crosswalk&> aSelectedItems;
	for (IRowSettingsPtr pRow = m_QuickList->GetFirstRow(); NULL != pRow; pRow = pRow->GetNextRow())
	{
		// (r.farnworth 2014-08-22 11:48) - PLID 62762 - The situation where the check could be null here never came up before, easier now to just check that the value is true
			if (pRow->Value[qlcChecked] == g_cvarTrue)
			{
				Crosswalk crosswalk;
				crosswalk.ICD9ID = pRow->Value[qlcICD9ID];
				crosswalk.ICD10ID = pRow->Value[qlcICD10ID];
				aSelectedItems.Add(crosswalk);
			}
	}

	// Populate the QuickList
	Nx::SafeArray<IUnknown *> saryItems(quickList->Items);
	m_QuickList->Clear();

	NexTech_Accessor::DiagDisplayType diagDisplayType = DiagQuickListUtils::GetAPIDiagDisplayType();
	bool bUnmatched9Code = false;
	bool bUnmatched10Code = false;
	bool bUnmatchedShared = false; // (r.farnworth 2014-08-22 11:48) - PLID 62762 - We need to keep the columns expanded if we have unmatched shared codes

	// (r.farnworth 2014-07-14 12:37) - PLID 62816 - Loop through our quicklist items and determine wheter or not we have any unmatched codes
	foreach(NexTech_Accessor::_DiagQuickListItemPtr pItem, saryItems)
	{
		//We dont want the columns to expand for shared items
		if (diagDisplayType == NexTech_Accessor::DiagDisplayType_ICD10 && pItem->ICD10 == NULL && pItem->ICD9 != NULL)
		{
			bUnmatched9Code = true;
			if (VarShort(pItem->IsShared) != 0) 
			{ 
				bUnmatchedShared = true; 
			}
		}
		else if (diagDisplayType == NexTech_Accessor::DiagDisplayType_ICD9 && pItem->ICD10 != NULL && pItem->ICD9 == NULL)
		{
			bUnmatched10Code = true;
			if (VarShort(pItem->IsShared) != 0) 
			{
				bUnmatchedShared = true; 
			}
		}

		AddQuickListItem(pItem, FALSE);
	}

	// (r.farnworth 2014-07-14 12:37) - PLID 62816 - If we have an unmatched code of the opposite type of our search preference, then we want to keep the columns from collapsing.
	// Moved from OnInit to be recalled when we need it
	if (!bUnmatchedShared)
	{
		if (!IsShowingICD9s() && !bUnmatched9Code)
		{
			m_QuickList->GetColumn(qlcICD9CodeNumber)->ColumnStyle = csFixedWidth | csVisible;
			m_QuickList->GetColumn(qlcICD9CodeNumber)->StoredWidth = 0;
			m_QuickList->GetColumn(qlcICD9Description)->ColumnStyle = csFixedWidth | csVisible;
			m_QuickList->GetColumn(qlcICD9Description)->StoredWidth = 0;
		}
		if (!IsShowingICD10s() && !bUnmatched10Code)
		{
			m_QuickList->GetColumn(qlcICD10CodeNumber)->ColumnStyle = csFixedWidth | csVisible;
			m_QuickList->GetColumn(qlcICD10CodeNumber)->StoredWidth = 0;
			m_QuickList->GetColumn(qlcICD10Description)->ColumnStyle = csFixedWidth | csVisible;
			m_QuickList->GetColumn(qlcICD10Description)->StoredWidth = 0;
		}
	}
	else
	{
		// (r.farnworth 2014-08-22 11:48) - PLID 62762 - We need to keep the columns expanded if we have unmatched shared codes
		m_QuickList->GetColumn(qlcICD9CodeNumber)->ColumnStyle = csFixedWidth | csVisible;
		m_QuickList->GetColumn(qlcICD9CodeNumber)->StoredWidth = 80;
		m_QuickList->GetColumn(qlcICD9Description)->ColumnStyle = csFixedWidth | csVisible | csWidthAuto;
		m_QuickList->GetColumn(qlcICD10CodeNumber)->ColumnStyle = csFixedWidth | csVisible;
		m_QuickList->GetColumn(qlcICD10CodeNumber)->StoredWidth = 80;
		m_QuickList->GetColumn(qlcICD10Description)->ColumnStyle = csFixedWidth | csVisible | csWidthAuto;
	}

	// Now select what we had before
	int nSelectedItems = aSelectedItems.GetCount();
	if (nSelectedItems > 0)
	{
		for (IRowSettingsPtr pRow = m_QuickList->GetFirstRow(); NULL != pRow; pRow = pRow->GetNextRow())
		{
			for (int i=0; i < nSelectedItems; i++)
			{
				if (VARCMP_EQ == VariantCompare( &pRow->Value[qlcICD9ID], &aSelectedItems[i].ICD9ID )
					&& VARCMP_EQ == VariantCompare( &pRow->Value[qlcICD10ID], &aSelectedItems[i].ICD10ID ))
				{
					pRow->Value[qlcChecked] = g_cvarTrue;
					break;
				}
			}
		}
	} // if (nSelectedItems > 0)
}

// (c.haag 2014-02-24) - PLID 60940 - Adds multiple QuickList items to the list
void CDiagQuickListDlg::AddQuickListItems(IDispatchPtr pDisp, BOOL bFromSearch)
{
	NexTech_Accessor::_AddDiagCodesToQuickListResultPtr pResult(pDisp);
	Nx::SafeArray<IUnknown *> saryNewQuickListItems(pResult->NewItems);
	NexTech_Accessor::DiagDisplayType diagDisplayType = DiagQuickListUtils::GetAPIDiagDisplayType();
	// (r.farnworth 2014-07-16 16:07) - PLID 62891
	bool bUnmatched9Code = false;
	bool bUnmatched10Code = false;
	// Do for all QuickList items
	foreach(NexTech_Accessor::_DiagQuickListItemPtr pItem, saryNewQuickListItems)
	{
		// These are the values in the code that we want to add to our QuickList
		_variant_t vNewICD9ID = (NULL != pItem->ICD9) ? pItem->ICD9->ID : g_cvarNull;
		_variant_t vNewICD10ID = (NULL != pItem->ICD10) ? pItem->ICD10->ID : g_cvarNull;

		// Avoid cases where we would add empty rows
		if ((vNewICD9ID.vt == VT_NULL && vNewICD10ID.vt == VT_NULL)) // Neither code filled in
		{
			continue;
		}

		if (!CheckForDuplicates(vNewICD9ID, vNewICD10ID))
		{
			// (r.farnworth 2014-07-16 16:07) - PLID 62891
			if (diagDisplayType == NexTech_Accessor::DiagDisplayType_ICD10 && pItem->ICD10 == NULL && pItem->ICD9 != NULL)
			{
				bUnmatched9Code = true;
			}
			else if (diagDisplayType == NexTech_Accessor::DiagDisplayType_ICD9 && pItem->ICD10 != NULL && pItem->ICD9 == NULL)
			{
				bUnmatched10Code = true;
			}

			// Add the item to the list
			AddQuickListItem(pItem, bFromSearch);
		}
		else
		{
			// The row already exists; the user selected a row that's already in our quicklist.
			// There's nothing to do here
		}
	} // foreach(NexTech_Accessor::_DiagQuickListItemPtr pItem, saryNewQuickListItems)

	// (r.farnworth 2014-07-16 16:07) - PLID 62891 - If we're coming back from copying another user's quicklist we should check to re-expand the columns
	if (!bFromSearch)
	{
		if ((!IsShowingICD9s() && bUnmatched9Code) || (!IsShowingICD10s() && bUnmatched10Code))
		{
			m_QuickList->GetColumn(qlcICD9CodeNumber)->ColumnStyle = csFixedWidth | csVisible;
			m_QuickList->GetColumn(qlcICD9CodeNumber)->StoredWidth = 80;
			m_QuickList->GetColumn(qlcICD9Description)->ColumnStyle = csFixedWidth | csVisible | csWidthAuto;
			m_QuickList->GetColumn(qlcICD10CodeNumber)->ColumnStyle = csFixedWidth | csVisible;
			m_QuickList->GetColumn(qlcICD10CodeNumber)->StoredWidth = 80;
			m_QuickList->GetColumn(qlcICD10Description)->ColumnStyle = csFixedWidth | csVisible | csWidthAuto;
		}
	}
}

// (c.haag 2014-02-24) - PLID 60940 - Adds a single QuickList item to the list
void CDiagQuickListDlg::AddQuickListItem(IDispatchPtr pDisp, BOOL bFromSearch)
{
	NexTech_Accessor::_DiagQuickListItemPtr pItem(pDisp);
	IRowSettingsPtr pRow = m_QuickList->GetNewRow();

	// (r.farnworth 2014-07-15 15:45) - PLID 62891 - We don't want to display matched 9-Codes when in 10-Mode and vice-versa so we'll check whether or not we want to display data here.
	bool bDisplay9Code = ((IsShowingICD9s() && pItem->ICD9 != NULL) || (!IsShowingICD9s() && pItem->ICD9 != NULL && pItem->ICD10 == NULL));
	bool bDisplay10Code = ((IsShowingICD10s() && pItem->ICD10 != NULL) || (!IsShowingICD10s() && pItem->ICD9 == NULL && pItem->ICD10 != NULL));

	// (r.farnworth 2014-07-24 11:47) - PLID 62826 - We don't want to let the user add unmatched codes
	bool bHideCheck = ((IsShowingICD10s() && pItem->ICD9 != NULL && pItem->ICD10 == NULL) || (IsShowingICD9s() && pItem->ICD9 == NULL && pItem->ICD10 != NULL));

	pRow->Value[qlcID] = pItem->ID;
	pRow->Value[qlcChecked] = bHideCheck ? g_cvarNull : (bFromSearch ? g_cvarTrue : g_cvarFalse);
	pRow->Value[qlcIsShared] = (VarShort(pItem->IsShared) == 0) ? g_cvarFalse : g_cvarTrue;
	pRow->Value[qlcICD9ID] = (NULL != pItem->ICD9) ? pItem->ICD9->ID : g_cvarNull;
	pRow->Value[qlcICD9CodeNumber] = bDisplay9Code ? pItem->ICD9->Code : g_cvarNull;
	pRow->Value[qlcICD9Description] = bDisplay9Code ? pItem->ICD9->description : g_cvarNull;
	pRow->Value[qlcICD10ID] = (NULL != pItem->ICD10) ? pItem->ICD10->ID : g_cvarNull;
	pRow->Value[qlcICD10CodeNumber] = bDisplay10Code ? pItem->ICD10->Code : g_cvarNull;
	pRow->Value[qlcICD10Description] = bDisplay10Code ? pItem->ICD10->description : g_cvarNull;


	pRow = bFromSearch ? m_QuickList->AddRowAtEnd(pRow, NULL) : m_QuickList->AddRowSorted(pRow, NULL);
	if (VARIANT_FALSE != pItem->IsShared)
	{
		pRow->BackColor = SHARED_BACK_COLOR;
	}

	// (j.armen 2014-03-20 09:21) - PLID 60943 - If we are showing 9's and 10's, the 9 codes can be swapped out
	// (j.armen 2014-03-20 09:21) - PLID 60943 - If showing 9's and 10's, and the description is null, set the string with our placeholder, even if it is shared
	// (r.farnworth 2014-07-16 09:17) - PLID 62826 - OR if they're in 9 mode and the 9-description is null but the 10 isn't
	if(CanChange9Code(pRow) && pRow->Value[qlcICD9Description] == g_cvarNull
		&& ((IsShowingICD9s() && IsShowingICD10s()) || (!IsShowingICD10s() && pItem->ICD10 != NULL)))
	{
		SetHyperLinkFormat(pRow, qlcICD9Description, false);
		pRow->Value[qlcICD9Description] = "< No Matching Code >";
	}
	else if (CanChange10Code(pRow) && pRow->Value[qlcICD10Description] == g_cvarNull
		&& ((IsShowingICD9s() && IsShowingICD10s()) || (!IsShowingICD9s() && pItem->ICD9 != NULL)))
	{
		// (r.farnworth 2014-07-17 09:36) - PLID 62826 - Address unmatched 10s
		SetHyperLinkFormat(pRow, qlcICD10Description , false);
		pRow->Value[qlcICD10Description] = "< Find ICD-10 Code >";
	}
	
}

// (c.haag 2014-02-24) - PLID 60940 - Returns TRUE if the specified QuickList row belongs
// to another Practice user
BOOL CDiagQuickListDlg::IsSharedQuickListRow(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	return VarBool(pRow->Value[qlcIsShared]);
}

// (c.haag 2014-02-25) - PLID 60947 - Returns TRUE if at least one user is selected in the
// shared users list
BOOL CDiagQuickListDlg::HasSharedLists()
{
	for (IRowSettingsPtr pRow = m_UserList->GetFirstRow(); NULL != pRow; pRow = pRow->GetNextRow())
	{
		if (VarBool(pRow->Value[ulcChecked]))
		{
			return TRUE;
		}
	}
	return FALSE;
}

// Returns TRUE if we're showing ICD-9s
BOOL CDiagQuickListDlg::IsShowingICD9s()
{
	// Show columns and data based on the global Diagnosis Search Preference
	NexTech_Accessor::DiagDisplayType diagDisplayType = DiagQuickListUtils::GetAPIDiagDisplayType();
	return (diagDisplayType == NexTech_Accessor::DiagDisplayType_Crosswalk
		|| diagDisplayType == NexTech_Accessor::DiagDisplayType_ICD9);
}

// Returns TRUE if we're showing ICD-10s
BOOL CDiagQuickListDlg::IsShowingICD10s()
{
	// Show columns and data based on the global Diagnosis Search Preference
	NexTech_Accessor::DiagDisplayType diagDisplayType = DiagQuickListUtils::GetAPIDiagDisplayType();
	return (diagDisplayType == NexTech_Accessor::DiagDisplayType_Crosswalk
		|| diagDisplayType == NexTech_Accessor::DiagDisplayType_ICD10);
}

bool CDiagQuickListDlg::CanChange9Code(IRowSettingsPtr pRow)
{
	if(!pRow)
		return false;
	// (r.farnworth 2014-07-16 09:15) - PLID 62826 - Removed returning false if they were in 9 mode
	if(!IsShowingICD9s())
		return false;

	if(pRow->Value[qlcICD10ID] == g_cvarNull)
		return false;

	if(IsSharedQuickListRow(pRow))
		return false;

	return true;
}

// (j.armen 2014-03-20 09:12) - PLID 60943 - Allows the user to reassign the 9 code in a quicklist
void CDiagQuickListDlg::Change9Code(IRowSettingsPtr pRow)
{
	// Don't allow shared rows to be changed
	if(IsSharedQuickListRow(pRow)) {
		AfxMessageBox("You cannot set the ICD-9 code for a shared QuickList item");
		return;
	}

	// Run our search
	boost::optional<DiagCode9> pCode = CDiagCodeSearchDlg::DoManagedICD9Search(this);
	if(pCode)
	{
		// Update Display
		if (!CheckForDuplicates(_bstr_t(AsString(pCode->nDiagCodesID)), pRow->Value[qlcICD10ID]))
		{
			pRow->Value[qlcICD9ID] = _bstr_t(pCode->nDiagCodesID);
			pRow->Value[qlcICD9CodeNumber] = pCode->strCode;
			pRow->Value[qlcICD9Description] = pCode->strDescription;
		}
		else
		{
			MessageBox("This diagnosis code has already been selected.", NULL, MB_OK | MB_ICONEXCLAMATION);
			return;
		}

		Nx::SafeArray<BSTR> quickListIDsToDelete = Nx::SafeArray<BSTR>::FromValue(_bstr_t(pRow->Value[qlcID]));

		NexTech_Accessor::_QuickListDiagCommitPtr pCommit(__uuidof(NexTech_Accessor::QuickListDiagCommit));
		pCommit->ICD9DiagID = _bstr_t(pRow->Value[qlcICD9ID]);
		pCommit->ICD10DiagID = _bstr_t(pRow->Value[qlcICD10ID]);
		Nx::SafeArray<IUnknown *> saCommits = Nx::SafeArray<IUnknown *>::FromValue(pCommit);

		// Remove the old quicklist item and add the new one.
		// This should be an infrequent operation, thus we are going to use
		// existing API functionality rather than create a new function
		GetAPI()->RemoveDiagQuickListItemsAndRelatedBasedOnDisplayType(GetAPISubkey(), GetAPILoginToken(), quickListIDsToDelete, DiagQuickListUtils::GetAPIDiagDisplayType());
		GetAPI()->AddDiagCodesToQuickList(GetAPISubkey(), GetAPILoginToken(), saCommits);

		// (r.farnworth 2014-07-16 15:41) - PLID 62826 - Remove Hyperlinking.
		SetHyperLinkFormat(pRow, qlcICD9Description, true);
		// (r.farnworth 2014-07-24 11:47) - PLID 62826 - Add the checkbox again, unchecked
		pRow->Value[qlcChecked] = g_cvarFalse;
	}
}

void CDiagQuickListDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWoundCareSetupDlg)
	DDX_Control(pDX, IDCANCEL, m_btnClose);
	DDX_Control(pDX, IDC_BTN_CLEAR_MY_QUICKLIST, m_btnClearMyQuickList);
	DDX_Control(pDX, IDC_BTN_ADD_CODES_TO_VISIT, m_btnAddCodesToVisit);
	//}}AFX_DATA_MAP
}
BEGIN_MESSAGE_MAP(CDiagQuickListDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_COPY_FROM_USER, &CDiagQuickListDlg::OnBnClickedBtnCopyFromUser)
	ON_BN_CLICKED(IDC_BTN_CLEAR_MY_QUICKLIST, &CDiagQuickListDlg::OnBnClickedBtnClearMyQuicklist)
	ON_BN_CLICKED(IDC_BTN_ADD_CODES_TO_VISIT, &CDiagQuickListDlg::OnBnClickedBtnAddCodesToVisit)
	ON_WM_NCHITTEST()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CDiagQuickListDlg, CNxDialog)
	ON_EVENT(CDiagQuickListDlg, IDC_DIAG_SEARCH_LIST, 16, CDiagQuickListDlg::OnSelChosenDiagSearchList, VTS_DISPATCH)
	ON_EVENT(CDiagQuickListDlg, IDC_QUICKLIST_USERLIST, 10, CDiagQuickListDlg::OnEditingFinishedUserList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CDiagQuickListDlg, IDC_QUICKLIST, 6, CDiagQuickListDlg::OnRButtonDownQuickList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CDiagQuickListDlg, IDC_QUICKLIST, 19, CDiagQuickListDlg::LeftClickQuickList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CDiagQuickListDlg, IDC_QUICKLIST, 10, CDiagQuickListDlg::EditingFinishedDiags, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
END_EVENTSINK_MAP()

BOOL CDiagQuickListDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnAddCodesToVisit.AutoSet(NXB_OK);
		m_btnClearMyQuickList.AutoSet(NXB_DELETE);

		// (c.haag 2014-03-05) - PLID 60930 - Adjust the Visit button label if necessary
		if (m_bIsEMRTemplate)
		{
			m_btnAddCodesToVisit.SetWindowText("&Add To Template");
		}

		m_UserList = GetDlgItemUnknown(IDC_QUICKLIST_USERLIST);
		m_QuickList = GetDlgItemUnknown(IDC_QUICKLIST);
		m_diagSearch = DiagSearchUtils::BindDiagPreferenceSearchListCtrl(this, IDC_DIAG_SEARCH_LIST, GetRemoteData());

		// Bulk caching
		g_propManager.CachePropertiesInBulk("CDiagQuickListDlg", propNumber,
				"(Username = '%s') AND ("
				"Name = 'DiagQuickListDlg_LastDiagSearchPreference' "
				")",
				_Q(GetCurrentUserName()));
		g_propManager.CachePropertiesInBulk("CDiagQuickListDlg", propText,
				"(Username = '%s') AND ("
				"Name = 'DiagQuickListDlg_ColumnSort' "
				")",
				_Q(GetCurrentUserName()));

		// We always recall the column sort order unless this is the first time the user entered this dialog,
		// or the user changed their search style since the previous time they opened this dialog. Populate
		// bRecallColumnSort with whether we will recall the sort columns or not.
		BOOL bRecallColumnSort = TRUE;
		CString strSort = GetRemotePropertyText("DiagQuickListDlg_ColumnSort", "", 0, GetCurrentUserName(), false);
		int iLastDiagSearchPreference = GetRemotePropertyInt("DiagQuickListDlg_LastDiagSearchPreference", -1, 0, GetCurrentUserName(), false);
		int iSearchStyle = (int)DiagSearchUtils::GetPreferenceSearchStyle();
		if (-1 == iLastDiagSearchPreference
			|| -1 == iSearchStyle // Should never be true
			|| iSearchStyle != iLastDiagSearchPreference
			|| strSort.IsEmpty()
			)
		{
			bRecallColumnSort = FALSE;
		}

		// Now recall the column sort
		if (bRecallColumnSort)
		{
			// strSort is in the form "column_index;sort_ascending;sort_priority;columN_index;sort_ascending;sort_priority"
			// Translate it into an array
			CArray<long,long> aSort;
			StringAsArray(strSort, aSort, false);

			// Do for all sorted columns
			for (int i=0; i < aSort.GetCount(); i += 3)
			{
				int nCol = aSort[i];
				m_QuickList->GetColumn(nCol)->SortAscending = (0 == aSort[i+1]) ? VARIANT_FALSE : VARIANT_TRUE;
				m_QuickList->GetColumn(nCol)->SortPriority = (short)aSort[i+2];
			}

		} // if (bRecallColumnSort)
		else
		{
			// Don't assign any sort columns. We should defer to the API sort order.
		}

		Requery(TRUE);
	}
	NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
}

// (c.haag 2014-02-24) - PLID 60940 - Add the selected code from the Global Diagnosis Search into this quick list
void CDiagQuickListDlg::OnSelChosenDiagSearchList(LPDISPATCH lpRow)
{
	try 
	{
		if (NULL != lpRow)
		{
			// Add to data
			CDiagSearchResults results = DiagSearchUtils::ConvertPreferenceSearchResults(lpRow);
			NexTech_Accessor::_QuickListDiagCommitPtr pCommit(__uuidof(NexTech_Accessor::QuickListDiagCommit));
			if (-1 != results.m_ICD9.m_nDiagCodesID || -1 != results.m_ICD10.m_nDiagCodesID)
			{
				if (-1 != results.m_ICD9.m_nDiagCodesID)
				{
					pCommit->ICD9DiagID = _bstr_t(AsString(results.m_ICD9.m_nDiagCodesID));
				}
				else if (IsShowingICD9s() && IsShowingICD10s() && -1 != results.m_ICD10.m_nDiagCodesID)
				{
					// (j.armen 2014-03-24 10:08) - PLID 60943 - If no 9 exists, run the managed search
					boost::optional<DiagCode9> pCode = CDiagCodeSearchDlg::DoManagedICD9Search(this);
					if(!pCode) // If no code was selected, we are not going to add to quick list
						return;

					pCommit->ICD9DiagID = _bstr_t(pCode->nDiagCodesID);
				}

				if (-1 != results.m_ICD10.m_nDiagCodesID)
				{
					pCommit->ICD10DiagID = _bstr_t(AsString(results.m_ICD10.m_nDiagCodesID));
				}
				else if (IsShowingICD9s() && IsShowingICD10s() && -1 != results.m_ICD9.m_nDiagCodesID)
				{
					// (c.haag 2014-03-31) - PLID 60941 - If no 10 exists but we have a 9, open NexCode
					CNexCodeDlg dlg(this);
					if (IDOK == dlg.DoModal())
					{
						long nDiagCodeID_ICD10 = dlg.GetDiagCodeID();
						if (-1 == nDiagCodeID_ICD10)
						{
							// We didn't get an ICD-10, so end now.
							return;
						}
						else
						{
							pCommit->ICD10DiagID = _bstr_t(AsString(nDiagCodeID_ICD10));
						}
					}
					else
					{
						// If user cancels out of NexCode window, do not store ICD-9 code in QuickList.
						return;
					}
				}

				Nx::SafeArray<IUnknown *> saCommits = Nx::SafeArray<IUnknown *>::FromValue(pCommit);
				NexTech_Accessor::_AddDiagCodesToQuickListResultPtr pResult = GetAPI()->AddDiagCodesToQuickList(GetAPISubkey(), GetAPILoginToken(), saCommits);

				// Add the new codes to the list
				AddQuickListItems(pResult, TRUE);

				// The QuickList items should have been added below any existing items in the list. Ensure that the last item we added is visible in the case the user
				// has a large list. NOTE: In my tests, I find that this makes the next-to-last row visible. Whatever the fix is for that problem must be on the datalist side.
				m_QuickList->EnsureRowInView( m_QuickList->GetLastRow() );
			}
			else
			{
				// The user did not select a valid code (e.g. "No code found")
			}
		}
		else
		{
			// No selection made; do nothing
		}
	} NxCatchAll(__FUNCTION__);
}

// (c.haag 2014-02-24) - PLID 60948 - Called when the user checks a box in the shared user list to change
// which other users they're sharing QuickList items with.
void CDiagQuickListDlg::OnEditingFinishedUserList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit)
{
	try
	{
		if (bCommit)
		{
			// Save the change to data
			Nx::SafeArray<BSTR> saIDs;
			for (IRowSettingsPtr pRow = m_UserList->GetFirstRow(); NULL != pRow; pRow = pRow->GetNextRow())
			{
				if (VarBool(pRow->Value[ulcChecked]))
				{
					saIDs.Add(_bstr_t(VarString(pRow->Value[ulcID])));
				}
			}
			GetAPI()->SetDiagQuickListSharedUsers(GetAPISubkey(), GetAPILoginToken(), saIDs);

			// Now reload the QuickList
			Requery(FALSE);
		}
		else
		{
			// Edit was cancelled
		}
	} NxCatchAll(__FUNCTION__);
}

// (c.haag 2014-02-24) - PLID 60947 - Called when the user right-clicks on a row in the QuickList
void CDiagQuickListDlg::OnRButtonDownQuickList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		bool bIncludeResearch = true;
		if (NULL != pRow)
		{
			// Select the row so that the user knows which one we're referring to
			GetDlgItem(IDC_QUICKLIST)->SetFocus();
			m_QuickList->CurSel = pRow;
			// Get the mouse position for tracking the menu
			CPoint pt;
			GetCursorPos(&pt);
			// Create the menu. I'm not using an NxMenu because it disappears by itself at random.
			CMenu mnu;
			mnu.CreatePopupMenu();
			BOOL isSharedRow = IsSharedQuickListRow(pRow);
			if (qlcChecked == nCol)
			{
				// (c.haag 2014-02-24) - PLID 60945 - Unselect all items. Enable this even if nothing is selected.
				mnu.AppendMenu(MF_BYPOSITION|MF_STRING|MF_ENABLED, 2, "&Unselect All");
			}
			else
			{
				mnu.AppendMenu(MF_BYPOSITION|MF_STRING|(isSharedRow ? (MF_GRAYED|MF_DISABLED) : MF_ENABLED), 1, "&Remove Code");
			}

			// (r.farnworth 2014-07-23 15:39) - PLID 63028 - Look through the datalist and see if we have the currently selected row's ICD-9 listed more than once
			// and the other occurence isn't linked to an ICD-10, dont show the Re-search option.
			NXDATALIST2Lib::IRowSettingsPtr tRow = m_QuickList->GetFirstRow();
			while (tRow) {
				if (tRow->GetValue(qlcICD9ID) == pRow->GetValue(qlcICD9ID)
					&& tRow->GetValue(qlcICD10ID) == g_cvarNull && tRow != pRow) {
					bIncludeResearch = false;
					break; //Infinite loops are bad
				}
				tRow = tRow->GetNextRow();
			}

			// (j.armen 2014-03-20 09:14) - PLID 60943 - If we are showing 9's and 10's and they right clicked
			// description column for 9's, they may have the option to change the code
			// (r.farnworth 2014-07-17 15:06) - PLID 62826 - Add right-click functionality for 10s
			// (r.farnworth 2014-07-23 16:19) - PLID 63028 - Allow users to reselect a matching ICD-10 code after having already found a match through the right-click menu.
			if (pRow->Value[qlcICD9ID] == g_cvarNull && IsShowingICD9s())
			{
				mnu.AppendMenu(MF_BYPOSITION|MF_STRING|(isSharedRow || pRow->Value[qlcICD10ID] == g_cvarNull ? (MF_GRAYED|MF_DISABLED) : MF_ENABLED), 3, "&Change ICD-9 Code");
			}
			else if (IsShowingICD10s() && (pRow->Value[qlcICD10ID] == g_cvarNull
				|| (pRow->Value[qlcICD9ID] != g_cvarNull && pRow->Value[qlcICD10ID] != g_cvarNull && bIncludeResearch)))
			{
				mnu.AppendMenu(MF_BYPOSITION | MF_STRING | (isSharedRow || pRow->Value[qlcICD9ID] == g_cvarNull ? (MF_GRAYED | MF_DISABLED) : MF_ENABLED), 4, "&Change ICD-10 Code");
			}

			switch (mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this, NULL))
			{
			case 1: // (c.haag 2014-02-24) - PLID 60947 - Remove code
				{
					CString str = FormatString("Are you sure you wish to remove this item from your QuickList?\r\n\r\n");
					if (IsShowingICD9s() && VT_NULL != pRow->Value[qlcICD9ID].vt)
					{
						str += FormatString("ICD-9: %s - %s\r\n", VarString(pRow->Value[qlcICD9CodeNumber]), VarString(pRow->Value[qlcICD9Description]));
					}
					if (IsShowingICD10s() && VT_NULL != pRow->Value[qlcICD10ID].vt)
					{
						str += FormatString("ICD-10: %s - %s\r\n", VarString(pRow->Value[qlcICD10CodeNumber]), VarString(pRow->Value[qlcICD10Description]));
					}
					if (IDYES == AfxMessageBox(str, MB_YESNO))
					{
						Nx::SafeArray<BSTR> quickListIDsToDelete = Nx::SafeArray<BSTR>::FromValue( _bstr_t(AsString(pRow->Value[qlcID])) );
						GetAPI()->RemoveDiagQuickListItemsAndRelatedBasedOnDisplayType(GetAPISubkey(), GetAPILoginToken(), quickListIDsToDelete, DiagQuickListUtils::GetAPIDiagDisplayType());
						if (HasSharedLists())
						{
							// We have to requery data in case we still have the element in a shared list
							Requery(FALSE);
						}
						else
						{
							// We aren't sharing any lists so we can just remove the row
							m_QuickList->RemoveRow(pRow);
						}
					}
				}
				break;
			case 2: // Unselect all
				// (c.haag 2014-02-24) - PLID 60945 - Unselect all items
				{
					for (pRow = m_QuickList->GetFirstRow(); NULL != pRow; pRow = pRow->GetNextRow())
					{
						// (r.farnworth 2014-08-06 12:06) - PLID 63028 - This was causing the checkboxes to appear for all rows when selecting this option. Let's keep the hidden ones hidden
						if (pRow->Value[qlcChecked] != g_cvarNull)
						{
							pRow->Value[qlcChecked] = g_cvarFalse;
						}
					}
				}
				break;
			case 3:
				Change9Code(pRow);
				break;
			// (r.farnworth 2014-07-17 15:06) - PLID 62826 - Add right-click functionality
			case 4:
				{
					  // (r.farnworth 2014-07-23 16:19) - PLID 63028 - If we are re-picking a 10-code, then we need to clear the existing data out so that we don't create duplicate data.
					  if (pRow->Value[qlcICD10ID] != g_cvarNull) 
					  { 
						  //We need to clear the data in the API
						  Nx::SafeArray<BSTR> quickListIDsToDelete = Nx::SafeArray<BSTR>::FromValue(_bstr_t(pRow->Value[qlcID]));

						  NexTech_Accessor::_QuickListDiagCommitPtr pCommit(__uuidof(NexTech_Accessor::QuickListDiagCommit));
						  pCommit->ICD9DiagID = _bstr_t(pRow->Value[qlcICD9ID]);
						  Nx::SafeArray<IUnknown *> saCommits = Nx::SafeArray<IUnknown *>::FromValue(pCommit);

						  GetAPI()->RemoveDiagQuickListItemsAndRelatedBasedOnDisplayType(GetAPISubkey(), GetAPILoginToken(), quickListIDsToDelete, DiagQuickListUtils::GetAPIDiagDisplayType());
						  NexTech_Accessor::_AddDiagCodesToQuickListResultPtr pResult = GetAPI()->AddDiagCodesToQuickList(GetAPISubkey(), GetAPILoginToken(), saCommits);
						  Nx::SafeArray<IUnknown *> saryNewQuickListItems(pResult->NewItems);
						  NexTech_Accessor::_DiagQuickListItemPtr pItem = saryNewQuickListItems.GetAt(0);
						  pRow->Value[qlcID] = pItem->ID;

						  pRow->PutValue(qlcICD10ID, g_cvarNull);
						  pRow->PutValue(qlcICD10CodeNumber, g_cvarNull);
						  // (r.farnworth 2014-07-24 11:47) - PLID 62826 - Remove the checkbox as long as the code is unmatched
						  pRow->Value[qlcChecked] = g_cvarNull;
					  }
					  Change10Code(pRow);
				}
				break;
			default:
				break;
			}
		}
		else
		{
			// We didn't right-click on a row
		}

	} NxCatchAll(__FUNCTION__);
}

// (j.armen 2014-03-20 09:14) - PLID 60943 - Handle left click on the quick list
void CDiagQuickListDlg::LeftClickQuickList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try
	{
		IRowSettingsPtr pRow(lpRow);

		// (j.armen 2014-03-20 09:14) - PLID 60943 - Show the change code dialog
		if(nCol == qlcICD9Description
			&& CanChange9Code(pRow)
			&& pRow->CellFormatOverride[qlcICD9Description]
			&& pRow->CellFormatOverride[qlcICD9Description]->FieldType == cftTextSingleLineLink)
			Change9Code(pRow);

		// (r.farnworth 2014-07-17 09:36) - PLID 62826 - Address unmatched ICD-10s
		if (nCol == qlcICD10Description
			&& CanChange10Code(pRow)
			&& pRow->CellFormatOverride[qlcICD10Description]
			&& pRow->CellFormatOverride[qlcICD10Description]->FieldType == cftTextSingleLineLink)
			Change10Code(pRow);
	}NxCatchAll(__FUNCTION__);
}

void CDiagQuickListDlg::OnBnClickedBtnCopyFromUser()
{
	// (c.haag 2014-02-24) - PLID 60950 - Initial implementation. From here, users can choose which
	// other users' QuickLists to copy items from.
	try
	{
		if (m_UserList->GetRowCount() > 0)
		{
			CDiagQuickListCopyFromUserDlg dlg;
			if (IDOK == dlg.DoModal())
			{
				Nx::SafeArray<BSTR> saryUserIDs = Nx::SafeArray<BSTR>::From(dlg.m_astrSelectedUserIDs);
				NexTech_Accessor::_AddDiagCodesToQuickListResultPtr pResult = GetAPI()->CopyDiagQuickListsToCurrentUser(GetAPISubkey(), GetAPILoginToken(), saryUserIDs);			
				// Add the new codes to the list
				AddQuickListItems(pResult, FALSE);
				//Requery(False);
			}
		}
		else
		{
			AfxMessageBox("There are no other users to copy QuickList items from.");
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CDiagQuickListDlg::OnBnClickedBtnClearMyQuicklist()
{
	// (c.haag 2014-02-25) - PLID 60936 - Clear your QuickList (but don't touch any one else's)
	try
	{
		if (IDYES == AfxMessageBox("Continuing will clear your entire QuickList of saved codes. Are you sure you want to continue?", MB_YESNO))
		{
			if (IDYES == AfxMessageBox("Do you want to clear your entire QuickList of saved codes?", MB_YESNO))
			{
				// Clear the data
				GetAPI()->ClearCurrentUserDiagQuickList(GetAPISubkey(), GetAPILoginToken());

				// Refresh the list
				if (HasSharedLists())
				{
					// We have to requery data to get the shared QuickList items
					Requery(FALSE);
				}
				else
				{
					// We aren't sharing any lists so we can just clear ours
					m_QuickList->Clear();
				}
			}
			else
			{
				// User changed their mind
			}
		}
		else
		{
			// User changed their mind
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CDiagQuickListDlg::OnBnClickedBtnAddCodesToVisit()
{
	try
	{
		// (c.haag 2014-03-03) - PLID 60928 - Populate m_aSelectedQuickListItems with
		// all of the selected codes and then dismiss the dialog.
		m_aSelectedQuickListItems.RemoveAll();
		for (IRowSettingsPtr pRow = m_QuickList->GetFirstRow(); NULL != pRow; pRow = pRow->GetNextRow())
		{
			if (pRow->Value[qlcChecked] == g_cvarNull ? FALSE : VarBool(pRow->Value[qlcChecked]))
			{
				NexTech_Accessor::_DiagQuickListItemPtr pItem(__uuidof(NexTech_Accessor::DiagQuickListItem));
				pItem->ID = _bstr_t(pRow->Value[qlcID]);
				if (IsShowingICD9s()) // (r.farnworth 2014-07-24 11:47) - PLID 62826 - Only add 9 code information when we're in a mode fit for it.
				{
					NexTech_Accessor::_DiagQuickListCodePtr pCode(__uuidof(NexTech_Accessor::DiagQuickListCode));
					pCode->ID = _bstr_t(pRow->Value[qlcICD9ID]);
					pCode->Code = _bstr_t(pRow->Value[qlcICD9CodeNumber]);
					pCode->description = _bstr_t(pRow->Value[qlcICD9Description]);
					pItem->ICD9 = pCode;
				}
				if (IsShowingICD10s()) // (r.farnworth 2014-07-24 11:47) - PLID 62826 - Only add 10 code information when we're in a mode fit for it.
				{
					NexTech_Accessor::_DiagQuickListCodePtr pCode(__uuidof(NexTech_Accessor::DiagQuickListCode));
					pCode->ID = _bstr_t(pRow->Value[qlcICD10ID]);
					pCode->Code = _bstr_t(pRow->Value[qlcICD10CodeNumber]);
					pCode->description = _bstr_t(pRow->Value[qlcICD10Description]);
					pItem->ICD10 = pCode;
				}
				pItem->IsShared = pRow->Value[qlcIsShared];
				m_aSelectedQuickListItems.Add(pItem);
			}
		}

		// Now return the items
		if (m_aSelectedQuickListItems.GetCount() > 0)
		{
			__super::OnOK();
		}
		else
		{
			// (c.haag 2014-03-05) - PLID 60930 - Adjust the message if necessary
			AfxMessageBox(FormatString("Please select at least one code to add to the %s.", 
				m_bIsEMRTemplate ? "template" : "visit"));
		}
	}
	NxCatchAll(__FUNCTION__);
}

LRESULT CDiagQuickListDlg::OnNcHitTest(CPoint point) 
{
	/* Calculate the new position of the size grip */
	CRect rc;
	GetWindowRect(&rc);
	rc.top = rc.bottom - GetSystemMetrics( SM_CYHSCROLL );
	rc.left = rc.right - GetSystemMetrics( SM_CXVSCROLL ); 

	if (rc.PtInRect(point)) {
		return HTBOTTOMRIGHT;
	}
	
	return __super::OnNcHitTest(point);
}

void CDiagQuickListDlg::OnDestroy()
{
	try
	{
		// Save the column sort orderings in the following format:
		// "column_index;sort_ascending;sort_priority;columN_index;sort_ascending;sort_priority"
		// Where N is the column index.
		CArray<long,long> aSort;
		CString strSort;
		for (int i=0; i < m_QuickList->ColumnCount; i++)
		{
			IColumnSettingsPtr pCol = m_QuickList->GetColumn(i);
			if (m_QuickList->GetColumn(i)->SortPriority > -1)
			{
				aSort.Add( i );
				aSort.Add( (VARIANT_FALSE == m_QuickList->GetColumn(i)->SortAscending) ? 0 : 1);
				aSort.Add( m_QuickList->GetColumn(i)->SortPriority );
			}
		}

		if (aSort.GetCount() > 0)
		{
			strSort = ArrayAsString(aSort);
			SetRemotePropertyText("DiagQuickListDlg_ColumnSort", strSort, 0, GetCurrentUserName());
		}
		else
		{
			SetRemotePropertyText("DiagQuickListDlg_ColumnSort", "", 0, GetCurrentUserName());
		}
		SetRemotePropertyInt("DiagQuickListDlg_LastDiagSearchPreference", (int)DiagSearchUtils::GetPreferenceSearchStyle(), 0, GetCurrentUserName());

		__super::OnDestroy();
	}
	NxCatchAll(__FUNCTION__);
}

// (r.farnworth 2014-07-17 09:20) - PLID 62826 - Check if we change ICD-10 codes
bool CDiagQuickListDlg::CanChange10Code(IRowSettingsPtr pRow)
{
	if (!pRow)
		return false;

	if (!IsShowingICD10s())
		return false;

	if (pRow->Value[qlcICD9ID] == g_cvarNull)
		return false;

	if (IsSharedQuickListRow(pRow))
		return false;

	return true;
}

// (r.farnworth 2014-07-17 09:20) - PLID 62826 - Find the appropriate match type for the corresponding ICD-9 code and allow the user to choose a matching 10
void CDiagQuickListDlg::Change10Code(IRowSettingsPtr pRow)
{
	// Don't allow shared rows to be changed
	if (IsSharedQuickListRow(pRow)) {
		AfxMessageBox("You cannot set the ICD-10 code for a shared QuickList item");
		return;
	}

	Nx::SafeArray<BSTR> saICD9CodeIDs;
	saICD9CodeIDs.Add(AsString(pRow->Value[qlcICD9ID]));
	NexTech_Accessor::_NexGEMMatchResultsPtr codeMatchResults = GetAPI()->GetNexGEMMatchesFromICD9s(GetAPISubkey(), GetAPILoginToken(), saICD9CodeIDs);
	Nx::SafeArray<IUnknown*> codeResults = codeMatchResults->match;

	if (codeResults.GetSize() > 0)
	{
		NexTech_Accessor::_NexGEMMatchResultPtr matchedResult = codeResults.GetAt(0);

		if (matchedResult)
		{
			NexGEMMatchType matchType = MapMatchStatus(matchedResult->matchStatus);

			if (matchType == nexgemtDone) {
				// (r.farnworth 2014-07-18 12:36) - PLID 62966 - Handle exact matches when selecting a matching code for a standalone ICD-9 code in the Diagnosis Quicklist.
				//Check for duplicates
				if (!CheckForDuplicates(pRow->Value[qlcICD9ID], matchedResult->exactMatchedICD10->ID))
				{
					pRow->Value[qlcICD10ID] = atol(matchedResult->exactMatchedICD10->ID);
					pRow->Value[qlcICD10CodeNumber] = matchedResult->exactMatchedICD10->Code;
					pRow->Value[qlcICD10Description] = matchedResult->exactMatchedICD10->description;
				}
				else
				{
					MessageBox("This diagnosis code is already in the list and will be automatically removed.", NULL, MB_OK | MB_ICONEXCLAMATION);
					Nx::SafeArray<BSTR> quickListIDsToDelete = Nx::SafeArray<BSTR>::FromValue(_bstr_t(AsString(pRow->Value[qlcID])));
					GetAPI()->RemoveDiagQuickListItemsAndRelatedBasedOnDisplayType(GetAPISubkey(), GetAPILoginToken(), quickListIDsToDelete, DiagQuickListUtils::GetAPIDiagDisplayType());
					if (HasSharedLists())
					{
						// We have to requery data in case we still have the element in a shared list
						Requery(FALSE);
					}
					else
					{
						// We aren't sharing any lists so we can just remove the row
						m_QuickList->RemoveRow(pRow);
					}
					return;
				}
			}
			else if (matchType == nexgemtManyMatch)
			{
				// (r.farnworth 2014-07-18 12:39) - PLID 62967 - Handle multi-matches when selecting a matching code for a standalone ICD-9 code in the Diagnosis Quicklist
				//Call the API to get the multi-matches
				CString strDiagCodeID;
				strDiagCodeID.Format("%li", AsLong(pRow->GetValue(qlcICD9ID)));
				NexTech_Accessor::_DiagnosisCodeCommitsPtr pCodes = GetAPI()->GetNexGEMMultiMatchesFromICD9(GetAPISubkey(), GetAPILoginToken(), AsBstr(strDiagCodeID));
				pRow->Value[qlcICD10Description] = " <Select from possible matches >";

				//Popup the single-select list
				//Populate input array
				Nx::SafeArray<IUnknown *> saryCodes = pCodes->Codes;
				int index = 0;
				CString strDropdown = "-1;< Select from possible matches >;";
				CString strCodeDrop;
				m_pDiagCodeCommitMultiMatch->aryMultiMatch.RemoveAll();
				foreach(NexTech_Accessor::_DiagnosisCodeCommitPtr matchDiag, saryCodes){
					strCodeDrop.Format("%li;%s;", index++, AsString(matchDiag->Code) + " - " + AsString(matchDiag->description));
					strDropdown += strCodeDrop;
					m_pDiagCodeCommitMultiMatch->aryMultiMatch.Add(matchDiag);
				}

				//Populate dropdown
				NXDATALIST2Lib::IFormatSettingsPtr pfsMultiMatch(__uuidof(NXDATALIST2Lib::FormatSettings));
				pfsMultiMatch->PutDataType(VT_BSTR);
				pfsMultiMatch->PutFieldType(NXDATALIST2Lib::cftComboSimple);
				pfsMultiMatch->PutConnection(_variant_t((LPDISPATCH)NULL));
				pfsMultiMatch->PutEditable(VARIANT_TRUE);
				pfsMultiMatch->PutComboSource(AsBstr(strDropdown));
				pRow->PutRefCellFormatOverride(qlcICD10Description, pfsMultiMatch);
				pRow->PutValue(qlcICD10Description, -1);
				m_QuickList->StartEditing(pRow, qlcICD10Description);

			}
			else if (matchType == nexgemtNoMatch)
			{
				// (r.farnworth 2014-07-18 12:53) - PLID 62968 - Handle spawning NexCode when there is no match for a standalone ICD-9 code in the Diagnosis Quicklist
				//spawn NexCode
				CNexCodeDlg dlg(this);
				if (IDOK == dlg.DoModal())
				{
					long nDiagCodeID_ICD10 = dlg.GetDiagCodeID();
					if (-1 == nDiagCodeID_ICD10)
					{
						// We didn't get an ICD-10, so end now.
						return;
					}
					else
					{
						//Check for duplicates
						if (!CheckForDuplicates(pRow->Value[qlcICD9ID], _bstr_t(AsString(nDiagCodeID_ICD10))))
						{
							pRow->Value[qlcICD10ID] = _bstr_t(AsString(nDiagCodeID_ICD10));
							pRow->Value[qlcICD10CodeNumber] = _bstr_t(dlg.GetDiagCode());
							pRow->Value[qlcICD10Description] = _bstr_t(dlg.GetDiagCodeDescription());
						}
						else
						{
							MessageBox("This diagnosis code has already been selected.", NULL, MB_OK | MB_ICONEXCLAMATION);
							return;
						}
					}
				}
				else
				{
					// If user cancels out of NexCode window, do not store ICD-9 code in QuickList.
					pRow->PutValue(qlcICD10Description, AsBstr("< No Mapping - Open NexCode >"));
					SetHyperLinkFormat(pRow, qlcICD10Description, false);
					pRow->PutCellBackColor(qlcICD10Description, GetDiagnosisNexGEMMatchColor(-1, nexgemtNoMatch));
					return;
				}
			}

			// (r.farnworth 2014-07-17 11:15) - PLID 62966 - We have our code so let's update the view and commit it
			if (matchType == nexgemtNoMatch || matchType == nexgemtDone)
			{
				Nx::SafeArray<BSTR> quickListIDsToDelete = Nx::SafeArray<BSTR>::FromValue(_bstr_t(pRow->Value[qlcID]));

				NexTech_Accessor::_QuickListDiagCommitPtr pCommit(__uuidof(NexTech_Accessor::QuickListDiagCommit));
				pCommit->ICD9DiagID = _bstr_t(pRow->Value[qlcICD9ID]);
				pCommit->ICD10DiagID = _bstr_t(pRow->Value[qlcICD10ID]);
				Nx::SafeArray<IUnknown *> saCommits = Nx::SafeArray<IUnknown *>::FromValue(pCommit);

				GetAPI()->RemoveDiagQuickListItemsAndRelatedBasedOnDisplayType(GetAPISubkey(), GetAPILoginToken(), quickListIDsToDelete, DiagQuickListUtils::GetAPIDiagDisplayType());
				// (r.farnworth 2014-07-23 16:19) - PLID 63028 - We need to rewrite the ID value of the row now that we have new data there
				NexTech_Accessor::_AddDiagCodesToQuickListResultPtr pResult = GetAPI()->AddDiagCodesToQuickList(GetAPISubkey(), GetAPILoginToken(), saCommits);
				Nx::SafeArray<IUnknown *> saryNewQuickListItems(pResult->NewItems);
				NexTech_Accessor::_DiagQuickListItemPtr pItem = saryNewQuickListItems.GetAt(0);
				pRow->Value[qlcID] = pItem->ID;

				// (r.farnworth 2014-07-16 15:41) - PLID 62826 - Remove Hyperlinking.
				SetHyperLinkFormat(pRow, qlcICD10Description, true);
				pRow->PutCellBackColor(qlcICD10Description, GetDiagnosisNexGEMMatchColor(-1, nexgemtDone));
				// (r.farnworth 2014-07-24 11:47) - PLID 62826 - Add the checkbox again, unchecked
				pRow->Value[qlcChecked] = g_cvarFalse;
				
			}
		}
	}
	else
	{
		MessageBox("No matches could be found for this code.", NULL);
	}

}

// (r.farnworth 2014-07-18 12:39) - PLID 62967 - Handle multi-matches when selecting a matching code for a standalone ICD-9 code in the Diagnosis Quicklist
void CDiagQuickListDlg::EditingFinishedDiags(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try{

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow == NULL){
			return;
		}


		if (bCommit == FALSE){
			if (nCol == qlcICD10Description){
				pRow->PutValue(qlcICD10Description, AsBstr("< Select from possible matches >"));
				SetHyperLinkFormat(pRow, qlcICD10Description, false);
				pRow->PutCellBackColor(qlcICD10Description, GetDiagnosisNexGEMMatchColor(-1, nexgemtManyMatch));
			}
			return;
		}
		switch (nCol){
			case qlcICD10Description:
			{
				if (AsLong(varOldValue) == AsLong(varNewValue)){
					return;
				}

				NexTech_Accessor::_DiagnosisCodeCommitPtr selectedDiagCode = m_pDiagCodeCommitMultiMatch->aryMultiMatch.GetAt(AsLong(varNewValue));

				//Save
				// Make sure the selected ICD10 is already imported into PracData
				CString strDiagCodeID_ICD10 = AsString((LPCTSTR)selectedDiagCode->ID);
				VARIANT_BOOL vbICD10 = selectedDiagCode->ICD10;
				if (strDiagCodeID_ICD10.IsEmpty() && vbICD10 != VARIANT_FALSE){
					//Need to Import
					Nx::SafeArray<IUnknown *> saryCodeToCreate;
					saryCodeToCreate.Add(selectedDiagCode);

					NexTech_Accessor::_DiagnosisCodesPtr pImportedDiagCode =
						GetAPI()->CreateDiagnosisCodes(
						GetAPISubkey(),
						GetAPILoginToken(),
						saryCodeToCreate
						);

					if (pImportedDiagCode == NULL){
						MessageBox("Practice was unable to save this diagnosis code.", "Practice", MB_OK | MB_ICONERROR);
						return;
					}

					Nx::SafeArray<IUnknown *> saryImportedCodes = pImportedDiagCode->Codes;
					if (saryImportedCodes.GetSize() != 1){
						MessageBox("Unexpected ICD-10 import value.", "Practice", MB_OK | MB_ICONERROR);
						return;
					}

					NexTech_Accessor::_DiagnosisCodePtr pImportedICD10 = saryImportedCodes.GetAt(0);
					selectedDiagCode->ID = pImportedICD10->ID;

				}
				else{
					//// Make sure it's not already in the list
					if (CheckForDuplicates(pRow->Value[qlcICD9ID], selectedDiagCode->ID)) {
						MessageBox("This diagnosis code has already been selected.", NULL, MB_OK | MB_ICONEXCLAMATION);
						pRow->PutValue(qlcICD10Description, AsBstr("< Select from possible matches >"));
						SetHyperLinkFormat(pRow, qlcICD10Description, false);
						pRow->PutCellBackColor(qlcICD10Description, GetDiagnosisNexGEMMatchColor(-1, nexgemtManyMatch));
						return;
					}
				}

				//it was not a duplicate
				// Update the display
				pRow->Value[qlcICD10ID] = selectedDiagCode->ID;
				pRow->Value[qlcICD10CodeNumber] = selectedDiagCode->Code;
				pRow->Value[qlcICD10Description] = selectedDiagCode->description;

				Nx::SafeArray<BSTR> quickListIDsToDelete = Nx::SafeArray<BSTR>::FromValue(_bstr_t(pRow->Value[qlcID]));

				NexTech_Accessor::_QuickListDiagCommitPtr pCommit(__uuidof(NexTech_Accessor::QuickListDiagCommit));
				pCommit->ICD9DiagID = _bstr_t(pRow->Value[qlcICD9ID]);
				pCommit->ICD10DiagID = _bstr_t(pRow->Value[qlcICD10ID]);
				Nx::SafeArray<IUnknown *> saCommits = Nx::SafeArray<IUnknown *>::FromValue(pCommit);

				GetAPI()->RemoveDiagQuickListItemsAndRelatedBasedOnDisplayType(GetAPISubkey(), GetAPILoginToken(), quickListIDsToDelete, DiagQuickListUtils::GetAPIDiagDisplayType());
				// (r.farnworth 2014-07-23 16:19) - PLID 63028 - We need to rewrite the ID value of the row now that we have new data there
				NexTech_Accessor::_AddDiagCodesToQuickListResultPtr pResult = GetAPI()->AddDiagCodesToQuickList(GetAPISubkey(), GetAPILoginToken(), saCommits);
				Nx::SafeArray<IUnknown *> saryNewQuickListItems(pResult->NewItems);
				NexTech_Accessor::_DiagQuickListItemPtr pItem = saryNewQuickListItems.GetAt(0);
				pRow->Value[qlcID] = pItem->ID;

				SetHyperLinkFormat(pRow, qlcICD10Description, true);
				pRow->PutCellBackColor(qlcICD10Description, GetDiagnosisNexGEMMatchColor(-1, nexgemtDone));
				// (r.farnworth 2014-07-24 11:47) - PLID 62826 - Add the checkbox again, unchecked
				pRow->Value[qlcChecked] = g_cvarFalse;
			}
				break;
		}

	}NxCatchAll(__FUNCTION__);
}

// (r.farnworth 2014-07-17 15:52) - PLID 62826 - Moved into a function to be used elsewhere
BOOL CDiagQuickListDlg::CheckForDuplicates(_variant_t vNewICD9ID, _variant_t vNewICD10ID)
{
	BOOL bMatch = FALSE;

	if (!IsShowingICD10s()) { vNewICD10ID = g_cvarNull; }
	if (!IsShowingICD9s()) { vNewICD9ID = g_cvarNull; }

	for (IRowSettingsPtr pRow = m_QuickList->GetFirstRow(); NULL != pRow && !bMatch; pRow = pRow->GetNextRow())
	{
		bMatch = (VARCMP_EQ == VariantCompare(&pRow->Value[qlcICD9ID], &vNewICD9ID)
			&& VARCMP_EQ == VariantCompare(&pRow->Value[qlcICD10ID], &vNewICD10ID));

		// Did we find a matching row?
		if (bMatch)
		{
			// Yes we did! But...is it shared?
			if (IsSharedQuickListRow(pRow))
			{
				// Yes, it's shared. That must mean it's not in our list because all rows must be unique.
				// So, remove the shared row, and break immediately to create its replacement.
				m_QuickList->RemoveRow(pRow);
				bMatch = FALSE;
				break;
			}
			else
			{
				// No it's not shared; therefore it is our row. Our search is finished; leave bMatch a value
				// of TRUE so that we don't add the item to our list as a new row.
			}
		}
	}
	return bMatch;
}

// (r.farnworth 2014-07-18 12:39) - PLID 62967 - Handle hyper-linking
void CDiagQuickListDlg::SetHyperLinkFormat(IRowSettingsPtr pRow, enum QuickListColumns linkField, bool bRemove)
{
	if (bRemove)
	{
		NXDATALIST2Lib::IFormatSettingsPtr pNoHyperLink(__uuidof(NXDATALIST2Lib::FormatSettings));
		pNoHyperLink->PutFieldType(NXDATALIST2Lib::cftTextSingleLine);
		pRow->PutCellLinkStyle(linkField, dlLinkStyleFalse);
		pRow->PutRefCellFormatOverride(linkField, pNoHyperLink);
	}
	else
	{
		NXDATALIST2Lib::IFormatSettingsPtr pHyperLink(__uuidof(NXDATALIST2Lib::FormatSettings));
		pHyperLink->PutFieldType(NXDATALIST2Lib::cftTextSingleLineLink);
		pRow->PutCellLinkStyle(linkField, dlLinkStyleTrue);
		pRow->PutRefCellFormatOverride(linkField, pHyperLink);
	}

}