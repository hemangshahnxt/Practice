// NewPrescriptionTemplateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NewPrescriptionTemplateDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (j.jones 2008-06-09 11:23) - PLID 29154 - added

/////////////////////////////////////////////////////////////////////////////
// CNewPrescriptionTemplateDlg dialog

using namespace NXDATALIST2Lib;

CNewPrescriptionTemplateDlg::CNewPrescriptionTemplateDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CNewPrescriptionTemplateDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewPrescriptionTemplateDlg)
		m_nCount = -1;
		m_strTemplateName = "";
	//}}AFX_DATA_INIT
}


void CNewPrescriptionTemplateDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewPrescriptionTemplateDlg)
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_EDIT_PRESCRIPTION_COUNT, m_editCount);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewPrescriptionTemplateDlg, CNxDialog)
	//{{AFX_MSG_MAP(CNewPrescriptionTemplateDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewPrescriptionTemplateDlg message handlers

BOOL CNewPrescriptionTemplateDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {

		//set the limit to two characters
		m_editCount.SetLimitText(2);

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_TemplateCombo = BindNxDataList2Ctrl(IDC_FORMS_TEMPLATE_LIST, false);

		//these should never be different sizes
		ASSERT(m_aryExistingCounts.GetSize() == m_aryExistingTemplates.GetSize());

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
					IRowSettingsPtr pRow = m_TemplateCombo->GetNewRow();
					pRow->PutValue(0, _bstr_t(strFileName));
					m_TemplateCombo->AddRowSorted(pRow, NULL);
				}
			}
			//do once more
			CString strFileName = finder.GetFileName();
			// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
			if((strFileName.GetLength() >= 5 && strFileName.Right(5).CompareNoCase(".dotx") == 0)
				|| (strFileName.GetLength() >= 5 && strFileName.Right(5).CompareNoCase(".dotm") == 0)
				|| (strFileName.GetLength() >= 4 && strFileName.Right(4).CompareNoCase(".dot") == 0)) {
				
				//add to our list
				IRowSettingsPtr pRow = m_TemplateCombo->GetNewRow();
				pRow->PutValue(0, _bstr_t(strFileName));
				m_TemplateCombo->AddRowSorted(pRow, NULL);
			}

		} else {
			// Nothing was placed into the list, which means we can't do anything here
			MsgBox(MB_OK|MB_ICONINFORMATION, 
				"There are no templates in the shared templates Forms sub-folder:\n\n%s\n\n"
				"Without any prescription templates, you cannot use this setup.", strFind);
		}

		//default the count to be the next highest count from our list
		long nNewCount = 1;
		if(m_aryExistingCounts.GetSize() > 0) {
			nNewCount = (long)(m_aryExistingCounts.GetAt(m_aryExistingCounts.GetSize() - 1)) + 1;
		}

		//If the next number would be 31, don't set a default, because
		//31 and up is not allowed. In this case, leave it blank.
		if(nNewCount <= 30) {
			SetDlgItemInt(IDC_EDIT_PRESCRIPTION_COUNT, nNewCount);
		}

	}NxCatchAll("Error in CNewPrescriptionTemplateDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNewPrescriptionTemplateDlg::OnOK() 
{
	try {

		CString strTemplateName = "";
		long nCount = -1;

		IRowSettingsPtr pRow = m_TemplateCombo->GetCurSel();
		if(pRow == NULL) {
			AfxMessageBox("You must select a template to use before continuing.");
			return;
		}

		strTemplateName = VarString(pRow->GetValue(0), "");
		if(strTemplateName.IsEmpty()) {
			AfxMessageBox("You must select a valid template to use before continuing.");
			return;
		}

		nCount = GetDlgItemInt(IDC_EDIT_PRESCRIPTION_COUNT);

		//do not allow zero
		if(nCount <= 0) {
			AfxMessageBox("You must enter a prescription count greater than zero.");
			return;
		}

		//do not allow more than 30
		if(nCount > 30) {
			AfxMessageBox("You cannot enter a prescription count greater than 30.");
			return;
		}		

		//do not allow duplicate counts
		int i = 0;
		for(i=0;i<m_aryExistingCounts.GetSize();i++) {

			long nCurCount = (long)(m_aryExistingCounts.GetAt(i));

			if(nCurCount == nCount) {
				AfxMessageBox("The prescription count you entered already has a template configured for it.");
				return;
			}
		}

		//see if the template selected is used by another count, ask them if they want to re-use it
		BOOL bFound = FALSE;
		for(i=0;i<m_aryExistingTemplates.GetSize() && !bFound;i++) {

			long nCurCount = (long)(m_aryExistingCounts.GetAt(i));
			CString strCurTemplate = (CString)(m_aryExistingTemplates.GetAt(i));

			if(strCurTemplate.CompareNoCase(strTemplateName) == 0) {

				//found it
				bFound = TRUE;

				CString str;
				str.Format("The selected template is currently used for a prescription count of %li.\n"
					"Would you like to use it again for the count of %li?", nCurCount, nCount);
				if(IDNO == MessageBox(str, "Practice", MB_ICONQUESTION|MB_YESNO)) {
					return;
				}					
			}
		}

		//if we get here, we're good to go
		m_nCount = nCount;
		m_strTemplateName = strTemplateName;
	
		CNxDialog::OnOK();

	}NxCatchAll("Error in CNewPrescriptionTemplateDlg::OnOK");
}

void CNewPrescriptionTemplateDlg::OnCancel() 
{
	try {
	
		CNxDialog::OnCancel();

	}NxCatchAll("Error in CNewPrescriptionTemplateDlg::OnCancel");
}
