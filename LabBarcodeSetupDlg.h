// (r.gonet 10/17/2011) - PLID 45924 - Added

#pragma once

#include "PracticeRc.h"
#include "LabBarcode.h"

enum EBarcodePartListColumns {
		bcplcID = 0,
		bcplcPath,
		bcplcDescription,
		bcplcMaxLength,
		bcplcFillType,
		bcplcTextValue,
		bcplcLabCustomFieldID,
		bcplcModified,
		bcplcLabProcedureID, //TES 7/24/2012 - PLID 50393
	};

//TES 7/24/2012 - PLID 50393
enum LabProcedureListColumns {
	lplcID = 0,
	lplcName = 1,
};

// CLabBarcodeSetupDlg dialog

class CLabBarcodeSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CLabBarcodeSetupDlg)

	NXDATALIST2Lib::_DNxDataListPtr m_pPartsList;
	CNxStatic m_nxsName;
	CNxEdit m_nxeName;
	CNxStatic m_nxsDescription;
	CNxEdit m_nxeDescription;
	CNxStatic m_nxsTypeLabel;
	NxButton m_radioAutomatic;
	NxButton m_radioText;
	NxButton m_radioCustomField;
	CNxStatic m_nxsAutomaticLabel;
	CNxEdit m_nxeText;
	NXDATALIST2Lib::_DNxDataListPtr m_pCustomFieldList;
	CNxIconButton m_nxbOK;
	CNxIconButton m_nxbCancel;
	// (r.gonet 11/12/2011) - PLID 44212 - CLabBarcode is now abstract and must be a pointer to an implementation.
	CLabBarcode *m_plbBarcode;
	NXDATALIST2Lib::_DNxDataListPtr m_pLabProcedureList; //TES 7/24/2012 - PLID 50393
public:
	CLabBarcodeSetupDlg(CWnd* pParent);   // standard constructor
	virtual ~CLabBarcodeSetupDlg();

// Dialog Data
	enum { IDD = IDD_LAB_BARCODE_SETUP_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	void EnsureControls();
	void SaveBarcodeFieldToRow(NXDATALIST2Lib::IRowSettingsPtr pRow);
	void Save();
	void Load();
	

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	void AddPart(CLabBarcodePartPtr pPart);
	DECLARE_EVENTSINK_MAP()
	void SelChangedBarcodePartsList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnBnClickedOk();
};
