#pragma once
// (s.dhole 2012-04-17 09:24) - PLID 49734  New dialog, to select frames from a Framdata

// CInvFrameSelection dialog

class CInvFrameSelection : public CNxDialog
{
	DECLARE_DYNAMIC(CInvFrameSelection)

public:
	CInvFrameSelection(CWnd* pParent = NULL);   // standard constructor
	virtual ~CInvFrameSelection();

// Dialog Data
	enum { IDD = IDD_INV_FRAME_SELECTION };
	long m_nFrameDataID;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	void ApplyFilter();
	NXDATALIST2Lib::_DNxDataListPtr m_pFrameList,m_pFrameManufacturer,m_pFrameBrand,
		m_pFrameStyle,m_pFrameFrameCollection;
	CNxIconButton m_nxbFrameOK, m_nxbFrameCancel,m_nxbFrameSearch;
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedSelectFrameOk();
	afx_msg void OnBnClickedSelectFrameCancel();
	afx_msg void OnBnClickedSelectFrameSerach();
	afx_msg void OnSelectStyle();
	afx_msg void OnSelectManufacturer();
	DECLARE_EVENTSINK_MAP()
	void OnRequeryFinishedFrameList(short nFlags);
	void SelChangingManufactuer(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void OnSelChosenManufactuer(LPDISPATCH lpRow);
	void SelChangingStyle(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void OnSelChosenStyle(LPDISPATCH lpRow);
	void SelChangingCollection(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void OnSelChosenCollection(LPDISPATCH lpRow);
	void SelChangingBrand(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void OnSelChosenBrand(LPDISPATCH lpRow);


};
