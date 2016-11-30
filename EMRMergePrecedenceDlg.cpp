// EMRMergePrecedenceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EMRMergePrecedenceDlg.h"
#include "globalutils.h"

using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEMRMergePrecedenceDlg dialog


CEMRMergePrecedenceDlg::CEMRMergePrecedenceDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMRMergePrecedenceDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEMRMergePrecedenceDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	b_OutOfEMR = false;
}


void CEMRMergePrecedenceDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMRMergePrecedenceDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_STATIC_WORDTEMP, m_nxstaticWordtemp);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK2MERGE, m_btnOK2Merge);
	DDX_Control(pDX, IDCANCEL2MERGE, m_btnCancel2Merge);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEMRMergePrecedenceDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMRMergePrecedenceDlg)
	ON_COMMAND(ID_MERGEPREC_SELECTTEMPLATE, OnMenuSelectTemplate)
	ON_COMMAND(ID_MERGEPREC_UNSELECTTEMPLATE, OnMenuUnselectTemplate)
	ON_BN_CLICKED(IDCANCEL2MERGE, OnCancel2merge)
	ON_BN_CLICKED(IDOK2MERGE, OnOk2merge)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMRMergePrecedenceDlg message handlers

BOOL CEMRMergePrecedenceDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-28 12:03) - PLID 29806 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnOK2Merge.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnCancel2Merge.AutoSet(NXB_CANCEL);
		
		m_dlTemplates = BindNxDataListCtrl(this,IDC_LIST_TEMPLATES,GetRemoteData(),false);
		m_dlCollectionTemplates = BindNxDataListCtrl(this,IDC_LIST_COLLECTIONTEMPLATES,GetRemoteData(),false);

		if (b_OutOfEMR) {
			// a.walling this dialog is shown outside of EMR
			CRect lpDlgRect, lpListRect, lpItemRect;
			GetDlgItem(IDC_LIST_COLLECTIONTEMPLATES)->GetWindowRect(lpListRect);
			this->GetWindowRect(lpDlgRect);
			this->SetWindowText("Advanced Merge - Default Categories");

			// so disable the collections pane
			m_dlCollectionTemplates->PutEnabled(false);

			// and resize to hide the collections pane (resize to top of pane)
			this->SetWindowPos(0, 0, 0, lpDlgRect.Width(), (lpListRect.top - lpDlgRect.top), SWP_NOMOVE | SWP_NOZORDER);

			GetDlgItem(IDC_STATIC_WORDTEMP)->ShowWindow(SW_HIDE);

			// now show the right buttons
			GetDlgItem(IDOK2MERGE)->ShowWindow(SW_SHOW);
			GetDlgItem(IDOK2MERGE)->EnableWindow(TRUE);
			GetDlgItem(IDCANCEL2MERGE)->ShowWindow(SW_SHOW);
			GetDlgItem(IDCANCEL2MERGE)->EnableWindow(TRUE);
		}
		Load();

		// (m.hancock 2006-05-22 11:16) - PLID 20747 - Make sure our lists of modified rows are empty
		m_adwModTemplates.RemoveAll();
		m_adwModCollectionTemplates.RemoveAll();
	}
	NxCatchAll("Error in CEMRMergePrecedenceDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEMRMergePrecedenceDlg::OnOK()
{	
	Save();
	CDialog::OnOK();
}

void CEMRMergePrecedenceDlg::Load()
{
	if (!b_OutOfEMR) {
		m_dlCollectionTemplates->Requery();
	}

	m_dlTemplates->Requery();

	/*
	AddFolderToList("Forms", m_dlTemplates);
	AddFolderToList("Letters", m_dlTemplates);
	*/

	AddFolderToList("", m_dlTemplates);
	// (a.walling 2006-07-20 09:58) - PLID 21525 Use all files in the templates directory instead of two folders.

	// (c.haag 2004-10-05 14:01) - We don't want to open one recordset for
	// every template; just open a master recordset and fill it with categories.
	_RecordsetPtr prs = CreateRecordset("SELECT TemplateName, NoteCatID FROM EMRWordTemplateCategoryT LEFT JOIN NoteCatsF ON EMRWordTemplateCategoryT.NoteCatID = NoteCatsF.ID");
	FieldsPtr f = prs->Fields;
	while (!prs->eof)
	{
		CString strTemplateName = AdoFldString(f, "TemplateName");
		long nRow = m_dlTemplates->FindByColumn(0, f->Item["TemplateName"]->Value, 0, FALSE);
		if (nRow > -1) {
			m_dlTemplates->Value[nRow][2] = AdoFldLong(f, "NoteCatID");
		}
		prs->MoveNext();
	}
}

void CEMRMergePrecedenceDlg::Save()
{
	// (m.hancock 2006-05-22 12:36) - PLID 20747 - Saving EMR advanced merge settings takes too long.
	// Previously, this code used to remove all records in EMRWordTemplateCategoryT and EMRCollectionTemplateT, loop
	// through all rows in both datalists, and perform an insert for each row.  This was very slow.
	// To alleviate the slowness, I'm converting this so that an array is maintained for each datalist to keep track
	// of each row that is modified.  That way, the saving only needs to update the rows that have actually been changed.
	// Unfortunately, we still need to determine if there is an existing record in the database.  If there is, we simply
	// update that record; otherwise, we need to insert a new record.  This will cause some slowdown as ReturnRecords
	// is called, but for now, this should be a large enough speed boost.
	try {
		CString strSql;

		//Template Categories; Loop through m_adwModTemplates
		for(int x=0; x < m_adwModTemplates.GetSize(); x++) {
			//Get the row
			long nRow = m_adwModTemplates.GetAt(x);

			//Get the data from the datalist
			if (m_dlTemplates->Value[nRow][2].vt != VT_EMPTY) {
				CString strStatement;
				long nCategoryID = VarLong(m_dlTemplates->Value[nRow][2], -1);				
				if (nCategoryID != -1) {
					//Determine if we should update, delete, or insert
					// (m.hancock 2006-07-19 10:21) - PLID 20747 - Removed check for CategoryID because we should not be
					// looking for THIS category ID, but the old one, so we now check just by template name.
					_RecordsetPtr prs = CreateRecordset("SELECT ID FROM EMRWordTemplateCategoryT WHERE TemplateName = '%s'", _Q(VarString(m_dlTemplates->Value[nRow][0])));
					FieldsPtr f = prs->Fields;
					if(!prs->eof) {
							//A record exists, so update the category id
							strStatement.Format("UPDATE EMRWordTemplateCategoryT SET NoteCatID = %d WHERE ID = %li;\r\n",
								nCategoryID, VarLong(f->Item["ID"]->Value));
					}
					else {
						//A record does not exist, so insert
						strStatement.Format("INSERT INTO EMRWordTemplateCategoryT (TemplateName, NoteCatID) VALUES ('%s', %d);\r\n",
							_Q(VarString(m_dlTemplates->Value[nRow][0])), nCategoryID);
					}
				}
				else {
					//The user has removed this data, so delete the record
					strStatement.Format("DELETE FROM EMRWordTemplateCategoryT WHERE TemplateName = '%s';\r\n",
						_Q(VarString(m_dlTemplates->Value[nRow][0])));
				}

				//Add to the existing Sql batch
				strSql += strStatement;
			}
		}

		//Template Collections; Loop through m_adwModCollectionTemplates
		for(int y=0; y < m_adwModCollectionTemplates.GetSize(); y++) {
			//Get the row
			long nRow = m_adwModCollectionTemplates.GetAt(y);

			//Get the data from the datalist
			CString strStatement;
			long nID = VarLong(m_dlCollectionTemplates->Value[nRow][0]);
			CString strTemplateName = VarString(m_dlCollectionTemplates->Value[nRow][2], "");
			CString strTemplateFolder = VarString(m_dlCollectionTemplates->Value[nRow][3], "");

			//Determine if we should update, delete, or insert
			if (strTemplateName.GetLength() || strTemplateFolder.GetLength()) {			
				// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
				if (ReturnsRecordsParam("SELECT * FROM EMRCollectionTemplateT WHERE CollectionID = {INT}", nID)) {
					//A record exists, so update the category id
					strStatement.Format("UPDATE EMRCollectionTemplateT SET DefaultWordTemplateName = '%s', DefaultWordTemplatePath = '%s' WHERE CollectionID = %d",
						_Q(strTemplateName), _Q(strTemplateFolder), nID);
				}
				else {
					//A record does not exist, so insert
					strStatement.Format("INSERT INTO EMRCollectionTemplateT (CollectionID, DefaultWordTemplateName, DefaultWordTemplatePath) VALUES (%d, '%s', '%s')",
						nID, _Q(strTemplateName), _Q(strTemplateFolder));
				}
			}
			else {
				//The user has removed this data, so delete the record
				strStatement.Format("DELETE FROM EMRCollectionTemplateT WHERE CollectionID = %d",
						nID);
			}

			//Add to the existing Sql batch
			strSql += strStatement;
		}

		//Save the data in one batch sql statement
		if(!strSql.IsEmpty())
			ExecuteSql("%s", strSql);	
	
		// (a.walling 2010-09-08 15:02) - PLID 40377 - Another Rollback without an active transaction
	}NxCatchAllCall("Error saving Word Template categories", { return; });
}

BEGIN_EVENTSINK_MAP(CEMRMergePrecedenceDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEMRMergePrecedenceDlg)
	ON_EVENT(CEMRMergePrecedenceDlg, IDC_LIST_TEMPLATES, 10 /* EditingFinished */, OnEditingFinishedListTemplates, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CEMRMergePrecedenceDlg, IDC_LIST_COLLECTIONTEMPLATES, 4 /* LButtonDown */, OnLButtonDownListCollectiontemplates, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEMRMergePrecedenceDlg, IDC_LIST_COLLECTIONTEMPLATES, 6 /* RButtonDown */, OnRButtonDownListCollectiontemplates, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEMRMergePrecedenceDlg::OnEditingFinishedListTemplates(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	if (nRow > -1 && varNewValue.vt == VT_I4 && varNewValue.lVal == -1)
	{
		COleVariant vNull;
		vNull.vt = VT_NULL;
		m_dlTemplates->Value[nRow][2] = vNull;
	}

	// (m.hancock 2006-05-22 11:21) - PLID 20747 - Keep track of the row so we can save the changes later.
	AddToArray(m_adwModTemplates, nRow);
}

void CEMRMergePrecedenceDlg::OnLButtonDownListCollectiontemplates(long nRow, short nCol, long x, long y, long nFlags) 
{
	if (nRow == -1) return;

	CString strCurTemplate = VarString(m_dlCollectionTemplates->Value[nRow][2], "");
	CString strCurPath = VarString(m_dlCollectionTemplates->Value[nRow][3], "");

	// (a.walling 2007-06-14 16:02) - PLID 26342 - Support Word 2007 templates
	// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
	CString strFilter;
	// Always support Word 2007 templates
	strFilter = "Microsoft Word Templates|*.dot;*.dotx;*.dotm||";

	// (a.walling 2012-04-27 15:23) - PLID 46648 - Dialogs must set a parent!
	CFileDialog dlg(TRUE, "dot", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, strFilter, this);
	CString initialDir = GetTemplatePath() ^ strCurPath;
	dlg.m_ofn.lpstrInitialDir = initialDir;
	if (dlg.DoModal() == IDOK) {
		CString strFileName = dlg.GetFileName();
		CString strRelativePath = dlg.GetPathName();
		CString strTemplatePath = GetTemplatePath();
		CString strFolder = dlg.GetPathName();

		// (c.haag 2004-10-15 15:07) - Do some processing here to break up
		// the browsing results into a relative path and filename.
		strFileName.MakeLower();
		strTemplatePath.MakeLower();
		strRelativePath.MakeLower();

		long nIndex = -1;
		while (-1 != strRelativePath.Find(strFileName, nIndex + 1)) {
			nIndex = strRelativePath.Find(strFileName, nIndex + 1);
		}
		strRelativePath = strRelativePath.Left(nIndex);
		strFolder = strFolder.Left(nIndex);

		nIndex = strRelativePath.Find(strTemplatePath);
		if (nIndex != -1) {
			strFolder = strFolder.Right(strFolder.GetLength() - strTemplatePath.GetLength());
		}
		if (strFolder.Right(1) == '\\') {
			strFolder = strFolder.Left( strFolder.GetLength() - 1 );
		}

		m_dlCollectionTemplates->Value[nRow][2] = _bstr_t(dlg.GetFileName());
		m_dlCollectionTemplates->Value[nRow][3] = _bstr_t(strFolder);

		// (m.hancock 2006-05-22 11:21) - PLID 20747 - Keep track of the row so we can save the changes later.
		AddToArray(m_adwModCollectionTemplates, nRow);
	}
}

void CEMRMergePrecedenceDlg::OnRButtonDownListCollectiontemplates(long nRow, short nCol, long x, long y, long nFlags) 
{
	if (nRow == -1) return;
	m_dlCollectionTemplates->CurSel = nRow;
	CMenu* pMenu;
	pMenu = new CMenu;
	pMenu->CreatePopupMenu();
	pMenu->InsertMenu(-1, MF_BYPOSITION, ID_MERGEPREC_SELECTTEMPLATE, "&Select a Word template...");
	if (VarString(m_dlCollectionTemplates->Value[nRow][2],"").GetLength() ||
		VarString(m_dlCollectionTemplates->Value[nRow][3],"").GetLength())
	{
		pMenu->InsertMenu(-1, MF_BYPOSITION, ID_MERGEPREC_UNSELECTTEMPLATE, "&Unassign the selected Word template");
	}	
	CPoint pt;
	GetCursorPos(&pt);
	pMenu->TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this, NULL);
	delete pMenu;
}

void CEMRMergePrecedenceDlg::OnMenuSelectTemplate()
{
	if (m_dlCollectionTemplates->CurSel == -1) return;
	OnLButtonDownListCollectiontemplates(m_dlCollectionTemplates->CurSel, 0, 0, 0, 0);
}

void CEMRMergePrecedenceDlg::OnMenuUnselectTemplate()
{
	long nRow = m_dlCollectionTemplates->CurSel;
	if (nRow == -1) return;
	m_dlCollectionTemplates->Value[nRow][2] = "";
	m_dlCollectionTemplates->Value[nRow][3] = "";

	// (m.hancock 2006-05-22 11:21) - PLID 20747 - Keep track of the row so we can save the changes later.
	AddToArray(m_adwModCollectionTemplates, nRow);
}

void CEMRMergePrecedenceDlg::OnCancel2merge() 
{
	CDialog::OnCancel();	
}

void CEMRMergePrecedenceDlg::OnOk2merge() 
{
	Save();
	CDialog::OnOK();	
}

bool CEMRMergePrecedenceDlg::AddToArray(CDWordArray &adwArray, long nRow)
{
	// (m.hancock 2006-05-22 14:45) - PLID 20747 - We need to prevent rows
	// from being added to the array more than once.  Return true if we added
	// to the array successfully; return false if the row already exists in the array.
	bool bFound = false;

	//Loop through each value in the array
	for(int x=0; x < adwArray.GetSize(); x++) {
		//Check if this row already exists
		if((long)adwArray.GetAt(x) == nRow)
			bFound = true;
	}

	if(bFound) {
		//We found the row in the array
		return false;
	}
	else {
		//The row did not exist, so add it
		adwArray.Add(nRow);
		return true;
	}
}
