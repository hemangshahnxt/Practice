// MultiTemplateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "administratorrc.h"
#include "MultiTemplateDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CMultiTemplateDlg dialog


CMultiTemplateDlg::CMultiTemplateDlg(CWnd* pParent)
	: CNxDialog(CMultiTemplateDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMultiTemplateDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CMultiTemplateDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMultiTemplateDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_ADD_TEMPLATE, m_btnAdd);
	DDX_Control(pDX, IDC_REMOVE_TEMPLATE, m_btnRemove);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMultiTemplateDlg, CNxDialog)
	//{{AFX_MSG_MAP(CMultiTemplateDlg)
	ON_BN_CLICKED(IDC_ADD_TEMPLATE, OnAddTemplate)
	ON_BN_CLICKED(IDC_REMOVE_TEMPLATE, OnRemoveTemplate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMultiTemplateDlg message handlers

BOOL CMultiTemplateDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try
	{
		// (z.manning, 04/30/2008) - PLID 29864 - Set button styles
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnRemove.AutoSet(NXB_DELETE);
		
		m_pTemplates = BindNxDataListCtrl(IDC_TEMPLATES, false);

		// (j.armen 2012-01-26 11:24) - PLID 47809 - Parameratized
		if(!m_aryTemplateIDs.IsEmpty())
		{
			_RecordsetPtr rsTemplate = CreateParamRecordset("SELECT ID, Path FROM MergeTemplatesT WHERE ID IN ({INTARRAY})", m_aryTemplateIDs);
			while(!rsTemplate->eof)
			{
				IRowSettingsPtr pRow = m_pTemplates->GetRow(-1);
				pRow->PutValue(0, AdoFldLong(rsTemplate, "ID"));
				pRow->PutValue(1, _bstr_t(AdoFldString(rsTemplate, "Path")));
				m_pTemplates->AddRow(pRow);
				rsTemplate->MoveNext();
			}
		}
	}NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMultiTemplateDlg::OnAddTemplate() 
{
	try {
		// (a.walling 2007-06-14 16:02) - PLID 26342 - Support Word 2007 templates
		// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
		CString strFilter;
		// Always support Word 2007 templates
		strFilter = "Microsoft Word Templates|*.dot;*.dotx;*.dotm||";

		// (a.walling 2012-04-27 15:23) - PLID 46648 - Dialogs must set a parent!
		CFileDialog dlg(TRUE, "dot", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, strFilter, this);

		// set up open document dialog
		CString strTemplatePath = GetTemplatePath();
		dlg.m_ofn.lpstrInitialDir = strTemplatePath;
		dlg.m_ofn.lpstrTitle = "Select a merge template";

		if (dlg.DoModal() == IDOK) {	
			long nNewTemplateID;
			CString strPath = dlg.GetPathName();
			if(strPath.Left(GetSharedPath().GetLength()).CompareNoCase(GetSharedPath()) == 0) {
				strPath = strPath.Mid(GetSharedPath().GetLength());
				//Since this is presumed to be relative to the shared path, make sure it starts with exactly one '\'.
				if(strPath.Left(1) != "\\") {
					strPath = "\\" + strPath;
				}
			}
			
			//Either way, let's make it all upper case
			strPath.MakeUpper();
			//OK, strPath is now a valid record.
			// (j.armen 2012-01-26 11:19) - PLID 47809 - Parameratized
			_RecordsetPtr rsTemplateID = CreateParamRecordset("SELECT ID FROM MergeTemplatesT WHERE Path = {STRING}", strPath);
			if(rsTemplateID->eof) {
				//OK, we need to store it.
				nNewTemplateID = NewNumber("MergeTemplatesT", "ID");
				// (j.armen 2012-01-26 11:20) - PLID 47809 - Parameratized
				ExecuteParamSql("INSERT INTO MergeTemplatesT (ID, Path) VALUES ({INT}, {STRING})", nNewTemplateID, strPath);
			}
			else {
				//It already has been stored.
				nNewTemplateID = AdoFldLong(rsTemplateID, "ID");

				//Check whether this is already in our list.
				bool bFound = false;
				for(int i = 0; i < m_aryTemplateIDs.GetSize(); i++) {
					if(m_aryTemplateIDs[i] == nNewTemplateID) 
						bFound = true;
				}
				if(bFound) {
					MsgBox("The selected template is already part of this packet");
					return;
				}
			}
			
			//Now add this new template to our list.
			//Put in the datalist.
			IRowSettingsPtr pRow = m_pTemplates->GetRow(-1);
			pRow->PutValue(0, nNewTemplateID);
			pRow->PutValue(1, _bstr_t(strPath));
			m_pTemplates->AddRow(pRow);
			
			//Put it in our array.
			m_aryTemplateIDs.Add(nNewTemplateID);
		}
	}NxCatchAll("Error in CMultiTemplateDlg::OnAdd()");
}

void CMultiTemplateDlg::OnRemoveTemplate() 
{
	if(m_pTemplates->CurSel != -1) {
		long nTemplateID = VarLong(m_pTemplates->GetValue(m_pTemplates->CurSel, 0));
		for(int i = 0; i < m_aryTemplateIDs.GetSize(); i++) {
			if(m_aryTemplateIDs[i] == nTemplateID) {
				m_aryTemplateIDs.RemoveAt(i);
				i--;
			}
		}
		m_pTemplates->RemoveRow(m_pTemplates->CurSel);
	}
}

void CMultiTemplateDlg::OnOK() 
{
	CDialog::OnOK();
}

void CMultiTemplateDlg::OnCancel() 
{
	
	CDialog::OnCancel();
}
