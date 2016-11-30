// (r.gonet 09/21/2011) - PLID 45584 - Added

#pragma once

#include "AdministratorRc.h"
#include "LabCustomField.h"
#include "LabCustomFieldTemplate.h"
#include "LabCustomFieldControlManager.h"
#include "LabCustomFieldsView.h"

// CLabsSelCustomFieldsDlg dialog

// (r.gonet 09/21/2011) - PLID 45584 - CLabsSelCustomFieldsDlg allows both the editing of a lab custom fields template and the selection of one
//  for a certain lab procedure. It shows only the most up to date fields. No historical data and gives a nice
//  preview of the template instantiated.
class CLabsSelCustomFieldsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CLabsSelCustomFieldsDlg)
private:
	// Enums
	enum ELabCFTemplateColumns
	{
		lcftcID = 0,
		lcftcName,
	};

	enum EFieldsListColumns
	{
		ecflcID = 0,
		ecflcFieldID,
		ecflcSortOrder,
		ecflcName,
		ecflcDisplayName,
		ecflcRequired,
	};

	// Controls
	CNxStatic m_nxsTemplateHeader;
	NXDATALIST2Lib::_DNxDataListPtr m_pTemplateCombo;
	CNxIconButton m_nxbAddTemplate;
	CNxIconButton m_nxbDeleteTemplate;
	CNxIconButton m_nxbRenameTemplate;
	CNxStatic m_nxsFieldsHeader;
	CNxIconButton m_nxbAdd;
	CNxIconButton m_nxbEdit;
	CNxIconButton m_nxbRemove;
	CNxIconButton m_nxbUp;
	CNxIconButton m_nxbDown;
	NXDATALIST2Lib::_DNxDataListPtr m_pFieldsList;
	CNxStatic m_nxsPreviewHeader;
	CNxIconButton m_nxbClose;

	// Data Variables
	// (r.gonet 09/21/2011) - PLID 45584 - Lab Procedure we are selecting a template for.
	long m_nLabProcedureID;
	// (r.gonet 09/21/2011) - PLID 45584 - Template we are currently editing
	CLabProcCFTemplatePtr m_pTemplate;
	// (r.gonet 09/21/2011) - PLID 45584 - Template instance of the template currently being edited.
	CCFTemplateInstance *m_pTemplateInstance;
	// (r.gonet 09/21/2011) - PLID 45584 - Preview of the template fields.
	CLabCustomFieldsView *m_pdlgPreview;

	// Methods
	void EnsureControls();
	bool SaveTemplate();

public:
	CLabsSelCustomFieldsDlg(long nLabProcedureID, CWnd* pParent);   // standard constructor
	virtual ~CLabsSelCustomFieldsDlg();

// Dialog Data
	enum { IDD = IDD_LABS_SEL_CUSTOM_FIELDS_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	void RefreshPreview();
	afx_msg void OnBnClickedAddCustomFieldBtn();
	afx_msg void OnBnClickedEditCustomFieldBtn();
	afx_msg void OnBnClickedRemoveCustomFieldBtn();
	afx_msg void OnBnClickedOk();
	DECLARE_EVENTSINK_MAP()
	void EditingFinishedCustomFieldsList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	afx_msg void OnBnClickedFieldUpBtn();
	afx_msg void OnBnClickedFieldDownButton();
	void SelChangedCustomFieldsList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void SelChosenSelLabCfTemplateCombo(LPDISPATCH lpRow);
	afx_msg void OnBnClickedAddCfTemplateBtn();
	afx_msg void OnBnClickedDeleteCfTemplateBtn();
	afx_msg void OnBnClickedRenameCfTemplateBtn();
	void SetRefreshTimer();
	afx_msg void OnTimer(UINT nEventID);
	void OnDblClickCellCustomFieldsList(LPDISPATCH lpRow, short nColIndex);
	void EnsureFieldInView(NXDATALIST2Lib::IRowSettingsPtr pRow);
};
