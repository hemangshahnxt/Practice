#pragma once


#include "EMRPreviewCtrlDlg.h"
#include "EmrRc.h"
#include "PatientNexEMRDlg.h"
#include "NxAPI.h"

// (b.savon 2011-11-10 11:00) - PLID 25782
#import "NexTech.COM.tlb"

// (j.jones 2013-05-16 15:18) - PLID 56596 - replaced EMN.h with a forward declare
class CEMN;

// (b.savon 2011-11-22 11:54) - PLID 25782 - Added PatientID
class CSelectComPrinter
{
private:
	NexTech_COM::IPrintPtr m_pPrinter;
	bool m_bSelectedPrinter;
 
public:
	CSelectComPrinter(NexTech_COM::IPrintPtr pPrinter)
	{
		if( pPrinter ){
			m_pPrinter = pPrinter;
			m_bSelectedPrinter = false;
		} else{
			ThrowNxException("Invalid NexTech_COM Printer.");
		}
	}
 
	~CSelectComPrinter()
	{
		if(m_bSelectedPrinter) {
				m_pPrinter->ResetDefaultPrinter();
		}
	}

	bool SelectPrinter()
	{
		if(m_pPrinter->SelectDefaultPrinter()) {
			m_bSelectedPrinter = true;
			return true;
		} else{
			return false;
		}
	}
};

// CEMRPreviewMultiPopupDlg dialog
// (a.walling 2009-11-30 10:56) - PLID 24194 - This dialog simply provides a list of EMN and related info and allows the user to print multiple EMNs.

class CEMRPreviewMultiPopupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEMRPreviewMultiPopupDlg)

public:
	CEMRPreviewMultiPopupDlg(long nPatientID, CWnd* pParent);   // standard constructor, must use a parent
	~CEMRPreviewMultiPopupDlg();

	struct CEMNPointer {
		CEMNPointer() {
			pEMN = NULL;
			nID = NULL;
			bDelete = false;
			bHeaderFooterValid = false;
		};

		// (z.manning 2012-09-11 17:46) - PLID 52543 - Added modified date
		// (c.haag 2013-02-28) - PLID 55368 - Added EMN template ID
		CEMNPointer(CEMN* pEMN, long nID, long nTemplateID, const CString& strEMRTitle, const CString& strEMNTitle, COleDateTime dtDate, COleDateTime dtInputDate, COleDateTime dtModifiedDate)
		{
			this->pEMN = pEMN;
			
			this->nID = nID;

			this->nTemplateID = nTemplateID;
			this->strEMRTitle = strEMRTitle;
			this->strEMNTitle = strEMNTitle;
			this->dtDate = dtDate;
			this->dtInputDate = dtInputDate;
			this->dtModifiedDate = dtModifiedDate;

			this->bDelete = false;

			this->bHeaderFooterValid = false;
		};

		CEMN* pEMN;
		long nID;

		CString strEMRTitle;
		CString strEMNTitle;
		COleDateTime dtDate;
		COleDateTime dtInputDate;
		// (z.manning 2012-09-11 17:49) - PLID 52543
		COleDateTime dtModifiedDate;
		// (c.haag 2013-02-28) - PLID 55368
		long nTemplateID;

		bool bDelete;

		bool bHeaderFooterValid;
		CString strHeaderHTML;
		CString strFooterHTML;
	};

	// (z.manning 2012-09-11 17:49) - PLID 52543 - Added modified date
	// (c.haag 2013-02-28) - PLID 55368 - We also need the EMN template ID
	void AddAvailableEMN(CEMN* pEMN, long nID, long nTemplateID, const CString& strEMRTitle, const CString& strEMNTitle, COleDateTime dtDate, COleDateTime dtInputDate, COleDateTime dtModifiedDate);
	// (c.haag 2013-02-28) - PLID 55368 - Assigns the custom preview layout list
	void SetCustomPreviewLayoutList(NexTech_Accessor::_EMRCustomPreviewLayoutsPtr pLayouts);
	// (c.haag 2013-02-28) - PLID 55368 - Gets the number of custom preview layouts
	int GetCustomPreviewLayoutCount();

	CEMRPreviewCtrlDlg* m_pEMRPreviewCtrlDlg;

// Dialog Data
	enum { IDD = IDD_EMR_PREVIEW_MULTI_POPUP };

	CNxStatic		m_nxstaticPreviewArea;
	CNxColor		m_nxcolor;
	NxButton		m_checkPrintReverse;
	CNxIconButton	m_nxbPrint;
	CNxIconButton	m_nxbPrintPreview;
	// (b.savon 2011-11-07 09:56) - PLID 25782 - Added m_nxbPrintAll
	CNxIconButton	m_nxbPrintAll;
	CNxIconButton	m_nxbClose;
	NXDATALIST2Lib::_DNxDataListPtr m_pList;

	// (c.haag 2013-02-28) - PLID 55373 - Updated to include the layout name column and document path
	// (the latter user to be at the EMN scope). These are columns in the context of EMN rows.
	enum EListColumns {
		lcEMNPointer = 0,
		lcCheck = 2,
		lcLayoutName = 3,
		lcEMNTitle = 4,
		lcEMRTitle = 5,
		lcEMNDate = 6,
		lcEMNInputDate = 7,
		lcDocumentPath = 8,
	};

	// (c.haag 2013-02-28) - PLID 55373 - Columns in the context of layout rows if we have a tree hierarchy
	// where individual rows can be layouts.
	enum EListColumnsForLayouts {
		lclLayoutID = 1,
		lclCheck = 2,
		lclLayoutName = 3,
	};
	
	void NavigateToMessage(CString strMessage);

protected:
	CList<CEMNPointer, CEMNPointer&> m_listEMNs;
	// (c.haag 2013-02-28) - PLID 55368 - Custom preview layouts for EMN's
	NexTech_Accessor::_EMRCustomPreviewLayoutsPtr m_pCustomPreviewLayouts;

	CString m_strPrintTemplatePath;
	CStringArray m_arTempFiles;
	void CleanupTempFiles();

	bool m_bPrintPreview;
	bool m_bReady;
	bool m_bPrinting;

	void PrintAll();

	// (b.savon 2011-11-07 09:59) - PLID 25782 - Needed for printing complete medical history
	CString GetEMNFile( NXDATALIST2Lib::IRowSettingsPtr pRow );
	void PopulateAttachedHistoryDocuments( CList<CString, CString&> &lDocuments );
	void PopulateAttachedLabs( CList<CString, CString&> &lLabs, CList<CString, CString&> &lForms );
	void PrintEntireHistory();
	// (c.haag 2013-03-11) - PLID 55373 - Called by PrintEntireHistory to print the history for a single EMN
	// (d.thompson 2013-11-07) - No PLID - While implementing Print Auditing, I noticed this function is not actually called anywhere, 
	//	so I commented it out.
	//void PrintHistory(NXDATALIST2Lib::IRowSettingsPtr pRow);
	void PrintSingleFile(NexTech_COM::IPrintPtr &pPrinter, CString strFile);
	void UpdateGUIForPrinting();
	long m_nPatientID;
	BOOL m_bPrintTemplateTeardownFired;
	BOOL m_bPrintEntireHistory;

	// (c.haag 2013-02-28) - PLID 55373 - Populate the tree with just EMN's. There will be no child nodes; this is how we used to populate before this PL item.
	void PopulateTreeWithEMNRootNodes();
	// (c.haag 2013-02-28) - PLID 55373 - Populate the tree such that the root nodes are layouts and all children are EMN's.
	void PopulateTreeWithLayoutRootNodes();
	// (c.haag 2013-02-28) - PLID 55373 - Called by PopulateTreeWithLayoutRootNodes for eachlayout
	void PopulateLayoutNode(NexTech_Accessor::_EMRCustomPreviewLayoutPtr pLayout, int& nValidInputDateCount);

	// (c.haag 2013-02-28) - PLID 55373 - Creates an EMN row in the tree
	NXDATALIST2Lib::IRowSettingsPtr CreateEMNTreeRow(CEMNPointer& emn, NexTech_Accessor::_EMRCustomPreviewLayoutPtr pLayout, int& nValidInputDateCount);
	// (c.haag 2013-02-28) - PLID 55373 - Creates a layout row in the tree
	NXDATALIST2Lib::IRowSettingsPtr CreateLayoutTreeRow(NexTech_Accessor::_EMRCustomPreviewLayoutPtr pLayout);

	void PrepareEMNs();
	// (c.haag 2013-02-28) - PLID 55373 - Prepares a single EMN for printing and generates its default EMN preview if necessary
	void PrepareEMN(CEMNPointer& emn, NXDATALIST2Lib::IRowSettingsPtr pRow, Nx::SafeArray<BSTR>& saCustomPreviewLayoutIDs);
	// (c.haag 2013-02-28) - PLID 55373 - Called after custom previews are generated to assign each custom preview to its respective row in the tree
	void AssignCustomPreview(CEMNPointer& emn, NXDATALIST2Lib::IRowSettingsPtr pRow, NexTech_Accessor::_EMRCustomPreviewPtr pPreview);

	CString GetMultiPrintTemplate();
	// (c.haag 2013-02-28) - PLID 55373 - Overload for building listEMNPrintInfo for one row
	// (d.thompson 2013-11-07) - PLID 59351 - Now returns the CEMN* for the template generated
	// (r.farnworth 2015-07-28 10:06) - PLID 64618 - Added bOverrideCheck
	CEMN* GetMultiPrintTemplate(NXDATALIST2Lib::IRowSettingsPtr pRow,
		CList<NxPrintTemplate::DocInfo, NxPrintTemplate::DocInfo&>& listEMNPrintInfo, bool bOverrideCheck = false);

	// (c.haag 2013-02-28) - PLID 55373 - Returns the full path to the generated default preview of an EMN
	CString GetEMNDefaultPreview(CEMNPointer& emn);

	// (c.haag 2013-02-28) - PLID 55373 - Gets the print info for a given row. This must be called after PrepareEMNs() is called
	NxPrintTemplate::DocInfo GetRowDocInfo( NXDATALIST2Lib::IRowSettingsPtr pRow );

	// (r.gonet 06/13/2013) - PLID 56850 - Gets the template with the entire history (all emns on the list) on it.
	CString GetEntireHistoryTemplate();
	// (r.gonet 06/13/2013) - PLID 56850 - Sends the entire history (all the emns on the list) to the printer.
	void PrintHistoryCombined();
	// (d.thompson 2013-11-07) - PLID 59351 - Audit all prints
	void AuditPrinting(CEMN *pEMNToPrint);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	virtual BOOL OnInitDialog();
	virtual void OnCancel(); // (a.walling 2009-11-23 12:46) - PLID 36404 - Don't allow exit until printing complete

	// (a.walling 2009-11-23 12:44) - PLID 36404 - Get notified when the printing is complete
	afx_msg LRESULT OnPrintComplete(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDocumentComplete(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDestroy();
	afx_msg void OnBtnPrint();
	afx_msg void OnBtnPrintPreview();
	afx_msg void OnCheckPrintReverse();

	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	void OnLeftClickList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnBnClickedButtonPrintAll();
};
