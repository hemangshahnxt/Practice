// TemplateRuleInfo.h: interface for the CTemplateRuleInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TEMPLATERULEINFO_H__0643D03D_CE08_45D9_82B0_1BA157705EF2__INCLUDED_)
#define AFX_TEMPLATERULEINFO_H__0643D03D_CE08_45D9_82B0_1BA157705EF2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CTemplateRuleInfo  
{
public:
	CTemplateRuleInfo();
	virtual ~CTemplateRuleInfo();

	// (z.manning 2011-12-09 09:37) - PLID 46906
	void operator=(CTemplateRuleInfo &ruleSource);

	// (z.manning 2011-12-09 09:27) - PLID 46906 - Added comparison operators
	bool operator==(CTemplateRuleInfo &ruleCompare);
	bool operator!=(CTemplateRuleInfo &ruleCompare);

public:
	// (z.manning 2011-12-09 09:23) - PLID 46906 - Added ID
	long m_nID;
	CString m_strDescription;
	
	BOOL m_bWarningOnFail;
	CString m_strWarningOnFail;

	// Does the 
	BOOL m_bPreventOnFail;
	
	// For type list management
	UINT m_nTypeListObjectType; // 0 == ignore type list; 1 == IS in the list; 101 == IS NOT in the list
	CDWordArray m_aryTypeList;

	// For purpose list management
	UINT m_nPurposeListObjectType; // 0 == ignore type list; 2 == IS in the list; 102 == IS NOT in the list
	CDWordArray m_aryPurposeList;

	// For booking max management
	long m_nBookingCount;
	
	// How to interpret multiple details
	BOOL m_bAndDetails;

	// For all appointments
	BOOL m_bAllAppts;

	//TES 8/31/2010 - PLID 39630 - Added OverrideLocationTemplating
	BOOL m_bOverrideLocationTemplating;

	// (c.haag 2003-08-01 12:37) - Permissions
	CDWordArray m_adwUsers;
	CDWordArray m_adwPerms;
};

#endif // !defined(AFX_TEMPLATERULEINFO_H__0643D03D_CE08_45D9_82B0_1BA157705EF2__INCLUDED_)
