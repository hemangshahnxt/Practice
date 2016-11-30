#if !defined(AFX_TEMPLATEHITSET_H__ACEAF221_5A06_11D2_80D8_00104B2FE914__INCLUDED_)
#define AFX_TEMPLATEHITSET_H__ACEAF221_5A06_11D2_80D8_00104B2FE914__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TemplateHitSet.h : header file
//

struct CSchedTemplateInfo {
	CString m_strName;
	COLORREF m_clrColor;
};

/////////////////////////////////////////////////////////////////////////////
// CTemplateHitSet object (uses ADO now)

class CTemplateHitSet
{
public:
	// (z.manning, 05/21/2007) - PLID 26062 - Added an overload of requery that takes a comma delimited
	// list of IDs insteady of just one ID.
	//TES 6/22/2010 - PLID 39278 - Added a LocationID parameter, determines whether to include Resource Availability templates
	// (a.walling 2010-06-25 12:36) - PLID 39278 - separate param for including Resource Availability template
	void Requery(COleDateTime dtApplyDate, CArray<long,long> &arynResourceIDs, BOOL bIncludeTemplateRules, long nLocationID, bool bIncludeTemplates, bool bIncludeResourceAvailabilityTemplates);
	void Requery(COleDateTime dtApplyDate, long nResourceID, BOOL bIncludeTemplateRules, long nLocationID, bool bIncludeTemplates, bool bIncludeResourceAvailabilityTemplates);
	CTemplateHitSet();

	// Functions to emulate a recordset
	BOOL IsEOF();
	BOOL IsBOF();
	void MoveNext();
	void MoveFirst(); // (z.manning, 05/21/2007) - PLID 26062

	// Read-only public access to fields
	long GetLineItemID();
	const COleDateTime &GetStartTime();
	const COleDateTime &GetEndTime();
	long GetColor();
	const CString &GetText();
	long GetPriority();
	BOOL GetIsBlock();
	// (c.haag 2006-12-13 16:58) - PLID 23485 - These
	// functions are only called if the preference to
	// do template block scheduling ("EnablePrecisionTemplating")
	// is set
	long GetRuleID();
	BOOL GetRuleAndDetails();
	BOOL GetRuleAllAppts();
	BOOL GetRuleOverrideLocationTemplating(); //TES 8/31/2010 - PLID 39630
	long GetRuleObjectType();
	long GetRuleObjectID();
	long GetResourceID(); // (z.manning, 05/21/2007) - PLID 26062
	BOOL GetIsAllResources(); // (z.manning, 05/24/2007) - PLID 26062
	long GetTemplateID(); // (z.manning 2009-07-18 12:26) - PLID 34939
	long GetLocationID(); //TES 6/22/2010 - PLID 39278
	long GetCollectionID(); // (c.haag 2014-12-16) - PLID 64256
	const CString& GetCollectionName(); // (c.haag 2014-12-16) - PLID 64256
	
protected:
	// Access to the data
	ADODB::_RecordsetPtr m_prs;
	ADODB::FieldPtr m_fldLineItemID;
	ADODB::FieldPtr m_fldStartTime;
	ADODB::FieldPtr m_fldEndTime;
	ADODB::FieldPtr m_fldColor;
	ADODB::FieldPtr m_fldName;
	ADODB::FieldPtr m_fldPriority;
	ADODB::FieldPtr m_fldIsBlock;
	ADODB::FieldPtr m_fldRuleID;
	ADODB::FieldPtr m_fldRuleAndDetails;
	ADODB::FieldPtr m_fldRuleAllAppts;
	ADODB::FieldPtr m_fldRuleOverrideLocationTemplating; //TES 8/31/2010 - PLID 39630
	ADODB::FieldPtr m_fldRuleObjectType;
	ADODB::FieldPtr m_fldRuleObjectID;
	ADODB::FieldPtr m_fldResourceID; // (z.manning, 05/21/2007) - PLID 26062
	ADODB::FieldPtr m_fldAllResources; // (z.manning, 05/24/2007) - PLID 26062
	ADODB::FieldPtr m_fldTemplateID; // (z.manning 2009-07-18 12:28) - PLID 34939
	ADODB::FieldPtr m_fldLocationID; //TES 6/22/2010 - PLID 39278
	ADODB::FieldPtr m_fldCollectionID; // (c.haag 2014-12-16) - PLID 64256
	ADODB::FieldPtr m_fldCollectionName; // (c.haag 2014-12-16) - PLID 64256
	void LoadRecord();

	// Fields
	long	m_TemplateLineItemID;
	COleDateTime	m_StartTime;
	COleDateTime	m_EndTime;
	long	m_Color;
	CString m_strText;
	long	m_Priority;
	BOOL	m_bIsBlock;
	// (c.haag 2006-12-13 16:58) - PLID 23485 - These
	// fields are only populated if the preference to
	// do template block scheduling ("EnablePrecisionTemplating")
	// is set
	long	m_nRuleID;
	BOOL	m_bRuleAndDetails;
	BOOL	m_bRuleAllAppts;
	BOOL	m_bRuleOverrideLocationTemplating; //TES 8/31/2010 - PLID 39630
	long	m_nRuleObjectType;
	long	m_nRuleObjectID;
	long	m_nResourceID; // (z.manning, 05/24/2007) - PLID 26062
	BOOL	m_bAllResources; // (z.manning, 05/24/2007) - PLID 26062
	long	m_nTemplateID; // (z.manning 2009-07-18 12:28) - PLID 34939
	long	m_nLocationID;	//TES 6/22/2010 - PLID 39278
	long	m_nCollectionID; // (c.haag 2014-12-16) - PLID 64256
	CString m_strCollectionName; // (c.haag 2014-12-16) - PLID 64256
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEMPLATEHITSET_H__ACEAF221_5A06_11D2_80D8_00104B2FE914__INCLUDED_)
