#pragma once


// CQuickBookRepID dialog
// (a.vengrofski 2010-04-22 09:23) - PLID <38205> - File created
class CQuickBookRepID : public CNxDialog
{
	DECLARE_DYNAMIC(CQuickBookRepID)

public:
	CQuickBookRepID(CWnd* pParent);   // standard constructor
	virtual ~CQuickBookRepID();
	virtual BOOL OnInitDialog();
	NXDATALIST2Lib::_DNxDataListPtr m_pQBReps;


// Dialog Data
	enum { IDD = IDD_QUICKBOOK_REP_ID };

	CNxIconButton m_btnOK;

	enum EQBReps {
		qbrFirst = 0,
		qbrLast = 1 ,
		qbrQBID = 2,
		qbrPersonID = 3,
		qbrIsInMap = 4,
	};


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	void EditingFinishedQbrepsList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void EditingFinishingQbrepsList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	virtual void OnCancel();
};
