// EMROMRMapperDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "EMROMRMapperDlg.h"
#include "EmrRc.h"
#include "SingleSelectDlg.h"
#include "EmrOmrMap.h" 

// (b.spivey, August 02, 2012) - PLID 51928 - Created

using namespace NXDATALIST2Lib; 
// CEMROMRMapperDlg dialog

IMPLEMENT_DYNAMIC(CEMROMRMapperDlg, CNxDialog)

// (j.fouts 2013-02-27 09:59) - PLID 54718 - Added a unique name to NexTechDialog constructor to remember size and position with
CEMROMRMapperDlg::CEMROMRMapperDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMROMRMapperDlg::IDD, pParent, "EMROMRMapperDlg_ResizePosition")
{

}

CEMROMRMapperDlg::~CEMROMRMapperDlg()
{
}

void CEMROMRMapperDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_OMR_FORM_LOCATION, m_nxeditFormLocation);
	DDX_Control(pDX, IDOK, m_nxbtnSave);
	DDX_Control(pDX, IDCANCEL, m_nxbtnClose); 
	DDX_Control(pDX, IDC_BTN_OMR_NEW, m_nxbtnNew);
	DDX_Control(pDX, IDC_BTN_OMR_RENAME, m_nxbtnRename);
	DDX_Control(pDX, IDC_BTN_OMR_DELETE, m_nxbtnDelete);
	DDX_Control(pDX, IDC_EMR_OMR_MAPPER_BKGRD_CLR, m_nxclrBackground);
}


BEGIN_MESSAGE_MAP(CEMROMRMapperDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_OMR_NEW, OnBtnOmrNew)
	ON_BN_CLICKED(IDC_BTN_OMR_DELETE, OnBtnOmrDelete)
	ON_BN_CLICKED(IDC_BTN_OMR_RENAME, OnBtnOmrRename)
	ON_BN_CLICKED(IDOK, OnBtnOmrSave)
	ON_BN_CLICKED(IDCANCEL, OnBtnOmrClose)
	ON_BN_CLICKED(IDC_BTN_BROWSE_OMR_FORM, OnBtnOmrBrowse)
	ON_EN_KILLFOCUS(IDC_EDIT_OMR_FORM_LOCATION, OnKillFocusFormLocation) 
END_MESSAGE_MAP()


// CEMROMRMapperDlg message handlers

BOOL CEMROMRMapperDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog(); 
	try {
		SetMinSize(400, 500);
		// (v.maida 2016-06-06 15:55) - NX-100631 - Set a larger default size so that the dialog is easier to read if it's restored down to a smaller size.
		SetWindowPos(NULL, 0, 0, 994, 738, 0);
		// (b.savon 2013-03-06 10:30) - PLID 55467 - Show window maximized
		ShowWindow(SW_SHOWMAXIMIZED);
		
		//The map does not access data directly. 
		m_dlEmrOmrMapList = BindNxDataList2Ctrl(IDC_EMR_OMR_MAP_DL, false); 
		m_dlFormSelectList = BindNxDataList2Ctrl(IDC_FORM_SELECT_LIST, true); 
		m_dlTemplateSelectList = BindNxDataList2Ctrl(IDC_EMR_TEMPLATE_SELECT, true);

		//Button controls for aesthetics
		m_nxbtnSave.SetIcon(IDI_SAVE); 
		m_nxbtnClose.AutoSet(NXB_CLOSE);
		m_nxbtnNew.AutoSet(NXB_NEW);
		m_nxbtnRename.AutoSet(NXB_MODIFY);
		m_nxbtnDelete.AutoSet(NXB_DELETE);

		// (b.spivey, September 18, 2012) - PLID 51928 - This is the database limit. 
		m_nxeditFormLocation.SetLimitText(260); 
	
		//Color background
		m_nxclrBackground.SetColor(GetNxColor(GNC_ADMIN, 1));

		//Load with a selection. 
		m_dlFormSelectList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		IRowSettingsPtr pRow(m_dlFormSelectList->GetFirstRow());
		if (pRow) {
			m_dlFormSelectList->PutCurSel(pRow);
			m_EmrOmrMap.LoadEmrOmrMap(VarLong(pRow->GetValue(eomeID), -1), EmrOmrMap::ltsForm);
			m_dlTemplateSelectList->SetSelByColumn(eomeID, m_EmrOmrMap.GetCurrentTemplateID()); 
			m_nxeditFormLocation.SetWindowTextA(m_EmrOmrMap.GetFileLocation());
			LoadEmrOmrMapDatalist();
		}

	}NxCatchAll(__FUNCTION__); 

	return TRUE; 

}

//Loading the map. 
void CEMROMRMapperDlg::LoadEmrOmrMapDatalist()
{
	//clean the array and the tree. 
	m_dlEmrOmrMapList->Clear(); 
	CMap<long, long, NXDATALIST2Lib::IRowSettingsPtr, NXDATALIST2Lib::IRowSettingsPtr> mapEmrTree; 

	//If the map has not been loaded or we got no items, we leave now. 
	if(m_EmrOmrMap.GetItemCount() <= 0) {
		return;
	}

	//Loop through and start updating the list. 
	for (int i = 0; i < m_EmrOmrMap.GetItemCount(); i++) {
		EmrOmrMap::EmrItemDetail eidTempDetail = m_EmrOmrMap.GetItemAt(i); 
		
		long nDetailID = eidTempDetail.ItemID; 

		if (nDetailID < 0) { 
			continue; 
		}

		//We need to make sure we have parent rows for the item versus the selections. 
		NXDATALIST2Lib::IRowSettingsPtr pParentRow = NULL; 
		if (!mapEmrTree.Lookup(nDetailID, pParentRow)) {

			//Item name is on every selection. 
			CString strText = eidTempDetail.ItemName; 

			pParentRow = m_dlEmrOmrMapList->GetNewRow();
			pParentRow->PutValue(eomeID, nDetailID); 
			pParentRow->PutValue(eomeName, _variant_t(strText)); 
			pParentRow->PutValue(eomeOmrID, g_cvarNull); //NO OmrID. 

			// (b.savon 2013-03-06 10:22) - PLID 55467 - Since they are already sorted in the object map of
			// how they appear on the template, just add it to the datalist.
			m_dlEmrOmrMapList->AddRowAtEnd(pParentRow, NULL); 

			mapEmrTree.SetAt(nDetailID, pParentRow);
		}

		//Always get a new row for individual selections. 
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlEmrOmrMapList->GetNewRow(); 

		long nDataGroupID = eidTempDetail.DataGroupID;
		CString strText = eidTempDetail.SelectionText;
		long nOmrID = eidTempDetail.OmrID; 

		pRow->PutValue(eomeID, nDataGroupID);
		pRow->PutValue(eomeName, _variant_t(strText));
		if(nOmrID > 0) {
			// We may not have something here. That's OK. 
			pRow->PutValue(eomeOmrID, nOmrID); 
		}
		else {
			pRow->PutValue(eomeOmrID, g_cvarNull); 
		}

		//If we found something in the map we'll have a parent
		// (b.savon 2013-03-06 10:23) - PLID 55467 - Since they are already sorted in the object map of
		// how they appear on the template, just add it to the datalist.
		m_dlEmrOmrMapList->AddRowAtEnd(pRow, pParentRow);

		//Expand! 
		pParentRow->PutExpanded(TRUE); 
	}	
}

//When new, we get a new EMROMRMap. 
void CEMROMRMapperDlg::OnBtnOmrNew() {

	try {
		if (m_EmrOmrMap.GetModifiedStatus()) {
			UINT nResult = MessageBox("If you create a new form without saving you will lose your changes to the current form map. "
				"\r\n"
				"\r\n"
				"Do you want to save your changes to the current form map?" , 
			"Creating new Form Map", MB_ICONWARNING|MB_YESNOCANCEL);
			if (IDNO == nResult) {
				//They don't care. 
			}
			else if (IDYES == nResult) {
				//They wanted to save. 
				m_EmrOmrMap.SaveEmrOmrMap(); 
			}
			else {
				//They canceled. 
				return;
			}

		}

		long nTemplateID = -1; 
		CString strDescription = ""; 
		long nFormID = -1; 

		// (b.spivey, August 31, 2012) - PLID 51928 - switched this around since you select form then template on the dlg
		//Form description
		// (b.spivey, September 19, 2012) - PLID 51928 - Don't throw the prompt unless they hit okay. 
		if (IDOK == InputBoxLimited(this, "Enter a new form description", strDescription, "", 500, false, false, NULL)) {

			//If no description, then we don't make a form. 
			if(strDescription.GetLength() <= 0) {
				MessageBox("You must enter a description to make a new form.", "No description entered.", MB_OK|MB_ICONWARNING);
				return;
			}

		}
		else {
			return; 
		}

		//Get our template. 
		CSingleSelectDlg dlg(this);
		if(dlg.Open("EMRTemplateT", "Deleted <> 1", "ID", "Name", 
			"Select a template to map this form to.") == IDOK) {

				nTemplateID = dlg.GetSelectedID();
		} 
		else 
		{
			//They didn't choose a template, we're out. 
			return;
		}

		if (nTemplateID == -1) {
			MessageBox("You must select a template to create a new form.", "Template not selected.", MB_OK|MB_ICONWARNING);
			return;
		}

		//Create a new form. 
		nFormID = m_EmrOmrMap.CreateNewMap(nTemplateID, strDescription); 

		//Requery then set the selection. 
		m_dlFormSelectList->Requery(); 
		m_dlFormSelectList->SetSelByColumn(eomeID, _variant_t(nFormID)); 

		m_dlTemplateSelectList->SetSelByColumn(eomeID, nTemplateID); 

		// (b.spivey, September 18, 2012) - PLID 51928 - Clear this when new form.
		m_nxeditFormLocation.SetWindowTextA(""); 

		LoadEmrOmrMapDatalist(); 

		return; 
	} NxCatchAll(__FUNCTION__); 
}

//If they delete...
void CEMROMRMapperDlg::OnBtnOmrDelete() 
{
	try {
		//This is the point of no return. 
		if (m_EmrOmrMap.GetCurrentFormID() > 0 
			&& IDYES == MessageBox("If you delete this form you cannot recover the "
			"OMR/EMR mapping currently associated with it! \r\n\r\n "
			"Are you sure you want to delete this form? ", 
			"Deleting OMR Form", MB_ICONWARNING|MB_YESNO)) {

				//Delete the map. 
				m_EmrOmrMap.DeleteEmrOmrMap(); 

				//Requery, select nothing. 
				m_dlFormSelectList->Requery(); 
				m_dlFormSelectList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
				m_dlTemplateSelectList->PutCurSel(NULL);
				m_dlEmrOmrMapList->Clear(); 
				m_nxeditFormLocation.SetWindowText(""); 

				IRowSettingsPtr pRow = m_dlFormSelectList->GetFirstRow();
				m_dlFormSelectList->PutCurSel(pRow);
				if (pRow) {
					m_EmrOmrMap.LoadEmrOmrMap(VarLong(pRow->GetValue(eomeID), -1), EmrOmrMap::ltsForm);
					m_dlTemplateSelectList->SetSelByColumn(eomeID, m_EmrOmrMap.GetCurrentTemplateID()); 
					m_nxeditFormLocation.SetWindowTextA(m_EmrOmrMap.GetFileLocation());
					LoadEmrOmrMapDatalist();
				}
		}
		return;
	}NxCatchAll(__FUNCTION__);
}

//If they want to rename...
void CEMROMRMapperDlg::OnBtnOmrRename() 
{
	try {
		//Don't do this, it'll error. 
		if (m_EmrOmrMap.GetCurrentFormID() <= 0) {
			return;
		}

		CString strDescription;
		InputBoxLimited(this, "Enter a new form description", strDescription, "", 500, false, false, NULL);
		if (!strDescription.IsEmpty()) {
			m_EmrOmrMap.SetDescription(strDescription);
			long nID = VarLong(m_dlFormSelectList->GetCurSel()->GetValue(eomeID));
			m_dlFormSelectList->Requery(); 
			m_dlFormSelectList->SetSelByColumn(eomeID, nID); 
		}
	}NxCatchAll(__FUNCTION__);

}

BEGIN_EVENTSINK_MAP(CEMROMRMapperDlg, CNxDialog)
	ON_EVENT(CEMROMRMapperDlg, IDC_FORM_SELECT_LIST, 1, CEMROMRMapperDlg::SelChangingFormSelectList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CEMROMRMapperDlg, IDC_EMR_TEMPLATE_SELECT, 1, CEMROMRMapperDlg::SelChangingEmrTemplateSelect, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CEMROMRMapperDlg, IDC_EMR_OMR_MAP_DL, 8, CEMROMRMapperDlg::OnEditingStartingEmrOmrMap, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CEMROMRMapperDlg, IDC_EMR_OMR_MAP_DL, 9, CEMROMRMapperDlg::OnEditingFinishingEmrOmrMap, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CEMROMRMapperDlg, IDC_EMR_TEMPLATE_SELECT, 16, CEMROMRMapperDlg::OnSelChosenEmrTemplateSelect, VTS_DISPATCH)
	ON_EVENT(CEMROMRMapperDlg, IDC_FORM_SELECT_LIST, 16, CEMROMRMapperDlg::OnSelChosenFormSelectList, VTS_DISPATCH)
END_EVENTSINK_MAP()

//When changing selections for forms. 
void CEMROMRMapperDlg::SelChangingFormSelectList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {			
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

	// (b.spivey, August 20, 2012) - PLID 51928 - Commented out old code. 
	//	NXDATALIST2Lib::IRowSettingsPtr pRow(*lppNewSel); 

	//	//Get the template and form. 
	//	long nTemplateID = VarLong(pRow->GetValue(eomeOmrID), -1); 
	//	long nFormID = VarLong(pRow->GetValue(eomeID), -1);

	//	//Get the modified status. 
	//	if(m_EmrOmrMap.GetModifiedStatus()) {
	//		
	//		UINT nResult = MessageBox("This map has been modified. If you change the form now, these changes will be lost."
	//		"\r\n"
	//		"\r\n"
	//		"Do you wish to save the map now?", 
	//		"Map Modified", MB_YESNOCANCEL|MB_ICONWARNING);

	//		if(nResult == IDYES) {

	//			//Save map, update form template ID. 
	//			m_EmrOmrMap.SaveEmrOmrMap(); 
	//			NXDATALIST2Lib::IRowSettingsPtr pOldRow(lpOldSel);
	//			pOldRow->PutValue(eomeOmrID, m_dlTemplateSelectList->GetCurSel()->GetValue(eomeID));
	//		}
	//		else if (nResult == IDNO) {
	//			//Load the new map, select template, load emromr tree. 
	//			m_EmrOmrMap.LoadEmrOmrMap(nFormID, EmrOmrMap::ltsForm); 
	//			m_dlTemplateSelectList->SetSelByColumn(eomeID, nTemplateID); 
	//			LoadEmrOmrMapDatalist();
	//		}
	//		else {
	//			return;
	//			//Reset form description.
	//			NXDATALIST2Lib::IRowSettingsPtr pOldRow(lpOldSel);
	//			pOldRow->PutValue(eomeName, _variant_t(m_EmrOmrMap.GetOriginalDesc()));
	//		}
	//	}



	} NxCatchAll(__FUNCTION__);
}

//Changed templates. 
void CEMROMRMapperDlg::SelChangingEmrTemplateSelect(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {

		if (*lppNewSel == NULL) {			
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}
		// (b.spivey, August 20, 2012) - PLID  51928 - Commented out old code. 
		////Warn them -- They need to be aware that they're bound to delete the map this way. 
		//if (IDNO == MessageBox("If you change the template associated with this form then you will lose the current OMR/EMR map! "
		//	"\r\n\r\n"
		//	"Are you sure you wish to change the template currently associated with this form?", 
		//	"Changing EMR Template", MB_ICONWARNING|MB_YESNO)) {

		//		SafeSetCOMPointer(lppNewSel, lpOldSel);
		//}
		//else {
		//	//Lets load a new template. 
		//	IRowSettingsPtr pRow(*lppNewSel); 
		//	long nTemplateID = VarLong(pRow->GetValue(eomeID), -1);
		//	m_EmrOmrMap.LoadEmrOmrMap(nTemplateID, EmrOmrMap::ltsTemplate);
		//	LoadEmrOmrMapDatalist();
		//}

	} NxCatchAll(__FUNCTION__);
}

void CEMROMRMapperDlg::OnEditingStartingEmrOmrMap(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			*pbContinue = FALSE; 
			return;
		}

		//If this is a parent row, we don't edit at all
		 if(!pRow->GetParentRow()){
			*pbContinue = FALSE;
			return;
		}

		*pbContinue = TRUE;

	}NxCatchAll(__FUNCTION__);

}

//For changing the values. 
void CEMROMRMapperDlg::OnEditingFinishingEmrOmrMap(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue)
{
	try {
		IRowSettingsPtr pRow(lpRow); 

		//Null, eject!
		if(pRow == NULL) {
			*pbCommit = FALSE;
			*pbContinue = FALSE;
			return;
		}

		//not even remotely plausible, eject!
		if (nCol != eomeOmrID) {
			*pbCommit = FALSE;
			*pbContinue = FALSE;
			return;
		}

		long nNewValue = VarLong(*pvarNewValue, -1);
		long nOldValue = VarLong(varOldValue, -1);
		bool bDelete = false;

		//If they didn't enter anything valid, delete.
		if(CString(strUserEntered).IsEmpty() || atoi(CString(strUserEntered)) <= 0) {
			bDelete = true;
			pvarNewValue->vt = VT_NULL; 
		}
		else {
			//if letters and numbers?
			//atoi(CString(strUserEntered).("1234567890")) <= 0) {
		}

		//Seemingly pointless, but just good practice. 
		switch (nCol) {
			case eomeOmrID:
			{
				long nSize = m_EmrOmrMap.GetItemCount();
				long nChangingID = VarLong(pRow->GetValue(eomeID), -1); 

				//Looking for dupes. 
				for (int i = 0; i < nSize; i++) {
					//If we're looking at ourself, don't compare. 
					EmrOmrMap::EmrItemDetail tempDetail = m_EmrOmrMap.GetItemAt(i); 
					if (tempDetail.DataGroupID == nChangingID) {
						continue;
					}

					//If dupe found, we leave. 
					if (tempDetail.OmrID == nNewValue && bDelete == false)
					{
						MessageBox("This OMRID already exists in the map. You cannot have duplicate IDs in the map.", 
							"Duplicate OMRID Detected", MB_ICONWARNING|MB_OK);
						*pbCommit = FALSE;
						*pbContinue = FALSE;
						return; 
					}
				}

				//If we got this far, we must be gold. 
				m_EmrOmrMap.ModifyOmrMapValue(nChangingID, nNewValue, bDelete);
				*pbCommit = TRUE;
				*pbContinue = TRUE;
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CEMROMRMapperDlg::OnSelChosenEmrTemplateSelect(LPDISPATCH lpRow) 
{
	try {
		IRowSettingsPtr pRow(lpRow);

		if (pRow == NULL) {
			return;
		}

		//If the current selection is different and we have a form selected, we can change templates. 
		if(m_dlFormSelectList->CurSel) {
			if (VarLong(pRow->GetValue(eomeID), -1) != m_EmrOmrMap.GetCurrentTemplateID()) {
			
				UINT nResult = MessageBox("If you change the template associated with this form then you will lose the current OMR/EMR map! "
				"\r\n\r\n"
				"Would you like to save your current map and change the template?", 
				"Changing EMR Template", MB_ICONWARNING|MB_YESNO);
				if (IDYES == nResult) {
					//They wanted to save. 
					m_EmrOmrMap.SaveEmrOmrMap(); 
					//Lets load a new template. 
					long nTemplateID = VarLong(pRow->GetValue(eomeID), -1);
					m_EmrOmrMap.LoadEmrOmrMap(nTemplateID, EmrOmrMap::ltsTemplate);
					LoadEmrOmrMapDatalist();
				}
				else {
					//They canceled. 
					m_dlTemplateSelectList->SearchByColumn(eomeID, _bstr_t(AsString(m_EmrOmrMap.GetCurrentTemplateID())), 
						m_dlTemplateSelectList->GetFirstRow(), TRUE);
				}
			}
			else {
				return;
			}
		}
		else {
			m_dlTemplateSelectList->CurSel = NULL; 
		}

	}NxCatchAll(__FUNCTION__);
}

void CEMROMRMapperDlg::OnSelChosenFormSelectList(LPDISPATCH lpRow) 
{
	try {
		if (lpRow == NULL) {			
			return;
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow); 

		//Get the form. 
		long nFormID = VarLong(pRow->GetValue(eomeID), -1);

		//Get the modified status. 
		if(m_EmrOmrMap.GetModifiedStatus() && m_dlFormSelectList->CurSel 
			&& VarLong(m_dlFormSelectList->CurSel->GetValue(eomeID), -1) != m_EmrOmrMap.GetCurrentFormID()) {
			
			UINT nResult = MessageBox("This map has been modified. If you change the form now, these changes will be lost."
			"\r\n"
			"\r\n"
			"Do you wish to save the map now?", 
			"Map Modified", MB_YESNOCANCEL|MB_ICONWARNING);

			if(nResult == IDYES) {

				//Save map, update form template ID. 
				m_EmrOmrMap.SaveEmrOmrMap(); 
				m_EmrOmrMap.LoadEmrOmrMap(nFormID, EmrOmrMap::ltsForm); 
				m_dlTemplateSelectList->SetSelByColumn(eomeID, m_EmrOmrMap.GetCurrentTemplateID()); 
				m_nxeditFormLocation.SetWindowTextA(m_EmrOmrMap.GetFileLocation());
				LoadEmrOmrMapDatalist();
			}
			else if (nResult == IDNO) {
				//Load the new map, select template, load emromr tree. 
				m_EmrOmrMap.LoadEmrOmrMap(nFormID, EmrOmrMap::ltsForm); 
				m_dlTemplateSelectList->SetSelByColumn(eomeID, m_EmrOmrMap.GetCurrentTemplateID()); 
				m_nxeditFormLocation.SetWindowTextA(m_EmrOmrMap.GetFileLocation());
				LoadEmrOmrMapDatalist();
			}
			else {
				//reset selection, do nothing. 
				m_dlFormSelectList->SearchByColumn(eomeID, _bstr_t(AsString(m_EmrOmrMap.GetCurrentFormID())), 
					m_dlFormSelectList->GetFirstRow(), TRUE);
				return;
			}
		}
		else if (!m_EmrOmrMap.GetModifiedStatus()) { 
			//If we got here then we didn't modify. 
			m_EmrOmrMap.LoadEmrOmrMap(nFormID, EmrOmrMap::ltsForm); 
			m_dlTemplateSelectList->SetSelByColumn(eomeID, m_EmrOmrMap.GetCurrentTemplateID()); 
			m_nxeditFormLocation.SetWindowTextA(m_EmrOmrMap.GetFileLocation());
			LoadEmrOmrMapDatalist();
		}

	}NxCatchAll(__FUNCTION__);
}

void CEMROMRMapperDlg::OnBtnOmrSave() 
{
	try {
		//Nothing fancy here, just save if they want to save. 
		m_EmrOmrMap.SaveEmrOmrMap(); 
	}NxCatchAll(__FUNCTION__); 
}

void CEMROMRMapperDlg::OnBtnOmrClose() 
{
	try {
		if(m_EmrOmrMap.GetModifiedStatus()) {
			UINT nResult = MessageBox("If you close now you will lose your changes to the current OMR/EMR Map!"
				"\r\n"
				"\r\n"
				"Do you want to save your changes before you close?", 
			"Closing OMR/EMR Configuration", MB_ICONWARNING|MB_YESNOCANCEL);
			if (IDNO == nResult) {
				//Welp, close it. 
			}
			else if (IDYES == nResult) {
				//They wanted to save. 
				m_EmrOmrMap.SaveEmrOmrMap(); 
			}
			else {
				//They canceled. 
				return;
			}
		}
		CNxDialog::OnCancel(); 
	}NxCatchAll(__FUNCTION__); 
}

void CEMROMRMapperDlg::OnBtnOmrBrowse() 
{
	try {
		if (m_dlFormSelectList->GetCurSel()) {
			CString strFileName = "";
			//browse for a file
			CFileDialog BrowseFiles(TRUE,NULL,NULL,OFN_FILEMUSTEXIST, NULL, this);
			//If they cancel, forget it.
			if (BrowseFiles.DoModal() == IDCANCEL) {
				return;
			}
			
			strFileName = BrowseFiles.GetPathName();
			m_nxeditFormLocation.SetWindowTextA(strFileName); 
			m_EmrOmrMap.SetFileLocation(strFileName); 
		}
	}NxCatchAll(__FUNCTION__);
}

void CEMROMRMapperDlg::OnKillFocusFormLocation()
{
	try {
		if (m_dlFormSelectList->GetCurSel()) {
			CString strFilePath; 
			m_nxeditFormLocation.GetWindowTextA(strFilePath);
			m_EmrOmrMap.SetFileLocation(strFilePath); 
		}
		return; 
	}NxCatchAll(__FUNCTION__);
}