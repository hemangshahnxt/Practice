// UB92Categories.cpp : implementation file
//

#include "stdafx.h"
#include "UB92Categories.h"

using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CUB92Categories dialog


CUB92Categories::CUB92Categories(CWnd* pParent /*=NULL*/)
	: CNxDialog(CUB92Categories::IDD, pParent),
	m_UB92CatChecker(NetUtils::UB92Cats)
{
	//{{AFX_DATA_INIT(CUB92Categories)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CUB92Categories::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUB92Categories)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_ADD_CAT, m_btnAdd);
	DDX_Control(pDX, IDC_DEL_CAT, m_btnDelete);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUB92Categories, CNxDialog)
	//{{AFX_MSG_MAP(CUB92Categories)
	ON_BN_CLICKED(IDC_ADD_CAT, OnAddCat)
	ON_BN_CLICKED(IDC_DEL_CAT, OnDelCat)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUB92Categories message handlers

BOOL CUB92Categories::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	m_btnOK.AutoSet(NXB_CLOSE);
	m_btnAdd.AutoSet(NXB_NEW);
	m_btnDelete.AutoSet(NXB_DELETE);
	
	m_CategoryList = BindNxDataListCtrl(this,IDC_UB92_CATEGORY_LIST,GetRemoteData(),TRUE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CUB92Categories::OnCommand(WPARAM wParam, LPARAM lParam) 
{	
	return CDialog::OnCommand(wParam, lParam);
}

void CUB92Categories::OnOK() 
{	
	CDialog::OnOK();
}

void CUB92Categories::OnAddCat() 
{
	int nResult;
	CString strNewCode, strDesc;

	nResult = InputBoxLimited(this, "Enter New Code", strNewCode, "",50,false,false,NULL);
	if (nResult != IDOK || strNewCode == "")
		return;

	if(!IsRecordsetEmpty("SELECT ID FROM UB92CategoriesT WHERE Code = '%s'",_Q(strNewCode))) {
		AfxMessageBox("This code already exists in the list. Please enter a different code.");
		return;
	}

	nResult = InputBoxLimited(this, "Enter New Description", strDesc, "",255,false,false,NULL);
	if (nResult != IDOK || strDesc == "")
		return;

	try {
		long ID = NewNumber("UB92CategoriesT","ID");
		CString str;
		str.Format("INSERT INTO UB92CategoriesT (ID, Code, Name) VALUES (%li, '%s', '%s')",ID,_Q(strNewCode),_Q(strDesc));
		ExecuteSql("%s",str);

		IRowSettingsPtr pRow = m_CategoryList->GetRow(-1);
		pRow->PutValue(0,long(ID));
		pRow->PutValue(1,_bstr_t(strNewCode));
		pRow->PutValue(2,_bstr_t(strDesc));
		m_CategoryList->AddRow(pRow);

		m_UB92CatChecker.Refresh();

	}NxCatchAll("Error adding new revenue code.");
}

void CUB92Categories::OnDelCat() 
{
	long row = m_CategoryList->CurSel;

	if(row == -1)
		return;

	try {
		CString strSql = BeginSqlBatch();

		// (j.jones 2009-08-17 15:03) - PLID 24281 - reworded this to call it a revenue code
		if(IDNO == MessageBox("Are you sure you wish to delete this revenue code?","Practice",MB_ICONQUESTION|MB_YESNO))
			return;

		long ID = m_CategoryList->GetValue(row,0).lVal;

		//DRT 7/3/03 - Make sure no items already have this category
		_RecordsetPtr prs = CreateRecordset("SELECT ID FROM ServiceT WHERE UB92Category = %li", ID);
		if(!prs->eof) {
			//TES 3/13/2007 - PLID 24993 - Changed from "UB92" to "UB"
			// (j.jones 2009-08-17 15:03) - PLID 24281 - reworded this to call it a revenue code
			if(MsgBox(MB_YESNO, "There are items that have this revenue code selected. "
				"Deleting the revenue code will set them to having no code selected.  Are you sure you wish to proceed?") == IDNO) {
				return;
			}

			//wipe it out
			AddStatementToSqlBatch(strSql, "UPDATE ServiceT SET UB92Category = NULL WHERE UB92Category = %li", ID);

			AddStatementToSqlBatch(strSql, "DELETE FROM ServiceRevCodesT WHERE UB92CategoryID = %li", ID);
		}

		// (a.walling 2007-06-05 16:03) - PLID 26228
		// (a.walling 2007-08-17 14:08) - PLID 27092 - Support multiple UBCategories
		prs = CreateRecordset("SELECT UB92SetupID FROM UB04MultiGroupRevExpandT WHERE UBCategoryID = %li", ID);
		if (!prs->eof) {
			// (j.jones 2009-08-17 15:03) - PLID 24281 - reworded this to call it a revenue code
			if(MsgBox(MB_YESNO, "There are UB Groups that have this revenue code selected to expand. "
				"Deleting the revenue code will remove that feature for items in this group.  Are you sure you wish to proceed?") == IDNO) {
				return;
			}

			//wipe it out
			AddStatementToSqlBatch(strSql, "DELETE FROM UB04MultiGroupRevExpandT WHERE UBCategoryID = %li", ID);
		}

		//delete the item
		AddStatementToSqlBatch(strSql, "DELETE FROM UB92CategoriesT WHERE ID = %li",ID);

		ExecuteSqlBatch(strSql);

		m_CategoryList->RemoveRow(row);

		m_UB92CatChecker.Refresh();
	}NxCatchAll("Error deleting revenue code.");
}

BEGIN_EVENTSINK_MAP(CUB92Categories, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CUB92Categories)
	ON_EVENT(CUB92Categories, IDC_UB92_CATEGORY_LIST, 6 /* RButtonDown */, OnRButtonDownUb92CategoryList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CUB92Categories, IDC_UB92_CATEGORY_LIST, 9 /* EditingFinishing */, OnEditingFinishingUb92CategoryList, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CUB92Categories, IDC_UB92_CATEGORY_LIST, 10 /* EditingFinished */, OnEditingFinishedUb92CategoryList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CUB92Categories::OnRButtonDownUb92CategoryList(long nRow, short nCol, long x, long y, long nFlags) 
{
	// TODO: Add your control notification handler code here
	
}

void CUB92Categories::OnEditingFinishingUb92CategoryList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	if(nRow==-1)
		return;

	if(pvarNewValue->vt != VT_BSTR) {
		*pbCommit = FALSE;
		*pbContinue = FALSE;
	}

	CString str = strUserEntered;

	str.TrimRight();

	if(str.GetLength() == 0) {
		// (a.walling 2012-05-17 17:07) - PLID 50481 - Fix BSTR leaks
		::VariantClear(pvarNewValue);
		::VariantCopy(pvarNewValue, &varOldValue);
		*pbCommit = FALSE;
	}

	long ID = m_CategoryList->GetValue(nRow,0).lVal;

	if(nCol == 1 && !IsRecordsetEmpty("SELECT ID FROM UB92CategoriesT WHERE Code = '%s' AND ID <> %li",_Q(CString(pvarNewValue->bstrVal)), ID)) {
		AfxMessageBox("This code already exists in the list. Please enter a different code.");
		*pbCommit = FALSE;
		*pbContinue = FALSE;
	}
}

void CUB92Categories::OnEditingFinishedUb92CategoryList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	if(nRow==-1)
		return;

	try {

		long ID = m_CategoryList->GetValue(nRow,0).lVal;

		if(nCol == 1) {		
			ExecuteSql("UPDATE UB92CategoriesT SET Code = '%s' WHERE ID = %li",_Q(CString(varNewValue.bstrVal)),ID);
		}
		else if(nCol == 2) {
			CString str;
			str.Format("UPDATE UB92CategoriesT SET Name = '%s' WHERE ID = %li",_Q(CString(varNewValue.bstrVal)),ID);
			ExecuteSql("%s",str);
		}

		m_UB92CatChecker.Refresh();

	}NxCatchAll("Error adding new revenue code.");
}
