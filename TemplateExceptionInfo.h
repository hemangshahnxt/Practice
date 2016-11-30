// TemplateExceptionInfo.h: interface for the CTemplateExceptionInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TEMPLATEEXCEPTIONINFO_H__0B7D1B7F_64B5_4971_8CA0_D54D54C6EF1F__INCLUDED_)
#define AFX_TEMPLATEEXCEPTIONINFO_H__0B7D1B7F_64B5_4971_8CA0_D54D54C6EF1F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define SCHED_TEMPLATE_FLAG_TOP_PRIORITY			1
#define SCHED_TEMPLATE_FLAG_IGNORE					2

//
// (c.haag 2006-11-13 09:35) - PLID 5993 - Initial implementation of CTemplateExceptionInfo
//
class CTemplateExceptionInfo  
{
public:
	// (z.manning 2011-12-08 10:20) - PLID 46906 - Added ID
	long m_nID;
	long m_nTemplateID;

public:
	CString m_strDescription;
	COleDateTime m_dtStartDate;		// Start date - must not be null or invalid
	COleDateTime m_dtEndDate;		// End date - must not be null or invalid
	long m_nFlags;					// Flags - 1 = Top priority, 2 = Not present/ignored

public:
	CTemplateExceptionInfo();
	CTemplateExceptionInfo(ADODB::FieldsPtr& pFields);
	virtual ~CTemplateExceptionInfo();

	// (z.manning 2011-12-09 09:02) - PLID 46906 - Added comparison operators
	bool operator==(CTemplateExceptionInfo &exceptionCompare);
	bool operator!=(CTemplateExceptionInfo &exceptionCompare);

public:
	CString FormatDescriptionText() const;
};

#endif // !defined(AFX_TEMPLATEEXCEPTIONINFO_H__0B7D1B7F_64B5_4971_8CA0_D54D54C6EF1F__INCLUDED_)
