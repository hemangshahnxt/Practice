#if !defined(AFX_SCANMULTIDOCDLG_H__7CDE8113_5F16_43E4_B4B4_47E6C6CBC45C__INCLUDED_)
#define AFX_SCANMULTIDOCDLG_H__7CDE8113_5F16_43E4_B4B4_47E6C6CBC45C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ScanMultiDocDlg.h : header file
//

// (a.walling 2008-07-25 10:53) - PLID 30836 - Massive overhaul of this dialog, too numerous to document every change,
// since entire functions were completely redone.
// (a.walling 2008-07-25 10:53) - PLID 30836 - Added support for detecting barcodes and processing them.
/////////////////////////////////////////////////////////////////////////////
// CScanMultiDocDlg dialog

#include "nxtwain.h"
#include "ScannedImageViewerDlg.h"
#include <CxImage/ximage.h> // (a.walling 2013-05-08 16:15) - PLID 56610 - ximage.h now in CxImage/
#include "BarcodeDetector.h"
#include "shlobj.h"
#include "NxGdiPlusUtils.h"

class CScanMultiDocDlg : public CNxDialog
{
// Construction
public:
	CScanMultiDocDlg(CWnd* pParent);   // standard constructor

	// (r.galicki 2008-09-05 11:54) - PLID 31242 - current patient ID
	long m_nPatientID;
	// (r.galicki 2008-11-04 16:53) - PLID 31242 - PIC ID
	long m_nPIC;

	// (c.haag 2010-02-01 11:45) - PLID 37806 - Store the offset between the server and workstation
	COleDateTimeSpan m_dtOffset;

	// (j.gruber 2012-08-13 17:07) - PLID 52094 - Let them default the service date
	COleDateTime m_dtService;
	NXTIMELib::_DNxTimePtr		m_nxdService;

	// (a.walling 2010-01-28 14:31) - PLID 28806 - Now passes a connection pointer
	static void WINAPI CALLBACK OnNxTwainPreCompress(
		const LPBITMAPINFO pbmi, // 
		BOOL& bAttach, // bAttach
		BOOL& bAttachChecksum, // bAttachChecksum
		long& nChecksum, // Checksum
		ADODB::_Connection* lpCon); // Connection

	// (a.walling 2008-09-03 14:16) - PLID 22821 - use a gdiplus bitmap
	// (a.walling 2010-04-08 08:23) - PLID 38170 - Back to CxImage!
	static void WINAPI CALLBACK OnTWAINCallback(
		NXTWAINlib::EScanType type, /* Are we scanning to the patient folder, or to another location? */
		const CString& strFileName, /* The full filename of the document that was scanned */
		BOOL& bAttach,
		void* pUserData,
		CxImage& cxImage);

	enum EPatientListColumns {
		eplcPersonID = 0,
		eplcUserDefinedID,
		eplcLast,
		eplcFirst,
		eplcMiddle,
		eplcBirthDate,
		eplcColor,
	};

	enum EDocumentListColumns {
		eScanID,
		ePath,
		ePersonID,
		eName,
		eCategory,
		eUserDefinedID,
		eLast,
		eFirst,
		eMiddle,
		// (r.galicki 2008-09-04 10:44) - PLID 31242 - Added IsPhoto and ServiceDate columns to document list
		eIsPhoto,
		eServiceDate,
	};

	struct CScannedDocument {
		CString strFilename;
		CString strBarcode;
	};

// Dialog Data
	//{{AFX_DATA(CScanMultiDocDlg)
	enum { IDD = IDD_SCAN_MULTIPLE_DOCUMENTS };
	CNxIconButton	m_nxibGroup;
	CNxStatic	m_lblSelectedDocs;
	CNxIconButton	m_nxibAssocCat;
	CNxStatic	m_lblDefCat;
	CNxIconButton	m_nxibShowPreview;
	CNxStatic	m_lblDocPrefix;
	CNxEdit	m_editDocPrefix;
	NxButton	m_nxbDetectBarcodes;
	CNxIconButton	m_nxibBeginScan;
	CNxIconButton	m_nxibAssociate;
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnCommit;
	CNxColor	m_nxcolor;
	NxButton	m_nxbAlwaysSavePDF;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CScanMultiDocDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:	
	CString GetDocumentName();
	// (r.galicki 2008-09-04 09:36) - PLID 31242 - AddToDocuments now has IsPhoto and ServiceDate parameters, changed strDocCat parameter to long
	// (a.walling 2008-09-12 14:06) - PLID 31364 - Added saTempFiles for creating a grouped PDF document
	//(e.lally 2010-01-13) PLID 32201 - Added bAlwaysCreatePDF for whether or not to create all documents passed in as pdf files
	void AddToDocuments(IProgressDialog* pProgressDialog, long nPatientID, long nDocCat, CString strDocName, CString strDocFile, CString strFirst, CString strLast, long nUserDefinedID, BOOL bIsPhoto, const BOOL bAlwaysCreatePDF, const COleDateTime &dtService, CStringArray& saTempFiles);
	void PreviewImage(const CString& strPath);
	CScannedImageViewerDlg* m_pScannedImageDlg;

	long m_nCurrentIndex;
	long m_nLastUserDefinedID;

	DWORD m_nDocNum;
	
	BOOL m_bFinishedScanning;

	// (a.wilson 2013-05-07 13:42) - PLID 52960 - checkbox to save the default prefix.
	NxButton m_chkDefaultPrefix;
	CString m_strDefaultPrefix;

	long m_nDefaultCategoryID;
	// (r.galicki 2008-09-04 16:58) - PLID 31242 - Added default value for IsPhoto
	BOOL m_bDefaultIsPhoto;

	static CBarcodeDetector m_BarcodeDetector;
	// (d.thompson 2010-03-23) - PLID 37839 - Convert patient list to DL2
	//NXDATALISTLib::_DNxDataListPtr m_pPatientList;
	NXDATALIST2Lib::_DNxDataListPtr m_pPatList;
	// (r.galicki 2008-09-09 14:23) - PLID 31280 - Convert DocList to DL2
	NXDATALIST2Lib::_DNxDataListPtr m_pDocList;
	//NXDATALISTLib::_DNxDataListPtr m_pDocList;

	NXDATALIST2Lib::_DNxDataListPtr m_pCatList;

	void RemoveEmptyDocumentRows();
	void CleanupFiles();

	// (r.galicki 2008-09-15 11:06) - PLID 31280 - Check/Set row colors based on group status
	void EnsureRowColor(NXDATALIST2Lib::IRowSettingsPtr pRow);

	// Generated message map functions
	//{{AFX_MSG(CScanMultiDocDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnAssociate();
	afx_msg void OnScanMultiple();
	afx_msg void OnCommit();
	afx_msg void OnCancelChanges();
	afx_msg void OnDestroy();
	afx_msg void OnBtnShowPreview();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg LRESULT OnTwainImageScanned(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTwainXferdone(WPARAM wParam, LPARAM lParam);
	afx_msg void OnCheckDetectBarcodes();
	afx_msg void OnChangeEditDefaultDocumentPrefix();
	afx_msg void OnSelSetCategory(LPDISPATCH lpSel);
	afx_msg void OnBtnAssociateCategory();
	afx_msg void OnSelSetDocumentList(LPDISPATCH lpSel);
	// (d.thompson 2010-03-23) - PLID 37839 - Convert patient list to dl2
	//afx_msg void OnSelSetPatientScanList(long nRow);
	afx_msg void OnBtnGroup();
	afx_msg void OnRButtonDownDocumentList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnEditingStartingDocumentList(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnDragEndDocumentList(LPDISPATCH lpRow, short nCol, LPDISPATCH lpFromRow, short nFromCol, long nFlags);
	afx_msg void OnDragBeginDocumentList(BOOL FAR* pbShowDrag, LPDISPATCH lpRow, short nCol, long nFlags);
	afx_msg void OnBtnAlwaysSaveAsPDF();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedDefaultPrefixCheck();
public:
	void OnEditingFinishingDocumentList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	void SelChosenPatientScanList(LPDISPATCH lpRow);
	afx_msg void OnEnKillfocusEditDefaultServiceDate();
	
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCANMULTIDOCDLG_H__7CDE8113_5F16_43E4_B4B4_47E6C6CBC45C__INCLUDED_)
