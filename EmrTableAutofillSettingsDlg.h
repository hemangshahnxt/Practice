#pragma once


// CEmrTableAutofillSettingsDlg dialog
// (z.manning 2011-03-18 17:27) - PLID 23662 - Created

class CEmrInfoDataElement;

class CEmrTableAutofillSettingsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEmrTableAutofillSettingsDlg)

public:
	CEmrTableAutofillSettingsDlg(CEmrInfoDataElement *peide, CWnd* pParent);   // standard constructor
	virtual ~CEmrTableAutofillSettingsDlg();

// Dialog Data
	enum { IDD = IDD_EMR_EDIT_TABLE_AUTOFILL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	CEmrInfoDataElement *m_peide;

	CNxColor m_nxclrBackground;
	CNxIconButton m_btnOk;
	CNxIconButton m_btnCancel;

	void Load();
	BOOL ValidateAndSave();

	DECLARE_MESSAGE_MAP()
};
