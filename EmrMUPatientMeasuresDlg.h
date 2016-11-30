#pragma once
// (r.gonet 06/12/2013) - PLID 55151 - Removed a header include and added a forward declaration.
class CCHITReportInfoListing;

//(e.lally 2012-02-28) PLID 48265 - Created
// CEmrMUPatientMeasuresDlg dialog

class CEmrMUPatientMeasuresDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEmrMUPatientMeasuresDlg)

public:
	CCHITReportInfoListing* m_pCchitReportListing;
	CEmrMUPatientMeasuresDlg(CWnd* pParent = NULL, const CString& strWindowTitleOverride = "", CCHITReportInfoListing* pCchitReportListing = NULL);   // standard constructor
	virtual ~CEmrMUPatientMeasuresDlg();

// Dialog Data
	enum { IDD = IDD_EMR_MU_PATIENT_MEASURES_DLG };

protected:
	CNxIconButton m_btnClose;
	CString m_strWindowTitle;
	NXDATALIST2Lib::_DNxDataListPtr m_pMeasureList;
	HICON m_hIconGreenCheck, m_hIconRedX;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
};
