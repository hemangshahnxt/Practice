#include "AdministratorRc.h"

#pragma once
// (a.wilson 2014-01-13 15:53) - PLID 59956 - Created.
// CCPTAdditionalInfoDlg dialog

enum CCDAProcedureType
{
    cptInvalidType = -1,
    cptProcedure = 1,
    cptObservation = 2,
    cptAct = 3,
    cptEncounter = 4,
};

class CCPTAdditionalInfoDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CCPTAdditionalInfoDlg)

public:
	CCPTAdditionalInfoDlg(CWnd* pParent = NULL);
	virtual ~CCPTAdditionalInfoDlg();
	enum { IDD = IDD_CPT_ADDITIONAL_INFO };

	CString m_strTOS;
	CCDAProcedureType m_eCCDAType;

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);

	CNxIconButton m_nxClose;
	CNxEdit m_editTOS;
	CNxColor m_nxBackground;
	NXDATALIST2Lib::_DNxDataListPtr m_pCCDAType;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedCancel();
	DECLARE_EVENTSINK_MAP()
	void SelChangingCptCcdatype(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
};

