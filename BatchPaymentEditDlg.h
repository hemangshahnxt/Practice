#if !defined(AFX_BATCHPAYMENTEDITDLG_H__AC02292A_6F24_4D0A_B50F_A7FFCE05F355__INCLUDED_)
#define AFX_BATCHPAYMENTEDITDLG_H__AC02292A_6F24_4D0A_B50F_A7FFCE05F355__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BatchPaymentEditDlg.h : header file
//

#include "Color.h"

/////////////////////////////////////////////////////////////////////////////
// CBatchPaymentEditDlg dialog

enum EBatchPaymentPayType;

class CBatchPaymentEditDlg : public CNxDialog
{
// Construction
public:
	
	void Load();
	BOOL Save();

	long m_ID;
	// (b.savon 2012-06-13 15:53) - PLID 49879 - Flag for batch date changed
	BOOL m_bBatchDateChanged;
	CString m_strBatchDate;

	long m_nType;
	// (j.jones 2014-06-26 17:23) - PLID 62546 - added PayType
	EBatchPaymentPayType m_ePayType;

	long m_nAppliedPayID;
	COleCurrency m_cyMaxApplyAmt;

	NXDATALISTLib::_DNxDataListPtr m_InsuranceCoCombo,
					m_DescriptionCombo,
					m_LocationCombo,
					m_PayCatsCombo,
					m_DoctorCombo;

	CColor m_color;

	CBatchPaymentEditDlg(CWnd* pParent);   // standard constructor

	// (j.jones 2008-05-07 11:07) - PLID 29854 - added nxiconbuttons for modernization
	// (j.jones 2008-09-08 13:31) - PLID 26689 - added m_nxstaticPayCat
// Dialog Data
	//{{AFX_DATA(CBatchPaymentEditDlg)
	enum { IDD = IDD_BATCH_PAYMENT_EDIT_DLG };
	CNxStatic	m_nxstaticPayCat;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CDateTimePicker	m_dtPaymentDate;
	CNxColor	m_bkg;
	CNxEdit	m_nxeditEditTotal;
	CNxEdit	m_nxeditEditDescription;
	CNxEdit	m_nxeditBankName;
	CNxEdit	m_nxeditBankNo;
	CNxEdit	m_nxeditCheckNo;
	CNxEdit	m_nxeditCheckAcctNo;
	CNxStatic	m_nxstaticCurrencySymbolBatchpay;
	// (j.jones 2009-06-26 12:07) - PLID 33856 - added controls for Original batch payment amount
	CNxStatic	m_nxstaticOriginalCurrencySymbolBatchpay;
	CNxStatic	m_nxstaticOriginalAmountLabel;
	CNxEdit		m_nxeditOriginalAmount;
	// (j.jones 2015-10-08 10:25) - PLID 67307 - added capitation payment checkbox & date range
	NxButton	m_checkCapitation;
	CDateTimePicker	m_dtServiceDateFrom;
	CDateTimePicker	m_dtServiceDateTo;
	CNxStatic	m_nxstaticLabelServiceDate;
	CNxStatic	m_nxstaticLabelServiceDateTo;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBatchPaymentEditDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CBrush m_brush;

	// (j.jones 2011-06-24 13:03) - PLID 41863 - added m_bIsInUse,
	// so we know this batch payment is partially applied, and can
	// have extra validation upon saving
	BOOL m_bIsInUse;

	// (j.jones 2008-07-11 13:45) - PLID 28756 - this function will check
	// which insurance company is selected, and override the 
	// payment description and payment category 
	void TrySetDefaultInsuranceDescriptions();

	// (j.jones 2015-10-08 13:11) - PLID 67307 - renamed this function and removed its parameter, it only disables items
	void DisableItems_BatchPayInUse();

	// Generated message map functions
	//{{AFX_MSG(CBatchPaymentEditDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelChosenDescriptionCombo(long nRow);
	afx_msg void OnEditPayCat();
	afx_msg void OnEditPayDesc();
	afx_msg void OnTrySetSelFinishedLocations(long nRowEnum, long nFlags);
	afx_msg void OnTrySetSelFinishedProviderCombo(long nRowEnum, long nFlags);
	afx_msg void OnSelChosenInsuranceCompanies(long nRow);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// (j.jones 2015-10-08 10:25) - PLID 67307 - added capitation payment checkbox
	afx_msg void OnCheckCapitation();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BATCHPAYMENTEDITDLG_H__AC02292A_6F24_4D0A_B50F_A7FFCE05F355__INCLUDED_)
