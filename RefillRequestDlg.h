#pragma once

// (a.walling 2014-04-24 12:00) - VS2013 - MSXML Import
#include <NxDataUtilitiesLib/NxMsxml6Import.h>
#include "SureScriptsPractice.h"

// CRefillRequestDlg dialog
// (a.walling 2009-04-24 13:59) - PLID 33897 - Dialog to handle refill requests and responding to them

class CRefillRequestDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CRefillRequestDlg)

public:
	CRefillRequestDlg(CWnd* pParent);   // standard constructor
	virtual ~CRefillRequestDlg();

// Dialog Data
	enum { IDD = IDD_REFILL_REQUEST_DLG };

	MSXML2::IXMLDOMDocument2Ptr m_pMessage;
	SureScripts::ActionType m_atAction;
	
	CString m_strPrescriberOrderNumber;
	long m_nPrescriptionID;

	long m_nPatientID; // NxServer may have found this for us.
	long m_nMedicationID;
	long m_nPharmacyID;
	CString m_strPharmacyNCPDPID;
	long m_nPrescriberID;
	long m_nLocationID;
	CString m_strRxReferenceNumber;
	CString m_strPatientExplanation;

	CString m_strOriginalRefillRequested;

	// (a.walling 2009-07-08 08:32) - PLID 34261
	CString m_strDEASchedule;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	//TES 8/10/2009 - PLID 35131 - Stores information about the medication which we want to display to the user.
	CString m_strMedicationInfo;

	void LoadInfoFromMessage();
	void UpdateControls();

	// this should really be a shared utility function, but i'm already running out of time
	long GetNewMedication();

	CNxStatic m_nxsMedicationName;
	CNxStatic m_nxsNote;
	CNxStatic m_nxsExplanation;
	CNxStatic m_nxsDenialExplanation;
	CNxStatic m_nxsPrescriptionInfo;
	CNxStatic m_nxsDenialCode;

	CNxStatic m_nxsPatientInfo;
	CNxStatic m_nxsPrescriberInfo;
	CNxStatic m_nxsPharmacyInfo;

	CNxLabel m_nxlabelRefills;

	CNxEdit m_nxeditNote;
	CNxEdit m_nxeditRefills;

	NxButton m_nxgroupRequest;
	NxButton m_nxgroupResponse;

	CNxIconButton m_nxibApprove;
	CNxIconButton m_nxibDeny;
	CNxIconButton m_nxibDenySendNew;
	CNxIconButton m_nxibCancel;
	CNxIconButton m_nxibViewOriginal;
	CNxIconButton m_nxibAddPharmacy;
	CNxIconButton m_nxibFindPatient;
	// (a.walling 2009-07-06 15:39) - PLID 34263
	CNxIconButton m_nxibFindMedication;

	NXDATALIST2Lib::_DNxDataListPtr m_pDenialCodes;
	CNxColor m_nxcolor;

	CRect m_rcOriginalRefillRect;
	void ToggleRefillAsNeeded(BOOL bRefillAsNeeded);

	//TES 4/24/2009 - PLID 33901 - Remember that a patient has been manually selected, and thus should have 
	// SureScriptsPatientLinkT updated accordingly.
	bool m_bUpdatePatientLink;
	//TES 4/24/2009 - PLID 33901 - This will tie m_nPatientID to the information embedded in m_pMessage, iff
	// m_bUpdatePatientLink is true.
	void UpdatePatientLink();

	
	// (a.walling 2009-07-08 09:02) - PLID 34261 - Deny and send new -- bApprovingScheduled should be TRUE if 
	// we don't need to change the medication
	void DenySendNew(BOOL bApprovingScheduled) throw(...);

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedRefillApprove();
	afx_msg void OnBnClickedRefillDeny();
	afx_msg void OnBnClickedRefillDenySendNew();
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg virtual void OnCancel();
	afx_msg virtual void OnOK();
	afx_msg void OnBnClickedRefillFindPatient();
	afx_msg void OnBnClickedRefillAddPharmacy();
	afx_msg void OnBnClickedRefillViewOriginal();
	afx_msg void OnBnClickedRefillFindMedication();
};
