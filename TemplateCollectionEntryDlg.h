#pragma once

#include "NxAPI.h"
#include "CommonDialog.h"


// CTemplateCollectionEntryDlg dialog
// (z.manning 2014-12-04 13:19) - PLID 64228 - Created

class CTemplateLineItemInfo;

class CTemplateCollectionEntryDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CTemplateCollectionEntryDlg)

public:
	CTemplateCollectionEntryDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTemplateCollectionEntryDlg();

	int EditCollection(NexTech_Accessor::_SchedulerTemplateCollectionPtr pCollection);

// Dialog Data
	enum { IDD = IDD_TEMPLATE_COLLECTION_ENTRY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	CCommonDialog60	m_ctrlColorPicker;
	CNxIconButton m_btnClose;
	CNxIconButton m_btnNewApply;
	CNxIconButton m_btnEditApply;
	CNxIconButton m_btnDeleteApply;
	CNxIconButton m_btnRemoveAndReapply; // (z.manning 2016-03-07 10:52) - PLID 68443

	NXDATALIST2Lib::_DNxDataListPtr m_pdlApplies;
	enum ApplyListColumns {
		alcID = 0,
		alcResourceName,
		alcStartDate,
		alcEndDate,
		alcPeriod,
	};

	NexTech_Accessor::_SchedulerTemplateCollectionPtr m_pCollection;

	// (z.manning 2014-12-16 10:58) - PLID 64232 - Map to track the applies
	std::map<long, NexTech_Accessor::_SchedulerTemplateCollectionApplyPtr> m_mapApplies;

	void LoadApplies();
	// (z.manning 2016-03-11 11:25) - PLID 68443 - Added overload if you already have the apply objects
	void LoadApplies(NexTech_Accessor::_SchedulerTemplateCollectionAppliesPtr pApplies);

	void EnableControls(); // (c.haag 2014-12-16) - PLID 64240
	// (z.manning 2015-01-02 10:41) - PLID 64508
	void RefreshVisibleApplies();

	BOOL Validate();

	// (z.manning 2014-12-16 11:01) - PLID 64232
	// (z.manning 2016-03-04 08:59) - PLID 68443 - Plura now that we support multi-selection
	std::vector<NexTech_Accessor::_SchedulerTemplateCollectionApplyPtr> GetSelectedApplies();

	// (z.manning 2014-12-16 11:35) - PLID 64232
	void EditAndSaveApply(CTemplateLineItemInfo *pLineItem);

	// (z.manning 2014-12-16 17:01) - PLID 64232
	void EnsureRowForApply(NexTech_Accessor::_SchedulerTemplateCollectionApplyPtr pApply);

	// (z.manning 2014-12-19 11:40) - PLID 64239
	void CopyAndEditNewApply(const long nApplyIDToCopy);
	void CopyAndEditNewApply(NexTech_Accessor::_SchedulerTemplateCollectionApplyPtr pApplyToCopy);

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	afx_msg void OnBnClickedNameColorEntryChooseColor();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnBnClickedTemplateCollectionNewApply(); // (z.manning 2014-12-11 16:33) - PLID 64230
	afx_msg void OnBnClickedTemplateCollectionDeleteApply(); // (c.haag 2014-12-16) - PLID 64240
	afx_msg void OnClose();
	afx_msg void OnBnClickedTemplateCollectionEditApply();
	afx_msg void OnSelChangedCollectionApplyList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel); // (c.haag 2014-12-16) - PLID 64240
	afx_msg void DblClickCellApplyList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnRButtonUpApplyList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags); // (z.manning 2014-12-18 17:10) - PLID 64239
	afx_msg void OnBnClickedTemplateCollectionHideOutdated(); // (z.manning 2015-01-02 10:28) - PLID 64508
	void LeftClickTemplateCollectionApplyList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnBnClickedTemplateCollectionUpdateApply();
};
