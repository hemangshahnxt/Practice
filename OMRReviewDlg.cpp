// OMRReviewDlg.cpp : implementation file
//

// (j.dinatale 2012-08-02 09:06) - PLID 51911 - Created

#include "stdafx.h"
#include "Practice.h"
#include "OMRReviewDlg.h"
#include "EmrOmrMap.h"
#include "EmrRc.h"
#include <NxDataUtilitiesLib/NxSafeArray.h>
#include <foreach.h>
#include "OMRReviewBrowserCtrl.h"
#include "GlobalUtils.h"
#include "SingleSelectMultiColumnDlg.h" // (b.savon 2013-02-27 11:04) - PLID 54713
#include "OMRUtils.h" // (b.savon 2013-02-28 11:47) - PLID 54714

// (j.dinatale 2012-08-02 16:11) - PLID 51911 - created

namespace OMRReviewList{
	enum OMRReviewCols{
		ID = 0,
		Name = 1,
		Selected = 2,
		DataType = 3,
	};
};

// COMRReviewDlg dialog

IMPLEMENT_DYNAMIC(COMRReviewDlg, CNxDialog)

COMRReviewDlg::COMRReviewDlg(NexTech_COM::INxXmlGeneratorPtr pNxNXL, CWnd* pParent /*=NULL*/)
	: CNxDialog(COMRReviewDlg::IDD, pParent)
{
	m_pNxNXLtoReview = pNxNXL;
	m_nDocIndex = 0;
}

COMRReviewDlg::~COMRReviewDlg()
{
}

void COMRReviewDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_OMR_REVIEW_COMMIT, m_btnCommit);
	DDX_Control(pDX, IDC_OMR_REVIEW_DONOTCOMMIT, m_btnDoNotCommit);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_OMR_REVIEW_DLG_COLOR, m_bkgrnd);
	DDX_Control(pDX, IDC_OMR_REVIEW_NEXT, m_btnNextAttach);
	DDX_Control(pDX, IDC_OMR_REVIEW_PREV, m_btnPrevAttach);
	DDX_Control(pDX, IDC_RDO_EXPANDED, m_radioExpandAll);
	DDX_Control(pDX, IDC_RDO_COLLAPSED_ALL, m_radioCollapseAll);
	DDX_Control(pDX, IDC_RDO_COLLAPSED_SUCCESS, m_radioCollapseValid);
}


BEGIN_MESSAGE_MAP(COMRReviewDlg, CNxDialog)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_OMR_REVIEW_PREV, &COMRReviewDlg::OnBnClickedOmrReviewPrev)
	ON_BN_CLICKED(IDC_OMR_REVIEW_NEXT, &COMRReviewDlg::OnBnClickedOmrReviewNext)
	ON_BN_CLICKED(IDC_OMR_REVIEW_COMMIT, &COMRReviewDlg::OnBnClickedOmrReviewCommit)
	ON_BN_CLICKED(IDC_OMR_REVIEW_DONOTCOMMIT, &COMRReviewDlg::OnBnClickedOmrReviewDonotcommit)
	ON_BN_CLICKED(IDC_RDO_COLLAPSED_SUCCESS, &COMRReviewDlg::OnBnClickedRdoCollapsedSuccess)
	ON_BN_CLICKED(IDC_RDO_COLLAPSED_ALL, &COMRReviewDlg::OnBnClickedRdoCollapsedAll)
	ON_BN_CLICKED(IDC_RDO_EXPANDED, &COMRReviewDlg::OnBnClickedRdoExpanded)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// COMRReviewDlg message handlers
BOOL COMRReviewDlg::OnInitDialog()
{ 
	CNxDialog::OnInitDialog();
	try{
		//TES 5/15/2014 - PLID 62130 - Don't let Adobe get destroyed before our embedded browser is done with it
		GetMainFrame()->HoldAdobeAcrobatReference();

		m_pCtrl.reset(new COMRReviewBrowserCtrl);
		m_pCtrl->Create(GetHTMLControlRect(), this, -1);

		m_bkgrnd.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
		m_btnCancel.AutoSet(NXB_CLOSE);
		m_btnCommit.AutoSet(NXB_OK);
		m_btnDoNotCommit.AutoSet(NXB_CANCEL);
		m_btnNextAttach.AutoSet(NXB_RIGHT);
		m_btnPrevAttach.AutoSet(NXB_LEFT);

		// (b.spivey, March 01, 2013) - PLID 54717 - Hide and disable these controls. 
		m_btnPrevAttach.ShowWindow(SW_HIDE); 
		m_btnPrevAttach.EnableWindow(FALSE);
		m_btnNextAttach.ShowWindow(SW_HIDE); 
		m_btnNextAttach.EnableWindow(FALSE);
		GetDlgItem(IDC_LBL_OMR_REVIEW_DOC_COUNT)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LBL_OMR_REVIEW_DOC_COUNT)->EnableWindow(FALSE); 

		// (b.savon 2013-03-05 14:12) - PLID 55456 - Default to Collapse Valid & Show maximized
		m_radioCollapseValid.SetCheck(BST_CHECKED);
		// (v.maida 2016-06-06 15:55) - NX-100631 - Set a larger default size so that the dialog is easier to read if it's restored down to a smaller size.
		SetWindowPos(NULL, 0, 0, 994, 738, 0);
		ShowWindow(SW_SHOWMAXIMIZED);

		if(m_pNxNXLtoReview){
			long nPatientID = m_pNxNXLtoReview->GetPatientID();
			long nTemplateID = m_pNxNXLtoReview->GetTemplateID();
			long nFormID = m_pNxNXLtoReview->GetOMRFormID(); // (b.spivey, April 24, 2013) - PLID 56438 - form ID for loading. 

			if(nPatientID > 0){
				long nUserDefinedID = GetExistingPatientUserDefinedID(nPatientID);
				CString strName = GetExistingPatientName(nPatientID);
				if(nUserDefinedID > 0){
					GetDlgItem(IDC_OMR_REVIEW_USERDEFINED_LBL)->SetWindowText(AsString(nUserDefinedID));
				}
				

				if(!strName.IsEmpty()){
					GetDlgItem(IDC_OMR_REVIEW_PAT_LBL)->SetWindowText(strName);
					SetWindowText("OMR Review for " + strName);
				}
			}

			if(nTemplateID > 0){
				ADODB::_RecordsetPtr rsTemplateInfo = CreateParamRecordset(
					"SELECT EMRTemplateT.Name AS Name FROM EMRTemplateT WHERE ID = {INT}", nTemplateID);

				if(!rsTemplateInfo->eof){
					CString strName = AdoFldString(rsTemplateInfo, "Name", "");
					if(!strName.IsEmpty()){
						GetDlgItem(IDC_OMR_REVIEW_TEMPLATE_LBL)->SetWindowText(strName);
					}
				}

				m_pDetailList = BindNxDataList2Ctrl(IDC_OMR_PEND_COMMIT_DETAILS, false);

				if(m_pDetailList){
					// (b.savon 2013-02-28 16:26) - PLID 54714 - Set the columns to word wrap so we can actually read the details
					NXDATALIST2Lib::IFormatSettingsPtr pWordWrap(__uuidof(NXDATALIST2Lib::FormatSettings));
					pWordWrap->PutFieldType(NXDATALIST2Lib::cftTextWordWrap);

					EmrOmrMap OmrMap;
					// (b.spivey, April 24, 2013) - PLID 56438 - load from form now. 
					OmrMap.LoadEmrOmrMap(nFormID, EmrOmrMap::ltsForm);

					Nx::SafeArray<long> saryDataGroupIDs(m_pNxNXLtoReview->GetDataGroupIDs());
					CMap<long, long, NXDATALIST2Lib::IRowSettingsPtr, NXDATALIST2Lib::IRowSettingsPtr> dlItemToParentRows;

					for(int i = 0; i < OmrMap.GetItemCount(); i++){
						EmrOmrMap::EmrItemDetail emdDetail = OmrMap.GetItemAt(i);

						NXDATALIST2Lib::IRowSettingsPtr pParentRow = NULL;
						if(!dlItemToParentRows.Lookup(emdDetail.ItemID, pParentRow)){
							// create new parent row and add it to the map
							pParentRow = m_pDetailList->GetNewRow();
							pParentRow->PutValue(OMRReviewList::ID, emdDetail.ItemID);
							pParentRow->PutValue(OMRReviewList::Name, _bstr_t(emdDetail.ItemName));
							//So we know how to validate. 
							pParentRow->PutValue(OMRReviewList::DataType, emdDetail.DataType);
							// (b.savon 2013-02-28 16:27) - PLID 54714 - Wrap the text
							pParentRow->PutRefCellFormatOverride(OMRReviewList::Name, pWordWrap);

							// (b.savon 2013-03-06 10:19) - PLID 55467 - Since they are already sorted in the object map of
							// how they appear on the template, just add it to the datalist.
							m_pDetailList->AddRowAtEnd(pParentRow, NULL);
							dlItemToParentRows.SetAt(emdDetail.ItemID, pParentRow);
						}

						// (b.spivey, April 24, 2013) - PLID 56438 - skip unampped selections. 
						if (emdDetail.OmrID == -1) {
							continue; 
						}


						NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDetailList->GetNewRow();
						pRow->PutValue(OMRReviewList::ID, emdDetail.DataGroupID);
						pRow->PutValue(OMRReviewList::Name,  _bstr_t(emdDetail.SelectionText));
						// (b.savon 2013-02-28 13:02) - PLID 54714 - So we dont get a scroll bar and can read the text and color
						// alternating rows.
						pRow->PutRefCellFormatOverride(OMRReviewList::Name, pWordWrap);
						pRow->PutBackColor(OMRUtils::GetOMRDetailRowColor(i));

						// (b.spivey, August 31, 2012) - PLID 52286 - BOOLs get handled like ints/longs, we need a bool!
						bool bSelected = false;

						foreach(long nDataGroupID, saryDataGroupIDs){
							if(emdDetail.DataGroupID == nDataGroupID){
								bSelected = true;
								break;
							}
						}

						pRow->PutValue(OMRReviewList::Selected,  bSelected);
						// (b.savon 2013-03-06 10:19) - PLID 55467 - Since they are already sorted in the object map of
						// how they appear on the template, just add it to the datalist.
						m_pDetailList->AddRowAtEnd(pRow, pParentRow);
						pParentRow->PutExpanded(TRUE);
					}
				}
			}

			// (b.spivey, August 29, 2012) - PLID 52286 - warning for when you commit, validate selections. 
			m_bMultiSelectWarning = false; 
			ValidateSelections();

			Nx::SafeArray<BSTR> aryFiles(m_pNxNXLtoReview->GetFiles());

			// (j.fouts 2012-12-14 10:18) - PLID 54183 - Sort the documents using an alpha numeric sort
			std::vector<CString> arySortedDocs;
			aryFiles.To(std::back_inserter(arySortedDocs));
			
			std::sort(arySortedDocs.begin(), arySortedDocs.end(), &CompareAlphaNumeric);
			
			foreach(CString strValue, arySortedDocs)
			{
				m_aryScannedDocs.Add(strValue);
			}

			if(!aryFiles.GetCount()){
				GetDlgItem(IDC_LBL_OMR_REVIEW_DOC_COUNT)->ShowWindow(SW_HIDE);
			}
		}

		if(m_pDetailList && m_pCtrl && m_pCtrl->GetSafeHwnd()){
			ChangeZOrder(m_pCtrl.get(), GetDlgItem(IDC_OMR_PEND_COMMIT_DETAILS));
		}

		// (b.spivey, April 24, 2013) - PLID 56438 - Clear out unmapped items. 
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDetailList->GetFirstRow();
		while (pRow) {
			if(!pRow->GetFirstChildRow()) {
				NXDATALIST2Lib::IRowSettingsPtr pRemoveRow = pRow;

				pRow = pRow->GetNextRow();

				m_pDetailList->RemoveRow(pRemoveRow); 
			} 
			else {
				pRow = pRow->GetNextRow();
			} 
		}

		if(m_aryScannedDocs.GetCount()){
			LoadScannedDoc(0);
		}else{
			m_pCtrl->LoadUrl("about:blank");
		}

		// (b.spivey, April 25, 2013) - PLID 56438 - If we have no details, don't try to commit this. 
		if(!m_pDetailList->GetFirstRow()) {
			AfxMessageBox("Warning! The form you just loaded has no mapped fields or items! You may want to check your settings and "
				"see if the form you were expecting still exists. Practice cannot commit this OMR form. Click 'Do Not Commit' to delete "
				"this pending OMR if you believe the OMR Form was deleted. ", MB_OK|MB_ICONWARNING); 

			m_btnCommit.EnableWindow(FALSE); 
		}

		m_btnNextAttach.EnableWindow(m_pCtrl && m_pCtrl->GetSafeHwnd() && (m_aryScannedDocs.GetCount() > 1));
		m_btnPrevAttach.EnableWindow(FALSE);
	}NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COMRReviewDlg::LoadScannedDoc(long nIndex)
{
	if(nIndex < 0 || nIndex >= m_aryScannedDocs.GetCount()){
		return;
	}

	if(!m_pCtrl || !m_pCtrl->GetSafeHwnd()){
		return;
	}

	CString strBrowserFilename = m_aryScannedDocs[nIndex];
	/*
	CString strLabel;
	strLabel.Format("%li of %li document(s)", nIndex + 1, m_aryScannedDocs.GetCount());
	GetDlgItem(IDC_LBL_OMR_REVIEW_DOC_COUNT)->SetWindowText(strLabel);
	GetDlgItem(IDC_LBL_OMR_REVIEW_DOC_COUNT)->RedrawWindow();
	*/

	// (j.fouts 2012-12-13 15:49) - PLID 54184 - Just set the image file rather than loading a new html doc
	// (b.spivey, February 27, 2013) - PLID 54717 - Displaying PDFs only now. 
	m_pCtrl->LoadUrl(strBrowserFilename); 
	//m_pCtrl-> //SetImageUrl(strBrowserFilename);

}

void COMRReviewDlg::OnBnClickedOmrReviewPrev()
{
	try{
		/*
		LoadScannedDoc(m_nDocIndex - 1);
		m_nDocIndex--;
		m_btnNextAttach.EnableWindow(m_nDocIndex < m_aryScannedDocs.GetCount());
		m_btnPrevAttach.EnableWindow(m_nDocIndex > 0);
		*/
	}NxCatchAll(__FUNCTION__);
}

void COMRReviewDlg::OnBnClickedOmrReviewNext()
{
	try{
		/*
		LoadScannedDoc(m_nDocIndex + 1);
		m_nDocIndex++;
		m_btnNextAttach.EnableWindow((m_nDocIndex + 1) < m_aryScannedDocs.GetCount());
		m_btnPrevAttach.EnableWindow(m_nDocIndex > 0);
		*/
	}NxCatchAll(__FUNCTION__);
}

void COMRReviewDlg::OnBnClickedOmrReviewCommit()
{
	try{
		
		long nPatientID = m_pNxNXLtoReview->GetPatientID();

		// (b.savon 2013-02-28 16:00) - PLID 54714 - Reworked.  Let's validate the items before we commit. Don't collapse/expand anything
		// (b.spivey, September 10, 2012) - PLID 52286 - If we got this warning, we need to prevent committing. 
		if (!ValidateSelections(FALSE, FALSE)) {
			return; 
		}

		// (b.spivey, September 10, 2012) - PLID 52513 - If no patient found with this ID we need to make them select one. 
		if (!ReturnsRecordsParam("SELECT * FROM PatientsT WHERE PersonID = {INT}", nPatientID)) {
			AfxMessageBox("The patient ID in this Pending OMR does not map to a patient in NexTech Practice. "
				"Please select a patient to assign this Pending OMR to. "); 

			// (b.savon 2013-02-27 11:34) - PLID 54713 - Use the multi column single select dialog
			// Prepare
				//1. Columns to Select (order matters)
			CStringArray aryColumns;
			aryColumns.Add("PersonT.ID");
			aryColumns.Add("PatientsT.UserDefinedID");
			aryColumns.Add("PersonT.Last");
			aryColumns.Add("PersonT.First");
				//2. Column Headers (order matters)
			CStringArray aryColumnHeaders;
			aryColumnHeaders.Add("ID");
			aryColumnHeaders.Add("Patient ID");
			aryColumnHeaders.Add("Last");
			aryColumnHeaders.Add("First");
				//3. Sort order for columns (order matters)
			CSimpleArray<short> arySortOrder;
			arySortOrder.Add(-1);
			arySortOrder.Add(0);
			arySortOrder.Add(1);
			arySortOrder.Add(2);
			
			// Open the dialog
			CSingleSelectMultiColumnDlg dlg(this);
			HRESULT hr = dlg.Open(" PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID ",	/*From*/
								  " PersonT.ID > 0 AND PatientsT.CurrentStatus <> 4 ",					/*Where*/
								  aryColumns,															/*Select*/
								  aryColumnHeaders,														/*Column Names*/
								  arySortOrder,															/*Sort Order*/
								  "[1] - [2], [3]",														/*Display Columns*/
								  "Please select a patient to assign to this OMR.",						/*Description*/
								  "Select Patient for OMR"												/*Title Bar Header*/
								  );

			// If they clicked OK, be sure to check if they made a valid selection
			if( hr == IDOK ){
				//	Get the column values from the selected row
				CVariantArray varySelectedValues;
				dlg.GetSelectedValues(varySelectedValues);
				long nPatientID;
				if( varySelectedValues.GetSize() > 0 && 
					(nPatientID = VarLong(varySelectedValues.GetAt(0), -1)) > 0 ){
					// If valid, set ID
					m_pNxNXLtoReview->SetPatientID(nPatientID);
				}else{
					// If not, tell the user
					AfxMessageBox("You did not select a valid patient. Please verify the patient's name and try again.");
					return;
				}
			}else if( hr == IDCANCEL ){ //They clicked cancel, let them know they cannot commit
				AfxMessageBox("You cannot commit a Pending OMR without a patient.");
				return;
			}
		}

		EndDialog(IDYES);
	}NxCatchAll(__FUNCTION__);
}

void COMRReviewDlg::OnBnClickedOmrReviewDonotcommit()
{
	try{
		EndDialog(IDNO);
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-03-08 16:50) - PLID 55456 - Add format tree flag to not format when committing
// (b.savon 2013-02-28 16:28) - PLID 54714 - Changed return type to BOOL
// (b.spivey, August 31, 2012) - PLID 52286 - Validate Selections function -- actually just colors columns. 
BOOL COMRReviewDlg::ValidateSelections(BOOL bSilent /*= FALSE*/, BOOL bFormatTree /*= TRUE*/)
{
	try{
		if(!m_pDetailList) {
			//Something has to have gone horribly wrong.
			return FALSE;
		}

		//If we have more than one selection on a single select. 
		m_bMultiSelectWarning = false;

		//Parent row pointer, keep this for the outer loop. 
		NXDATALIST2Lib::IRowSettingsPtr pParentRow = m_pDetailList->GetFirstRow();
		
		// (b.savon 2013-02-28 16:29) - PLID 54714 - Reworked this function.  Separated out into ValidateItemSelections(..)
		while (pParentRow) {

			ValidateItemSelections(pParentRow, bFormatTree);

			pParentRow = pParentRow->GetNextRow(); 
		}

		//Throw prompt if warn is true. 
		if (m_bMultiSelectWarning && !bSilent){
			MessageBox("There is at least one single select item that has been detected as having multiple selections. Please review "
				"your selections before attempting to commit this EMN.", "Invalid Single Select", MB_ICONWARNING|MB_OK);
			return FALSE;
		}

	}NxCatchAll("Unable to validate OMR Selections!");
	
	return TRUE;
}

// (b.savon 2013-02-28 14:59) - PLID 54714 - Put into it's own function so we can valid items separately.  Previously,
// we just did a batch validation of all the selections.  Now, we need to validate if they edit the selections.  Since they edit
// a selection on a single detail at a time, it would be silly to do a batch validation of all the items.  Making this into a separate
// function gives us the ability to maximize our performance, albeit just a little, still necessary for the task at hand.
// (b.savon 2013-03-05 17:58) - PLID 55456 - Added default collapse flag to not collapse on corrections
void COMRReviewDlg::ValidateItemSelections(NXDATALIST2Lib::IRowSettingsPtr pParentRow, BOOL bCollapse /* = TRUE */)
{
	// The caller is responsible for passing in a valid parent row, but let's be safe
	if( pParentRow == NULL /*This isn't a valid row all together*/ || pParentRow->GetParentRow() /*This is a child row, silly*/  ){	
		ThrowNxException("Invalid parent row passed to COMRReviewDlg::ValidateItemSelections!");
	}

	//These variables matter for the inner loop.
	long nCountSelected = 0;
	NXDATALIST2Lib::IRowSettingsPtr pChildRow = pParentRow->GetFirstChildRow();

	while (pChildRow) {
		// (b.savon 2013-02-28 12:54) - PLID 54714 - Restructure
		//Keep count of selections for each parent, and color the parent row accordingly
		//after we count the selected children.
		if(VarBool(pChildRow->GetValue(OMRReviewList::Selected), false)) {
			nCountSelected++;
		}

		pChildRow = pChildRow->GetNextRow();
	}

	// (b.savon 2013-02-28 11:58) - PLID 54714 - Restructure
	// We only need to keep the count selected in the child loop and make our color determination outside in the parent loop.
	long nEMRDataType = VarLong(pParentRow->GetValue(OMRReviewList::DataType));
	OLE_COLOR ocRowColor = OMRUtils::GetOMRItemValidation(nEMRDataType, nCountSelected, m_bMultiSelectWarning);		
	pParentRow->PutBackColor(ocRowColor);
	pParentRow->PutBackColorSel(ocRowColor);

	// (b.savon 2013-03-05 14:12) - PLID 55456 - Only collapse/expand on full validation checks and not corrections (i.e. single validation)
	if( bCollapse ){
		PutParentRowView(pParentRow);
	}
}

// (b.savon 2013-03-05 14:12) - PLID 55456 - Set the view based on the parent properties.
void COMRReviewDlg::PutParentRowView(NXDATALIST2Lib::IRowSettingsPtr pParentRow)
{
	// The caller is responsible for passing in a valid parent row, but let's be safe
	if( pParentRow == NULL /*This isn't a valid row all together*/ || pParentRow->GetParentRow() /*This is a child row, silly*/  ){	
		ThrowNxException("Invalid parent row passed to COMRReviewDlg::PutParentRowView!");
	}

	if( m_radioExpandAll.GetCheck() == BST_CHECKED ){
		pParentRow->PutExpanded(VARIANT_TRUE);
		return;
	}

	if( m_radioCollapseAll.GetCheck() == BST_CHECKED ){
		pParentRow->PutExpanded(VARIANT_FALSE);
		return;		
	}

	if( m_radioCollapseValid.GetCheck() == BST_CHECKED ){
		if( pParentRow->GetBackColor() == OMR_SUCCESS ){
			pParentRow->PutExpanded(VARIANT_FALSE);
			return;		
		}else{
			pParentRow->PutExpanded(VARIANT_TRUE);
			return;	
		}
	}
}

CRect COMRReviewDlg::GetHTMLControlRect()
{
	CRect rcNextButtonRect;
	GetDlgItem(IDC_OMR_REVIEW_NEXT)->GetWindowRect(&rcNextButtonRect);
	ScreenToClient(&rcNextButtonRect);

	CRect rcPrevButtonRect;
	GetDlgItem(IDC_OMR_REVIEW_PREV)->GetWindowRect(&rcPrevButtonRect);
	ScreenToClient(&rcPrevButtonRect);

	CRect rcPatLabel;
	GetDlgItem(IDC_OMR_REVIEW_PAT_LBL)->GetWindowRect(&rcPatLabel);
	ScreenToClient(&rcPatLabel);

	CRect rcHTMLControl;
	rcHTMLControl.top = rcPatLabel.top;
	rcHTMLControl.bottom = rcNextButtonRect.bottom;
	rcHTMLControl.right = rcNextButtonRect.right;
	rcHTMLControl.left = rcPrevButtonRect.left;

	return rcHTMLControl;
}

void COMRReviewDlg::OnSize(UINT nType, int cx, int cy)
{
	try {
		CNxDialog::OnSize(nType, cx, cy);

		if (m_pCtrl && m_pCtrl->GetSafeHwnd()) {
			m_pCtrl->MoveWindow(GetHTMLControlRect());
		}
	} NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(COMRReviewDlg, CNxDialog)
ON_EVENT(COMRReviewDlg, IDC_OMR_PEND_COMMIT_DETAILS, 10, COMRReviewDlg::EditingFinishedOmrPendCommitDetails, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
END_EVENTSINK_MAP()

// (b.savon 2013-02-28 13:45) - PLID 54714 - Be able to edit the selections
void COMRReviewDlg::EditingFinishedOmrPendCommitDetails(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try{
		if( !bCommit ){
			return; //Nothing to do here
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if( pRow ){
			NXDATALIST2Lib::IRowSettingsPtr pParentRow = pRow->GetParentRow();
			if( pParentRow ){
				//This is a child
				if( nCol == OMRReviewList::Selected ){
					//We clicked the checkbox, and we want to commit
					BOOL bSelected = VarBool(varNewValue, FALSE);
					long nDataGroupID = VarLong(pRow->GetValue(OMRReviewList::ID), -1);
					
					if( nDataGroupID == -1 ){
						//Restore the original value.
						pRow->PutValue(OMRReviewList::Selected, varOldValue);
						//The datalist wasn't populated with the data group id!
						ThrowNxException("Error in the mapped OMR Form! Unable to determine the DataGroupID for this selection!");
					}

					// If our new value is in the checked state, add it to our list of data group id's.  Otherwise, remove it.
					if( bSelected ){
						m_pNxNXLtoReview->AddValue(nDataGroupID);
					}else{
						m_pNxNXLtoReview->RemoveValue(nDataGroupID);
					}

					//If we have a valid parent row, validate these items
					if( pParentRow ){
						ValidateItemSelections(pParentRow, FALSE);
					}
				}
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-03-05 14:12) - PLID 55456 - Uncheck the others in the group and then validate
void COMRReviewDlg::OnBnClickedRdoCollapsedSuccess()
{
	try{
		m_radioCollapseAll.SetCheck(BST_UNCHECKED);
		m_radioExpandAll.SetCheck(BST_UNCHECKED);
		ValidateSelections(TRUE);
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-03-05 14:12) - PLID 55456 - Uncheck the others in the group and then validate
void COMRReviewDlg::OnBnClickedRdoCollapsedAll()
{
	try{
		m_radioExpandAll.SetCheck(BST_UNCHECKED);
		m_radioCollapseValid.SetCheck(BST_UNCHECKED);
		ValidateSelections(TRUE);
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-03-05 14:12) - PLID 55456 - Uncheck the others in the group and then validate
void COMRReviewDlg::OnBnClickedRdoExpanded()
{
	try{
		m_radioCollapseAll.SetCheck(BST_UNCHECKED);
		m_radioCollapseValid.SetCheck(BST_UNCHECKED);
		ValidateSelections(TRUE);
	}NxCatchAll(__FUNCTION__);
}

void COMRReviewDlg::OnDestroy()
{
	try {
		//TES 5/15/2014 - PLID 62130 - Make sure the embedded browser gets destroyed
		m_pCtrl->DestroyWindow();
	}NxCatchAll(__FUNCTION__);

	//TES 5/15/2014 - PLID 62130 - Make sure the base class OnDestroy() handler gets called
	CNxDialog::OnDestroy();
}