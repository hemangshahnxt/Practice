// (a.wilson 2013-01-08 17:50) - PLID 54512 - new dialog to allow for predefined reasons as well as free text renewal responses.

#pragma once
#include "PatientsRc.h"

namespace NexTech_Accessor { enum RenewalResponseStatus; }

// CRenewalResponseReasonDlg dialog

class CRenewalResponseReasonDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CRenewalResponseReasonDlg)

public:
	enum { IDD = IDD_RENEWAL_RESPONSE_REASON_DLG };

	//constructor, deconstructor
	CRenewalResponseReasonDlg(CWnd* pParent = NULL);   
	virtual ~CRenewalResponseReasonDlg();

	//variables
	CString m_strDetailedReason;
	CArray<_bstr_t> m_arReasonSelections;

	//functions
	int PromptForResponseReason(NexTech_Accessor::RenewalResponseStatus responseStatus);

protected:

	//Controls
	CNxEdit m_editDetailedReason;
	CNxIconButton m_btnOK, m_btnCancel;
	NxButton m_chkSaveDetailedReason;
	NXDATALIST2Lib::_DNxDataListPtr m_pResponseReasonList, m_pResponseReasonCustomCombo;

	//standard
	NexTech_Accessor::RenewalResponseStatus m_eResponseStatus;

	//messages, events, functions
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);
	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	afx_msg void OnBnClickedOk();
	void SelChosenRenewalResponseCustomDefaults(LPDISPATCH lpRow);
	void MoveControlUp(int nDistance, int nControlID);
};
