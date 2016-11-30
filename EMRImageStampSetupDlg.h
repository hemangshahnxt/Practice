#pragma once

// CEMRImageStampSetupDlg dialog

// (j.jones 2010-02-10 15:39) - PLID 37224 - created

#include "EmrRc.h"

class CEMRImageStampSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEMRImageStampSetupDlg)

public:
	CEMRImageStampSetupDlg(CWnd* pParent);   // standard constructor
	virtual ~CEMRImageStampSetupDlg();

	BOOL m_bChanged;

// Dialog Data
	enum { IDD = IDD_EMR_IMAGE_STAMP_SETUP_DLG };
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnDelete;
	// (j.jones 2010-02-16 12:22) - PLID 37377 - added ability to show inactive stamps
	NxButton		m_checkShowInactive;
	// (b.spivey, August 20, 2012) - PLID 52130 - For image stamp categories. 
	CNxIconButton	m_btnEditCategories;

protected:
	// (a.walling 2013-03-20 13:57) - PLID 55787 - Keep track of icons to delete
	std::vector<shared_ptr<HICON__>> m_imageIcons;

	NXDATALIST2Lib::_DNxDataListPtr m_StampList;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	void HandleStampChange(); // (z.manning 2010-02-15 13:10) - PLID 37226

	// (z.manning 2010-09-08 10:00) - PLID 39490 - Move this logic to its own function
	void PromptForStampColor(LPDISPATCH lpRow);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBtnAddStamp();
	afx_msg void OnBtnDeleteStamp();
	// (b.spivey, August 20, 2012) - PLID 52130 - for editing stamp categories. 
	afx_msg void OnBtnEditCategories(); 
	DECLARE_EVENTSINK_MAP()
	void OnRButtonDownStampList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void OnEditingFinishingStampList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	void OnEditingFinishedStampList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	// (z.manning 2010-02-15 13:10) - PLID 37226
	void LeftClickStampList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	// (r.gonet 05/02/2012) - PLID 49949 - Removes the image from the selected row and removes the image from the data referenced by the row as well.
	void UnselectImage(NXDATALIST2Lib::IRowSettingsPtr pRow);
	// (r.gonet 05/02/2012) - PLID 49949 - Loads an image accounting for the extension. Returns true if success. False otherwise.
	bool LoadImg(IN CString strFilePath, OUT CxImage &xImage);
	// (r.gonet 05/02/2012) - PLID 49949 - Prompts the user to select an image from the file dialog and then updates the row and the stamp in data to use this image.
	bool SelectImage(NXDATALIST2Lib::IRowSettingsPtr pRow);
	afx_msg void OnBtnStampClose();
	// (j.jones 2010-02-16 12:22) - PLID 37377 - added ability to show inactive stamps
	afx_msg void OnCheckShowInactiveStamps();
	// (r.gonet 05/02/2012) - PLID 49949 - Updates the image preview column for a row.
	void UpdateImage(NXDATALIST2Lib::IRowSettingsPtr pRow);
	void RequeryFinishedStampList(short nFlags); // (z.manning 2010-09-08 10:00) - PLID 39490
	// (j.jones 2010-09-27 09:49) - PLID 39403 - added to check permissions
	void OnEditingStartingStampList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
};
