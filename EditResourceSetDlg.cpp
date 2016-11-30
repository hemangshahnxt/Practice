// EditResourceSetDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EditResourceSetDlg.h"


// CEditResourceSetDlg dialog

// (a.walling 2010-06-15 15:49) - PLID 39184 - Configuration dialog for a resource set

enum EResourceListColumns {
	lcID = 0,
	lcName,
	lcOrderIndex,
};

enum EUsedOnListColumns {
	lcAptTypeID = 0,
	lcAptType,
	lcAptPurposeID,
	lcAptPurpose,
};

IMPLEMENT_DYNAMIC(CEditResourceSetDlg, CNxDialog)

CEditResourceSetDlg::CEditResourceSetDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEditResourceSetDlg::IDD, pParent)
	, m_nResourceSetID(-1)
{

}

CEditResourceSetDlg::~CEditResourceSetDlg()
{
}

void CEditResourceSetDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_RESOURCE_MOVEUP, m_btnMoveUp);
	DDX_Control(pDX, IDC_BTN_RESOURCE_MOVEDOWN, m_btnMoveDown);
	DDX_Control(pDX, IDC_EDIT_RESOURCE_SET_NAME, m_editName);
	DDX_Control(pDX, IDC_STATIC_RESOURCE_NAME_LABEL, m_nxsNameLabel);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_NXCOLORCTRL1, m_bkgColor);
	DDX_Control(pDX, IDC_STATIC_RESOURCE_SET_USED_WITH_LABEL, m_nxsUsedWithLabel);
}


BEGIN_MESSAGE_MAP(CEditResourceSetDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_RESOURCE_MOVEDOWN, &CEditResourceSetDlg::OnBnClickedBtnMovedown)
	ON_BN_DOUBLECLICKED(IDC_BTN_RESOURCE_MOVEDOWN, &CEditResourceSetDlg::OnBnDoubleclickedBtnMovedown)
	ON_BN_CLICKED(IDC_BTN_RESOURCE_MOVEUP, &CEditResourceSetDlg::OnBnClickedBtnMoveup)
	ON_BN_DOUBLECLICKED(IDC_BTN_RESOURCE_MOVEUP, &CEditResourceSetDlg::OnBnDoubleclickedBtnMoveup)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CEditResourceSetDlg, CNxDialog)
	ON_EVENT(CEditResourceSetDlg, IDC_LIST_RESOURCE_SET, 7, CEditResourceSetDlg::RButtonUpListResourceSet, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEditResourceSetDlg, IDC_LIST_RESOURCES, 16, CEditResourceSetDlg::SelChosenListResources, VTS_DISPATCH)
	ON_EVENT(CEditResourceSetDlg, IDC_LIST_RESOURCE_SET, 6, CEditResourceSetDlg::RButtonDownListResourceSet, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEditResourceSetDlg, IDC_LIST_USED_ON, 6, CEditResourceSetDlg::RButtonDownListUsedOn, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEditResourceSetDlg, IDC_LIST_USED_ON, 7, CEditResourceSetDlg::RButtonUpListUsedOn, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEditResourceSetDlg, IDC_LIST_RESOURCE_SET, 28, CEditResourceSetDlg::CurSelWasSetListResourceSet, VTS_NONE)
END_EVENTSINK_MAP()

// CEditResourceSetDlg message handlers

BOOL CEditResourceSetDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	BOOL bRet = TRUE;

	try {
		m_btnMoveUp.AutoSet(NXB_UP);
		m_btnMoveDown.AutoSet(NXB_DOWN);
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_editName.LimitText(50);

		m_bkgColor.SetColor(GetNxColor(GNC_ADMIN, 0));
		m_pResourceCombo = BindNxDataList2Ctrl(this, IDC_LIST_RESOURCES, GetRemoteData(), true);
		m_pResourceSetList = BindNxDataList2Ctrl(this, IDC_LIST_RESOURCE_SET, GetRemoteData(), false);
		m_pResourceSetUsedOn = BindNxDataList2Ctrl(this, IDC_LIST_USED_ON, GetRemoteData(), false);

		CString strWhere;
		strWhere.Format("ResourceSetID = %li", m_nResourceSetID);
		m_pResourceSetList->WhereClause = (LPCTSTR)strWhere;
		m_pResourceSetUsedOn->WhereClause = (LPCTSTR)strWhere;

		if (m_nResourceSetID != -1) {
			ADODB::_RecordsetPtr prs = CreateParamRecordset("SELECT Name FROM ResourceSetT WHERE ID = {INT}", m_nResourceSetID);
			if (!prs->eof) {
				m_editName.SetWindowText(AdoFldString(prs, "Name", ""));
			}
			m_pResourceSetUsedOn->Requery();
			m_pResourceSetList->Requery();
		} else {
			bRet = FALSE;
			m_editName.SetFocus();
		}
		m_pResourceCombo->PutComboBoxText("Add a new resource");

		UpdateArrowButtons();

	} NxCatchAll(__FUNCTION__);

	return bRet;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CEditResourceSetDlg::OnBnClickedBtnMovedown()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pResourceSetList->CurSel;

		if (pRow) {
			NXDATALIST2Lib::IRowSettingsPtr pNextRow = pRow->GetNextRow();

			if (pNextRow) {
				_variant_t varOrderIndex = pNextRow->GetValue(lcOrderIndex);
				pNextRow->PutValue(lcOrderIndex, pRow->GetValue(lcOrderIndex));
				pRow->PutValue(lcOrderIndex, varOrderIndex);

				m_pResourceSetList->Sort();
			}
		}

		UpdateArrowButtons();
	} NxCatchAll(__FUNCTION__);
}

void CEditResourceSetDlg::OnBnDoubleclickedBtnMovedown()
{
	try {
		OnBnClickedBtnMovedown();
	} NxCatchAll(__FUNCTION__);
}

void CEditResourceSetDlg::OnBnClickedBtnMoveup()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pResourceSetList->CurSel;

		if (pRow) {
			NXDATALIST2Lib::IRowSettingsPtr pPreviousRow = pRow->GetPreviousRow();

			if (pPreviousRow) {
				_variant_t varOrderIndex = pPreviousRow->GetValue(lcOrderIndex);
				pPreviousRow->PutValue(lcOrderIndex, pRow->GetValue(lcOrderIndex));
				pRow->PutValue(lcOrderIndex, varOrderIndex);

				m_pResourceSetList->Sort();
			}
		}
		
		UpdateArrowButtons();
	} NxCatchAll(__FUNCTION__);
}

void CEditResourceSetDlg::OnBnDoubleclickedBtnMoveup()
{
	try {
		OnBnClickedBtnMoveup();
	} NxCatchAll(__FUNCTION__);
}

void CEditResourceSetDlg::RButtonUpListResourceSet(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {

			CMenu menu;
			menu.CreatePopupMenu();

			menu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, -1, "Remove");

			CPoint pt;
			GetCursorPos(&pt);

			int nCmdId = menu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this, NULL);
			if (nCmdId == -1) {
				m_pResourceSetList->RemoveRow(pRow);
				UpdateArrowButtons();
			}
		}

	} NxCatchAll(__FUNCTION__);
}

void CEditResourceSetDlg::SelChosenListResources(LPDISPATCH lpRow)
{
	try {
		m_pResourceSetList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {
			_variant_t var = pRow->GetValue(lcID);
			NXDATALIST2Lib::IRowSettingsPtr pExistingRow = m_pResourceSetList->FindByColumn(lcID, var, NULL, VARIANT_FALSE);

			if (pExistingRow == NULL) {
				long nOrderIndex = 0;
				NXDATALIST2Lib::IRowSettingsPtr pLastRow = m_pResourceSetList->GetLastRow();
				if (pLastRow) {
					nOrderIndex = VarLong(pLastRow->GetValue(lcOrderIndex));
					nOrderIndex++;
				}

				pExistingRow = m_pResourceSetList->GetNewRow();

				pExistingRow->PutValue(lcID, pRow->GetValue(lcID));
				pExistingRow->PutValue(lcName, pRow->GetValue(lcName));
				pExistingRow->PutValue(lcOrderIndex, nOrderIndex);

				m_pResourceSetList->AddRowAtEnd(pExistingRow, NULL);
			}
			
			m_pResourceSetList->FindByColumn(lcID, var, NULL, VARIANT_TRUE);

			UpdateArrowButtons();
			
			// (a.walling 2010-06-15 15:49) - PLID 39184 - Change the focus to the existing list so the mousewheel won't fire this message over and over
			GetDlgItem(IDC_LIST_RESOURCE_SET)->SetFocus();
		}

	} NxCatchAll(__FUNCTION__);
}

void CEditResourceSetDlg::OnOK()
{
	try {
		CString strName;
		m_editName.GetWindowText(strName);
		strName.TrimRight();
		strName.TrimLeft();

		if (strName.IsEmpty()) {
			MessageBox("The name of this resource set cannot be empty.", NULL, MB_ICONSTOP);
			return;
		}

		CParamSqlBatch batch;
		batch.Declare("SET NOCOUNT ON;");
		batch.Declare("DECLARE @ResourceSetID INT;");
		if (m_nResourceSetID != -1) {
			batch.Add("SET @ResourceSetID = {INT};", m_nResourceSetID);
			batch.Add("UPDATE ResourceSetT SET Name = {STRING} WHERE ID = @ResourceSetID;", strName);
			batch.Add("DELETE FROM ResourceSetDetailsT WHERE ResourceSetID = @ResourceSetID;");
		} else {
			batch.Add("INSERT INTO ResourceSetT(Name) VALUES({STRING});", strName);
			batch.Add("SET @ResourceSetID = SCOPE_IDENTITY();");
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pResourceSetList->GetFirstRow();
		long nOrderIndex = 0;
		while (pRow) {
			batch.Add("INSERT INTO ResourceSetDetailsT(ResourceSetID, ResourceID, OrderIndex) VALUES(@ResourceSetID, {INT}, {INT});",
				VarLong(pRow->GetValue(lcID)), nOrderIndex);
			
			nOrderIndex++;

			pRow = pRow->GetNextRow();
		}

		if (nOrderIndex == 0) {
			if (m_nResourceSetID == -1) {
				if (IDOK == MessageBox("This new resource set does not have any resources. It will not be saved.", NULL, MB_OKCANCEL|MB_ICONINFORMATION)) {
					CNxDialog::OnCancel();
				}
				return;
			} else {
				if (IDCANCEL == MessageBox("This resource set does not have any resources. It will be deleted.", NULL, MB_OKCANCEL|MB_ICONEXCLAMATION)) {					
					return;
				}
			}
		}

		{
			batch.Add("SELECT CONVERT(INT, @ResourceSetID) AS ResourceSetID;");
			batch.Declare("SET NOCOUNT OFF;");

			ADODB::_RecordsetPtr prs = batch.CreateRecordset(GetRemoteData());
			if (!prs->eof) {
				m_nResourceSetID = AdoFldLong(prs, "ResourceSetID");
			}
		
			// (a.walling 2010-06-16 17:47) - PLID 39184 - Need to ensure that links are cleared out before clearing out the empty resource sets
			ExecuteSqlStd(
				"BEGIN TRAN "
				""
				"DELETE EmptyAptResourceSetLinksT "
				"FROM AptResourceSetLinksT "
					"EmptyAptResourceSetLinksT "
				"INNER JOIN ResourceSetT "
					"ON EmptyAptResourceSetLinksT.ResourceSetID = ResourceSetT.ID "
				"LEFT JOIN ResourceSetDetailsT "
					"ON ResourceSetT.ID = ResourceSetDetailsT.ResourceSetID "
				"WHERE ResourceSetDetailsT.ResourceSetID IS NULL "
				""
				"IF @@ERROR <> 0 BEGIN ROLLBACK TRAN RETURN END "
				""
				"DELETE EmptyResourceSetsT "
				"FROM ResourceSetT "
					"EmptyResourceSetsT "
				"LEFT JOIN ResourceSetDetailsT "
					"ON EmptyResourceSetsT.ID = ResourceSetDetailsT.ResourceSetID "
				"WHERE ResourceSetDetailsT.ResourceSetID IS NULL "
				""
				"COMMIT TRAN"
			);
		}
	
		CNxDialog::OnOK();
		
	} NxCatchAll(__FUNCTION__);
}

void CEditResourceSetDlg::RButtonDownListResourceSet(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		m_pResourceSetList->CurSel = pRow;
	} NxCatchAll(__FUNCTION__);
}


void CEditResourceSetDlg::RButtonDownListUsedOn(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		m_pResourceSetUsedOn->CurSel = pRow;
	} NxCatchAll(__FUNCTION__);
}

void CEditResourceSetDlg::RButtonUpListUsedOn(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {

			CMenu menu;
			menu.CreatePopupMenu();

			menu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, -1, "Remove");

			CPoint pt;
			GetCursorPos(&pt);

			int nCmdId = menu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this, NULL);
			if (nCmdId == -1) {
				int nAptTypeID = VarLong(pRow->GetValue(lcAptTypeID));
				int nAptPurposeID = VarLong(pRow->GetValue(lcAptPurposeID), -1);

				if (IDNO == MessageBox("Are you sure you want to remove this link? This cannot be undone.", "", MB_ICONQUESTION | MB_YESNO)) {
					return;
				}

				CString strQuery;
				if (nAptPurposeID == -1) {
					ExecuteParamSql(
						"DELETE FROM AptResourceSetLinksT "
						"WHERE AptTypeID = {INT} "
							"AND AptPurposeID IS NULL "
							"AND ResourceSetID = {INT}",
						nAptTypeID, m_nResourceSetID);
				} else {
					ExecuteParamSql(
						"DELETE FROM AptResourceSetLinksT "
						"WHERE AptTypeID = {INT} "
						"AND AptPurposeID = {INT} "
							"AND ResourceSetID = {INT}",
						nAptTypeID, nAptPurposeID, m_nResourceSetID);
				}

				m_pResourceSetUsedOn->RemoveRow(pRow);
			}
		}

	} NxCatchAll(__FUNCTION__);
}

void CEditResourceSetDlg::CurSelWasSetListResourceSet()
{
	UpdateArrowButtons();
}

void CEditResourceSetDlg::UpdateArrowButtons()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pResourceSetList->CurSel;

		BOOL bCanMoveUp = FALSE;
		BOOL bCanMoveDown = FALSE;

		if (pRow) {
			if (pRow->GetNextRow()) {
				bCanMoveDown = TRUE;
			}

			if (pRow->GetPreviousRow()) {
				bCanMoveUp = TRUE;
			}
		}

		m_btnMoveUp.EnableWindow(bCanMoveUp);
		m_btnMoveDown.EnableWindow(bCanMoveDown);

	} NxCatchAll(__FUNCTION__);
}