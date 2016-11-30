// EMREMChecklistApprovalDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EMREMChecklistApprovalDlg.h"
#include "EMNDetail.h"
#include "EMRTopic.h"
#include "EMN.h"

// (j.jones 2007-09-27 16:39) - PLID 27547 - created

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEMREMChecklistApprovalDlg dialog

using namespace NXDATALIST2Lib;

enum EMCategoryListColumn {

	emclcCategoryID = 0,
	emclcCategoryName,
	emclcRequired,
	emclcFound,
};

enum EMContributingDetailListColumn {

	emcdlcItemName = 0,
	emcdlcItemData,
	emcdlcCategoryID,
	emcdlcCategoryName,
	emcdlcNumElements,
};

CEMREMChecklistApprovalDlg::CEMREMChecklistApprovalDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMREMChecklistApprovalDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEMREMChecklistApprovalDlg)
		// (b.spivey, February 24, 2012) - PLID 38409 - Removed the need for m_bAppproved. 
		m_pRuleInfo = NULL;
		m_aryTrackedCategories = NULL;
		m_bIsReadOnly = FALSE;
	//}}AFX_DATA_INIT
}


void CEMREMChecklistApprovalDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMREMChecklistApprovalDlg)
	DDX_Control(pDX, IDC_CODING_LEVEL_DESC, m_nxstaticCodingLevelDesc);
	DDX_Control(pDX, IDC_RULE_DESC, m_nxstaticRuleDesc);
	DDX_Control(pDX, IDC_CATEGORY_LIST_LABEL, m_nxstaticCategoryListLabel);
	DDX_Control(pDX, IDC_NO_CATEGORIES_LABEL, m_nxstaticNoCategoriesLabel);
	DDX_Control(pDX, IDC_DETAIL_LIST_LABEL, m_nxstaticDetailListLabel);
	DDX_Control(pDX, IDC_RULE_PASSED_LABEL, m_nxstaticRulePassedLabel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEMREMChecklistApprovalDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMREMChecklistApprovalDlg)
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMREMChecklistApprovalDlg message handlers

BOOL CEMREMChecklistApprovalDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		// (c.haag 2008-04-25 17:16) - PLID 29796 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		if(m_pRuleInfo == NULL || m_aryTrackedCategories == NULL) {
			//this should be impossible
			ASSERT(FALSE);
			// (b.spivey, February 24, 2012) - PLID 38409 - Removed the need for m_bApproved.
			CDialog::OnCancel();
			return TRUE;
		}

		m_CategoryList = BindNxDataList2Ctrl(this, IDC_EM_CATEGORY_LIST, GetRemoteData(), false);
		m_DetailList = BindNxDataList2Ctrl(this, IDC_EM_CONTRIBUTING_DETAIL_LIST, GetRemoteData(), false);

		// (j.jones 2013-04-23 16:30) - PLID 56372 - added a read only status, which disables the approve button,
		// also makes the lists gray so it is clearly read only
		if(m_bIsReadOnly) {
			m_CategoryList->PutReadOnly(VARIANT_TRUE);
			m_DetailList->PutReadOnly(VARIANT_TRUE);
			m_btnOK.EnableWindow(FALSE);
		}

		Load();

	}NxCatchAll("Error in CEMREMChecklistApprovalDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEMREMChecklistApprovalDlg::Load()
{
	try {

		//grab info for our labels
		CString strCodingLevelDesc = m_pRuleInfo->pRowInfo->strDescription;
		CString strRuleDesc = m_pRuleInfo->strDescription;

		//replace newlines with spaces
		strCodingLevelDesc.Replace("\r\n", "  ");
		strRuleDesc.Replace("\r\n", "  ");

		//replace & with &&
		strCodingLevelDesc.Replace("&", "&&");
		strRuleDesc.Replace("&", "&&");

		//now set the labels
		SetDlgItemText(IDC_CODING_LEVEL_DESC, strCodingLevelDesc);
		SetDlgItemText(IDC_RULE_DESC, strRuleDesc);

		// (b.spivey, February 24, 2012) - PLID 38409 - Removed the checkbox, you either approve or you don't. 

		//if there are no categories, then we need to resize the screen
		if(m_pRuleInfo->paryDetails.GetSize() == 0) {

			//get the reference rectangles
			CRect rcCategoryListRect;
			GetDlgItem(IDC_EM_CATEGORY_LIST)->GetWindowRect(&rcCategoryListRect);
			ScreenToClient(&rcCategoryListRect);
			// (b.spivey, February 24, 2012) - PLID 38409 - Removed the checkbox
			CRect rcOK;
			GetDlgItem(IDOK)->GetWindowRect(&rcOK);
			ScreenToClient(&rcOK);
			CRect rcCancel;
			GetDlgItem(IDCANCEL)->GetWindowRect(&rcCancel);
			ScreenToClient(&rcCancel);
			
			// (b.spivey, February 24, 2012) - PLID 38409 -Removed the checkbox
			long nOKMoved = rcOK.bottom - rcCategoryListRect.bottom;

			//hide unneeded controls
			GetDlgItem(IDC_CATEGORY_LIST_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_EM_CATEGORY_LIST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_DETAIL_LIST_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_EM_CONTRIBUTING_DETAIL_LIST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_RULE_PASSED_LABEL)->ShowWindow(SW_HIDE);

			//move the approved checkbox			
			// (b.spivey, February 24, 2012) - PLID 38409 - Removed the checkbox

			//move the OK button
			CRect rcMoveOKRect(rcOK.left, rcCategoryListRect.bottom - rcOK.Height(), rcOK.right, rcCategoryListRect.bottom);
			GetDlgItem(IDOK)->MoveWindow(rcMoveOKRect);

			//move the Cancel button
			CRect rcMoveCancelRect(rcCancel.left, rcCancel.top - nOKMoved, rcCancel.right, rcCancel.bottom - nOKMoved);
			GetDlgItem(IDCANCEL)->MoveWindow(rcMoveCancelRect);

			//resize the window
			CRect rcWindowRect;
			GetWindowRect(rcWindowRect);
			rcWindowRect.bottom -= nOKMoved;
			MoveWindow(rcWindowRect);
			return;
		}

		//otherwise, we have categories, so hide the label and continue loading
		GetDlgItem(IDC_NO_CATEGORIES_LABEL)->ShowWindow(SW_HIDE);

		//change the category label to indicate if all or any categories are required
		if(m_pRuleInfo->bRequireAllDetails) {
			SetDlgItemText(IDC_CATEGORY_LIST_LABEL, "All of the following categories are required to be complete for this rule:");
		}
		else {
			SetDlgItemText(IDC_CATEGORY_LIST_LABEL, "At least one of the following categories are required to be complete for this rule:");
		}
		
		//indicate whether Practice has determined this rule has passed
		// (j.jones 2013-01-04 14:48) - PLID 28135 - changed to say E/M, and not use an ampersand
		if(m_pRuleInfo->bPassed) {
			SetDlgItemText(IDC_RULE_PASSED_LABEL, "This rule has been satisfied by E/M elements on this EMN.");
		}
		else {
			SetDlgItemText(IDC_RULE_PASSED_LABEL, "This rule has not been satisfied by E/M elements on this EMN.");
		}

		//now fill the category list
		for(int i=0; i<m_pRuleInfo->paryDetails.GetSize(); i++) {
			ChecklistElementRuleDetailInfo *pRuleDetail = (ChecklistElementRuleDetailInfo*)(m_pRuleInfo->paryDetails.GetAt(i));			

			IRowSettingsPtr pCatRow = m_CategoryList->GetNewRow();
			pCatRow->PutValue(emclcCategoryID, (long)pRuleDetail->nCategoryID);
			pCatRow->PutValue(emclcCategoryName, _bstr_t(pRuleDetail->strCategoryName));
			pCatRow->PutValue(emclcRequired, (long)pRuleDetail->nMinElements);

			//now look up this category in the detail list to see how many were found
			BOOL bFound = FALSE;
			for(int j=0; j<m_aryTrackedCategories->GetSize() && !bFound; j++) {
				ChecklistTrackedCategoryInfo *pCatInfo = (ChecklistTrackedCategoryInfo*)(m_aryTrackedCategories->GetAt(j));
				if(pCatInfo != NULL && pCatInfo->nCategoryID == pRuleDetail->nCategoryID) {
					//found it
					bFound = TRUE;					
					pCatRow->PutValue(emclcFound, (long)pCatInfo->nTotalElementsFound);

					//now add its details to the "contributing detail" list
					for(int k=0; k<pCatInfo->aryDetailInfo.GetSize(); k++) {
						ChecklistTrackedCategoryDetailInfo *pDetailInfo = (ChecklistTrackedCategoryDetailInfo*)(pCatInfo->aryDetailInfo.GetAt(k));
						if(pDetailInfo != NULL && pDetailInfo->pDetail != NULL) {

							IRowSettingsPtr pDetailRow = m_DetailList->GetNewRow();
							pDetailRow->PutValue(emcdlcItemName, _bstr_t(pDetailInfo->pDetail->GetMergeFieldName(FALSE)));

							CString strItemData = "";
							//replicated the way we calculate tooltip text
							if(pDetailInfo->pDetail->GetStateVarType() == VT_NULL || pDetailInfo->pDetail->GetStateVarType() == VT_BSTR && VarString(pDetailInfo->pDetail->GetState()).IsEmpty()) {
								//if no data is entered, pull the default sentence format, and do not replace any of the fields
								pDetailInfo->pDetail->LoadContent(); //will do nothing if already loaded
								if(pDetailInfo->pDetail->m_EMRInfoType == eitImage)
									strItemData = "";
								else
									//TES 2/26/2010 - PLID 37463 - Updated to pull the Smart Stamp long form, but I don't believe this code
									// can ever be hit, as a detail with a blank state can't satisfy a checklist rule.
									// (z.manning 2010-07-26 15:17) - PLID 39848 - All tables use the same long form now
									strItemData = pDetailInfo->pDetail->m_strLongForm;
							}
							else {
								//otherwise call GetSentence() which will return a properly formatted sentence, based on the data selected
								strItemData = pDetailInfo->pDetail->m_pParentTopic->GetParentEMN()->GetSentence(pDetailInfo->pDetail, NULL, false, false);
							}

							pDetailRow->PutValue(emcdlcItemData, _bstr_t(strItemData));
							pDetailRow->PutValue(emcdlcCategoryID, (long)pCatInfo->nCategoryID);
							pDetailRow->PutValue(emcdlcCategoryName, _bstr_t(pRuleDetail->strCategoryName));
							pDetailRow->PutValue(emcdlcNumElements, (long)pDetailInfo->nElementsFound);

							m_DetailList->AddRowAtEnd(pDetailRow, NULL);
						}
					}
				}
			}

			//if none found, be sure to set the value to zero
			if(!bFound)
				pCatRow->PutValue(emclcFound, (long)0);

			m_CategoryList->AddRowAtEnd(pCatRow, NULL);
		}

	}NxCatchAll("Error in CEMREMChecklistApprovalDlg::Load");
}

void CEMREMChecklistApprovalDlg::OnOK() 
{
	try {
	// (b.spivey, February 24, 2012) - PLID 38409 - Removed the checkbox and extra messageboxes. This should be considered
	//	"OnApprove" because that's what happens when you click OK. 
	
		CDialog::OnOK();

	}NxCatchAll("Error in CEMREMChecklistApprovalDlg::OnOK");
}

HBRUSH CEMREMChecklistApprovalDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	if(pWnd->GetDlgCtrlID() == IDC_RULE_PASSED_LABEL) {

		if(m_pRuleInfo->bPassed)
			pDC->SetTextColor(RGB(0,124,0));
		else
			pDC->SetTextColor(RGB(174,0,0));	//this maroon color works in 256-color TS sessions
	}

	return hbr;
}
