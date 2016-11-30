#pragma once
#include "EmrPane.h"


class CEMRProgressPane : public CEmrPane
{
	DECLARE_DYNCREATE(CEMRProgressPane)
public:
	CEMRProgressPane();
	~CEMRProgressPane();

	void SetStatusText(const CString& strNewText);
	void SetProgressBar(const long& nCompleted, const long& nTotal);
	

protected:
	BOOL m_bIsEmrTemplate;
	CFont *m_pButtonFont;//(e.lally 2012-03-26) PLID 48264

	//Controls
	CProgressCtrl m_progressBar;
	CStatic m_progressStatusLabel;
	CNxIconButton m_btnShowDetails; //(e.lally 2012-03-26) PLID 48016
	CNxIconButton m_btnConfigure; //(e.lally 2012-03-26) PLID 48264

	virtual void CreateControls();
	virtual CRect GetControlRect(UINT nID);
	virtual void RepositionControls();

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC)
	{
		try {
			CRect rcClient;
			GetClientRect(&rcClient);
			// (a.walling 2012-02-24 09:38) - PLID 48386 - Get the color from CNexTechDialog
			pDC->FillSolidRect(rcClient, CNexTechDialog::GetSolidBackgroundRGBColor());
		} NxCatchAll(__FUNCTION__);
		return TRUE;
	}

	afx_msg void OnSize(UINT nType, int cx, int cy)
	{
		try {
			CDockablePane::OnSize(nType, cx, cy);
			RepositionControls();
		} NxCatchAll(__FUNCTION__);
	}

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	////
	/// UI State overrides
	virtual afx_msg void OnUpdateStatusLabel(CCmdUI* pCmdUI);
	virtual afx_msg void OnUpdateProgressBar(CCmdUI* pCmdUI);
	virtual afx_msg void OnUpdateShowDetails(CCmdUI* pCmdUI); //(e.lally 2012-03-26) PLID 48016
	virtual afx_msg void OnUpdateConfigure(CCmdUI* pCmdUI); //(e.lally 2012-03-26) PLID 48264


	////
	/// UI Command overrides


	DECLARE_MESSAGE_MAP()

};
