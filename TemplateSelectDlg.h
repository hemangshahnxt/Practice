#if !defined(AFX_TEMPLATENEWLINEITEMDLG_H__58AB117E_BA2A_45BC_A8D9_5103CA229B73__INCLUDED_)
#define AFX_TEMPLATENEWLINEITEMDLG_H__58AB117E_BA2A_45BC_A8D9_5103CA229B73__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TemplateNewLineItemDlg.h : header file
//

#include "CommonDialog.h"

enum ETemplateListColumns
{
	tlcID = 0,
	tlcColor = 1,
	tlcColorPreview = 2,
	tlcName = 3,
};

/////////////////////////////////////////////////////////////////////////////
// CTemplateSelectDlg dialog

// (z.manning, 11/14/2006) - PLID 23443 - Dialog to select a template when creating a new line item
// outside the template entry dialog.
class CTemplateSelectDlg : public CNxDialog
{
// Construction
public:
	// (z.manning 2014-12-05 13:40) - PLID 64219 - Added dialog type param
	CTemplateSelectDlg(ESchedulerTemplateEditorType eType, CWnd* pParent);   // standard constructor

	// (z.manning 2014-12-05 14:08) - PLID 64219
	ESchedulerTemplateEditorType m_eEditorType;

	// (z.manning 2014-12-17 15:23) - PLID 64427 - Made this public
	long m_nSelectedTemplateID;

	// (z.manning 2014-12-05 16:12) - PLID 64219
	COleDateTime m_dtStartTime;
	COleDateTime m_dtEndTime;
	BOOL m_bIsBlock;

	// (z.manning, 04/29/2008) - PLID 29814 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CTemplateSelectDlg)
	enum { IDD = IDD_TEMPLATE_SELECT };
	NxButton	m_btnNewTemplate;
	NxButton	m_btnSelectExisting;
	CCommonDialog60	m_ctrlColorPicker;
	CNxEdit	m_nxeditNewTemplateName;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTemplateSelectDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pdlTemplates;
	//TES 6/19/2010 - PLID 5888
	NXDATALIST2Lib::_DNxDataListPtr m_pLocationCombo;

	// (z.manning 2014-12-08 10:06) - PLID 64219
	NXTIMELib::_DNxTimePtr m_nxtStart;
	NXTIMELib::_DNxTimePtr m_nxtEnd;

	long m_nColor;

	// Generated message map functions
	//{{AFX_MSG(CTemplateSelectDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelectExistingTemplate();
	afx_msg void OnNewTemplate();
	virtual void OnOK();
	afx_msg void OnNewTemplateChooseColor();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnRequeryFinishedSelectExistingTemplate(short nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEMPLATENEWLINEITEMDLG_H__58AB117E_BA2A_45BC_A8D9_5103CA229B73__INCLUDED_)
