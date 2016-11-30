#pragma once

// (j.jones 2016-02-22 13:48) - PLID 68354 - created

// CErxPrescriptionReviewDlg dialog

class CErxPrescriptionReviewDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CErxPrescriptionReviewDlg)

public:
	CErxPrescriptionReviewDlg(CWnd* pParent, OLE_COLOR nColor);   // standard constructor
	virtual ~CErxPrescriptionReviewDlg();

	//the array of scripts to review
	std::vector<long> m_aryPrescriptionIDs;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ERX_PRESCRIPTION_REVIEW_DLG };
#endif

protected:
	CNxColor m_bkg;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxStatic m_nxstaticPatientNameLabel;
	CNxStatic m_nxstaticPatientNameField;
	CNxStatic m_nxstaticBirthdateLabel;
	CNxStatic m_nxstaticBirthdateField;
	CNxStatic m_nxstaticGenderLabel;
	CNxStatic m_nxstaticGenderField;

	NXDATALIST2Lib::_DNxDataListPtr m_ReviewList;

	OLE_COLOR m_nColor;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	void SetColor(OLE_COLOR nNewColor);
};
