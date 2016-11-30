#pragma once
#include "DevicePluginUtils.h"

using namespace DevicePluginUtils;
// (d.lange 2010-06-04 15:08) - PLID 39023 - Created
// CDevicePatientSelectDlg dialog

enum PatientComboColumn {
	pccID = 0,
	pccLastName,
	pccFirstName,
	pccMiddleName,	
	pccPatientID,
	pccPatientBirthDate,
};

class CDevicePatientSelectDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CDevicePatientSelectDlg)

public:
	CDevicePatientSelectDlg(CWnd* pParent, PatientElement *pePatient = NULL, long nPatientID = -1);   // standard constructor
	virtual ~CDevicePatientSelectDlg();

// Dialog Data
	enum { IDD = IDD_DEVICE_PATIENT_SELECT_DLG };

	// (j.jones 2011-02-03 17:17) - PLID 42322 - turned the close into OK/Cancel
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxStatic m_nxsFirst;
	CNxStatic m_nxsBirthDate;
	CNxStatic m_nxsGender;
	CNxStatic m_nxsSocial;

	long m_nPatientID;
	long m_nUserDefinedID;
	CString m_strPatientFirst;
	CString m_strPatientLast;
	CString m_strPatientMiddle;
	PatientElement *m_pePatient;

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	// (j.jones 2011-02-03 17:17) - PLID 42322 - added OnCancel
	virtual void OnCancel();

	void PopulatePatientDemographics();

	NXDATALIST2Lib::_DNxDataListPtr m_pPatientCombo;
	
	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	void SelChosenPatientDropdown(LPDISPATCH lpRow);
	void SelChangingPatientDropdown(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
};
