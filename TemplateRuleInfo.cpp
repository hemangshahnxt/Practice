// TemplateRuleInfo.cpp: implementation of the CTemplateRuleInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TemplateRuleInfo.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTemplateRuleInfo::CTemplateRuleInfo()
{
	// (z.manning 2011-12-09 09:24) - PLID 46906 - Added ID
	m_nID = -1;
	m_strDescription = "";
	
	m_bWarningOnFail = FALSE;
	m_strWarningOnFail = "";

	m_bPreventOnFail = FALSE;

	m_nTypeListObjectType = 0;
	m_aryTypeList.RemoveAll();

	m_nPurposeListObjectType = 0;
	m_aryPurposeList.RemoveAll();

	m_nBookingCount = -1;

	m_bAndDetails = TRUE;

	m_bAllAppts = FALSE;

	//TES 8/31/2010 - PLID 39630 - Added OverrideLocationTemplating
	m_bOverrideLocationTemplating = FALSE;

	m_adwUsers.RemoveAll();
	m_adwPerms.RemoveAll();
}

CTemplateRuleInfo::~CTemplateRuleInfo()
{

}


// (z.manning 2011-12-09 09:27) - PLID 46906 - Added comparison operators
bool CTemplateRuleInfo::operator==(CTemplateRuleInfo &ruleCompare)
{
	if(m_nID != ruleCompare.m_nID ||
		m_strDescription != ruleCompare.m_strDescription ||
		m_bWarningOnFail != ruleCompare.m_bWarningOnFail ||
		m_strWarningOnFail != ruleCompare.m_strWarningOnFail ||
		m_bPreventOnFail != ruleCompare.m_bPreventOnFail ||
		m_nTypeListObjectType != ruleCompare.m_nTypeListObjectType ||
		m_nPurposeListObjectType != ruleCompare.m_nPurposeListObjectType ||
		m_nBookingCount != ruleCompare.m_nBookingCount ||
		m_bAndDetails != ruleCompare.m_bAndDetails ||
		m_bAllAppts != ruleCompare.m_bAllAppts ||
		m_bOverrideLocationTemplating != ruleCompare.m_bOverrideLocationTemplating
		)
	{
		return false;
	}

	if(!AreArrayContentsMatched(m_aryTypeList, ruleCompare.m_aryTypeList)) {
		return false;
	}
	if(!AreArrayContentsMatched(m_aryPurposeList, ruleCompare.m_aryPurposeList)) {
		return false;
	}
	if(!AreArrayContentsMatched(m_adwUsers, ruleCompare.m_adwUsers)) {
		return false;
	}
	if(!AreArrayContentsMatched(m_adwPerms, ruleCompare.m_adwPerms)) {
		return false;
	}	

	return true;
}

// (z.manning 2011-12-09 09:27) - PLID 46906 - Added comparison operators
bool CTemplateRuleInfo::operator!=(CTemplateRuleInfo &ruleCompare)
{
	return !(*this == ruleCompare);
}

// (z.manning 2011-12-09 09:37) - PLID 46906
void CTemplateRuleInfo::operator=(CTemplateRuleInfo &ruleSource)
{
	m_nID = ruleSource.m_nID;
	m_strDescription = ruleSource.m_strDescription;
	m_bWarningOnFail = ruleSource.m_bWarningOnFail;
	m_strWarningOnFail = ruleSource.m_strWarningOnFail;
	m_bPreventOnFail = ruleSource.m_bPreventOnFail;
	m_nTypeListObjectType = ruleSource.m_nTypeListObjectType;
	m_nPurposeListObjectType = ruleSource.m_nPurposeListObjectType;
	m_nBookingCount = ruleSource.m_nBookingCount;
	m_bAndDetails = ruleSource.m_bAndDetails;
	m_bAllAppts = ruleSource.m_bAllAppts;
	m_bOverrideLocationTemplating = ruleSource.m_bOverrideLocationTemplating;

	m_aryTypeList.RemoveAll();
	m_aryTypeList.Append(ruleSource.m_aryTypeList);

	m_aryPurposeList.RemoveAll();
	m_aryPurposeList.Append(ruleSource.m_aryPurposeList);

	m_adwUsers.RemoveAll();
	m_adwUsers.Append(ruleSource.m_adwUsers);

	m_adwPerms.RemoveAll();
	m_adwPerms.Append(ruleSource.m_adwPerms);
}