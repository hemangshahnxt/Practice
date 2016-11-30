// ReferralSubDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "practicerc.h"
#include "ReferralSubDlg.h"
#include "MarketUtils.h"
#include "NxMessageDef.h"
#include "GlobalUtils.h"

//
//NxMessageDef messages that are posted by this sub dialog to its parent:
//
//NXM_REFERRAL_DOUBLECLICK		//The user has double clicked a row in the tree.
//NXM_REFERRAL_ONOK				//The user has pressed enter to initiate an "OnOK()" message
//NXM_REFERRAL_ONCANCEL			//The user has pressed escape to initiate an "OnCancel()" message
//

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALIST2Lib;

//Maximum allowed depth for the referral tree.  This is a longstanding "feature",
//	mostly used to make reports compatible with the tree levels.
#define MAX_REFERRAL_LEVELS		4

// (a.wilson 2012-5-8) PLID 14874 - added inactive variable.
// (r.goldschmidt 2014-08-27 17:36) - PLID 31191 - added count of active children
struct ReferralObject {
	long nID;
	CString strName;
	long nParent;
	BOOL bInactive;
	long nActiveChildrenCount;
};
// (a.wilson 2012-5-8) PLID 14874 - added inactive column.
// (r.goldschmidt 2014-08-27 17:36) - PLID 31191 - added count of active children
enum EReferralColumn {
	ercID = 0,
	ercName,
	ercParent,
	ercInactive,
	ercActiveChildrenCount
};

void UpdateDefaultReferralPreferences(long nOldID, long nNewID)
{
	// (z.manning, 06/15/2007) - PLID 20279 - We can't just update ConfigRT manually because that 
	// does not update the cache.
	_RecordsetPtr prs = CreateRecordset(
		"SELECT Name FROM ConfigRT "
		"WHERE Name IN ('DefaultPatientReferral', 'DefaultPhysicianReferral') AND IntParam = %li"
		, nOldID);
	for(; !prs->eof; prs->MoveNext()) {
		SetRemotePropertyInt(AdoFldString(prs,"Name"), nNewID, 0, "<None>");
	}
}


/////////////////////////////////////////////////////////////////////////////
// CReferralSubDlg dialog


CReferralSubDlg::CReferralSubDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CReferralSubDlg::IDD, pParent),
	m_tcReferralChecker(NetUtils::ReferralSourceT)
{
	//{{AFX_DATA_INIT(CReferralSubDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_lpDraggingRow = NULL;
}


void CReferralSubDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CReferralSubDlg)
	DDX_Control(pDX, IDC_ENABLE_DRAG, m_btnEnableDrag);
	DDX_Control(pDX, IDC_BTN_ADDTOPLEVELREFERRAL, m_btnAddNewTopLevelReferral);
	DDX_Control(pDX, IDC_INACTIVE_REFERRALS, m_btnShowInactive);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CReferralSubDlg, CNxDialog)
	//{{AFX_MSG_MAP(CReferralSubDlg)
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_SUBDIALOGPOPUP_ADD, OnAdd)
	ON_COMMAND(IDC_BTN_ADDTOPLEVELREFERRAL, OnAddTopLevel)
	ON_COMMAND(ID_SUBDIALOGPOPUP_RENAME, OnRename)
	ON_COMMAND(ID_SUBDIALOGPOPUP_DELETE, OnDelete)
	ON_COMMAND(ID_SUBDIALOGPOPUP_UNSELECT, OnUnselect)
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_SUBDIALOGPOPUP_INACTIVATE, OnInactivate)
	ON_COMMAND(ID_SUBDIALOGPOPUP_REACTIVATE, OnReactivate)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_INACTIVE_REFERRALS, &CReferralSubDlg::OnBnClickedInactiveReferrals)
	ON_BN_CLICKED(IDC_ENABLE_DRAG, &CReferralSubDlg::OnBnClickedEnableDrag)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CReferralSubDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CReferralSubDlg)
	ON_EVENT(CReferralSubDlg, IDC_TREE_LIST, 6 /* RButtonDown */, OnRButtonDownTreeList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CReferralSubDlg, IDC_TREE_LIST, 12 /* DragBegin */, OnDragBeginTreeList, VTS_PBOOL VTS_DISPATCH VTS_I2 VTS_I4)
	ON_EVENT(CReferralSubDlg, IDC_TREE_LIST, 13 /* DragOverCell */, OnDragOverCellTreeList, VTS_PBOOL VTS_DISPATCH VTS_I2 VTS_DISPATCH VTS_I2 VTS_I4)
	ON_EVENT(CReferralSubDlg, IDC_TREE_LIST, 14 /* DragEnd */, OnDragEndTreeList, VTS_DISPATCH VTS_I2 VTS_DISPATCH VTS_I2 VTS_I4)
	ON_EVENT(CReferralSubDlg, IDC_TREE_LIST, 3 /* DblClickCell */, OnDblClickCellTreeList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CReferralSubDlg, IDC_TREE_LIST, 2 /* SelChanged */, OnSelChangedTreeList, VTS_DISPATCH VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReferralSubDlg message handlers

BOOL CReferralSubDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		// (a.wilson 2012-5-9) PLID 14874 - default to not show inactive referrals.
		m_btnShowInactive.SetCheck(0);

		//Load Datalist2 tree control
		m_pList = BindNxDataList2Ctrl(this, IDC_TREE_LIST, GetRemoteData(), false);

		// (c.haag 2009-08-03 11:00) - PLID 23269 - Set up the new icon button
		m_btnAddNewTopLevelReferral.AutoSet(NXB_NEW);

		// (a.wilson 2012-5-23) PLID 50602 - handling new permissions
		if (!(GetCurrentUserPermissions(bioReferralTree) & (sptCreate | sptCreateWithPass))) {
			m_btnAddNewTopLevelReferral.EnableWindow(FALSE);
		}
		if (!(GetCurrentUserPermissions(bioReferralTree) & (sptWrite | sptWriteWithPass))) {
			m_btnEnableDrag.EnableWindow(FALSE);
		}

		ReloadTree();

	} NxCatchAll("Error in OnInitDialog()");


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


// (r.goldschmidt 2014-08-27 17:36) - PLID 31191 - added column to datalist for count of active children
void CReferralSubDlg::ReloadTree()
{
	m_pList->Clear();

	//we do not have a way to load as a tree, so we'll have to do it manually
	//For speed, we will load these all into an array
	CArray<ReferralObject, ReferralObject> aryReferrals, aryInactiveReferrals;
	// (a.wilson 2012-5-8) PLID 14874 - add inactive to the query and code.
	//Generate results based on check box.
	// (a.wilson 2012-5-16) PLID 50422 - handle the chance of bad data where a child is active but has an inactive parent.
	// (r.goldschmidt 2014-08-27 17:36) - PLID 31191 - added count of active children
	_RecordsetPtr prs = CreateParamRecordset(R"(
SELECT ReferralSourceT.PersonID, ReferralSourceT.Name, ReferralSourceT.Parent, PersonT.Archived AS Inactive,
	COALESCE(ChildCounter.ChildrenCount, 0) AS ActiveChildrenCount
FROM ReferralSourceT
INNER JOIN PersonT ON ReferralSourceT.PersonID = PersonT.ID
LEFT JOIN (
	SELECT Parent, COUNT(Parent) AS ChildrenCount
	FROM ReferralSourceT
	INNER JOIN PersonT ON PersonT.ID = ReferralSourceT.PersonID
	WHERE PersonT.Archived = 0
	GROUP BY Parent
	) AS ChildCounter ON ChildCounter.Parent = ReferralSourceT.PersonID
ORDER BY Parent, PersonID ASC
		)");
	
	while(!prs->eof) {
		ReferralObject ro;
		ro.nID = AdoFldLong(prs, "PersonID");
		ro.nParent = AdoFldLong(prs, "Parent");
		ro.strName = AdoFldString(prs, "Name", "");
		ro.bInactive = AdoFldBool(prs, "Inactive");
		ro.nActiveChildrenCount = AdoFldLong(prs, "ActiveChildrenCount", 0);
		
		//add the referralobject to the correct array depending if the show inactive checkbox is checked.
		if (ro.bInactive && m_btnShowInactive.GetCheck() == BST_UNCHECKED)	
			aryInactiveReferrals.Add(ro);
		else				
			aryReferrals.Add(ro);

		prs->MoveNext();
	}
	prs->Close();

	//Now that we have an array, loop through it and load
	BOOL bAtLeastOne = TRUE;
	while(aryReferrals.GetSize() > 0 && bAtLeastOne) {
		//flag to track if we found anything
		bAtLeastOne = FALSE;

		//Loop through all the referrals
		for(int i = 0; i < aryReferrals.GetSize(); i++) {
			ReferralObject ro = aryReferrals.GetAt(i);

			if(ro.nParent == -1) {
				//Root level
				IRowSettingsPtr pRow = m_pList->GetNewRow();
				pRow->PutValue(ercID, (long)ro.nID);
				pRow->PutValue(ercName, _bstr_t(ro.strName));
				pRow->PutValue(ercParent, (long)ro.nParent);
				pRow->PutValue(ercActiveChildrenCount, (long)ro.nActiveChildrenCount);
				// (a.wilson 2012-5-8) PLID 14874 - added inactive to the datalist values
				pRow->PutValue(ercInactive, (ro.bInactive ? g_cvarTrue : g_cvarFalse));
				pRow->PutForeColor((ro.bInactive ? RGB(143, 148, 152) : RGB(0, 0, 0)));
				pRow->PutForeColorSel((ro.bInactive ? RGB(143, 148, 152) : RGB(0, 0, 0)));
				m_pList->AddRowSorted(pRow, NULL);
				bAtLeastOne = TRUE;

				//remove from array
				aryReferrals.RemoveAt(i);
				i--;	//move i backward to compensate for the removal
			}
			else {
				//Child row, need to find the parent.  It may not exist yet.
				IRowSettingsPtr pParentRow = m_pList->FindByColumn(ercID, (long)ro.nParent, 0, VARIANT_FALSE);

				if(pParentRow != NULL) {
					//The parent is loaded, hooray.  Let us load our child.
					IRowSettingsPtr pRow = m_pList->GetNewRow();
					pRow->PutValue(ercID, (long)ro.nID);
					pRow->PutValue(ercName, _bstr_t(ro.strName));
					pRow->PutValue(ercParent, (long)ro.nParent);
					pRow->PutValue(ercActiveChildrenCount, (long)ro.nActiveChildrenCount);
					// (a.wilson 2012-5-8) PLID 14874 - added inactive to the datalist values
					pRow->PutValue(ercInactive, (ro.bInactive ? g_cvarTrue : g_cvarFalse));
					pRow->PutForeColor((ro.bInactive ? RGB(143, 148, 152) : RGB(0, 0, 0)));
					pRow->PutForeColorSel((ro.bInactive ? RGB(143, 148, 152) : RGB(0, 0, 0)));
					m_pList->AddRowSorted(pRow, pParentRow);
					bAtLeastOne = TRUE;

					//remove from array
					aryReferrals.RemoveAt(i);
					i--;	//move i backward to compensate for the removal
				//(a.wilson 2012-5-16) PLID 50422 - if for some reason we have a null parent then check if its inactive.
				} else if (m_btnShowInactive.GetCheck() == BST_UNCHECKED) {
					bool bDone = false;
					for (int j = 0; j < aryInactiveReferrals.GetSize() && bDone == false; j++) {
						//check each record in the inactive referral array to see if there is a match.
						if (aryInactiveReferrals.GetAt(j).nID == ro.nParent) {
							//if the parent is inactive, add the child because it is also considered inactive
							//and also might have children of its own.
							bAtLeastOne = TRUE;
							bDone = true;
							aryInactiveReferrals.Add(ro);
							aryReferrals.RemoveAt(i);
							i--;
						}
					}
				}
			}
		}
	}
	if(aryReferrals.GetSize() > 0) {
		//Somehow we still have children without parents
		AfxMessageBox("Your data has children referrals with no parents.  Please contact NexTech to correct this.");
	}
}

//Reloads the list of referrals if it has changed.  The caller is responsible for remembering any current selection.
void CReferralSubDlg::Update(bool bIgnoreDragPermission /* = false */)
{
	if(m_tcReferralChecker.Changed()) {
		//We need to reload the list.  Note that we have no obligation to keeping
		//	any selections, the caller must handle that.
		ReloadTree();
	}

	// (a.wilson 2012-5-23) PLID 50602 - handling new permissions
	if (!bIgnoreDragPermission) {
		if (!(GetCurrentUserPermissions(bioReferralTree) & (sptCreate | sptCreateWithPass))) {
			m_btnAddNewTopLevelReferral.EnableWindow(FALSE);
		} else {
			m_btnAddNewTopLevelReferral.EnableWindow(TRUE);
		}
		if (!(GetCurrentUserPermissions(bioReferralTree) & (sptWrite | sptWriteWithPass))) {
			m_btnEnableDrag.EnableWindow(FALSE);
		} else {
			m_btnEnableDrag.EnableWindow(TRUE);
		}
	}

}
// (a.wilson 2012-5-9) PLID 14874 - check if the current row is inactive and return -1 if so.
//Returns the ID of the currently selected referral source, -1 if no selection.
long CReferralSubDlg::GetSelectedReferralID(bool bReturnInactive /* = false */)
{
	IRowSettingsPtr pRow = m_pList->GetCurSel();

	if(pRow != NULL) {
		if (!(VarBool(pRow->GetValue(ercInactive))) || bReturnInactive)
			return VarLong(pRow->GetValue(ercID));
	}
		
	return -1;
}

//Returns the string name of the current selected referral source, "" if no selection
CString CReferralSubDlg::GetSelectedReferralName(bool bReturnInactive /* = false */)
{
	IRowSettingsPtr pRow = m_pList->GetCurSel();

	if(pRow != NULL) {
		if (!(VarBool(pRow->GetValue(ercInactive))) || bReturnInactive)
			return VarString(pRow->GetValue(ercName), "");
	}
		
	return "";

}

// (r.goldschmidt 2014-09-22 18:18) - PLID 31191 - Preference setting can make certain selections illegal. Check if selection is restricted, if so, also get warning message.
long CReferralSubDlg::ReferralRestrictedByPreference(long nReferralID, CString& strWarning)
{
	long nPreferenceSetting = GetRemotePropertyInt("AllowParentReferrals", 0, 0, "<None>", true);
	long nRestrictionLevel = nPreferenceSetting;
	strWarning = "Referral source selection error."; // just in case

	// only check if there are restrictions
	if (nPreferenceSetting != 0){
		
		IRowSettingsPtr pSelection = m_pList->FindByColumn(ercID, nReferralID, NULL, VARIANT_FALSE);

		if (pSelection){
			bool bIsTopLevel = VarLong(pSelection->GetValue(ercParent)) == -1;
			bool bHasActiveChildren = VarLong(pSelection->GetValue(ercActiveChildrenCount)) > 0;
			bool bIsInactive = !!VarBool(pSelection->GetValue(ercInactive));

			// allow a selection if it is inactive or has no active children
			if (bIsInactive || !bHasActiveChildren){
				nRestrictionLevel = 0;
			}

			// if we disallow selecting top level referrals that have active children
			if (nPreferenceSetting == 1){
				if (!bIsTopLevel){
					nRestrictionLevel = 0;
				}
			}
			// all other cases are restricted
			// these cases need to notify of warning
			switch (nRestrictionLevel){
			case 0: // selection is allowed
				break;
			case 1: // illegal selection according to AllowParentReferrals preference (1)
				strWarning = "Current referral source preferences do not allow selecting a top level parent that has active children.";
				break;
			case 2: // illegal selection according to AllowParentReferrals preference (2)
				strWarning = "Current referral source preferences do not allow selecting a parent that has active children.";
				break;
			default:
				ASSERT(FALSE); // unknown preference
			}
		}
	}
	return nRestrictionLevel;
}

void CReferralSubDlg::OnOK() 
{
	try {
		//For now, we don't do anything with the message, just notify our parent
		CWnd* pWnd = GetParent();
		if(pWnd)
			pWnd->PostMessage(NXM_REFERRAL_ONOK, 0, 0);
	} NxCatchAll("Error in OnOK()");
}

void CReferralSubDlg::OnCancel() 
{
	try {
		//For now, we don't do anything with the message, just notify our parent
		CWnd* pWnd = GetParent();
		if(pWnd && pWnd->GetSafeHwnd() && IsWindow(pWnd->GetSafeHwnd()))
			pWnd->PostMessage(NXM_REFERRAL_ONCANCEL, 0, 0);
	} NxCatchAll("Error in OnCancel()");
}

// (c.haag 2009-08-03 10:56) - PLID 23269 - Vertical spaces
#define BTN_SPACING_VERT		10
#define LIST_SPACING_VERT		8

void CReferralSubDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);

	try {
		CRect rcArea, rcDrag, rcBtn, rcList, rcInactive;
		GetClientRect(&rcArea);

		CWnd* pWnd = NULL;

		//Enable drag... button
		pWnd = GetDlgItem(IDC_ENABLE_DRAG);
		if(pWnd != NULL && pWnd->GetSafeHwnd() && IsWindow(pWnd->GetSafeHwnd())) {
			//Move this window to be at the bottom left
			pWnd->GetClientRect(&rcDrag);

			long nCurHeight = rcDrag.Height();

			rcDrag.bottom = rcArea.bottom;
			rcDrag.top = rcDrag.bottom - nCurHeight;

			pWnd->MoveWindow(rcDrag);
		}

		// (a.wilson 2012-5-9) PLID 14874 - Add new show inactive referral source button.
		pWnd = GetDlgItem(IDC_INACTIVE_REFERRALS);
		if(pWnd != NULL && pWnd->GetSafeHwnd() && IsWindow(pWnd->GetSafeHwnd())) {
			//Move this window to be at the bottom left
			pWnd->GetClientRect(&rcInactive);

			long nCurHeight = rcInactive.Height();

			rcInactive.bottom = rcDrag.top - BTN_SPACING_VERT;
			rcInactive.top = rcInactive.bottom - nCurHeight;

			pWnd->MoveWindow(rcInactive);
		}

		// (c.haag 2009-08-03 10:48) - PLID 23269 - Add new top level referral button
		pWnd = GetDlgItem(IDC_BTN_ADDTOPLEVELREFERRAL);
		if(pWnd != NULL && pWnd->GetSafeHwnd() && IsWindow(pWnd->GetSafeHwnd())) {
			//Position this from the top left down to the drag button
			pWnd->GetClientRect(&rcBtn);

			long nCurHeight = rcBtn.Height();

			rcBtn.bottom = rcInactive.top - BTN_SPACING_VERT;
			rcBtn.top = rcBtn.bottom - nCurHeight;
			
			rcBtn.left = rcArea.left;
			rcBtn.right = rcArea.right;

			pWnd->MoveWindow(rcBtn);
		}

		//tree datalist
		pWnd = GetDlgItem(IDC_TREE_LIST);
		if(pWnd != NULL && pWnd->GetSafeHwnd() && IsWindow(pWnd->GetSafeHwnd())) {
			//Position this from the top left down to the drag button
			pWnd->GetClientRect(&rcList);

			rcList.top = rcArea.top;
			rcList.bottom = rcBtn.top - LIST_SPACING_VERT;
			
			// (b.cardillo 2006-07-07 15:37) - PLID 20306 - And only as far as the right side of the
			// sub-dialog.  Prior to this, the datalist2's horizontal size was never being adjusted, 
			// so when the dialog was made to be more narrow than the datalist2's original width,  
			// the right side of the datalist2 was being cut off.
			rcList.left = rcArea.left;
			rcList.right = rcArea.right;

			pWnd->MoveWindow(rcList);
		}

		// (z.manning, 08/04/2006) - PLID 21775 - We no longer use the NxColor object for the background
		// color because its borders caused some drawing quirks. See OnCtlColor and m_brushBackground instead.
		//background NxColor
		/*pWnd = GetDlgItem(IDC_SUBDLG_BACKGROUND);
		if(pWnd != NULL && pWnd->GetSafeHwnd() && IsWindow(pWnd->GetSafeHwnd())) {
			//position this to the size of the entire subdlg.  We bump it up by a sizable amount so that the borders
			//	of the NxColor (the rounded corners) do not draw gray artifacts over whatever is around us.
			CRect rcBack = rcArea;
			rcBack.right += 25;
			rcBack.bottom += 25;
			pWnd->MoveWindow(rcBack);
		}*/

	} NxCatchAll("Error in OnSize()");
}

void CReferralSubDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	try {
		//If the list is disabled, no popup menu
		if(!m_pList->Enabled)
			return;

		// (z.manning, 08/04/2006) - PLID 21793 - We don't want this menu if we're outside the datalist.
		CWnd* pwndTree = GetDlgItem(IDC_TREE_LIST);
		if(pwndTree) {
			CRect rcTree;
			pwndTree->GetWindowRect(&rcTree);
			if(!rcTree.PtInRect(point)) {
				return;
			}
		}

		// (a.wilson 2012-5-23) PLID 50602 - handle added permissions of the referral tree (create, delete).
		CPermissions permRefTree = GetCurrentUserPermissions(bioReferralTree);

		CMenu mnu;
		mnu.LoadMenu(IDR_REFERRAL_MENU);
		CMenu* pSubMenu = mnu.GetSubMenu(0);
		if(pSubMenu) {
			IRowSettingsPtr pRow = m_pList->GetCurSel();
			// (a.wilson 2012-5-8) PLID 14874 - add inactivate and reactivate menu status.
			if(pRow == NULL) {
				//If no row, we don't want rename, delete, or unselect to be available options
				pSubMenu->EnableMenuItem(ID_SUBDIALOGPOPUP_RENAME, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
				pSubMenu->EnableMenuItem(ID_SUBDIALOGPOPUP_DELETE, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
				pSubMenu->EnableMenuItem(ID_SUBDIALOGPOPUP_UNSELECT, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
				pSubMenu->EnableMenuItem(ID_SUBDIALOGPOPUP_INACTIVATE, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
				pSubMenu->RemoveMenu(ID_SUBDIALOGPOPUP_REACTIVATE, MF_BYCOMMAND);

				//clarify the description of add to show that it's a root level node
				pSubMenu->ModifyMenu(ID_SUBDIALOGPOPUP_ADD, ((permRefTree & (sptCreate | sptCreateWithPass)) ? MF_BYCOMMAND : MF_BYCOMMAND|MF_DISABLED|MF_GRAYED), ID_SUBDIALOGPOPUP_ADD, "Add New Top Level Referral");
			} else if (pRow) {
				if (VarBool(pRow->GetValue(ercInactive))) {
					pSubMenu->RemoveMenu(ID_SUBDIALOGPOPUP_INACTIVATE, MF_BYCOMMAND);
					pSubMenu->EnableMenuItem(ID_SUBDIALOGPOPUP_RENAME, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
					pSubMenu->EnableMenuItem(ID_SUBDIALOGPOPUP_ADD, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);

					if ((pRow->GetParentRow() != NULL && VarBool(pRow->GetParentRow()->GetValue(ercInactive))) || 
						(!(permRefTree & (sptDynamic0 | sptDynamic0WithPass))))
						pSubMenu->EnableMenuItem(ID_SUBDIALOGPOPUP_REACTIVATE, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
					if (!(permRefTree & (sptDelete | sptDeleteWithPass))) {
						pSubMenu->EnableMenuItem(ID_SUBDIALOGPOPUP_DELETE, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
					}
				} else {
					pSubMenu->RemoveMenu(ID_SUBDIALOGPOPUP_REACTIVATE, MF_BYCOMMAND);
					
					if (!(permRefTree & (sptCreate | sptCreateWithPass))) {
						pSubMenu->EnableMenuItem(ID_SUBDIALOGPOPUP_ADD, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
					}
					if (!(permRefTree & (sptDelete | sptDeleteWithPass))) {
						pSubMenu->EnableMenuItem(ID_SUBDIALOGPOPUP_DELETE, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
					}
					if (!(permRefTree & (sptDynamic0 | sptDynamic0WithPass))) {
						pSubMenu->EnableMenuItem(ID_SUBDIALOGPOPUP_INACTIVATE, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
					}
					if (!(permRefTree & (sptWrite | sptWriteWithPass))) {
						pSubMenu->EnableMenuItem(ID_SUBDIALOGPOPUP_RENAME, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
					}
				}
			}
			pSubMenu->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, point.x, point.y, this, NULL);
		}

	} NxCatchAll("Error in OnContextMenu()");	
}

void CReferralSubDlg::OnRButtonDownTreeList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		//DRT 7/18/2006 - This was causing the list to refresh if you right clicked on the row
		//	already selected, thus making it impossible to get the popup menu to happen.
		IRowSettingsPtr pCurrentSel = m_pList->CurSel;
		IRowSettingsPtr pRow(lpRow);

		//quit if they didn't change
		if(pCurrentSel == pRow)
			return;

		//set the selection to whatever they r-clicked on
		m_pList->CurSel = pRow;
		OnSelChangedTreeList(NULL, lpRow);	//fake a sel changed

	} NxCatchAll("Error in OnRButtonDownTreeList()");
}

//just call OnAdd like the user used the right click menu.  This is legacy functionality from the old
//	pre-datalist tree.
void CReferralSubDlg::ExternalAddNew()
{
	OnAdd();
}
// (a.wilson 2012-6-26) PLID 50378 - added inactive check.
BOOL CReferralSubDlg::SelectReferralID(long nReferralID, BOOL bInactive)
{
	//Select the value they've passed in.  If the list is still loading, this function will wait until the given
	//	value is loaded.
	if (m_btnShowInactive.GetCheck() == BST_UNCHECKED && bInactive)
	{
		m_btnShowInactive.SetCheck(BST_CHECKED);
		ReloadTree();
	}
	IRowSettingsPtr pRow = m_pList->SetSelByColumn(ercID, (long)nReferralID);
	if(pRow == NULL)
		return FALSE;

	return TRUE;
}

void CReferralSubDlg::OnAdd()
{
	try
	{
		AddNewReferralSource(FALSE);

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2010-04-27 09:49) - PLID 38331 - Added new event handler for the top level referral button
void CReferralSubDlg::OnAddTopLevel()
{
	try
	{
		AddNewReferralSource(TRUE);

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2010-04-27 09:48) - PLID 38331
void CReferralSubDlg::AddNewReferralSource(BOOL bTopLevel)
{
	try {
		//Ensure they are allowed to make changes
		// (a.wilson 2012-5-23) PLID 50602 - update to new create permission.
		CPermissions permRefTree = GetCurrentUserPermissions(bioReferralTree);

		if (!(permRefTree & (sptCreate)) && (permRefTree & (sptCreateWithPass))) {
			if (!CheckCurrentUserPassword()) {
				return;
			}
		}

		//The parent is always the currently selected item -- we're adding a child.  If it is NULL, then we
		//	are adding a top level node.
		IRowSettingsPtr pParentRow;
		if(bTopLevel) {
			// (z.manning 2010-04-27 09:55) - PLID 38331 - If we're adding a top level referral then
			// force the parent row to null regardless of what is currently selected.
			pParentRow = NULL;
		}
		else {
			pParentRow = m_pList->GetCurSel();
		}


		//Count our depth, cannot exceed the max
		if(CountDepthOfRowFromTop(pParentRow) >= MAX_REFERRAL_LEVELS) {
			CString str;
			str.Format("Your referral tree may only be %li levels deep.", MAX_REFERRAL_LEVELS);
			AfxMessageBox(str);
			return;
		}
			

		CString strName = "";
		while(strName.IsEmpty()) {
			if(InputBoxLimited(this, "Enter the new referral name", strName, "",50,false,false,NULL) == IDCANCEL)
				return;
			strName.TrimRight();
		}
	
		//Check to see if the name is duplicated.  Checking data just in case something were added elsewhere and we were
		//	not updated.
		if(ReturnsRecords("SELECT PersonID FROM ReferralSourceT WHERE Name = '%s'", _Q(strName))) {
			AfxMessageBox("The name '" + strName + "' already exists, you may not duplicate referral names.");
			return;
		}

		//find the parent
		long nParentID = -1;		//-1 signifies no parent in data
		if(pParentRow != NULL) {
			nParentID = VarLong(pParentRow->GetValue(ercID));
			// (r.goldschmidt 2014-08-27 17:36) - PLID 31191 - increment active children count for parent
			long nParentChildrenCount = VarLong(pParentRow->GetValue(ercActiveChildrenCount)) + 1;
			pParentRow->PutValue(ercActiveChildrenCount, nParentChildrenCount);
		}
		//Now actually insert the new record
		long nID = -1;
		BEGIN_TRANS("ReferralAddNew") {
			nID = NewNumber("PersonT", "ID");
			ExecuteSql("INSERT INTO PersonT (ID) VALUES (%li)",	nID);
			ExecuteSql("INSERT INTO ReferralSourceT (PersonID, Name, Parent) "
				"VALUES (%li, '%s', %li);",
				nID, _Q(strName), nParentID);
		} END_TRANS("ReferralAddNew");

		//Send a refresh message
		CTableChecker *p[2];
		p[0] = &m_tcReferralChecker;
		p[1] = NULL;
		ReferralMultiRefresh(p, nID);
		// (a.wilson 2012-5-10) PLID 14874 - add inactive default value to row.
		// (r.goldschmidt 2014-08-27 17:36) - PLID 31191 - added count of active children to row
		//Add the record to the tree
		IRowSettingsPtr pRow = m_pList->GetNewRow();
		pRow->PutValue(ercID, (long)nID);
		pRow->PutValue(ercName, _bstr_t(strName));
		pRow->PutValue(ercParent, (long)nParentID);
		pRow->PutValue(ercInactive, g_cvarFalse);
		pRow->PutValue(ercActiveChildrenCount, (long)0);
		m_pList->AddRowSorted(pRow, pParentRow);

		//now select our new row.  The datalist2 will automatically expand if need be.
		m_pList->CurSel = pRow;

	} NxCatchAll(__FUNCTION__);
}

// (j.gruber 2006-11-14 15:19) - PLID 23535 - Let them merge duplicates,
//this function just figures out if it is a duplicate
BOOL CReferralSubDlg::IsDuplicate(const CString &name, long ID)
{
	_RecordsetPtr rs;

	rs = CreateRecordset("SELECT TOP 1 PersonID "
			"FROM ReferralSourceT "
			"WHERE Name = '%s' AND PersonID <> %li", 
			_Q(name),ID);

	if (rs->eof) {
		return FALSE;
	}
	else {
		return TRUE;
	}

}
// (j.gruber 2006-11-14 15:19) - PLID 23535 - Let them merge duplicates,
//this function was mainly taken from OnEndRename in the old ReferralTree.cpp, 
//but changed a little to use the new way we do referral sources
BOOL CReferralSubDlg::HandleDuplicates(CString strNewName, CString strOldName, long nID) {

	if (IsDuplicate(strNewName,nID))
	{	
		// allow them to combine sources
		if(AfxMessageBox ("This name already exists, would you like to combine these into the existing source and delete this source?", MB_YESNO) == IDNO) {
			MessageBox("You may not have multiple referral sources with the same name.");
			return FALSE;
		}
		// (a.wilson 2012-6-7) PLID 50602 - handle permissions on deletion of a referral during merge.
		CPermissions permRefTree = GetCurrentUserPermissions(bioReferralTree);
		if (!(permRefTree & (sptDelete))) {
			if (!(permRefTree & (sptDeleteWithPass))) {
				MessageBox("You do not have permission to merge referral sources.", "", MB_OK);
				return FALSE;
			} else {
				if (!CheckCurrentUserPassword()) {
					return FALSE;
				}
			}
		}

		try {
			//find out the ID's for the old source, and the new source names
			_RecordsetPtr rs = CreateRecordset("SELECT PersonID FROM ReferralSourceT WHERE Name = '%s'", _Q(strOldName));
			long nOldID = -1;
			if (! rs->eof) {
				nOldID= AdoFldLong(rs, "PersonID");
			}
			else {
				//we should be able to find it by the old name since it hasn't changed yet
				ASSERT(FALSE);
				return FALSE;
			}
			rs->Close();
			// (a.wilson 2012-6-7) PLID 50422 - check if the target is inactive, if so then stop them.
			rs = CreateRecordset("SELECT PersonID, Archived FROM ReferralSourceT "
				"INNER JOIN PersonT ON PersonT.ID = ReferralSourceT.PersonID "
				"WHERE Name = '%s'", _Q(strNewName));
			long nNewID = -1;
			if (! rs->eof) {
				// (a.walling 2010-10-27 15:57) - PLID 41067 - This was doing VarLong of the .lval member of a variant!
				nNewID = AdoFldLong(rs, "PersonID"); 

				if (AdoFldBool(rs, "Archived") == TRUE) {
					MessageBox("You may not combine with inactive referral sources.");
					return FALSE;
				}
			}
			else {
				ASSERT(FALSE);
				return FALSE;
			}
			rs->Close();

			//if they have costs we can't delete it
			rs = CreateRecordset("SELECT ReferralSource FROM MarketingCostsT WHERE ReferralSource = %li", nOldID);
			if(!rs->eof){
				if(AfxMessageBox("This referral source has costs associated with it, would you like to transfer those to the new source as well?  If not, the transfer will not happen.", MB_YESNO) == IDNO) {
					return FALSE;
				}
				else {
					ExecuteSql("UPDATE MarketingCostsT SET ReferralSource = %li WHERE ReferralSource = %li", nNewID, nOldID);
				}
			}
			rs->Close();
			//

			//check to see if the old one is a parent, if so, move all children to the new one
			ExecuteSql("UPDATE ReferralSourceT SET Parent = %li WHERE Parent = %li", nNewID, nOldID);

			//now move everyone who has oldname (what this label was), to newName (what this label was changed to)
			ExecuteSql("UPDATE MultiReferralsT SET ReferralID = %li WHERE ReferralID = %li", nNewID, nOldID);
			ExecuteSql("UPDATE PatientsT SET ReferralID = %li WHERE ReferralID = %li", nNewID, nOldID);

			//(e.lally 2011-05-03) PLID 43481 - Remove NexWeb display setup
			ExecuteParamSql("DELETE FROM NexWebDisplayT WHERE ReferralSourceID = {INT}", nOldID);

			//now delete this source that we transferred
			ExecuteSql("DELETE FROM ReferralSourceT WHERE PersonID = %li", nOldID);
			ExecuteSql("DELETE FROM PersonT WHERE ID = %li", nOldID);

			// (z.manning, 06/15/2007) - PLID 20279 - Also, make sure this value does not exist as one of the
			// preferences to select this referral source when selecting a referring phys or patient.
			UpdateDefaultReferralPreferences(nOldID, nNewID);

			//reload the tree
			ReloadTree();

			//now select the item that was selected before
			m_pList->SetSelByColumn(0, nNewID);

			//fire a table checker
			CTableChecker *p[2];
			p[0] = &m_tcReferralChecker;
			p[1] = NULL;
			ReferralMultiRefresh(p, nID);

			//in case our parent needs to know we changed, tell them
			CWnd* pWnd = GetParent();
			if(pWnd) {
				pWnd->PostMessage(NXM_REFERRAL_ONSELCHANGED, 0, 0);
			}


		} NxCatchAll("Error in CReferralSubDlg::HandleDuplicates()");

		//done
		CString str;
		str.Format("Successfully transferred all referrals from %s to %s.", strOldName, strNewName);
		AfxMessageBox(str);

		

		return FALSE;
	}
	else {
		return TRUE;
	}
}

void CReferralSubDlg::OnRename()
{
	try {
		//must have a row
		IRowSettingsPtr pRow = m_pList->GetCurSel();
		if(pRow == NULL)
			return;

		//current ID
		long nID = VarLong(pRow->GetValue(ercID));

		//DRT 8/6/2007 - PLID 26958 - Warn users before renaming.
		{
			_RecordsetPtr prsCount = CreateParamRecordset("SELECT COUNT(*) AS Cnt FROM MultiReferralsT WHERE ReferralID = {INT};"
				"SELECT COUNT(*) AS Cnt FROM MarketingCostsT WHERE ReferralSource = {INT};", nID, nID);
			long nPatients = AdoFldLong(prsCount, "Cnt", 0);
			prsCount = prsCount->NextRecordset(NULL);
			long nCosts = AdoFldLong(prsCount, "Cnt", 0);
			prsCount->Close();

			if(nPatients != 0 || nCosts != 0) {
				CString strMsg;
				strMsg.Format("Renaming this referral source will affect the displayed name on: \r\n"
					" - %li patient(s)\r\n"
					" - %li marketing cost(s)\r\n"
					"Are you sure you wish to rename this referral source?", nPatients, nCosts);
				if(AfxMessageBox(strMsg, MB_YESNO) != IDYES) {
					//Do not rename
					return;
				}
			}
		}

		//Check permissions
		// (a.wilson 2012-5-23) PLID 50602 - update to use new permissions.
		CPermissions permRefTree = GetCurrentUserPermissions(bioReferralTree);

		if (!(permRefTree & (sptWrite)) && (permRefTree & (sptWriteWithPass))) {
			if (!CheckCurrentUserPassword()) {
				return;
			}
		}

		//prompt for new name
		CString strName = VarString(pRow->GetValue(ercName), "");
		// (j.gruber 2006-11-14 12:35) - PLID 23535 - save the old name
		CString strOldName = strName;
		do {
			if(InputBoxLimited(this, "Enter the new referral name", strName, "",50,false,false,NULL) == IDCANCEL)
				return;
			strName.TrimRight();
		} while(strName.IsEmpty());

		//check for duplicates, ignoring our current row
		// (j.gruber 2006-11-14 12:25) - PLID 23535 - Allow them to merge 2 duplicate names together
		if (!HandleDuplicates(strName, strOldName, nID)) {
			return;
		}
		
		//at this point we're set, so update the data
		ExecuteSql("UPDATE ReferralSourceT SET Name = '%s' WHERE PersonID = %li", _Q(strName), nID);

		//fire a table checker
		CTableChecker *p[2];
		p[0] = &m_tcReferralChecker;
		p[1] = NULL;
		ReferralMultiRefresh(p, nID);

		//update the in-place data
		pRow->PutValue(ercName, _bstr_t(strName));

		//Notify our parent that the rename happened
		CWnd* pWnd = GetParent();
		if(pWnd)
			pWnd->PostMessage(NXM_REFERRAL_ONSELCHANGED, 0, 0);


	} NxCatchAll("Error in OnRename()");
}

void CReferralSubDlg::OnDelete()
{
	try {
		//Make sure we have a selection
		IRowSettingsPtr pRow = m_pList->GetCurSel();
		if(pRow == NULL)
			return;

		//We do not allow the user to delete parent nodes, all children must be moved or deleted first.
		if(pRow->GetFirstChildRow() != NULL) {
			AfxMessageBox("You may not delete a referral which has child sources.  Please move or delete these other sources first.");
			return;
		}

		long nID = VarLong(pRow->GetValue(ercID));

		//Check for data that we cannot allow to be deleted.  This is just copied from the old referrals.
		if (ExistsInTable("MarketingCostsT", "ReferralSource = %li", nID)) {
			MessageBox("There are existing costs associated with this referral source.  It cannot be deleted.");
			return;
		}
		if (ExistsInTable("ReferralSourceT", "Parent = %li", nID)) {
			MessageBox("This referral source has children. Please delete the children of this node before deleting the parent.");
			return;
		}
		if (ExistsInTable("MultiReferralsT", "ReferralID = %li", nID)) {
			MessageBox("This referral source is designated as a referral source for at least one patient or inquiry.  You must remove all patient and inquiry links before deleting.");
			return;
		}

		if(AfxMessageBox("Are you sure you want to delete this referral source?", MB_YESNO) != IDYES)
			return;

		//Check permissions
		// (a.wilson 2012-5-23) PLID 50602 - update to use new permissions.
		CPermissions permRefTree = GetCurrentUserPermissions(bioReferralTree);

		if (!(permRefTree & (sptDelete)) && (permRefTree & (sptDeleteWithPass))) {
			if (!CheckCurrentUserPassword()) {
				return;
			}
		}

		BEGIN_TRANS("DeleteReferral") {
			//Referral sources can also be in LW Groups!
			ExecuteSql("DELETE FROM GroupDetailsT WHERE PersonID = %li", nID);

			//DRT 4/17/2006 - PLID 19837 - The last check above ensures that we do *not* delete from MultiReferralsT... yet we have a line here?
			//	I'm changing so we will not ever delete from MultiReferralsT (which should not have happened before).
			//ExecuteSql("DELETE FROM MultiReferralsT WHERE ReferralID = %li", id);

			//(e.lally 2011-05-03) PLID 43481 - Remove NexWeb display setup
			ExecuteParamSql("DELETE FROM NexWebDisplayT WHERE ReferralSourceID = {INT}", nID);

			ExecuteSql("DELETE FROM ReferralSourceT WHERE PersonID = %li", nID);
			ExecuteSql("DELETE FROM PersonT WHERE ID = %i", nID);
			
		} END_TRANS("DeleteReferral");

		// (z.manning, 06/15/2007) - PLID 20279 - Also, make sure this value does not exist as one of the
		// preferences to select this referral source when selecting a referring phys or patient.
		UpdateDefaultReferralPreferences(nID, -1);

		//Refresh through tablecheckers
		CTableChecker *p[2];
		p[0] = &m_tcReferralChecker;
		p[1] = NULL;
		ReferralMultiRefresh(p, nID);

		// (r.goldschmidt 2014-08-28 16:42) - PLID 31191 - decrement child count of parent row
		IRowSettingsPtr pParentRow = pRow->GetParentRow();
		if (pParentRow){
			long nChildCount = VarLong(pParentRow->GetValue(ercActiveChildrenCount)) - 1;
			pParentRow->PutValue(ercActiveChildrenCount, nChildCount);
		}

		//remove the row
		m_pList->RemoveRow(pRow);

	} NxCatchAll("Error in OnDelete()");
}

void CReferralSubDlg::OnUnselect()
{
	try {
		m_pList->CurSel = NULL;
	} NxCatchAll("Error in OnUnselect()");
}

//////////////////////////////////////////////////////////
//														//
//			Code for drag/drop moving of referrals		//
//														//
//////////////////////////////////////////////////////////

#define IDT_DRAG_HOVER	1000
#define HOVER_DELAY_MS	1000		//1 second of hover = create placeholder
#define REFERRAL_PLACEHOLDER_ID	-2	//ID of the placeholder rows.  Not a valid referral ID.

void CReferralSubDlg::OnDragBeginTreeList(BOOL FAR* pbShowDrag, LPDISPATCH lpRow, short nCol, long nFlags) 
{
	try {
		IRowSettingsPtr pRow(lpRow);

		//If the user has not enabled drag/drop functionality, then cancel it out
		// (a.wilson 2012-5-21) PLID 14874 - do not allow the drag of an inactive referral.
		if(!IsDlgButtonChecked(IDC_ENABLE_DRAG) || VarBool(pRow->GetValue(ercInactive))) {
			*pbShowDrag = FALSE;
		}

		//Save this row for later use
		m_lpDraggingRow = lpRow;

	} NxCatchAll("Error in OnDragBeginTreeList()");
}

void CReferralSubDlg::OnDragOverCellTreeList(BOOL FAR* pbShowDrop, LPDISPATCH lpRow, short nCol, LPDISPATCH lpFromRow, short nFromCol, long nFlags) 
{
	try {
		//Clear the hover timer
		KillTimer(IDT_DRAG_HOVER);

		IRowSettingsPtr pDestRow(lpRow);
		IRowSettingsPtr pFromRow(lpFromRow);

		// (a.wilson 2012-5-21) PLID 14874 - handle inactives and prevent dragging.
		if(!IsDlgButtonChecked(IDC_ENABLE_DRAG) || (pDestRow && VarBool(pDestRow->GetValue(ercInactive))) || (pFromRow && VarBool(pFromRow->GetValue(ercInactive)))) {
			*pbShowDrop = FALSE;
			return;
		}

		ClearDragPlaceholders(pDestRow);
		CString strReasonFailed;
		*pbShowDrop = IsValidDrag(pFromRow, pDestRow, strReasonFailed);

		if(*pbShowDrop) {
			//If the row we are dragging to has children, make sure it is expanded
			if(pDestRow->GetFirstChildRow() != NULL)
				pDestRow->Expanded = VARIANT_TRUE;
			else {
				//This is a leaf node, perhaps they want to make it a parent node, so we'll start the timer for a placeholder.
				SetTimer(IDT_DRAG_HOVER, HOVER_DELAY_MS, NULL);
			}
		}

	} NxCatchAll("Error in OnDragOverCellTreeList()");
}

void CReferralSubDlg::OnDragEndTreeList(LPDISPATCH lpRow, short nCol, LPDISPATCH lpFromRow, short nFromCol, long nFlags) 
{
	try {
		long nID = -1;

		//end the hover timer, if it exists
		KillTimer(IDT_DRAG_HOVER);

		//require the enable to be checked
		if(IsDlgButtonChecked(IDC_ENABLE_DRAG)) {

			IRowSettingsPtr pDestRow(lpRow);
			IRowSettingsPtr pFromRow(lpFromRow);

			//Do not allow a drag onto no row (NULL)
			// (a.wilson 2012-5-21) PLID 14874 - do not allow a drag from or to an inative referral.
			if(pDestRow && pFromRow && !VarBool(pDestRow->GetValue(ercInactive)) && !VarBool(pFromRow->GetValue(ercInactive))) {
				//Ensure this is valid to drag
				CString strReasonFailed;
				if(IsValidDrag(pFromRow, pDestRow, strReasonFailed)) {
					//See what our new parent will be
					IRowSettingsPtr pDestParent = pDestRow->GetParentRow();
					IRowSettingsPtr pFromParent = pFromRow->GetParentRow(); // (r.goldschmidt 2014-09-29 16:30) - PLID 31191

					nID = VarLong(pFromRow->GetValue(ercID), -1);

					long nParentID = -1;
					if(pDestParent)
						nParentID = VarLong(pDestParent->GetValue(ercID));

					//DRT - NOTE:  We must ADD first, then DELETE second.  If you delete first, you will remove all the children from this
					//	row, and adding will then just bring over the root row.

					//Add the new row
					IRowSettingsPtr pNewRow = m_pList->AddRowSorted(pFromRow, pDestParent);

					//Remove the old row
					m_pList->RemoveRow(pFromRow);

					//Update the parent
					pNewRow->PutValue(ercParent, (long)nParentID);

					// (r.goldschmidt 2014-08-28 17:37) - PLID 31191 - update child counts for former and new parents
					if (pFromParent){ // former parent needs child count decremented
						long nChildCount = VarLong(pFromParent->GetValue(ercActiveChildrenCount)) - 1;
						pFromParent->PutValue(ercActiveChildrenCount, nChildCount);
					}
					if (pDestParent){ // new parent needs child count incremented
						long nChildCount = VarLong(pDestParent->GetValue(ercActiveChildrenCount)) + 1;
						pDestParent->PutValue(ercActiveChildrenCount, nChildCount);
					}

					//Now update the database
					ExecuteSql("UPDATE ReferralSourceT SET Parent = %li WHERE PersonID = %li", nParentID, VarLong(pNewRow->GetValue(ercID)));
				}
				else {
					//Notify the user why it failed
					AfxMessageBox(strReasonFailed);
				}

				//Dragging is over, get rid of our placeholders
				ClearDragPlaceholders();
			}
		}


		//We need to fire the selection changing.  In most cases the selection will have remained
		//	the same (if you are moving a row down in hierarchy.  However, if you are moving it to 
		//	be a sibling, you will "drop" on the sibling, leaving the selection on said sibling.
		//We will only do this if the cursel is different than the dragging row
		IRowSettingsPtr pCurSel = m_pList->CurSel;

		//clear the saved row for dragging
		m_lpDraggingRow = NULL;

		if (pCurSel != m_lpDraggingRow) {
			//The current selection has moved to a different row
			OnSelChangedTreeList(m_lpDraggingRow, pCurSel);
		}

		//Force a refresh of the table checker
		m_tcReferralChecker.Refresh(nID);
	
	} NxCatchAll("Error in OnDragEndTreeList()");
}

void CReferralSubDlg::OnTimer(UINT nIDEvent) 
{
	try {
		switch(nIDEvent) {
		case IDT_DRAG_HOVER:
			KillTimer(IDT_DRAG_HOVER);
			ASSERT(m_lpDraggingRow);
			if(m_lpDraggingRow) {
				//Only insert a placeholder if this row has no children.  If it has children, the
				//	user should drag it as a sibling.  Also ensure that we aren't hovering on a placeholder
				IRowSettingsPtr pRow = m_pList->GetCurSel();
				if(pRow->GetFirstChildRow() == NULL && VarLong(pRow->GetValue(ercID)) != REFERRAL_PLACEHOLDER_ID) {
					IRowSettingsPtr pPH = InsertPlaceholder(pRow);
					m_aryDragPlaceholders.Add(pPH);
					pRow->Expanded = VARIANT_TRUE;
				}
			}
			break;
		}

	} NxCatchAll("Error in OnTimer()");

	CDialog::OnTimer(nIDEvent);
}

void CReferralSubDlg::ClearDragPlaceholders(NXDATALIST2Lib::IRowSettingsPtr pRowToPreserve /*= NULL*/)
{
	for(int i = m_aryDragPlaceholders.GetSize() - 1; i >= 0; i--) {
		IRowSettingsPtr pRow = m_aryDragPlaceholders.GetAt(i);
		if(pRow != pRowToPreserve) {
			m_pList->RemoveRow(pRow);
			m_aryDragPlaceholders.RemoveAt(i);
		}
	}
}

BOOL CReferralSubDlg::IsValidDrag(NXDATALIST2Lib::IRowSettingsPtr pFromRow, NXDATALIST2Lib::IRowSettingsPtr pDestRow, CString &strReasonForFailure)
{
	if(pFromRow == NULL || pDestRow == NULL) {
		strReasonForFailure = "Invalid parameters.";
		return FALSE;
	}

	//Must be enabled
	if(!IsDlgButtonChecked(IDC_ENABLE_DRAG)) {
		strReasonForFailure = "You must enable drag/drop functionality by checking the box at the bottom of the referral list.";
		return FALSE;
	}

	//Currently, the only invalid conditions are:
	//	a)  Dragging a parent to its child
	{
		//pTopmostFromRow will the highest parent of the "from"
		IRowSettingsPtr pTopmostFromRow = pFromRow;
		while(pTopmostFromRow->GetParentRow())
			pTopmostFromRow = pTopmostFromRow->GetParentRow();

		//pTopmostToRow will be the highest parent of the "to"
		IRowSettingsPtr pTopmostDestRow = pDestRow;
		while(pTopmostDestRow->GetParentRow()) {
			//If the row in the destination ever matches the row we're copying from, then we are trying to
			//	drag down in the tree, which is not allowed
			if(pTopmostDestRow == pFromRow) {
				strReasonForFailure = "You may not move a referral source in such a way that it would become a child of itself.";
				return FALSE;
			}
			pTopmostDestRow = pTopmostDestRow->GetParentRow();
		}

		//We finally need to check at the very highest level, because the top-level rows have no parents (but not if both are NULL)
		if(pTopmostDestRow == pFromRow) {
			strReasonForFailure = "You may not move a referral source into a child of itself.";
			return FALSE;
		}
	}


	//	b)  Dragging a row such that the addition of the row + its children will exceed MAX_REFERRAL_LEVELS
	{
		//This is the depth BELOW, not including our current row.
		long nCurrentDepth = CountDepthBelowRow(pFromRow);

		//If we are dropping onto a placeholder, that placeholder will be counted in the below function.
		//now for the destination, we need to now how deep it is, in reference to the top of the tree.
		long nDestDepth = CountDepthOfRowFromTop(pDestRow);

		//Add 'em up.  If we go over the max, then this isn't allowed.
		long nFinalDepth = nCurrentDepth + nDestDepth;
		if(nFinalDepth > MAX_REFERRAL_LEVELS) {
			strReasonForFailure.Format("Your referral tree may only be %li levels deep.", MAX_REFERRAL_LEVELS);
			return FALSE;
		}
	}

	return TRUE;
}

NXDATALIST2Lib::IRowSettingsPtr CReferralSubDlg::InsertPlaceholder(NXDATALIST2Lib::IRowSettingsPtr pParentRow)
{
	IRowSettingsPtr pRow = m_pList->GetNewRow();
	pRow->PutValue(ercID, (long)REFERRAL_PLACEHOLDER_ID);
	pRow->PutValue(ercName, _bstr_t(""));
	pRow->PutValue(ercParent, (long)-1);
	pRow->PutValue(ercInactive, g_cvarFalse);	//(a.wilson 2012-5-21) PLID 14874 - adding inactive column to dragging.
	m_pList->AddRowSorted(pRow, pParentRow);

	return pRow;
}

//////////////////////////////////////////////////////////
//														//
//		End code for drag/drop moving of referrals		//
//														//
//////////////////////////////////////////////////////////

void CReferralSubDlg::OnDblClickCellTreeList(LPDISPATCH lpRow, short nColIndex) 
{
	try {
		if(lpRow == NULL)
			return;

		//For now, we don't do anything with the double click, just notify our parent
		CWnd* pWnd = GetParent();
		if(pWnd)
			pWnd->PostMessage(NXM_REFERRAL_DOUBLECLICK, 0, 0);

	} NxCatchAll("Error in OnDblClickCellTreeList()");
}

void CReferralSubDlg::OnSelChangedTreeList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try {
		//DRT 7/18/2006 - If we are dragging, do not fire the event for sel changed
		if(m_lpDraggingRow != NULL)
			return;

		//For now, we don't do anything with the double click, just notify our parent
		CWnd* pWnd = GetParent();
		if(pWnd)
			pWnd->PostMessage(NXM_REFERRAL_ONSELCHANGED, 0, 0);

	} NxCatchAll("Error in OnSelChangedTreeList()");
}

//enable or disable all controls in the sub dialog
void CReferralSubDlg::EnableAll(BOOL bEnable)
{
	//datalist
	if(bEnable)
		m_pList->Enabled = VARIANT_TRUE;
	else
		m_pList->Enabled = VARIANT_FALSE;

	//radio button
	// (a.wilson 2012-5-30) PLID 50602 - check permissions for write access.
	if (!(GetCurrentUserPermissions(bioReferralTree) & (sptWrite | sptWriteWithPass)) || !bEnable) {
		GetDlgItem(IDC_ENABLE_DRAG)->EnableWindow(FALSE);
	} else if (bEnable) {
		GetDlgItem(IDC_ENABLE_DRAG)->EnableWindow(TRUE);
	}
}

//There is a hidden NxColor control kept on the sub dialog.  This function will allow a caller to display that 
//	nxcolor with a given color, thus making a background of the entire subdialog, which can blend in with
//	its surroundings.
void CReferralSubDlg::UseBackgroundColor(COLORREF dwColor)
{
	//Set the brush we use for the background color.
	m_brushBackground.CreateSolidBrush(dwColor);
}

//Opposite of the above function -- allows a user to turn off the coloring if needed.  By default, the
//	coloring is not enabled.
void CReferralSubDlg::HideBackground()
{
	//Reset the background color.
	m_brushBackground.CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
}

BOOL CReferralSubDlg::OnEraseBkgnd(CDC* pDC) 
{
	// (a.walling 2008-04-03 14:21) - PLID 29497 - Do not erase background; the parent will handle this for us.
	return TRUE;
}

// (a.wilson 2012-5-8) PLID 14874 - catch inactivate on context menu.
void CReferralSubDlg::OnInactivate()
{
	try {
		IRowSettingsPtr pRow = m_pList->GetCurSel();

		if (pRow) {

			CString strChild;
			if (pRow->GetFirstChildRow() != NULL) {
				strChild = ("Doing so will also inactivate all child referral sources as well.");
			}
			if (IDNO == MsgBox(MB_YESNO, "Are you sure you want to inactivate referral source '%s'?\r\n"
				"%s", VarString(pRow->GetValue(ercName)), strChild)) {
				return;
			}

			//Check permissions
			// (a.wilson 2012-5-23) PLID 50602 - update to use new permissions.
			CPermissions permRefTree = GetCurrentUserPermissions(bioReferralTree);

			if (!(permRefTree & (sptDynamic0)) && (permRefTree & (sptDynamic0WithPass))) {
				if (!CheckCurrentUserPassword()) {
					return;
				}
			}
			//update records in sql. Added some flavor to this using CTE and OUTPUT
			//thanks to a.walling for the help of this code.
			_RecordsetPtr prs = CreateParamRecordset(
				"BEGIN TRANSACTION \r\n"
					"SET NOCOUNT ON \r\n"
					"DECLARE @IDT Table (ID INT NOT NULL); \r\n"
					"WITH ReferralTree (PersonID, Name, Parent) AS ( \r\n"
						"SELECT ref.PersonID, ref.Name, ref.Parent \r\n"
						"FROM ReferralSourceT AS ref \r\n"
						"WHERE ref.PersonID = {INT} \r\n"
						"UNION ALL \r\n"
						"SELECT ref.PersonID, ref.Name, ref.Parent \r\n"
						"FROM ReferralSourceT AS ref \r\n"
						"INNER JOIN ReferralTree AS parentRef ON ref.Parent = parentRef.PersonID \r\n"
					") \r\n"
					"UPDATE PersonT SET Archived = 1 OUTPUT INSERTED.ID INTO @IDT FROM PersonT \r\n"
					"INNER JOIN ReferralSourceT ON PersonT.ID = ReferralSourceT.PersonID \r\n"
					"INNER JOIN ReferralTree ON PersonT.ID = ReferralTree.PersonID \r\n"
					"WHERE Archived = 0 \r\n"
					"OPTION (MAXRECURSION 32) \r\n"
					"\r\n"
					"SET NOCOUNT OFF \r\n"
					//(a.wilson 2012-5-15) PLID 50378 - handle preferences for default referral.
					"SELECT Name FROM ConfigRT WHERE Name IN ('DefaultPatientReferral', 'DefaultPhysicianReferral') \r\n"
					"AND IntParam IN (SELECT ID FROM @IDT) \r\n"
					"\r\n"
					"SELECT * FROM @IDT \r\n"
				"COMMIT TRANSACTION ", VarLong(pRow->GetValue(ercID)));

			//(a.wilson 2012-5-15) PLID 50378 - have to do it this way so that the cache is updated.
			for (; !prs->eof; prs->MoveNext()) {
				SetRemotePropertyInt(AdoFldString(prs,"Name"), -1, 0, "<None>");
			}
			prs = prs->NextRecordset(NULL);

			bool bShowInactive = (m_btnShowInactive.GetCheck() == BST_CHECKED);

			for (; !prs->eof; prs->MoveNext()) {
				IRowSettingsPtr pReferralRow = m_pList->FindByColumn(ercID, AdoFldLong(prs, "ID"), NULL, VARIANT_FALSE);

				if (pReferralRow) {
					// (r.goldschmidt 2014-08-28 16:42) - PLID 31191 - decrement child count of parent row
					IRowSettingsPtr pParentRow = pRow->GetParentRow();
					if (pParentRow && !VarBool(pParentRow->GetValue(ercInactive))){
						long nChildCount = VarLong(pParentRow->GetValue(ercActiveChildrenCount)) - 1;
						pParentRow->PutValue(ercActiveChildrenCount, nChildCount);
					}
					if (bShowInactive) {
						pReferralRow->PutValue(ercActiveChildrenCount, (long)0);
						pReferralRow->PutValue(ercInactive, g_cvarTrue);
						pReferralRow->PutForeColor(RGB(143, 148, 152));
						pReferralRow->PutForeColorSel(RGB(143, 148, 152));
					} else {
						m_pList->RemoveRow(pReferralRow);
					}
				}
			}
			prs->Close();

			// (a.wilson 2012-5-21) PLID 53078 - Refresh through tablecheckers
			CTableChecker *p[2];
			p[0] = &m_tcReferralChecker;
			p[1] = NULL;
			ReferralMultiRefresh(p, -1);
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2012-5-8) PLID 14874 - catch reactivate on context menu.
void CReferralSubDlg::OnReactivate()
{
	try {
		IRowSettingsPtr pRow = m_pList->GetCurSel();

		if (pRow) {

			long nResult;
			if (pRow->GetFirstChildRow() != NULL) {
				nResult = MsgBox(MB_YESNO, "Would you like to reactivate all the child referral sources of '%s'?", 
					VarString(pRow->GetValue(ercName)));
			} else {
				nResult = IDNO;
			}

			//Check permissions
			// (a.wilson 2012-5-23) PLID 50602 - update to use new permissions.
			CPermissions permRefTree = GetCurrentUserPermissions(bioReferralTree);

			if (!(permRefTree & (sptDynamic0)) && (permRefTree & (sptDynamic0WithPass))) {
				if (!CheckCurrentUserPassword()) {
					return;
				}
			}

			//using CTE and OUTPUT we recursivly update the persont records (thanks to a.walling on help with this query).
			_RecordsetPtr prs;
			if (nResult == IDYES) {
				prs = CreateParamRecordset(
					"BEGIN TRANSACTION \r\n"
						"SET NOCOUNT ON \r\n"
						"DECLARE @IDT Table (ID INT NOT NULL); \r\n"
						"WITH ReferralTree (PersonID, Name, Parent) AS ( \r\n"
							"SELECT ref.PersonID, ref.Name, ref.Parent \r\n"
							"FROM ReferralSourceT AS ref \r\n"
							"WHERE ref.PersonID = {INT} \r\n"
							"UNION ALL \r\n"
							"SELECT ref.PersonID, ref.Name, ref.Parent \r\n"
							"FROM ReferralSourceT AS ref \r\n"
							"INNER JOIN ReferralTree AS parentRef ON ref.Parent = parentRef.PersonID \r\n"
						") \r\n"
						"UPDATE PersonT SET Archived = 0 OUTPUT INSERTED.ID INTO @IDT FROM PersonT \r\n"
						"INNER JOIN ReferralSourceT ON PersonT.ID = ReferralSourceT.PersonID \r\n"
						"INNER JOIN ReferralTree ON PersonT.ID = ReferralTree.PersonID \r\n"
						"WHERE Archived = 1 \r\n"
						"OPTION (MAXRECURSION 32) \r\n"
						"SET NOCOUNT OFF \r\n"
						"\r\n"
						"SELECT ID FROM @IDT \r\n"
					"COMMIT TRANSACTION ", VarLong(pRow->GetValue(ercID)));
			} else {
				prs = CreateParamRecordset(
					"SET NOCOUNT ON \r\n"
					"DECLARE @IDT Table (ID INT NOT NULL); \r\n"
					"INSERT INTO @IDT (ID) VALUES ({INT}) \r\n"
					"\r\n"
					"UPDATE PersonT SET Archived = 0 WHERE PersonT.ID IN (SELECT ID FROM @IDT) AND Archived = 1 \r\n"
					"SET NOCOUNT OFF \r\n"
					"SELECT ID FROM @IDT ", VarLong(pRow->GetValue(ercID)));
			}

			//update the datalist.
			for (; !prs->eof; prs->MoveNext()) {
				IRowSettingsPtr pReferralRow = m_pList->FindByColumn(ercID, AdoFldLong(prs, "ID"), NULL, VARIANT_FALSE);

				if (pReferralRow) {
					pReferralRow->PutValue(ercInactive, g_cvarFalse);
					pReferralRow->PutForeColor(RGB(0, 0, 0));
					pReferralRow->PutForeColorSel(RGB(0, 0, 0));

					// (r.goldschmidt 2014-08-28 16:42) - PLID 31191 - increment child count of parent row
					IRowSettingsPtr pParentRow = pReferralRow->GetParentRow();
					if (pParentRow){
						long nChildCount = VarLong(pParentRow->GetValue(ercActiveChildrenCount)) + 1;
						pParentRow->PutValue(ercActiveChildrenCount, nChildCount);
					}
				}
			}
			prs->Close();

			// (a.wilson 2012-5-21) PLID 53078 - Refresh through tablecheckers
			CTableChecker *p[2];
			p[0] = &m_tcReferralChecker;
			p[1] = NULL;
			ReferralMultiRefresh(p, -1);
		}
	} NxCatchAll(__FUNCTION__);
}
// (a.wilson 2012-5-9) PLID 14874 - all we need to do here is update the datalist based on the state of the checkbox.
void CReferralSubDlg::OnBnClickedInactiveReferrals()
{
	try {
		ReloadTree();
	} NxCatchAll(__FUNCTION__);
}
// (a.wilson 2012-5-30) PLID 50602 - we need to check permissions before allowing them to move around the referrals.
void CReferralSubDlg::OnBnClickedEnableDrag()
{
	try {
		CPermissions permRefTree = GetCurrentUserPermissions(bioReferralTree);

		if (!(permRefTree & (sptWrite)) && (permRefTree & (sptWriteWithPass)) && m_btnEnableDrag.GetCheck() == BST_CHECKED) {
			if (!CheckCurrentUserPassword()) {
				m_btnEnableDrag.SetCheck(0);
			}
		}
	} NxCatchAll(__FUNCTION__);
}
