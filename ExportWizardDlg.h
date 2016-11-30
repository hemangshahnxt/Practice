// ExportWizardDlg.h: interface for the CExportWizardDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EXPORTWIZARDDLG_H__DA4F47F5_5A5E_4814_8251_F418A3388C2B__INCLUDED_)
#define AFX_EXPORTWIZARDDLG_H__DA4F47F5_5A5E_4814_8251_F418A3388C2B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ExportUtils.h"

class CExportWizardDlg : public CPropertySheet  
{
	friend class CExportWizardNameDlg;
	friend class CExportWizardFormatDlg;
	friend class CExportWizardFieldsDlg;
	friend class CExportWizardFiltersDlg;
	friend class CExportWizardTodoDlg;
	friend class CExportWizardFileDlg;

public:
	CExportWizardDlg();
	virtual ~CExportWizardDlg();

	virtual int DoModal();

	long m_nExportID; //Will be set if DoModal has returned ID_WIZFINISH, the export will have been saved by that point.
	CString m_strName;
	ExportRecordType m_ertRecordType;
	ExportFilterOption m_efoFilterOption;
	long m_nFilterFlags; // (z.manning 2009-12-14 09:27) - PLID 36576
	bool m_bIncludeFieldNames;
	bool m_bManualSort;
	bool m_bCreateTodo;
	int m_nTodoIntervalAmount;
	DateInterval m_diTodoIntervalUnit;
	long m_nTodoUser;
	
	bool m_bAllowOtherTemplates;
	
protected:

	CArray<long,long> m_arEmnTemplateIDs;

	// (z.manning 2009-12-10 15:15) - PLID 36519 - Keep track of category IDs that can be used
	// to filter history exports.
	CArray<long,long> m_arynCategoryIDs;
	
	ExportOutputType m_eotOutputType;
	CString m_strFieldSeparator;
	CString m_strFieldEscape;
	CString m_strTextDelimiter;
	CString m_strTextEscape;
	CArray<SpecialChar,SpecialChar> m_arSpecialChars;
	CString m_strRecordSeparator;
	CString m_strRecordSeparatorEscape;
	
	CArray<SelectedExportField,SelectedExportField> m_arExportFields;

	FilterableDate m_fdDateFilter;
	DateFilterOption m_dfoFromDateType;
	int m_nFromDateOffset;
	COleDateTime m_dtFilterFrom;
	DateFilterOption m_dfoToDateType;
	int m_nToDateOffset;
	COleDateTime m_dtFilterTo;
	long m_nLwFilterID;
	CArray<SortField,SortField> m_arSortFields;
	
	bool m_bPromptForFile;
	CString m_strFileName;

	void Save();
};

#endif // !defined(AFX_EXPORTWIZARDDLG_H__DA4F47F5_5A5E_4814_8251_F418A3388C2B__INCLUDED_)
