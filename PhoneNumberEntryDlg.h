#pragma once


// PhoneNumberEntryDlg dialog

class PhoneNumberEntryDlg : public CNxDialog
{
	DECLARE_DYNAMIC(PhoneNumberEntryDlg)

public:
	PhoneNumberEntryDlg(CWnd* pParent);   // standard constructor
	virtual ~PhoneNumberEntryDlg();

// Dialog Data
	enum { IDD = IDD_PHONE_NUMBER_ENTRY };

protected:
    NXDATALIST2Lib::_DNxDataListPtr m_pCountryList;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	void UpdatePhoneNumberPreview();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnAreaCode_Changed();
	afx_msg void OnLocalNumber_Changed();
	afx_msg void OnExtension_Changed();

private:
	CString countryCodeString;
	CString areaCodeString;
	CString localNumberString;
	CString extensionString;

public:
	CString phoneNumber;
	DECLARE_EVENTSINK_MAP()
	void SelChosenCountryRegion(LPDISPATCH lpRow);
};
