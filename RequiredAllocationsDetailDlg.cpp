// RequiredAllocationsDetailDlg.cpp : implementation file
//

#include "stdafx.h"
#include "inventoryrc.h"
#include "RequiredAllocationsDetailDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//TES 6/12/2008 - PLID 28078 - Created
/////////////////////////////////////////////////////////////////////////////
// CRequiredAllocationsDetailDlg dialog


CRequiredAllocationsDetailDlg::CRequiredAllocationsDetailDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CRequiredAllocationsDetailDlg::IDD, pParent)
{
	m_nID = -1;
	//{{AFX_DATA_INIT(CRequiredAllocationsDetailDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CRequiredAllocationsDetailDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRequiredAllocationsDetailDlg)
	DDX_Control(pDX, IDOK, m_nxbOK);
	DDX_Control(pDX, IDCANCEL, m_nxbCancel);
	DDX_Control(pDX, IDC_REQUIRED_ALLOCATION_DETAIL_DESCRIPTION, m_nxeDescription);
	DDX_Control(pDX, IDC_REMOVE_REQUIRED_TYPE, m_nxbRemoveType);
	DDX_Control(pDX, IDC_REMOVE_REQUIRED_PURPOSE, m_nxbRemovePurpose);
	DDX_Control(pDX, IDC_REMOVE_ALL_REQUIRED_PURPOSES, m_nxbRemoveAllPurposes);
	DDX_Control(pDX, IDC_ADD_REQUIRED_TYPE, m_nxbAddType);
	DDX_Control(pDX, IDC_ADD_REQUIRED_PURPOSE, m_nxbAddPurpose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRequiredAllocationsDetailDlg, CNxDialog)
	//{{AFX_MSG_MAP(CRequiredAllocationsDetailDlg)
	ON_BN_CLICKED(IDC_ADD_REQUIRED_TYPE, OnAddRequiredType)
	ON_BN_CLICKED(IDC_ADD_REQUIRED_PURPOSE, OnAddRequiredPurpose)
	ON_BN_CLICKED(IDC_REMOVE_ALL_REQUIRED_PURPOSES, OnRemoveAllRequiredPurposes)
	ON_BN_CLICKED(IDC_REMOVE_REQUIRED_PURPOSE, OnRemoveRequiredPurpose)
	ON_BN_CLICKED(IDC_REMOVE_REQUIRED_TYPE, OnRemoveRequiredType)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRequiredAllocationsDetailDlg message handlers

using namespace NXDATALIST2Lib;
using namespace ADODB;

BOOL CRequiredAllocationsDetailDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		m_nxbAddType.AutoSet(NXB_RIGHT);
		m_nxbRemoveType.AutoSet(NXB_LEFT);
		m_nxbAddPurpose.AutoSet(NXB_RIGHT);
		m_nxbRemovePurpose.AutoSet(NXB_LEFT);
		m_nxbRemoveAllPurposes.AutoSet(NXB_LLEFT);
		m_nxbOK.AutoSet(NXB_OK);
		m_nxbCancel.AutoSet(NXB_CANCEL);

		//TES 8/4/2008 - PLID 28078 - Limit the size of the description.
		m_nxeDescription.SetLimitText(255);

		//TES 6/12/2008 - PLID 28078 - Load the description we were passed in.
		SetDlgItemText(IDC_REQUIRED_ALLOCATION_DETAIL_DESCRIPTION, m_strDescription);

		//TES 6/12/2008 - PLID 28078 - Requery all the lists based on the current requirement (may be -1)
		CString strTypeWhere;		
		strTypeWhere.Format("Inactive = 0 AND ID NOT IN (SELECT AptTypeID FROM ApptsRequiringAllocationsDetailT "
			"WHERE ParentID = %li)", m_nID);
		m_pAvailTypes = BindNxDataList2Ctrl(IDC_AVAILABLE_TYPES, false);
		m_pAvailTypes->WhereClause = _bstr_t(strTypeWhere);
		m_pAvailTypes->Requery();

		strTypeWhere.Format("ID IN (SELECT AptTypeID FROM ApptsRequiringAllocationsDetailT WHERE ParentID = %li)", m_nID);
		m_pSelectedTypes = BindNxDataList2Ctrl(IDC_SELECTED_TYPES, false);
		m_pSelectedTypes->WhereClause = _bstr_t(strTypeWhere);
		m_pSelectedTypes->Requery();

		// (c.haag 2008-12-29 15:55) - PLID 32580 - Hide inactive procedures from the purpose lists unless they already belong
		// to this detail.
		CString strPurposeWhere;
		strPurposeWhere.Format("ID NOT IN (SELECT AptPurposeID FROM ApptsRequiringAllocationsDetailT WHERE ParentID = %li) "
			"AND ID NOT IN (SELECT ID FROM ProcedureT WHERE Inactive = 1) "
			,m_nID);
		m_pAvailPurposes = BindNxDataList2Ctrl(IDC_AVAILABLE_PURPOSES_LIST, false);
		m_pAvailPurposes->WhereClause = _bstr_t(strPurposeWhere);
		m_pAvailPurposes->Requery();

		strPurposeWhere.Format("ID IN (SELECT AptPurposeID FROM ApptsRequiringAllocationsDetailT WHERE ParentID = %li)",
			m_nID);
		m_pSelectedPurposes = BindNxDataList2Ctrl(IDC_SELECTED_PURPOSES_LIST, false);		
		m_pSelectedPurposes->WhereClause = _bstr_t(strPurposeWhere);
		m_pSelectedPurposes->Requery();

	}NxCatchAll("Error in CRequiredAllocationsDetailDlg::OnInitDialog()");
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRequiredAllocationsDetailDlg::OnOK() 
{
	try {
		//TES 6/12/2008 - PLID 28078 - Validate the description.
		CString strDescription;
		GetDlgItemText(IDC_REQUIRED_ALLOCATION_DETAIL_DESCRIPTION, strDescription);
		if(strDescription.IsEmpty()) {
			MsgBox("Please enter a description for this requirement.");
			m_nxeDescription.SetFocus();
			return;
		}
		else if(ReturnsRecords("SELECT ID FROM ApptsRequiringAllocationsT WHERE Description = '%s' AND ID <> %li",
			_Q(strDescription), m_nID)) {
			MsgBox("The description '%s' is already in use for a different requirement.  Please enter a unique description for "
				"this requirement", strDescription);
			return;
		}

		m_strDescription = strDescription;

		//TES 6/12/2008 - PLID 28078 - Now, validate the types/purposes.
		if(m_pSelectedTypes->GetRowCount() == 0) {
			MsgBox("Please select at least one type to require allocations for");
			return;
		}

		if(m_pSelectedPurposes->GetRowCount() == 0) {
			MsgBox("Please select at least one purpose to require allocations for");
			return;
		}

		//TES 6/12/2008 - PLID 28078 - We're validated, so construct a SQL batch (can't be parameterized, there's not currently
		// a way for a parameterized batch to return records).
		CString strSql;
		AddDeclarationToSqlBatch(strSql, "DECLARE @nID int;");
		if(m_nID == -1) {
			AddStatementToSqlBatch(strSql, "SELECT @nID = COALESCE(Max(ID),0)+1 FROM ApptsRequiringAllocationsT;");
			AddStatementToSqlBatch(strSql, "INSERT INTO ApptsRequiringAllocationsT (ID, Description) "
				"VALUES (@nID, '%s')", _Q(m_strDescription));
		}
		else {
			AddStatementToSqlBatch(strSql, "SELECT @nID = %li;", m_nID);
			AddStatementToSqlBatch(strSql, "UPDATE ApptsRequiringAllocationsT SET Description = '%s' WHERE ID = @nID",
				_Q(m_strDescription));
		}

		//TES 6/12/2008 - PLID 28078 - Next, the details.
		AddStatementToSqlBatch(strSql, "DELETE FROM ApptsRequiringAllocationsDetailT WHERE ParentID = @nID");
		IRowSettingsPtr pTypeRow = m_pSelectedTypes->GetFirstRow();
		while(pTypeRow) {
			long nTypeID = VarLong(pTypeRow->GetValue(0));
			
			IRowSettingsPtr pPurposeRow = m_pSelectedPurposes->GetFirstRow();
			while(pPurposeRow) {
				AddStatementToSqlBatch(strSql, "INSERT INTO ApptsRequiringAllocationsDetailT (ParentID, AptTypeID, AptPurposeID) "
					"VALUES (@nID, %li, %li)", nTypeID, VarLong(pPurposeRow->GetValue(0)));
				pPurposeRow = pPurposeRow->GetNextRow();
			}
			
			pTypeRow = pTypeRow->GetNextRow();
		}

		if(m_nID == -1) {
			//TES 6/12/2008 - PLID 28078 - We need to get results out, so we can't use ExecuteSqlBatch().
			_RecordsetPtr rsNewID = CreateRecordset(
				"SET NOCOUNT ON \r\n"
				"BEGIN TRAN \r\n"
				"%s "
				"COMMIT TRAN \r\n"
				"SET NOCOUNT OFF \r\n"
				"SELECT @nID AS NewID \r\n",
				strSql);
			m_nID = AdoFldLong(rsNewID, "NewID");
		}
		else {
			//TES 6/12/2008 - PLID 28078 - We don't need any output, so just run ExecuteSqlBatch().
			ExecuteSqlBatch(strSql);
		}

			
		CNxDialog::OnOK();

	}NxCatchAll("Error in CRequiredAllocationsDetailDlg::OnOK()");

}

BEGIN_EVENTSINK_MAP(CRequiredAllocationsDetailDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CRequiredAllocationsDetailDlg)
	ON_EVENT(CRequiredAllocationsDetailDlg, IDC_AVAILABLE_PURPOSES_LIST, 3 /* DblClickCell */, OnDblClickCellAvailablePurposesList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CRequiredAllocationsDetailDlg, IDC_AVAILABLE_TYPES, 3 /* DblClickCell */, OnDblClickCellAvailableTypes, VTS_DISPATCH VTS_I2)
	ON_EVENT(CRequiredAllocationsDetailDlg, IDC_SELECTED_PURPOSES_LIST, 3 /* DblClickCell */, OnDblClickCellSelectedPurposesList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CRequiredAllocationsDetailDlg, IDC_SELECTED_TYPES, 3 /* DblClickCell */, OnDblClickCellSelectedTypes, VTS_DISPATCH VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CRequiredAllocationsDetailDlg::OnAddRequiredType() 
{
	try {
		AddCurrentType();
	}NxCatchAll("Error in CRequiredAllocationsDetailDlg::OnAddRequiredType()");
}

void CRequiredAllocationsDetailDlg::OnAddRequiredPurpose() 
{
	try {
		AddCurrentPurpose();
	}NxCatchAll("Error in CRequiredAllocationsDetailDlg::OnAddRequiredPurpose()");
	
}

void CRequiredAllocationsDetailDlg::OnRemoveAllRequiredPurposes() 
{
	try {
		//TES 6/12/2008 - PLID 28078 - Move everything over.
		m_pAvailPurposes->TakeAllRows(m_pSelectedPurposes);
	}NxCatchAll("Error in CRequiredAllocationsDetailDlg::OnRemoveAllRequiredPurposes()");
}

void CRequiredAllocationsDetailDlg::OnRemoveRequiredPurpose() 
{
	try {
		RemoveCurrentPurpose();
	}NxCatchAll("Error in CRequiredAllocationsDetailDlg::OnRemoveRequiredPurpose()");
}

void CRequiredAllocationsDetailDlg::OnRemoveRequiredType() 
{
	try {
		RemoveCurrentType();
	}NxCatchAll("Error in CRequiredAllocationsDetailDlg::OnRemoveRequiredType()");
}

void CRequiredAllocationsDetailDlg::OnDblClickCellAvailablePurposesList(LPDISPATCH lpRow, short nColIndex) 
{
	try {
		AddCurrentPurpose();
	}NxCatchAll("Error in CRequiredAllocationsDetailDlg::OnDblClickCellAvailablePurposesList()");
}

void CRequiredAllocationsDetailDlg::OnDblClickCellAvailableTypes(LPDISPATCH lpRow, short nColIndex) 
{
	try {
		AddCurrentType();
	}NxCatchAll("Error in CRequiredAllocationsDetailDlg::OnDblClickCellAvailableTypes()");
}

void CRequiredAllocationsDetailDlg::OnDblClickCellSelectedPurposesList(LPDISPATCH lpRow, short nColIndex) 
{
	try {
		RemoveCurrentPurpose();
	}NxCatchAll("Error in CRequiredAllocationsDetailDlg::OnDblClickCellSelectedPurposesList()");
}

void CRequiredAllocationsDetailDlg::OnDblClickCellSelectedTypes(LPDISPATCH lpRow, short nColIndex) 
{
	try {
		RemoveCurrentType();
	}NxCatchAll("Error in CRequiredAllocationsDetailDlg::OnDblClickCellSelectedTypes()");
}

void CRequiredAllocationsDetailDlg::AddCurrentPurpose()
{
	//TES 6/12/2008 - PLID 28078 - This function doesn't work on the datalist2, will be fixed by 24893
	// (b.cardillo 2008-07-15 13:08) - PLID 30737 - TES's comment is referring to the TakeCurrentRow() 
	// method, which was actually fixed by plid 30616, not 24893.  In any case, we're now getting rid 
	// of the manual move of the selected rows and using TakeCurrentRowAddAtEnd() for efficiency.
	//TES 8/4/2008 - PLID 28078 - Changed to TakeCurrentRowAddSorted()
	m_pSelectedPurposes->TakeCurrentRowAddSorted(m_pAvailPurposes, NULL);
}

void CRequiredAllocationsDetailDlg::AddCurrentType()
{
	//TES 6/12/2008 - PLID 28078 - This function doesn't work on the datalist2, will be fixed by 24893
	// (b.cardillo 2008-07-15 13:08) - PLID 30737 - TES's comment is referring to the TakeCurrentRow() 
	// method, which was actually fixed by plid 30616, not 24893.  In any case, we're now getting rid 
	// of the manual move of the selected rows and using TakeCurrentRowAddAtEnd() for efficiency.
	//TES 8/4/2008 - PLID 28078 - Changed to TakeCurrentRowAddSorted()
	m_pSelectedTypes->TakeCurrentRowAddSorted(m_pAvailTypes, NULL);
}

void CRequiredAllocationsDetailDlg::RemoveCurrentPurpose()
{
	//TES 6/12/2008 - PLID 28078 - This function doesn't work on the datalist2, will be fixed by 24893
	// (b.cardillo 2008-07-15 13:08) - PLID 30737 - TES's comment is referring to the TakeCurrentRow() 
	// method, which was actually fixed by plid 30616, not 24893.  In any case, we're now getting rid 
	// of the manual move of the selected rows and using TakeCurrentRowAddAtEnd() for efficiency.
	//TES 8/4/2008 - PLID 28078 - Changed to TakeCurrentRowAddSorted()
	m_pAvailPurposes->TakeCurrentRowAddSorted(m_pSelectedPurposes, NULL);
}

void CRequiredAllocationsDetailDlg::RemoveCurrentType()
{
	//TES 6/12/2008 - PLID 28078 - This function doesn't work on the datalist2, will be fixed by 24893
	// (b.cardillo 2008-07-15 13:08) - PLID 30737 - TES's comment is referring to the TakeCurrentRow() 
	// method, which was actually fixed by plid 30616, not 24893.  In any case, we're now getting rid 
	// of the manual move of the selected rows and using TakeCurrentRowAddAtEnd() for efficiency.
	//TES 8/4/2008 - PLID 28078 - Changed to TakeCurrentRowAddSorted()
	m_pAvailTypes->TakeCurrentRowAddSorted(m_pSelectedTypes, NULL);
}