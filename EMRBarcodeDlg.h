#pragma once


// CEMRBarcodeDlg dialog

// (j.dinatale 2011-07-26 17:41) - PLID 44702 - Created


class CEMRBarcodeDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEMRBarcodeDlg)

public:
	CEMRBarcodeDlg(CWnd* pParent);   // standard constructor
	virtual ~CEMRBarcodeDlg();

	long m_nEMRID;
	long m_nPersonID;

	void SetInfo(long nPersonID, long nEMRID);

// Dialog Data
	enum { IDD = IDD_EMR_BARCODE };

protected:
	CNxIconButton m_btnOK;
	CNxStatic m_nxsBarcode;
	CNxStatic m_nxsNumericCode;
	CNxLabel m_nxsInfoLink; // (j.dinatale 2011-09-12 17:52) - PLID 45369
	CFont m_fBarcodeFont;

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT message);
};
