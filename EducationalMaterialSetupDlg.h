#pragma once
#include "AdministratorRc.h"
#include "UIFormChangesTracker.h"

// CEducationalMaterialSetupDlg dialog
//
// (c.haag 2010-09-22 15:54) - PLID 40629 - Initial implementation. This class is used by
// the user to configure patient educational "packets" which are really just a list of letter
// writing filters and templates.

class CEducationalMaterialSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEducationalMaterialSetupDlg)

public:
	CEducationalMaterialSetupDlg(CWnd* pParent);   // standard constructor
	virtual ~CEducationalMaterialSetupDlg();

// Dialog Data
	enum { IDD = IDD_EDUCATION_TEMPLATES_CONFIG };

private:
	NXDATALIST2Lib::_DNxDataListPtr m_dlList;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnAdd;
	CNxIconButton m_btnDelete;

private:
	// This object is responsible for tracking changes made to EducationTemplatesT, which is the
	// only table this dialog configures.
	CUIFormChangesTracker m_ChangesTracker;

	// This is the list of all enumerated templates from the shared path
	CStringArray m_astrWordTemplates;
	BOOL m_bWordTemplatesEnumerated;

	// This tracks which MergeTemplateT.ID values are in the embedded dropdown
	CMap<long,long,BOOL,BOOL> m_mapEmbeddedMergeTemplateIDs;

private:
	// Populates m_astrWordTemplates with a list of all templates in the shared path
	void PopulateWordTemplateArray(const CString& strPath);
	// Populates m_astrWordTemplates with a list of all templates in the shared path
	void PopulateWordTemplateArray();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	virtual BOOL OnInitDialog();
public:
	DECLARE_EVENTSINK_MAP()
	void EditingFinishedListEducation(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	afx_msg void OnBnClickedAdd();
	afx_msg void OnBnClickedDelete();
	afx_msg void OnBnClickedOK();
	void RButtonUpListEducation(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void EditingFinishingListEducation(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
};
