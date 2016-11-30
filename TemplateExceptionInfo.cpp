// TemplateExceptionInfo.cpp: implementation of the CTemplateExceptionInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TemplateExceptionInfo.h"
#include "InternationalUtils.h"

using namespace ADODB;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTemplateExceptionInfo::CTemplateExceptionInfo()
{
	// (c.haag 2006-11-13 09:54) - PLID 5993 - Initial implementation
	m_dtStartDate = COleDateTime::GetCurrentTime();
	m_dtStartDate.SetDate( m_dtStartDate.GetYear(), m_dtStartDate.GetMonth(), m_dtStartDate.GetDay() );
	m_dtEndDate = m_dtStartDate;
	m_nTemplateID = -1;
	m_nFlags = 0;
	m_nID = -1;
}

CTemplateExceptionInfo::CTemplateExceptionInfo(FieldsPtr& pFields)
{
	// (z.manning 2011-12-08 10:22) - PLID 46906 - Added ID
	m_nID = AdoFldLong(pFields, "ID");
	m_nTemplateID = AdoFldLong(pFields, "TemplateID");
	m_strDescription = AdoFldString(pFields, "Description");
	m_dtStartDate = AdoFldDateTime(pFields, "StartDate");
	m_dtEndDate = AdoFldDateTime(pFields, "EndDate");
	m_nFlags = AdoFldLong(pFields, "Flags");
}

CTemplateExceptionInfo::~CTemplateExceptionInfo()
{

}

CString CTemplateExceptionInfo::FormatDescriptionText() const
{
	CString str;
	if (m_dtStartDate == m_dtEndDate) {
		str.Format("On %s: %s", FormatDateTimeForInterface(m_dtStartDate, NULL, dtoDate),
			m_strDescription);
	} else {
		str.Format("From %s through %s: %s", FormatDateTimeForInterface(m_dtStartDate, NULL, dtoDate),
			FormatDateTimeForInterface(m_dtEndDate, NULL, dtoDate), m_strDescription);
	}
	return str;
}

// (z.manning 2011-12-09 09:02) - PLID 46906 - Added comparison operators
bool CTemplateExceptionInfo::operator==(CTemplateExceptionInfo &exceptionCompare)
{
	if(m_nID != exceptionCompare.m_nID ||
		m_nTemplateID != exceptionCompare.m_nTemplateID ||
		m_strDescription != exceptionCompare.m_strDescription ||
		m_dtStartDate != exceptionCompare.m_dtStartDate ||
		m_dtEndDate != exceptionCompare.m_dtEndDate ||
		m_nFlags != exceptionCompare.m_nFlags
		)
	{
		return false;
	}

	return true;
}

// (z.manning 2011-12-09 09:02) - PLID 46906 - Added comparison operators
bool CTemplateExceptionInfo::operator!=(CTemplateExceptionInfo &exceptionCompare)
{
	return !(*this == exceptionCompare);
}