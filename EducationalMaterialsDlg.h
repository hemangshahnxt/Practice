#pragma once
#include "PatientsRc.h"

// CEducationalMaterialsDlg dialog
//
// (c.haag 2010-09-23 09:15) - PLID 40640 - Initial implementation. This class is used to detect what patient educational
// templates need to be generated for this patient, and then merge them.
//

class CEducationalMaterialsThreadData
{
public:
	HWND hwndParent; // The window handle of the CEducationalMaterialsDlg object
	HWND hwndProgressBar; // Handle to the progress bar
	HWND hwndProgressText; // Handle to the progress text
public:
	long nPatientID; // Patient ID
	HANDLE hStopThread; // Event to halt the thread
};

class CEducationalMaterialsThreadMessage
{
public:
	long nFilterID; // Detected FiltersT.ID
	long nTemplateID; // Detected MergeTemplatesT.ID
};

class CEducationalMaterialsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEducationalMaterialsDlg)

public:
	CEducationalMaterialsDlg(CWnd* pParent);   // standard constructor
	virtual ~CEducationalMaterialsDlg();

// Dialog Data
	enum { IDD = IDD_EDUCATIONAL_MATERIALS };

private:
	NXDATALIST2Lib::_DNxDataListPtr m_dlList;
	CNxStatic m_staticProgress;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;

private:
	CWinThread* m_pThread;
	HANDLE m_hStopThread;

public:
	long m_nPatientID;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnOK();
	afx_msg LRESULT OnProcessingAddRow(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnProcessingFinished(WPARAM wParam, LPARAM lParam);
};
