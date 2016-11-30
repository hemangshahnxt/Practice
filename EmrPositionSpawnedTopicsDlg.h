#pragma once

//TES 9/9/2009 - PLID 35495 - Created
// CEmrPositionSpawnedTopicsDlg dialog

class CEmrPositionSpawnedTopicsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEmrPositionSpawnedTopicsDlg)

public:
	CEmrPositionSpawnedTopicsDlg(CWnd* pParent);   // standard constructor
	virtual ~CEmrPositionSpawnedTopicsDlg();

	//TES 9/9/2009 - PLID 35495 - Because this loads topics much differently than other EMR code (that's the whole point),
	// we just need a template ID and we'll load the topics ourselves.  However, we do take the template name and date,
	// to save a database access.
	int Open(long nTemplateID, const CString &strTemplateName, const COleDateTime &dtTemplateDate);
// Dialog Data
	enum { IDD = IDD_EMR_POSITION_TOPICS_DLG };

protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pTree;
	CNxIconButton m_nxbOK;
	CNxIconButton m_nxbCancel;

	long m_nTemplateID;
	CString m_strTemplateName;
	COleDateTime m_dtTemplateDate;

	long m_nIconSize;

	void EnsureRowIcon(NXDATALIST2Lib::IRowSettingsPtr pRow, long nIconSize);

	//TES 9/10/2009 - PLID 35495 - Used when loading the topics
	struct TopicInfo {
		_variant_t varID;
		_variant_t varName;
		_variant_t varParentID;
		_variant_t varSourceActionID;
		_variant_t varSourceName;	//TES 9/18/2009 - PLID 35590
	};
	void AddChildRows(NXDATALIST2Lib::IRowSettingsPtr pRow, CArray<TopicInfo,TopicInfo&> *parTopics);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	//TES 9/10/2009 - PLID 35495 - Various utilities for dragging.
	LPDISPATCH m_lpDraggingRow;
	CArray<NXDATALIST2Lib::IRowSettings*,NXDATALIST2Lib::IRowSettings*> m_arDragPlaceholders;
	BOOL IsValidDrag(NXDATALIST2Lib::IRowSettings *pFromRow, NXDATALIST2Lib::IRowSettings *pToRow);
	void ClearDragPlaceholders(NXDATALIST2Lib::IRowSettings *pRowToPreserve = NULL);
	NXDATALIST2Lib::IRowSettingsPtr InsertPlaceholder(NXDATALIST2Lib::IRowSettings *pParentRow);

	//TES 9/10/2009 - PLID 35495 - Recursive function used to generate SQL batch in OnOK()
	void SaveChildRows(NXDATALIST2Lib::IRowSettingsPtr pParentRow, CString &strSqlBatch);

	//TES 9/18/2009 - PLID 35590 - Goes through all children of the given row, finds any topics that have duplicated
	// names, and renames them using their SourceName (if possible).
	void CheckDuplicateTopics(NXDATALIST2Lib::IRowSettingsPtr pParentRow);

	DECLARE_MESSAGE_MAP()
	//{{AFX_MSG(CEmrPositionSpawnedTopicsDlg)
	virtual BOOL OnInitDialog();
public:
	DECLARE_EVENTSINK_MAP()
	void OnDragBeginPositionTopicsTree(BOOL* pbShowDrag, LPDISPATCH lpRow, short nCol, long nFlags);
	void OnDragOverCellPositionTopicsTree(BOOL* pbShowDrop, LPDISPATCH lpRow, short nCol, LPDISPATCH lpFromRow, short nFromCol, long nFlags);
	void OnSelChangingPositionTopicsTree(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void OnDragEndPositionTopicsTree(LPDISPATCH lpRow, short nCol, LPDISPATCH lpFromRow, short nFromCol, long nFlags);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnOK();
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
