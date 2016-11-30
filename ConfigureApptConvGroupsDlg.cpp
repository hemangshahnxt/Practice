// ConfigureApptConvGroupsDlg.cpp : implementation file
//
// (j.gruber 2011-05-06 16:31) - PLID 43550 - created for

#include "stdafx.h"
#include "Practice.h"
#include "Marketingrc.h"
#include "ConfigureApptConvGroupsDlg.h"

enum ApptConvGroupList {
	acgID =0,
	acgName,
	acgDays,
};

enum AvailTypeColumns {
	atcID = 0,
	atcName,
};

enum SelectedTypeColumns {
	stcID = 0,
	stcName,
};

enum AvailCodeColumns {
	accID = 0,
	accType,
	accCode,
	accName,
};

enum SelectedCodeColumns {
	sccID = 0,
	sccType,
	sccCode,
	sccName,
};
// CConfigureApptConvGroupsDlg dialog

IMPLEMENT_DYNAMIC(CConfigureApptConvGroupsDlg, CNxDialog)

CConfigureApptConvGroupsDlg::CConfigureApptConvGroupsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CConfigureApptConvGroupsDlg::IDD, pParent)
{

}

CConfigureApptConvGroupsDlg::~CConfigureApptConvGroupsDlg()
{
}

void CConfigureApptConvGroupsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ACG_ADD, m_btnAdd);
	DDX_Control(pDX, IDC_ACG_DELETE, m_btnDelete);
	DDX_Control(pDX, IDC_ACG_RENAME, m_btnRename);
	DDX_Control(pDX, IDC_TYPE_MOVE_LEFT, m_btnTypeLeft);
	DDX_Control(pDX, IDC_TYPE_MOVE_RIGHT, m_btnTypeRight);
	DDX_Control(pDX, IDC_CODE_MOVE_LEFT, m_btnCodeLeft);
	DDX_Control(pDX, IDC_CODE_MOVE_RIGHT, m_btnCodeRight);
	DDX_Control(pDX, IDC_ACG_CLOSE, m_btnClose);
}

BOOL CConfigureApptConvGroupsDlg::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();

		//setup the icons
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnDelete.AutoSet(NXB_DELETE);
		m_btnRename.AutoSet(NXB_MODIFY);
		m_btnTypeLeft.AutoSet(NXB_LEFT);
		m_btnTypeRight.AutoSet(NXB_RIGHT);
		m_btnCodeLeft.AutoSet(NXB_LEFT);
		m_btnCodeRight.AutoSet(NXB_RIGHT);
		m_btnClose.AutoSet(NXB_CLOSE);		


		//bind the datalists
		m_pConvGroupList = BindNxDataList2Ctrl(IDC_APPT_CONV_GROUP_LIST, true);
		m_pTypeAvailList = BindNxDataList2Ctrl(IDC_APPT_TYPE_AVAIL_LIST, false);
		m_pTypeSelectedList = BindNxDataList2Ctrl(IDC_APPT_TYPE_SELECTED_LIST, false);
		m_pCodeAvailList = BindNxDataList2Ctrl(IDC_CODE_AVAIL_LIST, false);
		m_pCodeSelectedList = BindNxDataList2Ctrl(IDC_CODE_SELECT_LIST, false);		

		//purposely leave inactives in
		m_pTypeAvailList->FromClause = "AptTypeT ";
		m_pTypeSelectedList->FromClause = "AptTypeT ";

		CString strFrom;
		strFrom = "(SELECT ServiceT.ID, Name, 'Inventory Item' as Type, '' as Code FROM ServiceT INNER JOIN ProductT ON ServiceT.ID = ProductT.ID "
			" UNION ALL "
			" SELECT ServiceT.ID, Name, 'Service Code', CASE WHEN Subcode <> '' THEN LTrim(RTrim(Code)) + ' - ' + LTrim(RTrim(SubCode)) else Code END AS Code FROM ServiceT INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID) ServicesQ ";
		m_pCodeAvailList->FromClause = _bstr_t(strFrom);
		m_pCodeSelectedList->FromClause = _bstr_t(strFrom);

		CString strInstructions;
		strInstructions.Format("Choose the appointment types that you would like to track the conversion of that resulted in a charge of certain service codes and/or inventory items within X days of the appointment.");
		SetDlgItemText(IDC_ACG_INSTRUCTIONS, strInstructions);

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}


BEGIN_MESSAGE_MAP(CConfigureApptConvGroupsDlg, CNxDialog)
	ON_BN_CLICKED(IDC_ACG_ADD, &CConfigureApptConvGroupsDlg::OnBnClickedAcgAdd)
	ON_BN_CLICKED(IDC_ACG_RENAME, &CConfigureApptConvGroupsDlg::OnBnClickedAcgRename)
	ON_BN_CLICKED(IDC_ACG_DELETE, &CConfigureApptConvGroupsDlg::OnBnClickedAcgDelete)
	ON_BN_CLICKED(IDC_TYPE_MOVE_LEFT, &CConfigureApptConvGroupsDlg::OnBnClickedTypeMoveLeft)
	ON_BN_CLICKED(IDC_TYPE_MOVE_RIGHT, &CConfigureApptConvGroupsDlg::OnBnClickedTypeMoveRight)
	ON_BN_CLICKED(IDC_CODE_MOVE_RIGHT, &CConfigureApptConvGroupsDlg::OnBnClickedCodeMoveRight)
	ON_BN_CLICKED(IDC_CODE_MOVE_LEFT, &CConfigureApptConvGroupsDlg::OnBnClickedCodeMoveLeft)
	ON_BN_CLICKED(IDC_ACG_CLOSE, &CConfigureApptConvGroupsDlg::OnBnClickedAcgClose)
	ON_EN_KILLFOCUS(IDC_CONVERSION_DAYS, &CConfigureApptConvGroupsDlg::OnEnKillfocusConversionDays)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CConfigureApptConvGroupsDlg, CNxDialog)
	ON_EVENT(CConfigureApptConvGroupsDlg, IDC_APPT_TYPE_AVAIL_LIST, 3, CConfigureApptConvGroupsDlg::DblClickCellApptTypeAvailList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CConfigureApptConvGroupsDlg, IDC_APPT_TYPE_SELECTED_LIST, 3, CConfigureApptConvGroupsDlg::DblClickCellApptTypeSelectedList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CConfigureApptConvGroupsDlg, IDC_CODE_AVAIL_LIST, 3, CConfigureApptConvGroupsDlg::DblClickCellCodeAvailList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CConfigureApptConvGroupsDlg, IDC_CODE_SELECT_LIST, 3, CConfigureApptConvGroupsDlg::DblClickCellCodeSelectList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CConfigureApptConvGroupsDlg, IDC_APPT_CONV_GROUP_LIST, 16, CConfigureApptConvGroupsDlg::SelChosenApptConvGroupList, VTS_DISPATCH)
	ON_EVENT(CConfigureApptConvGroupsDlg, IDC_APPT_CONV_GROUP_LIST, 18, CConfigureApptConvGroupsDlg::RequeryFinishedApptConvGroupList, VTS_I2)
END_EVENTSINK_MAP()


// CConfigureApptConvGroupsDlg message handlers

void CConfigureApptConvGroupsDlg::OnBnClickedAcgAdd()
{
	try {
		CString strName;
		if(InputBox(this, "Enter a conversion group name", strName, "") == IDOK) {

			if (strName.IsEmpty()) {
				MsgBox("Please enter a non-blank name.");
				return;
			}

			if (strName.GetLength() > 50) {
				MsgBox("Pleae enter a name with 50 characters or less.");
				return;
			}

			//make sure there isn't already a name of this
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pConvGroupList->GetFirstRow();
			while (pRow) {
				CString strGroupName = VarString(pRow->GetValue(acgName), "");

				if (strGroupName.CompareNoCase(strName) == 0) {
					MsgBox("There is already a group with this name.");
					return;
				}
				pRow = pRow->GetNextRow();
			}

			//if we got here we are good to go
			ADODB::_RecordsetPtr rs = CreateParamRecordset(" SET NOCOUNT ON; "
				" DECLARE @nNewID INT; "
				" IF @@ERROR <> 0 BEGIN ROLLBACK TRAN RETURN END\r\n" 
				" SET @nNewID = (SELECT Coalesce(Max(ID), 0) + 1 FROM ApptServiceConvGroupsT); "
				" IF @@ERROR <> 0 BEGIN ROLLBACK TRAN RETURN END\r\n" 
				" INSERT INTO ApptServiceConvGroupsT(ID, Name) VALUES (@nNewID, {STRING}) "
				" IF @@ERROR <> 0 BEGIN ROLLBACK TRAN RETURN END\r\n" 
				" SET NOCOUNT OFF; "
				" IF @@ERROR <> 0 BEGIN ROLLBACK TRAN RETURN END\r\n" 
				" SELECT @nNewID as ID; ",
				strName);
			pRow = m_pConvGroupList->GetNewRow();
			if (pRow) {
				pRow->PutValue(acgID, AdoFldLong(rs->Fields, "ID"));
				pRow->PutValue(acgName, _bstr_t(strName));
				//we default to 1 year
				pRow->PutValue(acgDays, 365);

				m_pConvGroupList->AddRowSorted(pRow, NULL);
				m_pConvGroupList->CurSel = pRow;
				LoadDialog(pRow);
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CConfigureApptConvGroupsDlg::OnBnClickedAcgRename()
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pCurSel = m_pConvGroupList->CurSel;

		if (pCurSel) {
			long nCurID = VarLong(pCurSel->GetValue(acgID));
			CString strCurName = VarString(pCurSel->GetValue(acgName));

			CString strName;
			if(InputBox(this, "Enter a new conversion group name", strName, "") == IDOK) {

				if (strName.IsEmpty()) {
					MsgBox("Please enter a non-blank name.");
					return;
				}

				if (strCurName == strName) {
					return;
				}

				if (strName.GetLength() > 50) {
					MsgBox("Please enter a name with 50 characters or less");
					return;
				}

				//make sure there isn't already a name of this
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pConvGroupList->GetFirstRow();
				while (pRow) {
					CString strGroupName = VarString(pRow->GetValue(acgName), "");
					long nGroupID = VarLong(pRow->GetValue(acgID));

					if (nGroupID != nCurID) {

						if (strGroupName.CompareNoCase(strName) == 0) {
							MsgBox("There is already a group with this name.");
							return;
						}
					}
					pRow = pRow->GetNextRow();
				}

				//if we got here we are good to go
				ExecuteParamSql("UPDATE ApptServiceConvGroupsT SET Name = {STRING} WHERE ID = {INT}", strName, nCurID);
				pCurSel->PutValue(acgName, _bstr_t(strName));									
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CConfigureApptConvGroupsDlg::OnBnClickedAcgDelete()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pConvGroupList->CurSel;
		if (pRow) {
			if (MsgBox(MB_YESNO, "Are you sure you want to delete this group?  This action is unrecoverable!") == IDYES) {

				long nGroupID = VarLong(pRow->GetValue(acgID));

				ExecuteParamSql(" DELETE FROM ApptServiceConvServicesT WHERE GroupID = {INT}; "
					" IF @@ERROR <> 0 BEGIN ROLLBACK TRAN RETURN END\r\n" 
					" DELETE FROM ApptServiceConvTypesT WHERE GroupID = {INT}; "
					" IF @@ERROR <> 0 BEGIN ROLLBACK TRAN RETURN END\r\n" 
					" DELETE FROM ApptServiceConvGroupsT WHERE ID = {INT}; ",
					nGroupID, nGroupID, nGroupID);

				m_pConvGroupList->RemoveRow(pRow);
				LoadDialog(m_pConvGroupList->GetFirstRow());
			}
		}

	}NxCatchAll(__FUNCTION__);
}


void CConfigureApptConvGroupsDlg::DblClickCellApptTypeAvailList(LPDISPATCH lpRow, short nColIndex)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			MoveTypeRight(pRow);
		}
	}NxCatchAll(__FUNCTION__);
}

void CConfigureApptConvGroupsDlg::OnBnClickedTypeMoveLeft()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTypeSelectedList->CurSel;
		if (pRow) {
			MoveTypeLeft(pRow);
		}
	}NxCatchAll(__FUNCTION__);
}

void CConfigureApptConvGroupsDlg::OnBnClickedTypeMoveRight()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTypeAvailList->CurSel;
		if (pRow) {
			MoveTypeRight(pRow);
		}
	}NxCatchAll(__FUNCTION__);
}

void CConfigureApptConvGroupsDlg::DblClickCellApptTypeSelectedList(LPDISPATCH lpRow, short nColIndex)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			MoveTypeLeft(pRow);
		}
	}NxCatchAll(__FUNCTION__);
}

void CConfigureApptConvGroupsDlg::DblClickCellCodeAvailList(LPDISPATCH lpRow, short nColIndex)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			MoveCodeRight(pRow);
		}
	}NxCatchAll(__FUNCTION__);
}

void CConfigureApptConvGroupsDlg::OnBnClickedCodeMoveRight()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCodeAvailList->CurSel;
		if (pRow) {
			MoveCodeRight(pRow);
		}
	}NxCatchAll(__FUNCTION__);
}

void CConfigureApptConvGroupsDlg::OnBnClickedCodeMoveLeft()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCodeSelectedList->CurSel;
		if (pRow) {
			MoveCodeLeft(pRow);
		}
	}NxCatchAll(__FUNCTION__);
}

void CConfigureApptConvGroupsDlg::DblClickCellCodeSelectList(LPDISPATCH lpRow, short nColIndex)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			MoveCodeLeft(pRow);
		}
	}NxCatchAll(__FUNCTION__);
}

void CConfigureApptConvGroupsDlg::SelChosenApptConvGroupList(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		LoadDialog(pRow);
	}NxCatchAll(__FUNCTION__);
}

void CConfigureApptConvGroupsDlg::RequeryFinishedApptConvGroupList(short nFlags)
{
	try {

		//set the dialog to the first selection, if exists
		LoadDialog(m_pConvGroupList->GetFirstRow());

	}NxCatchAll(__FUNCTION__);
}

void CConfigureApptConvGroupsDlg::LoadDialog(NXDATALIST2Lib::IRowSettingsPtr pRow)
{

	try {

		if (pRow) {

			long nGroupID = VarLong(pRow->GetValue(acgID));

			m_pConvGroupList->CurSel = pRow;

			CString strWhere;
			strWhere.Format(" AptTypeT.ID NOT IN (SELECT ApptTypeID FROM ApptServiceConvTypesT WHERE GroupID = %li)", nGroupID); 
			m_pTypeAvailList->WhereClause = _bstr_t(strWhere);

			strWhere.Format(" AptTypeT.ID IN (SELECT ApptTypeID FROM ApptServiceConvTypesT WHERE GroupID = %li)", nGroupID); 
			m_pTypeSelectedList->WhereClause = _bstr_t(strWhere);

			strWhere.Format(" ServicesQ.ID NOT IN (SELECT ServiceID FROM ApptServiceConvServicesT WHERE GroupID = %li)", nGroupID); 
			m_pCodeAvailList->WhereClause = _bstr_t(strWhere);

			strWhere.Format(" ServicesQ.ID IN (SELECT ServiceID FROM ApptServiceConvServicesT WHERE GroupID = %li)", nGroupID); 
			m_pCodeSelectedList->WhereClause = _bstr_t(strWhere);

			long nDays = VarLong(pRow->GetValue(acgDays));
			SetDlgItemInt(IDC_CONVERSION_DAYS, nDays);
			
			m_pTypeAvailList->Clear(); 
			m_pTypeAvailList->ReadOnly = FALSE;
			m_pTypeSelectedList->Clear(); 
			m_pTypeSelectedList->ReadOnly = FALSE; 
			m_pCodeAvailList->Clear();  
			m_pCodeAvailList->ReadOnly = FALSE;  
			m_pCodeSelectedList->Clear(); 
			m_pCodeSelectedList->ReadOnly = FALSE;

			m_pTypeAvailList->Requery();
			m_pTypeSelectedList->Requery();
			m_pCodeAvailList->Requery();
			m_pCodeSelectedList->Requery();
			
			GetDlgItem(IDC_CONVERSION_DAYS)->EnableWindow(TRUE);
		}
		else {
			m_pTypeAvailList->Clear(); 
			m_pTypeAvailList->ReadOnly = TRUE;
			m_pTypeSelectedList->Clear(); 
			m_pTypeSelectedList->ReadOnly = TRUE; 
			m_pCodeAvailList->Clear();  
			m_pCodeAvailList->ReadOnly = TRUE;  
			m_pCodeSelectedList->Clear(); 
			m_pCodeSelectedList->ReadOnly = TRUE;

			SetDlgItemText(IDC_CONVERSION_DAYS, "");
			GetDlgItem(IDC_CONVERSION_DAYS)->EnableWindow(FALSE);
			
		}		

	}NxCatchAll(__FUNCTION__);
}

void CConfigureApptConvGroupsDlg::MoveTypeRight(NXDATALIST2Lib::IRowSettingsPtr pRow)
{

	try {

		NXDATALIST2Lib::IRowSettingsPtr pGroupRow = m_pConvGroupList->CurSel;

		if (pGroupRow) {

			long nGroupID = VarLong(pGroupRow->GetValue(acgID));

			if (pRow) {
				
				long nTypeID = VarLong(pRow->GetValue(atcID));

				ExecuteParamSql("INSERT INTO ApptServiceConvTypesT (GroupID, ApptTypeID) "
					" VALUES ({INT}, {INT}) ",
					nGroupID, nTypeID);

				m_pTypeSelectedList->TakeRowAddSorted(pRow);
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CConfigureApptConvGroupsDlg::MoveTypeLeft(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pGroupRow = m_pConvGroupList->CurSel;

		if (pGroupRow) {

			long nGroupID = VarLong(pGroupRow->GetValue(acgID));

			if (pRow) {
				
				long nTypeID = VarLong(pRow->GetValue(stcID));

				ExecuteParamSql("DELETE FROM ApptServiceConvTypesT WHERE GroupID = {INT} AND ApptTypeID = {INT} ",				
					nGroupID, nTypeID);

				m_pTypeAvailList->TakeRowAddSorted(pRow);
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CConfigureApptConvGroupsDlg::MoveCodeRight(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pGroupRow = m_pConvGroupList->CurSel;

		if (pGroupRow) {

			long nGroupID = VarLong(pGroupRow->GetValue(acgID));

			if (pRow) {
				
				long nServiceID = VarLong(pRow->GetValue(accID));

				ExecuteParamSql("INSERT INTO ApptServiceConvServicesT (GroupID, ServiceID) "
					" VALUES ({INT}, {INT}) ",
					nGroupID, nServiceID);

				m_pCodeSelectedList->TakeRowAddSorted(pRow);
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CConfigureApptConvGroupsDlg::MoveCodeLeft(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
		try {

		NXDATALIST2Lib::IRowSettingsPtr pGroupRow = m_pConvGroupList->CurSel;

		if (pGroupRow) {

			long nGroupID = VarLong(pGroupRow->GetValue(acgID));

			if (pRow) {
				
				long nServiceID = VarLong(pRow->GetValue(sccID));

				ExecuteParamSql("DELETE FROM ApptServiceConvServicesT WHERE GROUPID = {INT} AND ServiceID = {INT} ",					
					nGroupID, nServiceID);

				m_pCodeAvailList->TakeRowAddSorted(pRow);
			}
		}
	}NxCatchAll(__FUNCTION__);
}
void CConfigureApptConvGroupsDlg::OnBnClickedAcgClose()
{
	try {
		OnOK();
	}NxCatchAll(__FUNCTION__);
}

void CConfigureApptConvGroupsDlg::OnEnKillfocusConversionDays()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pGroupRow = m_pConvGroupList->CurSel;
		if (pGroupRow) {
			long nGroupID = VarLong(pGroupRow->GetValue(acgID));

			CString strDays;
			GetDlgItemText(IDC_CONVERSION_DAYS, strDays);
			if (strDays.IsEmpty()) {
				SetDlgItemInt(IDC_CONVERSION_DAYS, 0);
			}
			long nDays = GetDlgItemInt(IDC_CONVERSION_DAYS);
			if (nDays < 0) {
				nDays = 0;
				SetDlgItemInt(IDC_CONVERSION_DAYS, 0);
			}
			
			ExecuteParamSql("UPDATE ApptServiceConvGroupsT SET ConversionDayLimit = {INT} WHERE ID = {INT}",
				nDays, nGroupID);			
		}

	}NxCatchAll(__FUNCTION__);
}

void CConfigureApptConvGroupsDlg::OnCancel() 
{
	try {
		CNxDialog::OnCancel();
	}NxCatchAll(__FUNCTION__);
}
