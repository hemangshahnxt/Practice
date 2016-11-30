#pragma once


//TES 5/5/2009 - PLID 34178 - Created
// CSureScriptsConfirmationDlg dialog

class CSureScriptsConfirmationDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CSureScriptsConfirmationDlg)

public:
	CSureScriptsConfirmationDlg(CWnd* pParent);   // standard constructor
	virtual ~CSureScriptsConfirmationDlg();

	// (a.walling 2009-07-01 11:56) - PLID 34052 - Support Agent, Supervisor names
	// (a.walling 2009-11-18 15:41) - PLID 36349 - Also gender
	int Open(const CString &strPatientName, long nPatientID, COleDateTime dtPatientBirthDate, long nPatientGender, 
		const CString &strPharmacyName, const CString &strPharmAddress1, const CString &strPharmPhone, const CString &strPharmNCPDP,
		const CString &strPrescriber, const CString &strPrescriberSPI,
		const CString &strDrugName, const CString &strSIG, const CString &strQuantity, const CString &strQuantityUnit,
		BOOL bAllowSubstitutions, const CString &strRefills, const CString &strNotes, const CString& strAgent, const CString& strSupervisor);
protected:
	CNxIconButton m_btnSend;
	CNxIconButton m_btnCancel;
	CNxStatic m_nxsPrescriptionInfo;
	CNxColor m_bkg;

	//TES 5/5/2009 - PLID 34178 - Our full caption, compiled from the various parameters passed into the Open() function.
	CString m_strCaption;
// Dialog Data
	enum { IDD = IDD_SURESCRIPTS_CONFIRMATION_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
};
