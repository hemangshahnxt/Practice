#if !defined(AFX_MERGEPATIENTSDLG_H__80EC98C6_1F46_46E1_B929_EB24B96BB551__INCLUDED_)
#define AFX_MERGEPATIENTSDLG_H__80EC98C6_1F46_46E1_B929_EB24B96BB551__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MergePatientsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMergePatientsDlg dialog

// (b.savon 2013-08-09 11:46) - PLID 55400
enum MergePatientInsuranceInformation
{
	mpiiPrompt = 0,
	mpiiMergedIntoPatient = 1,
	mpiiMergedFromPatient = 2,
};

struct MergeInsuredPartyInformation
{
	long nPatientID;
	CMap<long, long, long, long> mapInsuredPartyID;
	CMap<long, long, bool, bool> mapInsuranceCoID;

	MergeInsuredPartyInformation()
	{
		nPatientID = -1;
	}
};

class CMergePatientsDlg : public CNxDialog
{
// Construction
public:
	CMergePatientsDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMergePatientsDlg)
	enum { IDD = IDD_MERGE_PATIENTS_DLG };
	NxButton	m_btnUpdateDemographics;
	NxButton	m_btnNoDemographics;
	CNxStatic	m_nxstaticStatusPatientName;
	CNxStatic	m_nxstaticStatusPatientId;
	CNxStatic	m_nxstaticStatusPatientSsn;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	// (b.savon 2013-08-12 13:16) - PLID 55400
	NxButton	m_rdoPrompt;
	NxButton	m_rdoUseFrom;
	NxButton	m_rdoUseInto;
	// (b.savon 2014-12-02 07:34) - PLID 58850 - Connectathon - Support new HL7 message type ADT^A40^ADT_A39 - Merge patient
	NxButton	m_chkSendViaHL7;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMergePatientsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pPatList;

	// (j.armen 2012-04-10 14:21) - PLID 49553 - Parameratized Functions
	CSqlFragment GenerateMoveString(const CString& strTable, const CString& strField);
	BOOL EnsureCurrentPatient();
	BOOL MovePatient();
	void AddInsuredMoveInfo(CSqlFragment& sqlExecute);
	void AddLinkMoveInfo(CSqlFragment& sqlExecute, const CString& strField);
	CSqlFragment GeneratePatientDataString(const CString& strField, const CString& strTable);
	// (b.savon 2013-08-09 11:48) - PLID 55400
	MergePatientInsuranceInformation GetInsuranceMergeBehavior();
	BOOL PromptForInsurance();
	BOOL UseMergedIntoPatientInsurance();
	BOOL UseMergedFromPatientInsurance();
	void ApplyInsuredPartyMergeControl();
	void SaveInsuredPartyMergeBehavior();
	void PerformLegacyInsuranceBehavior(CSqlFragment& sqlExecute);
	void UsePatientFromInsurance(CSqlFragment& sqlExecute);
	void UsePatientToInsurance(CSqlFragment& sqlExecute);
	void MigrateInsuredPartyInformation(MergePatientInsuranceInformation keepWho, long nDeletePatient, long nKeepPatient, CSqlFragment& sqlExecute);
	void MoveInsuredPartyTransferInformation(long nFromParty, long nToParty, CSqlFragment& sqlExecute);
	// (b.savon 2014-12-02 07:34) - PLID 58850 - Connectathon - Support new HL7 message type ADT^A40^ADT_A39 - Merge patient
	void SendViaHL7();
	long m_nCurrentUserDefinedID;

	long m_nCurrentID;
	long m_nNewID;

	long m_nCurrentFamilyID;
	long m_nNewFamilyID;

	CFont* m_pNameFont;

	// Generated message map functions
	//{{AFX_MSG(CMergePatientsDlg)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MERGEPATIENTSDLG_H__80EC98C6_1F46_46E1_B929_EB24B96BB551__INCLUDED_)
