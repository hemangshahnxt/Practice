#pragma once

//TES 7/14/2010 - PLID 39635 - Created
// CHL7SelectInsCoDlg dialog
#include "HL7ParseUtils.h"

class CHL7SelectInsCoDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CHL7SelectInsCoDlg)

public:
	CHL7SelectInsCoDlg(CWnd* pParent);   // standard constructor
	virtual ~CHL7SelectInsCoDlg();

	//TES 7/14/2010 - PLID 39635 - Variables to initialize the dialog
	CString m_strGroupName;
	HL7_PIDFields m_PID;
	HL7_InsuranceFields m_Insurance;
	long m_nInsurancePlacement;
	long m_nPatientID; //-1 for new patients

	//TES 7/14/2010 - PLID 39635 - Output.  -2 = Create New, -1 = Cancelled, positive equals InsuranceCoT.PersonID
	// that they selected.
	long m_nInsuranceCoID;
	CString m_strInsuranceCoName;

// Dialog Data
	enum { IDD = IDD_HL7_SELECT_INS_CO_DLG };

protected:
	CNxStatic m_nxsCaption;
	CNxStatic m_nxsName;
	CNxStatic m_nxsPlacement;
	CNxStatic m_nxsAddress1;
	CNxStatic m_nxsAddress2;
	CNxStatic m_nxsCity;
	CNxStatic m_nxsState;
	CNxStatic m_nxsZip;
	CNxIconButton m_nxbOK;
	CNxIconButton m_nxbCancel;
	CNxIconButton m_nxbCreateNew;

	NXDATALIST2Lib::_DNxDataListPtr m_pInsCoList;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCreateNewInsCo();
};
