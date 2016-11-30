#pragma once

// (r.goldschmidt 2015-04-07 15:41) - PLID 65480 - Create Configure Analytics License dialog and all of its UI elements.
// CAnalyticsLicensingConfigDlg dialog

enum EAnalyticsLicenseAction {
	alaAdd,
	alaEdit,
	alaRemove
};

// (r.goldschmidt 2015-04-07 15:41) - PLID 65480 - This defines the class for a license/user combo
struct CAnalyticsLicenseUserCombo
{
public:
	COleDateTime m_dtExpirationDate;
	CString m_strUserName, m_strDatabaseName, m_strDocumentName;
	long m_nDocumentID;

	void operator =(const CAnalyticsLicenseUserCombo &s)
	{
		m_dtExpirationDate = s.m_dtExpirationDate;
		m_strUserName = s.m_strUserName;
		m_strDatabaseName = s.m_strDatabaseName;
		m_strDocumentName = s.m_strDocumentName;
		m_nDocumentID = s.m_nDocumentID;
	}

	CAnalyticsLicenseUserCombo()
	{
		// had to use SQL to account for leap year shenanigans
		// defaults expiration date to one year plus one day in the future
		ADODB::_RecordsetPtr prs = CreateParamRecordset("SELECT Dbo.AsDateNoTime(DATEADD(DD, 1, DATEADD(YY, 1, GetDate()))) AS Expiration");
		m_dtExpirationDate = AdoFldDateTime(prs, "Expiration");

		m_strUserName = "";
		m_strDatabaseName = "PracData";
		m_strDocumentName = "";
		m_nDocumentID = -1;
	}

	CAnalyticsLicenseUserCombo(const COleDateTime &dt)
	{
		m_dtExpirationDate = dt;
		m_strUserName = "";
		m_strDatabaseName = "PracData";
		m_strDocumentName = "";
		m_nDocumentID = -1;
	}

	CAnalyticsLicenseUserCombo(const CAnalyticsLicenseUserCombo &s)
		: m_dtExpirationDate(s.m_dtExpirationDate)
		, m_strUserName(s.m_strUserName)
		, m_strDatabaseName(s.m_strDatabaseName)
		, m_strDocumentName(s.m_strDocumentName)
		, m_nDocumentID(m_nDocumentID)
	{
	}

	bool IsSameExpirationDate(const CAnalyticsLicenseUserCombo &s)
	{
		return (m_dtExpirationDate == s.m_dtExpirationDate);
	}

	bool IsSameUserNameDocumentDatabaseCombo(const CAnalyticsLicenseUserCombo &s)
	{
		bool bReturnValue = false;
		if (!m_strUserName.Compare(s.m_strUserName)){ // if user names are the same
			if (!m_strDatabaseName.Compare(s.m_strDatabaseName)){ // and if the database names are the same
				if (m_nDocumentID == s.m_nDocumentID){ // and if the document ids are the same
					bReturnValue = true; // then the combo is the same
				}
			}
		}
		return bReturnValue;
	}

	bool IsValidUserNameDocumentDatabaseCombo()
	{
		if (m_strUserName.IsEmpty() && (m_nDocumentID != -1)){
			return false;
		}
		if (!m_strUserName.IsEmpty() && (m_nDocumentID == -1)){
			return false;
		}
		if ((!m_strUserName.IsEmpty() || m_nDocumentID != -1) && m_strDatabaseName.IsEmpty()){
			return false;
		}
		return true;
	}

	bool IsUserNameEmpty()
	{
		return m_strUserName.IsEmpty();
	}

};

class CAnalyticsLicensingConfigDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CAnalyticsLicensingConfigDlg)

public:
	CAnalyticsLicensingConfigDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAnalyticsLicensingConfigDlg();

	bool m_bIsTestAccount; // (r.goldschmidt 2015-12-10 12:56) - PLID 67697 - new permission, now need to know if test account

// Dialog Data
	enum { IDD = IDD_ANALYTICSLICENSINGCONFIG_DLG };

	void SetSaveType(EAnalyticsLicenseAction eAction);
	void SetClient(long nClientID);
	void SetLicense(long nLicenseID);
	void SetInitialUser(const CAnalyticsLicenseUserCombo &structLicenseUser);
	COleDateTime GetSavedUserExpiration();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	CNxColor m_background;

	CAnalyticsLicenseUserCombo m_structInitialLicenseUserCombo, m_structNewLicenseUserCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_pDocumentList; // datalist of analytics licenses
	bool m_bExpirationDatePast; // true if expiration date is set in the past
	EAnalyticsLicenseAction m_eAction; // action that is being performed by dialog
	long m_nClientID; // ID of client
	long m_nLicenseID; // ID for license being edited
	CDateTimePicker m_dtExpirationPicker;

	virtual BOOL OnInitDialog();
	void EnsureControls();
	bool Validate();
	void Save();
	void UpdateNewUser();

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnDtnDatetimechangeExpirationdate(NMHDR *pNMHDR, LRESULT *pResult);
	DECLARE_EVENTSINK_MAP()

	afx_msg void OnBnClickedSaveClose();
	afx_msg void OnBnClickedSaveAddanother();
};
