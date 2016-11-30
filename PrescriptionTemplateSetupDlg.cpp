// PrescriptionTemplateSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PrescriptionTemplateSetupDlg.h"
#include "NewPrescriptionTemplateDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (j.jones 2008-06-06 11:37) - PLID 29154 - created

/////////////////////////////////////////////////////////////////////////////
// CPrescriptionTemplateSetupDlg dialog

using namespace NXDATALIST2Lib;
using namespace ADODB;

enum TemplateListColumns {

	tlcCount = 0,
	tlcTemplateName,
	tlcIsNew,
};

CPrescriptionTemplateSetupDlg::CPrescriptionTemplateSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CPrescriptionTemplateSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPrescriptionTemplateSetupDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPrescriptionTemplateSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrescriptionTemplateSetupDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_DELETE_TEMPLATE, m_btnDelete);
	DDX_Control(pDX, IDC_BTN_ADD_TEMPLATE, m_btnAdd);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrescriptionTemplateSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CPrescriptionTemplateSetupDlg)
	ON_BN_CLICKED(IDC_BTN_ADD_TEMPLATE, OnBtnAddTemplate)
	ON_BN_CLICKED(IDC_DELETE_TEMPLATE, OnDeleteTemplate)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrescriptionTemplateSetupDlg message handlers

BOOL CPrescriptionTemplateSetupDlg::OnInitDialog() 
{
	
	try {

		CNxDialog::OnInitDialog();

		m_btnAdd.AutoSet(NXB_NEW);
		m_btnDelete.AutoSet(NXB_DELETE);
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_DefTemplateCombo = BindNxDataList2Ctrl(IDC_DEFAULT_TEMPLATE_COMBO, false);
		m_TemplateList = BindNxDataList2Ctrl(IDC_DEFAULT_PRESCRIPTION_TEMPLATE_LIST, true);

		// Load the name of the default template
		CString strDefTemplate = GetPropertyText("DefaultPrescriptionFilename", "", 0, false);

		//build the template combo (this code was copied and modified from the medications dialog)
		CFileFind finder;
		CString strFind = GetTemplatePath("Forms");

		// interestingly, searching for *.dot will also match *.dot*
		if (finder.FindFile(strFind ^ "*.dot"))
		{
			while (finder.FindNextFile())
			{
				CString strFileName = finder.GetFileName();
				// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
				if((strFileName.GetLength() >= 5 && strFileName.Right(5).CompareNoCase(".dotx") == 0)
					|| (strFileName.GetLength() >= 5 && strFileName.Right(5).CompareNoCase(".dotm") == 0)
					|| (strFileName.GetLength() >= 4 && strFileName.Right(4).CompareNoCase(".dot") == 0)) {

					//add to our list
					IRowSettingsPtr pRow = m_DefTemplateCombo->GetNewRow();
					pRow->PutValue(0, _bstr_t(strFileName));
					m_DefTemplateCombo->AddRowSorted(pRow, NULL);

					//see if it is our default template
					if(strFileName.CompareNoCase(strDefTemplate) == 0) {

						m_DefTemplateCombo->PutCurSel(pRow);
						pRow->ForeColor = RGB(255,0,0);
					}
				}
			}
			//do once more
			CString strFileName = finder.GetFileName();
			// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
			if((strFileName.GetLength() >= 5 && strFileName.Right(5).CompareNoCase(".dotx") == 0)
				|| (strFileName.GetLength() >= 5 && strFileName.Right(5).CompareNoCase(".dotm") == 0)
				|| (strFileName.GetLength() >= 4 && strFileName.Right(4).CompareNoCase(".dot") == 0)) {
				
				//add to our list
				IRowSettingsPtr pRow = m_DefTemplateCombo->GetNewRow();
				pRow->PutValue(0, _bstr_t(strFileName));
				m_DefTemplateCombo->AddRowSorted(pRow, NULL);

				//see if it is our default template
				if(strFileName.CompareNoCase(strDefTemplate) == 0) {

					m_DefTemplateCombo->PutCurSel(pRow);
					pRow->ForeColor = RGB(255,0,0);
				}
			}

		} else {
			// Nothing was placed into the list, which means we can't do anything here
			MsgBox(MB_OK|MB_ICONINFORMATION, 
				"There are no templates in the shared templates Forms sub-folder:\n\n%s\n\n"
				"Without any prescription templates, you cannot use this setup.", strFind);
		}

	}NxCatchAll("Error in CPrescriptionTemplateSetupDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPrescriptionTemplateSetupDlg::OnBtnAddTemplate() 
{
	try {

		CNewPrescriptionTemplateDlg dlg(this);

		//we have an (artificial) cutoff of 30 templates, so don't let them add more
		if(m_TemplateList->GetRowCount() >= 30) {
			AfxMessageBox("You may not have more than 30 templates configured.");
			return;
		}

		//pass in the existing counts and template names
		IRowSettingsPtr pRow = m_TemplateList->GetFirstRow();
		while(pRow) {

			long nCount = VarLong(pRow->GetValue(tlcCount));
			CString strTemplate = VarString(pRow->GetValue(tlcTemplateName), "");

			dlg.m_aryExistingCounts.Add(nCount);
			dlg.m_aryExistingTemplates.Add(strTemplate);

			pRow = pRow->GetNextRow();
		}

		if(dlg.DoModal() == IDOK) {

			if(dlg.m_nCount == -1 || dlg.m_strTemplateName.IsEmpty()) {
				//should be impossible
				ASSERT(FALSE);
				return;
			}

			//the dialog would be responsible for detecting duplicates,
			//so all we need to do is simply add a new row

			_variant_t varTrue(VARIANT_TRUE, VT_BOOL);

			IRowSettingsPtr pNewRow = m_TemplateList->GetNewRow();
			pNewRow->PutValue(tlcCount, (long)dlg.m_nCount);
			pNewRow->PutValue(tlcTemplateName, _bstr_t(dlg.m_strTemplateName));
			pNewRow->PutValue(tlcIsNew, varTrue);
			m_TemplateList->AddRowSorted(pNewRow, NULL);
		}

	}NxCatchAll("Error in CPrescriptionTemplateSetupDlg::OnBtnAddTemplate");	
}

void CPrescriptionTemplateSetupDlg::OnDeleteTemplate() 
{
	try {

		IRowSettingsPtr pRow = m_TemplateList->GetCurSel();
		if(pRow == NULL) {
			AfxMessageBox("No template is selected.");
			return;
		}

		if(IDNO == MessageBox("If you remove this default template, the template file will not be deleted, "
			"instead the standard prescription template will be used for this prescription count.\n\n"
			"Are you sure you wish to remove this default template?", "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {			
			return;
		}

		long nCount = VarLong(pRow->GetValue(tlcCount));

		//don't bother adding to our list if it is new
		if(!VarBool(pRow->GetValue(tlcIsNew), FALSE)) {
			//add to our list
			m_aryDeletedTemplateCounts.Add(nCount);
		}

		m_TemplateList->RemoveRow(pRow);

	}NxCatchAll("Error in CPrescriptionTemplateSetupDlg::OnDeleteTemplate");		
}

void CPrescriptionTemplateSetupDlg::OnOK() 
{
	try {

		//first remove any counts we cleared

		for(int i=0; i<m_aryDeletedTemplateCounts.GetSize();i++) {

			long nCount = (long)(m_aryDeletedTemplateCounts.GetAt(i));

			//clear the ConfigRT setting
			SetRemotePropertyText("DefaultPrescriptionFilenameByCount", "", nCount, "<None>");
		}
		m_aryDeletedTemplateCounts.RemoveAll();

		
		//now save all new ones

		IRowSettingsPtr pRow = m_TemplateList->GetFirstRow();
		while(pRow) {

			//is it new?
			if(VarBool(pRow->GetValue(tlcIsNew), FALSE)) {

				long nCount = VarLong(pRow->GetValue(tlcCount));
				CString strTemplate = VarString(pRow->GetValue(tlcTemplateName), "");
				
				//set the ConfigRT setting
				SetRemotePropertyText("DefaultPrescriptionFilenameByCount", strTemplate, nCount, "<None>");
			}

			pRow = pRow->GetNextRow();
		}

		//set the default template
		CString strDefaultTemplate = "";
		IRowSettingsPtr pDefRow = m_DefTemplateCombo->GetCurSel();		
		if(pDefRow) {
			strDefaultTemplate = VarString(pDefRow->GetValue(0),"");
		}
		SetPropertyText("DefaultPrescriptionFilename", strDefaultTemplate, 0);
			
		CNxDialog::OnOK();

	}NxCatchAll("Error in CPrescriptionTemplateSetupDlg::OnOK");
}

void CPrescriptionTemplateSetupDlg::OnCancel() 
{
	try {
			
		CNxDialog::OnCancel();

	}NxCatchAll("Error in CPrescriptionTemplateSetupDlg::OnCancel");
}
