//DRT 10/28/2008 - PLID 31789 - Created for NxCoC Importer

#pragma once

// CNxCoCMainSheet dialog
#include "NxCoCImportWizardSheet.h"

class CNxCoCMainSheet : public CNxCoCWizardSheet
{
	DECLARE_DYNAMIC(CNxCoCMainSheet)

public:
	CNxCoCMainSheet(CNxCoCWizardMasterDlg* pParent = NULL);   // standard constructor
	virtual ~CNxCoCMainSheet();

// Dialog Data
	enum { IDD = IDD_NXCOC_MAIN_SHEET };

protected:
	bool m_bNeedToLoadContent;
	DWORD m_dwCompressedFileSize;

	BOOL Validate();
	void EnableDownloadButtons(BOOL bEnable);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBrowse();
	afx_msg void OnBnClickedDownload();
	afx_msg LRESULT OnTransferBegin(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTransferProgress(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTransferEnd(WPARAM wParam, LPARAM lParam);


	DECLARE_MESSAGE_MAP()
	NxButton m_btnUsePic;
	NxButton m_btnUseLW;
	CNxEdit m_editPath;
	CNxStatic m_nxstaticWelcomeText; // (z.manning 2009-04-09 12:21) - PLID 33934
	CNxStatic m_nxstaticDownloadText;
};
