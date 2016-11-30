#pragma once

#include "EmrPane.h"

//(e.lally 2012-02-16) PLID 48065 - Created
class CEMRClassicButtonPane : public CEmrPane
{
	DECLARE_DYNCREATE(CEMRClassicButtonPane)
public:
	CEMRClassicButtonPane();
	~CEMRClassicButtonPane();

	void UpdateEMRProblemButtonIcon();
	

protected:
	BOOL m_bIsTemplate;
	CFont *m_pButtonFont;

	//BUTTONS
	//(e.lally 2012-02-16) PLID 48065 - Moved all these buttons from EmrTreeWnd
	CNxIconButton m_btnNewEmn; //The New EMN button
	CNxIconButton m_btnNextNode; //The Next button (>>).
	CNxIconButton m_btnPreviousNode; //The Previous button (<<).
	CButton m_btnEditMode; //The Edit Mode button.
	CNxIconButton m_btnAddTopics; //The Add Topics/SubTopics button.
	CNxIconButton m_btnAddItemToTopic; //The Add Item To Topic button.
	CNxIconButton m_btnSaveChanges; //The Save Changes button
	CNxIconButton m_btnSaveAndClose; // (z.manning 2009-08-12 12:41) - PLID 27694 - Added save and close button
	CNxIconButton m_btnCancel;		//(e.lally 2012-04-03) PLID 48065 - Cancel
	CNxIconButton m_btnCreateToDo; //The Create ToDo Task button
	CNxIconButton m_btnRecordAudio; //The Record Audio button
	CNxIconButton m_btnEMChecklist; // The "E/M Checklist" button
	CNxIconButton m_btnPatientSummary; // (j.jones 2008-07-09 08:57) - PLID 24624 - added patient summary
	CNxIconButton m_btnEMRProblemList;	// (j.jones 2008-07-17 13:03) - PLID 30729 - added problem list
	CNxIconButton m_btnEPrescribing; // (j.jones 2009-03-03 12:08) - PLID 33308 - added E-Prescribing
	CNxIconButton m_btnPositionTopics;	//TES 9/9/2009 - PLID 35495 - Added a button to position spawned template topics
#ifdef _DEBUG
	CNxIconButton m_btnDebug; // (c.haag 2007-08-04 10:03) - PLID 26946 - Used for EMR debugging
#endif

	void CreateControls();
	CRect GetControlRect(UINT nID);
	void RepositionControls();

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg BOOL OnEraseBkgnd(CDC* pDC)
	{
		CRect rcClient;
		GetClientRect(&rcClient);
		// (a.walling 2012-02-24 09:38) - PLID 48386 - Get the color from CNexTechDialog
		pDC->FillSolidRect(rcClient, CNexTechDialog::GetSolidBackgroundRGBColor());
		return TRUE;
	}

	afx_msg void OnSize(UINT nType, int cx, int cy)
	{
		CDockablePane::OnSize(nType, cx, cy);
		RepositionControls();
	}

	////
	/// UI State overrides

	afx_msg void OnUpdateClassicBtnEditMode(CCmdUI* pCmdUI);


	////
	/// UI Command overrides

	afx_msg void OnAddTopicsBtnMenu();
	afx_msg void OnProblemListBtnMenu();


	DECLARE_EVENTSINK_MAP()
	DECLARE_MESSAGE_MAP()
};
