#pragma once
#include "PatientDialog.h"
#include "GenericBrowserDlg.h"
// CPatientDashboardDlg dialog
#include "EmrTemplateManagerDlg.h"

#import "NexTech.COM.tlb"

// (j.gruber 2012-03-20 13:52) - PLID 48702 -created for

class IPatientDashboardBrowserInterface : public IGenericBrowserInterface 
{
public:
	IPatientDashboardBrowserInterface();

	STDMETHOD(GetExternal)( 
		/* [out] */ IDispatch **ppDispatch);


private:
	// (j.gruber 2012-06-11 10:17) - PLID 50223 - custom menu
	virtual HRESULT CustomContextMenu(DWORD dwID, POINT *ppt, IUnknown *pcmdtReserved, IDispatch *pdispReserved);
};

enum PatientDashboardType
    {
        pdtAllergies = -1,
        pdtCurrentMeds = -2,
        pdtPrescriptions = -3,
        pdtHistoryImages = -4,
        pdtEMNSummary = -5,
        pdtProcedures = -6,
        pdtDiagCodes = -7,
        pdtCreateEMNs = -8,
        pdtProblems = -9,        
        pdtLabs = -10,
        pdtEMNItems = -11,
        pdtAppointments = -12,
		pdtBills = -13,
		pdtHistoryAttachments = -14, // (c.haag 2015-04-29) - NX-100441
    };


class CPatientDashboardDlg : public CBrowserEnabledDlg
{
	DECLARE_DYNAMIC(CPatientDashboardDlg)
	DECLARE_DISPATCH_MAP();

public:
	CPatientDashboardDlg(CNxView* pParent = NULL);   // standard constructor
	virtual ~CPatientDashboardDlg();

	void SetColor(OLE_COLOR nNewColor);

	void NavigateToStream(IStreamPtr pStream) throw(...);

	IStreamPtr m_pPendingStream;

// Dialog Data
	enum { IDD = IDD_PATIENT_DASHBOARD };

protected:
	
	// (j.gruber 2012-03-20 13:44) - PLID 49054
	long SelectEMRCollection(long x, long y, CWnd *pParent, BOOL bIsEMR /*as opposed to an EMR template*/);	
	class CEmrTemplateManagerDlg* m_pdlgTemplateManager;

	void SetBrowserHTML(CString strHTML);
	
	void LoadPendingStream();

	void RefreshTemplateList();
	class IPatientDashboardBrowserInterface* m_piClient;

	// (j.gruber 2012-05-16 10:23) - PLID 50401 - width settings
	long m_nMainLeftWidth;	
	BOOL m_bCloseRight;
	CString m_strSplitPos;

	// (c.haag 2015-05-06) - NX-100442 - Need to track whether a case history is open
	BOOL m_bCaseHistoryIsOpen;
		
	void RefreshPartialDashboard(PatientDashboardType pdtTypeToRefresh, CString strIDToRefresh);	

	void SetScrollPosition(CString strDivToSetScroll, CString strScrollValue);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	
	virtual void UpdateView(bool bForceRefresh = true);
	virtual void StoreDetails();	
	afx_msg void OnTimer(UINT nIDEvent);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	DECLARE_MESSAGE_MAP()
	// (j.gruber 2012-06-11 15:48) - PLID 50225 - table checkers
	afx_msg LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnTableChangedEx(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMsgEditEMRTemplate(WPARAM wParam, LPARAM lParam); // (j.gruber 2012-06-26 10:07) - PLID 49054
	afx_msg void OnBeforeNavigate2Browser(LPDISPATCH pDisp, VARIANT FAR* URL, VARIANT FAR* Flags, VARIANT FAR* TargetFrameName, VARIANT FAR* PostData, VARIANT FAR* Headers, BOOL FAR* Cancel);
	afx_msg void OnDestroy();	// (j.armen 2012-07-03 17:11) - PLID 51206
	DECLARE_EVENTSINK_MAP()

#pragma region DispatchMapDeclarations
	// (j.gruber 2012-03-20 13:35) - PLID 49046 - create new EMN
	afx_msg void CreateNewEMN(long nID);
	// (j.gruber 2012-03-20 13:35) - PLID 49047 - open summary
	afx_msg void OpenEMNSummary(long nID);
	// (j.gruber 2012-03-20 13:35) - PLID 49053 - open existing EMN
	afx_msg void OpenExistingEMN(long nID);
	// (j.gruber 2012-03-20 13:35) - PLID 49049 - Configure EMNs
	afx_msg void ConfigureEMNs();
	// (j.gruber 2012-03-20 13:35) - PLID 49051 - Configure Dashboard
	afx_msg void ConfigureDashboard();
	// (j.gruber 2012-03-20 13:35) - PLID 49052 - Open Part 1
	afx_msg void OpenLockManager();
	// (j.gruber 2012-03-20 13:38) - PLID 49054 - editing templates
	afx_msg void EditTemplates();
	// (j.gruber 2012-03-20 13:35) - PLID 49052 - Open Part 1
	afx_msg void OpenProblemList();
	// (j.gruber 2012-03-20 13:35) - PLID 49052 - Open Part 1
	afx_msg void OpenEMRSummary();
	// (j.gruber 2012-03-20 13:35) - PLID 49052 - Open Part 1
	afx_msg void OpenPtSummary();
	// (j.gruber 2012-03-20 13:35) - PLID 49052 - Open Part 1
	afx_msg void OpenPtSeen();
	// (j.gruber 2012-03-20 13:35) - PLID 49053 - Open Part 2
	afx_msg void OpenPtsToBill();
	// (j.gruber 2012-03-20 13:35) - PLID 49053 - Open Part 2
	afx_msg void OpenAnalysis();
	// (j.gruber 2012-03-20 13:35) - PLID 49053 - Open Part 2
	afx_msg void OpenWellness();
	// (j.gruber 2012-03-20 13:35) - PLID 49053 - Open Part 2
	afx_msg void OpenLab(long nID);
	// (j.gruber 2012-06-26 17:00) - PLID 51214 - open image
	// (c.haag 2015-04-30) - NX-100444 - Renamed to OpenAttachment
	afx_msg void OpenAttachment(long nID);

#pragma endregion

};

