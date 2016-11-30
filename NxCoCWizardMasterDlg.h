//DRT 10/28/2008 - PLID 31789 - Created for NxCoC Importer
#pragma once


// CNxCoCWizardMasterDlg dialog

#include "NxCoCImportInfo.h"
class CNxCoCWizardSheet;

class CNxCoCWizardMasterDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CNxCoCWizardMasterDlg)

public:
	CNxCoCWizardMasterDlg(CWnd* pParent);   // standard constructor
	virtual ~CNxCoCWizardMasterDlg();

// Dialog Data
	enum { IDD = IDD_NXCOC_WIZARD_MASTER_DLG };


public:
	CNxCoCImportInfo m_ImportInfo;

	// (z.manning 2009-04-09 11:13) - PLID 33934 - We are going to use the cycle of care importer
	// as the EMR standard importer. Though we'll have to change a few things including the wording
	// so this variable tells us if this is an EMR std import;
	BOOL m_bEmrStandard;

	// (z.manning 2009-04-09 11:21) - PLID 33934 - This function will return text specific to what
	// type of import we're doing i.e. "Cycle of Care" or "EMR Standard"
	CString GetImportTypeText();

protected:
	//Members
	long m_nActiveSheetIndex;
	CArray<CNxCoCWizardSheet*, CNxCoCWizardSheet*> m_arypWizardSheets;

	//Functionality
	void SetActiveSheet(int nSheetIndex);
	void RefreshButtons();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedNext();
	afx_msg void OnBnClickedBack();
	afx_msg void OnBnClickedImport();
	afx_msg void OnCancel();
protected:
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnImport;
};
