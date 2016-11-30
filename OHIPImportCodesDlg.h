#pragma once
#include "afxwin.h"

// (a.walling 2008-12-10 14:45) - PLID 32355 - Merged the OHIP Code Importer into Practice

// COHIPImportCodesDlg dialog

class COHIPImportCodesDlg : public CNxDialog
{
	DECLARE_DYNAMIC(COHIPImportCodesDlg)

public:
	COHIPImportCodesDlg(CWnd* pParent);   // standard constructor
	virtual ~COHIPImportCodesDlg();

// Datalist Columns
	enum EListColumns {
		elcCheck = 0,
		elcCode,
		elcProvFee,
		elcSpecFee,
	};

// Dialog Data
	enum { IDD = IDD_OHIP_IMPORT_CODES_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CNxIconButton m_btnLoad;
	CNxIconButton m_btnBrowseAndLoad;
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnImport;
	CNxEdit m_editSuffix;
	CNxStatic m_lblSuffix;
	CNxStatic m_lblPath;
	CNxStatic m_lblHelp;
	CNxEdit m_editPath;
	CNxColor m_nxcolor;

	NxButton m_chkBlank;

	NxButton m_nxbPreferProv;
	NxButton m_nxbPreferSpec;
	
	NXDATALIST2Lib::_DNxDataListPtr m_dl;

	afx_msg void OnBnClickedBrowseAndLoad();
	afx_msg void OnBnClickedOhipLoad();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnEnChangeOHIPPath();
	BOOL ImportFile(void);
	long ImportCodes(void);
	afx_msg void OnDestroy();
};
