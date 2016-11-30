#pragma once

//TES 11/1/2013 - PLID 59276 - Created

//Forward Declarations
namespace Intervention
{
	class IInterventionTemplate;
	class IInterventionConfigurationManager;
	typedef boost::shared_ptr<IInterventionTemplate> IInterventionTemplatePtr;
	typedef boost::shared_ptr<IInterventionConfigurationManager> IInterventionConfigurationManagerPtr;
}

// CCDSInterventionDlg dialog

class CCDSInterventionDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CCDSInterventionDlg)

public:
	CCDSInterventionDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCDSInterventionDlg();

	//Set by caller
	long m_nInterventionID;

// Dialog Data
	enum { IDD = IDD_CDS_INTERVENTION_DLG };

protected:

	NXDATALIST2Lib::_DNxDataListPtr m_pCriteriaList;
	NXTIMELib::_DNxTimePtr m_nxtInputDate;
	CNxColor m_bkg1, m_bkg2, m_bkg3;
	CNxIconButton m_nxbOK, m_nxbCancel;
	CNxIconButton m_nxbGoToCitation; //TES 12/5/2013 - PLID 59276

	BOOL m_bSavedAck;
	long m_nSavedAckBy;
	CString m_strSavedAckByName;
	COleDateTime m_dtSavedAckDate;
	CString m_strSavedAckNote;

	BOOL m_bAcknowledged;
	long m_nAckBy;
	COleDateTime m_dtAckDate;

	//Used to load information about the CDS template
	Intervention::IInterventionConfigurationManagerPtr m_pConfigurationManager;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnAcknowledgeCdsIntervention();
	DECLARE_EVENTSINK_MAP()
	void RequeryFinishedCdsInterventionCriteriaList(short nFlags);
	afx_msg void OnBnClickedGoToCitation();
};
