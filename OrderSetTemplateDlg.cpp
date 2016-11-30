// OrderSetTemplateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "OrderSetTemplateDlg.h"
#include "AdministratorRc.h"
#include "GetNewIDName.h"
#include "AuditTrail.h"
#include "OrderSetTemplateLabSetupDlg.h"
#include "PrescriptionUtilsNonAPI.h"	// (j.jones 2013-03-27 17:23) - PLID 55920 - we only need the non-API header here

using namespace NXDATALIST2Lib;
using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37025 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



//(e.lally 2009-05-07) PLID 34187 - Created
// COrderSetTemplateDlg dialog

enum EOrderSetTemplateListColumns {
	oslOrderSetTmplID = 0,
	oslOrderSetTmplName,
};

enum ELabProcedureListColumns {
	lplcLabProcID = 0,
	lplcLabProcName,
	lplcLabProcTypeName,
	lplcLabProcType,
};

// (b.savon 2013-03-20 17:16) - PLID 54459 - Added FDB enum
enum EMedicationListColumns {
	mlcMedID = 0,
	mlcMedName,
	mlcFDBID,
	mlcFirstDataBankOutOfDate, //TES 5/9/2013 - PLID 56614
};

enum EReferringPhysListColumns {
	rplcRefPhyID = 0,
	rplcRefPhyName,
	rplcSpecialty,
};

enum ESelectedLabProceduresColumns {
	slpcID = 0,
	slpcOrderSetTmplID,
	slpcLabProcID,
	slpcName,
	slpcProcType,
	slpcToBeOrdered,
};

// (b.savon 2013-03-20 17:16) - PLID 54459 - Added FDB enum
enum ESelectedMedicationsColumns {
	smcID = 0,
	smcOrderSetTmplID,
	smcMedID,
	smcName,
	smcFDBID,
	smcFDBOutOfDate, //TES 5/9/2013 - PLID 56614
};

enum ESelectedReferringPhysColumns {
	srpcID = 0,
	srpcOrderSetTmplID,
	srpcRefPhyID,
	srpcRefPhyName,
	srpcSpecialty,
};

IMPLEMENT_DYNAMIC(COrderSetTemplateDlg, CNxDialog)

COrderSetTemplateDlg::COrderSetTemplateDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(COrderSetTemplateDlg::IDD, pParent)
{
	m_nOrderSetTemplateID = -1;
	m_bIsNewTemplate = FALSE;
}


void COrderSetTemplateDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ORDER_SET_TMPL_COLOR1, m_nxcolorTop);
	DDX_Control(pDX, IDC_ORDER_SET_TMPL_COLOR2, m_nxcolorBottom);
	DDX_Control(pDX, IDC_ORDER_SET_TMPL_LABEL, m_nxstaticOrderSet);
	DDX_Control(pDX, IDC_ORDER_SET_TMPL_LABS_LABEL, m_nxstaticLabs);
	DDX_Control(pDX, IDC_ORDER_SET_TMPL_MEDS_LABEL, m_nxstaticMeds);
	DDX_Control(pDX, IDC_ORDER_SET_TMPL_REFPHY_LABEL, m_nxstaticRefPhy);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_ADD_ORDER_SET_TEMPLATE, m_btnAdd);
	DDX_Control(pDX, IDC_DELETE_ORDER_SET_TEMPLATE, m_btnDelete);
}


BEGIN_MESSAGE_MAP(COrderSetTemplateDlg, CNxDialog)
	ON_BN_CLICKED(IDC_ADD_ORDER_SET_TEMPLATE, &COrderSetTemplateDlg::OnBtnClickedAddOrderSetTemplate)
	ON_BN_CLICKED(IDC_DELETE_ORDER_SET_TEMPLATE, &COrderSetTemplateDlg::OnBtnClickedDeleteOrderSetTemplate)
END_MESSAGE_MAP()


// COrderSetTemplateDlg message handlers

BEGIN_EVENTSINK_MAP(COrderSetTemplateDlg, CNxDialog)
ON_EVENT(COrderSetTemplateDlg, IDC_ORDER_SET_TEMPLATE_LIST, 18, COrderSetTemplateDlg::OnRequeryFinishedOrderSetTemplateList, VTS_I2)
ON_EVENT(COrderSetTemplateDlg, IDC_ORDER_SET_TEMPLATE_LIST, 16, COrderSetTemplateDlg::OnSelChosenOrderSetTemplateList, VTS_DISPATCH)
ON_EVENT(COrderSetTemplateDlg, IDC_ORDERSET_TMPL_LABS, 16, COrderSetTemplateDlg::OnSelChosenOrdersetTmplLabs, VTS_DISPATCH)
ON_EVENT(COrderSetTemplateDlg, IDC_ORDERSET_TMPL_MEDS, 16, COrderSetTemplateDlg::OnSelChosenOrdersetTmplMeds, VTS_DISPATCH)
ON_EVENT(COrderSetTemplateDlg, IDC_ORDERSET_TMPL_REFPHY, 16, COrderSetTemplateDlg::OnSelChosenOrdersetTmplRefPhy, VTS_DISPATCH)
ON_EVENT(COrderSetTemplateDlg, IDC_ORDERSET_TMPL_LABS_SELECTED, 6, COrderSetTemplateDlg::OnRButtonDownOrdersetTmplLabsSelected, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
ON_EVENT(COrderSetTemplateDlg, IDC_ORDERSET_TMPL_MEDS_SELECTED, 6, COrderSetTemplateDlg::OnRButtonDownOrdersetTmplMedsSelected, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
ON_EVENT(COrderSetTemplateDlg, IDC_ORDERSET_TMPL_REFPHY_SELECTED, 6, COrderSetTemplateDlg::OnRButtonDownOrdersetTmplRefPhySelected, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
ON_EVENT(COrderSetTemplateDlg, IDC_ORDERSET_TMPL_LABS_SELECTED, 19, COrderSetTemplateDlg::OnLeftClickOrdersetTmplLabsSelected, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
ON_EVENT(COrderSetTemplateDlg, IDC_ORDERSET_TMPL_MEDS, 18, COrderSetTemplateDlg::RequeryFinishedOrdersetTmplMeds, VTS_I2)
ON_EVENT(COrderSetTemplateDlg, IDC_ORDERSET_TMPL_MEDS_SELECTED, 18, COrderSetTemplateDlg::RequeryFinishedOrdersetTmplMedsSelected, VTS_I2)
END_EVENTSINK_MAP()


BOOL COrderSetTemplateDlg::OnInitDialog()
{
	try{
		CNxDialog::OnInitDialog();

		m_bIsNewTemplate = FALSE;

		//(e.lally 2009-05-08) PLID 34187 - Bind all the datalists. Only requery the master lists.
		m_pdlOrderSetList = BindNxDataList2Ctrl(IDC_ORDER_SET_TEMPLATE_LIST, true);
		m_pdlLabProceduresList = BindNxDataList2Ctrl(IDC_ORDERSET_TMPL_LABS, false);
		m_pdlLabProceduresSelected = BindNxDataList2Ctrl(IDC_ORDERSET_TMPL_LABS_SELECTED, false);
		// (e.lally 2009-05-13) PLID 34206 - Check the labs license, disable the master dropdown if not licensed.
		if(!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcLabs, CLicense::cflrSilent)) 
		{
			m_pdlLabProceduresList->Enabled = FALSE;
		}
		else{
			m_pdlLabProceduresList->Requery();
		}
		
		m_pdlMedicationsList = BindNxDataList2Ctrl(IDC_ORDERSET_TMPL_MEDS, true);
		m_pdlMedicationsSelected = BindNxDataList2Ctrl(IDC_ORDERSET_TMPL_MEDS_SELECTED, false);
		m_pdlReferringPhysiciansList = BindNxDataList2Ctrl(IDC_ORDERSET_TMPL_REFPHY, true);
		m_pdlReferringPhysiciansSelected = BindNxDataList2Ctrl(IDC_ORDERSET_TMPL_REFPHY_SELECTED, false);

		//Set the icon buttons
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnDelete.AutoSet(NXB_DELETE);

	}NxCatchAll(__FUNCTION__);
	return TRUE;
}

void COrderSetTemplateDlg::EnsureControls()
{
	// (e.lally 2009-05-13) PLID 34187
	BOOL bEnable = FALSE;
	if(m_pdlOrderSetList->GetRowCount() > 0){
		bEnable = TRUE;
	}
	GetDlgItem(IDC_DELETE_ORDER_SET_TEMPLATE)->EnableWindow(bEnable);

	// (e.lally 2009-07-17) PLID 34206 - Check the labs license, disable the master dropdown if not licensed.
	if(!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcLabs, CLicense::cflrSilent)) 
	{
		GetDlgItem(IDC_ORDERSET_TMPL_LABS)->EnableWindow(FALSE);
	}
	else{
		GetDlgItem(IDC_ORDERSET_TMPL_LABS)->EnableWindow(bEnable);
	}
	GetDlgItem(IDC_ORDERSET_TMPL_LABS_SELECTED)->EnableWindow(bEnable);
	GetDlgItem(IDC_ORDERSET_TMPL_MEDS)->EnableWindow(bEnable);
	GetDlgItem(IDC_ORDERSET_TMPL_MEDS_SELECTED)->EnableWindow(bEnable);
	GetDlgItem(IDC_ORDERSET_TMPL_REFPHY)->EnableWindow(bEnable);
	GetDlgItem(IDC_ORDERSET_TMPL_REFPHY_SELECTED)->EnableWindow(bEnable);
}

void COrderSetTemplateDlg::OnBtnClickedAddOrderSetTemplate()
{
	try{
		//(e.lally 2009-05-08) PLID 34187

		//(e.lally 2009-05-11) PLID 34187 - Handle the save and continue response
		int nSaveResponse = SaveAndContinue();
		// (e.lally 2009-07-20) PLID 34187 - Save and continue options are now Yes - save and continue or No- do not continue.
		//The save will have already taken place.
		if(nSaveResponse == IDNO){
			return;
		}

		CGetNewIDName dlg(this);
		CString strTemplateName;
		
		dlg.m_pNewName = &strTemplateName;
		dlg.m_nMaxLength = 255;
		int nDoModalResult = dlg.DoModal();
		if (nDoModalResult == IDOK) 
		{		
			strTemplateName.TrimRight();
			//Is it blank?
			if(strTemplateName == "") {
				MessageBox("You cannot enter a blank name for an Order Set Template.");
				return;
			}
			//check if this template name already exists
			if(!CreateParamRecordset(GetRemoteData(), "SELECT ID FROM OrderSetTemplatesT WHERE Name = {STRING}", strTemplateName)->eof) {
				MessageBox("There is already an Order Set Template with this name. Please choose a new name.","Practice",MB_OK|MB_ICONEXCLAMATION);
				return;
			}
			
			_RecordsetPtr rs = CreateParamRecordset("SET NOCOUNT ON \r\n"
				"INSERT INTO OrderSetTemplatesT(Name) VALUES({STRING})\r\n"
				"SET NOCOUNT OFF "
				"SELECT Convert(int, SCOPE_IDENTITY()) AS NewID ", strTemplateName); 
			if(!rs->eof){
				//Get the new ID
				m_nOrderSetTemplateID = AdoFldLong(rs, "NewID");
				//add a new row to our list of order set templates
				IRowSettingsPtr pNewRow = m_pdlOrderSetList->GetNewRow();
				ASSERT(pNewRow != NULL);
				pNewRow->PutValue(oslOrderSetTmplID, m_nOrderSetTemplateID);
				pNewRow->PutValue(oslOrderSetTmplName, _bstr_t(strTemplateName));
				m_pdlOrderSetList->AddRowSorted(pNewRow, NULL);
				m_pdlOrderSetList->CurSel = pNewRow;
				//Flag that this is a new template that must be saved
				m_bIsNewTemplate = TRUE;
				//Clear out the selected details datalists
				ClearDetails();
			}
		}

		EnsureControls();
	}NxCatchAll(__FUNCTION__);
}
void COrderSetTemplateDlg::ClearDetails()
{
	//(e.lally 2009-05-08) PLID 34187
	m_pdlLabProceduresSelected->Clear();
	m_pdlMedicationsSelected->Clear();
	m_pdlReferringPhysiciansSelected->Clear();

	ClearArrays();
}

void COrderSetTemplateDlg::OnBtnClickedDeleteOrderSetTemplate()
{
	try{
		//(e.lally 2009-05-08) PLID 34187
		IRowSettingsPtr pCurRow = m_pdlOrderSetList->GetCurSel();
		if(pCurRow == NULL){
			return;
		}
		CString strTemplateName, strMsg;
		long nTemplateID = VarLong(pCurRow->GetValue(oslOrderSetTmplID));
		ASSERT(nTemplateID == m_nOrderSetTemplateID);
		strTemplateName = VarString(pCurRow->GetValue(oslOrderSetTmplName));
		strMsg.Format("Are you sure you wish to delete the '%s' template and all it details?", strTemplateName);
		UINT response = MessageBox(strMsg,"Practice",MB_YESNO|MB_ICONEXCLAMATION);
		if(response == IDYES){
			if(DeleteOrderSetTemplate(nTemplateID)) {

				//Remove the row from the datalist
				m_pdlOrderSetList->RemoveRow(pCurRow);
				m_pdlOrderSetList->CurSel = NULL;
				m_nOrderSetTemplateID = -1;
				//ensure we reset the details
				ClearDetails();

				ReLoad();
				EnsureControls();
			}
		}
		
	}NxCatchAll(__FUNCTION__);
}


void COrderSetTemplateDlg::OnOK()
{
	try {
		//(e.lally 2009-05-08) PLID 34187
		Save();
		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}

void COrderSetTemplateDlg::OnCancel()
{
	try {
		//(e.lally 2009-05-11) PLID 34187 - Double check the loss of changes for new templates and unsaved changes.
		if(m_bIsNewTemplate){
			if(IDYES == MessageBox("This template has not been saved. Are you sure you wish to cancel?", "Practice", MB_YESNO)){
				//The cancelling of a new template removes it.
				DeleteOrderSetTemplate(m_nOrderSetTemplateID);
			}
			else{
				//Don't cancel, stay here.
				return;
			}
		}
		else if(HasChanges()){
			if(IDNO == MessageBox("Are you sure you wish to cancel and lose these changes?", "Practice", MB_YESNO)){
				//They do not want to cancel.
				return;
			}
		}
		CNxDialog::OnCancel();
	}NxCatchAll(__FUNCTION__);
}


void COrderSetTemplateDlg::LoadTemplateDetails(long nOrderSetTemplateID)
{
	//(e.lally 2009-05-08) PLID 34187 - clear the arrays of added and deleted details
	ClearArrays();

	CString strWhere;
	//All the datalists filter on the same column name in their respective tables
	strWhere.Format("OrderSetTemplateID = %li", nOrderSetTemplateID);

	m_pdlLabProceduresSelected->WhereClause = _bstr_t(strWhere);
	m_pdlLabProceduresSelected->Requery();
	m_pdlMedicationsSelected->WhereClause = _bstr_t(strWhere);
	m_pdlMedicationsSelected->Requery();
	m_pdlReferringPhysiciansSelected->WhereClause = _bstr_t(strWhere);
	m_pdlReferringPhysiciansSelected->Requery();

}

void COrderSetTemplateDlg::ReLoad()
{
	//(e.lally 2009-05-08) PLID 34187
	//Only load the first row if we have no current selection.
	if(m_pdlOrderSetList->GetCurSel() != NULL){
		return;
	}
	//Get the first row in the list and load it
	IRowSettingsPtr pRow = m_pdlOrderSetList->GetFirstRow();
	if(pRow != NULL){
		m_pdlOrderSetList->CurSel = pRow;
		m_nOrderSetTemplateID = VarLong(pRow->GetValue(oslOrderSetTmplID));
		LoadTemplateDetails(m_nOrderSetTemplateID);
	}
}

void COrderSetTemplateDlg::OnRequeryFinishedOrderSetTemplateList(short nFlags)
{
	try{
		EnsureControls();
		//(e.lally 2009-05-08) PLID 34187
		if(m_pdlOrderSetList->GetCurSel() == NULL){
			ReLoad();
		}
	}NxCatchAll(__FUNCTION__);
}

void COrderSetTemplateDlg::OnSelChosenOrderSetTemplateList(LPDISPATCH lpRow)
{
	try{
		// (e.lally 2009-07-20) PLID 34187 - If they selected the same template, take no action
		IRowSettingsPtr pRow(lpRow);
		if(pRow != NULL){
			long nSelTemplateID = VarLong(pRow->GetValue(oslOrderSetTmplID));
			if(nSelTemplateID == -1 || nSelTemplateID == m_nOrderSetTemplateID){
				return;
			}
		}
		//(e.lally 2009-05-11) PLID 34187 - Handle the save and continue response
		int nResponse = SaveAndContinue();
		// (e.lally 2009-07-20) PLID 34187 - Save and continue options are now Yes to save and continue and No to not continue
		if(nResponse == IDNO){
			//Set our template selection back to current template
			m_pdlOrderSetList->SetSelByColumn(oslOrderSetTmplID, m_nOrderSetTemplateID);
			return;
		}

		//(e.lally 2009-05-08) PLID 34187 - Get the new selected row and load it
		if(pRow != NULL){
			m_pdlOrderSetList->CurSel = pRow;
			m_nOrderSetTemplateID = VarLong(pRow->GetValue(oslOrderSetTmplID));
			LoadTemplateDetails(m_nOrderSetTemplateID);
			m_bIsNewTemplate = FALSE;
		}
		else{
			ClearDetails();
			ReLoad();
		}
	}NxCatchAll(__FUNCTION__);
}

void COrderSetTemplateDlg::OnSelChosenOrdersetTmplLabs(LPDISPATCH lpRow)
{
	try{
		//(e.lally 2009-05-08) PLID 34187
		IRowSettingsPtr pRow(lpRow);
		if(pRow != NULL){
			//Gather the ID and name of the lab procedure
			CString strName = VarString(pRow->GetValue(lplcLabProcName), "");
			long nLabProcID = VarLong(pRow->GetValue(lplcLabProcID));
			//Add it to the selected list
			IRowSettingsPtr pNewRow = m_pdlLabProceduresSelected->GetNewRow();
			if(pNewRow != NULL){
				_variant_t vtNull;
				vtNull.vt = VT_NULL;
				pNewRow->PutValue(slpcID, vtNull);
				pNewRow->PutValue(slpcOrderSetTmplID, m_nOrderSetTemplateID);
				pNewRow->PutValue(slpcLabProcID, nLabProcID);
				pNewRow->PutValue(slpcName, _bstr_t(strName));
				pNewRow->PutValue(slpcProcType, pRow->GetValue(lplcLabProcType));
				pNewRow->PutValue(slpcToBeOrdered, "");

				//It the new row to the list
				m_pdlLabProceduresSelected->AddRowSorted(pNewRow, NULL);

				//add it to our array
				m_paryAddedLabRowPtr.Add(pNewRow);
				m_aryAddedDetailNames.Add("Lab Procedure: " + strName);
				
			}
			//Reset the current selection in the master list
			m_pdlLabProceduresList->CurSel = NULL;
		}
	}NxCatchAll(__FUNCTION__);
}

void COrderSetTemplateDlg::OnSelChosenOrdersetTmplMeds(LPDISPATCH lpRow)
{
	try{
		//(e.lally 2009-05-08) PLID 34187
		IRowSettingsPtr pRow(lpRow);
		if(pRow != NULL){
			//Gather the ID and name of the lab procedure
			CString strName = VarString(pRow->GetValue(mlcMedName), "");
			long nMedicationID = VarLong(pRow->GetValue(mlcMedID));
			// (b.savon 2013-03-20 17:26) - PLID 54459 - Color the FDB meds
			long nFromFDB = VarLong(pRow->GetValue(mlcFDBID), -1);
			//TES 5/9/2013 - PLID 56614 - Pull whether the record is out of date
			// (j.jones 2015-05-20 10:33) - PLID 65518 - treat the 0 FDBID as never being out of date
			BOOL bFDBOutOfDate = VarBool(pRow->GetValue(mlcFirstDataBankOutOfDate), FALSE) && nFromFDB > 0;
			//Add it to the selected list
			IRowSettingsPtr pNewRow = m_pdlMedicationsSelected->GetNewRow();
			if(pNewRow != NULL){
				_variant_t vtNull;
				vtNull.vt = VT_NULL;
				pNewRow->PutValue(smcID, vtNull);
				pNewRow->PutValue(smcOrderSetTmplID, m_nOrderSetTemplateID);
				pNewRow->PutValue(smcMedID, nMedicationID);
				pNewRow->PutValue(smcName, _bstr_t(strName));
				//TES 5/9/2013 - PLID 56614 - Fill the FDBOutOfDate column
				pNewRow->PutValue(smcFDBOutOfDate, bFDBOutOfDate?g_cvarTrue:g_cvarFalse);

				// (b.savon 2013-03-20 17:27) - PLID 54459 - Color it
				//TES 5/9/2013 - PLID 56614 - Highlight the outdated codes
				if( nFromFDB > 0 ){
					if(bFDBOutOfDate) {
						pNewRow->PutBackColor(ERX_IMPORTED_OUTOFDATE_COLOR);
					}
					else {
						pNewRow->PutBackColor(ERX_IMPORTED_COLOR);
					}
				}

				//It the new row to the list
				m_pdlMedicationsSelected->AddRowSorted(pNewRow, NULL);

				//add it to our array
				m_aryAddedMedications.Add(nMedicationID);
				m_aryAddedDetailNames.Add("Medication: " + strName);
			}
			//Reset the current selection in the master list
			m_pdlMedicationsList->CurSel = NULL;
		}
	}NxCatchAll(__FUNCTION__);
}

void COrderSetTemplateDlg::OnSelChosenOrdersetTmplRefPhy(LPDISPATCH lpRow)
{
	try{
		//(e.lally 2009-05-08) PLID 34187
		IRowSettingsPtr pRow(lpRow);
		if(pRow != NULL){
			//Gather the ID and name of the lab procedure
			CString strName = VarString(pRow->GetValue(rplcRefPhyName), "");
			long nRefPhyID = VarLong(pRow->GetValue(rplcRefPhyID));
			//Add it to the selected list
			IRowSettingsPtr pNewRow = m_pdlReferringPhysiciansSelected->GetNewRow();
			if(pNewRow != NULL){
				_variant_t vtNull;
				vtNull.vt = VT_NULL;
				pNewRow->PutValue(srpcID, vtNull);
				pNewRow->PutValue(srpcOrderSetTmplID, m_nOrderSetTemplateID);
				pNewRow->PutValue(srpcRefPhyID, nRefPhyID);
				pNewRow->PutValue(srpcRefPhyName, _bstr_t(strName));
				pNewRow->PutValue(srpcSpecialty, pRow->GetValue(rplcSpecialty));

				//It the new row to the list
				m_pdlReferringPhysiciansSelected->AddRowSorted(pNewRow, NULL);

				//add it to our array
				m_aryAddedReferringPhys.Add(nRefPhyID);
				m_aryAddedDetailNames.Add("Referring Physician: " + strName);
			}
			//Reset the current selection in the master list
			m_pdlReferringPhysiciansList->CurSel = NULL;
		}
	}NxCatchAll(__FUNCTION__);
}

void COrderSetTemplateDlg::HandleRButtonDown(LPDISPATCH lpRow, EOrderSetComponent eSelectList)
{
	//(e.lally 2009-05-08) PLID 34187
	//If they clicked on an area with no row, just return
	if(lpRow == NULL){
		return;
	}
	_DNxDataListPtr pSelectedList;
	CArray<long> *paryAdded, *paryDeleted;
	int nIDCol =-1, nDetailIDCol =-1, nDetailNameCol=-1;
	CString strDetailType;
	switch(eSelectList){
		case eLabProcedure:
			pSelectedList = m_pdlLabProceduresSelected;
			paryAdded = NULL;
			paryDeleted = &m_aryDeletedLabProcs;
			nIDCol = (int)slpcID;
			nDetailIDCol = (int)slpcLabProcID;
			nDetailNameCol = (int)slpcName;
			strDetailType = "Lab Procedure";
			break;
		case eMedication:
			pSelectedList = m_pdlMedicationsSelected;
			paryAdded = &m_aryAddedMedications;
			paryDeleted = &m_aryDeletedMedications;
			nIDCol = (int)smcID;
			nDetailIDCol = (int)smcMedID;
			nDetailNameCol = (int)smcName;
			strDetailType = "Medication";
			break;
		case eReferral:
			pSelectedList = m_pdlReferringPhysiciansSelected;
			paryAdded = &m_aryAddedReferringPhys;
			paryDeleted = &m_aryDeletedReferringPhys;
			nIDCol = (int)srpcID;
			nDetailIDCol = (int)srpcRefPhyID;
			nDetailNameCol = (int)srpcRefPhyName;
			strDetailType = "Referring Physician";
			break;
		default:
			ASSERT(FALSE);
			return;
	}

	IRowSettingsPtr pRow(lpRow);
	//set the cur sel row to this one
	pSelectedList->CurSel = pRow;
	//Create a popup menu to remove this row
	CMenu pMenu;
	pMenu.CreatePopupMenu();
	pMenu.InsertMenu(0, MF_BYPOSITION, 1, "Remove");
	
	CPoint pt;
	GetCursorPos(&pt);
	int nReturn = pMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD,pt.x, pt.y,this);
	if(nReturn == 1) {
		//The user seleced the menu option to remove, if it is not a new entry,
		//	add it to our delete array. Remove the physical row from the dl.
		long nID = VarLong(pRow->GetValue(nIDCol), -1);
		CString strDetailName = VarString(pRow->GetValue(nDetailNameCol));
		if(nID > -1){
			//Use our array pointer to add it to the proper array
			paryDeleted->Add(nID);
			m_aryDeletedDetailNames.Add(strDetailType + ": " + strDetailName);
		}
		else{
			long nDetailID = VarLong(pRow->GetValue(nDetailIDCol));
			//Search the array of added details and remove any unsaved entry matching our ID.
			int i=0, nCount=0;
			if(eSelectList != eLabProcedure){
				ASSERT(paryAdded != NULL);
				for(i=0, nCount=paryAdded->GetCount(); i<nCount ; i++){
					if(paryAdded->GetAt(i) == nDetailID){
						paryAdded->RemoveAt(i);
						break;
					}
				}
			}
			else{
				for(i=0, nCount=m_paryAddedLabRowPtr.GetCount(); i<nCount ; i++){
					if((LPDISPATCH)m_paryAddedLabRowPtr.GetAt(i) == (LPDISPATCH)pRow){
						m_paryAddedLabRowPtr.RemoveAt(i);
						break;
					}
				}
			}
			//(e.lally 2009-05-13) PLID 34206 - Do the same search through the audit array
			for(i=0, nCount=m_aryAddedDetailNames.GetCount(); i<nCount ; i++){
				if(m_aryAddedDetailNames.GetAt(i) == (strDetailType + ": " + strDetailName)){
					m_aryAddedDetailNames.RemoveAt(i);
					break;
				}
			}

		}
		//Remove the row from the proper list.
		pSelectedList->RemoveRow(pRow);
	}
}
void COrderSetTemplateDlg::OnRButtonDownOrdersetTmplLabsSelected(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try{
		//(e.lally 2009-05-08) PLID 34187
		//Handle right click for lab procedures
		HandleRButtonDown(lpRow, eLabProcedure);
	}NxCatchAll(__FUNCTION__);
}

void COrderSetTemplateDlg::OnRButtonDownOrdersetTmplMedsSelected(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try{
		//(e.lally 2009-05-08) PLID 34187
		//Handle right click for medications
		HandleRButtonDown(lpRow, eMedication);
	}NxCatchAll(__FUNCTION__);
}

void COrderSetTemplateDlg::OnRButtonDownOrdersetTmplRefPhySelected(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try{
		//(e.lally 2009-05-08) PLID 34187
		//Handle right click for referring physicians
		HandleRButtonDown(lpRow, eReferral);
	}NxCatchAll(__FUNCTION__);
}
BOOL COrderSetTemplateDlg::HasChanges()
{
	//(e.lally 2009-05-11) PLID 34187
	//Do any of the arrays for added details have contents?
	if(m_paryAddedLabRowPtr.GetCount() >0){
		return TRUE;
	}
	if(m_aryAddedMedications.GetCount() > 0){
		return TRUE;
	}
	if(m_aryAddedReferringPhys.GetCount() > 0){
		return TRUE;
	}
	//Do any of the arrays for deleted details have contents?
	if(m_aryDeletedLabProcs.GetCount() > 0){
		return TRUE;
	}
	if(m_aryDeletedMedications.GetCount() > 0){
		return TRUE;
	}
	if(m_aryDeletedReferringPhys.GetCount() > 0){
		return TRUE;
	}
	// (e.lally 2009-07-20) PLID 34187
	if(m_aryDeletedTemplates.GetCount() > 0){
		return TRUE;
	}
		
	return FALSE;
}

void COrderSetTemplateDlg::ClearArrays()
{
	//(e.lally 2009-05-08) PLID 34187 - clear the array of added items
	m_paryAddedLabRowPtr.RemoveAll();
	m_aryAddedMedications.RemoveAll();
	m_aryAddedReferringPhys.RemoveAll();
	//and clear the deleted arrays
	m_aryDeletedLabProcs.RemoveAll();
	m_aryDeletedMedications.RemoveAll();
	m_aryDeletedReferringPhys.RemoveAll();
	// (e.lally 2009-05-13) PLID 34206 - Clear audit arrays
	m_aryAddedDetailNames.RemoveAll();
	m_aryDeletedDetailNames.RemoveAll();
}

void COrderSetTemplateDlg::Save()
{
	CAuditTransaction auditTrans;
	int i=0, nCount = 0;
	// (e.lally 2009-07-20) PLID 34187
	nCount = m_aryDeletedTemplates.GetCount();
	for(i=0; i<nCount; i++){
		long nTemplateID = m_aryDeletedTemplates.GetAt(i);
		CString strSqlBatch, strOldValue;
		CNxParamSqlArray args;
		// (e.lally 2009-05-13) PLID 34206 - Get template name for auditing
		AddParamStatementToSqlBatch(strSqlBatch, args, "SELECT Name FROM OrderSetTemplatesT WHERE ID = {INT};", nTemplateID);
		AddParamStatementToSqlBatch(strSqlBatch, args, "DELETE FROM OrderSetTemplateLabsT WHERE OrderSetTemplateID = {INT};", nTemplateID);
		AddParamStatementToSqlBatch(strSqlBatch, args, "DELETE FROM OrderSetTemplateMedicationsT WHERE OrderSetTemplateID = {INT};", nTemplateID);
		AddParamStatementToSqlBatch(strSqlBatch, args, "DELETE FROM OrderSetTemplateReferralsT WHERE OrderSetTemplateID = {INT};", nTemplateID);

		AddParamStatementToSqlBatch(strSqlBatch, args, "DELETE FROM OrderSetTemplatesT WHERE ID = {INT};", nTemplateID);
		//execute sql batch
		// (e.lally 2009-06-21) PLID 34680 - Fixed to use create recordset function.
		_RecordsetPtr prs = CreateParamRecordsetBatch(GetRemoteData(), strSqlBatch, args);
		if(!prs->eof){
			strOldValue = AdoFldString(prs, "Name");
		}

		//if(m_bIsNewTemplate == FALSE)
		{
			// (e.lally 2009-05-13) PLID 34206 - Audit
			AuditEvent(-1, "", BeginNewAuditEvent(), aeiOrderSetTemplateDeleted, nTemplateID, strOldValue, "<Deleted>", aepMedium, aetDeleted);
		}
	}
	//Clear our separate array
	m_aryDeletedTemplates.RemoveAll();


	//(e.lally 2009-05-08) PLID 34187
	if(m_nOrderSetTemplateID == -1){
		EnsureControls();
		auditTrans.Commit();
		return;
	}
	CString strSqlBatch;
	CNxParamSqlArray args;

	// (e.lally 2009-05-13) PLID 34206 - Get the template name for auditing
	CString strTemplateName;
	IRowSettingsPtr pRow = m_pdlOrderSetList->GetCurSel();
	if(pRow != NULL){
		if(m_nOrderSetTemplateID == VarLong(pRow->GetValue(oslOrderSetTmplID))){
			strTemplateName = VarString(pRow->GetValue(oslOrderSetTmplName));
		}
	}
	if(strTemplateName.IsEmpty()){
		pRow = m_pdlOrderSetList->FindByColumn(oslOrderSetTmplID, m_nOrderSetTemplateID, m_pdlOrderSetList->GetFirstRow(), VARIANT_FALSE);
		if(pRow != NULL){
			strTemplateName = VarString(pRow->GetValue(oslOrderSetTmplName));
		}
	}
	ASSERT(!strTemplateName.IsEmpty());

	if(m_bIsNewTemplate){
		// (e.lally 2009-05-13) PLID 34206 - Audit
		AuditEvent(-1, "", auditTrans, aeiOrderSetTemplateCreated, m_nOrderSetTemplateID, "", strTemplateName, aepMedium, aetCreated);
	}

	//loop through all the add and delete arrays
	int nTotalCount =0;
	{
		int nDetailID =0;
		nCount = m_paryAddedLabRowPtr.GetCount();
		nTotalCount += nCount;
		for(i=0; i<nCount; i++){
			LPDISPATCH lpdispLabRow = (LPDISPATCH)m_paryAddedLabRowPtr.GetAt(i);
			if(lpdispLabRow == NULL){
				ThrowNxException("COrderSetTemplateDlg::Save - Could not find row for newly added lab");
			}
			IRowSettingsPtr pLabRow(lpdispLabRow);
			CString strToBeOrdered = VarString(pLabRow->GetValue(slpcToBeOrdered), "");
			nDetailID = VarLong(pLabRow->GetValue(slpcLabProcID));
			AddParamStatementToSqlBatch(strSqlBatch, args, "INSERT INTO OrderSetTemplateLabsT "
				"(OrderSetTemplateID, LabProcedureID, ToBeOrdered) VALUES({INT}, {INT}, {STRING})\r\n"
				, m_nOrderSetTemplateID, nDetailID, strToBeOrdered);
		}
		nCount = m_aryAddedMedications.GetCount();
		nTotalCount += nCount;
		for(i=0; i<nCount; i++){
			nDetailID = m_aryAddedMedications.GetAt(i);
			AddParamStatementToSqlBatch(strSqlBatch, args, "INSERT INTO OrderSetTemplateMedicationsT "
				"(OrderSetTemplateID, MedicationID) VALUES({INT}, {INT})\r\n", m_nOrderSetTemplateID, nDetailID);
		}

		nCount = m_aryAddedReferringPhys.GetCount();
		nTotalCount += nCount;
		for(i=0; i<nCount; i++){
			nDetailID = m_aryAddedReferringPhys.GetAt(i);
			AddParamStatementToSqlBatch(strSqlBatch, args, "INSERT INTO OrderSetTemplateReferralsT "
				"(OrderSetTemplateID, ReferringPhyID) VALUES({INT}, {INT})\r\n", m_nOrderSetTemplateID, nDetailID);
		}

		// (e.lally 2009-05-13) - PLID 34206 - Audit
		nCount = m_aryAddedDetailNames.GetCount();
		ASSERT(nCount == nTotalCount);
		CString strNewValue;
		for(i=0; i<nCount; i++){
			strNewValue = m_aryAddedDetailNames.GetAt(i);
			// (e.lally 2009-05-13) PLID 34206 - Audit
			AuditEvent(-1, "", auditTrans, aeiOrderSetTemplateDetailCreated, m_nOrderSetTemplateID, "", strTemplateName + " - " + strNewValue, aepMedium, aetCreated);
		}
	}

	//Changed lab procedures array
	{
		// (e.lally 2009-05-26) PLID 34241 - Moved below the auditing for additions.
		nCount = m_aryChangedLabProcs.GetSize();
		for(i = 0; i < nCount; i++) {
			OrderSetLabInfo labinfo = m_aryChangedLabProcs.GetAt(i);
			ASSERT(labinfo.nID != -1);//this should only be possible if the array was set larger than the number of actual entries being used.
			AddParamStatementToSqlBatch(strSqlBatch, args,
				"UPDATE OrderSetTemplateLabsT SET ToBeOrdered = {STRING} WHERE ID = {INT};"
				, labinfo.strToBeOrdered, labinfo.nID);
			// (e.lally 2009-05-26) PLID 34241 - Audit
			AuditEvent(-1, "", auditTrans, aeiOrderSetTemplateLabToBeOrdered, m_nOrderSetTemplateID, strTemplateName + " - " + labinfo.strProcedureName + ": " + labinfo.strOrigToBeOrdered, strTemplateName + " - " + labinfo.strProcedureName + ": " + labinfo.strToBeOrdered, aepMedium, aetChanged);
		}
	}

	//Delete arrays
	{
		nTotalCount =0;
		int nID =0;
		nCount = m_aryDeletedLabProcs.GetCount();
		nTotalCount += nCount;
		for(i=0; i<nCount; i++){
			nID = m_aryDeletedLabProcs.GetAt(i);
			AddParamStatementToSqlBatch(strSqlBatch, args, "DELETE FROM OrderSetTemplateLabsT WHERE ID = {INT}\r\n", nID);
		}
		nCount = m_aryDeletedMedications.GetCount();
		nTotalCount += nCount;
		for(i=0; i<nCount; i++){
			nID = m_aryDeletedMedications.GetAt(i);
			AddParamStatementToSqlBatch(strSqlBatch, args, "DELETE FROM OrderSetTemplateMedicationsT WHERE ID = {INT}\r\n", nID);
		}

		nCount = m_aryDeletedReferringPhys.GetCount();
		nTotalCount += nCount;
		for(i=0; i<nCount; i++){
			nID = m_aryDeletedReferringPhys.GetAt(i);
			AddParamStatementToSqlBatch(strSqlBatch, args, "DELETE FROM OrderSetTemplateReferralsT WHERE ID = {INT}\r\n", nID);
		}

		// (e.lally 2009-05-13) - PLID 34206 - Audit
		nCount = m_aryDeletedDetailNames.GetCount();
		ASSERT(nCount == nTotalCount);
		CString strOldValue;
		for(i=0; i<nCount; i++){
			strOldValue = m_aryDeletedDetailNames.GetAt(i);
			// (e.lally 2009-05-13) PLID 34206 - Audit
			AuditEvent(-1, "", auditTrans, aeiOrderSetTemplateDetailDeleted, m_nOrderSetTemplateID, strTemplateName + " - " + strOldValue, "<Deleted>", aepMedium, aetDeleted);

		}
	}


	//execute sql batch
	if(!strSqlBatch.IsEmpty()){
		// (e.lally 2009-06-21) PLID 34680 - Leave as execute batch
		ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, args);
	}
	auditTrans.Commit();

	m_bIsNewTemplate = FALSE;

	//(e.lally 2009-05-08) PLID 34187 - clear the arrays
	ClearArrays();

	return;
}

int COrderSetTemplateDlg::SaveAndContinue()
{
	//(e.lally 2009-05-11) PLID 34187 - See if we need to save before continuing
	if(m_nOrderSetTemplateID == -1){
		EnsureControls();
		return IDYES;
	}
	BOOL bPromptToSave = FALSE;
	CString strMessage;
	// (e.lally 2009-07-20) PLID 34187 - Cahnged the wording of the prompts and made the options be 
	//	-Yes to save and continue
	//	-No to not continue (and not save)
	if(m_bIsNewTemplate){
		strMessage = "This new template will be saved before continuing.\n"
			"Do you still wish to continue?";
		bPromptToSave = TRUE;
	}
	else if(HasChanges()){
		strMessage = "The changes to this template will be saved before continuing.\n"
			"Do you still wish to continue?";
		bPromptToSave = TRUE;
	}

	//We'll default the response to "yes" if there was nothing to save.
	int nResponse = IDYES;
	if(bPromptToSave){
		nResponse = MessageBox(strMessage, "Practice", MB_YESNO);
		if(nResponse == IDYES){
			Save();
		}
	}
	return nResponse;
}

BOOL COrderSetTemplateDlg::DeleteOrderSetTemplate(long nTemplateID)
{
	if(nTemplateID == -1){
		EnsureControls();
		return FALSE;
	}
	// (e.lally 2009-05-13) PLID 34187 - Check for use on existing order sets before trying to delete
	_RecordsetPtr prsExisting = CreateParamRecordset(GetRemoteData(), "SELECT COUNT(*) AS OrderSetCount FROM OrderSetsT WHERE OrderSetTemplateID = {INT}", nTemplateID);
	if(!prsExisting->eof){
		long nOrderSetCount = AdoFldLong(prsExisting, "OrderSetCount", 0);
		if(nOrderSetCount > 0){
			MessageBox(FormatString("This Order Set Template is used on %li patient Orders Set(s) and cannot be removed.", nOrderSetCount), "Practice", MB_OK);
			//If this is an "unsaved" template, officially save it
			if(m_bIsNewTemplate){
				Save();
			}
			return FALSE;
		}
	}
	// (e.lally 2009-07-20) PLID 34187 - Immediately delete the template if it was a new "unsaved" template to begin with.
	//	Otherwise add it to our array for unsaved changes.
	if(m_bIsNewTemplate == FALSE){
		m_aryDeletedTemplates.Add(nTemplateID);
	}
	else{
		CString strSqlBatch, strOldValue;
		CNxParamSqlArray args;
		// (e.lally 2009-05-13) PLID 34206 - Get template name for auditing
		AddParamStatementToSqlBatch(strSqlBatch, args, "SELECT Name FROM OrderSetTemplatesT WHERE ID = {INT};", nTemplateID);
		AddParamStatementToSqlBatch(strSqlBatch, args, "DELETE FROM OrderSetTemplateLabsT WHERE OrderSetTemplateID = {INT};", nTemplateID);
		AddParamStatementToSqlBatch(strSqlBatch, args, "DELETE FROM OrderSetTemplateMedicationsT WHERE OrderSetTemplateID = {INT};", nTemplateID);
		AddParamStatementToSqlBatch(strSqlBatch, args, "DELETE FROM OrderSetTemplateReferralsT WHERE OrderSetTemplateID = {INT};", nTemplateID);

		AddParamStatementToSqlBatch(strSqlBatch, args, "DELETE FROM OrderSetTemplatesT WHERE ID = {INT};", nTemplateID);
		//execute sql batch
		// (e.lally 2009-06-21) PLID 34680 - Fixed to use create recordset function.
		_RecordsetPtr prs = CreateParamRecordsetBatch(GetRemoteData(), strSqlBatch, args);
		if(!prs->eof){
			strOldValue = AdoFldString(prs, "Name");
		}
	}
	m_bIsNewTemplate = FALSE;

	return TRUE;
}

void COrderSetTemplateDlg::OnLeftClickOrdersetTmplLabsSelected(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try{

		IRowSettingsPtr pRow(lpRow);
		const long nOrderSetLabID = VarLong(pRow->GetValue(slpcID), -1);

		// (e.lally 2009-05-14) PLID 34241 - Check column and Lab procedure type
		switch(nCol){
			case slpcToBeOrdered:
			{
				if(g_pLicense && g_pLicense->CheckForLicense(CLicense::lcLabs, CLicense::cflrSilent)) {
					if(pRow != NULL){
						//TODO: switch this to an enum, ensure this field is not VT_EMPTY
						short nLabProcType = VarShort(pRow->GetValue(slpcProcType), -1);
						if(nLabProcType != 1){ //Not a Biopsy type
							CString strText = VarString(pRow->GetValue(slpcToBeOrdered), "");
							const CString strOriginalText = strText;
							COrderSetTemplateLabSetupDlg dlg(this);
							dlg.m_pstrToBeOrderedText = &strText;
							if(IDOK == dlg.DoModal()){
								//Set the new text
								pRow->PutValue(slpcToBeOrdered, _bstr_t(strText));
								// (z.manning 2009-05-15 10:06) - If this is an existing order set lab template
								// and the text changed then mark it as changed.
								if(nOrderSetLabID != -1 && strText != strOriginalText) {
									BOOL bFound = FALSE;
									for(int nChangedLabIndex = 0; !bFound && nChangedLabIndex < m_aryChangedLabProcs.GetSize(); nChangedLabIndex++) {
										OrderSetLabInfo labinfo = m_aryChangedLabProcs.GetAt(nChangedLabIndex);
										if(labinfo.nID == nOrderSetLabID) {
											bFound = TRUE;
											labinfo.strToBeOrdered = strText;
											m_aryChangedLabProcs.SetAt(nChangedLabIndex, labinfo);
										}
									}

									if(!bFound) {
										OrderSetLabInfo labinfo;
										labinfo.nID = nOrderSetLabID;
										// (e.lally 2009-05-26) PLID 34241 - Track our lab procedure name for easier auditing.
										labinfo.strProcedureName = VarString(pRow->GetValue(slpcName), "");
										labinfo.strToBeOrdered = strText;
										// (e.lally 2009-05-26) PLID 34241 - Set the original to be ordered text for auditing.
										labinfo.strOrigToBeOrdered = strOriginalText;
										m_aryChangedLabProcs.Add(labinfo);
									}
								}
							}
						}
						else {
							MessageBox("This is a biopsy type lab, so there is no To Be Ordered field.");
						}
					}
				}
				break;
			}
			default:
				break;
		}
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-03-20 17:14) - PLID 54459 - Color the meds
void COrderSetTemplateDlg::RequeryFinishedOrdersetTmplMeds(short nFlags)
{
	try{
		for(NXDATALIST2Lib::IRowSettingsPtr pRow = m_pdlMedicationsList->GetFirstRow(); pRow; pRow = pRow->GetNextRow()) 
		{						
			long nFromFDB = VarLong(pRow->GetValue(mlcFDBID), -1);
			if( nFromFDB > 0 ){ 
				//TES 5/9/2013 - PLID 56614 - Highlight the outdated codes
				if(VarBool(pRow->GetValue(mlcFirstDataBankOutOfDate), FALSE)) {
					pRow->PutBackColor(ERX_IMPORTED_OUTOFDATE_COLOR);
				}
				else {
					pRow->PutBackColor(ERX_IMPORTED_COLOR);
				}
			}
		}	
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-03-20 17:29) - PLID 54459 
void COrderSetTemplateDlg::RequeryFinishedOrdersetTmplMedsSelected(short nFlags)
{
	try{
		for(NXDATALIST2Lib::IRowSettingsPtr pRow = m_pdlMedicationsSelected->GetFirstRow(); pRow; pRow = pRow->GetNextRow()) 
		{						
			long nFromFDB = VarLong(pRow->GetValue(smcFDBID), -1);
			//TES 5/9/2013 - PLID 56614 - Highlight the outdated codes
			if( nFromFDB > 0 ){ 
				if(VarBool(pRow->GetValue(smcFDBOutOfDate), FALSE)) {
					pRow->PutBackColor(ERX_IMPORTED_OUTOFDATE_COLOR);
				}
				else {
					pRow->PutBackColor(ERX_IMPORTED_COLOR);
				}
			}
		}	
	}NxCatchAll(__FUNCTION__);
}
