// ApptPrototypesDlg.cpp : implementation file
// (b.cardillo 2011-02-22 16:25) - PLID 40419 - Admin UI for managing Appointment Prototypes
//

#include "stdafx.h"
#include "Practice.h"
#include "ApptPrototypesDlg.h"
#include "ApptPrototypeEntryDlg.h"

using namespace NXDATALIST2Lib;

namespace EApptPrototypeListColumns {
	enum _Enum {
		ID,
		Name,
		Description,
	};
};

// CApptPrototypesDlg dialog

IMPLEMENT_DYNAMIC(CApptPrototypesDlg, CNxDialog)

CApptPrototypesDlg::CApptPrototypesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CApptPrototypesDlg::IDD, pParent)
{

}

CApptPrototypesDlg::~CApptPrototypesDlg()
{
}

void CApptPrototypesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ADD_BTN, m_btnAdd);
	DDX_Control(pDX, IDC_EDIT_BTN, m_btnEdit);
	DDX_Control(pDX, IDC_DELETE_BTN, m_btnDelete);
	DDX_Control(pDX, IDCANCEL, m_btnClose);
}


BEGIN_MESSAGE_MAP(CApptPrototypesDlg, CNxDialog)
	ON_BN_CLICKED(IDC_ADD_BTN, &CApptPrototypesDlg::OnBnClickedAddBtn)
	ON_BN_CLICKED(IDC_EDIT_BTN, &CApptPrototypesDlg::OnBnClickedEditBtn)
	ON_BN_CLICKED(IDC_DELETE_BTN, &CApptPrototypesDlg::OnBnClickedDeleteBtn)
END_MESSAGE_MAP()


// CApptPrototypesDlg message handlers
BEGIN_EVENTSINK_MAP(CApptPrototypesDlg, CNxDialog)
	ON_EVENT(CApptPrototypesDlg, IDC_APPOINTMENT_PROTOTYPE_LIST, 3, CApptPrototypesDlg::DblClickCellAppointmentPrototypeList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CApptPrototypesDlg, IDC_APPOINTMENT_PROTOTYPE_LIST, 29, CApptPrototypesDlg::SelSetAppointmentPrototypeList, VTS_DISPATCH)
END_EVENTSINK_MAP()

BOOL CApptPrototypesDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		// Set button styles
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnEdit.AutoSet(NXB_MODIFY);
		m_btnDelete.AutoSet(NXB_DELETE);
		m_btnClose.AutoSet(NXB_CLOSE);
		
		NXDATALIST2Lib::_DNxDataListPtr pdl = GetDlgItem(IDC_APPOINTMENT_PROTOTYPE_LIST)->GetControlUnknown();
		pdl->PutAdoConnection(GetRemoteData());
		pdl->Requery();
		ReflectEnabledControls(FALSE);
	} NxCatchAllCall(__FUNCTION__, { EndDialog(IDCANCEL); return FALSE; });

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CApptPrototypesDlg::GetOtherPrototypeNames(OUT CStringArray &arystrOtherNames, IRowSettings *lpExceptRow)
{
	NXDATALIST2Lib::_DNxDataListPtr pdl = GetDlgItem(IDC_APPOINTMENT_PROTOTYPE_LIST)->GetControlUnknown();
	LONG nExceptRowID;
	if (lpExceptRow != NULL) {
		IRowSettingsPtr pExceptRow = lpExceptRow;
		nExceptRowID = VarLong(pExceptRow->GetValue(EApptPrototypeListColumns::ID));
	} else {
		nExceptRowID = -1;
	}
	arystrOtherNames.RemoveAll();
	for (IRowSettingsPtr row = pdl->GetFirstRow(); row != NULL; row = row->GetNextRow()) {
		if (nExceptRowID == -1 || VarLong(row->GetValue(EApptPrototypeListColumns::ID)) != nExceptRowID) {
			arystrOtherNames.Add(VarString(row->GetValue(EApptPrototypeListColumns::Name)));
		}
	}
}

void CApptPrototypesDlg::EditApptPrototypeRow(IRowSettings *lpRow)
{
	IRowSettingsPtr row = lpRow;
	CStringArray arystrOtherNames;
	GetOtherPrototypeNames(arystrOtherNames, row);
	CApptPrototypeEntryDlg dlg(this);
	if (dlg.DoModal(VarLong(row->GetValue(EApptPrototypeListColumns::ID)), arystrOtherNames) == IDOK) {
		row->PutValue(EApptPrototypeListColumns::Name, AsVariant(dlg.GetCommittedPrototypeName()));
		row->PutValue(EApptPrototypeListColumns::Description, AsVariant(dlg.GetCommittedPrototypeDescription()));
	}
}

void CApptPrototypesDlg::DblClickCellAppointmentPrototypeList(LPDISPATCH lpRow, short nColIndex)
{
	try {
		if (lpRow != NULL && nColIndex != -1) {
			NXDATALIST2Lib::IRowSettingsPtr row(lpRow);
			EditApptPrototypeRow(row);
		}
	} NxCatchAll(__FUNCTION__);
}

void CApptPrototypesDlg::ReflectEnabledControls(BOOL bIsRowSelected)
{
	GetDlgItem(IDC_EDIT_BTN)->EnableWindow(bIsRowSelected);
	GetDlgItem(IDC_DELETE_BTN)->EnableWindow(bIsRowSelected);
}

void CApptPrototypesDlg::ReflectEnabledControls()
{
	BOOL bIsRowSelected;
	{
		NXDATALIST2Lib::_DNxDataListPtr pdl = GetDlgItem(IDC_APPOINTMENT_PROTOTYPE_LIST)->GetControlUnknown();
		if (pdl->GetCurSel() != NULL) {
			bIsRowSelected = TRUE;
		} else {
			bIsRowSelected = FALSE;
		}
	}

	ReflectEnabledControls(bIsRowSelected);
}

void CApptPrototypesDlg::OnBnClickedAddBtn()
{
	try {
		CStringArray arystrOtherNames;
		GetOtherPrototypeNames(arystrOtherNames, NULL);
		CApptPrototypeEntryDlg dlg(this);
		if (dlg.DoModal(-1, arystrOtherNames) == IDOK) {
			NXDATALIST2Lib::_DNxDataListPtr pdl = GetDlgItem(IDC_APPOINTMENT_PROTOTYPE_LIST)->GetControlUnknown();
			IRowSettingsPtr row = pdl->GetNewRow();
			row->PutValue(EApptPrototypeListColumns::ID, dlg.GetCommittedPrototypeID());
			row->PutValue(EApptPrototypeListColumns::Name, AsVariant(dlg.GetCommittedPrototypeName()));
			row->PutValue(EApptPrototypeListColumns::Description, AsVariant(dlg.GetCommittedPrototypeDescription()));
			row = pdl->AddRowSorted(row, NULL);
			pdl->PutCurSel(row);
			ReflectEnabledControls(TRUE);
		}
	} NxCatchAll(__FUNCTION__);
}

void CApptPrototypesDlg::OnBnClickedEditBtn()
{
	try {
		NXDATALIST2Lib::_DNxDataListPtr pdl = GetDlgItem(IDC_APPOINTMENT_PROTOTYPE_LIST)->GetControlUnknown();
		IRowSettingsPtr row = pdl->GetCurSel();
		if (row != NULL) {
			EditApptPrototypeRow(row);
		} else {
			MessageBox(_T("Please select an appointment prototype to edit."), NULL, MB_OK|MB_ICONEXCLAMATION);
		}
	} NxCatchAll(__FUNCTION__);
}

void CApptPrototypesDlg::OnBnClickedDeleteBtn()
{
	try {
		NXDATALIST2Lib::_DNxDataListPtr pdl = GetDlgItem(IDC_APPOINTMENT_PROTOTYPE_LIST)->GetControlUnknown();
		IRowSettingsPtr row = pdl->GetCurSel();
		if (row != NULL) {
			if (MessageBox(_T("Are you sure you want to delete appointment prototype '") + VarString(row->GetValue(EApptPrototypeListColumns::Name)) + _T("'?"), NULL, MB_OKCANCEL|MB_ICONQUESTION) == IDOK) {
				CParamSqlBatch sql;
				sql.Add(
					_T("DECLARE @prototypeID INT \r\n")
					_T("SET @prototypeID = {INT} \r\n")
					, VarLong(row->GetValue(EApptPrototypeListColumns::ID)));
				sql.Add(
					_T("DECLARE @tDeletePropertySets TABLE (ID INT NOT NULL) \r\n")
					_T("INSERT INTO @tDeletePropertySets (ID) \r\n")
					_T("SELECT ID FROM ApptPrototypePropertySetT WHERE ApptPrototypeID = @prototypeID \r\n")
					);
				// Delete PropertySet AptPurposeSets
				{
					sql.Add(
						_T("DECLARE @tDeleteAptPurposeSets TABLE (ID INT NOT NULL) \r\n")
						_T("INSERT INTO @tDeleteAptPurposeSets (ID) \r\n")
						_T("SELECT ID FROM ApptPrototypePropertySetAptPurposeSetT WHERE ApptPrototypePropertySetID IN (SELECT D.ID FROM @tDeletePropertySets D) \r\n")
						);
					sql.Add(
						_T("DELETE FROM ApptPrototypePropertySetAptPurposeSetDetailT \r\n")
						_T("WHERE ApptPrototypePropertySetAptPurposeSetID IN (SELECT D.ID FROM @tDeleteAptPurposeSets D)\r\n")
						);
					sql.Add(
						_T("DELETE FROM ApptPrototypePropertySetAptPurposeSetT \r\n")
						_T("WHERE ID IN (SELECT D.ID FROM @tDeleteAptPurposeSets D)\r\n")
						);
				}
				// Delete PropertySet ResourceSets
				{
					sql.Add(
						_T("DECLARE @tDeleteResourceSets TABLE (ID INT NOT NULL) \r\n")
						_T("INSERT INTO @tDeleteResourceSets (ID) \r\n")
						_T("SELECT ID FROM ApptPrototypePropertySetResourceSetT WHERE ApptPrototypePropertySetID IN (SELECT D.ID FROM @tDeletePropertySets D) \r\n")
						);
					sql.Add(
						_T("DELETE FROM ApptPrototypePropertySetResourceSetDetailT \r\n")
						_T("WHERE ApptPrototypePropertySetResourceSetID IN (SELECT D.ID FROM @tDeleteResourceSets D)\r\n")
						);
					sql.Add(
						_T("DELETE FROM ApptPrototypePropertySetResourceSetT \r\n")
						_T("WHERE ID IN (SELECT D.ID FROM @tDeleteResourceSets D)\r\n")
						);
				}
				// Delete PropertySet AptTypes
				{
					sql.Add(
						_T("DELETE FROM ApptPrototypePropertySetAptTypeT \r\n")
						_T("WHERE ApptPrototypePropertySetID IN (SELECT D.ID FROM @tDeletePropertySets D) \r\n")
						);
				}
				// Delete PropertySet Locations
				{
					sql.Add(
						_T("DELETE FROM ApptPrototypePropertySetLocationT \r\n")
						_T("WHERE ApptPrototypePropertySetID IN (SELECT D.ID FROM @tDeletePropertySets D) \r\n")
						);
				}
				// Delete PropertySets
				{
					sql.Add(
						_T("DELETE FROM ApptPrototypePropertySetT \r\n")
						_T("WHERE ID IN (SELECT D.ID FROM @tDeletePropertySets D) \r\n")
						);
				}
				// Finally delete the Prototype itself
				{
					sql.Add(
						_T("DELETE FROM ApptPrototypeT \r\n")
						_T("WHERE ID = @prototypeID \r\n")
						);
				}
				// Now run it to actually commit the deletion in data
				sql.Execute(GetRemoteData());
				// And remove it from on screen
				pdl->RemoveRow(row);
				// Reflect the enabled state
				ReflectEnabledControls();
			}
		} else {
			MessageBox(_T("Please select an appointment prototype to delete."), NULL, MB_OK|MB_ICONEXCLAMATION);
		}
	} NxCatchAll(__FUNCTION__);
}

void CApptPrototypesDlg::SelSetAppointmentPrototypeList(LPDISPATCH lpSel)
{
	try {
		ReflectEnabledControls(lpSel != NULL);
	} NxCatchAll(__FUNCTION__);
}
