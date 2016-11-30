// ConfigureSuperbillAddFileDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ConfigureSuperbillAddFileDlg.h"
#include "SchedulerRc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//DRT 6/6/2008 - PLID 30306 - Created.


//Datalist enumerations
enum eListColumns {
	elcFilename = 0,
	elcFullPath,
};

/////////////////////////////////////////////////////////////////////////////
// CConfigureSuperbillAddFileDlg dialog


CConfigureSuperbillAddFileDlg::CConfigureSuperbillAddFileDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CConfigureSuperbillAddFileDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConfigureSuperbillAddFileDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	// (d.thompson 2009-10-20) - PLID 36007
	m_bOverrideTemplates = false;
	m_paryOverrideList = NULL;
	m_strPromptText = "";
}


void CConfigureSuperbillAddFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigureSuperbillAddFileDlg)
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_ADD_SUPERBILL_DESCRIPTION_TEXT, m_nxstaticDescText);
	DDX_Control(pDX, IDC_SUPERBILL_ADD_PROMPT_TEXT, m_nxstaticPromptText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConfigureSuperbillAddFileDlg, CNxDialog)
	//{{AFX_MSG_MAP(CConfigureSuperbillAddFileDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigureSuperbillAddFileDlg message handlers

//I mostly copied this from CSuperbillDlg, but I ended up re-writing a lot of this version, it was pretty ugly.
void CConfigureSuperbillAddFileDlg::FillSuperBillList(CString strPath) {

	NXDATALIST2Lib::IRowSettingsPtr pRow;

	//build file list (automatically selecting the default template)
	CFileFind finder;
	CString strFind = strPath;
	BOOL bWorking = finder.FindFile(strFind ^ "*.*");
	
	while (bWorking) {
		bWorking = finder.FindNextFile();

		if(finder.IsDots()) {
			//We don't want dots
		}
		else if(finder.IsDirectory()) {
			//Recurse to this list
			FillSuperBillList(finder.GetFilePath());
		}
		else {
			//A normal file, add it to our list
			CString strFileName = finder.GetFileName();
			// (a.walling 2008-04-28 13:17) - PLID 28108 - Filenames are case insensitive
			CString strFileNameLower = strFileName;
			strFileNameLower.MakeLower();
			// (a.walling 2007-06-14 16:17) - PLID 26342
			// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
			if(strFileNameLower.Right(4) == ".dot" || (strFileNameLower.Right(5) == ".dotx") || (strFileNameLower.Right(5) == ".dotm")) {
				pRow = m_pList->GetNewRow();
				pRow->PutValue(elcFilename, _bstr_t(strFileName));
				pRow->PutValue(elcFullPath, _bstr_t(finder.GetFilePath()));
				m_pList->AddRowSorted(pRow, NULL);
			}
		}
	}
}

// (d.thompson 2009-10-20) - PLID 36007 - Fill based on the contents of our override array
void CConfigureSuperbillAddFileDlg::FillSuperBillOverrideList()
{
	for(int i = 0; i < m_paryOverrideList->GetSize(); i++) {
		CString strLocalPath = m_paryOverrideList->GetAt(i);
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->GetNewRow();
		pRow->PutValue(elcFilename, _bstr_t(strLocalPath));
		pRow->PutValue(elcFullPath, _bstr_t(GetSharedPath() ^ "Templates\\Forms\\" + strLocalPath));
		m_pList->AddRowSorted(pRow, NULL);
	}
}

BOOL CConfigureSuperbillAddFileDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		//Bind List
		m_pList = BindNxDataList2Ctrl(IDC_SUPERBILL_ADD_LIST, false);

		//Setup buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// (d.thompson 2009-10-20) - PLID 36007 - Provide some prompt text that will display above the 
		//	datalist where a caller can display a notice.  This is blank by default if not used.
		SetDlgItemText(IDC_SUPERBILL_ADD_PROMPT_TEXT, m_strPromptText);

		//Populate the list of superbills
		// (d.thompson 2009-10-20) - PLID 36007 - Provided the ability to override the list.  If that's
		//	enabled, we will fill the list with only the predefined templates.
		if(!m_bOverrideTemplates) {
			FillSuperBillList(GetSharedPath() ^ "Templates\\Forms");
		}
		else {
			FillSuperBillOverrideList();
		}

	} NxCatchAll("Error in OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CConfigureSuperbillAddFileDlg::OnOK() 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRowSel = m_pList->GetCurSel();
		if(pRowSel == NULL) {
			AfxMessageBox("You must select a superbill before continuing.");
			return;
		}

		//Save the full path into the results
		m_strResults_FullPath = VarString(pRowSel->GetValue(elcFullPath));

		//Now strip off the shared path / templates / forms portion so we get the
		//	starting point for the superbill (superbills must always be in templates\forms).
		m_strResults_SuperbillPath = m_strResults_FullPath;
		m_strResults_SuperbillPath = m_strResults_SuperbillPath.Mid( CString(GetSharedPath() ^ "Templates\\Forms\\").GetLength());

		CDialog::OnOK();
	} NxCatchAll("Error in OnOK");
}

//No cleanup to do, just dismiss the dialog
void CConfigureSuperbillAddFileDlg::OnCancel() 
{
	CDialog::OnCancel();
}

void CConfigureSuperbillAddFileDlg::OverrideTemplateList(CStringArray *paryLocalPaths)
{
	m_bOverrideTemplates = true;
	m_paryOverrideList = paryLocalPaths;
}
