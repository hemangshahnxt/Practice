#pragma once

//TES 2/24/2012 - PLID 44841 - Created
// CConfigureReportViewDlg dialog

// (r.gonet 03/07/2013) - PLID 44465 - The two font types we allow for the report view.
// Saved to ConfigRT.IntParam
enum ELabReportViewFontType
{
	lrvftProportional = 0,
	lrvftMonospaced = 1,
};

class CConfigureReportViewDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CConfigureReportViewDlg)

public:
	CConfigureReportViewDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CConfigureReportViewDlg();

// Dialog Data
	enum { IDD = IDD_CONFIGURE_REPORT_VIEW_DLG };

protected:
	CNxIconButton m_nxbOK, m_nxbCancel;
	NXDATALIST2Lib::_DNxDataListPtr m_pFieldList;
	// (r.gonet 03/07/2013) - PLID 44465 - Added two radio buttons to control the proportionality of the value font.
	NxButton m_radioProportionalFont;
	NxButton m_radioMonospacedFont;
	// (r.gonet 03/07/2013) - PLID 44465 - Also need to explain it to the user.
	CNxIconButton m_btnHelpFontType;
	// (r.gonet 03/07/2013) - PLID 43599 - Added a checkbox to trim consecutive spaces in values.
	NxButton m_checkTrimExtraSpaces;

	//TES 3/26/2012 - PLID 49208 - Color the row as "greyed out" if it's not being shown, normal otherwise
	void ColorRow(NXDATALIST2Lib::IRowSettingsPtr pRow);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	// (r.gonet 03/07/2013) - PLID 44465 - Added
	afx_msg void OnBtnFontTypeHelp();
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	void OnEditingStartingReportViewFields(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
	void OnEditingFinishingReportViewFields(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	void OnEditingFinishedReportViewFields(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
};
