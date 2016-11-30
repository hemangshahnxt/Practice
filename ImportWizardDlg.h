// ImportWizardDlg.h: interface for the ImportWizardDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMPORTWIZARDDLG_H__27FE85AB_81F8_4230_9C0C_1599F2DA9015__INCLUDED_)
#define AFX_IMPORTWIZARDDLG_H__27FE85AB_81F8_4230_9C0C_1599F2DA9015__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ImportUtils.h"
#include "ShowProgressFeedbackDlg.h"
#include <NxUILib\NxPropertyPage.h> // (b.savon 2015-04-28 15:19) - PLID 65485

class CImportWizardDlg  : public CPropertySheet
{
public:
	// (j.politis 2015-04-27 14:12) - PLID 65524 - Allow the import wizard to be resizable
	BOOL OnInitDialog();
	CImportWizardDlg();
	virtual ~CImportWizardDlg();

	virtual int DoModal();

	ImportRecordType m_irtRecordType;
	CString m_strFileName;
	CString m_strFieldSeparator;
	CString m_strTextQualifier;
	CShowProgressFeedbackDlg *m_pProgressDlg;
	// (j.politis 2015-04-27 14:12) - PLID 65524 - Allow the import wizard to be resizable
	static int CALLBACK XmnPropSheetCallback(HWND hWnd, UINT message, LPARAM lParam);
	void OnPageSetActive(CPropertyPage *pPage);

protected:
	// (j.politis 2015-04-27 14:12) - PLID 65524 - Allow the import wizard to be resizable
	BOOL m_bNeedInit;
	CSize m_ClientSize;
	int	m_nMinCX;
	int	m_nMinCY;
	int	m_pageRightPadding;
	int	m_pageBottomPadding;

protected:
	// (j.politis 2015-04-27 14:12) - PLID 65524 - Allow the import wizard to be resizable
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI); 
	void ResizePage(CPropertyPage *pPage);
};

#endif // !defined(AFX_IMPORTWIZARDDLG_H__27FE85AB_81F8_4230_9C0C_1599F2DA9015__INCLUDED_)
