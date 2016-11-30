#pragma once
#define VISIONWEBSERVICE_USER_ID		_T("VisionWebUserID")
#define VISIONWEBSERVICE_USER_PASSWORD		_T("VisionWebPassword")
#define VISIONWEBSERVICE_USER_ACCOUNT_URL		_T("VisionWebUserAccountUrl")
#define VISIONWEBSERVICE_CHANGE_PASSWORD_URL		_T("VisionWebChangePasswordUrl")
#define VISIONWEBSERVICE_ORDER_URL		_T("VisionWebOrderUrl")
#define VISIONWEBSERVICE_ORDER_ERROR_DETAIL_URL		_T("VisionWebOrderErrorDetailUrl")
#define VISIONWEBSERVICE_ORDER_STATUS_URL		_T("VisionWebOrderStatusUrl")
#define VISIONWEBSERVICE_CATALOG_URL		_T("VisionWebCatalogUrl")
#define VISIONWEBSERVICE_REFID		_T("VisionWebRefID")
#define VISIONWEBSERVICE_ERROR_URL		_T("VisionWebErrorUrl")
#define VISIONWEBSERVICE_SUPPLIER_URL  _T("VisionWebSupplierUrl")

// (s.dhole 2011-01-26 18:11) - PLID 41281 New Dialog
#include "InventoryRc.h"
// CVisionWebServiceSetupDlg dialog

class CVisionWebServiceSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CVisionWebServiceSetupDlg)

public:
	CVisionWebServiceSetupDlg(CWnd* pParent);   // standard constructor
	virtual ~CVisionWebServiceSetupDlg();
	CNxIconButton	m_SaveSetupBtn;
	CNxIconButton	m_CancelBtn;

// Dialog Data
	enum { IDD = IDD_VISIONWEB_SERVICE_SETUP };
	afx_msg void OnBnClickedOk();

protected:
	enum VisionWebServiceUrlColumns {
		vwURLType = 0,
		vwUrl,
	};


	NXDATALIST2Lib::_DNxDataListPtr m_VisionWebSetupUrlList;
	void AddRowToList(LPCTSTR urlType);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void Save();
	BOOL OnInitDialog();

	afx_msg void OnBtnClickedSave();
	afx_msg void OnBtnClickedCancel();
	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	void EditingFinishingVisionwebServiceUrlList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
};
