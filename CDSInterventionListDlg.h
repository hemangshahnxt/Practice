#pragma once


// CCDSInterventionListDlg dialog
//TES 11/1/2013 - PLID 59276 - Created

class CCDSInterventionListDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CCDSInterventionListDlg)

public:
	CCDSInterventionListDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCDSInterventionListDlg();

	//Two ways to open: with a list of specific IDs, or with a patient's ID
	void OpenWithNewInterventions(const CDWordArray &arInterventions);
	void OpenWithPatientInterventions(long nPatientID, const CString &strPatientName);

// Dialog Data
	enum { IDD = IDD_CDS_INTERVENTION_LIST_DLG };

protected:
	CDWordArray m_arInterventionIDs;
	long m_nPatientID;
	CString m_strPatientName;

	CNxIconButton m_nxbClose;
	NXDATALIST2Lib::_DNxDataListPtr m_pInterventionList;
	CNxColor m_bkg;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	void LeftClickCdsInterventionList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnBnClickedIncludeAcknowledged();
};
