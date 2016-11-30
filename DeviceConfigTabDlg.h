#pragma once
#include "DevicePlugin.h"

// (r.gonet 06/11/2013) - PLID 56370 - Eliminated some magic numbers.
#define IMAGE_DPI_AUTO -2L
#define IMAGE_DPI_NULL -1L

// (d.lange 2010-05-07 15:22) - PLID 38536 - created
// CDeviceConfigTabDlg dialog
enum ActivePluginColumn {
	apcPluginID = 0,
	apcLaunchSetting,
	apcPluginFileName,
	apcEnabled,
	apcPluginName,
	apcDeviceExportPath,
	apcDeviceExePath,
	apcDeviceAddlPath,
	// (j.jones 2011-03-10 14:10) - PLID 41349 - added NotificationRule
	apcNotificationRule,
	// (j.jones 2010-11-02 14:19) - PLID 41189 - added DevicePatientMatchRule
	apcDevicePatientMatchRule,
	// (j.jones 2010-12-28 14:51) - PLID 41796 - added ConvertPDFToImage
	apcConvertPDFToImage,
	// (b.savon 2014-12-03 14:15) - PLID 64184 - Add pdf converter column
	apcPDFToImageConverter,
	// (r.gonet 06/11/2013) - PLID 57127 - added ImageDPI
	apcImageDPI,
	// (b.savon 2013-05-23 15:43) - PLID 42902 - added DefaultCategory
	apcDefaultCategory,
};

enum DetectedPluginColumn {
	dpcID = 0,
	dpcLaunchSetting,
	dpcPluginFileName,
	dpcPluginName,
	// (b.savon 2013-05-23 16:58) - PLID 42902
	dpcDefaultCategoryID,
};

// (j.jones 2011-03-10 14:10) - PLID 41349 - added NotificationRule
// these are stored in data, so do not change their values
enum EDeviceNotificationRule {
	dpnrDoNothing = 0,		//do nothing when a new file is found
	dpnrAutoPopup = 1,		//auto-popup the device import when a new file is found
	dpnrNotifyUser = 2,		// (j.jones 2011-03-15 14:33) - PLID 42738 - give a popup notification
};

// (b.savon 2012-02-02 13:15) - PLID 47930 - added dpmrMatchByDemographics
// (j.jones 2010-11-02 14:19) - PLID 41189 - added enum for DevicePatientMatchRule values,
// these are stored in data, so do not change their values
enum EDevicePatientMatchRule {
	dpmrMatchByIDOnly = 0,				//match by UserDefinedID and nothing else
	dpmrMatchByIDOrDemographics = 1,	//match by UserDefinedID or patient demographics
	dpmrMatchByDemographics = 2,		//match by patient demographics and nothing else
};

// (j.jones 2010-12-28 14:51) - PLID 41796 - added enum for ConvertPDFToImage values,
// these are stored in data, do not change them!
enum EConvertPDFToImage {
	cptiDoNotConvert = 0,
	cptiConvertToImage = 1,
};

// (b.savon 2014-12-03 10:41) - PLID 64184 - added enum for the PDF to Image converter
enum EPDFToImageConverter{
	pticTwoPilots = 0,
	pticBCL = 1,
};

class CDeviceConfigTabDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CDeviceConfigTabDlg)

public:
	CDeviceConfigTabDlg(CWnd* pParent);   // standard constructor
	virtual ~CDeviceConfigTabDlg();

// Dialog Data
	enum { IDD = IDD_DEVICES_CONFIG_TAB_DLG };
	

	CNxIconButton m_btnDelete;

protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pActivePluginsList;
	NXDATALIST2Lib::_DNxDataListPtr m_pDetectedPluginsList;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	void BuildDevicePluginsCombo();

	// (b.savon 2011-9-13) - PLID 45441 - Device plugins need to have unique names per machine.
	BOOL IsValidPluginName( CString strPluginName, BOOL bConfigTab = FALSE );
	CString m_strPluginNameOriginal;
	CString m_strDeviceExportPathOriginal;
	CString m_strDeviceExePathOriginal;
	CString m_strDeviceAddlPathOriginal;
	BOOL bRetry;

	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	afx_msg void SelChosenDetectedDevicesCombo(LPDISPATCH lpRow);
	afx_msg void OnBnClickedBtnDelete();
	afx_msg void LeftClickActiveDevicePlugins(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void SelSetActiveDevicePlugins(LPDISPATCH lpSel);
	afx_msg void EditingFinishedActiveDevicePlugins(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	afx_msg void RequeryFinishedActiveDevicePlugins(short nFlags);
	void EditingStartingActiveDevicePlugins(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
	void EditingStartedActiveDevicePlugins(LPDISPATCH lpRow, short nCol, long nEditType);
};
