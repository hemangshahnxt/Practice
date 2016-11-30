#pragma once

// (j.jones 2011-06-17 08:39) - PLID 44157 - created

// CNewCropNameOverridesDlg dialog

class CNewCropNameOverridesDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CNewCropNameOverridesDlg)

public:
	CNewCropNameOverridesDlg(CWnd* pParent);   // standard constructor
	virtual ~CNewCropNameOverridesDlg();

// Dialog Data
	enum { IDD = IDD_NEWCROP_NAME_OVERRIDES_DLG };
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	// (j.jones 2010-04-13 16:09) - PLID 38183 - added ability to suppress the suffix
	NxButton	m_checkSuppressSuffix;

protected:

	// (j.jones 2010-07-29 10:36) - PLID 39880 - added ability to override provider & user names
	NXDATALIST2Lib::_DNxDataListPtr m_OverrideNameList;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	virtual BOOL OnInitDialog();
	// (j.jones 2010-07-29 10:53) - PLID 39880 - added ability to override provider & user names
	void OnEditingFinishingNewcropSetupNameOverrideList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	afx_msg void OnOk();
};
