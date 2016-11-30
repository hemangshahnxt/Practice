#pragma once

#include "PatientsRc.h"

// (d.singleton 2013-06-05 14:01) - PLID 57057 - added new dialog to choose hl7 group id, facility name, facility id for immunization hl7 export


// CImmunizationExportConfig dialog

class CImmunizationExportConfig : public CNxDialog
{
	DECLARE_DYNAMIC(CImmunizationExportConfig)

public:
	CImmunizationExportConfig(CWnd* pParent = NULL);   // standard constructor
	virtual ~CImmunizationExportConfig();

// Dialog Data
	enum { IDD = IDD_IMMUNIZATION_EXPORT };

	CString m_strFacilityName;
	CString m_strFacilityID;
	long m_nHL7GroupID;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	BOOL OnInitDialog();

	CNxIconButton m_nxbOK;
	CNxIconButton m_nxbCANCEL;
	NXDATALIST2Lib::_DNxDataListPtr m_dlHL7GroupID;

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	afx_msg void OnBnClickedOk();
};
