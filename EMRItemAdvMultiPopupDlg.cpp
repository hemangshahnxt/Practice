// EMRItemAdvMultiPopupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PatientsRc.h"
#include "EMRItemAdvMultiPopupDlg.h"
#include "EMNDetail.h"
#include "EMRItemAdvPopupWnd.h"
#include "EMRItemAdvImageDlg.h"
#include "EMRTopic.h"
#include "EMN.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


using namespace ADODB;
using namespace NXDATALIST2Lib;


DetailPopup::DetailPopup() 
{
	m_pDetail = m_pSourceDetail = NULL;
	m_pWindow = NULL;
	m_pParent = m_pChild = m_pPrevious = m_pNext = NULL;
}


DetailPopup::~DetailPopup()
{
	//TES 1/28/2008 - PLID 24157 - Release the detail, so it can clean itself up if necessary.
	if(m_pDetail) {
		//m_pDetail->Release();
		// (a.walling 2009-10-12 16:05) - PLID 36024
		m_pDetail->__Release("~DetailPopup detail");
	}
	// (a.walling 2009-10-12 10:07) - PLID 36024
	if(m_pSourceDetail) {
		//m_pSourceDetail->Release();
		// (a.walling 2009-10-12 16:05) - PLID 36024
		m_pSourceDetail->__Release("~DetailPopup sourcedetail");
	}
	//TES 1/14/2008 - PLID 24157 - If we created an EMRItemAdvPopupWnd, destroy it.
	if(m_pWindow) {
		if(IsWindow(m_pWindow->GetSafeHwnd())) {
			m_pWindow->DestroyWindow();
		}
		delete m_pWindow;
		m_pWindow = NULL;
	}
	//TES 1/11/2008 - PLID 24157 - We are responsible for our children and successors, not parents or predecessors.
	if(m_pNext) delete m_pNext;
	m_pNext = NULL;
	if(m_pChild) delete m_pChild;
	m_pChild = NULL;
};

DetailPopup* DetailPopup::GetLastInTree() 
{
	//TES 1/11/2008 - PLID 24157 - The order is us, then our children, then our successors.  So if we have successors,
	// then whatever's last for them is last for us, otherwise check our children, otherwise, we're it!
	if(m_pNext) return m_pNext->GetLastInTree();
	else if(m_pChild) return m_pChild->GetLastInTree();
	else return this;
}

DetailPopup* DetailPopup::GetPreviousInTree() 
{
	//TES 1/11/2008 - PLID 24157 - We should only have either a predecessor or a parent, but not both.  So whichever one
	// we have, that's the "previous" one to us in the tree.  If we have neither, we must be first.
	if(m_pPrevious) {
		ASSERT(!m_pParent);
		return m_pPrevious;
	}
	else if(m_pParent) {
		return m_pParent;
	}
	else {
		return NULL;
	}
}
DetailPopup* DetailPopup::GetParentInTree() 
{
	//TES 1/11/2008 - PLID 24157 - Find the first node in our list, then return its parent.
	DetailPopup *p = this;
	while(p->m_pPrevious != NULL) p = p->m_pPrevious;
	return p->m_pParent;
}

DetailPopup* DetailPopup::GetNextInTree() 
{
	//TES 1/11/2008 - PLID 24157 - This particular function was pulled out of RichTextNode in richeditutils.
	//If we have a child, it's next.
	if(m_pChild) return m_pChild;

	//Otherwise, if we have a follower, it's next.
	if(m_pNext) return m_pNext;

	//Otherwise, it's whatever follows our parent (which is the parent of the first node in our list).
	DetailPopup *pParent = GetParentInTree();
	while(pParent && pParent->m_pNext == NULL) {
		pParent = pParent->GetParentInTree();
	}
	if(pParent) {
		return pParent->m_pNext;
	}
	else {
		//End of the line!
		return NULL;
	}
}

void DetailPopup::RemoveDetailFromTree(CEMNDetail *pDetail) 
{
	//TES 1/11/2008 - PLID 24157 - If either our child or our successor is the detail to be removed, we will remove it.
	DetailPopup *dpDoomed = NULL;
	if(m_pChild) {
		if(m_pChild->m_pDetail == pDetail) {
			//TES 1/11/2008 - PLID 24157 - This is what we need to remove.  Its successor will be our new child.
			dpDoomed = m_pChild;
			m_pChild = dpDoomed->m_pNext;
			if(m_pChild) {
				m_pChild->m_pParent = this;
				m_pChild->m_pPrevious = NULL;
			}
		}
	}
	if(dpDoomed == NULL && m_pNext != NULL) {
		//TES 1/11/2008 - PLID 24157 - OK, it wasn't our child.  Is it our successor?
		if(m_pNext->m_pDetail == pDetail) {
			//TES 1/11/2008 - PLID 24157 - This is what we need to remove.  Its successor will be our new successor.
			dpDoomed = m_pNext;
			m_pNext = dpDoomed->m_pNext;
			if(m_pNext) {
				m_pNext->m_pPrevious = this;
			}
		}
	}

	if(dpDoomed) {
		//TES 1/11/2008 - PLID 24157 - We're removing a detail, did it have any children?
		if(dpDoomed->m_pChild) {
			//TES 1/11/2008 - PLID 24157 - Well, this is a little weird.  We would generally expect that the things 
			// spawned by this detail would be removed if it's removed.  And maybe they will be soon.  But we were only
			// asked to remove this detail, not its children, so we need to make sure they don't get lost.  Let's just
			// go ahead and move them to the end of the tree, that is, as successors of the last top-level-node.
			DetailPopup* dpParent = GetParentInTree();
			DetailPopup* dpTopLevel = this;
			while(dpParent) {
				dpTopLevel = dpParent;
				dpParent = dpParent->GetParentInTree();
			}
			//TES 1/11/2008 - PLID 24157 - We're now at the top level, move to the end and add the doomed node's 
			// children afterwards.
			DetailPopup* dpNext = dpTopLevel->m_pNext;
			DetailPopup* dpLast = dpTopLevel;
			while(dpNext) {
				dpLast = dpNext;
				dpNext = dpNext->m_pNext;
			}
			dpLast->m_pNext = dpDoomed->m_pChild;
			dpDoomed->m_pChild->m_pParent = NULL;
			dpDoomed->m_pChild->m_pPrevious = dpLast;
		}

		//TES 1/11/2008 - PLID 24157 - OK, we've reassigned everything around the doomed detail, now we just need
		// to clean it up, first making sure that it's not pointing to anything (so it doesn't try to recursively clean
		// up other details that we're not removing).
		dpDoomed->m_pNext = NULL;
		dpDoomed->m_pPrevious = NULL;
		dpDoomed->m_pChild = NULL;
		dpDoomed->m_pParent = NULL;
		delete dpDoomed;
	} else {
		//TES 1/11/2008 - PLID 24157 - Well, neither our child nor successor was the detail in question, so we'll just
		// pass the request on to whoever's next in the tree.
		//TES 1/23/2008 - PLID 24157 - Make sure there actual IS a next node in the tree.
		DetailPopup* dpNext = GetNextInTree();
		if(dpNext) {
			dpNext->RemoveDetailFromTree(pDetail);
		}
	}

}

/////////////////////////////////////////////////////////////////////////////
// CEMRItemAdvMultiPopupDlg dialog

enum DetailListColumns {
	dlcDetail = 0,
	dlcSourceDetail = 1,
	dlcName = 2,
	dlcWindow = 3,
	dlcNeedReposition = 4,
};

// (a.walling 2009-01-20 14:34) - PLID 29800 - Need to IMPLEMENT_/DECLARE_DYNAMIC to enable MFC's CObject-based RTTI
IMPLEMENT_DYNAMIC(CEMRItemAdvMultiPopupDlg, CNxDialog);

CEMRItemAdvMultiPopupDlg::CEMRItemAdvMultiPopupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMRItemAdvMultiPopupDlg::IDD, pParent)
{
	m_pDetailsToPopup = NULL;
	m_pCurrentWnd = NULL;
	m_pSourceDetail = NULL;
	m_nSourceDataGroupID = -1;
	m_szIdealPopupWndSize = CSize(0,0);
	m_bShowList = false;
	// (a.walling 2010-01-14 16:09) - PLID 31118 - Deprecated
	//m_bNeedRepositionBackToEmn = false;
	//{{AFX_DATA_INIT(CEMRItemAdvMultiPopupDlg)
	//}}AFX_DATA_INIT
}

CEMRItemAdvMultiPopupDlg::~CEMRItemAdvMultiPopupDlg()
{
	//TES 1/28/2008 - PLID 28673 - It's no longer our responsibility to clean this up, our caller will hold onto the
	// memory so it can restore the dialog if necessary, and clean it up when appropriate.
	/*if(m_pDetailsToPopup) {
		//TES 1/11/2008 - PLID 24157 - This will go through and release all our EMN details (we added a reference to them
		// when they were added).
		m_pDetailsToPopup->ReleaseDetail();
	}
	//TES 1/11/2008 - PLID 24157 - Now clean up our own memory.
	delete m_pDetailsToPopup;*/

}

void CEMRItemAdvMultiPopupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMRItemAdvMultiPopupDlg)
	DDX_Control(pDX, IDC_SHOW_LIST, m_nxbShowList);
	DDX_Control(pDX, IDC_NEXT_DETAIL, m_nxbNext);
	DDX_Control(pDX, IDC_PREVIOUS_DETAIL, m_nxbPrevious);
	DDX_Control(pDX, IDOK, m_nxbOK);
	DDX_Control(pDX, IDC_BACK_TO_EMN, m_nxbBackToEmn);
	DDX_Control(pDX, IDC_EMR_MULTIPOP_COLOR, m_nxc);
	DDX_Control(pDX, IDC_ITEM_PLACEHOLDER, m_nxstaticItemPlaceholder);
	//}}AFX_DATA_MAP
}

BEGIN_EVENTSINK_MAP(CEMRItemAdvMultiPopupDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEMRItemAdvMultiPopupDlg)
	ON_EVENT(CEMRItemAdvMultiPopupDlg, IDC_DETAIL_LIST, 2 /* SelChanged */, OnSelChangedDetailList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CEMRItemAdvMultiPopupDlg, IDC_DETAIL_LIST, 1 /* SelChanging */, OnSelChangingDetailList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CEMRItemAdvMultiPopupDlg, IDC_DETAIL_LIST, 19 /* LeftClick */, OnLeftClickDetailList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BEGIN_MESSAGE_MAP(CEMRItemAdvMultiPopupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMRItemAdvMultiPopupDlg)
	ON_BN_CLICKED(IDC_NEXT_DETAIL, OnNextDetail)
	ON_BN_CLICKED(IDC_PREVIOUS_DETAIL, OnPreviousDetail)
	ON_MESSAGE(NXM_EMR_POPUP_POST_STATE_CHANGED, OnPostStateChanged)
	ON_BN_CLICKED(IDC_BACK_TO_EMN, OnBackToEmn)
	ON_MESSAGE(NXM_EMR_POPUP_RESIZED, OnPopupResized)
	ON_BN_CLICKED(IDC_SHOW_LIST, OnShowList)
	ON_MESSAGE(NXM_EMR_MINIMIZE_PIC, OnEmrMinimizePic)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMRItemAdvMultiPopupDlg message handlers

BOOL CEMRItemAdvMultiPopupDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (a.walling 2010-01-26 17:46) - PLID 31118 - Bulk cache these properties
		g_propManager.CachePropertiesInBulk("CEMRItemAdvMultiPopupDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'EMR_MultiPopup_ShowList' OR "
			"Name = 'DefaultEMRImagePenColor' OR "
			"Name = 'EMR_MultiPopup_BackToEmnOnLastItem' OR "
			"Name = 'EmnPopupAutoDismiss' OR "
			"Name = 'EMRRememberPoppedUpTableColumnWidths' " // (r.gonet 02/14/2013) - PLID 40017
			")"
			, _Q(GetCurrentUserName()));

		m_pDetailList = BindNxDataList2Ctrl(this, IDC_DETAIL_LIST, NULL, false);

		//TES 1/18/2007 - PLID 24157 - Set the font in our list to be 1/3 again bigger than its default; this is to 
		// make it easier to click on rows using a Tablet PC stylus.
		// Get the font object
		// (a.walling 2012-08-16 08:38) - PLID 52164 - Fixed memory leak -- GetFont() returns a raw IFontDisp* which must be released.
		IFontPtr pDetailFont = IFontDispPtr(m_pDetailList->GetFont(), false);
		// Adjust it to be 1/3 bigger
		CY cy;
		pDetailFont->get_Size(&cy);
		cy.int64 = cy.int64 * 4 / 3;
		pDetailFont->put_Size(cy);
		// And make sure the datalist is using it (rather than the Windows default)
		m_pDetailList->PutOverrideDefaultFont(VARIANT_TRUE);

		//TES 2/25/2008 - PLID 28827 - We changed these to say "Next" and "Previous" rather than being arrows.
		//m_nxbPrevious.AutoSet(NXB_LEFT);
		//m_nxbNext.AutoSet(NXB_RIGHT);

		//DRT 4/22/2008 - PLID 29749 - Set to OK type for green checkmark
		m_nxbBackToEmn.AutoSet(NXB_OK);
		m_nxbOK.AutoSet(NXB_OK);
		// (a.walling 2010-01-14 14:54) - PLID 31118
		m_nxbBackToEmn.SetWindowText("&Done");

		//TES 1/18/2008 - PLID 24157 - At least for the time being, we're setting all colors on this dialog
		// to the patient color.
		m_nxc.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));

		//TES 1/11/2008 - PLID 24157 - We may have had details added before we were made visible (in fact, that's probable),
		// so make sure they are reflected on screen.
		RefreshDetails();

		//TES 3/25/2008 - PLID 28827 - If there are no details, they must have removed spawned details off of the EMN,
		// and then clicked the * to restore the popup.  There's no point in going on, in that case.
		if(!m_pDetailsToPopup) {
			MsgBox("All details spawned by this item have been deleted.");
			OnOK();
			return TRUE;
		}

		//TES 2/21/2008 - PLID 28827 - We'll want to pass into the popupwnd our preferred size for items (such as text
		// or slider items) that don't really care what size they are.  We'll set that sizeto the size of our placeholder
		// in the resources, that way if we want to change it we can just change the resources, which will make it easier
		// to tell that everything looks good.
		CRect rc;
		GetDlgItem(IDC_ITEM_PLACEHOLDER)->GetWindowRect(&rc);
		m_szIdealPopupWndSize.cx = rc.Width();
		m_szIdealPopupWndSize.cy = rc.Height();

		//TES 8/20/2008 - PLID 25456 - Because this dialog has a child window, which in turn has a control on it 
		// (for image, table, and narrative types, anyway), we need to call this function so that keyboard controls, among
		// others, will get passed to those controls rather than eaten by the dialog.  The function is s undocumented, but 
		// I've found several forum posts and such online (mostly dealing with property sheets, but it's the same issue),
		// indicating that this is the "correct" solution.
		InitControlContainer();

		//TES 2/25/2008 - PLID 28827 - Remember whether or not we're showing the list of details.
		m_bShowList = GetRemotePropertyInt("EMR_MultiPopup_ShowList", 1, 0, GetCurrentUserName(), true) == 0 ? false : true;
		if(m_bShowList) {
			m_nxbShowList.SetWindowText(">> Hide List");
		}
		else {
			m_nxbShowList.SetWindowText("<< Show List");
		}
		
		//TES 1/14/2008 - PLID 24157 - Since our list currently has nothing selected, this will select the first row.
		OnNextDetail();

	} NxCatchAll("Error in OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEMRItemAdvMultiPopupDlg::AddDetail(CEMNDetail *pDetail, CEMNDetail *pSourceDetail)
{
	//TES 1/11/2008 - PLID 24157 - First, make sure that this detail isn't destroyed until we're done with it.
	//pDetail->AddRef();
	// (a.walling 2009-10-12 16:05) - PLID 36024
	pDetail->__AddRef("CEMRItemAdvMultiPopupDlg::AddDetail detail");
	// (a.walling 2009-10-12 10:08) - PLID 36024
	//pSourceDetail->AddRef();	
	// (a.walling 2009-10-12 16:05) - PLID 36024
	pSourceDetail->__AddRef("CEMRItemAdvMultiPopupDlg::AddDetail source detail");

	//TES 1/11/2008 - PLID 24157 - Now, make a node for this detail.
	DetailPopup *pDp = new DetailPopup;
	pDp->m_pDetail = pDetail;
	pDp->m_pSourceDetail = pSourceDetail;
		
	//TES 1/11/2008 - PLID 24157 - Now, figure out where to put it.
	if(!m_pDetailsToPopup) {
		//TES 1/11/2008 - PLID 24157 - Sweet, this is the first one.
		m_pDetailsToPopup = pDp;
	}
	else {
		//TES 1/11/2008 - PLID 24157 - OK, we need to figure out where in the tree to put it.  We'll go through in
		// reverse order; if we find the detail that spawned this one, then we will add it at the end of that detail's
		// children.  If we find a detail that has the same source detail as us, then we will add ourselves immediately
		// after it.
		//Our pointer for traversal.
		DetailPopup *dpCurrent = m_pDetailsToPopup->GetLastInTree();
		bool bDone = false;
		while(dpCurrent && !bDone) {
			if(dpCurrent->m_pDetail == pSourceDetail) {
				//TES 1/11/2008 - PLID 24157 - Add as the last child.
				DetailPopup *dpChild = dpCurrent->m_pChild;
				if(!dpChild) {
					//TES 1/11/2008 - PLID 24157 - This is the first child of this detail.
					pDp->m_pParent = dpCurrent; 
					dpCurrent->m_pChild = pDp;
					bDone = true;
				}
				else {
					//TES 1/11/2008 - PLID 24157 - Add on to the end.
					DetailPopup *dpCurrentChild = dpChild;
					while(dpChild) {
						dpCurrentChild = dpChild;
						dpChild = dpChild->m_pNext;
					}
					pDp->m_pPrevious = dpCurrentChild;
					dpCurrentChild->m_pNext = pDp;
					bDone = true;
				}
			}
			else if(dpCurrent->m_pSourceDetail == pSourceDetail) {
				//TES 1/11/2008 - PLID 24157 - Insert ourselves into the list immediately following dpCurrent.
				pDp->m_pPrevious = dpCurrent;
				pDp->m_pNext = dpCurrent->m_pNext;
				dpCurrent->m_pNext = pDp;
				bDone = true;
			}
			//TES 1/11/2008 - PLID 24157 - Keep traversing.
			dpCurrent = dpCurrent->GetPreviousInTree(); 
		}
		if(!bDone) {
			//TES 1/11/2008 - PLID 24157 - If we get here, there was nothing matching, so just add at the end 
			// of the top level.
			dpCurrent = m_pDetailsToPopup;
			DetailPopup *dpNext = dpCurrent->m_pNext;
			while(dpNext) {
				dpCurrent = dpNext;
				dpNext = dpNext->m_pNext;
			}
			pDp->m_pPrevious = dpCurrent;
			dpCurrent->m_pNext = pDp;
		}
	}

	//TES 1/11/2008 - PLID 24157 - Now, refresh the display to show this new detail.
	RefreshDetails();
}

void CEMRItemAdvMultiPopupDlg::RefreshDetails()
{
	//TES 1/11/2008 - PLID 24157 - If we don't have a window, then we don't need to do anything.
	if(GetSafeHwnd()) {
		//TES 1/14/2008 - PLID 24157 - Remember which detail was selected.
		CEMNDetail *pCurDetail = NULL;
		if(m_pDetailList->CurSel != NULL) {
			pCurDetail = (CEMNDetail*)VarLong(m_pDetailList->CurSel->GetValue(0));
		}
		
		//TES 1/11/2008 - PLID 24157 - Recreate our list by traversing our tree in "tree order".
		m_pDetailList->SetRedraw(FALSE);
		m_pDetailList->Clear();
		DetailPopup *pDp = m_pDetailsToPopup;
		while(pDp) {
			IRowSettingsPtr pRow = m_pDetailList->GetNewRow();
			pRow->PutValue(dlcDetail, (long)pDp->m_pDetail);
			pRow->PutValue(dlcSourceDetail, (long)pDp->m_pSourceDetail);
			pRow->PutValue(dlcName, _bstr_t(pDp->m_pDetail->GetLabelText()));
			pRow->PutValue(dlcWindow, (long)pDp->m_pWindow);
			//TES 2/25/2008 - PLID 28827 - Added a column for tracking which windows need to be repositioned.
			pRow->PutValue(dlcNeedReposition, (long)0);
			IRowSettingsPtr pNewRow = m_pDetailList->AddRowAtEnd(pRow, NULL);
			//TES 1/14/2008 - PLID 24157 - If this was our previously selected detail, re-select it.
			if(pDp->m_pDetail == pCurDetail) {
				m_pDetailList->CurSel = pNewRow;
			}
			pDp = pDp->GetNextInTree();
		}

		if(m_pDetailList->CurSel == NULL) {
			//TES 1/23/2008 - PLID 24157 - Our displayed detail may have been removed, make sure the display is 
			// up to date.
			RefreshDetailWindow();
		}

		//TES 2/21/2008 - PLID 28827 - We may have a different count of details now, update our caption.
		RefreshTitle();

		//TES 5/6/2010 - PLID 38551 - We may now have a Next button when we previously didn't (or vice versa), so make sure that button
		// is up to date.
		RefreshButtons();

		// TES 1/15/2008 - PLID 24157 - Now make sure all the background colors are up to date.
		RefreshDetailColors();
		m_pDetailList->SetRedraw(TRUE);
	}
}

void CEMRItemAdvMultiPopupDlg::RemoveDetail(CEMNDetail *pDetail)
{
	if(pDetail == m_pDetailsToPopup->m_pDetail) {
		DetailPopup *dpDoomed = m_pDetailsToPopup;
		//TES 1/11/2008 - PLID 24157 - We can't call RemoveDetailFromTree on the detail being removed, so do it ourselves.
		// First, does this node have a successor?
		if(m_pDetailsToPopup->m_pNext) {
			//TES 1/11/2008 - PLID 24157 - Yes, it does.  That will be our new root node.
			m_pDetailsToPopup = dpDoomed->m_pNext;
			m_pDetailsToPopup->m_pPrevious = NULL;
			dpDoomed->m_pNext = NULL;
			//TES 1/11/2008 - PLID 24157 - Did it have any children?
			if(dpDoomed->m_pChild) {
				//TES 1/11/2008 - PLID 24157 - Move them to the end of the top level.
				DetailPopup *dpNext = m_pDetailsToPopup->m_pNext;
				DetailPopup *dpLast = m_pDetailsToPopup;
				while(dpNext) {
					dpLast = dpNext;
					dpNext = dpNext->m_pNext;
				}
				dpLast->m_pNext = dpDoomed->m_pChild;
				dpDoomed->m_pChild->m_pPrevious = dpLast;
				dpDoomed->m_pChild->m_pParent = NULL;
				dpDoomed->m_pChild = NULL;
			}
		}
		else if(m_pDetailsToPopup->m_pChild) {
			//TES 1/11/2008 - PLID 24157 - It didn't have a successor, but it did have a child, the child will be our
			// new root node.
			m_pDetailsToPopup = dpDoomed->m_pChild;
			m_pDetailsToPopup->m_pParent = NULL;
			dpDoomed->m_pChild = NULL;
		}
		else {
			//TES 1/11/2008 - PLID 24157 - It didn't have a parent or child, this must have been the only node in the
			// tree.  So, we have no root node anymore.
			m_pDetailsToPopup = NULL;
		}

		//TES 1/11/2008 - PLID 24157 - We've detached this node now, so we just need to clean it up.
		delete dpDoomed;

	}
	else {
		//TES 1/11/2008 - PLID 24157 - Just pass it on.
		m_pDetailsToPopup->RemoveDetailFromTree(pDetail);
	}
	//TES 1/11/2008 - PLID 24157 - Now, update the screen to reflect the absence of this detail.
	RefreshDetails();
}

void CEMRItemAdvMultiPopupDlg::OnSelChangedDetailList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try {
		//TES 1/14/2008 - PLID 24157 - Just update the screen to make sure the new detail is shown.
		RefreshDetailWindow();
	}NxCatchAll("Error in CEMRItemAdvMultiPopupDlg::OnSelChangedDetailList()");
}


void CEMRItemAdvMultiPopupDlg::CreateControlsWindow(CEMNDetail *pDetail)
{
	// Calculate the max amount of space the controls can take up
	//TES 2/21/2008 - PLID 28827 - We used to have a bunch of code here copied out of CEMRItemAdvPopupDlg; now we do
	// our own calculations, in their own function down by the PopupResized handler that it needs to be in sync with.
	CSize szMax = CalcMaxSize();
	
	//TES 1/14/2008 - PLID 24157 - For the moment, we're keeping the size static, so we'll pass in the size of our
	// placeholder control as both the minimum and maximum.  For this item we also added the ability to pass in an offset, 
	// since our placeholder isn't at the upper left of our window.
	//TES 2/21/2008 - PLID 28827 - We no longer keep the size static; however, we do still pass in the offset here.
	CRect rcPlaceholder;
	GetDlgItem(IDC_ITEM_PLACEHOLDER)->GetWindowRect(&rcPlaceholder);
	ScreenToClient(&rcPlaceholder);
	/*szMax.cx = rcPlaceholder.Width();
	szMax.cy = rcPlaceholder.Height();*/

	// (j.jones 2009-12-16 14:17) - PLID 31021 - we once again take a minimum size
	CSize szMin;
	szMin.cx = 200;
	szMin.cy = 200;

	if(szMin.cx > szMax.cx) {
		szMax.cx = szMin.cx;
	}
	if(szMin.cy > szMax.cy) {
		szMax.cy = szMin.cy;
	}

	//TES 1/14/2008 - PLID 24157 - Also, pass in true to have it draw a border.
	// (a.walling 2008-01-18 15:03) - PLID 14982 - Need to pass another param for the real detail, which is pDetail.
	//TES 2/21/2008 - PLID 28827 - This function no longer takes a minimum size, but rather an ideal size, which we
	// calculated earlier and just need to pass in.
	m_pCurrentWnd->Initialize(this, this, rcPlaceholder, pDetail, pDetail, false, szMax, szMin, rcPlaceholder.top, rcPlaceholder.left, m_szIdealPopupWndSize, true);
}

void CEMRItemAdvMultiPopupDlg::OnNextDetail() 
{
	try {
		//TES 1/14/2008 - PLID 24157 - If there's a currently selected row, make sure whatever changes have been 
		// made to it have been reflected.
		IRowSettingsPtr pRow = m_pDetailList->CurSel;
		if(pRow) {
			CEMNDetail *pCurDetail = (CEMNDetail*)VarLong(pRow->GetValue(dlcDetail));
			pCurDetail->RequestStateChange(pCurDetail->GetState());

			if (pCurDetail->Is2DImage()) {
				if (pCurDetail->m_pEmrItemAdvDlg && IsWindow(pCurDetail->m_pEmrItemAdvDlg->GetSafeHwnd())) {
					CEMRItemAdvPopupWnd *pWnd = (CEMRItemAdvPopupWnd*)VarLong(pRow->GetValue(dlcWindow));
					CEmrItemAdvImageDlg* pItemAdvImageDlg = dynamic_cast<CEmrItemAdvImageDlg*>(pCurDetail->m_pEmrItemAdvDlg);
					pItemAdvImageDlg->SetCurrentPenColor(pWnd->m_nCurPenColor);
					pItemAdvImageDlg->SetCurrentPenSize(pWnd->m_fltCurPenSize);
				}
			}
		}
		//TES 1/14/2008 - PLID 24157 - It's possible that requesting that state change may have caused the detail
		// list to be requeried, so re-obtain the CurSel.
		pRow = m_pDetailList->CurSel;
		//TES 1/14/2008 - PLID 24157 - Now advance to the next row (if nothing is selected, just go to the first row).
		if(pRow) {
			m_pDetailList->CurSel = pRow->GetNextRow();
		}
		else {
			m_pDetailList->CurSel = m_pDetailList->GetFirstRow();
		}
		// TES 1/15/2008 - PLID 24157 - Make sure the background colors are up to date.
		RefreshDetailColors();
		//TES 1/14/2008 - PLID 24157 - Make sure the correct detail is displayed.
		RefreshDetailWindow();
	}NxCatchAll("Error in CEMRItemAdvMultiPopupDlg::OnNextDetail()");	
}

void CEMRItemAdvMultiPopupDlg::RefreshDetailWindow()
{
	//TES 1/14/2008 - PLID 24157 - Get the current selection.
	IRowSettingsPtr pRow = m_pDetailList->CurSel;
	CEMRItemAdvPopupWnd* pWnd = NULL;
	//TES 2/21/2008 - PLID 28827 - Track whether the windows is already created and positioned properly.
	BOOL bWindowExisted = FALSE;
	BOOL bNeedReposition = FALSE;
	if(pRow) {
		CEMNDetail* pDetail = (CEMNDetail*)VarLong(pRow->GetValue(dlcDetail));
		
		//TES 1/14/2008 - PLID 24157 - Make sure all the detail's list elements or whatever are loaded.
		pDetail->LoadContent();

		//TES 1/18/2008 - PLID 24157 - Make sure the extra "Back to EMN" button is hidden.
		GetDlgItem(IDC_BACK_TO_EMN)->ShowWindow(SW_HIDE);
		//TES 2/25/2008 - PLID 28827 - And make sure the main "Back to EMN" button is visible.
		GetDlgItem(IDOK)->ShowWindow(SW_SHOWNA);

		//Does this already have a window?
		pWnd = (CEMRItemAdvPopupWnd*)VarLong(pRow->GetValue(dlcWindow));
		if(!pWnd) {
			//TES 1/14/2008 - PLID 24157 - Nope, we need to create one.
			pWnd = new CEMRItemAdvPopupWnd;

			// Set member variables for the window
			//TES 1/18/2008 - PLID 24157 - At least for the time being, we're setting all colors on this dialog
			// to the patient color.
			pWnd->m_clrHilightColor = GetNxColor(GNC_PATIENT_STATUS, 1);
			// (r.gonet 02/14/2013) - PLID 40017 - The popup window needs to know if it should save the column widths when the user resizes the columns in the popup.
			BOOL bRememberPoppedUpTableColumnWidths = GetRemotePropertyInt("EmrRememberPoppedUpTableColumnWidths", 0, 0, "<None>", true) != 0 ? TRUE : FALSE;
			pWnd->SetRememberPoppedUpTableColumnWidths(bRememberPoppedUpTableColumnWidths);
			//TES 1/16/2008 - PLID 24157 - For these image-related values, try and get them from the item's 
			// EmrItemAdvImageDlg.
			if(pDetail->m_EMRInfoType == eitImage) {
				if(pDetail->m_pEmrItemAdvDlg && IsWindow(pDetail->m_pEmrItemAdvDlg->GetSafeHwnd())) {
					// (j.armen 2014-07-23 11:19) - PLID 62836 - Methods for getting current Pen Color / Size
					CEmrItemAdvImageDlg* pItemAdvImageDlg = dynamic_cast<CEmrItemAdvImageDlg*>(pDetail->m_pEmrItemAdvDlg);
					if (pDetail->Is2DImage())
					{
						pWnd->m_nCurPenColor = pItemAdvImageDlg->GetCurrentPenColor();
						pWnd->m_fltCurPenSize = pItemAdvImageDlg->GetCurrentPenSize();
					}
					pWnd->m_bIsEraserState = pItemAdvImageDlg->m_bIsEraserState;
				}
				else {
					//TES 1/16/2008 - PLID 24157 - We still want to initialize the pen color, the rest are fine as
					// whatever the inkpicture defaults them to.
					pWnd->m_nCurPenColor = GetRemotePropertyInt("DefaultEMRImagePenColor",RGB(255,0,0),0,GetCurrentUserName(),TRUE);
				}
			}
			//TES 1/16/2008 - PLID 24157 - The linked item list is a proeperty of the detail's EMN.
			pWnd->m_strLinkedItemList = pDetail->m_pParentTopic->GetParentEMN()->GetTableItemList();
			
			//TES 1/14/2008 - PLID 24157 - Hide the existing window, if any.
			if(m_pCurrentWnd) {
				m_pCurrentWnd->ShowWindow(SW_HIDE);
			}
			//TES 1/14/2008 - PLID 24157 - Now set this new one as our current window, and create it.
			m_pCurrentWnd = pWnd;
			CreateControlsWindow(pDetail);

			//TES 1/14/2008 - PLID 24157 - Update our datalist.
			pRow->PutValue(dlcWindow, (long)pWnd);

			//TES 1/14/2008 - PLID 24157 - And update our detail tree.
			DetailPopup *pDp = m_pDetailsToPopup;
			bool bFound = false;
			while(pDp && !bFound) {
				if(pDp->m_pDetail == pDetail) {
					pDp->m_pWindow = pWnd;
					bFound = true;
				}
				pDp = pDp->GetNextInTree();
			}
		}
		else {
			//TES 2/21/2008 - PLID 28827 - The window's already created, we need to reposition ourself around it.
			bWindowExisted = TRUE;
			//TES 2/21/2008 - PLID 28827 - Check whether the window itself needs to be repositioned.
			bNeedReposition = VarLong(pRow->GetValue(dlcNeedReposition),0) == 1 ? TRUE : FALSE;
		}

		// (a.wetta 2007-03-09 10:15) - PLID 24757 - Make sure that the popup window is on top so that
		// its controls get the focus first and not the OK and Cancel button on the dialog.
		pWnd->SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
	/*
	else {
		//TES 1/18/2008 - PLID 24157 - Show the extra Back To EMN button.
		GetDlgItem(IDC_BACK_TO_EMN)->ShowWindow(SW_SHOWNA);
		//TES 2/25/2008 - PLID 28827 - And hide the other one, otherwise it's a little confusing.
		GetDlgItem(IDOK)->ShowWindow(SW_HIDE);

		//TES 3/11/2008 - PLID 28827 - Check whether we need to reposition this screen (because the list has been shown/hidden).
		bNeedReposition = m_bNeedRepositionBackToEmn;
		//TES 3/11/2008 - PLID 28827 - If we did need to reposition, we won't any more.
		m_bNeedRepositionBackToEmn = false;
	}
	*/

	//TES 5/6/2010 - PLID 38551 - Moved some of the RefreshDetailWindow() code into its own function.
	RefreshButtons();

	//TES 1/14/2008 - PLID 24157 - We've got a valid new window at this point, if it's not already the currently displayed
	// window, then display it.
	if(pWnd != m_pCurrentWnd || bNeedReposition) {
		//TES 1/14/2008 - PLID 24157 - First hide whatever's already showing.
		//TES 1/23/2008 - PLID 24157 - Make sure the window is actually a valid window.
		if(m_pCurrentWnd && m_pCurrentWnd->GetSafeHwnd() && ::IsWindow(m_pCurrentWnd->GetSafeHwnd())) {
			m_pCurrentWnd->ShowWindow(SW_HIDE);
		}

		m_pCurrentWnd = pWnd;

		if(bNeedReposition) {
			//TES 2/21/2008 - PLID 28827 - This window was created, but we've shown or hidden the detail list since
			// then, so we need to force it to reposition all its controls.
			if(m_pCurrentWnd) {
				m_pCurrentWnd->MoveWindow(0,0,1,1);
				m_pCurrentWnd->CallRepositionControls();
				if(pRow) {
					//TES 2/21/2008 - PLID 28827 - Now we don't need to do that for this window again.
					pRow->PutValue(dlcNeedReposition, (long)0);
				}
			}
			else {
				//TES 3/11/2008 - PLID 28827 - This means we need to resize the "Back To EMN" screen, probably because
				// the detail list was shown/hidden.  This message handler will do the trick.
				PostMessage(NXM_EMR_POPUP_RESIZED, (WPARAM)m_pCurrentWnd);
			}

		}
		else if(bWindowExisted) {
			//TES 2/21/2008 - PLID 28827 - Tell ourselves to resize ourselves to the new detail.
			PostMessage(NXM_EMR_POPUP_RESIZED, (WPARAM)m_pCurrentWnd);
		}

		//TES 1/23/2008 - PLID 24157 - Make sure the window is actually a valid window, and show it.
		if(m_pCurrentWnd && m_pCurrentWnd->GetSafeHwnd() && ::IsWindow(m_pCurrentWnd->GetSafeHwnd())) {
			m_pCurrentWnd->ShowWindow(SW_SHOWNA);
		}
	}

	//TES 2/21/2008 - PLID 28827 - Make sure our caption reflects the current detail.
	RefreshTitle();
}

//TES 5/6/2010 - PLID 38551 - Moved some of the RefreshDetailWindow() code into its own function.
void CEMRItemAdvMultiPopupDlg::RefreshButtons()
{
	//TES 1/18/2008 - PLID 24157 - Now, make sure the next and previous buttons are enabled correctly.
	{
		NXDATALIST2Lib::IRowSettingsPtr pCurSel = m_pDetailList->CurSel;
		NXDATALIST2Lib::IRowSettingsPtr pFirst = m_pDetailList->GetFirstRow();
		NXDATALIST2Lib::IRowSettingsPtr pLast = m_pDetailList->GetLastRow();

		if(pCurSel == pFirst) {
			m_nxbPrevious.EnableWindow(FALSE);
		}
		else {
			m_nxbPrevious.EnableWindow(TRUE);
		}

		// (a.walling 2010-01-14 14:39) - PLID 31118 - Disable 'next' if on the last item.
		if(pCurSel == NULL || pCurSel == pLast) {
			// (a.walling 2010-01-26 17:41) - PLID 31118 - A preference that will probably never be used, but apparently has some use.
			// (a.walling 2010-01-27 11:06) - PLID 31118 - Default EMR_MultiPopup_BackToEmnOnLastItem to TRUE.
			bool bBackToEmnOnLastItem = GetRemotePropertyInt("EMR_MultiPopup_BackToEmnOnLastItem", TRUE, 0, GetCurrentUserName(), true) ? true : false;
			
			m_nxbNext.EnableWindow(FALSE);
			if (bBackToEmnOnLastItem) {
				m_nxbNext.ShowWindow(SW_HIDE);
				if (!m_nxbBackToEmn.IsWindowVisible()) {
					m_nxbBackToEmn.ShowWindow(SW_SHOWNA);
				}
			}

			// I've decided to keep the top-right 'Back to EMN' button, since our 'Done' button is now in the bottom-right.
			// It doesn't really look bad now, and it doesn't hurt to have it there as well.
			/*
			m_nxbOK.ShowWindow(SW_HIDE);
			*/
		}
		else {
			m_nxbNext.EnableWindow(TRUE);
			if (!m_nxbNext.IsWindowVisible()) {
				m_nxbNext.ShowWindow(SW_SHOWNA);
			}
			if (m_nxbBackToEmn.IsWindowVisible()) {
				m_nxbBackToEmn.ShowWindow(SW_HIDE);
			}
			/*
			if (!m_nxbOK.IsWindowVisible()) {
				m_nxbNext.ShowWindow(SW_SHOWNA);
			}
			*/
		}
	}
}
void CEMRItemAdvMultiPopupDlg::OnPreviousDetail() 
{
	try {
		//TES 1/14/2008 - PLID 24157 - If there's a currently selected row, make sure whatever changes have been 
		// made to it have been reflected.
		IRowSettingsPtr pRow = m_pDetailList->CurSel;
		if(pRow) {
			CEMNDetail *pCurDetail = (CEMNDetail*)VarLong(pRow->GetValue(dlcDetail));
			pCurDetail->RequestStateChange(pCurDetail->GetState());

			if (pCurDetail->Is2DImage()) {
				if (pCurDetail->m_pEmrItemAdvDlg && IsWindow(pCurDetail->m_pEmrItemAdvDlg->GetSafeHwnd())) {
					CEMRItemAdvPopupWnd *pWnd = (CEMRItemAdvPopupWnd*)VarLong(pRow->GetValue(dlcWindow));
					CEmrItemAdvImageDlg* pItemAdvImageDlg = dynamic_cast<CEmrItemAdvImageDlg*>(pCurDetail->m_pEmrItemAdvDlg);
					pItemAdvImageDlg->SetCurrentPenColor(pWnd->m_nCurPenColor);
					pItemAdvImageDlg->SetCurrentPenSize(pWnd->m_fltCurPenSize);
				}
			}
		}
		//TES 1/14/2008 - PLID 24157 - It's possible that requesting that state change may have caused the detail
		// list to be requeried, so re-obtain the CurSel.
		pRow = m_pDetailList->CurSel;
		//TES 1/14/2008 - PLID 24157 - Now advance to the previous row (if nothing is selected, just go to the last row).
		if(pRow) {
			m_pDetailList->CurSel = pRow->GetPreviousRow();
		}
		else {
			m_pDetailList->CurSel = m_pDetailList->GetLastRow();
		}
		// TES 1/15/2008 - PLID 24157 - Make sure the background colors are up to date.
		RefreshDetailColors();
		//TES 1/14/2008 - PLID 24157 - Make sure the correct detail is displayed.
		RefreshDetailWindow();
	}NxCatchAll("Error in CEMRItemAdvMultiPopupDlg::OnPreviousDetail()");	
}

void CEMRItemAdvMultiPopupDlg::RefreshDetailColors()
{
	//TES 1/15/2008 - PLID 24157 - If we don't have a window, then we don't need to do anything.
	if(GetSafeHwnd()) {
		IRowSettingsPtr pRow = m_pDetailList->GetFirstRow();
		//TES 1/15/2008 - PLID 24157 - Check the state on each row.
		while(pRow) {
			CEMNDetail *pDetail = (CEMNDetail*)VarLong(pRow->GetValue(dlcDetail));
			if(pDetail->IsStateSet()) {
				//TES 1/15/2008 - PLID 24157 - This detail is complete.
				//TES 1/18/2008 - PLID 24157 - At least for the time being, we're setting all colors on this dialog
				// to the patient color.
				COLORREF clr = GetNxColor(GNC_PATIENT_STATUS, 1);
				pRow->PutBackColor(clr);
				pRow->PutBackColorSel(RGB(GetRValue(clr)/2,GetGValue(clr)/2,GetBValue(clr)/2));
				//TES 1/23/2008 - PLID 24157 - With this new backcolor, the datalist will decided to color the text
				// white instead of black.  But it won't actually do that until the selection changes, in the meantime
				// the text will be dark and difficult to read.  So, just set it to white ourselves.
				pRow->PutForeColorSel(RGB(255,255,255));
			}
			else {
				//TES 1/15/2008 - PLID 24157 - This detail is incomplete.
				pRow->PutBackColor(m_pDetailList->GetNewRow()->GetBackColor());
				pRow->PutBackColorSel(m_pDetailList->GetNewRow()->GetBackColorSel());
				//TES 1/23/2008 - PLID 24157 - We also need to reset the ForeColorSel, now that we set it.
				pRow->PutForeColorSel(m_pDetailList->GetNewRow()->GetForeColorSel());
			}
			pRow = pRow->GetNextRow();
		}		
	}
}

LRESULT CEMRItemAdvMultiPopupDlg::OnPostStateChanged(WPARAM wParam, LPARAM lParam)
{
	try {
		//TES 1/15/2008 - PLID 24157 - Make sure all our background colors are up to date.
		RefreshDetailColors();

		//TES 1/17/2008 - PLID 24157 - Some detail's state has changed, we need all our narratives and linked tables to 
		// be updated.
		IRowSettingsPtr pRow = m_pDetailList->GetFirstRow();
		while(pRow) {
			CEMRItemAdvPopupWnd *pWnd = (CEMRItemAdvPopupWnd*)VarLong(pRow->GetValue(dlcWindow));
			if(pWnd) {
				CEMNDetail *pDetail = (CEMNDetail*)VarLong(pRow->GetValue(dlcDetail));
				if(pDetail->m_EMRInfoType == eitNarrative) {
					pWnd->UpdateRichTextAppearance();
				}
				else if(pDetail->m_EMRInfoType == eitTable) {
					pWnd->m_strLinkedItemList = pDetail->m_pParentTopic->GetParentEMN()->GetTableItemList();
					pWnd->UpdateLinkedItemList(pDetail);
				}
			}
			pRow = pRow->GetNextRow();
		}

		//TES 1/15/2008 - PLID 24157 - If this is a single-select list, and the preference is to dismiss it as soon as
		// its set, then move to the next detail.
		CEMNDetail *pDetail = (CEMNDetail*)lParam;
		//TES 1/17/2008 - PLID 24157 - But only if this is our currently selected item, we might get this message for a
		// single-select list that was popped up from a narrative, for example.
		IRowSettingsPtr pCurSel = m_pDetailList->CurSel;
		if(pCurSel) {
			if(pDetail == (CEMNDetail*)VarLong(pCurSel->GetValue(dlcDetail)) && 
				pDetail->m_EMRInfoType == eitSingleList && pDetail->IsStateSet()) {
				if(GetRemotePropertyInt("EmnPopupAutoDismiss", 1, 0, GetCurrentUserName(), true)) {
					OnNextDetail();
				}
			}
		}
	}NxCatchAll("Error in CEMRItemAdvMultiPopupDlg::OnPostStateChanged()");
	return 0;
}

void CEMRItemAdvMultiPopupDlg::OnBackToEmn()
{
	OnOK();
}

void CEMRItemAdvMultiPopupDlg::OnSelChangingDetailList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		if (*lppNewSel == NULL) {
			//TES 1/18/2008 - PLID 24157 - Don't let them select nothing, change it back to the old row
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	
	}NxCatchAll("Error in CEMRItemAdvMultiPopupDlg::OnSelChangingDetailList()");
}

void CEMRItemAdvMultiPopupDlg::OnLeftClickDetailList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		if (m_pDetailList->CurSel)
		{
			CEMNDetail *pCurDetail = (CEMNDetail*)VarLong(m_pDetailList->CurSel->GetValue(dlcDetail));
			if (pCurDetail->Is2DImage()) {
				if (pCurDetail->m_pEmrItemAdvDlg && IsWindow(pCurDetail->m_pEmrItemAdvDlg->GetSafeHwnd())) {
					CEMRItemAdvPopupWnd *pWnd = (CEMRItemAdvPopupWnd*)VarLong(m_pDetailList->CurSel->GetValue(dlcWindow));
					CEmrItemAdvImageDlg* pItemAdvImageDlg = dynamic_cast<CEmrItemAdvImageDlg*>(pCurDetail->m_pEmrItemAdvDlg);
					pItemAdvImageDlg->SetCurrentPenColor(pWnd->m_nCurPenColor);
					pItemAdvImageDlg->SetCurrentPenSize(pWnd->m_fltCurPenSize);
				}
			}
		}

		//TES 1/22/2008 - PLID 24157 - Per Christina, the details are now hyperlinks.  So we need to set the selection
		// to the one they've clicked on, and handle that new selection.
		IRowSettingsPtr pNewSel(lpRow);
		m_pDetailList->CurSel = pNewSel;
		RefreshDetailWindow();
	}NxCatchAll("Error in CEMRItemAdvMultiPopupDlg::OnLeftClickDetailList()");
}

// (a.walling 2010-03-25 08:17) - PLID 37802 - Pass in bRemovingItem if the item is being removed (which occurs when editing or updating the item for example)
void CEMRItemAdvMultiPopupDlg::HandleDetailChange(CEMNDetail *pDetail, BOOL bRemovingItem)
{
	//TES 1/23/2008 - PLID 24157 - If we don't have a window yet, we don't need to do anything.
	if(GetSafeHwnd()) {
		//TES 1/23/2008 - PLID 24157 - Is the detail in our list?
		IRowSettingsPtr pRow = m_pDetailList->GetFirstRow();
		while(pRow) {
			CEMNDetail *pRowDetail = (CEMNDetail*)VarLong(pRow->GetValue(dlcDetail));
			if(pRowDetail == pDetail) {
				CEMRItemAdvPopupWnd *pWnd = (CEMRItemAdvPopupWnd*)VarLong(pRow->GetValue(dlcWindow));
				if(pWnd) {
					//TES 1/23/2008 - PLID 24157 - Found a valid window, update its display.
					// (a.walling 2010-03-25 08:18) - PLID 37802 - If we are 'removing' the item, call CreateControls to ensure everything is updated
					if (bRemovingItem) {
						// (a.walling 2010-10-25 15:51) - PLID 41093 - Calc the combo sql
						pDetail->CalcComboSql();
						pWnd->CreateControls();
					}
					pWnd->ReflectCurrentState();
				}
			}
			//TES 6/3/2008 - PLID 29098 - Even if the detail isn't in our list, if we have a narrative, it needs to be
			// updated to reflect the change.
			else if(pRowDetail->m_EMRInfoType == eitNarrative) {
				CEMRItemAdvPopupWnd *pWnd = (CEMRItemAdvPopupWnd*)VarLong(pRow->GetValue(dlcWindow));
				if(pWnd) {
					pWnd->UpdateRichTextAppearance();
				}
			}

			pRow = pRow->GetNextRow();
		}
	}
}

void CEMRItemAdvMultiPopupDlg::SetPoppedUpDetails(DetailPopup *pDetails)
{
	if(m_pDetailsToPopup) {
		//TES 1/23/2008 - PLID 28673 - This shouldn't happen!  We can't have two detail popup trees!
		AfxThrowNxException("CEMRItemAdvMultiPopupDlg::SetPoppedUpDetails() called on non-empty dialog");
	}

	if(GetSafeHwnd()) {
		//TES 1/23/2008 - PLID 28673 - This shouldn't happen!  We don't plan on having to update our datalist,
		// and any windows, all that sort of thing.
		AfxThrowNxException("CEMRItemAdvMultiPopupDlg::SetPoppedUpDetails() called on visible dialog");
	}

	//TES 1/23/2008 - PLID 28673 - OK, set our tree.
	m_pDetailsToPopup = pDetails;

	//TES 1/23/2008 - PLID 28673 - Now, check for any details that have been deleted, and take them out of the tree.
	// I'd like it if there was a better algorithm for this, but I haven't managed to come up with one yet.
	BOOL bOneRemoved = TRUE;
	CEMN* pEMN = NULL;
	CArray<CEMNDetail*,CEMNDetail*> arAllDetails;
	while(bOneRemoved) {
		bOneRemoved = FALSE;
		DetailPopup *pDp = m_pDetailsToPopup;
		while(pDp && !bOneRemoved) {
			if(!pEMN) {
				pEMN = pDp->m_pDetail->m_pParentTopic->GetParentEMN();
				pEMN->GenerateTotalEMNDetailArray(&arAllDetails);
			}
			bool bMatched = false;
			for(int i = 0; i < arAllDetails.GetSize() && !bMatched; i++) {
				if(pDp->m_pDetail == arAllDetails[i]) {
					bMatched = true;
				}
			}
			if(!bMatched) {
				//TES 1/28/2008 - PLID 28673 - This isn't in the EMN, therefore it must have been deleted.  Remove
				// it from our tree.
				RemoveDetail(pDp->m_pDetail);
				bOneRemoved = TRUE;
			}
			else {
				pDp = pDp->GetNextInTree();
			}
		}
	}

}


DetailPopup* CEMRItemAdvMultiPopupDlg::TakeDetailsToPopup()
{
	//TES 1/28/2008 - PLID 28673 - The original intent of this was to transfer responsibility for cleaning up the
	// memory for this tree, however the responsibility is now always the caller's.  If we change that, however, which
	// we might for optimization purposes, this would be the place where this dialog would give up responsibility.  As
	// it is, we just return the pointer.
	return m_pDetailsToPopup;
}

BOOL CEMRItemAdvMultiPopupDlg::DestroyWindow() 
{
	//TES 1/28/2008 - PLID 28673 - We need to destroy any CEMRItemAdvPopupWnds that were created; the rest of
	// the information in our tree we leave alone, as it may be used later to recreate this dialog, but in such a
	// case, we will want to recreate all the CEMRItemAdvPopupWnds.
	try {
		DetailPopup *pDp = m_pDetailsToPopup;
		while(pDp) {
			CWnd *pWnd = pDp->m_pWindow;
			if(pWnd) {
				if(pWnd->GetSafeHwnd()) {
					pWnd->DestroyWindow();
				}
				delete pWnd;
			}
			pDp->m_pWindow = NULL;
			
			pDp = pDp->GetNextInTree();
		}
	
	}NxCatchAll("Error in CEMRItemAdvMultiPopupDlg::DestroyWindow()");

	return CDialog::DestroyWindow();
}

LRESULT CEMRItemAdvMultiPopupDlg::OnPopupResized(WPARAM wParam, LPARAM lParam)
{
	//TES 2/21/2008 - PLID 28827 - Go through and reposition all our controls around the new position of the detail's
	// popupwnd.  This calculation needs to be kept in sync with the calculations in CalcMaxSize() below.
	
	//TES 2/21/2008 - PLID 28827 - Get controls window's size (if we're not currently showing a control window, 
	// then we will keep whatever the last size we used was.
	CEMRItemAdvPopupWnd *pWnd = (CEMRItemAdvPopupWnd*)wParam;
	if(pWnd) {
		pWnd->GetWindowRect(&m_rcPopupWnd);
		ScreenToClient(&m_rcPopupWnd);
	}
	
	//TES 2/21/2008 - PLID 28827 - Determine the size of the detail list (whether or not its visible).
	CRect rcList;
	GetDlgItem(IDC_DETAIL_LIST)->GetWindowRect(&rcList);
	ScreenToClient(&rcList);

	//TES 2/21/2008 - PLID 28827 - Now offset the popupwnd depending whether we're showing the list.
	int nLeft = 20; //10 pixels from window's edge to the NxColor, 10 more pixels to the list.
	if(m_bShowList) {
		nLeft += rcList.Width() + 10;
	}
	m_rcPopupWnd.OffsetRect(nLeft - m_rcPopupWnd.left, 0);
	if(pWnd) pWnd->MoveWindow(&m_rcPopupWnd);

	//TES 2/21/2008 - PLID 28827 - Next, center the "Back To EMN" button (usually hidden) behind that window.
	// (a.walling 2010-01-14 14:50) - PLID 31118
	/*
	CRect rcBackToEmn;
	m_nxbBackToEmn.GetWindowRect(&rcBackToEmn);
	ScreenToClient(&rcBackToEmn);
	m_nxbBackToEmn.MoveWindow(m_rcPopupWnd.left+(m_rcPopupWnd.Width()/2)-(rcBackToEmn.Width()/2), m_rcPopupWnd.top+m_rcPopupWnd.Height()/2-rcBackToEmn.Height()/2, rcBackToEmn.Width(), rcBackToEmn.Height());
	*/
	//TES 2/21/2008 - PLID 28827 - Now, put the Next button 10 pixels below the popupwnd, aligned right.
	CRect rcNext;
	m_nxbNext.GetWindowRect(&rcNext);
	ScreenToClient(&rcNext);
	CRect rc(m_rcPopupWnd.right-rcNext.Width(), m_rcPopupWnd.bottom+10, m_rcPopupWnd.right, m_rcPopupWnd.bottom+10+rcNext.Height());
	m_nxbNext.MoveWindow(&rc);
	m_nxbNext.RedrawWindow();
	rcNext = rc;

	// (a.walling 2010-01-14 14:50) - PLID 31118
	m_nxbBackToEmn.MoveWindow(&rc);
	m_nxbBackToEmn.RedrawWindow();

	//TES 2/21/2008 - PLID 28827 - Now, put the Previous button 10 pixels below the popupwnd, aligned left.
	CRect rcPrevious;
	m_nxbPrevious.GetWindowRect(&rcPrevious);
	ScreenToClient(&rcPrevious);
	rc.SetRect(m_rcPopupWnd.left, m_rcPopupWnd.bottom+10, m_rcPopupWnd.left+rcPrevious.Width(), m_rcPopupWnd.bottom+10+rcPrevious.Height());
	m_nxbPrevious.MoveWindow(rc);
	m_nxbPrevious.RedrawWindow();

	//TES 2/21/2008 - PLID 28827 - Set the detail list to the same height as the popup wnd (its horizontal position 
	// won't change).  Then make sure its visibility is correct.
	rc.SetRect(rcList.left, rcList.top, rcList.right, m_rcPopupWnd.bottom);
	GetDlgItem(IDC_DETAIL_LIST)->MoveWindow(&rc);
	if(m_bShowList) {
		//TES 3/4/2008 - PLID 28827 - We need to only do this if the window isn't already visible.  The reason is that
		// if we do call ShowWindow(SW_SHOW) and it's already visible, then sometimes (I don't know why or when), the 
		// COleControl code will decide to resize the list to a different size.  Isn't COM programming fun?
		if(!GetDlgItem(IDC_DETAIL_LIST)->IsWindowVisible()) {
			GetDlgItem(IDC_DETAIL_LIST)->EnableWindow(TRUE);
			GetDlgItem(IDC_DETAIL_LIST)->ShowWindow(SW_SHOW);
			//TES 2/21/2008 - PLID 28827 - Don't let it get hidden behind the NxColor.
			GetDlgItem(IDC_DETAIL_LIST)->BringWindowToTop();
		}
	}
	else {
		GetDlgItem(IDC_DETAIL_LIST)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_DETAIL_LIST)->EnableWindow(FALSE);
	}

	//TES 2/21/2008 - PLID 28827 - Now, size the color to 10 pixels outside the Next button (the upper left corner will
	// stay the same).
	CRect rcColor;
	m_nxc.GetWindowRect(&rcColor);
	ScreenToClient(&rcColor);
	rc.SetRect(rcColor.left,rcColor.top,rcNext.right+10, rcNext.bottom+10);
	m_nxc.MoveWindow(&rc);
	rcColor = rc;

	//TES 2/21/2008 - PLID 28827 - Now, put the OK (actually, "Back To EMN") button 10 pixels above the popup wnd 
	// and 10 pixels below the top of the NxColor, aligned right.
	CRect rcOK;
	m_nxbOK.GetWindowRect(&rcOK);
	ScreenToClient(&rcOK);
	rc.SetRect(m_rcPopupWnd.right-rcOK.Width(), rcColor.top + 10, m_rcPopupWnd.right, m_rcPopupWnd.top - 10);
	m_nxbOK.MoveWindow(&rc);

	//TES 2/25/2008 - PLID 28827 - Same with the Show/Hide List button, aligned left.
	CRect rcShowList;
	m_nxbShowList.GetWindowRect(&rcShowList);
	ScreenToClient(&rcShowList);
	rc.SetRect(m_rcPopupWnd.left, rcColor.top + 10, m_rcPopupWnd.left + rcShowList.Width(), m_rcPopupWnd.top - 10);
	m_nxbShowList.MoveWindow(&rc);

	//TES 2/21/2008 - PLID 28827 - Now, adjust the entire window to 10 pixels bigger than the nxcolor.
	CRect rcWindow;
	GetWindowRect(&rcWindow);
	ClientToScreen(&rcColor);
	rc.SetRect(rcWindow.left, rcWindow.top, rcColor.right+10, rcColor.bottom+10);

	//TES 2/21/2008 - PLID 28827 - Finally, center the window in our work area.
	CRect rcDesktop;
	SystemParametersInfo(SPI_GETWORKAREA, 0, rcDesktop, 0);
	long nScreenCenterX = rcDesktop.Width()/2;
	long nScreenCenterY = rcDesktop.Height()/2;
	MoveWindow(nScreenCenterX - (rc.Width()/2), nScreenCenterY - (rc.Height()/2), rc.Width(), rc.Height());

	//All done!
	return 0;
}

CSize CEMRItemAdvMultiPopupDlg::CalcMaxSize()
{
	//TES 2/21/2008 - PLID 28827 - This calculation is basically the OnPopupResized() calculation in reverse, so make
	// sure to keep them in sync.  We want to determine the size of the popupwnd that will result in our dialog 
	// being the size of the entire work area.  So, start with that size, and work our way backward.
	CRect rc;
	SystemParametersInfo(SPI_GETWORKAREA, 0, rc, 0);
	long nScreenSizeX = rc.Width();
	long nScreenSizeY = rc.Height();

	//TES 2/21/2008 - PLID 28827 - Adjust off the window border
	CSize szBorder;
	CalcWindowBorderSize(this, &szBorder);
	nScreenSizeX -= szBorder.cx;
	nScreenSizeY -= szBorder.cy;

	//TES 2/21/2008 - PLID 28827 - Subtract 10 pixels on each side for the border around the NxColor.
	nScreenSizeX -= 20;
	nScreenSizeY -= 20;

	//TES 2/21/2008 - PLID 28827 - Subtract 10 pixels more for the border between the NxColor and the controls.
	nScreenSizeX -= 20;
	nScreenSizeY -= 20;

	//TES 2/21/2008 - PLID 28827 - Subtract the width of the list, if it's visible.
	if(m_bShowList) {
		CRect rcList;
		GetDlgItem(IDC_DETAIL_LIST)->GetWindowRect(&rcList);
		ScreenToClient(&rcList);
		nScreenSizeX -= rcList.Width();
	}

	//TES 2/21/2008 - PLID 28827 - Subtract 10 pixels for the border between the list and the popupwnd.
	nScreenSizeX -= 10;

	//TES 2/21/2008 - PLID 28827 - Subtract the height of the OK button.
	CRect rcOK;
	GetDlgItem(IDOK)->GetWindowRect(&rcOK);
	ScreenToClient(&rcOK);
	nScreenSizeY -= rcOK.Height();

	//TES 2/21/2008 - PLID 28827 - Subtract 10 pixels for the border between the OK button and the popupwnd.
	nScreenSizeY -= 10;

	//TES 2/21/2008 - PLID 28827 - Subtract the height of the Next button.
	CRect rcNext;
	m_nxbNext.GetWindowRect(&rcNext);
	ScreenToClient(&rcNext);
	nScreenSizeY -= rcNext.Height();

	//TES 2/21/2008 - PLID 28827 - Subtract 10 pixels for the border between the Next button and the popupwnd.
	nScreenSizeY -= 10;

	//TES 2/21/2008 - PLID 28827 - We now have the maximum size for the popupwnd that will keep us on the screen.
	return CSize(nScreenSizeX, nScreenSizeY);
}

void CEMRItemAdvMultiPopupDlg::RefreshTitle()
{
	//TES 2/21/2008 - PLID 28827 - If we're on a detail, give the name of the detail and its position in the list.
	IRowSettingsPtr pRow = m_pDetailList->CurSel;
	if(pRow) {
		CEMNDetail *pRowDetail = (CEMNDetail*)VarLong(pRow->GetValue(dlcDetail));
		CString strTitle;
		// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to use the new index-less method instead of the index
		strTitle.Format("Detail %li of %li - %s", pRow->CalcRowNumber()+1, m_pDetailList->GetRowCount(), pRowDetail->GetLabelText());
		SetWindowText(strTitle);
	}
	else {
		//TES 2/21/2008 - PLID 28827 - No detail, just use the default text.
		SetWindowText("EMR Popups");
	}
	
}

void CEMRItemAdvMultiPopupDlg::OnShowList() 
{
	try {
		//TES 2/21/2008 - PLID 28827 - Update our flag.
		m_bShowList = !m_bShowList;
		//TES 2/21/2008 - PLID 28827 - Update the button text.
		if(m_bShowList) {
			m_nxbShowList.SetWindowText(">> Hide List");
		}
		else {
			m_nxbShowList.SetWindowText("<< Show List");
		}

		//TES 2/21/2008 - PLID 28827 - Go through each row, tell any popupwnds that their MaxSize has changed, and
		// remind ourselves that we need to force them to reposition their controls next time they're shown.
		IRowSettingsPtr pRow = m_pDetailList->GetFirstRow();
		CSize szNewMax = CalcMaxSize();
		while(pRow) {
			CEMRItemAdvPopupWnd* pWnd = (CEMRItemAdvPopupWnd*)VarLong(pRow->GetValue(dlcWindow));
			if(pWnd) {
				pWnd->SetMaxSize(szNewMax);
				pRow->PutValue(dlcNeedReposition, (long)1);
			}
			pRow = pRow->GetNextRow();
		}
		//TES 3/11/2008 - PLID 28827 - ALSO, remember that we need to reposition on the Back To EMN screen.
		// (a.walling 2010-01-14 16:09) - PLID 31118 - Deprecated
		//m_bNeedRepositionBackToEmn = true;

		//TES 2/21/2008 - PLID 28827 - Remember this preference.
		SetRemotePropertyInt("EMR_MultiPopup_ShowList", m_bShowList?1:0, 0, GetCurrentUserName());

		//TES 2/21/2008 - PLID 28827 - Update the currently displayed detail.
		RefreshDetailWindow();
	}NxCatchAll("Error in CEMRItemAdvMultiPopupDlg::OnShowList()");

}

LRESULT CEMRItemAdvMultiPopupDlg::OnEmrMinimizePic(WPARAM wParam, LPARAM lParam)
{
	try
	{
		// (z.manning 2008-11-04 09:36) - PLID 31904 - If we got a message to minimize the PIC
		// then we need to end this dialog as it's likely modal to the PicContainerDlg.
		EndDialog(IDOK);
		if(GetParent() != NULL) {
			GetParent()->SendMessage(NXM_EMR_MINIMIZE_PIC);
		}
		GetMainFrame()->SetForegroundWindow();

	}NxCatchAll("CEMRItemAdvMultiPopupDlg::OnEmrMinimizePic");
	return S_OK;
}

// (a.walling 2008-12-19 09:29) - PLID 29800 - Refresh custom stamps for images in the multipopup tree (excluding pIgnore)
void CEMRItemAdvMultiPopupDlg::RefreshCustomStamps(LPCTSTR strNewStamps, CWnd* pIgnore)
{
	DetailPopup* pDetail = m_pDetailsToPopup;
	while(pDetail) {
		if (pDetail->m_pWindow && pDetail->m_pWindow != pIgnore) {
			pDetail->m_pWindow->SetCustomStamps(strNewStamps);
			pDetail->m_pWindow->CallRepositionControls();
		}
		
		pDetail = pDetail->GetNextInTree();
	}
}