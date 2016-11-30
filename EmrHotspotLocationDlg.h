#pragma once

//TES 2/9/2010 - PLID 37223 - Created
// CEmrHotspotLocationDlg dialog

class CEMRHotSpot;

class CEmrHotspotLocationDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEmrHotspotLocationDlg)

public:
	CEmrHotspotLocationDlg(CWnd* pParent);   // standard constructor
	virtual ~CEmrHotspotLocationDlg();

	void SetSpot(CEMRHotSpot *pSpot) {m_pSpot = pSpot;}
	CEMRHotSpot* GetSpot() {return m_pSpot;}

protected:
	//TES 2/9/2010 - PLID 37223 - The spot whose location we're modifying.
	CEMRHotSpot *m_pSpot;

	//TES 2/10/2010 - PLID 37223 - Used when editing the lists.
	long m_nCurrentLocationID, m_nCurrentQualifierID;

	CNxIconButton m_nxbOK, m_nxbCancel, m_nxbModifyLocations, m_nxbModifyQualifiers;
	NXDATALIST2Lib::_DNxDataListPtr m_pLocations, m_pQualifiers;

// Dialog Data
	enum { IDD = IDD_EMR_HOTSPOT_LOCATION_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnOK();
protected:
	NxButton m_checkLeft;
	NxButton m_checkRight;
public:
	DECLARE_EVENTSINK_MAP()
	void OnRequeryFinishedHotspotLocation(short nFlags);
	void OnTrySetSelFinishedHotspotLocation(long nRowEnum, long nFlags);
	void OnRequeryFinishedHotspotQualifier(short nFlags);
	void OnTrySetSelFinishedHotspotQualifier(long nRowEnum, long nFlags);
	afx_msg void OnHotspotLeftSide();
	afx_msg void OnHotspotRightSide();
	afx_msg void OnModifyLocations();
	afx_msg void OnModifyQualifiers();
};
