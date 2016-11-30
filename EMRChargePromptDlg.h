#if !defined(AFX_EMRCHARGEPROMPTDLG_H__8EE1A780_5317_4FA0_9146_353FAAB8C673__INCLUDED_)
#define AFX_EMRCHARGEPROMPTDLG_H__8EE1A780_5317_4FA0_9146_353FAAB8C673__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMRChargePromptDlg.h : header file
//

// (j.jones 2013-05-16 14:35) - PLID 56596 - removed EMN.h and added forward declares
class CEmrCodingGroup;
class CEMNChargeArray;

/////////////////////////////////////////////////////////////////////////////
// CEMRChargePromptDlg dialog

class CEMRChargePromptDlg : public CNxDialog
{
// Construction
public:
	// (z.manning 2011-07-11 14:11) - PLID 44469 - Added optional param for coding group
	// (j.jones 2012-01-26 11:19) - PLID 47700 - added patient ID, and an optional EMNID
	CEMRChargePromptDlg(CWnd* pParent, long nPatientID, CEMNChargeArray *parypCharges, long nEMNID = -1,
		CEmrCodingGroup *pCodingGroup = NULL, CEmnCodingGroupInfo *pEmnCodingInfo = NULL);   // standard constructor
	~CEMRChargePromptDlg();

// Dialog Data
	//{{AFX_DATA(CEMRChargePromptDlg)
	enum { IDD = IDD_EMR_CHARGE_PROMPT_DLG };
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMRChargePromptDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pList;
	enum EChargeListColumns {
		clcCode = 0,
		clcSubCode,
		clcCategory,// (s.tullis 2015-04-01 14:09) - PLID 64978 - Added Charge Category
		clcDescription,
		clcQuantity,
		clcMod1,
		clcMod2,
		clcMod3,
		clcMod4,
		clcUnitCost,
		clcInsuredPartyID,	// (j.jones 2012-01-26 10:59) - PLID 47700 - added Resp. dropdown
	};

	CNxColor m_nxcolorBackground;

	// (z.manning 2011-07-07 16:15) - PLID 44469 - Changed this to an array as we now support multiple charges
	CEMNChargeArray *m_parypOriginalCharges;
	// (j.jones 2013-05-16 15:12) - PLID 56596 - converted to be a reference so we don't need to include a .h for it
	CEMNChargeArray &m_arypWorkingCharges;
	CMap<EMNCharge*,EMNCharge*,EMNCharge*,EMNCharge*> m_mapWorkingChargeToOriginal;

	// (s.tullis 2015-04-14 11:47) - PLID 64978 - added charge category
	void SetCPTCategoryCombo(NXDATALIST2Lib::IRowSettingsPtr pRow, EMNCharge* pCharge);
	NXDATALIST2Lib::IFormatSettingsPtr GetCPTMultiCategoryCombo(EMNCharge* pCharge);
	void ForceShowColumn(short iColumnIndex, long nPercentWidth, long nPixelWidth);

	// (j.jones 2012-01-26 11:19) - PLID 47700 - added patient ID, and an optional EMNID
	long m_nPatientID;
	long m_nEMNID;

	// (s.tullis 2015-04-14 11:36) - PLID 65539 - Templates will never display the category column in the Codes topic.
	BOOL m_bIsTemplate;

	// (z.manning 2011-07-11 16:27) - PLID 44469 - Also allow for an array of all loaded charges in case we having
	// a coding group and they may be changing the quantity.
	// (j.jones 2013-05-16 15:12) - PLID 56596 - converted to be a reference so we don't need to include a .h for it
	CEMNChargeArray &m_arypAllLoadedCharges;

	CEmrCodingGroup *m_pCodingGroup;
	CEmnCodingGroupInfo *m_pEmnCodingInfo;

	BOOL IsCodingGroup();
	void UpdateControls();
	void RefreshChargeList();
	void SaveChargeListToArray();

	// Generated message map functions
	//{{AFX_MSG(CEMRChargePromptDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnNoChanges();
	afx_msg void OnEditingFinishingAddChargeList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnEnChangeEmrChargePromptQuantity();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRCHARGEPROMPTDLG_H__8EE1A780_5317_4FA0_9146_353FAAB8C673__INCLUDED_)
