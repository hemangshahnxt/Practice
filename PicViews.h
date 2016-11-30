#pragma once

#include "HistoryDlg.h"
#include "PatientLabsDlg.h"
#include "ProcInfoCenterDlg.h"
#include "EmrHelpers.h"
#include "NexTechDialogView.h"

// (a.walling 2011-12-08 17:27) - PLID 46641 - Create views for Labs, History, and ProcInfoCenter

class CPicDocument;
class CPicDocTemplate;

class CHistoryView : public CNexTechDialogView, Emr::InterfaceAccessImpl<CHistoryView>
{
public:
	DECLARE_DYNCREATE(CHistoryView)

	CHistoryView() : m_historyDlg(this) {}

	CHistoryDlg* GetHistoryDlg()
	{
		if (!this) return NULL;
		return &m_historyDlg;
	}

protected:
	virtual CWnd* CreateChildWnd();

	CHistoryDlg m_historyDlg;
};

class CPatientLabsView : public CNexTechDialogView, Emr::InterfaceAccessImpl<CPatientLabsView>
{
public:
	DECLARE_DYNCREATE(CPatientLabsView)

	CPatientLabsView() : m_patientLabsDlg(this) {}

	CPatientLabsDlg* GetPatientLabsDlg()
	{
		if (!this) return NULL;
		return &m_patientLabsDlg;
	}

protected:
	virtual CWnd* CreateChildWnd();

	CPatientLabsDlg m_patientLabsDlg;
};

class CProcInfoCenterView : public CNexTechDialogView, Emr::InterfaceAccessImpl<CProcInfoCenterView>
{
public:
	DECLARE_DYNCREATE(CProcInfoCenterView)

	CProcInfoCenterView() : m_procInfoCenterDlg(this) {}

	CProcInfoCenterDlg* GetProcInfoCenterDlg()
	{
		if (!this) return NULL;
		return &m_procInfoCenterDlg;
	}

protected:
	virtual CWnd* CreateChildWnd();

	CProcInfoCenterDlg m_procInfoCenterDlg;
};

class CPicDocTemplate : public CMultiDocTemplate
{
	DECLARE_DYNAMIC(CPicDocTemplate)
public:
	CPicDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass,
		CRuntimeClass* pFrameClass, CWnd* pOverrideFrameParent)
		: CMultiDocTemplate(nIDResource, pDocClass, pFrameClass, NULL)
		, m_pOverrideFrameParent(pOverrideFrameParent)
	{
	}

	// (a.walling 2012-11-09 08:45) - PLID 53671 - Views are now created / accessed via the doctemplate
	CHistoryView* GetHistoryView(bool bAutoCreate = true);
	CPatientLabsView* GetPatientLabsView(bool bAutoCreate = true);
	CProcInfoCenterView* GetProcInfoCenterView(bool bAutoCreate = true);

	CFrameWnd* CreateNewEmrFrame(CDocument* pDoc, CRuntimeClass* pViewClass);

	virtual void SetDefaultTitle(CDocument* pDocument);
	
	// (a.walling 2012-11-09 08:45) - PLID 53671 - Find views with a specific type
	template<typename ViewType>
	ViewType* FindView()
	{
		ViewType* pView = NULL;
		POSITION pos = GetFirstDocPosition();
		while (pos && !pView) {
			if (CPicDocument* pDoc = dynamic_cast<CPicDocument*>(GetNextDoc(pos))) {
				pView = pDoc->FindView<ViewType>();
			}			
		}
		return pView;
	}

protected:
	CWnd* m_pOverrideFrameParent;
};


class CPicDocument : public CDocument
{
	DECLARE_DYNCREATE(CPicDocument)
public:

	CPicDocTemplate* GetPicDocTemplate() const;

	// (a.walling 2012-11-09 08:45) - PLID 53671 - Find views with a specific type
	template<typename ViewType>
	ViewType* FindView()
	{
		ViewType* pView = NULL;
		POSITION pos = GetFirstViewPosition();
		while (pos && !pView) {
			pView = dynamic_cast<ViewType*>(GetNextView(pos));
		}
		return pView;
	}

	// (a.walling 2012-11-09 08:45) - PLID 53671 - Allow setting a path name directly without attempting filesystem resolution
	void SetPathNameDirect(CString strPath)
	{
		m_strPathName = strPath;
	}

protected:
};