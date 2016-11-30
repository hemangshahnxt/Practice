#pragma once

// CEmrProblemNewActionDlg dialog
// (c.haag 2014-07-22) - PLID 62789 - Initial implementation

class CEmrProblemNewActionDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEmrProblemNewActionDlg)

public:
	CEmrProblemNewActionDlg(EmrActionObject SourceType,
		EmrActionObject DestType, CWnd* pParent = NULL);   // standard constructor
	virtual ~CEmrProblemNewActionDlg();

// Dialog Data
	enum { IDD = IDD_EMR_PROBLEM_NEW_ACTION };

// Outputs
public:
	_variant_t m_selStatus;
	_variant_t m_selDescription;
	_variant_t m_selAssocWith;
	_variant_t m_selSNOMEDCode;
	_variant_t m_selDoNotShowOnCCDA;// (s.tullis 2015-02-24 11:31) - PLID 64724 
	// (r.gonet 2015-03-17 10:33) - PLID 65013 - Store the selected DoNotShowOnProblemPrompt value, which is VT_BOOL
	_variant_t m_selDoNotShowOnProblemPrompt;
// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_dlStatusCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_dlSNOMEDList;
	NxButton m_radioLink1;
	NxButton m_radioLink2;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxStatic m_staticLinkTo;
	CNxEdit	m_editProblemActionDescription;
	// (s.tullis 2015-02-24 11:31) - PLID 64724 
	NxButton m_checkDoNotShowOnCCDA;
	// (r.gonet 2015-03-17 10:33) - PLID 65013 - "Do not show on problem prompt" checkbox.
	NxButton m_checkDoNotShowOnProblemPrompt;
protected:
	EmrActionObject m_SourceType; // The type of object spawning the action
	EmrActionObject m_DestType; // The type of object being spawned

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	DECLARE_EVENTSINK_MAP()
	void SelChangingComboActionProblemStatus(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChangingListProblemSnomed(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	afx_msg void OnBnClickedBtnEditSnomedList();
};
