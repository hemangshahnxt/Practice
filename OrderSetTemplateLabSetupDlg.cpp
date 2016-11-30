// OrderSetTemplateLabSetup.cpp : implementation file
//

#include "stdafx.h"
#include "AdministratorRc.h"
#include "OrderSetTemplateLabSetupDlg.h"

using namespace NXDATALIST2Lib;
using namespace ADODB;

// COrderSetTemplateLabSetupDlg dialog

IMPLEMENT_DYNAMIC(COrderSetTemplateLabSetupDlg, CNxDialog)

COrderSetTemplateLabSetupDlg::COrderSetTemplateLabSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(COrderSetTemplateLabSetupDlg::IDD, pParent)
{

}

void COrderSetTemplateLabSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ORDERSET_TMPL_LAB_SETUP_COLOR, m_nxcolor);
	DDX_Control(pDX, IDC_ORDERSET_TMPL_LAB_TO_BE_ORDERED_TEXT, m_nxeditToBeOrderedText);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(COrderSetTemplateLabSetupDlg, CNxDialog)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(COrderSetTemplateLabSetupDlg, CNxDialog)
ON_EVENT(COrderSetTemplateLabSetupDlg, IDC_ORDERSET_TMPL_LAB_TO_BE_ORDERED_LIST, 16, COrderSetTemplateLabSetupDlg::OnSelChosenOrdersetTmplLabToBeOrderedList, VTS_DISPATCH)
END_EVENTSINK_MAP()


// COrderSetTemplateLabSetupDlg message handlers

BOOL COrderSetTemplateLabSetupDlg::OnInitDialog()
{
	// (e.lally 2009-05-14) PLID 34241
	try{
		CNxDialog::OnInitDialog();

		m_pdlToBeOrderedMaster = BindNxDataList2Ctrl(IDC_ORDERSET_TMPL_LAB_TO_BE_ORDERED_LIST, true);

		//Set the icon buttons
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		//Set the text box
		m_nxeditToBeOrderedText.SetLimitText(1000);
		m_nxeditToBeOrderedText.SetWindowText(*m_pstrToBeOrderedText);
	}NxCatchAll(__FUNCTION__);
	return TRUE;
}

void COrderSetTemplateLabSetupDlg::OnOK()
{
	try {
		// (e.lally 2009-05-14) PLID 34241
		CString strNewText;
		m_nxeditToBeOrderedText.GetWindowText(strNewText);
		*m_pstrToBeOrderedText = strNewText;
		CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__);
}

void COrderSetTemplateLabSetupDlg::OnCancel()
{
	try {
		// (e.lally 2009-05-14) PLID 34241
		CNxDialog::OnCancel();
	}NxCatchAll(__FUNCTION__);
}

void COrderSetTemplateLabSetupDlg::OnSelChosenOrdersetTmplLabToBeOrderedList(LPDISPATCH lpRow)
{
	try {
		// (e.lally 2009-05-14) PLID 34241
		CString strCurrentText;
		m_nxeditToBeOrderedText.GetWindowText(strCurrentText);

		IRowSettingsPtr pRow(lpRow);
		if(pRow != NULL){
			CString strNewText = VarString(pRow->GetValue(1), "");
			//Check if this is the first thing in the text list
			if(strCurrentText.IsEmpty()){
				strCurrentText = strNewText;
			}
			else{
				//Append a comma and our new text
				strCurrentText += ", " + strNewText;
			}
			m_nxeditToBeOrderedText.SetWindowText(strCurrentText);
		}
	}NxCatchAll(__FUNCTION__);
}
