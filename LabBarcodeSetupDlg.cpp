// (r.gonet 10/17/2011) - PLID 45924 - Dialog that lets the user configure the lab barcode.
// LabBarcodeSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "LabBarcodeSetupDlg.h"
#include "LabBarcode.h"
#include "MsgBox.h"


// CLabBarcodeSetupDlg dialog

IMPLEMENT_DYNAMIC(CLabBarcodeSetupDlg, CNxDialog)

CLabBarcodeSetupDlg::CLabBarcodeSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CLabBarcodeSetupDlg::IDD, pParent)
{
	// (r.gonet 11/12/2011) - PLID 44212 - CLabBarcode is now abstract
	m_plbBarcode = new CLabCorpLabBarcode();
}

CLabBarcodeSetupDlg::~CLabBarcodeSetupDlg()
{
	// (r.gonet 11/12/2011) - PLID 44212 - CLabBarcode is now abstract
	if(m_plbBarcode) {
		delete m_plbBarcode;
	}
}

void CLabBarcodeSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BARCODE_SETUP_NAME_LBL, m_nxsName);
	DDX_Control(pDX, IDC_BARCODE_SETUP_NAME_EDIT, m_nxeName);
	DDX_Control(pDX, IDC_BARCODE_SETUP_DESCRIPTION_LBL, m_nxsDescription);
	DDX_Control(pDX, IDC_BARCODE_SETUP_DESCRIPTION_EDIT, m_nxeDescription);
	DDX_Control(pDX, IDC_BARCODE_SETUP_TYPE_LBL, m_nxsTypeLabel);
	DDX_Control(pDX, IDC_BARCODE_SETUP_AUTO_RADIO, m_radioAutomatic);
	DDX_Control(pDX, IDC_BARCODE_SETUP_TEXT_RADIO, m_radioText);
	DDX_Control(pDX, IDC_BARCODE_SETUP_AUTOMATIC_LBL, m_nxsAutomaticLabel);
	DDX_Control(pDX, IDC_BARCODE_SETUP_TEXT_EDIT, m_nxeText);
	DDX_Control(pDX, IDC_BARCODE_SETUP_CUSTOMFIELD_RADIO, m_radioCustomField);
	DDX_Control(pDX, IDOK, m_nxbOK);
	DDX_Control(pDX, IDCANCEL, m_nxbCancel);
}

using namespace NXDATALIST2Lib;

// (r.gonet 10/17/2011) - PLID 45924 - Makes all controls reflect each other and the internal state.
void CLabBarcodeSetupDlg::EnsureControls()
{
	// Default everything
	m_nxeName.SetReadOnly(TRUE);
	m_nxeDescription.SetReadOnly(TRUE);
	m_nxsTypeLabel.ShowWindow(SW_HIDE);
	m_radioAutomatic.EnableWindow(FALSE);
	m_radioText.EnableWindow(FALSE);
	m_radioCustomField.EnableWindow(FALSE);
	m_radioAutomatic.ShowWindow(SW_HIDE);
	m_radioText.ShowWindow(SW_HIDE);
	m_radioCustomField.ShowWindow(SW_HIDE);
	m_nxsAutomaticLabel.ShowWindow(SW_HIDE);
	m_nxeText.ShowWindow(SW_HIDE);
	GetDlgItem(IDC_BARCODE_SETUP_CUSTOMFIELD_COMBO)->ShowWindow(SW_HIDE);
	//TES 7/24/2012 - PLID 50393 - Added a Lab Procedure dropdown, for top-level records only
	GetDlgItem(IDC_LAB_PROCEDURE_LBL)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_LAB_PROCEDURE)->ShowWindow(SW_HIDE);

	// Now if we have selected a part row, we need to ensure some controls are visible and color and expand the hierarchy.
	NXDATALIST2Lib::IRowSettingsPtr pPartRow = m_pPartsList->CurSel;
	if(pPartRow) {
		if(pPartRow->GetValue(bcplcFillType).vt != VT_NULL && (ELabBarcodePartFillType)VarLong(pPartRow->GetValue(bcplcFillType)) != lbpftContainer) {
			// When you select a row, you always have certain things shown, like the radio buttons.
			m_nxsTypeLabel.ShowWindow(SW_SHOW);
			m_radioAutomatic.ShowWindow(SW_SHOW);
			m_radioText.ShowWindow(SW_SHOW);
			m_radioCustomField.ShowWindow(SW_SHOW);

			// Certain fill types though require certain fields to be shown for them though.
			switch(VarLong(pPartRow->GetValue(bcplcFillType))) {
				case lbpftAuto:
					m_nxsAutomaticLabel.ShowWindow(SW_SHOW);
					break;
				case lbpftText:
					m_nxeText.ShowWindow(SW_SHOW);
					break;
				case lbpftField:
					GetDlgItem(IDC_BARCODE_SETUP_CUSTOMFIELD_COMBO)->ShowWindow(SW_SHOW);
					break;
				default:
					ASSERT(FALSE);
					break;
			}
		}
		//TES 7/24/2012 - PLID 50393 - If this row doesn't have a parent, show the Lab Procedure dropdown
		if(pPartRow->GetParentRow() == NULL) {
			GetDlgItem(IDC_LAB_PROCEDURE_LBL)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_LAB_PROCEDURE)->ShowWindow(SW_SHOW);
		}
	}

	// Go through the entire list and color the unfilled rows yellow and expand the rows when necessary.
	pPartRow = m_pPartsList->FindAbsoluteFirstRow(VARIANT_TRUE);
	while(pPartRow) {
		if((VarLong(pPartRow->GetValue(bcplcFillType), -1) == lbpftText && VarString(pPartRow->GetValue(bcplcTextValue), "") == "")
			|| (VarLong(pPartRow->GetValue(bcplcFillType), -1) == lbpftField && VarLong(pPartRow->GetValue(bcplcLabCustomFieldID), -1) == -1))
		{
			pPartRow->PutBackColor(RGB(255, 255, 0) );
			// Ensure this branch then is expanded to let the user know there is something here they need to attend to.
			NXDATALIST2Lib::IRowSettingsPtr pParentRow = pPartRow->GetParentRow();
			while(pParentRow != NULL) {
				pParentRow->Expanded = VARIANT_TRUE;
				pParentRow = pParentRow->GetParentRow();
			}
		} else {
			pPartRow->PutBackColor(RGB(255, 255, 255));
		}

		pPartRow = m_pPartsList->FindAbsoluteNextRow(pPartRow, VARIANT_FALSE);
	}
}

// (r.gonet 10/17/2011) - PLID 45924 - Syncs the controls the user has messed with back to the barcode part row.
void CLabBarcodeSetupDlg::SaveBarcodeFieldToRow(NXDATALIST2Lib::IRowSettingsPtr pRow) {
	if(pRow) {
		CString strTextValue;
		m_nxeText.GetWindowText(strTextValue);
		NXDATALIST2Lib::IRowSettingsPtr pFieldRow = m_pCustomFieldList->CurSel;
		long nCustomFieldID = pFieldRow ? VarLong(pFieldRow->GetValue(0), -1) : -1;

		switch(VarLong(pRow->GetValue(bcplcFillType), -1)) {
			case lbpftContainer:
				break;
			case lbpftAuto:
				break;
			case lbpftText:
				if(strTextValue != VarString(pRow->GetValue(bcplcTextValue), "")) {
					pRow->PutValue(bcplcTextValue, _variant_t(_bstr_t(strTextValue)));
					pRow->PutValue(bcplcModified, g_cvarTrue);
				}
				break;
			case lbpftField:
				if(nCustomFieldID != VarLong(pRow->GetValue(bcplcLabCustomFieldID), -1)) {
					pRow->PutValue(bcplcLabCustomFieldID, _variant_t(nCustomFieldID));
					pRow->PutValue(bcplcModified, g_cvarTrue);
				}
				break;
			default:
				break;
		}

		//TES 7/24/2012 - PLID 50393 - If this is a top-level row, copy the selected Lab Procedure into the datalist.
		if(pRow->GetParentRow() == NULL) {
			IRowSettingsPtr pProcedureRow = m_pLabProcedureList->CurSel;
			long nLabProcedureID = -1;
			if(pProcedureRow) {
				nLabProcedureID = VarLong(pProcedureRow->GetValue(lplcID), -1);
			}
			if(VarLong(pRow->GetValue(bcplcLabProcedureID), -1) != nLabProcedureID) {
				pRow->PutValue(bcplcLabProcedureID, nLabProcedureID);
				pRow->PutValue(bcplcModified, g_cvarTrue);
			}
		}
	}
}

// (r.gonet 10/17/2011) - PLID 45924 - Saves the barcode configuration the user has setup.
void CLabBarcodeSetupDlg::Save()
{
	// We may have a straggler row
	SaveBarcodeFieldToRow(m_pPartsList->CurSel);

	// (r.gonet 11/12/2011) - PLID 44212 - CLabBarcode is now abstract and must be a pointer
	if(!m_plbBarcode) {
		// We should have this allocated at this point. How did we get here without it being allocated?
		ASSERT(FALSE);
		return;
	}

	// Go through all rows in the datalist and update the part it is associated with. We'll save those later.
	NXDATALIST2Lib::IRowSettingsPtr pPartRow = m_pPartsList->FindAbsoluteFirstRow(VARIANT_FALSE);
	while(pPartRow) {
		if(VarBool(pPartRow->GetValue(bcplcModified), FALSE) != FALSE) {
			// (r.gonet 11/12/2011) - PLID 44212 - CLabBarcode is now abstract and must be a pointer
			CLabBarcodePartPtr pPart = m_plbBarcode->GetPart(pPartRow->GetValue(bcplcPath));
			//TES 7/24/2012 - PLID 50393 - If this row is top-level, save its selected LabProcedureID
			if(pPartRow->GetParentRow() == NULL) {
				pPart->SetLabProcedureID(VarLong(pPartRow->GetValue(bcplcLabProcedureID),-1));
			}
			else {				
				pPart->SetTextValue(pPartRow->GetValue(bcplcTextValue));
				pPart->SetLabCustomField(pPartRow->GetValue(bcplcLabCustomFieldID));
			}			
			pPart->SetModificationStatus(CLabBarcodePart::emsModified);
			pPartRow->PutValue(bcplcModified, g_cvarFalse);
		}

		pPartRow = m_pPartsList->FindAbsoluteNextRow(pPartRow, VARIANT_FALSE);
	}
	// (r.gonet 11/12/2011) - PLID 44212 - CLabBarcode is now abstract and must be a pointer
	// Save the barcode structure with all its parts to the database.
	m_plbBarcode->Save();
}

// (r.gonet 10/17/2011) - PLID 45924 - Callback for the barcode hierarchy traversal. Will add the barcode part to the appropriate space in the tree.
void LabBarcodeTraverseCallback(CLabBarcodePartPtr pPart, void *pArg)
{
	CLabBarcodeSetupDlg *pDlg = (CLabBarcodeSetupDlg *)pArg;
	pDlg->AddPart(pPart);	
}

void CLabBarcodeSetupDlg::Load()
{
	// (r.gonet 11/12/2011) - PLID 44212 - CLabBarcode is now abstract and must be a pointer
	if(!m_plbBarcode) {
		// Ensure we have a barcode by this point. We should though, so I don't think this branch is possible.
		m_plbBarcode = new CLabCorpLabBarcode();
	}
	// Load the parts into the barcode structure.
	m_plbBarcode->Load();
	// Now load our datalist with the hierarchy of parts. The hierarchy must be maintained so we must traverse the tree
	//  and add parts in a certain order.
	m_plbBarcode->PreOrderTraverse(LabBarcodeTraverseCallback, (void *)this);
}

// (r.gonet 10/17/2011) - PLID 45924 - Add a barcode part to the tree list
void CLabBarcodeSetupDlg::AddPart(CLabBarcodePartPtr pPart)
{
	// (r.gonet 11/12/2011) - PLID 44212 - CLabBarcode is now abstract and must be a pointer
	if(!m_plbBarcode) {
		// This should be allocated at this point!
		ASSERT(FALSE);
		return;
	}
	// (r.gonet 11/12/2011) - PLID 44212 - CLabBarcode is now abstract and must be a pointer
	if(pPart == m_plbBarcode->GetRoot()) {
		// Woah! We don't want the root part. That is not a real part of the barcode.
		return;
	}

	// Create a row based on the barcode part.
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pPartsList->GetNewRow();
	pRow->PutValue(bcplcID, _variant_t(pPart->GetID()));
	pRow->PutValue(bcplcPath, _variant_t(pPart->GetPath()));
	pRow->PutValue(bcplcDescription, _variant_t(pPart->GetDescription()));
	pRow->PutValue(bcplcMaxLength, _variant_t(pPart->GetMaxLength()));
	pRow->PutValue(bcplcFillType, _variant_t((long)pPart->GetFillType()));
	pRow->PutValue(bcplcTextValue, _variant_t(pPart->GetTextValue()));
	pRow->PutValue(bcplcLabCustomFieldID, _variant_t(pPart->GetLabCustomField()));
	pRow->PutValue(bcplcModified, g_cvarFalse);
	//TES 7/24/2012 - PLID 50393 - Remember the associated LabProcedureID
	pRow->PutValue(bcplcLabProcedureID, _variant_t(pPart->GetLabProcedureID()));
	// (r.gonet 11/12/2011) - PLID 44212 - CLabBarcode is now abstract and must be a pointer
	CLabBarcodePartPtr pParentPart = m_plbBarcode->GetParentPart(pPart->GetPath());
	if(pParentPart == m_plbBarcode->GetRoot()) {
		// Add this part as a top level part since its only parent is the root, which we don't print here.
		m_pPartsList->AddRowAtEnd(pRow, NULL);
	} else {
		// This part has a parent part. So find that parent part's row and then add this as the child row of that parent.
		NXDATALIST2Lib::IRowSettingsPtr pParentRow = m_pPartsList->FindByColumn(bcplcPath, _variant_t(pParentPart->GetPath()), m_pPartsList->FindAbsoluteFirstRow(VARIANT_TRUE), VARIANT_FALSE);
		if(pParentRow) {
			// From the way we traverse the tree, this branch should always be taken.
			m_pPartsList->AddRowAtEnd(pRow, pParentRow);
		}
	}
}

BEGIN_MESSAGE_MAP(CLabBarcodeSetupDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CLabBarcodeSetupDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CLabBarcodeSetupDlg message handlers

BOOL CLabBarcodeSetupDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		m_pPartsList = BindNxDataList2Ctrl(this, IDC_BARCODE_PARTS_LIST, GetRemoteData(), false);
		// Load the parts list with the hierarchy.
		Load();
		m_pCustomFieldList = BindNxDataList2Ctrl(this, IDC_BARCODE_SETUP_CUSTOMFIELD_COMBO, GetRemoteData(), false);
		m_pCustomFieldList->Requery();
		m_pCustomFieldList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);

		//TES 7/24/2012 - PLID 50393 - Added a dropdown with the list of lab procedures (shown for top-level records)
		m_pLabProcedureList = BindNxDataList2Ctrl(IDC_LAB_PROCEDURE);

		m_nxbOK.AutoSet(NXB_OK);
		m_nxbCancel.AutoSet(NXB_CANCEL);

		// Color and expand the rows.
		EnsureControls();

	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CLabBarcodeSetupDlg, CNxDialog)
	ON_EVENT(CLabBarcodeSetupDlg, IDC_BARCODE_PARTS_LIST, 2, CLabBarcodeSetupDlg::SelChangedBarcodePartsList, VTS_DISPATCH VTS_DISPATCH)
END_EVENTSINK_MAP()

// (r.gonet 10/17/2011) - PLID 45924 - Display the new configuration options for this barcode part, according to the specs.
void CLabBarcodeSetupDlg::SelChangedBarcodePartsList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		// Save the old row
		NXDATALIST2Lib::IRowSettingsPtr pOldRow(lpOldSel);
		if(pOldRow) {
			// Sync back the controls to the datalist row.
			SaveBarcodeFieldToRow(pOldRow);
		}

		// Now put the new selection's information into the control fields.
		NXDATALIST2Lib::IRowSettingsPtr pNewRow(lpNewSel);
		if(pNewRow) {
			CString strPath = pNewRow->GetValue(bcplcPath);
			CString strDescription = pNewRow->GetValue(bcplcDescription);
			m_nxeName.SetWindowText(strPath);
			m_nxeDescription.SetWindowText(strDescription);
			
			// Fill certain fields depending on the type of fill this part has.
			if(pNewRow->GetValue(bcplcFillType).vt != VT_NULL) {
				// Clear out all fields.
				m_radioAutomatic.SetCheck(FALSE);
				m_radioText.SetCheck(FALSE);
				m_radioCustomField.SetCheck(FALSE);
				m_nxeText.SetWindowText(_bstr_t(""));

				// Fill the radio correctly. And perhaps the textual value or custom field.
				switch(VarLong(pNewRow->GetValue(bcplcFillType))) {
					case lbpftContainer:
						break;
					case lbpftAuto:
						m_radioAutomatic.SetCheck(TRUE);
						break;
					case lbpftText:
						{
							CString strTextValue = VarString(pNewRow->GetValue(bcplcTextValue), "");
							m_radioText.SetCheck(TRUE);
							m_nxeText.SetWindowText(strTextValue);
							m_nxeText.SetLimitText(VarLong(pNewRow->GetValue(bcplcMaxLength), 255));
						}
						break;
					case lbpftField:
						{
							long nLabCustomFieldID = VarLong(pNewRow->GetValue(bcplcLabCustomFieldID), -1);
							m_radioCustomField.SetCheck(TRUE);
							m_pCustomFieldList->SetSelByColumn(0, _variant_t(nLabCustomFieldID));
						}
						break;
					default:
						ASSERT(FALSE);
						break;
				}
				
			}

			//TES 7/24/2012 - PLID 50393 - If this is a top-level row, load the LabProcedureID from the list.
			if(pNewRow->GetParentRow() == NULL) {
				m_pLabProcedureList->SetSelByColumn(lplcID, pNewRow->GetValue(bcplcLabProcedureID));
			}
		}
		// Things have been modified, so ensure our list is colored/expanded correctly and that the right fields show.
		EnsureControls();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 10/17/2011) - PLID 45924 - Save and close
void CLabBarcodeSetupDlg::OnBnClickedOk()
{
	try {
		Save();
		CNxDialog::OnOK();
	} NxCatchAll(__FUNCTION__);
}
