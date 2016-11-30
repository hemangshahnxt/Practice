#pragma once


// CPharmacyIDDlg dialog

//DRT 11/19/2008 - PLID 32092 - Created
class CPharmacyIDDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CPharmacyIDDlg)

public:
	CPharmacyIDDlg(CWnd* pParent);   // standard constructor
	virtual ~CPharmacyIDDlg();

	//Parameters that should be provided by the caller
	long m_nPharmacyID;		//LocationsT.ID
	CString m_strPharmacyName;	//LocationsT.Name

// Dialog Data
	enum { IDD = IDD_PHARM_ID_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();

	//Dialog Functionality
	bool SaveToData();
	void LoadFromData();

	// (a.walling 2009-03-30 10:19) - PLID 33729 - Get the linked status and save the original NCPDPID
	CString m_strOriginalNCPDPID;
	_variant_t m_varLinkToDirectory;

	//Control variables
	CNxStatic m_nxstaticPharmName;
	CNxEdit m_nxeditNCPDPID;
	CNxEdit m_nxeditFileID;
	CNxEdit m_nxeditStateLicNum;
	CNxEdit m_nxeditMedicareNum;
	CNxEdit m_nxeditMedicaidNum;
	CNxEdit m_nxeditPPONum;
	CNxEdit m_nxeditPayerID;
	CNxEdit m_nxeditBINLocNum;
	CNxEdit m_nxeditDEANum;
	CNxEdit m_nxeditHIN;
	CNxEdit m_nxeditNAICCode;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;


	DECLARE_MESSAGE_MAP()
};
