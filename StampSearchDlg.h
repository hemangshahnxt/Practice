#pragma once

// (a.walling 2012-08-28 08:15) - PLID 52321 - StampSearch - Host dialog and interaction with EMR

class CStampSearchCtrl;

class CStampSearchDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CStampSearchDlg)

public:
	CStampSearchDlg(CWnd* pParent);   // standard constructor
	virtual ~CStampSearchDlg();

	// (a.walling 2012-08-28 08:15) - PLID 52321 - Stamp was chosen, set the internal var and close the dialog
	void OnStampClicked(long nStampID);

// Dialog Data
	enum { IDD = IDD_BLANKDIALOG };

	long GetClickedStampID() const
	{ return m_nClickedStampID; }

protected:
	
	virtual BOOL OnInitDialog();

	virtual void OnCancel();

	long m_nClickedStampID;

	scoped_ptr<CStampSearchCtrl> m_pCtrl;

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()
};

