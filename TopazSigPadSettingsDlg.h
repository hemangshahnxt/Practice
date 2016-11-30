#pragma once


// CTopazSigPadSettingsDlg dialog
// (d.singleton 2013-05-03 09:12) - PLID 56520 - need to be able to set the COM port the Topaz sig pad is using.  Then read that value when initializing the pad.

class CTopazSigPadSettingsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CTopazSigPadSettingsDlg)

public:
	CTopazSigPadSettingsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTopazSigPadSettingsDlg();

// Dialog Data
	enum { IDD = IDD_TOPAZ_SIGNATURE_SETUP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	BOOL OnInitDialog();
	void RefreshControls(BOOL bShow);

	NXDATALIST2Lib::_DNxDataListPtr m_dlComPorts;
	NxButton m_nxbOn;
	NxButton m_nxbOff;
	CNxIconButton m_nxbOK;
	CNxIconButton m_nxbCancel;
	
	long m_nOriginalPort;
	long m_nOriginalStatus;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedRadioTopazOn();
	afx_msg void OnBnClickedRadioTopazOff();
	DECLARE_EVENTSINK_MAP()
	void SelChosenComPorts(LPDISPATCH lpRow);
public:
	long m_nCurrentPort;
	long m_nCurrentStatus;	
};
