#pragma once

//TES 2/24/2012 - PLID 45127 - Created
// CSelectStampDlg dialog

class CSelectStampDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CSelectStampDlg)

public:
	CSelectStampDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSelectStampDlg();

	//TES 3/28/2012 - PLID 49294 - This dialog needs to know what image it's on, so it can filter the list of stamps accordingly
	long m_nImageEmrInfoMasterID;

	//TES 2/24/2012 - PLID 45127 - Get information about the stamp selected by the user.
	void GetSelectedStamp(long &nStampID, CString &strStampText, CString &strTypeName, COLORREF &clrStampColor);

// Dialog Data
	enum { IDD = IDD_SELECT_STAMP_DLG };

protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pStampList;
	CNxIconButton m_nxbOK, m_nxbCancel;

	long m_nSelectedStampID;
	CString m_strSelectedStampText, m_strSelectedTypeName;
	COLORREF m_clrSelectedColor;
	BOOL m_bSelectedShowDot;

	//TES 3/28/2012 - PLID 49294 - We now let them switch back and forth between filtering on the current image's stamps.
	bool m_bFilteringStamps;
	void RequeryStampList();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	void OnDblClickCellStampList(LPDISPATCH lpRow, short nColIndex);
	void OnRequeryFinishedStampList(short nFlags);
};
