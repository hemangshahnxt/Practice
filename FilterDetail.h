#if !defined(AFX_FILTERDETAIL_H__3F331A21_46B0_11D4_957A_00C04F4C8415__INCLUDED_)
#define AFX_FILTERDETAIL_H__3F331A21_46B0_11D4_957A_00C04F4C8415__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FilterDetail.h : header file
//

#define INVALID_DYNAMIC_ID	-1
/////////////////////////////////////////////////////////////////////////////
// CFilterDetailDlg dialog
class CFilterFieldInfo;
enum FieldOperatorEnum;

class CFilterDetail
{
	friend class CFilterDetailDlg;
	friend class CFilter;
	friend class CFilterDlg;
	friend class CFilterFieldInfo;
//	friend bool  CFilterFieldInfo::AddJoinToMap(CFilterDetail *pDetail, CMapStringToPtr &mapJoins) const;

// Construction
public:
	// (c.haag 2010-12-07 17:37) - PLID 40640 - We now take in a connection object
	CFilterDetail(long nFilterBase, BOOL (WINAPI* pfnGetNewFilterString)(long, long, CString&, LPCTSTR, CString&) = NULL, ADODB::_ConnectionPtr pCon = NULL);					// standard constructor
	virtual ~CFilterDetail();		// standard virtual destructor

// Handy members
public:
	// Be SURE to call the base class implementation if you derive from these
	virtual void SetOperator(long nDefault);			// Set the detail's operator
	virtual void SetValue(LPCTSTR strDefault);		// Set the detail's value
	virtual void SetUseOr(bool bUseOr);					// Set whether the detail uses AND or OR
	virtual void SetDynamicID(long nDynamicID);		//Sets the dynamic record ID.

// Interface
public:
	bool IsEmpty(); // Tells whether anything is set in this detail
	
	// You can always get the detail's clause, but you can't set it's clause
	// Use SetDetailString or SetDetail to control the contents of the detail
	bool GetWhereClause(CString &strOutClause, CMapStringToString &mapUsedFilters, bool bIncludePrefix, BOOL bSilent = FALSE);
	
	// Get and set the formatted detail string
	bool GetDetailString(CString &strOut, bool bIncludePrefix = false);
	void SetDetailString(LPCTSTR strDetail);

	// Sets the base variables according to the parameters passed
	void SetDetail(const CString &strApparentField, long nOperator, LPCTSTR strValue, bool bUseOr, long nDynamicID);
	void SetDetail(long nFieldIndex, long nOperator, LPCTSTR strValue, bool bUseOr, long nDynamicID);

	// Make sure to call the base class implementation of these functions if you derive them
	virtual void SetPosition(long nIndexPosition); // Responsibility of derived class: This is called when the position of a detail is changed
	virtual void Refresh(); // Responsibility of derived class: Make use of values in base variables
	virtual bool Store(); // Responsibility of derived class: Change the base variables to contain the right values

	static void AssertValidArray(CFilterFieldInfo * arFields, long nFieldCount);
	
// These are static and so always available even without instanciating an object
	static const CFilterFieldInfo g_FilterFields[];
	static const long g_nFilterFieldCount;

	static const CFilterFieldInfo g_BaseDependencies[];
	static const long g_nBaseDependencyCount;

	long CFilterDetail::GetDetailInfoId();
	CString GetDetailFieldNameApparent();
	FieldOperatorEnum GetDetailOperator();
	CString GetDetailValue();
	bool GetDetailUseOr();
	long GetDetailInfoIndex();
	// (r.gonet 10/09/2013) - PLID 56236 - Gets the dynamic record ID
	long GetDynamicRecordID();

	//figure out if there is an advanced filter in here
	BOOL HasAdvancedFilter();

// Implementation
protected:
	bool m_bUseOr;
	CString m_strValue;
	long m_nFieldIndex;
	FieldOperatorEnum m_foOperator;
	long m_nFilterType;
	long m_nDynamicRecordID; //Used for dynamically generated filters (such as EMR items).
	// (c.haag 2010-12-07 17:37) - PLID 40640 - We now take in a connection object
	ADODB::_ConnectionPtr m_pCon;

	BOOL (WINAPI* m_pfnGetNewFilterString)(long, long, CString&, LPCTSTR, CString&); //Used on obsolete items.
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILTERDETAILDLG_H__3F331A21_46B0_11D4_957A_00C04F4C8415__INCLUDED_)
