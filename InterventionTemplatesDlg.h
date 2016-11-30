#pragma once

#include "AdministratorRc.h"

// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS

//TES 5/19/2009 - PLID 34302 - Created
// CInterventionTemplatesDlg dialog

//Forward Declarations
namespace Intervention
{
	class IInterventionTemplate;
	class IInterventionConfigurationManager;
	typedef boost::shared_ptr<IInterventionTemplate> IInterventionTemplatePtr;
	typedef boost::shared_ptr<IInterventionConfigurationManager> IInterventionConfigurationManagerPtr;
}

class CInterventionTemplatesDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CInterventionTemplatesDlg)

public:
	CInterventionTemplatesDlg(CWnd* pParent);   // standard constructor
	virtual ~CInterventionTemplatesDlg();

	int OpenWellnessConfiguration();
	int OpenDecisionSupportConfiguration();

protected:

	Intervention::IInterventionConfigurationManagerPtr m_pConfigurationManager;
	virtual int DoModal();

//Controls
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pTemplateList, m_pEmrItemList, m_pCompletionItemList, 
		m_pAvailableCriteriaList, m_pCriteriaList;
	RICHTEXTEDITORLib::_DRichTextEditorPtr m_reGuidelines, m_reReferenceMaterials;

	//TES 5/26/2009 - PLID 34302 - Prevent editing the criteria list while it's requerying.
	bool m_bRequeryingCriteria;

	//TES 5/22/2009 - PLID 34302 - Loads all the data for the currently selected template.
	void Refresh();
	//TES 5/22/2009 - PLID 34302 - Ensures that the currently selected template is configured in a valid fashion.
	// Will warn the user and return false, in which case the user should be kept on this template until they fix it.
	bool ValidateCurrentTemplate();

// Dialog Data
	enum { IDD = IDD_WELLNESS_TEMPLATES_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
private:
	void SelectTemplateRow(NXDATALIST2Lib::IRowSettingsPtr pRow);

	void HideCompletionList();
	void HideDesisionExtraFields();
	void HideLastXDays();
	long GetTempletID();
	_variant_t GetVariantDate(COleDateTime Dt);

public:
	CNxIconButton m_nxbAddTemplate;
	CNxIconButton m_nxbDeleteTemplate;
	CNxIconButton m_nxbClose;
	CNxEdit m_nxeTemplateName;
	// (s.dhole 2013-10-31 16:35) - PLID 
	CNxEdit m_nxeDeveloper,m_nxeFundingSource,m_nxeCitiation,m_nxeRefrenceInfo ;
	NXTIMELib::_DNxTimePtr m_pReleaseDate,m_pRevisionDate;
	CNxLabel m_nxlGuidline, m_nxlReference; 
	CNxIconButton m_nxbMarkInactive, m_nxbViewInactive; //TES 11/27/2013 - PLID 59848

	afx_msg void OnAddInterventionTemplate();
	DECLARE_EVENTSINK_MAP()
	void OnSelChosenInterventionTemplateList(LPDISPATCH lpRow);
	afx_msg void OnDeleteInterventionTemplate();
	afx_msg void OnKillfocusInterventionTemplateName();
	void OnSelChosenEmrItemList(LPDISPATCH lpRow);
	void OnRButtonDownCompletionItems(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void OnSelChosenAvailableCriteriaList(LPDISPATCH lpRow);
	void OnRequeryFinishedTemplateCriteria(short nFlags);
	void OnEditingFinishingTemplateCriteria(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	void OnEditingFinishedTemplateCriteria(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void OnRButtonDownTemplateCriteria(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void OnSelChangingInterventionTemplateList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	afx_msg void OnOK();
	void OnEditingStartingTemplateCriteria(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
	void OnKillFocusGuidelines();
	void OnKillFocusReferenceMaterials();
	void OnLeftClickTemplateCriteria(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnCancel();
	afx_msg void OnEnKillfocusEditFunding();
	void KillFocusDtReleaseDate();
	void KillFocusDtRevisionDate();
	afx_msg void OnEnKillfocusEditDeveloper();
	afx_msg void OnEnKillfocusEditReferenceInfo();
	afx_msg void OnEnKillfocusEditCitiation();
	afx_msg void OnBnClickedInactivateCdsRule();
	afx_msg void OnBnClickedViewInactiveCdsRules();
};
