#pragma once
#include "AdministratorRc.h"

// (s.tullis 2014-05-21 17:34) - PLID 62018 - Remove the Claim Setup infomation from the Admin Billing dialog to a new dialog that launches from the Additional Service Code Setup Menu
class CClaimSetup : public CNxDialog
{
	// Construction
public:
	CClaimSetup(CWnd* pParent);   // standard constructor
	long m_nServiceID;
	CString strCodeName;
	CString strSubCodeName;
	CString strCPTCodeDesc;
	CString m_strServCode;
	CString m_strClaimNote;
	// (r.gonet 07/08/2014) - PLID 62572 - Changed to BOOLs from longs
	BOOL m_bReqRefPhy;
	BOOL m_bBatchIfZero;
	// (r.gonet 07/08/2014) - PLID 62572 - Added DefaultAsOnHold to store the old value of the ChargesT.DefaultAsOnHold BIT field.
	BOOL m_bDefaultAsOnHold;
	// (r.gonet 08/06/2014) - PLID 63096 - Added IsLabCharge to store the old value of the ServiceT.LabCharge BIT field.
	BOOL m_bIsLabCharge;
	long eOldNocType;
	BOOL NocWarnedOnce;

	CNxLabel m_nxlNOC;
	NxButton m_checkRequireRefPhys;
	NxButton m_checkBatchWhenZero;
	// (r.gonet 07/08/2014) - PLID 62572 - Added a checkbox for Default As On Hold
	NxButton m_checkDefaultAsOnHold;
	// (r.gonet 08/06/2014) - PLID 63096 - Added a checkbox for Is Lab Charge
	NxButton m_checkIsLabCharge;
	CNxIconButton m_btnOk;
	NXDATALIST2Lib::_DNxDataListPtr m_NOCCombo;

	// Dialog Data
	enum { IDD = IDD_BILLTAB_CLAIM_SETUP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	BOOL OnInitDialog();
	BOOL PreTranslateMessage(MSG* pMsg);
	void SecureControls();
	LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	void PopupNOCDescription();
	CString CheckClaimNote(CString claimnote);
	// (r.gonet 07/08/2014) - PLID 62572 - Validates the changes before saving. Returns true if validation succeeded and false otherwise.
	bool Validate();
	afx_msg void OnBnClickedCptRequireRefPhys();
	afx_msg void OnCheckBatchWhenZero();
	afx_msg void OnCloseClick();
	afx_msg void OnClickNDCdefault();
	// (r.gonet 05/22/2014) - PLID 61832 - Open the charge level claim providers config dialog
	afx_msg void OnConfigureProviders();
	DECLARE_MESSAGE_MAP()
};
