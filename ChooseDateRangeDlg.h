#pragma once


// (j.dinatale 2011-11-14 13:04) - PLID 45658 - created
// CChooseDateRangeDlg dialog

class CChooseDateRangeDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CChooseDateRangeDlg)

public:
	CChooseDateRangeDlg(CWnd* pParent);   // standard constructor
	virtual ~CChooseDateRangeDlg();
	virtual BOOL OnInitDialog();
	virtual int DoModal();

	COleDateTime GetFromDate();
	COleDateTime GetToDate();

	void SetMinDate(COleDateTime dtMin);
	void SetMaxDate(COleDateTime dtMax);
	// (c.haag 2014-12-17) - PLID 64253 - Lets the caller choose whether to show the cancel button
	// when the dialog is open.
	void SetCancelButtonVisible(BOOL bVisible);
	// (c.haag 2014-12-17) - PLID 64253 - Lets the caller choose between two different times of day
	// when the dialog is open.
	void SetChooseTimeRange(BOOL bChooseTimeRange);
	// (z.manning 2016-03-11 15:41) - PLID 68584
	void SetToDateOptional(BOOL bToDateOptional);

// Dialog Data
	enum { IDD = IDD_CHOOSE_DATE_RANGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel; // (c.haag 2014-12-17) - PLID 64253

	CDateTimePicker	m_dtpTo;
	CDateTimePicker	m_dtpFrom;
	NXTIMELib::_DNxTimePtr m_pTimeTo;
	NXTIMELib::_DNxTimePtr m_pTimeFrom;

	COleDateTime m_dtTo;
	COleDateTime m_dtFrom;

	COleDateTime m_dtMax;
	COleDateTime m_dtMin;

	BOOL m_bCancelButtonVisible; // (c.haag 2014-12-17) - PLID 64253
	BOOL m_bChooseTimeRange; // (c.haag 2014-12-17) - PLID 64253
	BOOL m_bToDateOptional; // (z.manning 2016-03-11 15:40) - PLID 68584

	// (z.manning 2016-03-11 16:04) - PLID 68584
	BOOL IsEndDateVisible();
	void UpdateControls();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedDateToCheck();
};
