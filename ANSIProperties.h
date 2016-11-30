#if !defined(AFX_ANSIPROPERTIES_H__A76E75FA_F02F_419E_94BC_FE7FB85091B0__INCLUDED_)
#define AFX_ANSIPROPERTIES_H__A76E75FA_F02F_419E_94BC_FE7FB85091B0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ANSIProperties.h : header file
//

#include "FinancialRc.h"

enum ANSIVersion;

/////////////////////////////////////////////////////////////////////////////
// CANSIProperties dialog

class CANSIProperties : public CNxDialog
{
// Construction
public:

	NXDATALISTLib::_DNxDataListPtr m_ANSIList;

	CANSIProperties(CWnd* pParent);   // standard constructor

	BOOL Save();
	void Load();

	long m_FormatID;

	BOOL m_bShowAdvOptions;

	void ShowAdvOptions(BOOL bShow);

	void DisableBoxes(BOOL bEnable);

	// (j.jones 2008-02-15 11:51) - PLID 28943 - added checkbox for SendSubscriberPerClaim
	// (j.jones 2008-02-19 11:45) - PLID 29004 - added checkbox for Send2330BAddress
	// (j.jones 2008-05-07 10:58) - PLID 29854 - added nxiconbuttons for modernization
	// (j.jones 2008-09-09 16:44) - PLID 26482 - added m_checkHide2300PatAmtPaidWhenZero
	// (j.jones 2008-10-13 12:50) - PLID 31636 - added m_nxeditFilenameElig and m_nxstaticFilenameEligLabel

// Dialog Data
	//{{AFX_DATA(CANSIProperties)
	enum { IDD = IDD_ANSI_PROPERTIES };
	NxButton	m_checkHide2300PatAmtPaidWhenZero;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnDelete;
	CNxIconButton	m_btnShowAdvanced;
	CNxIconButton	m_btnAdd;
	NxButton	m_checkExport2330BAddress;
	NxButton	m_checkSendSubscriberPerClaim;
	NxButton	m_checkFourDigitRevCode;
	NxButton	m_check1000APER;
	NxButton	m_checkSeparateBatchesByInsCo;
	NxButton	m_checkUseAddnl2010AAID;
	// (j.jones 2009-08-04 11:34) - PLID 14573 - renamed to m_checkPrependPayerNSF
	NxButton	m_checkPrependPayerNSF;
	NxButton	m_checkExport2330BPERSegment;
	NxButton	m_checkTruncateCurrency;
	NxButton	m_checkUseSSN;
	NxButton	m_checkExportAll2010AAIDs;
	// (j.jones 2009-08-04 11:34) - PLID 14573 - removed THIN IDs
	//NxButton	m_checkUseTHINPayerIDs;
	NxButton	m_checkUse2010AB;
	NxButton	m_DontSubmitSecondary;
	// (a.wilson 2014-06-27 09:12) - PLID 62517
	NxButton	m_DontSendSecondaryDiagnosisMismatch;
	NxButton	m_checkHide2310D;
	// (j.jones 2012-01-06 16:15) - PLID 47351 - added Hide2310CWhenBillLocation
	NxButton	m_checkHide2310CWhenBillLocation;
	NxButton	m_UnPunctuate;
	NxButton	m_checkUseSV106;	
	NxButton	m_radioIndiv;
	NxButton	m_radioGroup;
	NxButton	m_checkUse2420A;
	NxButton	m_zipfile;
	NxButton	m_capitalize;
	CNxEdit	m_nxeditReceiverName;
	CNxEdit	m_nxeditAnsiContact;
	CNxEdit	m_nxeditEditIsa07Qual;
	CNxEdit	m_nxeditReceiverIdIsa08;
	CNxEdit	m_nxeditReceiverIdGs03;
	CNxEdit	m_nxeditEdit1000BQual;
	CNxEdit	m_nxeditReceiverId1000B;
	CNxEdit	m_nxeditEditIsa05Qual;
	CNxEdit	m_nxeditSubmitterIdIsa06;
	CNxEdit	m_nxeditSubmitterIdGs02;
	CNxEdit	m_nxeditEdit1000AQual;
	CNxEdit	m_nxeditSubmitterId1000A;
	CNxEdit	m_nxeditEdit1000APer05Qual;
	CNxEdit	m_nxeditEdit1000APer06Id;
	CNxEdit	m_nxeditEditIsa01Qual;
	CNxEdit	m_nxeditEditIsa02;
	CNxEdit	m_nxeditEditIsa03Qual;
	CNxEdit	m_nxeditEditIsa04;
	CNxEdit	m_nxeditEditAddnl2010AaQual;
	CNxEdit	m_nxeditEditAddnl2010Aa;
	CNxEdit	m_nxeditFilenameAnsi;
	CNxEdit	m_nxeditFilenameElig;
	CNxStatic m_nxstaticFilenameEligLabel;
	// (j.jones 2009-10-01 14:02) - PLID 35711 - added ability to prepend a patient ID with a code
	NxButton	m_checkPrependPatientID;
	CNxEdit		m_nxeditPrependPatientID;
	// (j.jones 2010-10-08 17:15) - PLID 40878 - added 4010/5010 toggle
	NxButton	m_radio4010;
	NxButton	m_radio5010;
	// (j.jones 2012-07-20 11:32) - PLID 47901 - added control for the ANSI Version label
	CNxStatic m_nxstaticANSIVersionLabel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CANSIProperties)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CBrush m_brush;

	// (j.jones 2010-10-08 17:15) - PLID 40878 - added 4010/5010 toggle
	ANSIVersion m_avLastSavedANSIVersion;
	ANSIVersion m_avLastCheckedANSIVersion;

	// Generated message map functions
	//{{AFX_MSG(CANSIProperties)
	afx_msg void OnOkBtn();
	virtual BOOL OnInitDialog();
	afx_msg void OnAddAnsi();
	afx_msg void OnDeleteAnsi();
	afx_msg void OnSelChosenAnsiStyleList(long nRow);
	afx_msg void OnBtnShowAdvOptions();
	// (j.jones 2009-08-04 11:34) - PLID 14573 - removed THIN IDs
	//afx_msg void OnCheckUseThinPayerId();	
	afx_msg void OnCheckUseAdditional2010aaId();
	afx_msg void OnSeparateBatchByInsuranceCo();
	// (j.jones 2007-04-05 10:26) - PLID 25506 - added the 1000A PER05/06 override
	afx_msg void OnCheck1000aPer();
	// (j.jones 2009-10-01 14:02) - PLID 35711 - added ability to prepend a patient ID with a code
	afx_msg void OnCheckPrependPatientId();
	// (j.jones 2010-10-08 17:15) - PLID 40878 - added 4010/5010 toggle
	afx_msg void OnRadio4010();
	afx_msg void OnRadio5010();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedDontSubmitSecondaries();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ANSIPROPERTIES_H__A76E75FA_F02F_419E_94BC_FE7FB85091B0__INCLUDED_)
