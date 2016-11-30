#pragma once

#include "DevicePluginImportDlg.h"
//(e.lally 2011-03-30) PLID 42733 - Created
// CDeviceImportCombineFilesDlg dialog

class CDeviceImportCombineFilesDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CDeviceImportCombineFilesDlg)

public:
	CDeviceImportCombineFilesDlg(CWnd* pParent);   // standard constructor
	virtual ~CDeviceImportCombineFilesDlg();

	void LoadFiles(long nPreselectPatientID, long nPreselectCategoryID, CStringArray& aryFilesToCombine);

// Dialog Data
	enum { IDD = IDD_DEVICE_IMPORT_COMBINE_FILES_DLG };

protected:
	CNxIconButton m_btnImportHistory;
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnMoveUp;
	CNxIconButton m_btnMoveDown;
	NxButton m_chkShowFullFilePath;
	// (d.lange 2011-05-04 12:15) - PLID 43253 - Added checkbox for creating a ToDo Alarm
	NxButton m_chkCreateToDo;

	NXDATALIST2Lib::_DNxDataListPtr m_pPatientCombo, m_pCategoryCombo, m_pFileList;
	CStringArray m_aryFilesToCombine;
	long m_nSelectedPatientID;
	long m_nSelectedCategoryID;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	void LoadFilesIntoList();
	bool IsValidState();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedMoveUp();
	afx_msg void OnBnClickedMoveDown();
	//(e.lally 2011-04-22) PLID 42733
	afx_msg void OnShowFullFilePath();
	afx_msg void OnBnClickedChkCreateToDo();
};
