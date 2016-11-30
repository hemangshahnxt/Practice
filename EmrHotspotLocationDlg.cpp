// EmrHotspotLocationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AdministratorRc.h"
#include "EmrHotspotLocationDlg.h"
#include "EMRHotSpot.h"
#include "EditComboBox.h"
#include "GlobalLabUtils.h"

//TES 2/9/2010 - PLID 37223 - Created
// CEmrHotspotLocationDlg dialog

// (a.walling 2014-07-02 11:53) - PLID 62696 - These queries were repeated thrice each
namespace {
	boost::optional<CString> GetQualifierName(long id)
	{
		if (id != -1) {
			ADODB::_RecordsetPtr rsQual = CreateParamRecordset("SELECT Name FROM AnatomyQualifiersT WHERE ID = {INT}", id);
			if (!rsQual->eof) {
				return AdoFldString(rsQual, "Name", "");
			}
			else {
				return boost::none;
			}
		}
		return "<no qualifier>";
	}

	boost::optional<CString> GetAnatomicLocationName(long id)
	{
		if (id != -1) {
			ADODB::_RecordsetPtr rsLocation = CreateParamRecordset("SELECT Description FROM LabAnatomyT WHERE ID = {INT}", id);
			if (!rsLocation->eof) {
				return AdoFldString(rsLocation, "Description", "");
			}
			else {
				return boost::none;
			}
		}
		return "";
	}
}

IMPLEMENT_DYNAMIC(CEmrHotspotLocationDlg, CNxDialog)

CEmrHotspotLocationDlg::CEmrHotspotLocationDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEmrHotspotLocationDlg::IDD, pParent)
{
	m_pSpot = NULL;

	m_nCurrentLocationID = -1;
	m_nCurrentQualifierID = -1;
}

CEmrHotspotLocationDlg::~CEmrHotspotLocationDlg()
{
}

void CEmrHotspotLocationDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEmrHotspotLocationDlg)
	DDX_Control(pDX, IDOK, m_nxbOK);
	DDX_Control(pDX, IDCANCEL, m_nxbCancel);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_HOTSPOT_LEFT_SIDE, m_checkLeft);
	DDX_Control(pDX, IDC_HOTSPOT_RIGHT_SIDE, m_checkRight);
	DDX_Control(pDX, IDC_MODIFY_LOCATIONS, m_nxbModifyLocations);
	DDX_Control(pDX, IDC_MODIFY_QUALIFIERS, m_nxbModifyQualifiers);
}


BEGIN_MESSAGE_MAP(CEmrHotspotLocationDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CEmrHotspotLocationDlg::OnOK)
	ON_BN_CLICKED(IDC_HOTSPOT_LEFT_SIDE, &CEmrHotspotLocationDlg::OnHotspotLeftSide)
	ON_BN_CLICKED(IDC_HOTSPOT_RIGHT_SIDE, &CEmrHotspotLocationDlg::OnHotspotRightSide)
	ON_BN_CLICKED(IDC_MODIFY_LOCATIONS, &CEmrHotspotLocationDlg::OnModifyLocations)
	ON_BN_CLICKED(IDC_MODIFY_QUALIFIERS, &CEmrHotspotLocationDlg::OnModifyQualifiers)
END_MESSAGE_MAP()

enum LocationColumns
{
	lcID = 0,
	lcName = 1,
};

enum QualifierColumns
{
	qcID = 0,
	qcName = 1,
};

using namespace NXDATALIST2Lib;
using namespace ADODB;

// CEmrHotspotLocationDlg message handlers
BOOL CEmrHotspotLocationDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		//TES 2/9/2010 - PLID 37223 - Set our NxButtons
		m_nxbOK.AutoSet(NXB_OK);
		m_nxbCancel.AutoSet(NXB_CANCEL);

		//TES 2/9/2010 - PLID 37223 - Initialize our datalists.
		m_pLocations = BindNxDataList2Ctrl(IDC_HOTSPOT_LOCATION);
		m_pQualifiers = BindNxDataList2Ctrl(IDC_HOTSPOT_QUALIFIER);

		//TES 2/9/2010 - PLID 37223 - Set the location (copied from LabRequisitionDlg)
		m_nCurrentLocationID = m_pSpot->GetAnatomicLocationID();
		if(m_nCurrentLocationID != -1) {
			long nTrySetResult = m_pLocations->TrySetSelByColumn_Deprecated(lcID, m_nCurrentLocationID);
			// (c.haag 2010-01-28 16:18) - PLID 36776 - The combo text is actually used when saving. We should never
			// have it be a sentinel value (e.g. "Loading..."). If the combo is not done requerying yet, just run a parameter
			// query to get the name and put it in the combo text.
			if(nTrySetResult == sriNoRowYet_WillFireEvent || nTrySetResult == sriNoRow) {
				//TES 11/10/2009 - PLID 36128 - It must be inactive
				auto oName = GetAnatomicLocationName(m_nCurrentLocationID);
				if(oName) {
					m_pLocations->PutComboBoxText((const char*)oName.get());
				}
				else {
					//Wha???
					// (c.haag 2010-03-17 16:43) - PLID 37223 - This can actually happen if the record was deleted from data while
					// the hotspot setup window was open. Don't throw an exception; just set the location ID to -1.
					//ThrowNxException("Invalid Anatomic Location %li found!", m_nCurrentLocationID);
					m_nCurrentLocationID = -1;
				}
			}
		}

		//TES 2/9/2010 - PLID 37223 - Set the qualifier (copied from LabRequisitionDlg)
		m_nCurrentQualifierID = m_pSpot->GetAnatomicQualifierID();
		long nTrySetResult = m_pQualifiers->TrySetSelByColumn_Deprecated(qcID, m_nCurrentQualifierID);
		// (c.haag 2010-01-28 16:18) - PLID 36776 - The combo text is actually used when saving. We should never
		// have it be a sentinel value (e.g. "Loading..."). If the combo is not done requerying yet, just run a parameter
		// query to get the name and put it in the combo text.
		if(nTrySetResult == sriNoRowYet_WillFireEvent || nTrySetResult == sriNoRow) {
			//TES 11/10/2009 - PLID 36128 - It must be inactive

			// (a.walling 2014-07-02 11:53) - PLID 62696 - Show <no qualifier> instead of a blank row
			auto oName = GetQualifierName(m_nCurrentQualifierID);
			if (!oName) {
				//Wha???
				// (c.haag 2010-03-17 16:43) - PLID 37223 - This can actually happen if the record was deleted from data while
				// the hotspot setup window was open. Don't throw an exception; just set the qualifier ID to -1.
				//ThrowNxException("Invalid Anatomic Location Qualifier %li found!", m_nCurrentQualifierID);
				m_nCurrentQualifierID = -1;
				oName = "<no qualifier>";
			}
			m_pQualifiers->PutComboBoxText((const char*)oName.get());
		}

		//TES 2/9/2010 - PLID 37223 - Set the side
		AnatomySide as = m_pSpot->GetSide();
		if(as == asLeft) {
			CheckDlgButton(IDC_HOTSPOT_LEFT_SIDE, BST_CHECKED);
		}
		else if(as == asRight) {
			CheckDlgButton(IDC_HOTSPOT_RIGHT_SIDE, BST_CHECKED);
		}

	}NxCatchAll(__FUNCTION__);
	return TRUE;
}

void CEmrHotspotLocationDlg::OnOK()
{
	try {
		//TES 2/9/2010 - PLID 37223 - If the combo text is in use, they can't have changed the location.
		if(!m_pLocations->IsComboBoxTextInUse) {
			if(m_pLocations->CurSel) {
				//TES 2/10/2010 - PLID 37296 - Added names, for auditing.
				m_pSpot->SetAnatomicLocation(VarLong(m_pLocations->CurSel->GetValue(lcID)), VarString(m_pLocations->CurSel->GetValue(lcName)));
			}
			else {
				//TES 2/10/2010 - PLID 37296 - Added names, for auditing.
				m_pSpot->SetAnatomicLocation(-1, "");
			}
		}

		//TES 2/9/2010 - PLID 37223 - If the combo text is in use, they can't have changed the qualifier.
		if(!m_pQualifiers->IsComboBoxTextInUse) {
			if(m_pQualifiers->CurSel) {
				//TES 2/10/2010 - PLID 37296 - Added names, for auditing.
				m_pSpot->SetAnatomicQualifier(VarLong(m_pQualifiers->CurSel->GetValue(qcID)), VarString(m_pQualifiers->CurSel->GetValue(qcName)));
			}
			else {
				//TES 2/10/2010 - PLID 37296 - Added names, for auditing.
				m_pSpot->SetAnatomicQualifier(-1, "");
			}
		}

		//TES 2/9/2010 - PLID 37223 - Save the side they checked.
		AnatomySide asNew = asNone;
		if(IsDlgButtonChecked(IDC_HOTSPOT_LEFT_SIDE)) {
			asNew = asLeft;
		}
		else if(IsDlgButtonChecked(IDC_HOTSPOT_RIGHT_SIDE)) {
			asNew = asRight;
		}
		m_pSpot->SetSide(asNew);
	}NxCatchAll(__FUNCTION__);

	CNxDialog::OnOK();
}
BEGIN_EVENTSINK_MAP(CEmrHotspotLocationDlg, CNxDialog)
	ON_EVENT(CEmrHotspotLocationDlg, IDC_HOTSPOT_LOCATION, 18, CEmrHotspotLocationDlg::OnRequeryFinishedHotspotLocation, VTS_I2)
	ON_EVENT(CEmrHotspotLocationDlg, IDC_HOTSPOT_LOCATION, 20, CEmrHotspotLocationDlg::OnTrySetSelFinishedHotspotLocation, VTS_I4 VTS_I4)
	ON_EVENT(CEmrHotspotLocationDlg, IDC_HOTSPOT_QUALIFIER, 18, CEmrHotspotLocationDlg::OnRequeryFinishedHotspotQualifier, VTS_I2)
	ON_EVENT(CEmrHotspotLocationDlg, IDC_HOTSPOT_QUALIFIER, 20, CEmrHotspotLocationDlg::OnTrySetSelFinishedHotspotQualifier, VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

void CEmrHotspotLocationDlg::OnRequeryFinishedHotspotLocation(short nFlags)
{
	try {
		//TES 2/9/2010 - PLID 37223 - Added, based off LabRequisitionDlg
		if(m_nCurrentLocationID != -1) {
			if (NULL == m_pLocations->FindByColumn(lcID, m_nCurrentLocationID, NULL, VARIANT_TRUE)) {
				//TES 11/6/2009 - PLID 36189 - It may be inactive
				auto oName = GetAnatomicLocationName(m_nCurrentLocationID);
				if(oName) {
					m_pLocations->PutComboBoxText((const char*)oName.get());
				}
				else {
					// (c.haag 2010-03-17 16:13) - PLID 37223 - This can happen if the user deletes the currently selected 
					// anatomic location. The CEditComboBox object does not have support for checking memory (as opposed
					// to data). In this situation, just inform the user and clear the selection. (This is Tom's item but I'm helping out)
					MessageBox("The currently selected Anatomic Location has been deleted from your data. The selection will now be reset.", "NexTech Practice", MB_OK | MB_ICONERROR);
					m_pLocations->PutCurSel(m_pLocations->GetFirstRow());
				}	
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CEmrHotspotLocationDlg::OnTrySetSelFinishedHotspotLocation(long nRowEnum, long nFlags)
{
	try {
		//TES 2/9/2010 - PLID 37223 - Added, based off LabRequisitionDlg
		if(nFlags == dlTrySetSelFinishedFailure) {
			if(m_nCurrentLocationID > 0) {
				//TES 11/6/2009 - PLID 36189 - It may be inactive
				auto oName = GetAnatomicLocationName(m_nCurrentLocationID);
				if(oName) {
					m_pLocations->PutComboBoxText((const char*)oName.get());
					return;
				}
			}
			//Else if it failed to load anything, check if we need to clear the combo box text
			if(m_pLocations->GetIsComboBoxTextInUse() != FALSE)
				m_pLocations->PutComboBoxText("");
		}
	}NxCatchAll(__FUNCTION__);
}

void CEmrHotspotLocationDlg::OnRequeryFinishedHotspotQualifier(short nFlags)
{
	try {
		//TES 2/9/2010 - PLID 37223 - Added, based off LabRequisitionDlg
		// (a.walling 2014-07-02 11:53) - PLID 62696 - Show <no qualifier> instead of a blank row
		if (NULL == m_pQualifiers->FindByColumn(qcID, m_nCurrentQualifierID, NULL, VARIANT_TRUE)) {
			//TES 11/6/2009 - PLID 36189 - It may be inactive
			auto oName = GetQualifierName(m_nCurrentQualifierID);
			if (!oName) {
				// (c.haag 2010-03-17 16:13) - PLID 37223 - This can happen if the user deletes the currently selected 
				// anatomic location. The CEditComboBox object does not have support for checking memory (as opposed
				// to data). In this situation, just inform the user and clear the selection. (This is Tom's item but I'm helping out)
				MessageBox("The currently selected Location Qualifier has been deleted from your data. The selection will now be reset.", "NexTech Practice", MB_OK | MB_ICONERROR);
				m_pQualifiers->PutCurSel(m_pQualifiers->GetFirstRow());
			} else {
				m_pQualifiers->PutComboBoxText((const char*)oName.get());
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CEmrHotspotLocationDlg::OnTrySetSelFinishedHotspotQualifier(long nRowEnum, long nFlags)
{
	try {
		//TES 2/9/2010 - PLID 37223 - Added, based off LabRequisitionDlg
		if(nFlags == dlTrySetSelFinishedFailure) {
			// (a.walling 2014-07-02 11:53) - PLID 62696 - Show <no qualifier> instead of a blank row
			//TES 11/6/2009 - PLID 36189 - It may be inactive
			auto oName = GetQualifierName(m_nCurrentQualifierID);
			if (!oName) {
				//Else if it failed to load anything, check if we need to clear the combo box text
				if (m_pQualifiers->GetIsComboBoxTextInUse()) {
					m_pQualifiers->PutComboBoxText("<no qualifier>");
				}
			}
			else {
				m_pQualifiers->PutComboBoxText((const char*)oName.get());
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CEmrHotspotLocationDlg::OnHotspotLeftSide()
{
	try {
		//TES 2/9/2010 - PLID 37223 - It can't be both left and right.
		CheckDlgButton(IDC_HOTSPOT_RIGHT_SIDE, BST_UNCHECKED);
	}NxCatchAll(__FUNCTION__);
}

void CEmrHotspotLocationDlg::OnHotspotRightSide()
{
	try {
		//TES 2/9/2010 - PLID 37223 - It can't be both left and right.
		CheckDlgButton(IDC_HOTSPOT_LEFT_SIDE, BST_UNCHECKED);
	}NxCatchAll(__FUNCTION__);
}

void CEmrHotspotLocationDlg::OnModifyLocations()
{
	try {
		//TES 2/10/2010 - PLID 37223 - Added, copied from CLabRequisitionDlg
		//(e.lally 2006-07-07) PLID 21356 - Added anatomic locations to the edit combo box supported objects.
		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		CEditComboBox dlg(this, 14, "Edit Anatomic Location List");

		//TES 11/6/2009 - PLID 36189 - Don't pass in the datalist, since it filters out inactive
		IRowSettingsPtr pRow = m_pLocations->GetCurSel();
		if (pRow != NULL) {
			m_nCurrentLocationID = VarLong(pRow->GetValue(lcID),-1);

			// (j.jones 2007-07-20 12:13) - PLID 26749 - set the
			// current anatomy ID, if we have one
			if(m_nCurrentLocationID > 0)
				dlg.m_nCurIDInUse = m_nCurrentLocationID;
		}
		else {
			//TES 11/6/2009 - PLID 36189 - It may be inactive
			dlg.m_nCurIDInUse = m_nCurrentLocationID;
		}

		dlg.DoModal();

		//TES 11/6/2009 - PLID 36189 - EditComboBox won't requery automatically for us any more.
		m_pLocations->Requery();

		// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
		m_pLocations->TrySetSelByColumn_Deprecated(0, m_nCurrentLocationID);
	}NxCatchAll(__FUNCTION__);
}

void CEmrHotspotLocationDlg::OnModifyQualifiers()
{
	try {
		//TES 2/10/2010 - PLID 37223 - Added, copied from CLabRequisitionDlg
		//TES 11/10/2009 - PLID 36128 - Replaced the Left/Right checkboxes with an editable list, function copied from OnEditAnatomyList().
		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		CEditComboBox dlg(this, 70, "Edit Anatomic Location Qualifier List");

		//TES 11/6/2009 - PLID 36189 - Don't pass in the datalist, since it filters out inactive
		IRowSettingsPtr pRow = m_pQualifiers->GetCurSel();
		if (pRow != NULL) {
			m_nCurrentQualifierID = VarLong(pRow->GetValue(qcID),-1);

			// (j.jones 2007-07-20 12:13) - PLID 26749 - set the
			// current anatomy ID, if we have one
			if(m_nCurrentQualifierID > 0)
				dlg.m_nCurIDInUse = m_nCurrentQualifierID;
		}
		else {
			//TES 11/6/2009 - PLID 36189 - It may be inactive
			dlg.m_nCurIDInUse = m_nCurrentQualifierID;
		}

		dlg.DoModal();

		//TES 11/6/2009 - PLID 36189 - EditComboBox won't requery automatically for us any more.
		m_pQualifiers->Requery();

		// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
		m_pQualifiers->TrySetSelByColumn_Deprecated(0, m_nCurrentQualifierID);

	}NxCatchAll(__FUNCTION__);
}
