#pragma once


// COrderSetTemplateLabSetupDlg dialog

class COrderSetTemplateLabSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(COrderSetTemplateLabSetupDlg)

public:
	COrderSetTemplateLabSetupDlg(CWnd* pParent);   // standard constructor

	CString* m_pstrToBeOrderedText;

// Dialog Data
	enum { IDD = IDD_ORDER_SET_TEMPLATE_LAB_SETUP };

	DECLARE_EVENTSINK_MAP()
	void OnSelChosenOrdersetTmplLabToBeOrderedList(LPDISPATCH lpRow);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	NXDATALIST2Lib::_DNxDataListPtr m_pdlToBeOrderedMaster;

	CNxColor m_nxcolor;
	//CNxStatic m_nxstaticLabs;
	CNxEdit m_nxeditToBeOrderedText;
	CNxIconButton m_btnOk;
	CNxIconButton m_btnCancel;

	DECLARE_MESSAGE_MAP()
};
