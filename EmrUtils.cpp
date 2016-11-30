//EmrUtils.cpp

//Implements functions related to Emr enums.

#include "stdafx.h"
#include <NxPracticeSharedLib/RichEditUtils.h>
#include "EmrUtils.h"
#include "DateTimeUtils.h"
#include "AuditTrail.h"
#include "EmrItemAdvImageDlg.h"
#include "Reports.h"
#include "ReportInfo.h"
#include "GlobalReportUtils.h"
#include "MergeEngine.h"
#include "LetterWriting.h"
#include "EmrItemAdvTableDlg.h"
#include "EMNDetail.h"
#include "MultiSelectDlg.h"
#include "EMRTopic.h"
#include "EmrTreeWnd.h"
#include "EmrTopicWnd.h"
#include "DateTimeUtils.h"
#include "InternationalUtils.h"
#include "ShowProgressFeedbackDlg.h"
#include "EMNUnspawner.h"
#include "EmrProblemDeleteDlg.h"
#include "TodoUtils.h"
#include "DontShowDlg.h"
#include "SureScriptsPractice.h"
#include "WellnessDataUtils.h"
#include "LabRequisitionDlg.h"
#include "EmrItemAdvNarrativeDlg.h"
#include "NxCDO.h"
#include "FaxSendDlg.h"
#include "FaxChooseRecipientDlg.h"
#include "GlobalStringUtils.h"
#include "DecisionRuleUtils.h"
#include "NxMD5.h"
#include "Base64.h"
#include "FileUtils.h"
#include "Mirror.h"
#include "boost/make_shared.hpp"
#include "boost/ref.hpp"
#include "EmrActionDlg.h"
#include "ImageArray.h"
#include "PicContainerDlg.h"
#include "SignatureDlg.h"
#include "SElectDlg.h"
#include "EMRTemplateFrameWnd.h"
#include "ASDDlg.h"
#include <NxXMLUtils.h>
#include "EmrPreviewPopupDlg.h"
#include "NxAPIManager.h"
#include "NxDataUtilitiesLib\SafeArrayUtils.h"
#include "FirstDataBankUtils.h"
#include "EMNMedication.h"	// (j.jones 2012-11-30 14:03) - PLID 53966 - moved the EMNMedication class to its own file
#include "EmrCodingGroupManager.h"
#include "EMRItemAdvMultiPopupDlg.h"
#include "EMR.h"
#include "WriteTokenInfo.h"
#include "NxImageCache.h"
#include "NxAutoQuantum.h"
#include "EMN.h"
#include "DiagSearchUtils.h" // (b.savon 2014-03-03 13:39) - PLID 61065 - Refactored
#include "NexGEM.h" // (a.walling 2014-03-12 12:31) - PLID 61334 - #import of API in stdafx causes crash in cl.exe
#include <CancerCaseDocument.h>
#include "CCDInterface.h"
#include <HistoryUtils.h>
#include "EmrItemAdvImageState.h"	// (j.armen 2014-07-21 16:32) - PLID 62836
#include "pugixml/pugixml.hpp"

// (a.walling 2011-08-11 16:43) - PLID 45021 - TableRow.m_pID is now TableRow.m_ID, which is not allocated on the heap.

// (a.walling 2010-01-21 16:43) - PLID 37022 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

#include "NexGEM.h" // (a.walling 2014-03-12 12:31) - PLID 61334 - #import of API in stdafx causes crash in cl.exe
#include <CancerCaseDocument.h>
#include "CCDInterface.h"
#include <HistoryUtils.h>
#include "EmrItemAdvImageState.h"	// (j.armen 2014-07-21 16:32) - PLID 62836
#include "PatientView.h"
#include "AdminView.h"

// (a.walling 2011-08-11 16:43) - PLID 45021 - TableRow.m_pID is now TableRow.m_ID, which is not allocated on the heap.

// (a.walling 2010-01-21 16:43) - PLID 37022 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

// (a.walling 2009-10-13 10:01) - PLID 35930
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



// (a.walling 2010-07-30 10:00) - No longer necessary -- was only for VS6 and those with outdated SDKs.
//#ifndef DBINITCONSTANTS
//// (a.walling 2008-05-30 10:52) - PLID 22049 - Define this interface
//extern const GUID IID_ISQLServerErrorInfo	= {0x5cf4ca12,0xef21,0x11d0,{0x97,0xe7,0x0,0xc0,0x4f,0xc2,0xad,0x98}};
//#endif

using namespace ADODB;

// (a.walling 2010-03-09 14:25) - PLID 37640 - Moved to cpp
CEmrActionArray::~CEmrActionArray()
{
	RemoveAllAndFreeEntries();
}

void CEmrActionArray::RemoveAllAndFreeEntries()
{
	long nCount = GetSize();
	for (long i=0; i<nCount; i++) {
		delete GetAt(i);
	}
	CArray<CEmrAction *, CEmrAction *>::RemoveAll();
}

void CEmrActionArray::AppendCopy(const CEmrActionArray &aryeiaCopyFrom)
{
	for (long i=0; i<aryeiaCopyFrom.GetSize(); i++) {
		Add(new CEmrAction(*aryeiaCopyFrom.GetAt(i)));
	}
}

long CEmrActionArray::Find(long nDestType, long nDestID, long nSortOrder, bool bPopup) const
{
	for (long i=0; i<GetSize(); i++) {
		CEmrAction *p = GetAt(i);
		if (p->m_nDestType == nDestType && p->m_nDestID == nDestID && p->m_nSortOrder == nSortOrder && p->m_bPopup == bPopup) {
			return i;
		}
	}
	return -1;
}

long CEmrActionArray::Find(const CEmrAction &ActionToFind) const
{
	return Find(ActionToFind.m_nDestType, ActionToFind.m_nDestID, ActionToFind.m_nSortOrder, ActionToFind.m_bPopup);
}

// (j.jones 2012-11-27 09:54) - PLID 53144 - moved these functions to the .cpp, from the .h
	
// (j.jones 2012-11-26 15:24) - PLID 53144 - added bStampDefaultsChanged
// (j.gruber 2013-09-30 12:19) - PLID 58675 - added groupID and codes
// (j.gruber 2014-07-22 15:38) - PLID 62627 - keyword columns
CEmrTableDropDownItem::CEmrTableDropDownItem(const CEmrTableDropDownItem &f)
: nID_(f.nID_), strData(f.strData), nSortOrder(f.nSortOrder), bInactive(f.bInactive), nGlassesOrderDataID(f.nGlassesOrderDataID), strGlassesOrderDataName(f.strGlassesOrderDataName), bStampFilterChanged(f.bStampFilterChanged), bStampDefaultsChanged(f.bStampDefaultsChanged), nDropDownGroupID(f.nDropDownGroupID), bCodesChanged(f.bCodesChanged), bUseKeyword(f.bUseKeyword), strKeywordOverride(f.strKeywordOverride)
{
	// (z.manning 2009-02-10 17:02) - PLID 33029 - Need to handle actions here
	aryActions.RemoveAll();
	aryActions.Append(f.aryActions);

	// (a.walling 2014-06-30 10:21) - PLID 62497 - Cleanup copy here using operator=

	aryStampFilter = f.aryStampFilter;

	// (j.jones 2012-11-26 15:18) - PLID 53144 - broke this up into two arrays
	aryStampDefaults = f.aryStampDefaults;

	// (j.gruber 2013-09-30 12:21) - PLID 58675 - codes
	aryCodes = f.aryCodes;
}

// (z.manning 2009-02-12 10:42) - PLID 33029 - The dropdown ID increment is now a required paramater.
// I alos added an action ID increment since dropdown items can now have actions.
CString CEmrTableDropDownItem::GenerateXml(long nIDIncrement, IN OUT long &nActionIDIncrement) const
{
	// (z.manning 2011-09-29 11:01) - PLID 45742 - Added XML to save stamp filter data
	CString strStampFilterXml;
	// (a.walling 2014-06-30 10:21) - PLID 62497
	for(auto dropdownStampInfo : aryStampFilter)
	{
		strStampFilterXml += FormatString("<SF StampID=\"%li\" />\r\n", dropdownStampInfo.nStampID);
	}

	// (j.jones 2012-11-26 15:16) - PLID 53144 - broke out stamp defaults into their own table
	CString strStampDefaultsXml;
	// (a.walling 2014-06-30 10:21) - PLID 62497
	for (auto dropdownStampInfo : aryStampDefaults)
	{
		strStampDefaultsXml += FormatString("<SD StampID=\"%li\" />\r\n", dropdownStampInfo.nStampID);
	}

	// (j.gruber 2013-09-30 12:13) - PLID 58675 - Codes
	CString strCodeXml;
	strCodeXml = aryCodes.GenerateXML("CD");
	


	CString strActionXml = GenerateActionXml(&aryActions, nActionIDIncrement);
	CString strGlassesOrderDataIDXml = (nGlassesOrderDataID==-1?"":FormatString(" GlassesOrderDataID=\"%li\"",nGlassesOrderDataID));
	// (j.gruber 2014-07-22 12:52) - PLID 62627 - added Keyword fields	
	return FormatString(
		"<T TableArbVal=\"%li\" ID=\"%li\" Data=\"%s\" Inactive=\"%li\" SortOrder=\"%li\"%s UseKeyword=\"%li\" KeywordOverride=\"%s\">\r\n%s\r\n%s\r\n%s\r\n%s</T>\r\n",
		nIDIncrement, nID, ConvertToQuotableXMLString(strData), bInactive ? 1 : 0, nSortOrder, strGlassesOrderDataIDXml, 
		bUseKeyword ? 1 : 0, ConvertToQuotableXMLString(strKeywordOverride),
		strActionXml, strStampFilterXml, strStampDefaultsXml, strCodeXml);
}

CString CEmrTableDropDownItemArray::GenerateXml() const
{
	long nActionIDIncrement = 1;
	long nIDIncrement = 0;
	CString strAns;
	// (a.walling 2014-06-30 10:21) - PLID 62497
	for (auto& item : m_data) {
		// (z.manning 2009-02-12 10:41) - PLID 33029 - Need to pass in an action ID increment value
		// now that dropdown items can have actions.
		strAns += item->GenerateXml(nIDIncrement, nActionIDIncrement);
		++nIDIncrement;
	}
	return strAns;
}

namespace Emr
{
	// (a.walling 2014-08-04 09:39) - PLID 62683 - Serialize to xml
	std::string ActionFilter::ToXml() const
	{
		using namespace pugi;

		if (anatomicLocationFilters.empty()) {
			return{};
		}

		xml_document doc;
		auto r = doc.append_child("filter");

		long lastLocationID = -1;
		xml_node location;
		for (const auto& detail : anatomicLocationFilters) {
			if (!location || lastLocationID != detail.anatomicLocationID) {
				lastLocationID = detail.anatomicLocationID;
				location = r.append_child("location");
				location.append_attribute("id") = detail.anatomicLocationID;
}

			if (detail.qualifierID == -1) {
				// any qualifier, is there a side?
				// (r.gonet 2015-02-12 11:03) - PLID 64859 - Compare against 0 rather than -1 since 0 means sideNull. -1 was the old sentinel value when the column was nullable.
				if (detail.anatomySide != 0) {
					location.append_attribute("anyQualifierSide") = detail.anatomySide;
				}
			} 
			else if (detail.qualifierID == 0) {
				// explicit no qualifier, is there a side?
				// (r.gonet 2015-02-12 11:03) - PLID 64859 - Compare against 0 rather than -1 since 0 means sideNull. -1 was the old sentinel value when the column was nullable.
				if (detail.anatomySide != 0) {
					location.append_attribute("noQualifierSide") = detail.anatomySide;
				}
			}
			else {
				auto q = location.append_child("qualifier");
				q.append_attribute("id") = detail.qualifierID;
				// (r.gonet 2015-02-12 11:03) - PLID 64859 - Compare against 0 rather than -1 since 0 means sideNull. -1 was the old sentinel value when the column was nullable.
				if (detail.anatomySide != 0) {
					q.append_attribute("side") = detail.anatomySide;
				}
			}
		}
		
		std::ostringstream o;
		r.print(o, "", format_raw);

		auto s = o.str();

		return s;
	}

	// (a.walling 2014-08-04 09:39) - PLID 62683 - Serialize from xml
	ActionFilter ActionFilter::FromXml(const char* sz)
	{
		using namespace pugi;

		ActionFilter filter;

		xml_document doc;
		auto result = doc.load(sz);

		if (result) {
			if (auto r = doc.child("filter")) {
				AnatomicLocationFilter detail;				
				for (auto location : r.children("location")) {
					detail.anatomicLocationID = location.attribute("id").as_int();

					// (r.gonet 2015-02-13 10:08) - PLID 64859 - This anatomic location is selected
					// in the filter. Make sure we add at least one filter for it.
					bool bAddedAtLeastOneFilter = false;
					if (auto anyQ = location.attribute("anyQualifierSide")) {
						detail.qualifierID = -1;
						detail.anatomySide = anyQ.as_int();
						filter.anatomicLocationFilters.insert(detail);
						// (r.gonet 2015-02-13 10:08) - PLID 64859
						bAddedAtLeastOneFilter = true;
					}

					if (auto noQ = location.attribute("noQualifierSide")) {
						detail.qualifierID = 0;
						detail.anatomySide = noQ.as_int();
						filter.anatomicLocationFilters.insert(detail);
						// (r.gonet 2015-02-13 10:08) - PLID 64859
						bAddedAtLeastOneFilter = true;
					}

					for (auto qualifier : location.children("qualifier")) {
						detail.qualifierID = qualifier.attribute("id").as_int();
						detail.anatomySide = qualifier.attribute("side").as_int();
						ASSERT(detail.qualifierID > 0);
						filter.anatomicLocationFilters.insert(detail);
						// (r.gonet 2015-02-13 10:08) - PLID 64859
						bAddedAtLeastOneFilter = true;
					}

					if (!bAddedAtLeastOneFilter) {
						// (r.gonet 2015-02-13 10:08) - PLID 64859 - Locations without any qualifiers are location filters that have not had a qualifier setup yet, such as from an
						// anatomic location filters prior to the laterality mod being run. This case is treated as <Any Qualifier> sideAny. This anatomic location is selected, even 
						// with no qualifiers defined, and should be filtered upon.
						detail.qualifierID = -1;
						detail.anatomySide = 0;
						filter.anatomicLocationFilters.insert(detail);
					} else {
						// (r.gonet 2015-02-13 10:08) - PLID 64859 - The anatomic location filter already has qualifiers setup for it.
						// and at least one filter has been added for it.
					}
				}
			}
		}
		else if (result.status != status_no_document_element) {
			ThrowNxException(__FUNCTION__" parse error %s for `%s`", result.description(), sz);
		}

		return filter;
	}

	long ActionFilter::GetAnyQualifierSide(long anatomicLocationID) const
	{
		auto range = anatomicLocationFilters.equal_range({ anatomicLocationID, -1 }); 
		if (range.first != range.second) {
			return range.first->anatomySide;
		}
		return Emr::sideNull;
	}

	long ActionFilter::GetNoQualifierSide(long anatomicLocationID) const
	{
		auto range = anatomicLocationFilters.equal_range({ anatomicLocationID, 0 });
		if (range.first != range.second) {
			return range.first->anatomySide;
		}
		return Emr::sideNull;
	}
	
	// (a.walling 2014-08-08 09:56) - PLID 62685 - Returns map of qualifier names
	AnatomyQualifierMap MakeAnatomyQualifiersMap()
	{
		AnatomyQualifierMap qualifiers;
		
		qualifiers.emplace(-1, "<any qualifier>");
		qualifiers.emplace(0, "<no qualifier>");
		for (ADODB::_RecordsetPtr prs = CreateRecordsetStd("SELECT ID, Name FROM AnatomyQualifiersT"); !prs->eof; prs->MoveNext()) {
			qualifiers.emplace(AdoFldLong(prs, "ID"), AdoFldString(prs, "Name"));
		}

		return qualifiers;
	}
	
	// Convert AnatomySide to WhichAnatomySide; the latter has sideNone as 4 so it can be combined bitwise with Left, Right
	WhichAnatomySide ToWhichAnatomySide(AnatomySide as)
	{
		if (as == asNone) {
			return sideNone;
		}
		else {
			return static_cast<WhichAnatomySide>(as);
		}
	}

	CString DescribeWhichAnatomySide(long side)
	{
		CString str;

		if ( (side & sideLeft) && (side & sideRight) ) {
			str = "bilateral";
		}
		else if (side & sideLeft) {
			str = "left";
		}
		else if (side & sideRight) {
			str = "right";
		}

		if (side & sideNone) {
			if (!str.IsEmpty()) {
				str += " and ";
			}
			str += "no side";
		}

		return str;
	}

	// (a.walling 2014-08-08 09:56) - PLID 62685 - Laterality - Side / Qualifier text description generation
	CString DescribeFilter(const ActionFilter& filter, const AnatomyQualifierMap& qualifierMap, long anatomicLocationID)
	{		
		struct QualifierStatement
		{
			CString name;
			CString sides;
		};

		std::vector<QualifierStatement> statements;

		// (a.walling 2014-09-11 15:20) - PLID 62685 - We now describe any qualifier / any side the same as any other detail

		auto range = filter.equal_range(anatomicLocationID);
		for (auto it = range.first; it != range.second; ++it)
		{
			CString name;
			auto q = qualifierMap.find(it->qualifierID);
			if (q == qualifierMap.end()) {
				ASSERT(FALSE);
				name.Format("qualifier%li", it->qualifierID);
			}
			else {
				name = q->second;
			}
			statements.push_back({ name, DescribeWhichAnatomySide(it->anatomySide) });
		}

		CString desc;

		sort(
			begin(statements)
			, end(statements)
			, [](const QualifierStatement& l, const QualifierStatement& r){ 
				return l.name.CompareNoCase(r.name) < 0; 
			}
		);

		for (const auto& statement : statements) {
			desc.AppendFormat("%s %s, ", statement.sides, statement.name);
		}

		desc.TrimRight(", ");

		return desc;
	}
}

// (j.jones 2008-07-18 17:31) - PLID 30779 - added problem class
// (a.walling 2009-05-04 09:49) - PLID 28495 - Diag code
// (a.walling 2009-05-04 09:50) - PLID 33751 - Chronicity
// (b.spivey, October 22, 2013) - PLID 58677 - added codeID

CEmrProblem::CEmrProblem()
{
	COleDateTime dtInvalid;
	dtInvalid.SetStatus(COleDateTime::invalid);
	m_nID = -1;
	m_nPatientID = -1;
	m_dtEnteredDate = dtInvalid;
	m_dtModifiedDate = dtInvalid;
	m_dtOnsetDate = dtInvalid;
	m_nStatusID = -1;
	m_nDiagICD9CodeID = -1;
	m_nDiagICD10CodeID = -1;
	m_nChronicityID = -1;
	m_nCodeID = -1;
	// (s.tullis 2015-02-23 15:44) - PLID 64723
	m_bDoNotShowOnCCDA = FALSE;
	// (r.gonet 2015-03-09 18:21) - PLID 65008 - Initialize DoNotShowOnProblemPrompt
	m_bDoNotShowOnProblemPrompt = FALSE;
	m_bIsModified = FALSE;
	m_bIsDeleted = FALSE;
	m_nEmrProblemActionID = -1;
	// (c.haag 2009-05-16 11:24) - PLID 34277 - Reference counting
	m_nRefCnt = 1;

	//for auditing
	m_strLastDescription = "";
	m_nLastStatusID = -1;
	m_dtLastOnsetDate = dtInvalid;
	m_nLastDiagICD9CodeID = -1;
	m_nLastDiagICD10CodeID = -1;
	m_nLastChronicityID = -1;
	m_nLastCodeID = -1; 
	// (s.tullis 2015-02-23 15:44) - PLID 64723
	m_bLastDoNotShowOnCCDA = FALSE;
	// (r.gonet 2015-03-09 18:21) - PLID 65008 - Initialize the old value of DoNotShowOnProblemPrompt
	m_bLastDoNotShowOnProblemPrompt = FALSE;
}


// (a.walling 2009-05-04 09:49) - PLID 28495 - Diag code
// (a.walling 2009-05-04 09:50) - PLID 33751 - Chronicity
// (z.manning 2009-05-27 10:13) - PLID 34297 - Added patient ID
// (b.spivey, October 22, 2013) - PLID 58677 - Add codeID
// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs
// (s.tullis 2015-02-23 15:44) - PLID 64723
// (r.gonet 2015-03-09 18:21) - PLID 65008 - Added bDoNotShowOnProblemPrompt. True to have the problem show in the prompt when we switch patients
// or go to the EMR tab. False to not show in the prompt.
CEmrProblem::CEmrProblem(long nID, long nPatientID, CString strDescription, COleDateTime dtEnteredDate, COleDateTime dtModifiedDate, COleDateTime dtOnsetDate,
		long nStatusID, long nDiagCodeID_ICD9, long nDiagCodeID_ICD10, long nChronicityID, BOOL bIsModified, long nCodeID, BOOL bDoNotShowOnCCDA,
		BOOL bDoNotShowOnProblemPrompt)
{
	m_nID = nID;
	m_nPatientID = nPatientID;
	m_strDescription = strDescription;
	m_dtEnteredDate = dtEnteredDate;
	m_dtModifiedDate = dtModifiedDate;
	m_dtOnsetDate = dtOnsetDate;
	m_nStatusID = nStatusID;
	m_nDiagICD9CodeID = nDiagCodeID_ICD9;
	m_nDiagICD10CodeID = nDiagCodeID_ICD10;
	m_nChronicityID = nChronicityID;
	m_nCodeID = nCodeID; 
	// (s.tullis 2015-03-13 09:06) - PLID 64723 - Initialize do not show on ccda
	m_bDoNotShowOnCCDA = bDoNotShowOnCCDA;
	// (r.gonet 2015-03-09 18:21) - PLID 65008 - Initialize bDoNotShowOnProblemPrompt.
	m_bDoNotShowOnProblemPrompt = bDoNotShowOnProblemPrompt;
	m_bIsModified = bIsModified;
	m_bIsDeleted = FALSE;
	m_nEmrProblemActionID = -1;
	// (c.haag 2009-05-16 11:24) - PLID 34277 - Reference counting
	m_nRefCnt = 1;

	//for auditing
	m_strLastDescription = m_strDescription;
	m_nLastStatusID = m_nStatusID;
	m_dtLastOnsetDate = m_dtOnsetDate;
	m_nLastDiagICD9CodeID = m_nDiagICD9CodeID;
	m_nLastDiagICD10CodeID = m_nDiagICD10CodeID;
	m_nLastChronicityID = m_nChronicityID;
	
	m_nLastCodeID = nCodeID;
	// (s.tullis 2015-03-13 09:06) - PLID 64723 - Remember for auditing
	m_bLastDoNotShowOnCCDA = bDoNotShowOnCCDA;
	// (r.gonet 2015-03-09 18:21) - PLID 65008 - Remember the old value of the DoNotShowOnProblemPrompt field.
	m_bLastDoNotShowOnProblemPrompt = bDoNotShowOnProblemPrompt;
}

// (a.walling 2009-05-04 09:49) - PLID 28495 - Diag code
// (a.walling 2009-05-04 09:50) - PLID 33751 - Chronicity
// (b.spivey, October 22, 2013) - PLID 58677 - Added codeID
// (s.tullis 2015-02-23 15:44) - PLID 64723
CEmrProblem::CEmrProblem(CEmrProblem *epSource)
{
	m_nID = epSource->m_nID;
	m_nPatientID = epSource->m_nPatientID;
	m_strDescription = epSource->m_strDescription;
	m_dtEnteredDate = epSource->m_dtEnteredDate;
	m_dtModifiedDate = epSource->m_dtModifiedDate;
	m_dtOnsetDate = epSource->m_dtOnsetDate;
	m_nStatusID = epSource->m_nStatusID;
	m_nDiagICD9CodeID = epSource->m_nDiagICD9CodeID;
	m_nDiagICD10CodeID = epSource->m_nDiagICD10CodeID;
	m_nChronicityID = epSource->m_nChronicityID;
	m_nCodeID = epSource->m_nCodeID; 
	m_bDoNotShowOnCCDA = epSource->m_bDoNotShowOnCCDA;
	// (r.gonet 2015-03-09 18:21) - PLID 65008 - Initialize bDoNotShowOnProblemPrompt.
	m_bDoNotShowOnProblemPrompt = epSource->m_bDoNotShowOnProblemPrompt;
	m_bIsModified = epSource->m_bIsModified;
	m_bIsDeleted = epSource->m_bIsDeleted;
	m_nEmrProblemActionID = epSource->m_nEmrProblemActionID;
	// (c.haag 2009-05-16 11:24) - PLID 34277 - Reference counting
	m_nRefCnt = 1;

	//for auditing
	m_strLastDescription = epSource->m_strLastDescription;
	m_nLastStatusID = epSource->m_nLastStatusID;
	m_dtLastOnsetDate = epSource->m_dtLastOnsetDate;
	m_nLastDiagICD9CodeID = epSource->m_nLastDiagICD9CodeID;
	m_nLastDiagICD10CodeID = epSource->m_nLastDiagICD10CodeID;
	m_nLastCodeID = epSource->m_nLastCodeID;
	m_bLastDoNotShowOnCCDA = epSource->m_bLastDoNotShowOnCCDA;
	// (r.gonet 2015-03-09 18:21) - PLID 65008 - Remember the old value of the DoNotShowOnProblemPrompt field.
	m_bLastDoNotShowOnProblemPrompt = epSource->m_bLastDoNotShowOnProblemPrompt;
}

// (c.haag 2009-05-20 11:23) - PLID 34277 - Overload where data is gathered
// from a fields object
// (b.spivey, October 22, 2013) - PLID 58677 - Added codeID
CEmrProblem::CEmrProblem(ADODB::FieldsPtr& f)
{
	m_nID = AdoFldLong(f, "ID");
	ReloadFromData(f);
	m_bIsModified = FALSE;
	m_bIsDeleted = FALSE;	
	// (c.haag 2009-05-16 11:24) - PLID 34277 - Reference counting
	m_nRefCnt = 1;
	
	//for auditing
	m_strLastDescription = m_strDescription;
	m_nLastStatusID = m_nStatusID;
	m_dtLastOnsetDate = m_dtOnsetDate;
	// (j.jones 2014-02-24 15:28) - PLID 61010 - EMR problems now have
	// an ICD-9 code and/or an ICD-10 code, no more than one pair.
	m_nLastDiagICD9CodeID = m_nDiagICD9CodeID;
	m_nLastDiagICD10CodeID = m_nDiagICD10CodeID;
	m_nLastChronicityID = m_nChronicityID;
	m_nLastCodeID = m_nCodeID; 
	// (s.tullis 2015-02-23 15:44) - PLID 64723
	m_bLastDoNotShowOnCCDA = m_bDoNotShowOnCCDA;
	// (r.gonet 2015-03-09 18:21) - PLID 65008 - Remember the old value of DoNotShowOnProblemPrompt.
	m_bLastDoNotShowOnProblemPrompt = m_bDoNotShowOnProblemPrompt;
}

// (c.haag 2009-05-20 11:17) - PLID 34277 - Load from a recordset fields object
void CEmrProblem::ReloadFromData(ADODB::FieldsPtr& f)
{
	COleDateTime dtInvalid;
	dtInvalid.SetStatus(COleDateTime::invalid);
	m_strDescription = AdoFldString(f, "Description");
	m_dtEnteredDate = AdoFldDateTime(f, "EnteredDate");
	m_dtModifiedDate = AdoFldDateTime(f, "ModifiedDate");
	m_dtOnsetDate = AdoFldDateTime(f, "OnsetDate", dtInvalid);
	m_nStatusID = AdoFldLong(f, "StatusID");
	// (a.walling 2009-05-04 09:49) - PLID 28495 - Diag code	
	// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs
	m_nDiagICD9CodeID = AdoFldLong(f, "DiagCodeID", -1);
	m_nDiagICD10CodeID = AdoFldLong(f, "DiagCodeID_ICD10", -1);	
	// (a.walling 2009-05-04 09:50) - PLID 33751 - Chronicity
	m_nChronicityID = AdoFldLong(f, "ChronicityID", -1);
	// (c.haag 2009-05-22 16:40) - PLID 34277 - We no longer pull these fields as
	// they now belong to problem links
	//m_eprtTypeID = (EMRProblemRegardingTypes)AdoFldLong(f, "EmrRegardingType");
	//m_nEMRRegardingID = AdoFldLong(f, "EmrRegardingID");
	//m_nEMRDataID = AdoFldLong(f, "EMRDataID", -1);
	m_nEmrProblemActionID = AdoFldLong(f, "EmrProblemActionID", -1);
	// (z.manning 2009-05-27 10:20) - PLID 34297 - Added patient ID
	m_nPatientID = AdoFldLong(f, "PatientID", -1);
	// (b.spivey, October 22, 2013) - PLID 58677 - Set codeID
	m_nCodeID = AdoFldLong(f, "CodeID", -1);
	// (s.tullis 2015-02-23 15:44) - PLID 64723 
	m_bDoNotShowOnCCDA = AdoFldBool(f, "DoNotShowOnCCDA", FALSE);
	// (r.gonet 2015-03-09 18:21) - PLID 65008 - Load DoNotShowOnProblemPrompt from the recordset.
	m_bDoNotShowOnProblemPrompt = AdoFldBool(f, "DoNotShowOnProblemPrompt", FALSE);
}

// (z.manning 2009-05-27 17:19) - PLID 34340 - Handy function for auditing newly created problems.
void CEmrProblem::AuditNew(long &nAuditTransactionID)
{
	if(nAuditTransactionID == -1) {
		nAuditTransactionID = BeginAuditTransaction();
	}
	CString strPatientName = GetExistingPatientName(m_nPatientID);
	AuditEvent(m_nPatientID, strPatientName,nAuditTransactionID,aeiEMNProblemDesc, m_nID, "", m_strDescription, aepMedium,aetCreated);
	AuditEvent(m_nPatientID, strPatientName,nAuditTransactionID,aeiEMNProblemStatus, m_nID, "",VarString(GetTableField("EMRProblemStatusT", "Name", "ID", m_nStatusID), ""),aepMedium,aetCreated);
	// (a.walling 2009-05-04 13:03) - PLID 28495
	// (j.jones 2014-02-24 15:28) - PLID 61010 - EMR problems now have
	// an ICD-9 code and/or an ICD-10 code, no more than one pair.
	if (m_nDiagICD9CodeID != -1 || m_nDiagICD10CodeID != -1) {

		CString strDiagCode9, strDiagCode10;
		GetICD9And10Codes(m_nDiagICD9CodeID, m_nDiagICD10CodeID, strDiagCode9, strDiagCode10);

		CString strOldDiagCodeAudit, strNewDiagCodeAudit;
		GetProblemICD910AuditDescriptions("", "", strDiagCode9, strDiagCode10, strOldDiagCodeAudit, strNewDiagCodeAudit);					
		//don't fill the old value here, even though the function gave us one
		AuditEvent(m_nPatientID, strPatientName, nAuditTransactionID, aeiEMNProblemDiagCode, m_nID, "", strNewDiagCodeAudit, aepMedium, aetCreated);
	}
	// (a.walling 2009-05-04 13:03) - PLID 33751
	if (m_nChronicityID != -1) {
		AuditEvent(m_nPatientID, strPatientName,nAuditTransactionID,aeiEMNProblemChronicity, m_nID, "",VarString(GetTableField("EMRProblemChronicityT", "Name", "ID", m_nChronicityID), ""),aepMedium,aetCreated);
	}
	// (b.spivey, October 22, 2013) - PLID 58677 - Set codeID
	if (m_nCodeID != -1) {
		AuditEvent(m_nPatientID, strPatientName,nAuditTransactionID,aeiEMNProblemSNOMEDCode, m_nID, "", VarString(GetTableField("CodesT", "Code", "ID", m_nCodeID), ""),aepMedium,aetCreated);
	}
	// (s.tullis 2015-02-23 15:44) - PLID 64723 - audit whether checked or not checked
	AuditEvent(m_nPatientID, strPatientName, nAuditTransactionID, aeiEMNProblemDoNotShowOnCCDA, m_nID, "", m_bDoNotShowOnCCDA ? "Checked":"Unchecked", aepMedium, aetCreated);
	
	// (r.gonet 2015-03-06 10:16) - PLID 65008 - Audit the EMR Problem's Do Not Show On Problem Prompt flag. Always audit the value, even if not checked.
	AuditEvent(m_nPatientID, strPatientName, nAuditTransactionID, aeiEMNProblemDoNotShowOnProblemPrompt, m_nID, "", m_bDoNotShowOnProblemPrompt ? "Checked" : "Unchecked", aepMedium, aetCreated);
}

// (c.haag 2016-06-09 14:54) - PLID-66502 - Write pertinent information about this EMR object to NxLog. This is used to help pin down save errors.
void CEmrProblem::LogEmrObjectData(int nIndent)
{
	// Log this object
	::LogEmrObjectData(nIndent, m_nID, this, esotProblem, (m_nID == -1), m_bIsModified, m_bIsDeleted, m_strDescription,
		"m_nStatusID = %d"
		, m_nStatusID);
}

// (c.haag 2009-07-09 10:57) - PLID 34829 - Now optionally pass in an EMR object that
// is to be the owner of this newly allocated problem. We must ensure that the problem
// assigned to this link has the same EMR owner.
CEmrProblemLink::CEmrProblemLink(const CEmrProblemLink* src, CEMR* pOwningEMR /*= NULL */)
{
	m_pProblem = NULL;
	*this = *src;
	if (NULL != m_pProblem && NULL != pOwningEMR && NULL != src->GetEMR() && pOwningEMR != src->GetEMR())
	{
		// (c.haag 2009-07-09 11:03) - PLID 34829 - If we get here, it means this link and the source link
		// belong to two different EMN's. We will need to allocate the problem using the owning EMR, and
		// then assign it to this object.
		CEmrProblem* pOwningProblem = pOwningEMR->AllocateEmrProblem(src->GetProblem());
		CEmrProblem* pOldProblem = m_pProblem;
		m_pProblem->Release();
		m_pProblem = pOwningProblem;
	}
}

//these functions attempt to populate the pointers of the problem
// (c.haag 2009-05-16 11:42) - PLID 34277 - All UpdatePointer functions now correspond
// with EMR problem links.
void CEmrProblemLink::UpdatePointersWithEMR(CEMR *pEMR) {

	m_pEMR = pEMR;
}

void CEmrProblemLink::UpdatePointersWithEMN(CEMN *pEMN) {

	m_pEMN = pEMN;

	//now try to update the EMR
	if(m_pEMN != NULL && m_pEMN->GetParentEMR() != NULL) {
		UpdatePointersWithEMR(m_pEMN->GetParentEMR());
	}
}

void CEmrProblemLink::UpdatePointersWithTopic(CEMRTopic *pTopic) {

	m_pTopic = pTopic;

	//now try to update the EMN
	if(m_pTopic != NULL && m_pTopic->GetParentEMN() != NULL) {
		UpdatePointersWithEMN(m_pTopic->GetParentEMN());
	}
}

void CEmrProblemLink::UpdatePointersWithDetail(CEMNDetail *pDetail) {

	m_pDetail = pDetail;

	//now try to update the topic
	if(m_pDetail != NULL && m_pDetail->m_pParentTopic != NULL) {
		UpdatePointersWithTopic(m_pDetail->m_pParentTopic);
	}
}

// (c.haag 2008-07-21 14:45) - PLID 30725
void CEmrProblemLink::UpdatePointersWithCharge(CEMN* pEMN, EMNCharge* pCharge)
{
	m_pCharge = pCharge;
	UpdatePointersWithEMN(pEMN);
}

// (c.haag 2008-07-21 14:45) - PLID 30725
void CEmrProblemLink::UpdatePointersWithDiagCode(CEMN* pEMN, EMNDiagCode* pDiagCode)
{
	m_pDiagCode = pDiagCode;
	UpdatePointersWithEMN(pEMN);
}

// (c.haag 2008-07-22 09:45) - PLID 30725
void CEmrProblemLink::UpdatePointersWithMedication(CEMN* pEMN, EMNMedication* pMedication)
{
	m_pMedication = pMedication;
	UpdatePointersWithEMN(pEMN);
}

// (z.manning 2009-05-27 10:00) - PLID 34297 - Moved this logic here from EMRProblemEditDlg.cpp
void CEmrProblemLink::GetDetailNameAndValue(OUT CString &strDetailName, OUT CString &strDetailValue)
{
	switch(m_eprtTypeID)
	{
		case eprtEmrItem:
		case eprtEmrDataItem:

			if(GetDetail()) {
				if(GetDetail()->GetStateVarType() == VT_NULL || GetDetail()->GetStateVarType() == VT_BSTR && VarString(GetDetail()->GetState()).IsEmpty()) {
					//TES 2/26/2010 - PLID 37463 - Check whether to use the "Smart Stamps" long form
					// (z.manning 2010-07-26 15:23) - PLID 39848 - All tables now use the same long form
					strDetailValue = GetDetail()->m_strLongForm;
				} else {
					CStringArray saDummy;
					// (c.haag 2006-11-14 10:49) - PLID 23543 - If the info type is an image, we may get a debug assertion
					// failure when calling GetSentence. Rather than try to get a sentence, just return a sentinel value.
					if (eitImage == GetDetail()->m_EMRInfoType) {
						strDetailValue = "<image>";
					} else {
						// (c.haag 2008-02-22 13:53) - PLID 29064 - GetSentence may access the database when doing calculations on
						// dropdown table columns. Make sure we pass in our connection object so it won't try to use the global one
						// which belongs to the main thread.
						strDetailValue = ::GetSentence(GetDetail(), NULL, false, false, saDummy, ecfParagraph, NULL, NULL, NULL);
					}
				}

				strDetailName = GetDetail()->GetMergeFieldOverride().IsEmpty() ? GetDetail()->GetLabelText() : GetDetail()->GetMergeFieldOverride();
			}
			break;

		case eprtEmrTopic:

			if(GetTopic()) {
				strDetailName = GetTopic()->GetName();
			}
			break;

		case eprtEmrEMN:

			if(GetEMN()) {
				strDetailName = GetEMN()->GetDescription();
			}
			break;

		case eprtEmrEMR:

			if(GetEMR()) {
				strDetailName = GetEMR()->GetDescription();
			}					
			break;

		case eprtEmrDiag: {

			// (j.jones 2014-03-24 09:42) - PLID 61505 - if a problem is linked to a diagnosis, show the 9 and 10 code if both exist
			EMNDiagCode *pCode = GetDiagCode();
			if(pCode) {
				if(pCode->nDiagCodeID != -1 && pCode->nDiagCodeID_ICD10 != -1) {
					strDetailName = pCode->strCode_ICD10 + " - " + pCode->strCodeDesc_ICD10 + " (" + pCode->strCode + " - " + pCode->strCodeDesc + ")";
				}
				else if(pCode->nDiagCodeID_ICD10 != -1) {
					strDetailName = pCode->strCode_ICD10 + " - " + pCode->strCodeDesc_ICD10;
				}
				else if(pCode->nDiagCodeID != -1) {
					strDetailName = pCode->strCode + " - " + pCode->strCodeDesc;
				}
				else {
					//this should be impossible
					ASSERT(FALSE);
					strDetailName = "<Unknown Diagnosis Code>";
				}
			}
			break;
		}

		case eprtEmrCharge:

			if(GetCharge()) {
				strDetailName = GetCharge()->strDescription;
			}
			break;

		case eprtEmrMedication:

			if(GetMedication()) {
				strDetailName = GetMedication()->m_strDrugName;
			}
			
			break;
		case eprtUnassigned:
			if(GetProblem() != NULL) {
				strDetailName = GetExistingPatientName(GetProblem()->m_nPatientID);
			}
			break;

		case eprtLab: // (z.manning 2009-05-26 15:07) - PLID 34345
			strDetailName = GetLabText();
			break;

		default:
			ASSERT(FALSE); // This type is not supported
			break;
	}
}

// (z.manning 2009-05-27 12:13) - PLID 34340
void CEmrProblemLink::Audit(const AuditEventItems aei, long &nAuditTransactionID, const CString &strPatientName)
{
	if(nAuditTransactionID == -1) {
		nAuditTransactionID = BeginAuditTransaction();
	}
	CString strNewValue, strDetailName, strDetailValue;
	GetDetailNameAndValue(strDetailName, strDetailValue);
	CString strOwnerType = GetProblemTypeDescription(GetType());
	CString strLinkType;
	if(aei == aeiEMNProblemLinkDeleted) {
		strLinkType = "Unlinked from";
	}
	else if(aei == aeiEMNProblemLinkCreated) {
		strLinkType = "Linked to";
	}
	else {
		ASSERT(FALSE);
	}
	strNewValue.Format("%s %s: %s%s%s", strLinkType, strOwnerType, strDetailName, strDetailValue.IsEmpty() ? "" : " - ", strDetailValue);
	ASSERT(GetProblem()->m_nID != -1);
	AuditEvent(GetProblem()->m_nPatientID, strPatientName, nAuditTransactionID, aei, GetProblem()->m_nID, GetProblem()->m_strDescription, strNewValue, aepHigh, aetChanged);
}

// (c.haag 2016-06-09 14:54) - PLID-66502 - Write pertinent information about this EMR object to NxLog. This is used to help pin down save errors.
void CEmrProblemLink::LogEmrObjectData(int nIndent)
{
	// Log this object
	::LogEmrObjectData(nIndent, m_nID, this, esotProblemLink, (m_nID == -1), FALSE /* Links are never modified */ , m_bIsDeleted, GetProblemTypeDescription(GetType()),
		"m_nEMRRegardingID = %d  m_nEMRDataID = %d"
		, m_nEMRRegardingID
		, m_nEMRDataID);
}

// (a.walling 2008-07-28 14:56) - PLID 30855 - Get the associated EMN query string
// (j.armen 2013-05-14 12:12) - PLID 56680 - Pass back a CSqlFragment instead
CSqlFragment CEmrProblem::GetRegardingEMNFormatQueryString(EMRProblemRegardingTypes nType, long nRegardingID)
{
	// (a.walling 2014-07-23 09:12) - PLID 63003 - Use CONST_INT for EMRProblemLinkT.EMRRegardingType enums
	switch(nType) {
	case eprtEmrDataItem:
	case eprtEmrItem:
		// (c.haag 2009-05-12 12:29) - PLID 34234 - Added support for new problem link structure
		return CSqlFragment(
			"SELECT\r\n"
			"	EMRMasterT.ID AS EmnID\r\n"
			"FROM EMRMasterT\r\n"
			"INNER JOIN EMRDetailsT ON EMRDetailsT.EmrID = EMRMasterT.ID\r\n"
			"LEFT JOIN EMRProblemLinkT ON EMRProblemLinkT.EmrRegardingID = EMRDetailsT.ID\r\n"
			"	AND EMRProblemLinkT.EmrRegardingType = {CONST_INT}\r\n"
			"LEFT JOIN EMRProblemsT ON EMRProblemsT.ID = EMRProblemLinkT.EMRProblemID\r\n"
			"WHERE EMRDetailsT.ID = {INT}", nType, nRegardingID);
		break;
	case eprtEmrTopic:
		// (c.haag 2009-05-12 12:29) - PLID 34234 - Added support for new problem link structure
		return CSqlFragment(
			"SELECT\r\n"
			"	EMRMasterT.ID AS EmnID\r\n"
			"FROM EMRMasterT\r\n"
			"INNER JOIN EMRTopicsT ON EMRTopicsT.EmrID = EMRMasterT.ID\r\n"
			"LEFT JOIN EMRProblemLinkT ON EMRProblemLinkT.EmrRegardingID = EMRTopicsT.ID\r\n"
			"	AND EMRProblemLinkT.EmrRegardingType = {CONST_INT}\r\n"
			"LEFT JOIN EMRProblemsT ON EMRProblemsT.ID = EMRProblemLinkT.EMRProblemID\r\n"
			"WHERE EMRTopicsT.ID = {INT}", nType, nRegardingID);
		break;
	case eprtEmrEMN: // this is dumb, but it's here for the sake of completeness
		// (c.haag 2009-05-12 12:29) - PLID 34234 - Added support for new problem link structure
		return CSqlFragment(
			"SELECT\r\n"
			"	EMRMasterT.ID AS EmnID\r\n"
			"FROM EMRMasterT\r\n"
			"LEFT JOIN EMRProblemLinkT ON EMRProblemLinkT.EmrRegardingID = EMRMasterT.ID\r\n"
			"	AND EMRProblemLinkT.EmrRegardingType = {CONST_INT}\r\n"
			"LEFT JOIN EMRProblemsT ON EMRProblemsT.ID = EMRProblemLinkT.EMRProblemID\r\n"
			"WHERE EMRMasterT.ID = {INT}", nType, nRegardingID);
		break;
	case eprtEmrDiag:
		// (c.haag 2009-05-12 12:29) - PLID 34234 - Added support for new problem link structure
		return CSqlFragment(
			"SELECT\r\n"
			"	EMRMasterT.ID AS EmnID\r\n"
			"FROM EMRMasterT\r\n"
			"INNER JOIN EMRDiagCodesT ON EMRDiagCodesT.EmrID = EMRMasterT.ID\r\n"
			"LEFT JOIN EMRProblemLinkT ON EMRProblemLinkT.EmrRegardingID = EMRDiagCodesT.ID\r\n"
			"	AND EMRProblemLinkT.EmrRegardingType = {CONST_INT}\r\n"
			"LEFT JOIN EMRProblemsT ON EMRProblemsT.ID = EMRProblemLinkT.EMRProblemID\r\n"
			"WHERE EMRDiagCodesT.ID = {INT}", nType, nRegardingID);
		break;
	case eprtEmrCharge:
		// (c.haag 2009-05-12 12:29) - PLID 34234 - Added support for new problem link structure
		return CSqlFragment(
			"SELECT\r\n"
			"	EMRMasterT.ID AS EmnID\r\n"
			"FROM EMRMasterT\r\n"
			"INNER JOIN EMRChargesT ON EMRChargesT.EmrID = EMRMasterT.ID\r\n"
			"LEFT JOIN EMRProblemLinkT ON EMRProblemLinkT.EmrRegardingID = EMRChargesT.ID\r\n"
			"	AND EMRProblemLinkT.EmrRegardingType = {CONST_INT}\r\n"
			"LEFT JOIN EMRProblemsT ON EMRProblemsT.ID = EMRProblemLinkT.EMRProblemID\r\n"
			"WHERE EMRChargesT.ID = {INT}", nType, nRegardingID);
		break;
	case eprtEmrMedication:
		// (c.haag 2009-05-12 12:29) - PLID 34234 - Added support for new problem link structure
		return CSqlFragment(
			"SELECT\r\n"
			"	EMRMasterT.ID AS EmnID\r\n"
			"FROM EMRMasterT\r\n"
			"INNER JOIN EmrMedicationsT ON EmrMedicationsT.EmrID = EMRMasterT.ID\r\n"
			"LEFT JOIN EMRProblemLinkT ON EMRProblemLinkT.EmrRegardingID = EmrMedicationsT.MedicationID\r\n"
			"	AND EMRProblemLinkT.EmrRegardingType = {CONST_INT}\r\n"
			"LEFT JOIN EMRProblemsT ON EMRProblemsT.ID = EMRProblemLinkT.EMRProblemID\r\n"
			"WHERE EmrMedicationsT.MedicationID = {INT}", nType, nRegardingID);
		break;

	case eprtInvalid:
	case eprtEmrEMR:
	case eprtUnassigned: // (c.haag 2008-12-05 09:42) - PLID 28496
	case eprtLab: // (z.manning 2009-05-26 10:26) - PLID 34340
		return CSqlFragment();
		break;

	default:
		ThrowNxException("Unknown problem type");
	}
}

CString CEmrProblem::GetEMNParamQueryString(EMRProblemRegardingTypes nType)
{
	CString strType;
	strType.Format("%li", nType);
	
	CString strQuery;
	switch(nType) {
	case eprtEmrItem:
	case eprtEmrDataItem:
		// (c.haag 2009-05-12 13:06) - PLID 34234 - Use new EMR problem link structure
		strQuery =
			"SELECT EMRMasterT.ID AS EmnID FROM EMRMasterT "
			"INNER JOIN EMRDetailsT ON EMRDetailsT.EmrID = EMRMasterT.ID "
			"LEFT JOIN EMRProblemLinkT ON EMRProblemLinkT.EmrRegardingID = EMRDetailsT.ID "
			"AND EMRProblemLinkT.EmrRegardingType = " + strType + " " +
			"LEFT JOIN EMRProblemsT ON EMRProblemsT.ID = EMRProblemLinkT.EMRProblemID "
			"WHERE EMRProblemsT.ID = {INT}";
		break;
	case eprtEmrTopic:
		// (c.haag 2009-05-12 13:06) - PLID 34234 - Use new EMR problem link structure
		strQuery =
			"SELECT EMRMasterT.ID AS EmnID FROM EMRMasterT "
			"INNER JOIN EMRTopicsT ON EMRTopicsT.EmrID = EMRMasterT.ID "
			"LEFT JOIN EMRProblemLinkT ON EMRProblemLinkT.EmrRegardingID = EMRTopicsT.ID "
			"AND EMRProblemLinkT.EmrRegardingType = " + strType + " " +
			"LEFT JOIN EMRProblemsT ON EMRProblemsT.ID = EMRProblemLinkT.EMRProblemID "
			"WHERE EMRProblemsT.ID = {INT}";
		break;
	case eprtEmrEMN:
		// (c.haag 2009-05-12 13:06) - PLID 34234 - Use new EMR problem link structure
		strQuery =
			"SELECT EMRMasterT.ID AS EmnID FROM EMRMasterT "
			"LEFT JOIN EMRProblemLinkT ON EMRProblemLinkT.EmrRegardingID = EMRMasterT.ID "
			"AND EMRProblemLinkT.EmrRegardingType = " + strType + " " +
			"LEFT JOIN EMRProblemsT ON EMRProblemsT.ID = EMRProblemLinkT.EMRProblemID "
			"WHERE EMRProblemsT.ID = {INT}";
		break;
	case eprtEmrDiag:
		// (c.haag 2009-05-12 13:06) - PLID 34234 - Use new EMR problem link structure
		strQuery =
			"SELECT EMRMasterT.ID AS EmnID FROM EMRMasterT "
			"INNER JOIN EMRDiagCodesT ON EMRDiagCodesT.EmrID = EMRMasterT.ID "
			"LEFT JOIN EMRProblemLinkT ON EMRProblemLinkT.EmrRegardingID = EMRDiagCodesT.ID "
			"AND EMRProblemLinkT.EmrRegardingType = " + strType + " " +
			"LEFT JOIN EMRProblemsT ON EMRProblemsT.ID = EMRProblemLinkT.EMRProblemID "
			"WHERE EMRProblemsT.ID = {INT}";
		break;
	case eprtEmrCharge:
		// (c.haag 2009-05-12 13:06) - PLID 34234 - Use new EMR problem link structure
		strQuery =
			"SELECT EMRMasterT.ID AS EmnID FROM EMRMasterT "
			"INNER JOIN EMRChargesT ON EMRChargesT.EmrID = EMRMasterT.ID "
			"LEFT JOIN EMRProblemLinkT ON EMRProblemLinkT.EmrRegardingID = EMRChargesT.ID "
			"AND EMRProblemLinkT.EmrRegardingType = " + strType + " " +
			"LEFT JOIN EMRProblemsT ON EMRProblemsT.ID = EMRProblemLinkT.EMRProblemID "
			"WHERE EMRProblemsT.ID = {INT}";
		break;
	case eprtEmrMedication:
		// (c.haag 2009-05-12 13:06) - PLID 34234 - Use new EMR problem link structure
		strQuery =
			"SELECT EMRMasterT.ID AS EmnID FROM EMRMasterT "
			"INNER JOIN EmrMedicationsT ON EmrMedicationsT.EmrID = EMRMasterT.ID "
			"LEFT JOIN EMRProblemLinkT ON EMRProblemLinkT.EmrRegardingID = EmrMedicationsT.MedicationID "
			"AND EMRProblemLinkT.EmrRegardingType = " + strType + " " +
			"LEFT JOIN EMRProblemsT ON EMRProblemsT.ID = EMRProblemLinkT.EMRProblemID "
			"WHERE EMRProblemsT.ID = {INT}";
		break;

	case eprtInvalid:
	case eprtEmrEMR:
	case eprtUnassigned: // (c.haag 2008-12-05 09:42) - PLID 28496
	case eprtLab: // (z.manning 2009-05-26 10:30) - PLID 34340
		strQuery.Empty();
		break;

	default:
		ThrowNxException("Unknown problem type");
	}

	return strQuery;
}

// (a.walling 2008-08-26 12:05) - PLID 30855 - Get the associated EMN from a problem ID\
// (z.manning 2009-07-01 17:32) - PLID 34765 - Option to include EMN IDs for EMR based problems
//	(will include all EMNs within that EMR)
// (j.armen 2013-05-14 12:13) - PLID 56680 - Pass back as a CSqlFragment
CSqlFragment CEmrProblem::GetEMNQueryFromProblemID(long nProblemID, BOOL bIncludeEmrProblems)
{
	CSqlFragment sqlEmrProblemCase;
	if(bIncludeEmrProblems) {
		// (z.manning 2009-07-01 17:57) - PLID 34765 - We want to include EMN IDs for EMR based problems so add the SQL to do so.
		//TES 10/12/2011 - PLID 45908 - This would fail if the EMR had more than one EMN in it.  I had to rewrite the whole query to allow for that.
		// (a.walling 2014-07-23 09:12) - PLID 63003 - Use CONST_INT for EMRProblemLinkT.EMRRegardingType enums
		sqlEmrProblemCase.Create("OR EmrMasterT.EmrGroupID IN "
			"(SELECT EmrRegardingID FROM EmrProblemLinkT WHERE EmrProblemLinkT.EmrProblemID = {INT} AND EmrProblemLinkT.EmrRegardingType = {CONST_INT})",
			nProblemID, eprtEmrEMR);
	}

	// (c.haag 2009-05-12 12:34) - PLID 34234 - The query now uses the new problem linking structure
	//TES 10/12/2011 - PLID 45908 - I rewrote this query so that it wouldn't fail if the problem had multiple EMNs (which it might, if it
	// was linked to an EMR).
	// (a.walling 2014-07-23 09:12) - PLID 63003 - Use CONST_INT for EMRProblemLinkT.EMRRegardingType enums
	return CSqlFragment(
		"SELECT\r\n"
		"	EmrMasterT.ID\r\n"
		"FROM EmrMasterT\r\n"
		"WHERE\r\n"
		"	EmrMasterT.ID IN (SELECT EmrDetailsT.EmrID\r\n"
		"		FROM EmrDetailsT INNER JOIN EmrProblemLinkT ON EmrDetailsT.ID = EmrProblemLinkT.EmrRegardingID\r\n"
		"		WHERE EmrProblemLinkT.EmrProblemID = {INT} AND EmrProblemLinkT.EmrRegardingType IN ({CONST_INT},{CONST_INT}))\r\n"
		"	OR EmrMasterT.ID IN (SELECT EmrTopicsT.EmrID\r\n"
		"		FROM EmrTopicsT INNER JOIN EmrProblemLinkT ON EmrTopicsT.ID = EmrProblemLinkT.EmrRegardingID\r\n"
		"		WHERE EmrProblemLinkT.EmrProblemID = {INT} AND EmrProblemLinkT.EmrRegardingType = {CONST_INT})\r\n"
		"	OR EmrMasterT.ID IN (SELECT EmrProblemLinkT.EmrRegardingID\r\n"
		"		FROM EmrProblemLinkT\r\n"
		"		WHERE EmrProblemLinkT.EmrProblemID = {INT} AND EmrProblemLinkT.EmrRegardingType = {CONST_INT})\r\n"
		"	OR EmrMasterT.ID IN (SELECT EmrDiagCodesT.EmrID\r\n"
		"		FROM EmrDiagCodesT INNER JOIN EmrProblemLinkT ON EmrDiagCodesT.ID = EmrProblemLinkT.EmrRegardingID\r\n"
		"		WHERE EmrProblemLinkT.EmrProblemID = {INT} AND EmrProblemLinkT.EmrRegardingType = {CONST_INT})\r\n"
		"	OR EmrMasterT.ID IN (SELECT EmrChargesT.EmrID\r\n"
		"		FROM EmrChargesT INNER JOIN EmrProblemLinkT ON EmrChargesT.ID = EmrProblemLinkT.EmrRegardingID\r\n"
		"		WHERE EmrProblemLinkT.EmrProblemID = {INT} AND EmrProblemLinkT.EmrRegardingType = {CONST_INT})\r\n"
		"	OR EmrMasterT.ID IN (SELECT EmrMedicationsT.EmrID\r\n"
		"		FROM EmrMedicationsT INNER JOIN EmrProblemLinkT ON EmrMedicationsT.MedicationID = EmrProblemLinkT.EmrRegardingID\r\n"
		"		WHERE EmrProblemLinkT.EmrProblemID = {INT} AND EmrProblemLinkT.EmrRegardingType = {CONST_INT})\r\n"
		"	{SQL}\r\n\r\n",
			nProblemID, eprtEmrDataItem, eprtEmrItem,
			nProblemID, eprtEmrTopic,
			nProblemID, eprtEmrEMN,
			nProblemID, eprtEmrDiag,
			nProblemID, eprtEmrCharge,
			nProblemID, eprtEmrMedication,
			sqlEmrProblemCase);
}

CEmrTableStateIterator::CEmrTableStateIterator(const CString& strState) :
m_strState(strState)
{
	// (c.haag 2007-08-18 11:22) - PLID 27112 - Constructor for CEmrTableStateIterator
	m_nStateLen = strState.GetLength();
	m_nPos = 0;
}

// (z.manning 2010-02-18 09:15) - PLID 37427 - Added nEmrDetailImageStampID
// (z.manning 2011-03-02 14:49) - PLID 42335 - Added nStampID
BOOL CEmrTableStateIterator::ReadNextElement(OUT long& nRow, OUT long& nColumn, OUT CString& strData, OUT long& nEmrDetailImageStampID, OUT long& nEmrDetailImageStampPointer, OUT long &nStampID)
{
	// (c.haag 2007-08-18 11:23) - PLID 27112 - Reads the next unread element from the state. Returns
	// FALSE if we did not get a value because we've been through the whole state. Throws an exception
	// if the state is malformed.
	if (m_nPos >= m_nStateLen) {
		return FALSE; // We're done becuase there are no more elements to iterate through
	}
	int pos = m_strState.Find(";", m_nPos);
	if (-1 == pos || pos - m_nPos <= 0) {
		ASSERT(FALSE); // Last chance to look at the data
		ThrowNxException("CEmrTableStateIterator::ReadNextElement was called for an invalid state");
	}

	// Read the row
	CString strRow = m_strState.Mid(m_nPos, pos - m_nPos);
	nRow = atoi(strRow);

	// Read the column
	m_nPos = pos + 1;
	pos = m_strState.Find(";", m_nPos);
	if (-1 == pos || pos - m_nPos <= 0) {
		ASSERT(FALSE); // Last chance to look at the data
		ThrowNxException("CEmrTableStateIterator::ReadNextElement was called for an invalid state");
	}
	CString strColumn = m_strState.Mid(m_nPos, pos - m_nPos);
	nColumn = atoi(strColumn);

	// Read the data
	m_nPos = pos + 1;
	pos = FindDelimiter(m_strState, ';', '\\', m_nPos);
	if (-1 == pos || pos - m_nPos < 0) { // Do <0 rather than <=0 because we can have empty data
		ASSERT(FALSE); // Last chance to look at the data
		ThrowNxException("CEmrTableStateIterator::ReadNextElement was called for an invalid state");
	}
	strData = ReadFromDelimitedField(m_strState.Mid(m_nPos ,pos - m_nPos), ';', '\\');

	// (z.manning 2010-02-18 09:16) - PLID 37427 - Added nEmrDetailImageStampID to the table state
	m_nPos = pos + 1;
	pos = FindDelimiter(m_strState, ';', '\\', m_nPos);
	if (-1 == pos || pos - m_nPos < 0) { // Do <0 rather than <=0 because we can have empty data
		ASSERT(FALSE); // Last chance to look at the data
		ThrowNxException("CEmrTableStateIterator::ReadNextElement was called for an invalid state");
	}
	CString strEmrDetailImageStampID = ReadFromDelimitedField(m_strState.Mid(m_nPos ,pos - m_nPos), ';', '\\');
	nEmrDetailImageStampID = atol(strEmrDetailImageStampID);
	if(nEmrDetailImageStampID == 0) {
		ASSERT(FALSE);
		ThrowNxException("CEmrTableStateIterator::ReadNextElement - Invalid value for detail image stamp ID: %s (state = %s)"
			, strEmrDetailImageStampID, m_strState);
	}

	// (z.manning 2010-02-18 09:16) - PLID 37427 - Added nEmrDetailImageStampPointer to the table state
	m_nPos = pos + 1;
	pos = FindDelimiter(m_strState, ';', '\\', m_nPos);
	if (-1 == pos || pos - m_nPos < 0) { // Do <0 rather than <=0 because we can have empty data
		ASSERT(FALSE); // Last chance to look at the data
		ThrowNxException("CEmrTableStateIterator::ReadNextElement was called for an invalid state");
	}
	CString strEmrDetailImageStampPointer = ReadFromDelimitedField(m_strState.Mid(m_nPos ,pos - m_nPos), ';', '\\');
	nEmrDetailImageStampPointer = atol(strEmrDetailImageStampPointer);

	// (z.manning 2011-03-02 14:51) - PLID 42335 - Added the global stamp ID to the table state
	m_nPos = pos + 1;
	pos = FindDelimiter(m_strState, ';', '\\', m_nPos);
	if (-1 == pos || pos - m_nPos < 0) { // Do <0 rather than <=0 because we can have empty data
		ASSERT(FALSE); // Last chance to look at the data
		ThrowNxException("CEmrTableStateIterator::ReadNextElement was called for an invalid state (nStampID)");
	}
	CString strStampID = ReadFromDelimitedField(m_strState.Mid(m_nPos ,pos - m_nPos), ';', '\\');
	nStampID = atol(strStampID);
	if(nStampID == 0) {
		ASSERT(FALSE);
		ThrowNxException("CEmrTableStateIterator::ReadNextElement - Invalid value for stamp ID: %s (state = %s)"
			, strEmrDetailImageStampID, m_strState);
	}

	// Seek to the next element and return
	m_nPos = pos + 1;
	return TRUE;
}


// (a.walling 2007-03-12 09:44) - PLID 19884 - Return the name of the item type given the enum
// (a.walling 2007-04-03 09:08) - PLID 25454 - Support subtypes too
CString GetDataTypeName(EmrInfoType eitDataType, EmrInfoSubType eistSubType /*=eistNone */)
{
	CString str;

	if (eistSubType == eistCurrentMedicationsTable) {
		str = "Current Medications Table";
	} else if (eistSubType == eistAllergiesTable) {
		str = "Allergies Table";
	} else {
		switch(eitDataType) {
			case eitText:
				str = "Text";
				break;
			case eitSingleList:
				str = "Single-Select List";
				break;
			case eitMultiList:
				str = "Multi-Select List";
				break;
			case eitImage:
				str = "Image";
				break;
			case eitSlider:
				str = "Slider";
				break;
			case eitNarrative:
				str = "Narrative";
				break;
			case eitTable:
				str = "Table";
				break;
			default:
				ThrowNxException("Invalid item type passed to GetDataTypeName()");
				break;
		}
	}

	return str;
}

// (a.walling 2007-04-03 15:41) - PLID 25454 - CSS class for datatype
// (a.walling 2007-04-03 15:42) - PLID 25454 - CSS class for topic completion status
// (b.cardillo 2012-03-28 21:54) - PLID 42207 - (additional) Removed this functionality because it's never called.

CString GetEmrActionObjectName(EmrActionObject Type, BOOL bLevel2) {
	switch(Type) {
	case eaoCpt:
		// (j.jones 2012-08-24 16:26) - PLID 52091 - these are charges, and can be products,
		// so I changed the name from Service Code
		return "Charge";
		break;
	// (b.savon 2014-07-14 10:09) - PLID 62706 - Deprecate the old Diag DestType and handle the changes Practice wide
	//case eaoDiag:
	//	// (a.wilson 2014-02-18 09:13) - PLID 60765 - rename to "Diagnosis Code" instead of "ICD9 Code".
	//	return "Diagnosis Code";
	//	break;
	case eaoEmrItem:
		return bLevel2 ? "EMR Item" : "Item";
		break;
	case eaoEmrDataItem:
		return bLevel2 ? "EMR List Option" : "List Option";
		break;
	case eaoProcedure:
		return "Procedure";
		break;
	//DRT 1/17/2008 - PLID 28602
	case eaoEmrImageHotSpot:
		return "Image Hot Spot For";
		break;
	// (c.haag 2008-07-17 17:32) - PLID 30723
	case eaoMint:
		return "EMN";
		break;
	// (c.haag 2008-07-22 10:36) - PLID 30723
	case eaoMedication:
		return "Medication";
		break;
	// (z.manning 2008-10-02 10:11) - PLID 21094
	case eaoLab:
		return "Lab";
		break;
	// (z.manning 2009-02-10 15:31) - PLID 33026
	case eaoEmrTableDropDownItem:
		return "EMR Table Dropdown Item";
		break;
	case eaoSmartStamp: // (z.manning 2010-02-15 11:01) - PLID 37226
		return "Smart Stamp";
		break;
	case eaoMintItems: // (z.manning 2011-11-14 14:43) - PLID 46231
		return "Topic";
		break;
	case eaoWoundCareCodingCondition: // (r.gonet 08/03/2012) - PLID 51949
		return "Wound Care Coding Condition";
		break;
	// (b.savon 2014-07-14 10:09) - PLID 62706 - Deprecate the old Diag DestType and handle the changes Practice wide
	case eaoDiagnosis:
		return "Diagnosis Code";
		break;
	}
	ASSERT(FALSE);
	return "";
}

CString GetEmrActionObjectSourceTable(EmrActionObject Type) {
	switch(Type) {
	case eaoCpt:
		// (j.jones 2012-08-24 16:26) - PLID 52091 - these can be products too, so this has to be a left join
		return "ServiceT LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID";
		break;
	// (b.savon 2014-07-14 10:28) - PLID 62706 - Deprecate the old Diag DestType and handle the changes Practice wide
	//case eaoDiag:
	//	return "DiagCodes";
	//	break;
	case eaoEmrItem:
		return "EmrInfoT";
		break;
	case eaoEmrDataItem:
		return "EmrDataT";
		break;
	case eaoProcedure:
		return "ProcedureT";
		break;
	//DRT 1/17/2008 - PLID 28602
	case eaoEmrImageHotSpot:
		return "EMRImageHotSpotsT";
		break;
	// (z.manning 2009-02-10 15:25) - PLID 33026 - EMR table dropdown items can now spawn
	case eaoEmrTableDropDownItem:
		return "EMRTableDropdownInfoT";
		break;
	case eaoSmartStamp: // (z.manning 2010-02-15 11:02) - PLID 37226
		return "EMRImageStampsT";
		break;
	case eaoWoundCareCodingCondition: // (r.gonet 08/03/2012) - PLID 51949
		return "WoundCareConditionT";
		break;
	// (b.savon 2014-07-14 10:38) - PLID 62706 - Deprecate the old Diag DestType and handle the changes Practice wide
	case eaoDiagnosis:
		return R"(EmrActionDiagnosisDataT 
				LEFT JOIN DiagCodes AS ICD9Q ON EmrActionDiagnosisDataT.DiagCodeID_ICD9 = ICD9Q.ID
				LEFT JOIN DiagCodes AS ICD10Q ON EmrActionDiagnosisDataT.DiagCodeID_ICD10 = ICD10Q.ID)";

	}
	ASSERT(FALSE);
	return "";
}

CString GetEmrActionObjectSourceIdField(EmrActionObject Type) {
	switch(Type) {
	case eaoCpt:
		return "ServiceT.ID";
		break;
	// (b.savon 2014-07-14 10:28) - PLID 62706 - Deprecate the old Diag DestType and handle the changes Practice wide
	//case eaoDiag:
	//	return "DiagCodes.ID";
	//	break;
	case eaoEmrItem:
		return "EmrInfoT.ID";
		break;
	case eaoEmrDataItem:
		return "EmrDataT.ID";
		break;
	case eaoProcedure:
		return "ProcedureT.ID";
		break;
	//DRT 1/17/2008 - PLID 28602
	case eaoEmrImageHotSpot:
		return "EMRImageHotSpotsT.ID";
		break;
	// (z.manning 2009-02-10 15:25) - PLID 33026 - EMR table dropdown items can now spawn
	case eaoEmrTableDropDownItem:
		return "EMRTableDropdownInfoT.ID";
		break;
	case eaoSmartStamp: // (z.manning 2010-02-15 11:05) - PLID 37226
		return "EMRImageStampsT.ID";
		break;
	case eaoWoundCareCodingCondition: // (r.gonet 08/03/2012) - PLID 51949
		return "WoundCareConditionT.ConditionID";
		break;
	// (b.savon 2014-07-14 10:38) - PLID 62706 - Deprecate the old Diag DestType and handle the changes Practice wide
	case eaoDiagnosis:
		return "EmrActionDiagnosisDataT.EmrActionID";
		break;
	}
	ASSERT(FALSE);
	return "";
}

CString GetEmrActionObjectSourceNameField(EmrActionObject Type) {
	switch(Type) {
	case eaoCpt:
		// (j.jones 2012-08-24 16:25) - PLID 52091 - don't show a CPT code if it's a product
		return "CASE WHEN CPTCodeT.Code Is Not Null THEN CPTCodeT.Code + ' - ' ELSE '' END + ServiceT.Name";
		break;
	// (b.savon 2014-07-14 10:28) - PLID 62706 - Deprecate the old Diag DestType and handle the changes Practice wide
	//case eaoDiag:
	//	return "DiagCodes.CodeNumber + ' - ' + DiagCodes.CodeDesc";
	//	break;
	case eaoEmrItem:
		return "EmrInfoT.Name";
		break;
	case eaoEmrDataItem:
		return "EmrDataT.Data";
		break;
	case eaoProcedure:
		return "ProcedureT.Name";
		break;
	case eaoEmrImageHotSpot:
		//DRT 1/17/2008 - PLID 28602 - This is not supported for Image hotspots, as they have no names to lookup.  Use
		//	the EMR item instead with an override.  This code left so it doesn't throw errors, but you will get no name.
		ASSERT(FALSE);
		return "''";
		break;
	// (z.manning 2009-02-10 15:25) - PLID 33026 - EMR table dropdown items can now spawn
	case eaoEmrTableDropDownItem:
		return "EMRTableDropdownInfoT.Data";
		break;
	case eaoSmartStamp: // (z.manning 2010-02-15 11:05) - PLID 37226
		return "EMRImageStampsT.TypeName";
		break;
	case eaoWoundCareCodingCondition: // (r.gonet 08/03/2012) - PLID 51949
		return "WoundCareConditionT.Name";
		break;
	// (b.savon 2014-07-14 10:28) - PLID 62706 - Deprecate the old Diag DestType and handle the changes Practice wide
	// Kathy said: if just a 9, do what it did before.  if just a 10, resemble that.  if a comination you can do "icd-10 code - icd-10 description (icd-9 code)"
	case eaoDiagnosis:
		return R"(CASE WHEN ICD10Q.CodeNumber IS NOT NULL AND ICD9Q.CodeNumber IS NOT NULL THEN (ICD10Q.CodeNumber + ' - ' + ICD10Q.CodeDesc + ' (' + ICD9Q.CodeNumber + ')') ELSE
				  CASE WHEN ICD10Q.CodeNumber IS NOT NULL THEN (ICD10Q.CodeNumber + ' - ' + ICD10Q.CodeDesc) ELSE 
				  CASE WHEN ICD9Q.CodeNumber IS NOT NULL THEN (ICD9Q.CodeNumber + ' - ' + ICD9Q.CodeDesc) END END END)";
		break;
	}
	ASSERT(FALSE);
	return "";
}

// (j.jones 2012-08-27 12:17) - PLID 52091 - added a dest name function
CString GetEmrActionObjectDestNameField(EmrActionObject Type)
{		
	switch(Type) {
	case eaoCpt:
		return "CASE WHEN CPTCodeT.Code Is Not Null THEN CPTCodeT.Code + ' - ' ELSE '' END + ServiceT.Name";
		break;
	// (b.savon 2014-07-14 10:28) - PLID 62706 - Deprecate the old Diag DestType and handle the changes Practice wide
	//case eaoDiag:
	//	return "DiagCodes.CodeNumber + ' - ' + DiagCodes.CodeDesc";
	//	break;
	case eaoEmrItem:
		return "EmrInfoT.Name";
		break;
	case eaoProcedure:
		return "ProcedureT.Name";
		break;
	case eaoMint:
	case eaoMintItems:
		return "EMRTemplateT.Name";
		break;
	case eaoMedication:
		return "EMRDataT.Data";
		break;
	case eaoLab:
		return "LabProceduresT.Name";
		break;
	case eaoTodo:
		// (j.jones 2012-08-27 13:59) - this is not supported, todos don't have unique names
		ASSERT(FALSE);
		return "''";
		break;
	// (b.savon 2014-07-14 10:28) - PLID 62706 - Deprecate the old Diag DestType and handle the changes Practice wide
	// Kathy said: if just a 9, do what it did before.  if just a 10, resemble that.  if a comination you can do "icd-10 code - icd-10 description (icd-9 code)"
	case eaoDiagnosis:
		return R"(CASE WHEN ICD10Q.CodeNumber IS NOT NULL AND ICD9Q.CodeNumber IS NOT NULL THEN (ICD10Q.CodeNumber + ' - ' + ICD10Q.CodeDesc + ' (' + ICD9Q.CodeNumber + ')') ELSE
				  CASE WHEN ICD10Q.CodeNumber IS NOT NULL THEN (ICD10Q.CodeNumber + ' - ' + ICD10Q.CodeDesc) ELSE 
				  CASE WHEN ICD9Q.CodeNumber IS NOT NULL THEN (ICD9Q.CodeNumber + ' - ' + ICD9Q.CodeDesc) END END END)";
		break;
	}
	//unsupported type
	ASSERT(FALSE);
	return "";
}

CString GetHeaderSql(long nEmrID)
{
	CString strSql;
	strSql.Format("SELECT PersonT.First + CASE WHEN PersonT.Middle = '' THEN '' ELSE ' ' END + PersonT.Middle + ' ' + PersonT.Last AS PatNameFML, "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatNameLFM, "
		"EMRMasterT.Date, PersonT.BirthDate, EmrMasterT.PatientAge, dbo.GetEmnProviderList(EmrMasterT.ID) AS DocName "
		"FROM EMRMasterT INNER JOIN PersonT ON EMRMasterT.PatientID = PersonT.ID "
		"WHERE EMRMasterT.ID = %li", nEmrID);
	return strSql;
}

CString GetHeaderFieldDataField(EmrHeaderField ehfType)
{
	switch(ehfType) {
	case ehfPatientNameFML:
		return "PatNameFML";
		break;
	case ehfPatientNameLFM:
		return "PatNameLFM";
		break;
	case ehfEmrDate:
		return "Date";
		break;
	case ehfPatientBirthDate:
		return "BirthDate";
		break;
	case ehfPatientAge:
		return "PatientAge";
		break;
	case ehfProviderName:
		return "DocName";
		break;
	}
	ASSERT(FALSE);
	return "";
}

CString GetHeaderFieldDisplayName(EmrHeaderField ehfType)
{
	switch(ehfType) {
	case ehfPatientNameFML:
		return "Patient";
		break;
	case ehfPatientNameLFM:
		return "Patient";
		break;
	case ehfEmrDate:
		return "Date";
		break;
	case ehfPatientBirthDate:
		return "DOB";
		break;
	case ehfPatientAge:
		return "Age";
		break;
	case ehfProviderName:
		return "Provider";
		break;
	}
	ASSERT(FALSE);
	return "";
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// Merge related functions
//

// (c.haag 2006-07-10 17:40) - PLID 20661 - This now fills two lists of fields. strFilledFields
// is filled in the form "EMR_Category_A,EMR_Category_B,EMR_Category_C...", and strEmptyFields
// is done in the form ""","","",...".
// (c.haag 2004-06-30 10:48) - Returns a list of all categories as merge
// fields in the form "EMR_Category_A,EMR_Category_B,EMR_Category_C..."
void GetEmrCategoryMergeFieldList(CString& strFilledFields, CString& strEmptyFields)
{
	_RecordsetPtr prsEMRCat = CreateRecordset("SELECT EMRCategoriesT.Name AS Name FROM EMRCategoriesT ORDER BY EMRCategoriesT.Name");
	strFilledFields.Empty();
	strEmptyFields.Empty();
	while (!prsEMRCat->eof)
	{
		// Get the merge field name based on this category
		// (b.cardillo 2004-06-02 13:01) - For some reason this used to do the work of ConvertToHeaderName() in 
		// place here instead of just calling it.  (This replaces (c.haag 2004-05-27 08:56) PLID 12614.)
		CString strCatFieldName = ConvertToHeaderName("EMR_Category", AdoFldString(prsEMRCat, "Name"));
		strFilledFields += strCatFieldName + ",";
		strEmptyFields += "\"\",";
		prsEMRCat->MoveNext();
	}
	if (strFilledFields.GetLength()) {
		strFilledFields = strFilledFields.Left( strFilledFields.GetLength() - 1 );
	}
	if (strEmptyFields.GetLength()) {
		strEmptyFields = strEmptyFields.Left( strEmptyFields.GetLength() - 1 );
	}
}

// (c.haag 2006-07-10 17:42) - PLID 20661 - This now fills two lists of fields. strFilledFields
// is filled in the form "Item_1,Item_2,Item_3..." and strEmptyFields is done in the form ""","","",...".
void GetEmrItemMergeFieldList(const CString& strWhereClause, CString& strFilledFields, CString& strEmptyFields)
{
	// (b.cardillo 2014-10-05 21:56) - PLID 62200 - Just get distinct names, since the WHERE clause 
	// could result in including any number of old instances a given EMR item, or other sets of EMR 
	// items that have the same name for whatever reason. Since we always merge by name, there is 
	// never any point in including multiple copies of the same one, it only slows down Word and 
	// causes confusion in the Word user interface when adding fields (dups get unique names, like 
	// EMR_MyItem1 and EMR_MyItem2).
	_RecordsetPtr prsEMRInfo = CreateRecordset("SELECT DISTINCT EMRInfoT.Name FROM EMRInfoT %s ORDER BY EMRInfoT.Name",
		!strWhereClause.IsEmpty() ? "WHERE " + strWhereClause : "");
	strFilledFields.Empty();
	strEmptyFields.Empty();
	while (!prsEMRInfo->eof)
	{
		CString strInsert;
		// (b.cardillo 2004-06-02 13:01) - For some reason this used to do the work of ConvertToHeaderName() in 
		// place here instead of just calling it.  (This replaces (c.haag 2004-05-27 08:56) PLID 12614.)
		CString strFieldName = ConvertToHeaderName("EMR", AdoFldString(prsEMRInfo, "Name"));
		strFilledFields += strFieldName + ",";
		strEmptyFields += "\"\",";
		prsEMRInfo->MoveNext();
	}
	if (strFilledFields.GetLength()) {
		strFilledFields = strFilledFields.Left( strFilledFields.GetLength() - 1 );
	}
	if (strEmptyFields.GetLength()) {
		strEmptyFields = strEmptyFields.Left( strEmptyFields.GetLength() - 1 );
	}
}

EmrInvalidNameReason IsValidEMRMergeFieldName(const CString& strName)
{
	CString ConvertToHeaderName(const CString &strPrefix, const CString &strHeaderBaseText, OPTIONAL OUT BOOL *pbTruncated = NULL);
	CString str = ConvertToHeaderName("EMR", strName);
	if (str.GetLength() > MAX_MERGE_FIELD_LENGTH) {
		return einrTooLong;
	}
	else if (
		!str.CompareNoCase("EMR_Date") ||
		!str.CompareNoCase("EMR_Input_Date") ||
		!str.CompareNoCase("EMR_Provider") ||
		!str.CompareNoCase("EMR_Provider_Secondary") ||		//DRT 1/9/2007 - PLID 24160
		!str.CompareNoCase("EMR_Procedure_List") ||
		!str.CompareNoCase("EMR_DiagCode_List") || 
		!str.CompareNoCase("EMR_ServiceCode_List") || 
		// (j.jones 2012-07-24 12:47) - PLID 44349 - added billable service codes
		!str.CompareNoCase("EMR_BillableServiceCode_List") || 
		!str.CompareNoCase("EMR_Medication_List") || // (a.walling 2006-12-11 13:14) - PLID 17298
		!str.CompareNoCase("EMR_Notes") || 
		!str.CompareNoCase("EMR_Collection") ||
		!str.Left(13).CompareNoCase("EMR_Category_") ||
		!str.CompareNoCase("EMR_Surgery_Appt") ||
		!str.CompareNoCase("EMR_Other_Appts_List") ||
		// (j.jones 2007-08-16 09:36) - PLID 27054 - added Visit Type
		!str.CompareNoCase("EMR_Visit_Type") ||
		// (b.eyers 2016-02-23) - PLID 68323 - added discharge status, admission time, discharge time
		!str.CompareNoCase("EMR_Discharge_Status_Description") ||
		!str.CompareNoCase("EMR_Discharge_Status_Code") ||
		!str.CompareNoCase("EMR_Admission_Time") ||
		!str.CompareNoCase("EMR_Discharge_Time"))
	{
		return einrReservedName;
	}
	return einrOK;
}

/*void GetEMRMINTListWithActions(CDWordArray& adwEMRMINTIDs)
{
	CString strMINTIDs;
	for (long i=0; i < adwEMRMINTIDs.GetSize(); i++)
	{
		CString str;
		if (i == 0) {
			str.Format("%d", adwEMRMINTIDs[i]);
		} else {
			str.Format(",%d", adwEMRMINTIDs[i]);
		}
		str += strMINTIDs;
	}

	BOOL bRepeat = TRUE;
	while (bRepeat && !strMINTIDs.IsEmpty())
	{
		_RecordsetPtr prs = CreateRecordset(
			"SELECT DestID FROM EMRActionsT WHERE SourceType = 4 AND DestType = 6 AND SourceID IN "
			"(SELECT ID FROM EMRDataT WHERE EMRInfoID IN (SELECT EMRInfoID FROM EMRInfoDefaultsT "
			"WHERE EMRInfoID IN (SELECT EMRInfoID FROM EMRTemplateDetailsT WHERE TemplateID IN (%s))))",
			strMINTIDs);
		while (!prs->eof)
		{
			bRepeat = FALSE;
			while (!prs->eof)
			{
				// Go through each DestID and see if we already have it in our array of 
				// EMR Item ID's.
				long nEMRInfoID = AdoFldLong(prs, "DestID");
				for (int i=0; i < adwEMRMINTIDs.GetSize(); i++)
				{
					if ((long)adwEMRMINTIDs[i] == nEMRInfoID)
						break;
				}
				// If we do, just continue iterating through the recordset.
				if (i == adwEMRMINTIDs.GetSize()) {
					CString str;
					str.Format("%d", nEMRInfoID);
					adwEMRMINTIDs.Add(nEMRInfoID);
					if (!strMINTIDs.IsEmpty()) {
						strMINTIDs += ",";
					}
					strMINTIDs += str;
					bRepeat = TRUE; // There could be more ID's, so we need to run the loop again
				}
				prs->MoveNext();
			}
		}
	}
}*/

CString FillIDsInArray(CDWordArray& adw, LPCTSTR szSql, ...)
{
	CString strOutput;
	CString strSql;

	// Parse string inserting arguments where appropriate
	va_list argList;
	va_start(argList, szSql);
	strSql.FormatV(szSql, argList);
	va_end(argList);

	// Open the recordset
	// (j.jones 2010-04-20 10:08) - PLID 38273 - changed to CreateRecordsetStd
	_RecordsetPtr prs = CreateRecordsetStd(strSql);

	// Build the list of original ID's
	adw.RemoveAll();
	while (!prs->eof)
	{
		CString str;
		long nID = VarLong(prs->Fields->Item[0L]->Value);
		adw.Add(nID);
		str.Format("%d", nID);
		if (!strOutput.IsEmpty()) {
			strOutput += ",";
		}
		strOutput += str;
		prs->MoveNext();
	}
	prs->Close();
	return strOutput;
}

void RemoveIDsFromArray(CDWordArray& adwEMRInfoIDs, LPCTSTR szSql, ...)
{
	CString strSql;

	// Parse string inserting arguments where appropriate
	va_list argList;
	va_start(argList, szSql);
	strSql.FormatV(szSql, argList);
	va_end(argList);

	// Open the recordset
	// (j.jones 2010-04-20 10:08) - PLID 38273 - changed to CreateRecordsetStd
	_RecordsetPtr prs = CreateRecordsetStd(strSql);
	// Traverse the recordset to find ID's we want to get rid of. We assume
	// the ID column is the first column of the recordset.
	while (!prs->eof)
	{
		long nID = VarLong(prs->Fields->Item[0L]->Value);
		for (long i=0; i < adwEMRInfoIDs.GetSize(); i++)
		{
			if (adwEMRInfoIDs[i] == (DWORD)nID) {
				adwEMRInfoIDs.RemoveAt(i);
				break;
			}
		}
		prs->MoveNext();
	}
}

// (j.jones 2007-08-02 10:14) - PLID 26912 - added lprsInfo as a required parameter
// (z.manning 2011-02-24 12:14) - PLID 42579 - This now takes an array of detail IDs
_variant_t LoadEMRDetailState(_Recordset *lprsInfo, CArray<long,long> &arynDetailIDs, EmrInfoType nEMRInfoDatatype, OPTIONAL IN ADODB::_Connection *lpCon /*= NULL*/)
{
	if(arynDetailIDs.GetSize() == 0) {
		return ::LoadEMRDetailStateBlank(nEMRInfoDatatype);
	}

	_variant_t varNull;
	varNull.vt = VT_NULL;

	_ConnectionPtr pCon;
	if(lpCon) pCon = lpCon;
	else pCon = GetRemoteData();
	switch (nEMRInfoDatatype) {
	case eitText:
		{
			// (z.manning 2011-02-24 12:26) - PLID 42579 - This type does note support loading its state from
			// more than one previous detail.
			if(arynDetailIDs.GetSize() > 1) {
				ASSERT(FALSE);
				AfxThrowNxException("EMR info type %d does not support loading state from multiple details", nEMRInfoDatatype);
			}

			// (j.jones 2007-08-02 10:16) - PLID 26912 - if prsInfo was passed in, check that the recordset
			if(lprsInfo) {
				_RecordsetPtr rs = lprsInfo;
				//there are no current cases where the passed in recordset would not know the value
				//TES 11/7/2007 - PLID 27979 - VS2008 - Need to explicitly return a _variant_t.  I'm choosing
				// to not just return the Value, since that would mean that, in the case where the value was NULL,
				// the behavior would change from returning a VT_BSTR representing an empty string, to VT_NULL,
				// and I don't want to change behavior, even though that seems like the more correct behavior here.
				return _variant_t(AdoFldString(rs, "Text", ""));
			}

			//if we had no recordset, then we need to query ourselves

			// (j.jones 2007-07-30 14:59) - PLID 26878 - converted to use a parameter query
			_CommandPtr pCmd = OpenParamQuery(pCon, "SELECT Text FROM EmrDetailsT WHERE ID = ?");
			AddParameterLong(pCmd, "DetailID", arynDetailIDs.GetAt(0));

			_RecordsetPtr rs = CreateRecordset(pCmd);
			if (!rs->eof) {
				// Return the string
				//TES 11/7/2007 - PLID 27979 - VS2008 - Need to explicitly return a _variant_t.  I'm choosing
				// to not just return the Value, since that would mean that, in the case where the value was NULL,
				// the behavior would change from returning a VT_BSTR representing an empty string, to VT_NULL,
				// and I don't want to change behavior, even though that seems like the more correct behavior here.
				return _variant_t(VarString(rs->GetFields()->GetItem("Text")->GetValue(), ""));
			} else {
				// No detail!  Why was this function called with a non-existent detail id?
				ASSERT(FALSE);
				return varNull;
			}
		}
		break;
	case eitSingleList:
		{
			// (z.manning 2011-02-24 12:26) - PLID 42579 - This type does note support loading its state from
			// more than one previous detail.
			if(arynDetailIDs.GetSize() > 1) {
				ASSERT(FALSE);
				AfxThrowNxException("EMR info type %d does not support loading state from multiple details", nEMRInfoDatatype);
			}

			// (j.jones 2007-08-02 10:54) - PLID 26912 - if prsInfo was passed in, check that the recordset
			// to see if we know ahead of time there are no selected items
			if(lprsInfo) {
				_RecordsetPtr rs = lprsInfo;
				//there are no current cases where the passed in recordset would not know the value,
				//but just incase it is NULL, assume we have selections because it means we don't know
				long nHasSelections = AdoFldLong(rs, "Detail_HasListSelections",1);
				if(nHasSelections == 0)
					return "";
			}

			//if we had no recordset, or that recordset told us we have selections, then we need to query the data

			// (j.jones 2007-07-30 14:59) - PLID 26878 - converted to use a parameter query
			_CommandPtr pCmd = OpenParamQuery(pCon, "SELECT EMRDataID FROM EmrSelectT WHERE EMRDetailID = ?");
			AddParameterLong(pCmd, "DetailID", arynDetailIDs.GetAt(0));

			_RecordsetPtr rs = CreateRecordset(pCmd);
			if (!rs->eof) {
				// Return the data id as a string
				//TES 11/7/2007 - PLID 27979 - VS2008 - Need to explicitly return a _variant_t.
				return _variant_t(AsString(VarLong(rs->GetFields()->GetItem("EMRDataID")->GetValue())));
			} else {
				// No selection for this detail.  We used to not allow this, but now "no selection" is valid.
				return "";
			}
		}
		break;
	case eitMultiList:
		{
			// (z.manning 2011-02-24 12:26) - PLID 42579 - This type does note support loading its state from
			// more than one previous detail.
			if(arynDetailIDs.GetSize() > 1) {
				ASSERT(FALSE);
				AfxThrowNxException("EMR info type %d does not support loading state from multiple details", nEMRInfoDatatype);
			}

			// (j.jones 2007-08-02 10:54) - PLID 26912 - if prsInfo was passed in, check that the recordset
			// to see if we know ahead of time there are no selected items
			if(lprsInfo) {
				_RecordsetPtr rs = lprsInfo;
				//there are no current cases where the passed in recordset would not know the value,
				//but just incase it is NULL, assume we have selections because it means we don't know
				long nHasSelections = AdoFldLong(rs, "Detail_HasListSelections",1);
				if(nHasSelections == 0)
					return "";
			}

			//if we had no recordset, or that recordset told us we have selections, then we need to query the data

			// (j.jones 2007-07-30 14:59) - PLID 26878 - converted to use a parameter query
			_CommandPtr pCmd = OpenParamQuery(pCon, "SELECT EMRDataID FROM EmrSelectT WHERE EMRDetailID = ?");
			AddParameterLong(pCmd, "DetailID", arynDetailIDs.GetAt(0));

			_RecordsetPtr rs = CreateRecordset(pCmd);
			if (!rs->eof) {
				// Return the list of data ids as a semicolon-delimited string
				CString strAns;
				FieldPtr fldDataID = rs->GetFields()->GetItem("EMRDataID");
				while (!rs->eof) {
					if (!strAns.IsEmpty()) {
						strAns += "; ";
					}
					CString str;
					str.Format("%li", VarLong(fldDataID->GetValue()));
					strAns += str;
					rs->MoveNext();
				}
				//TES 11/7/2007 - PLID 27979 - VS2008 - Need to explicitly return a _variant_t.
				return _variant_t(strAns);
			} else {
				// No selection for this detail.  We used to not allow this, but now "no selection" is valid.
				return "";
			}
		}

		break;
	case eitImage:
		{
			// (z.manning 2011-02-24 12:26) - PLID 42579 - This type does note support loading its state from
			// more than one previous detail.
			if(arynDetailIDs.GetSize() > 1) {
				ASSERT(FALSE);
				AfxThrowNxException("EMR info type %d does not support loading state from multiple details", nEMRInfoDatatype);
			}

			// (j.jones 2007-08-02 10:16) - PLID 26912 - if prsInfo was passed in, check that the recordset
			// (z.manning 2011-02-14 12:46) - PLID 42452 - Order by OrderIndex!
			// (z.manning 2011-01-27 16:27) - PLID 42335 - Added UsedInTableData
			// (z.manning 2011-09-08 10:23) - PLID 45335 - Added 3D fields
			_RecordsetPtr rs, rsDetailImageStamps = NULL;
			CString strDetailStampSql = 
				"SELECT ID, EmrImageStampID, OrderIndex, XPos, YPos, SmartStampTableSpawnRule \r\n"
				"	, CONVERT(bit, CASE WHEN EmrDetailTableDataQ.EmrDetailImageStampID IS NULL THEN 0 ELSE 1 END) AS UsedInTableData \r\n"
				"	, XPos3D, YPos3D, ZPos3D, XNormal, YNormal, ZNormal, HotSpot3D \r\n"
				"FROM EmrDetailImageStampsT \r\n"
				"LEFT JOIN (SELECT DISTINCT EmrDetailTableDataT.EmrDetailImageStampID FROM EmrDetailTableDataT WHERE EmrDetailTableDataT.EmrDetailImageStampID IS NOT NULL) EmrDetailTableDataQ ON EmrDetailImageStampsT.ID = EmrDetailTableDataQ.EmrDetailImageStampID \r\n"
				"WHERE EmrDetailID = {INT} AND EmrDetailImageStampsT.Deleted = 0 \r\n"
				"ORDER BY EmrDetailImageStampsT.OrderIndex \r\n";

			if(lprsInfo) {
				rs = lprsInfo;
				BOOL bHasDetailImageStamps = AdoFldBool(rs->GetFields(), "Detail_HasDetailImageStamps", TRUE);
				if(bHasDetailImageStamps) {
					rsDetailImageStamps = CreateParamRecordset(pCon, strDetailStampSql, arynDetailIDs.GetAt(0));
				}
			}
			else {
				//if we had no recordset, then we need to query ourselves

				// (j.jones 2007-07-30 14:59) - PLID 26878 - converted to use a parameter query
				// (r.gonet 05/31/2011) - PLID 43896 - Restore zoom related fields when we want to save zoom and pan offsets.
				// (z.manning 2011-10-05 16:59) - PLID 45842 - Added PrintData
				rs = CreateParamRecordset(pCon,
					"SELECT InkData, InkImagePathOverride, InkImageTypeOverride, ImageTextData, PrintData \r\n"
					//"	ZoomLevel, OffsetX, OffsetY \r\n"
					"FROM EMRDetailsT WHERE EMRDetailsT.ID = {INT} \r\n" + strDetailStampSql
					, arynDetailIDs.GetAt(0), arynDetailIDs.GetAt(0));
				rsDetailImageStamps = rs->NextRecordset(NULL);
			}

			if (!rs->eof) {
				FieldsPtr pflds = rs->GetFields();
				// Create a new object from the data
				CEmrItemAdvImageState ais;				
				//
				// (c.haag 2006-11-09 17:46) - PLID 23365 - Whenever we pull InkImageTypeOverride, we could
				// get a null value. If so, it means we pulled it from an EmrInfo item where the user must
				// choose the image to assign to the detail. In that event, we need to assign "blank default
				// values" to the image state. In our program, this means clearing the path and assigning the
				// image type of itDiagram.

				// (c.haag 2007-02-09 15:39) - PLID 23365 - The previous comment is wrong. The simple story
				// is this: EmrDetailsT.InkImageTypeOverride will be non-null if there is a detail-specific
				// image to override the EmrInfoT-specific image. If it is null, we must pull the image data
				// from EmrInfoT later on. What's important here is that ais reflects the state of the detail
				// as it is in data.
				//
				if (itUndefined == (ais.m_eitImageTypeOverride = (eImageType)VarLong(pflds->GetItem("InkImageTypeOverride")->GetValue(), -1))) {
					ais.m_strImagePathOverride.Empty();				
				} else {
					ais.m_strImagePathOverride = VarString(pflds->GetItem("InkImagePathOverride")->GetValue(), "");
				}
				ais.m_varInkData = pflds->GetItem("InkData")->GetValue();
				ais.m_varTextData = pflds->GetItem("ImageTextData")->GetValue();
				ais.m_varPrintData = pflds->GetItem("PrintData")->GetValue();
				
				// (r.gonet 2013-04-08 17:48) - PLID 56150 - Technically, these serialized fields should all be NULL if there is no data for them but
				//  ensure that we handle cases of bad data where the field is an empty byte array.
				if(ais.m_varInkData.vt != VT_NULL && GetElementCountFromSafeArrayVariant(ais.m_varInkData) == 0) {
					ais.m_varInkData = g_cvarNull;
				}
				if(ais.m_varTextData.vt != VT_NULL && GetElementCountFromSafeArrayVariant(ais.m_varTextData) == 0) {
					ais.m_varTextData = g_cvarNull;
				}
				if(ais.m_varPrintData.vt != VT_NULL && GetElementCountFromSafeArrayVariant(ais.m_varPrintData) == 0) {
					ais.m_varPrintData = g_cvarNull;
				}
				/* (r.gonet 05/31/2011) - PLID 43896 - Put back in when we want to save and restore zoom and pan offsets
				ais.m_dZoomLevel = VarFloat(pflds->GetItem("ZoomLevel")->GetValue(), -1);
				ais.m_nOffsetX = VarLong(pflds->GetItem("OffsetX")->GetValue(), 0);
				ais.m_nOffsetY = VarLong(pflds->GetItem("OffsetY")->GetValue(), 0);
				*/
				if(rsDetailImageStamps != NULL) {
					// (z.manning 2010-02-23 15:21) - PLID 37412 - We also need to load the detail image stamp part of the
					// state.
					CEmrDetailImageStampArray arypDetailStamps;
					for(; !rsDetailImageStamps->eof; rsDetailImageStamps->MoveNext()) {
						arypDetailStamps.Add(new EmrDetailImageStamp(rsDetailImageStamps->GetFields()));
					}
					ais.m_varDetailImageStamps = arypDetailStamps.GetAsVariant();
					arypDetailStamps.Clear();
				}
				return ais.AsSafeArrayVariant();
			} 
			else {
				// No detail!  Why was this function called with a non-existent detail id?
				ASSERT(FALSE);
				return varNull;
			}
		}
		break;
	case eitSlider:
		{
			// (z.manning 2011-02-24 12:26) - PLID 42579 - This type does note support loading its state from
			// more than one previous detail.
			if(arynDetailIDs.GetSize() > 1) {
				ASSERT(FALSE);
				AfxThrowNxException("EMR info type %d does not support loading state from multiple details", nEMRInfoDatatype);
			}

			// (j.jones 2007-08-02 10:16) - PLID 26912 - if prsInfo was passed in, check that the recordset
			if(lprsInfo) {
				_RecordsetPtr rs = lprsInfo;
				//there are no current cases where the passed in recordset would not know the value
				return rs->Fields->Item["SliderValue"]->Value;
			}

			//if we had no recordset, then we need to query ourselves

			// (j.jones 2007-07-30 14:59) - PLID 26878 - converted to use a parameter query
			_CommandPtr pCmd = OpenParamQuery(pCon, "SELECT SliderValue FROM EmrDetailsT WHERE ID = ?");
			AddParameterLong(pCmd, "DetailID", arynDetailIDs.GetAt(0));

			_RecordsetPtr rs = CreateRecordset(pCmd);
			if(!rs->eof) {
				return rs->Fields->GetItem("SliderValue")->Value;
			}
			else {
				//No detail!  Caramba!
				ASSERT(FALSE);
				return varNull;
			}
		}
		break;
	case eitNarrative:
		{
			// (z.manning 2011-02-24 12:26) - PLID 42579 - This type does note support loading its state from
			// more than one previous detail.
			if(arynDetailIDs.GetSize() > 1) {
				ASSERT(FALSE);
				AfxThrowNxException("EMR info type %d does not support loading state from multiple details", nEMRInfoDatatype);
			}

			// (j.jones 2007-08-02 10:16) - PLID 26912 - if prsInfo was passed in, check that the recordset
			if(lprsInfo) {
				_RecordsetPtr rs = lprsInfo;
				//there are no current cases where the passed in recordset would not know the value
				//TES 11/7/2007 - PLID 27979 - VS2008 - Need to explicitly return a _variant_t.  I'm choosing
				// to not just return the Value, since that would mean that, in the case where the value was NULL,
				// the behavior would change from returning a VT_BSTR representing an empty string, to VT_NULL,
				// and I don't want to change behavior, even though that seems like the more correct behavior here.
				return _variant_t(AdoFldString(rs, "Text", ""));
			}

			//if we had no recordset, then we need to query ourselves

			// (j.jones 2007-07-30 14:59) - PLID 26878 - converted to use a parameter query
			_CommandPtr pCmd = OpenParamQuery(pCon, "SELECT Text FROM EmrDetailsT WHERE ID = ?");
			AddParameterLong(pCmd, "DetailID", arynDetailIDs.GetAt(0));

			_RecordsetPtr rs = CreateRecordset(pCmd);
			if (!rs->eof) {
				// Return the string
				//TES 11/7/2007 - PLID 27979 - VS2008 - Need to explicitly return a _variant_t.  I'm choosing
				// to not just return the Value, since that would mean that, in the case where the value was NULL,
				// the behavior would change from returning a VT_BSTR representing an empty string, to VT_NULL,
				// and I don't want to change behavior, even though that seems like the more correct behavior here.
				return _variant_t(VarString(rs->GetFields()->GetItem("Text")->GetValue(), ""));
			} else {
				// No detail!  Why was this function called with a non-existent detail id?
				ASSERT(FALSE);
				return varNull;
			}
		}
		break;
	case eitTable:
		{
			//the format is X;Y;Data;Repeat...;

			//TES 3/28/2006 - We need to order properly; that way, if the varState gets recreated, it will still be
			//the same (provided the actual data really hasn't changed).
			// (c.haag 2007-02-05 15:31) - PLID 24423 - Cleaned up the sorting and supported special sorting for Current Medication details
			// (c.haag 2007-04-03 08:59) - PLID 25468 - Expanded the special sorting described above to special Allergy items
			//DRT 7/26/2007 - PLID 26830 - Parameterized to speed execution.
			// (z.manning 2010-02-18 09:24) - PLID 37427 - Added EmrDetailImageStampID
			// (z.manning 2010-02-18 16:19) - PLID 37412 - We need to left join the EmrDataT table for rows as tables rows
			// are no longer required to have an EmrDataID
			// (z.manning 2011-02-24 13:08) - PLID 42579 - Reworked this to support multipe details
			// (z.manning 2011-03-02 15:10) - PLID 42335 - Added EmrImageStampID
			_RecordsetPtr rs = CreateParamRecordset(pCon, 
				"SELECT EmrDetailsT.ID AS EmrDetailID, EmrDetailsT.ID, EMRDataID_X, EMRDataID_Y, EMRDetailTableDataT.Data \r\n"
				"	, EmrDetailImageStampID, ImageDetail.EmrInfoMasterID AS ImageInfoMasterID, EmrDetailImageStampsT.EmrImageStampID \r\n"
				"FROM EMRDetailTableDataT \r\n"
				"INNER JOIN EmrDetailsT ON EMRDetailTableDataT.EmrDetailID = EmrDetailsT.ID \r\n"
				"INNER JOIN EmrMasterT ON EmrDetailsT.EmrID = EmrMasterT.ID \r\n"
				"LEFT JOIN EmrDataT RowData ON EMRDetailTableDataT.EMRDataID_X = RowData.ID \r\n"
				"INNER JOIN EmrDataT ColumnData ON EMRDetailTableDataT.EMRDataID_Y = ColumnData.ID \r\n"
				"LEFT JOIN EmrDetailImageStampsT ON EmrDetailTableDataT.EmrDetailImageStampID = EmrDetailImageStampsT.ID AND EmrDetailImageStampsT.Deleted = 0 \r\n"
				"LEFT JOIN ( \r\n"
				"	SELECT EmrDetailsT.ID, EmrInfoT.EmrInfoMasterID \r\n"
				"	FROM EmrDetailsT \r\n"
				"	INNER JOIN EmrInfoT ON EmrDetailsT.EmrInfoID = EmrInfoT.ID \r\n"
				"	) ImageDetail ON EmrDetailImageStampsT.EmrDetailID = ImageDetail.ID \r\n"
				"WHERE EMRDetailTableDataT.EMRDetailID IN ({INTARRAY}) \r\n"
				// (z.manning 2011-02-25 12:25) - PLID 42579 - We must load the detail in order with most recent first
				// in order for smart stamp table states to load properly.
				"ORDER BY EmrMasterT.Date DESC, EmrMasterT.ID DESC, EmrDetailsT.ID DESC, ImageDetail.ID DESC \r\n"
				// (z.manning 2011-02-25 12:12) - PLID 42579 - There is no need to order by the content when loading the state.
				//"	COALESCE(EmrDetailImageStampsT.OrderIndex, 0), \r\n"
				//"	(CASE WHEN @AutoAlphabetize = 1 THEN CASE WHEN @DataSubType IN (1,2) AND RowData.ListType <> 2 THEN RowData.SortOrder ELSE -1 END ELSE RowData.SortOrder END), \r\n"
				//"	(CASE WHEN @AutoAlphabetize = 1 THEN CASE WHEN @DataSubType IN (1,2) AND RowData.ListType <> 2 THEN '' ELSE RowData.Data END ELSE '' END), \r\n"
				//"	(CASE WHEN @AutoAlphabetize = 1 THEN -1 ELSE ColumnData.SortOrder END), \r\n"
				//"	(CASE WHEN @AutoAlphabetize = 1 THEN ColumnData.Data ELSE '' END) \r\n"
				, arynDetailIDs);
			if(rs->eof) {
				return "";
			}
			else {
				CString strState = LoadTableStateFromRecordset(rs, FALSE, pCon);
				return _variant_t(strState);
			}
		}
		break;
	default:
		// Unexpected
		ASSERT(FALSE);
		return varNull;
		break;
	}
}

// (z.manning 2011-02-24 13:35) - PLID 42579 - This function is used to load a table's state. Loading a table staet
// is now far more complicated because we may potentially load various parts of the state from multiple tables for
// smart stamp tables. Note: The caller is responsible for determing what possible details we may need to load from.
// Caller also must pass details in the correct order (i.e. most recent detail first).
CString LoadTableStateFromRecordset(ADODB::_Recordset *prsTableStates, BOOL bFromOldInfoID, IN ADODB::_Connection *lpCon)
{
	_RecordsetPtr prs(prsTableStates);

	// (z.manning 2011-02-25 09:26) - PLID 42579 - This struct stores info about 1 element (i.e. 1 table cell) of the state.
	struct TableStateData
	{
		long nEmrDataID_X;
		long nEmrDataID_Y;
		CString strData;
		long nDetailImageStampID;
		long nStampID;

		// (z.manning 2011-02-25 09:42) - PLID 42579 - These variable are needed when loading from an old version
		// of the info item.
		long nOldEmrDataID_X;
		long nOldEmrDataID_Y;
		long nListType;
		long nOldListType;
	};
	// (z.manning 2011-02-25 09:27) - PLID 42579 - This struct allows us to store state elements separately
	// based on the smart stamp image item that the smart stamp table row came from. For normal (EmrDataT)
	// table rows, the data is stored here too with varInfoMasterID as VT_NULL.
	struct TableStateLinkedImage
	{
		_variant_t varInfoMasterID;
		CArray<TableStateData*,TableStateData*> arypState;

		~TableStateLinkedImage()
		{
			for(int i = 0; i < arypState.GetSize(); i++) {
				delete arypState.GetAt(i);
			}
		}
	};
	// (z.manning 2011-02-25 09:29) - PLID 42579 - The primary struct for storing the state info for a table detail.
	struct TableStateDetail
	{
		long nID;
		CArray<TableStateLinkedImage*,TableStateLinkedImage*> arypImages;

		~TableStateDetail()
		{
			for(int i = 0; i < arypImages.GetSize(); i++) {
				delete arypImages.GetAt(i);
			}
		}
	};

	TableStateDetail *pDetail = NULL;
	TableStateLinkedImage *pImage = NULL;
	// (z.manning 2011-02-25 09:32) - PLID 42579 - Since we are potentiall loading from multiple previous
	// table details, we need an array of detail state data.
	CArray<TableStateDetail*,TableStateDetail*> arypDetails;
	// (z.manning 2011-02-25 09:32) - PLID 42579 - When loading the state from an old version of the info item
	// the query runs in a cursor, so we may have multiple recordsets here.
	for(; prs != NULL; prs = prs->NextRecordset(NULL))
	{
		for(; !prs->eof; prs->MoveNext())
		{
			long nTableDetailID = AdoFldLong(prs->GetFields(), "EmrDetailID");
			if(pDetail == NULL || nTableDetailID != pDetail->nID) {
				// (z.manning 2011-02-25 09:34) - PLID 42579 - We're on a new detail so allocate memory for it
				// and add it to the array.
				pDetail = new TableStateDetail;
				pDetail->nID = nTableDetailID;
				arypDetails.Add(pDetail);
				// (z.manning 2011-02-25 09:34) - PLID 42579 - Since we started a new detail, reset the linked image pointer.
				pImage = NULL;
			}
			_variant_t varImageInfoMasterID = prs->GetFields()->GetItem("ImageInfoMasterID")->GetValue();
			if(pImage == NULL || pImage->varInfoMasterID != varImageInfoMasterID) {
				// (z.manning 2011-02-25 09:36) - PLID 42579 - We are on a different linked image for this detail
				// so allocate a new object and add it to the array.
				// NOTE: Since we depend on these being loaded in order of linked image within each detail, the
				// caller must be sure to order by this when creating the recordset.
				pImage = new TableStateLinkedImage;
				pImage->varInfoMasterID = varImageInfoMasterID;
				pDetail->arypImages.Add(pImage);
			}
			// (z.manning 2011-02-25 09:41) - PLID 42579 - Now load the state data for this iteration of the recordset.
			TableStateData *pData = new TableStateData;
			pData->nEmrDataID_X = AdoFldLong(prs->GetFields(), "EMRDataID_X", -1);
			// (z.manning 2011-03-15 17:36) - PLID 42856 - This needs a default parameter because we may be loading
			// the state from an old info ID and it's possible columns were deleted in the new version of the item.
			pData->nEmrDataID_Y = AdoFldLong(prs->GetFields(), "EMRDataID_Y", -1);
			pData->strData = AdoFldString(prs->GetFields(), "Data", "");
			pData->nDetailImageStampID = AdoFldLong(prs->GetFields(), "EmrDetailImageStampID", -1);
			pData->nStampID = AdoFldLong(prs, "EmrImageStampID", -1);
			if(bFromOldInfoID) {
				// (z.manning 2011-02-25 09:41) - PLID 42579 - We need to load more fields if we're loading
				// from an old version of the info item.
				pData->nOldEmrDataID_X = AdoFldLong(prs->GetFields(), "OldEmrDataID_X", -1);
				pData->nOldEmrDataID_Y = AdoFldLong(prs->GetFields(), "OldEmrDataID_Y", -1);
				pData->nListType = AdoFldLong(prs->GetFields(), "ListType", -1);
				pData->nOldListType = AdoFldLong(prs->GetFields(), "OldListType", -1);
			}
			pImage->arypState.Add(pData);
		}
	}

	// (z.manning 2011-02-25 09:42) - PLID 42579 - Go time. Let's go through all the state data
	// we just loaded and construct a table state based off of it.
	CString strState;
	CMap<long,long,bool,bool> mapUsedImageInfoMasterIDs;
	for(int nDetailIndex = 0; nDetailIndex < arypDetails.GetSize(); nDetailIndex++)
	{
		TableStateDetail *pDetail = arypDetails.GetAt(nDetailIndex);
		for(int nImageIndex = 0; nImageIndex < pDetail->arypImages.GetSize(); nImageIndex++)
		{
			TableStateLinkedImage *pImage = pDetail->arypImages.GetAt(nImageIndex);
			long nImageInfoMasterID = VarLong(pImage->varInfoMasterID, -1);
			for(int nStateIndex = 0; nStateIndex < pImage->arypState.GetSize(); nStateIndex++)
			{
				TableStateData *pData = pImage->arypState.GetAt(nStateIndex);
				BOOL bOkToAppendData = TRUE;

				if(pData->nEmrDataID_Y == -1) {
					// (z.manning 2011-03-15 18:01) - PLID 42856 - We must have a valid EmrDataID_Y. If we don't it likely
					// means that we are loading from an old info ID and the EmrDataT object has been deleted sometime between
					// the old version and the current version of the item.
					bOkToAppendData = FALSE;
				}

				if(nImageInfoMasterID == -1) {
					// (z.manning 2011-02-25 09:45) - PLID 42579 - This is a normal (EmrDataT) row so we only
					// want to load state data from it if it's the most recent table detail which had better
					// be at index zero of the detail array.
					if(nDetailIndex > 0) {
						bOkToAppendData = FALSE;
					}
				}
				else {
					// (z.manning 2011-02-25 09:46) - PLID 42579 - This is a smart stamp table row. We want to load state data
					// from it as long as we have not loaded state data from this same image item already.
					bool bDummy;
					if(mapUsedImageInfoMasterIDs.Lookup(nImageInfoMasterID, bDummy)) {
						bOkToAppendData = FALSE;
					}
				}

				// (z.manning 2011-02-25 09:49) - PLID 42579 - Check and see if we are loading from an old info ID as
				// we need to do additional processing. Since we are potentially loading from multiple details we also
				// check the EMR data IDs of the old and new info item because it's possible that some, but not all of the
				// details were loading from are on the same version.
				if(bFromOldInfoID && bOkToAppendData && (pData->nOldEmrDataID_Y != pData->nEmrDataID_Y || pData->nOldEmrDataID_X != pData->nEmrDataID_X))
				{
					if(pData->nOldListType != pData->nListType)
					{
						if(pData->nOldListType == LIST_TYPE_DROPDOWN && pData->nListType == LIST_TYPE_TEXT)
						{
							// (z.manning 2011-04-26 11:46) - PLID 37604 - We now support column changes from dropdown
							// to text. However, we have to get the actual dropdown text values since we only store a 
							// delimited ID list in data for dropdown columns.
							if(!pData->strData.IsEmpty()) {
								_RecordsetPtr prs = CreateParamRecordset(lpCon, 
									"SELECT Data FROM EmrTableDropdownInfoT \r\n"
									"WHERE ID IN ({INTSTRING}) \r\n"
									, pData->strData);
								CString strTextData;
								for(; !prs->eof; prs->MoveNext()) {
									if(!strTextData.IsEmpty()) {
										strTextData += ", ";
									}
									strTextData += AdoFldString(prs, "Data", "");
								}
								pData->strData = strTextData;
							}
						}
						else {
							// (z.manning 2011-02-25 09:52) - PLID 42579 - If the column list type has changed on the new
							// version of the info item we do not want to attempt to load the state for that column.
							bOkToAppendData = FALSE;
						}
					}
					else if(pData->nListType == LIST_TYPE_DROPDOWN && !pData->strData.IsEmpty()) {
						// (z.manning 2011-02-25 09:55) - PLID 42579 - If this is a dropdown column then we need to
						// map the selected dropdown IDs to the new version of the item.
						//
						// PLID 42592 - TODO: Optimize this to not query data every single time.
						pData->strData = MapDropdownIDTextToNewDataID(pData->strData, pData->nEmrDataID_Y, lpCon);
					}
				}

				if(bOkToAppendData) {
					// (z.manning 2011-02-25 10:00) - PLID 42579 - As long as it's ok to do so, let's go ahead and add this
					// state element to the remembered table state.
					AppendTableStateWithUnformattedElement(strState, pData->nEmrDataID_X, pData->nEmrDataID_Y, pData->strData, pData->nDetailImageStampID, NULL, pData->nStampID);
				}
			}

			// (z.manning 2011-02-25 10:01) - PLID 42579 - Make sure we do not load data from the same image item again.
			mapUsedImageInfoMasterIDs.SetAt(nImageInfoMasterID, true);
		}
	}

	// (z.manning 2011-02-25 10:01) - PLID 42579 - Clean up any memory we allocated.
	for(int i = 0; i < arypDetails.GetSize(); i++) {
		delete arypDetails.GetAt(i);
	}
	
	return strState;
}

// (z.manning 2011-10-21 16:59) - PLID 44649 - Added bUseUndefinedImageType
_variant_t LoadEMRDetailStateBlank(EmrInfoType nDatatype, BOOL bUseUndefinedImageType /* = FALSE */)
{
	_variant_t varNull;
	varNull.vt = VT_NULL;

	switch(nDatatype) {
	case eitText:
		return _bstr_t("");
		break;
	case eitSingleList:
		return _bstr_t("");
		break;
	case eitMultiList:
		return _bstr_t("");
		break;
	case eitImage:
		{
			CEmrItemAdvImageState ais;
			if(bUseUndefinedImageType) {
				// (z.manning 2011-10-21 17:00) - PLID 44649 - We want to set the image type override to undefined.
				//
				// Note: It may make sense to always set the image type to undefined here as that would be more
				// blank than the default type (itDiagram) but I did not want to risk any unintended consequences
				// by doing this all the time.
				ais.m_eitImageTypeOverride = itUndefined;
			}
			return ais.AsSafeArrayVariant();
		}
		break;
	case eitSlider:
		{
			return varNull;
		}
		break;
	case eitNarrative:
		return _bstr_t("");
		break;
	case eitTable:
		return _bstr_t("");
		break;
	default:
		ASSERT(FALSE);
		return varNull;
		break;
	}
}

// (z.manning 2011-02-24 17:06) - PLID 42579
// (z.manning 2011-10-20 13:20) - PLID 44649 - Added info sub type param
// (z.manning 2011-11-16 12:52) - PLID 38130 - Added output param for the remembered detail ID
BOOL TryLoadDetailStateFromExistingByInfoID(const long nEmrInfoID, const EmrInfoType eInfoType, const EmrInfoSubType eInfoSubType, const long nPatientID, const long nEmrGroupID, BOOL bRememberForEmr, ADODB::_Connection *lpCon, OUT _variant_t &varState, OUT long &nRememberedDetailID)
{
	const char* szSubType = "";
	if(eInfoType == eitImage) {
		// (z.manning 2011-10-20 14:58) - PLID 44649 - For images we need to also match the subtype since we can't
		// load 2D states on 3D images or vice versa.
		szSubType = "AND DataSubType = @nDataSubType ";
	}

	// (z.manning 2011-10-05 16:59) - PLID 45842 - Added PrintData
	CString strCommonSelectClause;
	strCommonSelectClause.Format(
		"EmrDetailsT.ID, EMRInfoID, Text, SliderValue, InkData, InkImagePathOverride, InkImageTypeOverride, ImageTextData, EmrDetailsT.PrintData, \r\n"
		//"		ZoomLevel, OffsetX, OffsetY, \r\n"
		// (z.manning 2010-02-23 14:38) - PLID 37412 - Load whether or not detail has detail image stamps
		//"		CASE WHEN EmrDetailsT.ID IN (SELECT EmrDetailID FROM EmrDetailImageStampsT WHERE EmrDetailImageStampsT.Deleted = 0) THEN CONVERT(bit, 1) ELSE CONVERT(bit, 0) END AS Detail_HasDetailImageStamps, "
		//"		CASE WHEN EmrDetailsT.ID IN (SELECT A.EMRDetailID FROM EmrSelectT A) THEN 1 ELSE 0 END AS Detail_HasListSelections \r\n"
		// (r.gonet 12/18/2012) - PLID 54254 - Use left joins rather than IN for a less time consuming query.
		"		CASE WHEN EmrDetailImageStampsQ.EmrDetailID IS NOT NULL THEN CONVERT(BIT, 1) ELSE CONVERT(BIT, 0) END AS Detail_HasDetailImageStamps, "
		"		CASE WHEN EmrSelectQ.EmrDetailID IS NOT NULL THEN 1 ELSE 0 END AS Detail_HasListSelections \r\n" 
		"	FROM (\r\n"		

		// (a.walling 2013-03-18 16:00) - PLID 55733 - Removing temp tables for better exec plan, less IO, less tempdb contention. Now we use a query directly
		// for the detail population rather than populate a detail temp table.
		"SELECT EmrDetailsT.ID, dbo.AsDateNoTime(EmrMasterT.Date) AS EmrMasterDate, EmrMasterT.ID AS EmrMasterID\r\n"
		"FROM EmrDetailsT \r\n"
		"INNER JOIN EmrMasterT ON EmrDetailsT.EmrID = EmrMasterT.ID \r\n"
		"WHERE PatientID = @nPatientID \r\n"
		" AND (@nRememberForEMR = 0 OR EmrMasterT.EMRGroupID = @nEMRGroupID) \r\n"
		" AND EMRDetailsT.Deleted = 0 AND EMRMasterT.Deleted = 0 \r\n"
		// (a.walling 2013-03-18 15:38) - PLID 55730 - This used to use a temp table for the emrinfo IDs and the same query below using = if the count was 1
		// SQL will assume temp table variables have 1 row, so this was redundant and used the same exec plan. If there were lots of rows, it would perform
		// poorly, and do so N times for N records in the temp table.
		" AND EMRInfoID IN (\r\n"		
			// Get the set of all EmrInfoT.IDs who are children of the same master record as our detail's master record
			// (a.walling 2013-03-18 15:38) - PLID 55730 - Using this here directly rather than populating a temp table allows this query
			// to get proper cardinality estimates and use a better execution plan, avoiding nested loops when there are many copies. This eliminates
			// thousands of seeks, and brought the exec time of this query in the worst case found with Bryn Mawr's data down to 30ms from 250ms
			"SELECT ID FROM EmrInfoT WHERE EmrInfoMasterID = @nEmrInfoMasterID AND DataType = @nDataType %s"
		") \r\n"

		"	) T INNER JOIN EmrDetailsT ON T.ID = EmrDetailsT.ID \r\n"
		"	LEFT JOIN \r\n"
		"	( \r\n"
		"		SELECT EmrDetailImageStampsT.EmrDetailID \r\n"
		"		FROM EmrDetailImageStampsT \r\n" 
		"		WHERE EmrDetailImageStampsT.Deleted = 0 \r\n"
		"		GROUP BY EmrDetailImageStampsT.EmrDetailID \r\n"
		"	) EmrDetailImageStampsQ ON EmrDetailImageStampsQ.EmrDetailID = EmrDetailsT.ID \r\n"
		"	LEFT JOIN \r\n"
		"	( \r\n"
		"		SELECT EmrSelectT.EmrDetailID \r\n"
		"		FROM EmrSelectT \r\n"
		"		GROUP BY EmrSelectT.EmrDetailID \r\n"
		"	) EmrSelectQ ON EmrSelectQ.EmrDetailID = EmrDetailsT.ID \r\n"
		, szSubType
	);
	_RecordsetPtr rsExistingDetail = CreateParamRecordset(lpCon, 
		"SET NOCOUNT ON \r\n"
		"\r\n"
		"DECLARE @nPrimaryEmrInfoID INT \r\n"
		"SET @nPrimaryEmrInfoID = {INT} \r\n"
		"DECLARE @nEmrInfoMasterID INT \r\n"
		"SET @nEmrInfoMasterID = (SELECT EmrInfoMasterID FROM EmrInfoT SourceItem WHERE ID = @nPrimaryEmrInfoID) \r\n"
		"DECLARE @nDataType INT \r\n"
		"SET @nDataType = {INT} \r\n"
		"DECLARE @nDataSubType INT \r\n"
		"SET @nDataSubType = {INT} \r\n"
		// Now get the set of all non-deleted details with any of those EmrInfoIDs for our patient
		"DECLARE @nPatientID INT \r\n"
		"SET @nPatientID = {INT} \r\n"
		"DECLARE @nEMRGroupID INT \r\n"
		"SET @nEMRGroupID = {INT} \r\n"
		"DECLARE @nRememberForEMR INT \r\n"
		"SET @nRememberForEMR = {INT} \r\n"
		// (z.manning 2011-02-24 11:57) - PLID 42579 - We must now select all details when loading a smart stamp table's
		// remembered state.
		"DECLARE @bSelectAllDetails BIT \r\n"
		"IF @nDataType = {CONST} AND EXISTS (SELECT 1 FROM EmrInfoT WHERE SmartStampsEnabled = 1 AND ChildEmrInfoMasterID = @nEmrInfoMasterID) BEGIN \r\n"
		"	SET @bSelectAllDetails = 1 \r\n"
		"END ELSE BEGIN \r\n"
		"	SET @bSelectAllDetails = 0 \r\n"
		"END \r\n"
		// (a.walling 2013-03-18 16:00) - PLID 55733 - Removing temp tables for better exec plan, less IO, less tempdb contention. Now we use a query directly		
		"\r\n"
		"SET NOCOUNT OFF \r\n"
		// Ok, we now know all the matching detail IDs, select the EmrDetailsT fields for the "last" one of them
		// (z.manning 2011-02-24 12:19) - PLID 42579 - We now sometimes select all the EmrDetailsT records.
		// Note: When selecting all details we don't have an order clause as whatever loads these details is
		// responsible for ordering them.
		"\r\n"
		"IF @bSelectAllDetails = 0 BEGIN \r\n"
		"	SELECT TOP 1 {CONST_STRING} "
		"	ORDER BY EmrMasterDate DESC, EmrMasterID DESC, EmrDetailsT.ID DESC \r\n"
		"END ELSE BEGIN \r\n"
		"	SELECT {CONST_STRING} \r\n"
		"END \r\n"
		, nEmrInfoID, eInfoType, eInfoSubType, nPatientID, nEmrGroupID, bRememberForEmr ? 1 : 0, eitTable
		, strCommonSelectClause, strCommonSelectClause);

	// (z.manning 2011-02-24 12:23) - PLID 42549 - We now potentially load the state of a detail from multiple
	// instances of previous copies of the same item so I changed this code accordingly.
	CArray<long,long> arynDetailIDs;
	CArray<long,long> arynInfoIDs;
	BOOL bAtLeastOneInfoIDDoesNotMatch = FALSE;
	for(; !rsExistingDetail->eof; rsExistingDetail->MoveNext()) {
		long nEMNDetailID = AdoFldLong(rsExistingDetail, "ID");
		long nReturnedEMRInfoID = AdoFldLong(rsExistingDetail, "EMRInfoID");
		arynDetailIDs.Add(nEMNDetailID);
		arynInfoIDs.Add(nReturnedEMRInfoID);
		if(nReturnedEMRInfoID != nEmrInfoID) {
			bAtLeastOneInfoIDDoesNotMatch = TRUE;
		}
	}

	if(arynDetailIDs.GetSize() > 0)
	{
		if(bAtLeastOneInfoIDDoesNotMatch) {
			//if the InfoIDs differ, the load needs to map the old state to the new state
			varState = LoadEMRDetailStateFromOldInfoID(arynDetailIDs, arynInfoIDs, nEmrInfoID, eInfoType, eInfoSubType, lpCon);
		}
		else {
			//if the InfoIDs are the same, then the normal load can be used
			// (j.jones 2007-08-02 10:25) - PLID 26912 - pass in rsExistingDetail to be reused
			rsExistingDetail->MoveFirst();
			varState = LoadEMRDetailState(rsExistingDetail, arynDetailIDs, eInfoType, lpCon);
		}

		//TES 7/9/2012 - PLID 51359 - If this is an HTML narrative, we need to de-reference it (it has DetailIDs and DataIDs stored inside it, as well
		// as GUIDs which, if we don't update them, won't be GU any more).
		if(eInfoType == eitNarrative) {
			if(varState.vt == VT_BSTR) {
				CString strDetailText = VarString(varState);
				if(!IsRtfNarrative(strDetailText)) {
					NexTech_COM::IHtmlUtilsPtr pUtils;
					pUtils.CreateInstance("NexTech_COM.HtmlUtils");
					if(pUtils == NULL) {
						AfxThrowNxException("Invalid NexTech_Com pointer in EMRUtils::TryLoadDetailStateFromExistingByInfoID()");
					}
					varState = pUtils->GetDereferencedXml(_bstr_t(strDetailText));
				}
			}
		}

		// (z.manning 2011-11-16 12:53) - PLID 38130 - Set the remembered detail ID output parameter. In the event that
		// we have more than one remembered detail we'll just pick one at random since (at this point) it doesn't matter
		// because we only care whether or not a detail was remembered.
		nRememberedDetailID = arynDetailIDs.GetAt(0);

		return TRUE;
	}

	return FALSE;
}

// (j.jones 2007-08-01 14:06) - PLID 26905 - added lprsInfo as a required parameter
// (j.jones 2008-09-22 15:20) - PLID 31408 - added nEMRGroupID as a parameter
// (z.manning 2011-11-16 12:21) - PLID 38130 - Removed default parameters and added an output parameter for 
// the remembered detail ID from which we loaded the state.
_variant_t LoadEMRDetailStateDefault(ADODB::_Recordset *lprsInfo, long nEmrInfoID, long nPatientID, long nEMRGroupID, long nEmrTemplateDetailID, IN ADODB::_Connection *lpCon, OUT long &nRememberedDetailID)
{
	_variant_t varNull;
	varNull.vt = VT_NULL;

	_ConnectionPtr pCon;
	if(lpCon) pCon = lpCon;
	else pCon = GetRemoteData();

	_RecordsetPtr rsEmrInfo;

	// (j.jones 2007-08-01 14:08) - PLID 26905 - if prsInfo was passed in, assign that to our recordset
	if(lprsInfo) {
		rsEmrInfo = lprsInfo;
	}
	else {
		// (j.jones 2007-08-01 14:09) - PLID 26905 - Otherwise load the recordset ourselves.
		// Remember if we add any field to this recordset, we must also add it to the callers
		// that pass in prsInfo to this function.

		//TES 3/28/2006 - Filter out defaults that are for EmrDataT records that are labels or Inactive.

		// (j.jones 2007-07-30 14:12) - PLID 26878 - converted to use a parameter query
		//DRT 2/14/2008 - PLID 28698 - Added an optimization to check if any hotspot actions exist, but this query can only be run for
		//	non-template loading, which is guaranteed to not have hotspots defaulted, so it's always 0 in this case.
		//TES 6/5/2008 - PLID 29416 - The system tables are hardcoded to display as "Remember"ing, but the data doesn't
		// have that flag set, so override the data in that case.
		// (j.jones 2008-09-22 15:12) - PLID 31408 - supported RememberForEMR, which is always disabled when allergy/current meds		
		// (c.haag 2008-10-16 12:25) - PLID 31709 - TableRowsAsFields
		// (j.jones 2010-02-11 15:24) - PLID 37318 - load ChildEMRInfoMasterID and SmartStampsEnabled
		rsEmrInfo = CreateParamRecordset(pCon, "SELECT DataType, EmrInfoT.BackgroundImageFilePath AS Info_BackgroundImageFilePath, "
			"EmrInfoT.BackgroundImageType AS Info_BackgroundImageType, EmrInfoT.DefaultText AS Info_DefaultText, "
			// (j.jones 2010-06-21 12:22) - PLID 37981 - generic tables never remember their values
			"CASE WHEN EmrInfoT.DataSubType IN ({INT}, {INT}) THEN convert(bit,1) WHEN EmrInfoT.DataSubType = {INT} THEN Convert(bit,0) "
			"	ELSE EmrInfoT.RememberForPatient END AS Info_RememberForPatient, "
			"CASE WHEN EmrInfoT.DataSubType IN ({INT}, {INT}, {INT}) THEN convert(bit,0) "
			"	ELSE EmrInfoT.RememberForEMR END AS Info_RememberForEMR, "
			"EmrInfoT.SliderMin AS Info_SliderMin, "
			"EmrInfoT.SliderMax AS Info_SliderMax, EmrInfoT.SliderInc AS Info_SliderInc, "
			"EmrInfoT.TableRowsAsFields, EmrInfoT.DataSubType, "			
			"EmrInfoT.ChildEMRInfoMasterID, EmrInfoT.SmartStampsEnabled, "
			"CASE WHEN EmrInfoT.ID IN (SELECT EMRInfoDefaultsT.EMRInfoID FROM EMRInfoDefaultsT INNER JOIN EMRDataT ON EMRInfoDefaultsT.EMRDataID = EMRDataT.ID WHERE EMRDataT.Inactive = 0 AND EMRDataT.IsLabel = 0) THEN 1 ELSE 0 END AS Info_HasDefaultValue, "
			"NULL AS Template_HasDefaultValue, NULL AS DefaultText, NULL AS SliderValue, 0 AS HasSliderValue, 0 AS HasTemplateHotSpots "
			"FROM EMRInfoT "
			"WHERE EMRInfoT.ID = {INT}",
			eistCurrentMedicationsTable, eistAllergiesTable, eistGenericTable,
			eistCurrentMedicationsTable, eistAllergiesTable, eistGenericTable, nEmrInfoID);
	}

	if(!rsEmrInfo->eof) {
		EmrInfoType nDataType = (EmrInfoType)AdoFldByte(rsEmrInfo, "DataType");
		EmrInfoSubType eDataSubType = (EmrInfoSubType)AdoFldByte(rsEmrInfo, "DataSubType");
		
		//TES 10/27/2004: If this is one of those items that should be remembered per patient, and if this patient already
		//has this item, then use that.
		// (c.haag 2007-01-25 08:13) - PLID 24416 - The patient ID may be -1 if this is a
		// template or there's simply no patient...in which case there is no reason to run
		// the rsExistingDetail query

		// (j.jones 2008-09-22 15:19) - PLID 31408 - Supported RememberForEMR, which will
		// remember items values only if they were on EMNs in the same EMR, as opposed to
		// RememberForPatient which looks at all EMNs the patient has ever had.

		BOOL bRememberForPatient = AdoFldBool(rsEmrInfo, "Info_RememberForPatient");
		BOOL bRememberForEMR = AdoFldBool(rsEmrInfo, "Info_RememberForEMR");

		if((nPatientID > 0 && bRememberForPatient) || (nEMRGroupID != -1 && bRememberForEMR)) {

			//TES 12/5/2006 - PLID 23724 - We can just compare on EmrInfoMasterID now.
			// (j.jones 2006-08-22 11:38) - PLID 22157 - we need to check all previous versions of this Info item
			//CString strAllPreviousInfoIDs = GeneratePastEMRInfoIDs(nEmrInfoID);

			// (a.wetta 2007-03-06 10:38) - PLID 25008 - We want to find the last use of this detail for when it had the same data type as it has
			// now.  If the last data type is not the same as its current data type, the value would be invalid.

			// (j.jones 2007-08-02 10:25) - PLID 26912 - added extra fields to be passed into LoadEMRDetailState

			// (j.jones 2007-07-30 14:12) - PLID 26878 - converted to use a parameter query

			// (b.cardillo 2007-09-12 16:47) - PLID 27367 - The old query we had here was perfectly fine, but for 
			// some reason SQL Server was not coming up with a very optimal execution plan, so on large data it was 
			// taking an inordinate amount of time.  All I've done here is break it into 3 pieces (the middle one 
			// being itself split into two approaches depending on the result of the first piece) so it is forced 
			// to run as efficiently as possible.  The end result is still the "last" matching detail record for 
			// this patient.

			// (j.jones 2008-09-22 17:21) - PLID 31408 - supported remembering values per EMR
			// (z.manning 2011-02-24 17:25) - PLID 42579 - I moved the logic here to its own function.
			_variant_t varState;
			// (z.manning 2011-11-16 12:56) - PLID 38130 - Pass in parameter for remembered detail ID
			if(TryLoadDetailStateFromExistingByInfoID(nEmrInfoID, nDataType, eDataSubType, nPatientID, nEMRGroupID, bRememberForEMR, pCon, varState, nRememberedDetailID)) {
				return varState;
			}
		}
		
		switch(nDataType) {
		case eitText:
			{
				if(nEmrTemplateDetailID == -1) {
					//TES 6/23/2004: Text types now have a default.
					CString strDefault = AdoFldString(rsEmrInfo, "Info_DefaultText", "");
					if(strDefault == "") {
						return LoadEMRDetailStateBlank(nDataType);
					}
					else {
						return _bstr_t(strDefault);
					}
				}
				else {

					// (j.jones 2007-08-01 16:27) - PLID 26905 - if the recordset is based on a
					// template detail, then we already have this information, otherwise the
					// field will be NULL and we'll have to open a new recordset

					_variant_t varDefault = rsEmrInfo->Fields->Item["DefaultText"]->Value;
					if(varDefault.vt == VT_BSTR)
						////TES 11/7/2007 - PLID 27979 - VS2008 - The return value of this function is _variant_t,
						// so it's bizarre that we were converting a _variant_t into a CString before returning it,
						// and equally bizarre that we were passing in a default value after already checking that
						// the vt can't be VT_NULL.  Let's just return the variant directly.
						//return VarString(varDefault, "");
						return varDefault;

					// (j.jones 2007-07-30 14:12) - PLID 26878 - converted to use a parameter query
					_CommandPtr pCmdDefault = OpenParamQuery(pCon,
						"SELECT DefaultText FROM EmrTemplateDetailsT WHERE ID = ?");

					AddParameterLong(pCmdDefault, "TemplateDetailID", nEmrTemplateDetailID);

					_RecordsetPtr rsDefault = CreateRecordset(pCmdDefault);
					if(rsDefault->eof) {
						return LoadEMRDetailStateBlank(nDataType);
					}
					else {
						return _bstr_t(AdoFldString(rsDefault, "DefaultText", ""));
					}
				}
			}
			break;
		case eitSingleList:
			{
				if(nEmrTemplateDetailID == -1) {

					// (j.jones 2007-08-01 14:37) - PLID 26905 - ActiveData.ID is no longer
					// in the rsEmrInfo recordset, instead we have Info_HasDefaultValue which tells us
					// whether a default value exists. Since spawning non-template details with
					// default values isn't too common, this isn't a huge loss.

					long nHasDefaultValue = AdoFldLong(rsEmrInfo, "Info_HasDefaultValue", 0);
					if(nHasDefaultValue == 0)
						return LoadEMRDetailStateBlank(nDataType);

					//otherwise, if we have a default value, we have to load it

					_CommandPtr pCmdData = OpenParamQuery(pCon,
						"SELECT ActiveData.ID "
						"FROM EMRInfoT LEFT JOIN (EMRInfoDefaultsT INNER JOIN "
						"(SELECT * FROM EMRDataT WHERE EmrDataT.Inactive = 0 AND EmrDataT.IsLabel = 0) AS ActiveData "
						"ON EMRInfoDefaultsT.EMRDataID = ActiveData.ID) ON EMRInfoT.ID = EMRInfoDefaultsT.EMRInfoID "
						"WHERE EMRInfoT.ID = ?");

					AddParameterLong(pCmdData, "EMRInfoID", nEmrInfoID);

					_RecordsetPtr rsData = CreateRecordset(pCmdData);
					if(!rsData->eof) {
						long nDefault = AdoFldLong(rsData, "ID", -1);
						if(nDefault != -1)
							//TES 11/7/2007 - PLID 27979 - VS2008 - Need to explicitly return a _variant_t.
							return _variant_t(AsString(nDefault));
					}
					rsData->Close();

					//if we didn't return a value, return the blank value
					return LoadEMRDetailStateBlank(nDataType);
				}
				else {

					// (j.jones 2007-08-01 16:39) - PLID 26905 - if the recordset is based on a 
					// template detail, then it should fill in Template_HasDefaultValue, which tells
					// us whether a default template value exists. If NULL, it means we don't know,
					// so we have to process a recordset anyways. We always processed a recordset here
					// before, so any reduction is good.

					long nHasDefaultValue = AdoFldLong(rsEmrInfo, "Template_HasDefaultValue", 1);
					if(nHasDefaultValue == 0)
						return LoadEMRDetailStateBlank(nDataType);

					// (j.jones 2007-07-30 14:12) - PLID 26878 - converted to use a parameter query
					_CommandPtr pCmdDefault = OpenParamQuery(pCon,
						"SELECT EmrDataID FROM EmrTemplateSelectT WHERE EmrTemplateDetailID = ?");

					AddParameterLong(pCmdDefault, "TemplateDetailID", nEmrTemplateDetailID);

					_RecordsetPtr rsDefault = CreateRecordset(pCmdDefault);
					if(!rsDefault->eof) {
						//TES 11/7/2007 - PLID 27979 - VS2008 - Need to explicitly return a _variant_t.
						return _variant_t(AsString(rsDefault->Fields->GetItem("EmrDataID")->Value));
					}
					else {
						return LoadEMRDetailStateBlank(nDataType);
					}
				}
			}
			break;
		case eitMultiList:
			// (j.jones 2004-10-22 14:47) - Load all the defaults into a semi-colon delimited list
			{
				if(nEmrTemplateDetailID == -1) {
					CString strDefault = "";

					// (j.jones 2007-08-01 14:37) - PLID 26905 - ActiveData.ID is no longer
					// in the rsEmrInfo recordset, instead we have Info_HasDefaultValue which tells us
					// whether a default value exists. Since spawning non-template details with
					// default values isn't too common, this isn't a huge loss.

					long nHasDefaultValue = AdoFldLong(rsEmrInfo, "Info_HasDefaultValue", 0);
					if(nHasDefaultValue == 0)
						return LoadEMRDetailStateBlank(nDataType);

					_CommandPtr pCmdData = OpenParamQuery(pCon,
						"SELECT ActiveData.ID "
						"FROM EMRInfoT LEFT JOIN (EMRInfoDefaultsT INNER JOIN "
						"(SELECT * FROM EMRDataT WHERE EmrDataT.Inactive = 0 AND EmrDataT.IsLabel = 0) AS ActiveData "
						"ON EMRInfoDefaultsT.EMRDataID = ActiveData.ID) ON EMRInfoT.ID = EMRInfoDefaultsT.EMRInfoID "
						"WHERE EMRInfoT.ID = ?");

					AddParameterLong(pCmdData, "EMRInfoID", nEmrInfoID);

					_RecordsetPtr rsData = CreateRecordset(pCmdData);

					if(rsData->eof)
						return LoadEMRDetailStateBlank(nDataType);

					while(!rsData->eof) {
						long nDefault = AdoFldLong(rsData, "ID", -1);
						if(nDefault == -1)
							return LoadEMRDetailStateBlank(nDataType);
						else {
							strDefault += AsString(nDefault);
							strDefault += "; ";
						}
						rsData->MoveNext();
					}
					rsData->Close();

					strDefault.TrimRight("; ");
					//TES 11/7/2007 - PLID 27979 - VS2008 - Need to explicitly return a _variant_t.
					return _variant_t(strDefault);
				}
				else {
					CString strDefault = "";

					// (j.jones 2007-08-01 16:39) - PLID 26905 - if the recordset is based on a 
					// template detail, then it should fill in Template_HasDefaultValue, which tells
					// us whether a default template value exists. If NULL, it means we don't know,
					// so we have to process a recordset anyways. We always processed a recordset here
					// before, so any reduction is good.

					long nHasDefaultValue = AdoFldLong(rsEmrInfo, "Template_HasDefaultValue", 1);
					if(nHasDefaultValue == 0)
						return LoadEMRDetailStateBlank(nDataType);
					
					// (j.jones 2007-07-30 14:12) - PLID 26878 - converted to use a parameter query
					_CommandPtr pCmdDefault = OpenParamQuery(pCon,
						"SELECT EmrDataID FROM EmrTemplateSelectT WHERE EmrTemplateDetailID = ?");

					AddParameterLong(pCmdDefault, "TemplateDetailID", nEmrTemplateDetailID);

					_RecordsetPtr rsDefaults = CreateRecordset(pCmdDefault);
					while(!rsDefaults->eof) {
						long nDefault = VarLong(rsDefaults->GetFields()->GetItem("EmrDataID")->GetValue(), -1);
						if(nDefault == -1)
							return LoadEMRDetailStateBlank(nDataType);
						else {
							strDefault += AsString(nDefault);
							strDefault += "; ";
						}
						rsDefaults->MoveNext();
					}
					strDefault.TrimRight("; ");
					//TES 11/7/2007 - PLID 27979 - VS2008 - Need to explicitly return a _variant_t.
					return _variant_t(strDefault);
				}
			}
			break;
		case eitImage: {
			// Image type info items don't currently have default values.
			//return LoadEMRDetailStateBlank(nDataType);
			//
			// (c.haag 2006-11-09 17:47) - PLID 23365 - Whenever we pull BackgroundImageType, we could
			// get a null value. If so, it means we pulled it from an EmrInfo item where the user must
			// choose the image to assign to the detail. In that event, we need to assign "blank default
			// values" to the image state. In our program, this means clearing the path and assigning the
			// image type of itDiagram.

			// (c.haag 2007-02-09 15:40) - PLID 23365 - The previous comment is wrong. The simple story
			// is that EmrInfoT.BackgroundImageType is NULL if there is no default image for the info
			// item. When the detail is added, it should have no image unless the detail itself is assigned
			// one in its InkImage*Override fields. The user will have to pick an image.
			//
			CEmrItemAdvImageState ais;
			ais.m_eitImageTypeOverride = (eImageType)AdoFldLong(rsEmrInfo, "Info_BackgroundImageType", -1);
			if (itUndefined == ais.m_eitImageTypeOverride) {
				ais.m_strImagePathOverride.Empty();
			} else {
				ais.m_strImagePathOverride = AdoFldString(rsEmrInfo, "Info_BackgroundImageFilePath", "");
			}

			//DRT 2/14/2008 - PLID 28698 - Load the hotspot state.
			if(nEmrTemplateDetailID == -1) {
				//There is no possibility of a default value for non-template images with hotspots.  The default is off for all.
			}
			else
			{
				// (z.manning 2011-10-06 17:48) - PLID 45842 - I reworked this code to use a conditional SQL batch as we now have
				// two possible queries to run here but we may not need to run either of them.
				bool bHotSpots = false, bTemplateDetail = false;
				CParamSqlBatch sqlBatch;
				//Optimization in the loading query, so we don't need to ask for all the hotspots if they are not going to be needed.
				if(AdoFldLong(rsEmrInfo, "HasTemplateHotSpots", 0)) {
					//On a template
					sqlBatch.Add("SELECT EMRImageHotSpotID FROM EMRHotSpotTemplateSelectT WHERE EmrDetailID = {INT}", nEmrTemplateDetailID);
					bHotSpots = true;
				}

				EmrInfoSubType eSubType = (EmrInfoSubType)AdoFldByte(rsEmrInfo, "DataSubType", eistNone);
				if(eSubType == eist3DImage) {
					sqlBatch.Add("SELECT PrintData FROM EmrTemplateDetailsT WHERE EmrTemplateDetailsT.ID = {INT}", nEmrTemplateDetailID);
					bTemplateDetail = true;
				}

				if(!sqlBatch.IsEmpty())
				{
					_RecordsetPtr prsDefaults = sqlBatch.CreateRecordsetNoTransaction(GetRemoteData());

					if(bHotSpots) {
						while(!prsDefaults->eof) {
							//Semicolon delimited list
							ais.m_strSelectedHotSpotData += FormatString("%li;", AdoFldLong(prsDefaults, "EMRImageHotSpotID"));
							prsDefaults->MoveNext();
						}
						prsDefaults = prsDefaults->NextRecordset(NULL);
					}

					if(bTemplateDetail) {
						if(!prsDefaults->eof) {
							// (z.manning 2011-10-06 17:47) - PLID 45842 - Load print data
							ais.m_varPrintData = prsDefaults->GetFields()->GetItem("PrintData")->GetValue();
						}
						prsDefaults = prsDefaults->NextRecordset(NULL);
					}
				}
			}

			return ais.AsSafeArrayVariant();

		   } break;
		case eitSlider:
			
			if(nEmrTemplateDetailID == -1) {
				//Sliders don't have defaults yet (though they probably should)
				return LoadEMRDetailStateBlank(nDataType);
			}
			else {

				// (j.jones 2007-08-01 16:27) - PLID 26905 - If the recordset is based on a
				// template detail, then we already have this information, but you can't tell
				// based on the SliderValue field because NULL is a valid value. So the
				// recordsets that legitimately have a SliderValue define "1 AS HasSliderValue"
				// while recordsets without it define "0 AS HasSliderValue". If 0, we have to
				// query from data.

				long nHasSliderValue = AdoFldLong(rsEmrInfo, "HasSliderValue", 0);
				if(nHasSliderValue == 1)
					return rsEmrInfo->Fields->Item["SliderValue"]->Value;;

				// (j.jones 2007-07-30 14:12) - PLID 26878 - converted to use a parameter query
				_CommandPtr pCmdDefault = OpenParamQuery(pCon,
					"SELECT SliderValue FROM EmrTemplateDetailsT WHERE ID = ?");

				AddParameterLong(pCmdDefault, "TemplateDetailID", nEmrTemplateDetailID);

				_RecordsetPtr rsDefault = CreateRecordset(pCmdDefault);
				if(rsDefault->eof) {
					return LoadEMRDetailStateBlank(nDataType);
				}
				else {
					return rsDefault->Fields->GetItem("SliderValue")->Value;
				}
			}
			break;
		case eitNarrative:
			{
				if(nEmrTemplateDetailID == -1) {
					//TES 6/23/2004: Text types now have a default.
					CString strDefault = AdoFldString(rsEmrInfo, "Info_DefaultText", "");
					if(strDefault == "") {
						return LoadEMRDetailStateBlank(nDataType);
					}
					else {
						return _bstr_t(strDefault);
					}
				}
				else {

					// (j.jones 2007-08-01 16:27) - PLID 26905 - if the recordset is based on a
					// template detail, then we already have this information, otherwise the
					// field will be NULL and we'll have to open a new recordset

					_variant_t varDefault = rsEmrInfo->Fields->Item["DefaultText"]->Value;
					if(varDefault.vt == VT_BSTR)
						////TES 11/7/2007 - PLID 27979 - VS2008 - The return value of this function is _variant_t,
						// so it's bizarre that we were converting a _variant_t into a CString before returning it,
						// and equally bizarre that we were passing in a default value after already checking that
						// the vt can't be VT_NULL.  Let's just return the variant directly.
						//return VarString(varDefault, "");
						return varDefault;

					// (j.jones 2007-07-30 14:12) - PLID 26878 - converted to use a parameter query
					_CommandPtr pCmdDefault = OpenParamQuery(pCon,
						"SELECT DefaultText FROM EmrTemplateDetailsT WHERE ID = ?");

					AddParameterLong(pCmdDefault, "TemplateDetailID", nEmrTemplateDetailID);

					_RecordsetPtr rsDefault = CreateRecordset(pCmdDefault);
					if(rsDefault->eof) {
						return LoadEMRDetailStateBlank(nDataType);
					}
					else {
						return _bstr_t(AdoFldString(rsDefault, "DefaultText", ""));
					}
				}
			}
			break;
		case eitTable:
			{
				// (j.jones 2007-08-01 16:56) - PLID 26905 - returned if not an EMRTemplateDetail
				if(nEmrTemplateDetailID == -1)
					return "";

				//the format is X;Y;Data;Repeat...;
				
				//TES 3/28/2006 - We need to order properly; that way, if the varState gets recreated, it will still be
				//the same (provided the actual data really hasn't changed).
				// (c.haag 2007-02-05 15:31) - PLID 24423 - Cleaned up the sorting and supported special sorting for Current Medication details
				// (c.haag 2007-04-03 08:59) - PLID 25468 - Expanded the special sorting described above to special Allergy items
				//DRT 7/26/2007 - PLID 26830 - Parameterized to speed execution.
				_RecordsetPtr rs = CreateParamRecordset(pCon, 
					"DECLARE @TemplateDetailID INT SET @TemplateDetailID = {INT} "
					"SELECT EMRDataID_X, EMRDataID_Y, EMRTemplateTableDefaultsT.Data "
					"FROM EMRTemplateTableDefaultsT INNER JOIN EmrDataT RowData ON EMRTemplateTableDefaultsT.EMRDataID_X = RowData.ID "
					"INNER JOIN EmrDataT ColumnData ON EMRTemplateTableDefaultsT.EMRDataID_Y = ColumnData.ID "
					"WHERE EMRTemplateDetailID = @TemplateDetailID "
					// (z.manning 2011-02-25 12:58) - PLID 42579 - No need to order by content when loading a table's state
					//"ORDER BY "
					//"(CASE WHEN @AutoAlphabetize = 1 THEN CASE WHEN @DataSubType IN (1,2) AND RowData.ListType <> 2 THEN RowData.SortOrder ELSE -1 END ELSE RowData.SortOrder END), "
					//"	(CASE WHEN @AutoAlphabetize = 1 THEN CASE WHEN @DataSubType IN (1,2) AND RowData.ListType <> 2 THEN '' ELSE RowData.Data END ELSE '' END), "
					//"(CASE WHEN @AutoAlphabetize = 1 THEN -1 ELSE ColumnData.SortOrder END), "
					//"	(CASE WHEN @AutoAlphabetize = 1 THEN ColumnData.Data ELSE '' END) "
					, nEmrTemplateDetailID);
				if (!rs->eof) {
					CString strAns;
					FieldPtr fldDataID_X = rs->GetFields()->GetItem("EMRDataID_X");
					FieldPtr fldDataID_Y = rs->GetFields()->GetItem("EMRDataID_Y");
					FieldPtr fldData = rs->GetFields()->GetItem("Data");
					while (!rs->eof) {
						// (c.haag 2007-08-18 09:02) - PLID 27111 - Add the table element represented by the
						// data fields to strAns
						AppendTableStateWithUnformattedElement(strAns, VarLong(fldDataID_X->GetValue(),-1), VarLong(fldDataID_Y->GetValue()), VarString(fldData->GetValue(), ""), -1, NULL, -1);
						rs->MoveNext();
					}
					//TES 11/7/2007 - PLID 27979 - VS2008 - Need to explicitly return a _variant_t.
					return _variant_t(strAns);
				} else {
					// No selection for this detail.  We used to not allow this, but now "no selection" is valid.
					return "";
				}

				return "";
			}
			break;
		default:
			ASSERT(FALSE);
			return varNull;
			break;
		}
	}
	else {
		ASSERT(FALSE);
		return varNull;
	}
}

// (j.jones 2006-08-22 12:12) - PLID 22157 - When remembering a past item's value, and perhaps other places in the future, 
// we would need to load an old InfoIDs state into a new InfoID. This function will do that.
// (z.manning 2011-02-24 12:15) - PLID 42579 - This now takes an array of detail IDs and info IDs
// (z.manning 2011-10-20 13:20) - PLID 44649 - Added info sub type param
_variant_t LoadEMRDetailStateFromOldInfoID(CArray<long,long> &arynOldEMRDetailIDs, CArray<long,long> &arynOldEMRInfoIDs, long nNewEMRInfoID, EmrInfoType nEMRInfoDatatype, EmrInfoSubType eEMRInfoSubType, OPTIONAL IN ADODB::_Connection *lpCon /*= NULL*/)
{
	// (z.manning 2011-02-24 12:30) - PLID 42579 - The count of both arrays MUST match.
	if(arynOldEMRDetailIDs.GetCount() != arynOldEMRInfoIDs.GetCount()) {
		ASSERT(FALSE);
		AfxThrowNxException("LoadEMRDetailStateFromOldInfoID - Called with different number of detail and info IDs");
	}

	_variant_t varNull;
	varNull.vt = VT_NULL;
	_ConnectionPtr pCon;
	if(lpCon) pCon = lpCon;
	else pCon = GetRemoteData();
	
	// (j.jones 2007-07-23 09:16) - PLID 26773 - added connection pointer for use in threads
	//DRT 9/7/2007 - PLID 27330 - We already know the new info data type, don't make this function query data for it again.
	// (z.manning 2011-02-24 12:35) - PLID 42579 - Reworked this to handle multiple details
	for(int nOldInfoIndex = arynOldEMRInfoIDs.GetSize() - 1; nOldInfoIndex >= 0; nOldInfoIndex--) {
		long nOldEMRInfoID = arynOldEMRInfoIDs.GetAt(nOldInfoIndex);
		if(!AreInfoIDTypesCompatible(nOldEMRInfoID, nNewEMRInfoID, nEMRInfoDatatype, eEMRInfoSubType, pCon)) {
			arynOldEMRInfoIDs.RemoveAt(nOldInfoIndex);
			arynOldEMRDetailIDs.RemoveAt(nOldInfoIndex);
		}
	}

	if(arynOldEMRDetailIDs.GetSize() == 0) {
		_variant_t varBlankState = ::LoadEMRDetailStateBlank(nEMRInfoDatatype);
		return varBlankState;
	}

	switch (nEMRInfoDatatype) {
	case eitText:
		{
			// (z.manning 2011-02-24 12:26) - PLID 42579 - This type does note support loading its state from
			// more than one previous detail.
			if(arynOldEMRDetailIDs.GetSize() > 1) {
				ASSERT(FALSE);
				AfxThrowNxException("EMR info type %d does not support loading state from multiple details (old info ID)", nEMRInfoDatatype);
			}

			//if the type changed, we can't load anything, but if the type is the same,
			//then nothing special needs to be done here to map to the new ID

			// (j.jones 2007-07-30 14:54) - PLID 26878 - converted to use a parameter query
			_CommandPtr pCmd = OpenParamQuery(pCon, "SELECT Text FROM EMRDetailsT WHERE ID = ?");
			AddParameterLong(pCmd, "DetailID", arynOldEMRDetailIDs.GetAt(0));

			_RecordsetPtr rs = CreateRecordset(pCmd);
			if (!rs->eof) {
				// Return the string
				//TES 11/7/2007 - PLID 27979 - VS2008 - Need to explicitly return a _variant_t.  I'm choosing
				// to not just return the Value, since that would mean that, in the case where the value was NULL,
				// the behavior would change from returning a VT_BSTR representing an empty string, to VT_NULL,
				// and I don't want to change behavior, even though that seems like the more correct behavior here.
				return _variant_t(VarString(rs->GetFields()->GetItem("Text")->GetValue(), ""));
			} else {
				// No detail!  Why was this function called with a non-existent detail id?
				ASSERT(FALSE);
				return varNull;
			}
		}
		break;
	case eitSingleList:
		{
			// (z.manning 2011-02-24 12:26) - PLID 42579 - This type does note support loading its state from
			// more than one previous detail.
			if(arynOldEMRDetailIDs.GetSize() > 1) {
				ASSERT(FALSE);
				AfxThrowNxException("EMR info type %d does not support loading state from multiple details (old info ID)", nEMRInfoDatatype);
			}

			//if the type changed from anything other than a multi-select list, we can't load anything,
			//but if the type is the same (or a list), we need to remap data IDs and load appropriately
			//(to an extent - if the old type was multi-select, we're only going to load the first value)

			// (j.jones 2007-07-30 14:54) - PLID 26878 - converted to use a parameter query
			_CommandPtr pCmd = OpenParamQuery(pCon, "SELECT EMRDataID FROM EmrSelectT WHERE EMRDetailID = ?");
			AddParameterLong(pCmd, "DetailID", arynOldEMRDetailIDs.GetAt(0));

			_RecordsetPtr rs = CreateRecordset(pCmd);
			if (!rs->eof) {

				long nOldDataID = VarLong(rs->GetFields()->GetItem("EMRDataID")->GetValue());
				
				//now calculate the new DataID
				// (j.jones 2007-07-23 09:16) - PLID 26773 - added connection pointer for use in threads
				long nNewDataID = CalculateMappedDataID(nOldDataID, nNewEMRInfoID, pCon);

				// Return the data id as a string
				//TES 11/7/2007 - PLID 27979 - VS2008 - Need to explicitly return a _variant_t.
				return _variant_t(AsString(nNewDataID));
			} else {
				// No selection for this detail.  We used to not allow this, but now "no selection" is valid.
				return "";
			}
		}
		break;
	case eitMultiList:
		{
			// (z.manning 2011-02-24 12:26) - PLID 42579 - This type does note support loading its state from
			// more than one previous detail.
			if(arynOldEMRDetailIDs.GetSize() > 1) {
				ASSERT(FALSE);
				AfxThrowNxException("EMR info type %d does not support loading state from multiple details (old info ID)", nEMRInfoDatatype);
			}

			//if the type changed from anything other than single to multi-select, we can't load anything,
			//but if the type is the same (or a list), we need to remap data IDs and load appropriately

			// (j.jones 2007-07-30 14:54) - PLID 26878 - converted to use a parameter query
			_CommandPtr pCmd = OpenParamQuery(pCon, "SELECT EMRDataID FROM EmrSelectT WHERE EMRDetailID = ?");
			AddParameterLong(pCmd, "DetailID", arynOldEMRDetailIDs.GetAt(0));

			_RecordsetPtr rs = CreateRecordset(pCmd);
			if (!rs->eof) {
				// Return the list of data ids as a semicolon-delimited string
				CString strAns;
				FieldPtr fldDataID = rs->GetFields()->GetItem("EMRDataID");
				while (!rs->eof) {
					if (!strAns.IsEmpty()) {
						strAns += "; ";
					}
					CString str;

					long nOldDataID = VarLong(fldDataID->GetValue());
				
					//now calculate the new DataID
					// (j.jones 2007-07-23 09:16) - PLID 26773 - added connection pointer for use in threads
					long nNewDataID = CalculateMappedDataID(nOldDataID, nNewEMRInfoID, pCon);

					str.Format("%li", nNewDataID);
					strAns += str;
					rs->MoveNext();
				}
				//TES 11/7/2007 - PLID 27979 - VS2008 - Need to explicitly return a _variant_t.
				return _variant_t(strAns);
			} else {
				// No selection for this detail.  We used to not allow this, but now "no selection" is valid.
				return "";
			}
		}

		break;
	case eitImage:
		{
			// (z.manning 2011-02-24 12:26) - PLID 42579 - This type does note support loading its state from
			// more than one previous detail.
			if(arynOldEMRDetailIDs.GetSize() > 1) {
				ASSERT(FALSE);
				AfxThrowNxException("EMR info type %d does not support loading state from multiple details (old info ID)", nEMRInfoDatatype);
			}

			//if the type changed, we can't load anything, but if the type is the same,
			//then nothing special needs to be done here to map to the new ID

			//(e.lally 2006-08-31) PLID 22342 - If an item is saved on an EMN and has the value
			//remembered for the patient, when the type of item is changed to something that is not
			//compatible, this function will return and empty variant which we do not handle properly
			//in the rest of the EMR code (for instance when we do a VarDouble or VarString). To prevent this,
			//we should return the proper variant type.
			//Later, we will review the returns for failures in these sections (PLID 22335)
			// (j.jones 2006-10-16 09:46) - PLID 22335 - I've since done that across all these functions

			// (j.jones 2007-07-30 14:54) - PLID 26878 - converted to use a parameter query
			// (z.manning 2011-10-05 16:59) - PLID 45842 - Added PrintData
			_RecordsetPtr rs = CreateParamRecordset(pCon,
				"SELECT InkData, InkImagePathOverride, InkImageTypeOverride, ImageTextData, EmrDetailsT.PrintData \r\n"
				// (r.gonet 05/31/2011) - PLID 43896 - Added zoom related fields
				//"	ZoomLevel, OffsetX, OffsetY \r\n"
				"FROM EMRDetailsT WHERE EMRDetailsT.ID = {INT} \r\n"
				// (z.manning 2011-02-14 12:47) - PLID 42452 - Order stamps by OrderIndex
				// (z.manning 2011-01-27 16:27) - PLID 42335 - Added UsedInTableData
				// (z.manning 2011-09-08 10:24) - PLID 45335 - Added 3D fields
				"SELECT ID, EmrImageStampID, OrderIndex, XPos, YPos, SmartStampTableSpawnRule \r\n"
				"	, CONVERT(bit, CASE WHEN EmrDetailTableDataQ.EmrDetailImageStampID IS NULL THEN 0 ELSE 1 END) AS UsedInTableData \r\n"
				"	, XPos3D, YPos3D, ZPos3D, XNormal, YNormal, ZNormal, HotSpot3D \r\n"
				"FROM EmrDetailImageStampsT \r\n"
				"LEFT JOIN (SELECT DISTINCT EmrDetailTableDataT.EmrDetailImageStampID FROM EmrDetailTableDataT WHERE EmrDetailTableDataT.EmrDetailImageStampID IS NOT NULL) EmrDetailTableDataQ ON EmrDetailImageStampsT.ID = EmrDetailTableDataQ.EmrDetailImageStampID \r\n"
				"WHERE EmrDetailID = {INT} AND EmrDetailImageStampsT.Deleted = 0 \r\n"
				"ORDER BY EmrDetailImageStampsT.OrderIndex \r\n"
				, arynOldEMRDetailIDs.GetAt(0), arynOldEMRDetailIDs.GetAt(0));			
			if (!rs->eof) {
				FieldsPtr pflds = rs->GetFields();
				// Create a new object from the data
				CEmrItemAdvImageState ais;
				//
				// (c.haag 2006-11-09 17:46) - PLID 23365 - Whenever we pull InkImageTypeOverride, we could
				// get a null value. If so, it means we pulled it from an EmrInfo item where the user must
				// choose the image to assign to the detail. In that event, we need to assign "blank default
				// values" to the image state. In our program, this means clearing the path and assigning the
				// image type of itDiagram.

				// (c.haag 2007-02-09 15:44) - PLID 23365 - The previous comment is wrong. The simple story
				// is this: EmrDetailsT.InkImageTypeOverride will be non-null if there is a detail-specific
				// image to override the EmrInfoT-specific image. If it is null, we must pull the image data
				// from EmrInfoT later on. What's important here is that ais reflects the state of the detail
				// as it is in data.
				//
				if (itUndefined == (ais.m_eitImageTypeOverride = (eImageType)VarLong(pflds->GetItem("InkImageTypeOverride")->GetValue(), -1))) {
					ais.m_strImagePathOverride.Empty();
				} else {
					ais.m_strImagePathOverride = VarString(pflds->GetItem("InkImagePathOverride")->GetValue(), "");
				}
				ais.m_varInkData = pflds->GetItem("InkData")->GetValue();
				ais.m_varTextData = pflds->GetItem("ImageTextData")->GetValue();
				ais.m_varPrintData = pflds->GetItem("PrintData")->GetValue();

				// (r.gonet 2013-04-08 17:48) - PLID 56150 - Technically, these serialized fields should all be NULL if there is no data for them but
				//  ensure that we handle cases of bad data where the field is an empty byte array.
				if(ais.m_varInkData.vt != VT_NULL && GetElementCountFromSafeArrayVariant(ais.m_varInkData) == 0) {
					ais.m_varInkData = g_cvarNull;
				}
				if(ais.m_varTextData.vt != VT_NULL && GetElementCountFromSafeArrayVariant(ais.m_varTextData) == 0) {
					ais.m_varTextData = g_cvarNull;
				}
				if(ais.m_varPrintData.vt != VT_NULL && GetElementCountFromSafeArrayVariant(ais.m_varPrintData) == 0) {
					ais.m_varPrintData = g_cvarNull;
				}
				/* (r.gonet 05/31/2011) - PLID 43896 - Put back in when we want to save and restore zoom and pan offsets.
				ais.m_dZoomLevel = VarFloat(pflds->GetItem("ZoomLevel")->GetValue(), -1);
				ais.m_nOffsetX = VarLong(pflds->GetItem("OffsetX")->GetValue(), 0);
				ais.m_nOffsetY = VarLong(pflds->GetItem("OffsetY")->GetValue(), 0);
				*/

				_RecordsetPtr rsDetailImageStamps = rs->NextRecordset(NULL);

				if(rsDetailImageStamps != NULL) {
					// (z.manning 2010-02-23 15:21) - PLID 37412 - We also need to load the detail image stamp part of the
					// state.
					CEmrDetailImageStampArray arypDetailStamps;
					for(; !rsDetailImageStamps->eof; rsDetailImageStamps->MoveNext()) {
						arypDetailStamps.Add(new EmrDetailImageStamp(rsDetailImageStamps->GetFields()));
					}
					ais.m_varDetailImageStamps = arypDetailStamps.GetAsVariant();
					arypDetailStamps.Clear();
				}
				return ais.AsSafeArrayVariant();
			} else {
				// No detail!  Why was this function called with a non-existent detail id?
				ASSERT(FALSE);
				return varNull;
			}
		}
		break;
	case eitSlider:
		{
			// (z.manning 2011-02-24 12:26) - PLID 42579 - This type does note support loading its state from
			// more than one previous detail.
			if(arynOldEMRDetailIDs.GetSize() > 1) {
				ASSERT(FALSE);
				AfxThrowNxException("EMR info type %d does not support loading state from multiple details (old info ID)", nEMRInfoDatatype);
			}

			//if the type changed, we can't load anything, but if the type is the same,
			//then nothing special needs to be done here to map to the new ID

			//(e.lally 2006-08-31) PLID 22342 - If an item is saved on an EMN and has the value
			//remembered for the patient, when the type of item is changed to something that is not
			//compatible, this function will return and empty variant which we do not handle properly
			//in the rest of the EMR code (for instance when we do a VarDouble or VarString). To prevent this,
			//we should return the proper variant type.
			//Later, we will review the returns for failures in these sections (PLID 22335)
			// (j.jones 2006-10-16 09:46) - PLID 22335 - I've since done that across all these functions

			// (j.jones 2007-07-30 14:54) - PLID 26878 - converted to use a parameter query
			_CommandPtr pCmd = OpenParamQuery(pCon, "SELECT SliderValue FROM EmrDetailsT WHERE ID = ?");
			AddParameterLong(pCmd, "DetailID", arynOldEMRDetailIDs.GetAt(0));

			_RecordsetPtr rs = CreateRecordset(pCmd);
			if(!rs->eof) {
				return rs->Fields->GetItem("SliderValue")->Value;
			}
			else {
				//No detail!  Caramba!
				ASSERT(FALSE);
				return varNull;
			}
		}
		break;
	case eitNarrative:
		{
			//(e.lally 2006-08-31) PLID 22342 - If an item is saved on an EMN and has the value
			//remembered for the patient, when the type of item is changed to something that is not
			//compatible, this function will return and empty variant which we do not handle properly
			//in the rest of the EMR code (for instance when we do a VarDouble or VarString). To prevent this,
			//we should return the proper variant type.
			//Later, we will review the returns for failures in these sections (PLID 22335)

			// (j.jones 2007-07-30 14:54) - PLID 26878 - converted to use a parameter query
			_CommandPtr pCmd = OpenParamQuery(pCon, "SELECT Text FROM EmrDetailsT WHERE ID = ?");
			AddParameterLong(pCmd, "DetailID", arynOldEMRDetailIDs.GetAt(0));

			_RecordsetPtr rs = CreateRecordset(pCmd);
			if (!rs->eof) {
				// Return the string
				//TES 11/7/2007 - PLID 27979 - VS2008 - Need to explicitly return a _variant_t.  I'm choosing
				// to not just return the Value, since that would mean that, in the case where the value was NULL,
				// the behavior would change from returning a VT_BSTR representing an empty string, to VT_NULL,
				// and I don't want to change behavior, even though that seems like the more correct behavior here.
				return _variant_t(VarString(rs->GetFields()->GetItem("Text")->GetValue(), ""));
			} else {
				// No detail!  Why was this function called with a non-existent detail id?
				ASSERT(FALSE);
				return varNull;
			}
		}
		break;
	case eitTable:
		{
			//we need to remap data IDs and dropdown IDs, and load appropriately

			//the format is X;Y;Data;Repeat...;

			//TES 3/28/2006 - We need to order properly; that way, if the varState gets recreated, it will still be
			//the same (provided the actual data really hasn't changed).
			// (c.haag 2007-02-05 15:31) - PLID 24423 - Cleaned up the sorting and supported special sorting for Current Medication details
			// (c.haag 2007-04-03 08:59) - PLID 25468 - Expanded the special sorting described above to special Allergy items
			//DRT 7/26/2007 - PLID 26830 - Parameterized to speed execution.
			//TES 6/5/2008 - PLID 29416 - The system tables are hardcoded to display as "Remember"ing, but the data doesn't
			// have that flag set, so override the data in that case (also changed to CreateParamRecordset)
			//TES 6/5/2008 - PLID 30309 - Pull the new DataIDs (for X and Y), as well as whether the old and new columns
			// are dropdown columns.  Previously it was opening 3 recordsets in every iteration of this loop to get that
			// information.
			// (z.manning 2010-02-18 09:28) - PLID 37427 - Added EmrDetailImageStampID
			// (z.manning 2010-02-18 16:19) - PLID 37412 - We need to left join the EmrDataT table for rows as tables rows
			// are no longer required to have an EmrDataID
			// (a.walling 2010-03-02 16:44) - PLID 36703 - Get the column types directly
			// (z.manning 2011-02-24 14:46) - PLID 42579 - Changed this to support multiple details
			// (z.manning 2011-03-02 15:11) - PLID 42335 - Added EmrImageStampID
			_RecordsetPtr rs = CreateParamRecordset(pCon, 
				"DECLARE @NewEmrInfoID INT \r\n"
				"SET @NewEmrInfoID = {INT} \r\n"
				"SELECT EmrDetailsT.ID AS EmrDetailID, RowData_New.ID AS EMRDataID_X, ColumnData_New.ID AS EMRDataID_Y, EMRDetailTableDataT.Data, EmrDetailImageStampID, \r\n"
				"	RowData.ID AS OldEmrDataID_X, ColumnData.ID AS OldEmrDataID_Y, ImageDetail.EmrInfoMasterID AS ImageInfoMasterID, \r\n"
				"	ColumnData.ListType AS OldListType, ColumnData_New.ListType AS ListType \r\n"
				"	, EmrDetailImageStampsT.EmrImageStampID \r\n"
				"FROM EMRDetailTableDataT \r\n"
				"INNER JOIN EmrDetailsT ON EMRDetailTableDataT.EmrDetailID = EmrDetailsT.ID \r\n"
				"INNER JOIN EmrMasterT ON EmrDetailsT.EmrID = EmrMasterT.ID \r\n"
				"LEFT JOIN EmrDataT RowData ON EMRDetailTableDataT.EMRDataID_X = RowData.ID \r\n"
				"LEFT JOIN EmrDataT RowData_New ON RowData_New.EmrInfoID = @NewEmrInfoID AND RowData_New.EmrDataGroupID = RowData.EmrDataGroupID \r\n"
				"INNER JOIN EmrDataT ColumnData ON EMRDetailTableDataT.EMRDataID_Y = ColumnData.ID \r\n"
				"LEFT JOIN EmrDataT ColumnData_New ON ColumnData_New.EmrInfoID = @NewEmrInfoID AND ColumnData_New.EmrDataGroupID = ColumnData.EmrDataGroupID \r\n"
				"LEFT JOIN EmrDetailImageStampsT ON EmrDetailTableDataT.EmrDetailImageStampID = EmrDetailImageStampsT.ID AND EmrDetailImageStampsT.Deleted = 0 \r\n"
				"LEFT JOIN ( \r\n"
				"	SELECT EmrDetailsT.ID, EmrInfoT.EmrInfoMasterID \r\n"
				"	FROM EmrDetailsT \r\n"
				"	INNER JOIN EmrInfoT ON EmrDetailsT.EmrInfoID = EmrInfoT.ID \r\n"
				"	) ImageDetail ON EmrDetailImageStampsT.EmrDetailID = ImageDetail.ID \r\n"
				"WHERE EMRDetailTableDataT.EMRDetailID IN ({INTARRAY}) \r\n"
				// (z.manning 2011-02-25 12:25) - PLID 42579 - We must load the detail in order with most recent first
				// in order for smart stamp table states to load properly.
				"ORDER BY EmrMasterT.Date DESC, EmrMasterT.ID DESC, EmrDetailsT.ID DESC, ImageDetail.ID DESC \r\n"
				// (z.manning 2011-02-25 12:16) - PLID 42579 - No need to order by content when loading the state
				//"	COALESCE(EmrDetailImageStampsT.OrderIndex, 0), \r\n"
				//"	(CASE WHEN @AutoAlphabetize = 1 THEN CASE WHEN @DataSubType IN (1,2) AND RowData.ListType <> 2 THEN RowData.SortOrder ELSE -1 END ELSE RowData.SortOrder END), \r\n"
				//"	(CASE WHEN @AutoAlphabetize = 1 THEN CASE WHEN @DataSubType IN (1,2) AND RowData.ListType <> 2 THEN '' ELSE RowData.Data END ELSE '' END), \r\n"
				//"	(CASE WHEN @AutoAlphabetize = 1 THEN -1 ELSE ColumnData.SortOrder END), \r\n"
				//"	(CASE WHEN @AutoAlphabetize = 1 THEN ColumnData.Data ELSE '' END) \r\n"
				, nNewEMRInfoID, arynOldEMRDetailIDs);
			if (!rs->eof) {
				CString strAns = LoadTableStateFromRecordset(rs, TRUE, pCon);
				return _variant_t(strAns);
			}
			else {
				// No selection for this detail.  We used to not allow this, but now "no selection" is valid.
				return "";
			}

			return "";
		}
		break;
	default:
		// Unexpected
		ASSERT(FALSE);
		return varNull;
		break;
	}
}

// (z.manning 2011-02-24 15:54) - PLID 42579
CString MapDropdownIDTextToNewDataID(LPCTSTR strDropdownData, const long nNewDataID_Y, OPTIONAL IN ADODB::_Connection *lpCon)
{
	// (c.haag 2008-02-22 17:31) - PLID 17936 - Data elements now exist in the form
	// "ID1,ID2,ID3..." so we can't use atoi any longer
	//long nOldDropdownID = atoi(strData);
	//strData = AsString(nNewDropdownID);
	CDWordArray anOldDropdownIDs;
	ParseDelimitedStringToDWordArray(strDropdownData, ",", anOldDropdownIDs);
	const int nOldDropdownIDs = anOldDropdownIDs.GetSize();
	CString strNewData;
	for (int nIndex=0; nIndex < nOldDropdownIDs; nIndex++) {
		// (j.jones 2007-07-23 09:16) - PLID 26773 - added connection pointer for use in threads
		long nNewDropdownID = CalculateMappedDropdownID(anOldDropdownIDs[nIndex], nNewDataID_Y, lpCon);
		if (!strNewData.IsEmpty()) {
			strNewData += ",";
		}
		strNewData += AsString(nNewDropdownID);
	}
	return strNewData;
}

BOOL ValidateEMRInfoName(IN const CString &strName, OPTIONAL IN const CString &strOldName, long nEMRInfoID, IN bool bIsNewInfoName)
{
	// Track whether we prompted the user at all
	BOOL bPromptedAtLeastOnce = FALSE;

	// Make sure it's not blank
	if (strName.IsEmpty()) {
		MsgBox("You cannot enter a blank name for an EMR item.");
		bPromptedAtLeastOnce = TRUE;
		return FALSE;
	}

	if (strName.CompareNoCase("<Remove Item>") == 0) {
		MsgBox("'<Remove Item>' is an invalid item name. Please change the name of this item.");
		bPromptedAtLeastOnce = TRUE;
		return FALSE;
	}

	// Make sure it's not too long (past the sql field length limit)
	if (strName.GetLength() > 255) {
		MsgBox(
			"The name you have entered for this EMR item is too long.  Please enter a name "
			"that is fewer than 255 characters.  Please note, is it recommended that you "
			"choose a name that is 35 characters or less for optimal use in Word merges.");
		bPromptedAtLeastOnce = TRUE;
		return FALSE;
	}

	// Make sure we don't use something reserved by the system for merging
	if (einrReservedName == IsValidEMRMergeFieldName(strName.Left(MAX_EMR_ITEM_NAME_MERGE_LENGTH)))
	{
		MsgBox("The name you have entered for this EMR item is reserved for system use. Please change the name of this item.");
		bPromptedAtLeastOnce = TRUE;
		return FALSE;
	}

	//Change the warning depending whether it's in use.
	// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
	if(ReturnsRecordsParam("SELECT ID FROM EmrDetailsT WHERE Deleted = 0 AND EmrInfoID = {INT}", nEMRInfoID)) {
		bPromptedAtLeastOnce = TRUE;
		if(IDYES != MsgBox(MB_YESNO, "This item is in use. Changing the name of this item will change it on all EMRs which use it.\n"
			"Are you sure you wish to continue?")) {
			return FALSE;
		}
	}

	//TES 2003-12-29: This is weird looking where clause, but it is because our CString comparison is case-sensitive,
	//but SQL's comparison is not case sensitive.  So if they're renaming something only by case, we don't
	//want to warn them if there are no other items with that name.
	// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
	if(ReturnsRecordsParam("SELECT ID FROM EMRInfoT WHERE Name = {STRING} AND Name <> {STRING}", strName, strOldName)) {
		CString strMessage;
		if(bPromptedAtLeastOnce) {
			strMessage = "There is already an EMR item with this name.\n"
			"It is recommended that you enter unique names for EMR items to minimize confusion.\n\n"
			"Do you still wish to use this name?";
		}
		else {
			strMessage = "There is already an EMR item with this name.\n"
			"It is recommended that you enter unique names for EMR items to minimize confusion.\n"
			"Do you still wish to use this name?";
		}
		bPromptedAtLeastOnce = TRUE;
		if(IDNO == MsgBox(MB_ICONQUESTION|MB_YESNO, strMessage,"Practice")) {
			return FALSE;
		}
	}

	// Give warnings if the new name implies a bad merge field name down the line
	{
		BOOL bPromptedForMergeFieldName;
		if (!ValidateEMRInfoNameMergeField(strName, strOldName, nEMRInfoID, bPromptedForMergeFieldName)) {
			return FALSE;
		}
		if (bPromptedForMergeFieldName && !bPromptedAtLeastOnce) {
			bPromptedAtLeastOnce = TRUE;
		}
	}
	

	// Now if nothing else gave a prompt to the user, we want to at least give her one chance to cancel.
	//TES 7/13/2004: We always want to give this warning, because it's always true, whether or not they've been prompted.
	//The only exceptions are a.) if it's new, or b.) if both names are longer than 35 characters, because they won't need to change it in that case.
	if (!bIsNewInfoName && (strOldName.GetLength() <= 35 || strName.GetLength() < 35) ) {
		int nResult = MsgBox(MB_YESNO, "If you change the name of this item, EVERY Word template that includes a merge field for this item will have to be manually changed to use the new name of this item.\n"
			"Are you sure you wish to change the name of this item from %s to %s?", strOldName, strName);
		bPromptedAtLeastOnce = TRUE;
		if (IDYES != nResult) {
			return FALSE;
		}
	}
	
	// If we made it here we've validated successfully so return TRUE.
	return TRUE;
}

BOOL ValidateEMRInfoNameMergeField(IN const CString &strName, OPTIONAL IN const CString &strOldName, long nEMRInfoID, OUT BOOL &bPromptWasGiven)
{
	// Initialze the one outbound parameter
	bPromptWasGiven = FALSE;

	// Calculate the merge field name
	CString ConvertToHeaderName(const CString &strPrefix, const CString &strHeaderBaseText, OPTIONAL OUT BOOL *pbTruncated = NULL);
	BOOL bTruncated = FALSE;
	CString strMergeFieldName = ConvertToHeaderName("EMR", strName, &bTruncated);

	// See if any other emrinfo would result in the same first 39 characters
	{
		// TODO: (b.cardillo 2004-06-21 16:34) - We could probably optimize this by having a 
		// SQL-side version of the ConvertToHeaderName() function.
		// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
		_RecordsetPtr prs = CreateParamRecordset("SELECT Name FROM EMRInfoT WHERE ID <> {INT}", nEMRInfoID);
		FieldPtr fldName = prs->GetFields()->GetItem("Name");
		while (!prs->eof) {
			CString strExistingName = AdoFldString(fldName);
			CString strExistingHeaderName = ConvertToHeaderName("EMR", strExistingName);
			if (strExistingHeaderName.CompareNoCase(strMergeFieldName) == 0) {
				int nResult = MsgBox(MB_ICONEXCLAMATION|MB_YESNO, 
					"The name you have entered for this EMR item is legal, however the "
					"merge field that would result, '%s', is identical to that of the "
					"existing EMR item '%s'.  It is recommended that you maintain uniqueness "
					"against other possible merge fields so that correct data can reliably "
					"be sent to Microsoft Word.\r\n\r\n"
					"Are you sure you want to use the non-optimal name for this EMR item?",
					strMergeFieldName, strExistingName);
				bPromptWasGiven = TRUE;
				if (nResult != IDYES) {
					// User wants to change the name, so return immediately.
					return FALSE;
				} else {
					// User accepted the name, so break out of the loop but proceed with the rest of the validating
					break;
				}
			}
			prs->MoveNext();
		}
	}

	// See if would be truncated by a merge
	if (bTruncated) {
		int nResult = MsgBox(MB_ICONQUESTION|MB_YESNO, 
			"The name you have entered for this EMR item has a legal length, however for "
			"optimal use in Microsoft Word merges it is recommended that the name be kept "
			"to less than 35 characters.  During a merge, the given name would be "
			"transferred as '%s'.\r\n\r\n"
			"Are you sure you want to use the non-optimal name for this EMR item?", 
			strMergeFieldName);
		bPromptWasGiven = TRUE;
		if (nResult != IDYES) {
			return FALSE;
		}
	}

	
	// If we made it here, nothing failed validation so it's valid
	return TRUE;
}

CString GetDisplayName(FollowupIncrement fiUnit)
{
	switch(fiUnit) {
	case fiDays:
		return "Days";
		break;
	case fiWeeks:
		return "Weeks";
		break;
	case fiMonths:
		return "Months";
		break;
	}
	ASSERT(FALSE);
	return "";
}

BOOL EmbeddedSpawnsViolateOnePerEmnRule(long nEMRInfoID, CStringArray& astrOffendingMints)
{
	// (c.haag 2005-08-24 13:41) - PLID 16238 - We want to make sure the following will not
	// happen in order:
	//
	// A. User creates EMN #1 and saves it
	// B. EMN #1 auto-creates EMN #2
	// C. EMN #2 auto-spawns all embedded mints
	// D. EMN #2 now has two or more instances of nEMRInfoID
	//
	astrOffendingMints.RemoveAll();

	//
	// Step 1: Get a list of all mint's that spawn one or more embedded mints by default. We
	// will never need to look outside this list. I used PLID 14444 comments for reference.
	//
	// (c.haag 2005-10-10 13:09) - PLID 16328 - Now deleted templates are ignored

	//DRT 2/19/2008 - PLID 28602 - Added checks for hotspots so they don't do the same.
	_RecordsetPtr prsL1Mints = CreateRecordset(
			"SELECT TemplateID, Name FROM EMRTemplateDetailsT LEFT JOIN EMRTemplateT ON EMRTemplateT.ID = EMRTemplateDetailsT.TemplateID "
			"WHERE EMRTemplateT.Deleted = 0 AND EMRTemplateDetailsT.ID IN "
			"	(SELECT EmrTemplateDetailID FROM EmrTemplateSelectT WHERE EMRDataID IN "
			"		(SELECT SourceID FROM EMRActionsT WHERE DestType = 9 AND SourceType = 4) "
			"	) OR EMRTemplatedetailsT.ID IN "
			"	(SELECT EMRDetailID FROM EMRHotSpotTemplateSelectT WHERE EMRImageHotSpotID IN "
			"		(SELECT SourceID FROM EMRActionsT WHERE DestType = 9 AND SourceType = 10) "
			"	) "
			// (c.haag 2006-03-20 10:43) - PLID 19775 - Needs a GROUP BY
			"GROUP By TemplateID, Name");
	FieldsPtr fL1Mints = prsL1Mints->Fields;
	while (!prsL1Mints->eof) {

		CArray<long, long> anEMRInfoIDs;
		long nL1MintID = AdoFldLong(fL1Mints, "TemplateID");

		//
		// Step 2: For this mint, grab the list of EMRInfoID's
		//
		//DRT 8/30/2007 - PLID 27259 - Parameterized.
		_RecordsetPtr prsL1Items = CreateParamRecordset("SELECT ActiveEMRInfoID FROM EmrInfoMasterT INNER JOIN EMRTemplateDetailsT ON EmrInfoMasterT.ID = EmrTemplateDetailsT.EmrInfoMasterID WHERE TemplateID = {INT} GROUP BY ActiveEMRInfoID",
			nL1MintID);
		FieldsPtr fL1Items = prsL1Items->Fields;
		while (!prsL1Items->eof) {
			anEMRInfoIDs.Add(AdoFldLong(fL1Items, "ActiveEMRInfoID"));
			prsL1Items->MoveNext();
		}

		//
		// Step 3: Determine if the combination of EMRInfoID's contained in the nL1MintID mint
		// and in all spawned ID's thereof will cause the one per emr constraint to fail
		//
		CArray<long, long> anMintIDs;
		anMintIDs.Add(nL1MintID);

		if (EmbeddedSpawnsViolateOnePerEmnRule(nL1MintID, anEMRInfoIDs, anMintIDs)) {
			astrOffendingMints.Add(AdoFldString(fL1Mints, "Name"));
		}

		// Move on to the next level 1 mint; there could be more of them that violate
		// one per emn rules.
		prsL1Mints->MoveNext();
	}

	return (astrOffendingMints.GetSize() > 0) ? TRUE : FALSE;
}

BOOL EmbeddedSpawnsViolateOnePerEmnRule(long nMintID, // The level 1 mint
										const CArray<long, long>& aEMRItemIDs, // The EMRItemID's of the level 1 mint
										const CArray<long, long>& aSpawningMints) // All of the mints that will be spawned
{
	//
	// Calculate all the embedded mints that would be spawned by other spawning mints.
	// Again, I used previous comments from PLID 14444.
	//
	CArray<long, long> anMintIDs;
	BOOL bNewRecordsAdded = TRUE;

	// Copy the mint ID list over because we're going to add to it shortly
	for (long i=0; i < aSpawningMints.GetSize(); i++)
		anMintIDs.Add(aSpawningMints[i]);

	while (bNewRecordsAdded) {

		// Build the source mint filter string. prsSpawnedMints will then pull all of the
		// mints spawned by every mint we found thus far. This will catch all levels of spawning.
		CString strMintList;
		for (i=0; i < anMintIDs.GetSize(); i++) {
			CString str;
			str.Format("%d,", anMintIDs[i]);
			strMintList += str;
		}
		strMintList.TrimRight(",");

		// Get the recordset of all mints that will be spawned by the level 1 mints and all
		// spawned mints.

		// (c.haag 2005-10-10 13:09) - PLID 16328 - Now deleted templates are ignored
		//DRT 2/19/2008 - PLID 28602 - Added checks for hotspots so they don't do the same.
		// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
		_RecordsetPtr prsSpawnedMints = CreateParamRecordset(
			" SELECT DestID FROM EMRActionsT WHERE (DestType = 9 AND SourceType = 4 AND SourceID IN "
			"	(SELECT EmrDataID FROM EmrTemplateSelectT WHERE "
			"		EMRTemplateDetailID IN (SELECT ID FROM EMRTemplateDetailsT WHERE TemplateID IN (SELECT ID FROM EMRTemplateT WHERE Deleted = 0) AND TemplateID IN ({INTSTRING})))) " // The item must be in our mint collection
			" OR (DestType = 9 AND SourceType = 10 AND SourceID IN "
			"	(SELECT EmrImageHotSpotID FROM EMRHotSpotTemplateSelectT WHERE EMRDetailID IN "
			"		(SELECT ID FROM EMRTemplateDetailsT WHERE TemplateID IN "
			"			(SELECT ID FROM EMRTemplateT WHERE Deleted = 0) "
			"		AND TemplateID IN ({INTSTRING})) "
			"	) "
			" ) "
			"",
			strMintList, strMintList);
		FieldsPtr fSpawnedMints = prsSpawnedMints->Fields;

		// Add mints that aren't already in anMintIDs to anMintIDs. If we don't
		// add anything, then we've covered all the permutations.
		bNewRecordsAdded = FALSE;
		while (!prsSpawnedMints->eof) {
			long nEmbeddedMintID = AdoFldLong(fSpawnedMints, "DestID");
			for (i=0; i < anMintIDs.GetSize(); i++) {
				if (anMintIDs[i] == nEmbeddedMintID)
					break;
			}
			if (i == anMintIDs.GetSize()) {
				anMintIDs.Add(nEmbeddedMintID);
				bNewRecordsAdded = TRUE;
			}
			prsSpawnedMints->MoveNext();
		}
	}

	// At this point in time, we have the ID's of every mint that will ever be spawned
	// in embedded form into the level 1 mint. Now we need to query all of the EMRInfoID's
	// that have one per emn restrictions from the spawned templates, and find out if, in the
	// case we include the content of aEMRItemIDs on the same emn, would cause the one per
	// emn restriction to be violated.
	CString strMints;
	for (i=0; i < anMintIDs.GetSize(); i++) {
		CString str;
		if (anMintIDs[i] != nMintID) {
			str.Format("%d,", anMintIDs[i]);
			strMints += str;
		}
	}
	strMints.TrimRight(",");

	if (strMints.GetLength()) {
		// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
		_RecordsetPtr prsOffending = CreateParamRecordset("SELECT ActiveEMRInfoID, Count(ActiveEMRInfoID) AS OffendingCount FROM EMRTemplateDetailsT "
			"INNER JOIN EmrInfoMasterT ON EmrTemplateDetailsT.EmrInfoMasterID = EmrInfoMasterT.ID LEFT JOIN EMRInfoT ON EMRInfoT.ID = EmrInfoMasterT.ActiveEMRInfoID "
			"WHERE OnePerEmn = 1 AND TemplateID IN ({INTSTRING}) GROUP BY ActiveEMRInfoID",
			strMints);
		FieldsPtr fOffending = prsOffending->Fields;
		while (!prsOffending->eof) {

			long nOffenderID = AdoFldLong(fOffending, "ActiveEMRInfoID");
			long nCount = AdoFldLong(fOffending, "OffendingCount");
			if (nCount > 1) {
				return TRUE; // If there is already more than one instance of a restricted item, we fail.
			} else {
				// If there is only one instance of a restricted item, see if there is an instance in
				// aEMRItemIDs (these are the items on the top-level template). If there is, we fail.
				for (long i=0; i < aEMRItemIDs.GetSize(); i++) {
					if (aEMRItemIDs[i] == nOffenderID)
						return TRUE;
				}
			}
			prsOffending->MoveNext();
		}
	}
	return FALSE;
}

void PrintEMNReport(long nEMNID)
{
	//now load and run the report
	CReportInfo infReport = CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(257)];
	infReport.nExtraID = nEMNID;

	//check to see if there is a default report
	_RecordsetPtr rsDefault = CreateRecordset("SELECT ID, CustomReportID FROM DefaultReportsT WHERE ID = 257");
	CString strFileName;

	if (rsDefault->eof) {
		strFileName = "EMRDataByPatientIndiv";
	}
	else {
		
		long nDefaultCustomReport = AdoFldLong(rsDefault, "CustomReportID", -1);

		if (nDefaultCustomReport > 0) {

			_RecordsetPtr rsFileName = CreateRecordset("SELECT FileName FROM CustomReportsT WHERE ID = 257 AND Number = %li", nDefaultCustomReport);

			if (rsFileName->eof) {

				//this should never happen
				MsgBox("Practice could not find the custom report.  Please contact NexTech for assistance");
			}
			else {
				
				//set the default
				infReport.nDefaultCustomReport = nDefaultCustomReport;
				strFileName =  AdoFldString(rsFileName, "FileName");
			}
		}
		else {
			strFileName = "EMRDataByPatientIndiv";
		}
	}			

	////////////////////////////////////////////////////////////////////////////////////////////
	RunReport(&infReport, true, GetMainFrame(), "EMR / Op Report");
}

CEmrDetailSource::CEmrDetailSource(EDetailSourceType edst /*= dstInvalid*/, long nSourceID /*= -1*/)
{
	m_edstSourceType = edst;
	m_nSourceID = nSourceID;
}

CEmrDetailSource::CEmrDetailSource(EmrActionObject eaoSourceType, long nSourceID)
{
	m_edstSourceType = CalcSourceType(eaoSourceType);
	m_nSourceID = nSourceID;
}

BOOL CEmrDetailSource::IsEqual(const CEmrDetailSource &eds) const 
{
	if (!m_edstSourceType == dstInvalid && eds.m_edstSourceType != dstInvalid) {
		// See if the types match
		if (m_edstSourceType == eds.m_edstSourceType) {
			// Their types are equal.  If the type is id-dependent then check the id, otherwise just return TRUE.
			switch (m_edstSourceType) {
			case dstSpawnedByProcedure:
			//case dstSpawnedByDiagCode: // (b.savon 2014-07-14 11:08) - PLID 62706 - Deprecate the old Diag DestType and handle the changes Practice wide
			case dstSpawnedByDiagnosis: 
			case dstSpawnedByData:
			case dstCopiedFromTemplate:
			case dstSpawnedByServiceCode:
			case dstSpawnedByInfo:
			case dstSpawnedByHotSpot:	//DRT 1/23/2008 - PLID 28690 - Compare these too
			case dstSpawnedByTableDropdownItem: // (z.manning 2009-02-17 09:49) - PLID 33072
			case dstSpawnedBySmartStamp: // (z.manning 2010-03-02 16:21) - PLID 37571
				// Types that DO have a meaning for the source id, so compare the ids
				if (m_nSourceID != -1 && eds.m_nSourceID != -1) {
					if (m_nSourceID == eds.m_nSourceID) {
						// We have an exact match, so return TRUE
						return TRUE;
					} else {
						// IDs don't match so the whole object doesn't match so return FALSE
						return FALSE;
					}
				} else {
					// At least one id is -1, which is an invalid ID so we DON'T have a match (even if both IDs were -1).
					return FALSE;
				}
				break;
			case dstEnteredByUser:
			case dstPhoto:
			case dstUnknownSource:
				// Types that don't have a meaning for the source id, so don't compare the ids, just return TRUE
				return TRUE;
				break;
			case dstInvalid:
				// By the logic of this function we know this case is impossible to get to
				ASSERT(FALSE);
				return FALSE;
				break;
			default:
				// An unexpected type, throw an exception
				ASSERT(FALSE);
				ThrowNxException("CEmrDetailSource::IsEqual: Unexpected source type %li!", m_edstSourceType);
				break;
			}
		} else {
			// Their types don't match so they are not equal, we don't even 
			// have to bother checking the source id.
			return FALSE;
		}
	} else {
		// One of them has an invalid type, which means that cannot be 
		// equal (even if they BOTH have an invalid type).
		return FALSE;
	}
}


CEmrDetailSource::EDetailSourceType CEmrDetailSource::CalcSourceType(EmrActionObject eao)
{
	switch (eao) {
	case eaoCpt:
		return dstSpawnedByServiceCode;
		break;
	// (b.savon 2014-07-14 10:56) - PLID 62706 - Deprecate the old Diag DestType and handle the changes Practice wide
	//case eaoDiag:
	//	return dstSpawnedByDiagCode;
	//	break;
	case eaoEmrItem:
		return dstSpawnedByInfo;
		break;
	case eaoEmrDataItem:
		return dstSpawnedByData;
		break;
	case eaoProcedure:
		return dstSpawnedByProcedure;
		break;
	case eaoEmrImageHotSpot:
		//DRT 1/23/2008 - PLID 28690
		return dstSpawnedByHotSpot;
		break;
	// (z.manning 2009-02-10 15:32) - PLID 33026
	case eaoEmrTableDropDownItem:
		return dstSpawnedByTableDropdownItem;
		break;
	case eaoSmartStamp: // (z.manning 2010-03-02 16:21) - PLID 37571
		return dstSpawnedBySmartStamp;
		break;
	case eaoWoundCareCodingCondition: // (r.gonet 08/03/2012) - PLID 51949
		return dstSpawnedByWoundCareCalculator;
	// (b.savon 2014-07-14 10:56) - PLID 62706 - Deprecate the old Diag DestType and handle the changes Practice wide
	case eaoDiagnosis:
		return dstSpawnedByDiagnosis;
		break;
	default:
		// Unexpected value
		ASSERT(FALSE);
		ThrowNxException("CalcEmrDetailSourceType: Unexpected action object value %li!", eao);
		break;
	}
}

CString CEmrDetailSource::GetDisplayName()
{
	switch(m_edstSourceType) {		
	case dstSpawnedByProcedure:
		return VarString(GetTableField("ProcedureT", "Name", "ID", m_nSourceID));
		break;
	
	// (b.savon 2014-07-14 10:56) - PLID 62706 - Deprecate the old Diag DestType and handle the changes Practice wide
	//case dstSpawnedByDiagCode:
	//	return VarString(GetTableField("DiagCodesT", "CodeNumber", "ID", m_nSourceID));
	//	break;

	case dstSpawnedByData:
		return VarString(GetTableField("EmrDataT", "Data", "ID", m_nSourceID));
		break;

	case dstSpawnedByServiceCode:
		return VarString(GetTableField("ServiceT", "Name", "ID", m_nSourceID));
		break;

	case dstSpawnedByInfo:
		return VarString(GetTableField("EmrInfoT", "Name", "ID", m_nSourceID));
		break;

	case dstCopiedFromTemplate:
		return VarString(GetTableField("EmrTemplateT", "Name", "ID", m_nSourceID));
		break;

	case dstEnteredByUser:
		return "";
		break;

	case dstSpawnedByHotSpot:
		{
			//DRT 1/23/2008 - PLID 28690 - There is no display name for hotspots
			//TES 2/16/2010 - PLID 37298 - There is now!
			_RecordsetPtr rsSpotName = CreateParamRecordset("SELECT LabAnatomyT.Description AS Location, AnatomyQualifiersT.Name AS Qualifier, EmrImageHotSpotsT.AnatomySide "
				"FROM EmrImageHotSpotsT LEFT JOIN LabAnatomyT ON EmrImageHotSpotsT.AnatomicLocationID = LabAnatomyT.ID "
				"LEFT JOIN AnatomyQualifiersT ON EmrImageHotSpotsT.AnatomicQualifierID = AnatomyQualifiersT.ID "
				"WHERE EmrImageHotSpotsT.ID = {INT}", m_nSourceID);
			if(rsSpotName->eof) {
				return "";
			}
			else {
				CString strLocation = AdoFldString(rsSpotName, "Location", "");
				CString strQualifier = AdoFldString(rsSpotName, "Qualifier", "");
				AnatomySide as = (AnatomySide)AdoFldLong(rsSpotName, "AnatomySide", asNone);
				// (z.manning 2010-04-30 17:36) - PLID 37553 - We now have a function to format this
				return ::FormatAnatomicLocation(strLocation, strQualifier, as);
			}
		}
		break;

	case dstSpawnedByTableDropdownItem:
		// (z.manning 2009-02-17 09:50) - PLID 33072
		return VarString(GetTableField("EMRTableDropdownInfoT", "Data", "ID", m_nSourceID));
		break;

	case dstSpawnedBySmartStamp: // (z.manning 2010-03-02 16:22) - PLID 37571
		return VarString(GetTableField("EmrImageStampsT", "TypeName", "ID", m_nSourceID), "");
		break;

	// (b.savon 2014-07-14 10:56) - PLID 62706 - Deprecate the old Diag DestType and handle the changes Practice wide
	case dstSpawnedByDiagnosis:
		return VarString(
				GetTableField(
					R"(EmrActionDiagnosisDataT 
						LEFT JOIN DiagCodes AS ICD9Q ON EmrActionDiagnosisDataT.DiagCodeID_ICD9 = ICD9Q.ID
						LEFT JOIN DiagCodes AS ICD10Q ON EmrActionDiagnosisDataT.DiagCodeID_ICD10 = ICD10Q.ID)",
					R"(CASE WHEN ICD10Q.CodeNumber IS NOT NULL AND ICD9Q.CodeNumber IS NOT NULL THEN (ICD10Q.CodeNumber + ' - ' + ICD10Q.CodeDesc + ' (' + ICD9Q.CodeNumber + ')') ELSE
						CASE WHEN ICD10Q.CodeNumber IS NOT NULL THEN (ICD10Q.CodeNumber + ' - ' + ICD10Q.CodeDesc) ELSE 
						CASE WHEN ICD9Q.CodeNumber IS NOT NULL THEN (ICD9Q.CodeNumber + ' - ' + ICD9Q.CodeDesc) END END END)",
					"EmrActionID",
					m_nSourceID
				),
				""
				);
		break;

	case dstInvalid:
	case dstPhoto:
	case dstUnknownSource:
	default:
		ASSERT(FALSE);
		return "";
	}
}

extern CString GenerateXMLFromSemiColonDelimitedIDList(const CString &strSemiColonDelimitedIDList);

// (c.haag 2007-03-19 11:43) - PLID 25242 - We now support an optional EMN parameter in case the detail has no parent topic
// (c.haag 2008-02-22 12:51) - PLID 29064 - Added an optional parameter for a connection pointer
CString GetDataOutput(long nEmrDetailID, CMergeEngine *pMi, bool bAllowHtml, bool bInHtml, CStringArray &saTempFiles, OPTIONAL OUT bool *pbDataIsHtml /*= NULL*/, EmrCategoryFormat Format /*= ecfParagraph*/, CEMN* pParentEMN /*= NULL*/, CEmrPreviewImageInfo* pPreviewImageInfo /*= NULL*/,
					  OPTIONAL IN ADODB::_Connection *lpCon /*= NULL*/, TableRowID* pSingleTableRowID /*= NULL*/)
{
	// (a.walling 2009-10-23 09:23) - PLID 36046 - Track construction in initial reference count
	CEMNDetail *pDetail = CEMNDetail::CreateDetail(NULL, "GetDataOutput local detail");
	// (a.walling 2012-06-22 14:01) - PLID 51150 - No parent window param
	pDetail->LoadFromDetailID(nEmrDetailID, lpCon);
	pDetail->LoadContent(FALSE, NULL, lpCon);
	CString strRet = GetDataOutput(pDetail, pMi, bAllowHtml, bInHtml, saTempFiles, pbDataIsHtml, Format, pParentEMN, pPreviewImageInfo, lpCon, pSingleTableRowID);
	// (a.walling 2009-10-12 17:20) - PLID 36024 - Properly release the detail
	pDetail->__QuietRelease();
	//delete pDetail;
	return strRet;
}

// (a.walling 2007-04-12 12:28) - PLID 25605 - Pass in a CEmrPreviewImageInfo struct to handle saving images without a merge engine
// (c.haag 2007-03-19 11:43) - PLID 25242 - We now support an optional EMN parameter in case the detail has no parent topic
// (c.haag 2008-02-22 12:51) - PLID 29064 - Added an optional parameter for a connection pointer
// (a.walling 2010-03-26 17:56) - PLID 37923 - Limit on a specific table row
CString GetDataOutput(CEMNDetail *pDetail, CMergeEngine *pMi, bool bAllowHtml, bool bInHtml, CStringArray &saTempFiles, OPTIONAL OUT bool *pbDataIsHtml /*= NULL*/, EmrCategoryFormat Format /*= ecfParagraph*/, CEMN* pParentEMN /*= NULL*/, CEmrPreviewImageInfo* pPreviewImageInfo /*= NULL*/,
					  OPTIONAL IN ADODB::_Connection *lpCon /*= NULL*/, TableRowID* pSingleTableRowID /*= NULL*/)
{
	// (c.haag 2008-02-22 15:25) - PLID 29064 - Since we create recordsets in this function, we need to have a valid
	// connection pointer even if lpCon is NULL
	_ConnectionPtr pCon;
	if(lpCon) pCon = lpCon;
	else pCon = GetRemoteData();

	CString strDataOutput;
	// (c.haag 2007-03-16 11:58) - PLID 25242 - Assign the parent EMN if it's not already
	if (NULL == pParentEMN) {
		if (NULL != pDetail) {
			if (NULL != pDetail->m_pParentTopic) {
				pParentEMN = pDetail->m_pParentTopic->GetParentEMN();
			} else {
				// If we have a detail without a parent topic, then we have no parent EMN. This
				// is unexpected, but don't error out until the last second later on.
			}
		} else {
			// The detail is NULL; but that may be by intent. Don't error out until the last second 
			// later on.
		}
	}

	if(pbDataIsHtml) *pbDataIsHtml = false;

	// (a.walling 2008-02-08 08:59) - PLID 28857 - Need to ensure we are using the most accurate EmrInfoType for the state
	// (a.walling 2008-02-08 14:10) - PLID 28857 - Need to be sure that the pending EMRInfoType matches the state,
	// since some situations with the preview will cause IsStateSet to be called before the state is cleaned
	EmrInfoType eit = pDetail->m_EMRInfoType;


	switch(eit) {
	case eitText:
		// (a.walling 2007-10-19 16:57) - PLID 27820 - Need to escape for HTML
		// (a.walling 2007-10-23 12:36) - PLID 27820 - Only escape if we are allowing HTML and are in HTML.
		if (bAllowHtml && bInHtml) {
			strDataOutput = ConvertToHTMLEmbeddable(VarString(pDetail->GetState(), ""));
			// (a.walling 2007-10-23 13:53) - PLID 27820 - flag that this is HTML output
			if (pbDataIsHtml) {
				*pbDataIsHtml = TRUE;
			}
		} else {
			strDataOutput = VarString(pDetail->GetState(), "");
		}
		break;
	// (b.savon 2014-07-18 13:37) - PLID 62755 - Add bullet and number data format functionality for Single Select items in EMR
	case eitSingleList:
		{
			// (c.haag 2007-07-02 12:16) - PLID 26515 - Pass in the CEMNLoader object so that details
			// without parent topics can still take advantage of preloaded data
			pDetail->LoadContent(FALSE, (pParentEMN) ? pParentEMN->GetEMNLoader() : NULL, lpCon); //doesn't do anything if it is already loaded

			CString strSingleSelID = VarString(pDetail->GetState(), "");
			strSingleSelID.TrimRight();
			EmrMultiSelectFormat emsf = (EmrMultiSelectFormat)pDetail->m_nDataFormat;

			switch (emsf) {
			case emsfText:
			{
				//OK, just use the normal way of doing it.
			if (!strSingleSelID.IsEmpty()) {
				ASSERT(atoi(strSingleSelID) != 0);

				// (j.jones 2006-03-14 17:43) - PLID 19705 - we will have the ListElement array that tells us
				// the data name, so we don't need to query the database unless no detail was passed to us,
				// which I believe should never happen

					if (pDetail) {
						for (int i = 0; i<pDetail->GetListElementCount(); i++) {
						ListElement le = pDetail->GetListElement(i);
							if (le.nID == atoi(strSingleSelID)) {
							strDataOutput = le.strName;
						}
					}
				}
				else {
					// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
					strDataOutput = AdoFldString(CreateParamRecordset(pCon, "SELECT Data FROM EMRDataT WHERE ID = {INT}", atoi(strSingleSelID))->GetFields()->GetItem(0L), "");
				}

				}
				else {
				strDataOutput = "";
			}

			// (a.wetta 2006-08-10 10:19) - PLID 21899 - If this will be used in HTML, we need to format the data
			if (bAllowHtml && bInHtml) {
				strDataOutput = ConvertToHTMLEmbeddable(strDataOutput);
				*pbDataIsHtml = true;
			}

		}
		break;
			case emsfList:
			case emsfBulletList:
			case emsfNumberList:
			{
				if (!strSingleSelID.IsEmpty()) {
					ASSERT(atoi(strSingleSelID) != 0);

					if (bAllowHtml) {
						// (a.walling 2007-10-23 14:28) - PLID 27849 - Keep track of actual elements with data
						long nActualData = 0;
						if (pbDataIsHtml)
							*pbDataIsHtml = true;
						if (emsf == emsfBulletList)
							strDataOutput = "<UL>";
						else if (emsf == emsfNumberList)
							strDataOutput = "<OL>";
						// (a.walling 2008-06-04 16:50) - PLID 27965 - use a list with no bullets
						// (a.walling 2008-06-04 17:24) - PLID 30280 - use a class definition to fix issues with numbered lists
						else if (emsf == emsfList)
							strDataOutput = "<UL CLASS='UnstyledList'>";
						for (int i = 0; i < pDetail->GetListElementCount(); i++) {
							ListElement le = pDetail->GetListElement(i);
							CString str;
							if (le.nID == atoi(strSingleSelID)) {
								str = le.strName;
							}

							if (!str.IsEmpty()) {
								nActualData++;

								// (a.wetta 2006-08-10 10:25) - PLID 21899 - The str will be used in HTML, so make sure it's formatted correctly
								str = ConvertToHTMLEmbeddable(str);

								//if(emsf != emsfList)

								// (a.walling 2008-06-04 16:50) - PLID 27965 - use a list with no bullets
								strDataOutput += "<LI>";
								// (a.walling 2007-04-16 10:51) - PLID 25454 - Speaking of formatted correctly, let's close the LI tag.
								// Also, I don't see why we need the <br>; I tested it in various setups -- merging without the <br>
								// and just using the </li> instead works identical.
								strDataOutput += str + "</LI>";
								break;
							}
						}

						// (j.jones 2005-01-04 12:18) - somehow TrimRight with <br> doesn't work properly
						if (strDataOutput.Right(4) == "<br>")
							strDataOutput = strDataOutput.Left(strDataOutput.GetLength() - 4);

						// (j.jones 2005-02-11 14:41) - PLID 15483 - bulleted/numbered lists require a <br>
						// after them to be more compliant with Word. Read this PL item to understand why.
						if (emsf == emsfBulletList || emsf == emsfList) {
							strDataOutput += "</UL>";
							if (strDataOutput != "<UL></UL>")
								strDataOutput += "<br>";
						}
						else if (emsf == emsfNumberList) {
							strDataOutput += "</OL>";
							if (strDataOutput != "<OL></OL>")
								strDataOutput += "<br>";
						}

						// (a.walling 2007-10-23 14:29) - PLID 27849 - If we output no actual data,
						// then return a blank string rather than <UL></UL> etc.
						if (nActualData == 0) {
							strDataOutput = "";
						}
					}
					else {
						if (pbDataIsHtml)
							*pbDataIsHtml = false;
						int nIndex = 0;
						for (int i = 0; i < pDetail->GetListElementCount(); i++) {
							ListElement le = pDetail->GetListElement(i);
							CString str;
							if (le.nID == atoi(strSingleSelID)) {
								str = le.strName;
							}

							if (!str.IsEmpty()) {
								if (emsf == emsfBulletList) {
									strDataOutput += "*  " + str + "\r\n";
								}
								else if (emsf == emsfNumberList) {
									CString strLine;
									strLine.Format("%i.  %s\r\n", ++nIndex, str);
									strDataOutput += strLine;
								}
								else {
									CString strLine;
									strLine.Format("%s\r\n", str);
									strDataOutput += strLine;
								}
								break;
							}
						}
						strDataOutput.TrimRight("\r\n");
					}
				}
				else {
					strDataOutput = "";
				}
			}
				break;
			default:
				ASSERT(FALSE);
			}
		}
		break;
	case eitMultiList:
		{
			// (c.haag 2007-07-02 12:16) - PLID 26515 - Pass in the CEMNLoader object so that details
			// without parent topics can still take advantage of preloaded data
			pDetail->LoadContent(FALSE, (pParentEMN) ? pParentEMN->GetEMNLoader() : NULL, lpCon); //doesn't do anything if it is already loaded

			//TES 11/1/2004 - PLID 14395 - If this is formatted as a bullet or numbered-list, we'll return as HTML.
			CString strValueNames;			
			EmrMultiSelectFormat emsf = (EmrMultiSelectFormat)pDetail->m_nDataFormat;
			switch(emsf) {
			case emsfText:
				{
					//OK, just use the normal way of doing it.
					// (b.cardillo 2005-03-22 17:47) - PLID 15008 - Distinguish between the "final" separator and 
					// the regular separators.
					CString strSeparator = pDetail->m_strDataSeparator;
					CString strSeparatorFinal = pDetail->m_strDataSeparatorFinal;
					CString strLastValue;
					bool bAtLeastOneSelected = false;
					for(int i=0; i<pDetail->GetListElementCount(); i++) {
						ListElement le = pDetail->GetListElement(i);
						if(!le.bIsSelected)
							continue;
						bAtLeastOneSelected = true;
						if (!strLastValue.IsEmpty()) {
							if (!strValueNames.IsEmpty())
								strValueNames += strSeparator;
							strValueNames += strLastValue;
						}
						strLastValue = le.strName;
					}
					
					//TES 6/23/2008 - PLID 30473 - We want this even if the last value is empty, as long as at least one
					// was selected (and the string thus far is not empty).
					if (bAtLeastOneSelected /*!strLastValue.IsEmpty()*/) {
						if (!strValueNames.IsEmpty())
							strValueNames += strSeparatorFinal;
						strValueNames += strLastValue;
					}
					strDataOutput = strValueNames;

					// (a.wetta 2006-08-10 10:19) - PLID 21899 - If this will be used in HTML, we need to format the data
					if (bAllowHtml && bInHtml) {
						strDataOutput = ConvertToHTMLEmbeddable(strDataOutput);
						*pbDataIsHtml = true;
					}
				}
				break;
			case emsfList:
			case emsfBulletList:
			case emsfNumberList:
				{
					if(bAllowHtml) {
						// (a.walling 2007-10-23 14:28) - PLID 27849 - Keep track of actual elements with data
						long nActualData = 0;
						if(pbDataIsHtml)
							*pbDataIsHtml = true;
						if(emsf == emsfBulletList)
							strDataOutput = "<UL>";
						else if(emsf == emsfNumberList)
							strDataOutput = "<OL>";
						// (a.walling 2008-06-04 16:50) - PLID 27965 - use a list with no bullets
						// (a.walling 2008-06-04 17:24) - PLID 30280 - use a class definition to fix issues with numbered lists
						else if(emsf == emsfList)
							strDataOutput = "<UL CLASS='UnstyledList'>";
						for(int i=0; i<pDetail->GetListElementCount(); i++) {
							ListElement le = pDetail->GetListElement(i);
							if(!le.bIsSelected)
								continue;
							CString str = le.strName;
							if(!str.IsEmpty()) {
								nActualData++;

								// (a.wetta 2006-08-10 10:25) - PLID 21899 - The str will be used in HTML, so make sure it's formatted correctly
								str = ConvertToHTMLEmbeddable(str);

								//if(emsf != emsfList)
								
								// (a.walling 2008-06-04 16:50) - PLID 27965 - use a list with no bullets
								strDataOutput += "<LI>";
								// (a.walling 2007-04-16 10:51) - PLID 25454 - Speaking of formatted correctly, let's close the LI tag.
								// Also, I don't see why we need the <br>; I tested it in various setups -- merging without the <br>
								// and just using the </li> instead works identical.
								strDataOutput += str + "</LI>";
							}
						}
						
						// (j.jones 2005-01-04 12:18) - somehow TrimRight with <br> doesn't work properly
						if(strDataOutput.Right(4) == "<br>")
							strDataOutput = strDataOutput.Left(strDataOutput.GetLength()-4);

						// (j.jones 2005-02-11 14:41) - PLID 15483 - bulleted/numbered lists require a <br>
						// after them to be more compliant with Word. Read this PL item to understand why.
						if(emsf == emsfBulletList || emsf == emsfList) {
							strDataOutput += "</UL>";
							if(strDataOutput != "<UL></UL>")
								strDataOutput += "<br>";
						}
						else if(emsf == emsfNumberList) {
							strDataOutput += "</OL>";
							if(strDataOutput != "<OL></OL>")
								strDataOutput += "<br>";
						}

						// (a.walling 2007-10-23 14:29) - PLID 27849 - If we output no actual data,
						// then return a blank string rather than <UL></UL> etc.
						if (nActualData == 0) {
							strDataOutput = "";
						}
					}
					else {
						if(pbDataIsHtml)
							*pbDataIsHtml = false;
						int nIndex = 0;
						for(int i=0; i<pDetail->GetListElementCount(); i++) {
							ListElement le = pDetail->GetListElement(i);
							if(!le.bIsSelected)
								continue;
							CString str = le.strName;
							if(!str.IsEmpty()) {
								if(emsf == emsfBulletList) {
									strDataOutput += "*  " + str + "\r\n";
								}
								else if(emsf == emsfNumberList) {
									CString strLine;
									strLine.Format("%i.  %s\r\n", ++nIndex, str);
									strDataOutput += strLine;
								}
								else {
									CString strLine;
									strLine.Format("%s\r\n", str);
									strDataOutput += strLine;
								}
							}
						}
						strDataOutput.TrimRight("\r\n");
					}									

				}
				break;
			default:
				ASSERT(FALSE);
			}
		}
		break;
	case eitImage:
		{
			// (z.manning 2011-09-26 11:49) - PLID 45664 - We need to handle 3D images differently than normal images.
			if(pDetail->Is3DImage())
			{
				// (z.manning 2011-09-26 11:49) - PLID 45664 - The output for 3D models consists of a set of 2D images
				// as definied by the Nx3D control.
				CImageArray aryImages;
				pDetail->Get3DImageOutputData(&aryImages);
				CString strImageFilenames;
				for(int nImageIndex = 0; nImageIndex < aryImages.GetCount(); nImageIndex++)
				{
					// (z.manning 2011-09-26 11:57) - PLID 45664 - Create a temp file for each image the control gave us.
					Image image = aryImages.GetAt(nImageIndex);
					CString strTempFileBase = FormatString("Nx3DDetailImg_%li_%d", pDetail->GetID() == -1 ? (long)pDetail : pDetail->GetID(), nImageIndex);
					CString strTempFile;
					// (z.manning 2011-09-26 11:57) - PLID 45664 - The control can only return jpegs.
					HANDLE hImageFile = CreateNxTempFile(strTempFileBase, "jpg", &strTempFile, TRUE);
					if(hImageFile == INVALID_HANDLE_VALUE) {
						AfxThrowNxException("Invalid temp image file when generating 3D model images for detail %li", pDetail->GetID());
					}
					try {
						DWORD dwWritten;
						::WriteFile(hImageFile, image.pData, image.nSize, &dwWritten, NULL);
					}
					NxCatchAllCallThrow("Could not write to image file for 3D model", ::CloseHandle(hImageFile););
					::CloseHandle(hImageFile);
					if(!strImageFilenames.IsEmpty()) {
						if(pMi != NULL) {
							strImageFilenames += "\r\n";
						}
						else {
							// (z.manning 2011-09-26 11:58) - PLID 45664 - We return a pipe-delimited list of file names for the preview pane
							strImageFilenames += '|';
						}
					}
					if(pMi != NULL) {
						// (z.manning 2011-09-26 15:28) - PLID 45664 - We are merging so make sure we set the filename approprately.
						strTempFile = pMi->GetBitmapMergeName(strTempFile);
					}

					strImageFilenames += strTempFile;
				}

				return strImageFilenames;
			}

			// Create an image state object based on the current detail state
			CEmrItemAdvImageState aisState;
			aisState.CreateFromSafeArrayVariant(pDetail->GetState());

			// (j.jones 2005-11-09 11:47) -  PLID 17928 - if we "don't merge inkless images" it means we
			// do not merge images that have never been drawn on (drawing and erasing counts as drawing)
			// but we will also merge images that have been changed by the user, i.e. a blank image detail
			// where the user loaded a new image
			//
			// (z.manning 2009-03-31 16:54) - PLID 33716 - This preference only relates to merging so
			// only check it if the user actually is merging.
			BOOL bDontMergeInklessImages = FALSE;
			if(pMi != NULL) {
				bDontMergeInklessImages = GetRemotePropertyInt("EmnDontMergeInklessImages", 0, 0, GetCurrentUserName(), true) == 1;
			}

			// (a.walling 2009-01-13 15:52) - PLID 32107 - Use CEMNDetail::IsStateSet to determine whether to merge or not
			if(bDontMergeInklessImages && !pDetail->IsStateSet(NULL, true)) {
				//no ink, and no overriden path
				strDataOutput = "";
				break;
			}

			// Get the official background image path and type even though they may not be needed
			CString strOrigBackgroundImageFilePath;
			eImageType eitOrigBackgroundImageType = itUndefined;
			// (a.walling 2010-01-05 11:32) - PLID 36760 - Optimization -- if the state has an override within it,
			// then Render (and CalcCurrentBackgroundImageFullPath) do not end up using this information at all.
			// So, perform this step only if we do not already have an override. Also ignore this call if we are
			// using the built-in image, or if we have an override type of itForcedBlank
			if (aisState.m_strImagePathOverride.IsEmpty() && aisState.m_eitImageTypeOverride != itForcedBlank && pDetail->m_nEMRInfoID != EMR_BUILT_IN_INFO__IMAGE) {
				// We have to query the data in order to get the official background image path and type because 
				// we can't guarantee that pEMNDetail->m_pEmrItemAdvDlg is non-NULL and even if it were, we can't 
				// know if it has been fully loaded (i.e. there's no way to verify that (assuming we can be sure 
				// it's a CEmrItemAdvImageDlg) its m_strBackgroundImageFilePath and m_eitBackgroundImageType 
				// members have been initialized.
				//DRT 8/30/2007 - PLID 27259 - Parameterized.
				ADODB::_RecordsetPtr prs = CreateParamRecordset(pCon, "SELECT BackgroundImageFilePath, BackgroundImageType FROM EmrInfoT WHERE ID = {INT}", pDetail->m_nEMRInfoID);
				if (!prs->eof) {
					// Good, get the values out of the record
					ADODB::FieldsPtr pflds = prs->GetFields();
					//
					// (c.haag 2006-11-09 17:47) - PLID 23365 - Whenever we pull BackgroundImageType, we could
					// get a null value. If so, it means we pulled it from an EmrInfo item where the user must
					// choose the image to assign to the detail. In that event, we need to assign "blank default
					// values" to the image state. In our program, this means clearing the path and assigning the
					// image type of itDiagram.

					// (c.haag 2007-02-09 16:52) - PLID 23365 - The previous comment is wrong. The simple story
					// is that EmrInfoT.BackgroundImageType is NULL if there is no default image for the info
					// item. When the detail is added, it should have no image unless the detail itself is assigned
					// one in its InkImage*Override fields. The user will have to pick an image.
					if (itUndefined == (eitOrigBackgroundImageType = (eImageType)AdoFldLong(pflds, "BackgroundImageType", -1))) {
						strOrigBackgroundImageFilePath.Empty();
					} else {
						strOrigBackgroundImageFilePath = AdoFldString(pflds, "BackgroundImageFilePath", "");
					}
				} else {
					// There was no record in EMRInfoT for pEMNDetail->m_nEMRInfoID.  That shouldn't be possible.
					ASSERT(FALSE);
					strOrigBackgroundImageFilePath = "";
					eitOrigBackgroundImageType = itDiagram;
				}
			}
			// Get the image from the state variable
			//TES 2/7/2007 - PLID 18159 - We need to pass in the size of the image on the EMN, so that the text can
			// be scaled appropriately.  This doesn't have to be exact, so we will just take the size of the window, and
			// subtract the button width off the right, and an estimate of the label height off the top.
			CSize sz;
			CRect rImageWindow = pDetail->GetClientArea();

			// (a.wetta 2007-04-09 09:27) - PLID 24324 - Get the drawing properties area width from the image
			// (a.walling 2008-02-08 09:00) - PLID 28857 - No code change, just a comment. We check that the
			// EmrInfoType is an image here; this is correct. The advdlg may not have been recreated yet.
			// We would only reach this section if m_EmrInfoType == image and the pending data is null,
			// or if the pending type is an image, in which case this will be ignored since the m_EMRInfoType
			// does not match.
			// (z.manning 2009-02-19 11:16) - PLID 33151 - We need the ItemAdvDlg to exist or else we won't
			// know the dimensions of the image which means the image won't show on whatever we're trying
			// to output it to.
			/*
				CWnd *pTopicWnd = NULL;
				if(pDetail->m_pParentTopic != NULL) {
					pTopicWnd = pDetail->m_pParentTopic->GetInterface();
				}
				if(pDetail->m_pEmrItemAdvDlg == NULL) {
					pDetail->EnsureEmrItemAdvDlg(pTopicWnd);
					if(pDetail->m_pEmrItemAdvDlg) {
						((CEmrItemAdvImageDlg*)pDetail->m_pEmrItemAdvDlg)->EnableInkInput(FALSE);
					}
				}
			// (a.walling 2009-03-06 11:37) - PLID 33151 - OK, here is what I found:
			
				First off, we call pDetail->GetClientArea() BEFORE ensuring the advdlg exists. So, creating 
				the advdlg does nothing other than give us the sizes of the image controls.

				In the original issue (33151), the size of the rect is 0, and that is what is returned in 
				GetClientArea since the advdlg is NULL. With this fix, it still returns NULL/0. However the 
				GetDrawingPropertiesAreaDimensions call succeeds, and is subtracted from 0, resulting in some 
				huge number. This makes it look like it is working correctly, since it will only size as wide 
				as the image is, not to 4billion something for example.

				So it seems like we should put the GetClientArea call AFTER ensuring the advdlg exists. However 
				this, also, does not work. Although the advdlg is created, it is still not sized. It simply 
				retains the default size of the dialog it is based on. THEREFORE, it will always be: 
				rImageWindow: {top=9 bottom=155 left=9 right=305} regardless of the size of the image contained 
				within.

				So this fix did not really do anything. We can 'keep' this behaviour simply by not sizing down
				the image at all. This also appears to be the behaviour of sizing the image when it is sized in
				the EMRTopicWnd itself, anyway.

				Therefore this entire approach to fix the original issue was incorrect, and vicariously all 
				subsequent attempts to squash the bugs resulting from that. This can all be easily resolved 
				simply by removing any constraints on the image size. The preview pane itself prevents it 
				from being wider than a page.

				This results in the same behaviour as when the image is initially created in the CEMRTopicWnd,
				fixes the original issue, and introduces no real significant changes. The only thing that really
				needs to occur is to set the pPreviewImageInfo->nMaxWidth member to the actual width of the 
				unscaled image (because otherwise, it would be 0, and hence the preview pane would try to keep 
				it at that width).
			*/

			// (a.walling 2011-05-25 09:38) - PLID 43843 - Let's just get the actual image size from the control rather than trying to mess with whatever the size of the palette is

			//long nDrawingPropertiesAreaWidth = 0, nDrawingPropertiesAreaHeight = 0;
			//if (pDetail->m_pEmrItemAdvDlg && IsWindow(pDetail->m_pEmrItemAdvDlg->GetSafeHwnd()) && pDetail->m_EMRInfoType == eitImage) {
			//	((CEmrItemAdvImageDlg *)(pDetail->m_pEmrItemAdvDlg))->GetDrawingPropertiesAreaDimensions(nDrawingPropertiesAreaWidth, nDrawingPropertiesAreaHeight);
			//}

			//// (a.walling 2009-03-06 11:39) - PLID 33151 - Don't want to subtract anything from zero
			//if(!rImageWindow.IsRectNull()) {
			//	rImageWindow.right -= nDrawingPropertiesAreaWidth;
			//	rImageWindow.bottom -= 19;
			//}

			/*CRect rDisplayedImage;
			rDisplayedImage.SetRectEmpty();
			if (pDetail->m_pEmrItemAdvDlg && IsWindow(pDetail->m_pEmrItemAdvDlg->GetSafeHwnd()) && pDetail->m_EMRInfoType == eitImage) {
				rDisplayedImage = ((CEmrItemAdvImageDlg *)(pDetail->m_pEmrItemAdvDlg))->GetDisplayedImageSize();
			}*/



			// (a.walling 2007-04-12 11:45) - PLID 25605 - Scale to the size specified in our PreviewImageInfoStruct, if that won't resample.
			if (pPreviewImageInfo) {				
				// (a.walling 2011-09-14 15:51) - PLID 45498
				_ASSERTE(pDetail->m_EMRInfoType == eitImage);
				
				{
					//TES 1/25/2012 - PLID 47505 - See if they just want to use the file size, instead of the detail size.
					if(GetRemotePropertyInt("EMRPreview_UseOriginalFileSize", 0, 0, "<None>")) {
						//TES 1/25/2012 - PLID 47505 - Just set the image window to 0, and it will use the full resolution
						rImageWindow.SetRect(0,0,0,0);
					}
					else {
						// (a.walling 2011-09-14 15:51) - PLID 45498 - Calculate an estimate of how large the image is actually displayed;
						// the other approaches require an actual NxInkPicture object, which is not feasible in our situation. Plus this
						// ensures the calculation is consistent.
						// (a.wilson 2013-03-22 09:27) - PLID 55826 - pass in the detail so that we can filter out excluded stamps.
						long nStampCount = GetInkPictureStampCount(pDetail) + 1;

						// (a.walling 2015-03-19 15:30) - PLID 65082 - signature details do not even have the 'configure' button, so only the two rows apply
						if (pDetail->IsSignatureDetail()) {
							nStampCount = 0; 
						}

						long width = rImageWindow.Width();

						width -= 4; // padding for both sides
						// image
						width -= 4; // palette buffer
						width -= (26 * 2); // two columns of buttons
						width -= 10; // custom stamp buffer (applicable even if no custom stamps)

						if (nStampCount > 0) {
							long nMaxStampsPerColumn = rImageWindow.Height() / 26;
							long nColumns = nMaxStampsPerColumn ? ((nStampCount + nMaxStampsPerColumn - 1) / nMaxStampsPerColumn) : 1;
							width -= (nColumns * 26); // columns of custom stamps
						}
						if (width < 64) {
							width = 64;
						}

						rImageWindow.right = rImageWindow.left + width;
						rImageWindow.bottom = rImageWindow.top;
					}
				}

				pPreviewImageInfo->nMaxWidth = rImageWindow.Width();
			}

			// (c.haag 2007-06-04 10:12) - PLID 25242 - If pParentEMN is NULL by this point, we fail
			if (NULL == pParentEMN) {
				ASSERT(FALSE);
				ThrowNxException("::GetDataOutput was called for an image detail, but the parent EMN object was NULL!");
			}

			// (a.walling 2013-06-06 09:41) - PLID 57069 - Calc our current render state
			// (z.manning 2015-05-28 14:40) - PLID 66102 - Added max width
			CEmrItemAdvImageRenderState renderState(
				pDetail->GetImageTextStringFilter()
				, aisState
				, rImageWindow
				, strOrigBackgroundImageFilePath
				, eitOrigBackgroundImageType
				, !!GetRemotePropertyInt("EnableEMRImageTextScaling", 1, 0, "<None>", true)
				, pPreviewImageInfo == NULL ? 0 : pPreviewImageInfo->nMaxWidth
			);

			bool bUseLastRendered = false;

			// (a.walling 2013-06-06 09:41) - PLID 57069 - if the last render output exists, and the render state matches
			// we can simply use that rather than recreate it
			if (!pDetail->m_lastImageRenderDataOutput.IsEmpty() && renderState == pDetail->m_lastImageRenderState) {
				if (INVALID_FILE_ATTRIBUTES != ::GetFileAttributes(pDetail->m_lastImageRenderDataOutput)) {
					bUseLastRendered = true;
				} else {
					TRACE("CEMNDetail 0x%08x ENOEXIST last-rendered data output `%s`\n", pDetail, pDetail->m_lastImageRenderDataOutput);
				}
			}

			if (bUseLastRendered) {
				strDataOutput = pDetail->m_lastImageRenderDataOutput;
				// (z.manning 2015-05-28 14:51) - PLID 66102 - See if the previous render had a max width and if so, use it.
				if (pDetail->m_lastImageRenderState.nMaxWidth > 0 && pPreviewImageInfo != NULL && pPreviewImageInfo->nMaxWidth == 0) {
					pPreviewImageInfo->nMaxWidth = pDetail->m_lastImageRenderState.nMaxWidth;
				}

				TRACE("CEMNDetail 0x%08x Using last-rendered data output `%s`\n", pDetail, strDataOutput);
			} else {
				if (!pDetail->m_lastImageRenderDataOutput.IsEmpty()) {
					TRACE("CEMNDetail 0x%08x Invalidated last-rendered data output `%s`\n", pDetail, strDataOutput);
				}
				// (a.walling 2008-02-13 14:38) - PLID 28605 - Pass in the detail so the state can render selected hotspots
				HBITMAP hbmp = aisState.Render(pDetail, &sz, rImageWindow, pParentEMN->GetParentEMR()->GetPatientID(), strOrigBackgroundImageFilePath, eitOrigBackgroundImageType);
				if (hbmp)
				{
					// (a.walling 2009-03-06 11:40) - PLID 33151 - If the rImageWindow was 0, we will get the full resolution image.
					// however we set the nMaxWidth to the rImageWindow.Width, which was 0. This needs to be set correctly.
					if (pPreviewImageInfo != NULL && pPreviewImageInfo->nMaxWidth == 0) {
						pPreviewImageInfo->nMaxWidth = sz.cx;

						// (z.manning 2015-05-28 14:48) - PLID 66102 - Also update this in the render state
						renderState.nMaxWidth = pPreviewImageInfo->nMaxWidth;
					}

					// Save to a temp file
					if(pMi)
						strDataOutput = pMi->GetBitmapMergeName(hbmp);
					else if (pPreviewImageInfo) {
						BOOL bUnsaved = FALSE;
						long nID = pDetail->GetID();
						if (nID == -1) {
							// (a.walling 2007-08-08 17:16) - PLID 27017 - use the unique session number
							nID = GetUniqueSessionNum();
							bUnsaved = TRUE;
						}

						long nEmnID = -1;
						if (pDetail->m_pParentTopic) {
							if (pDetail->m_pParentTopic->GetParentEMN()) {
								nEmnID = pDetail->m_pParentTopic->GetParentEMN()->GetID();
							}
						}

						if (nEmnID == -1) {
							// (a.walling 2007-08-08 17:00) - PLID 27017 - Maintain the -1 EMN id.
							//nEmnID = GetTickCount() % 100000;
							bUnsaved = TRUE;
						}

						// The GetUniqueSessionNum IDs will help us get a unique filename, although creating the temp file
						// will guarantee us that we get a unique filename anyway. If this is an unsaved, then it is
						// extraordinarily temporary. We'll put this in the filename so we know not to copy it to the
						// documents path (will be prepended with 'nexemrt_')
						// (a.walling 2007-07-25 11:36) - PLID 26261 - use a common prefix to temp preview files (nexemrt_)
						// regardless of whether it is unsaved or not (they will just be compiled into an MHT anyway)

						CString strDesiredFileName = FormatString("%sEMN_%li_DetailImage%li", "nexemrt_", nEmnID, nID);
						pPreviewImageInfo->strTempFile = SaveBitmapToTempFile(hbmp, strDesiredFileName);
						strDataOutput = pPreviewImageInfo->strTempFile;
					}

					// (a.walling 2013-06-06 09:41) - PLID 57069 - Update our last rendered state and output
					pDetail->m_lastImageRenderState = renderState;
					pDetail->m_lastImageRenderDataOutput = strDataOutput;

					if (strDataOutput.IsEmpty()) {
						// Failed to create the merge output for this image
						// (a.walling 2010-06-30 14:24) - no PLID - Remove this ASSERT. It serves only to frustrate testing.
						//ASSERT(FALSE);
						strDataOutput = "<Missing Image>";
					}
					// We're done with the bitmap so close our handle
					DeleteObject(hbmp);
				} else {
					// Failed to render, we assume it's because the aisState didn't have an image, which is actually ok so we just pass the "<Missing Image>" text.
					// (a.walling 2007-07-11 13:22) - no PLID - I'm getting tired of this assertion all the time, especially when we want
					// to remain vigilant and check out any unexpected assertions. However it's tough to tell which one is this minor ASSERT,
					// and which we actually need to pay attention to. So I'm changing this to include the expression in the ASSERT so one
					// can quickly identify it for what it is. Perhaps eventually we'll remove this assert entirely.

					// (a.walling 2007-08-10 08:51) - PLID 27040 - I've just checked with Tom, the original creator of this statement. We decided
					// that, while this IS an error state (the image is not found!) and it NEEDS to be handled (most likely 8500 scope), GetDataOutput
					// is probably not the appropriate place to do so. One idea is when loading the detail/EMN, we should check for the existance of all
					// referenced images, and perhaps then prompt or somehow inform the user that their image is missing, and also provide a way to
					// browse for a replacement image (or the same image in a different path), probably also displaying any info we have about it (size etc)
					// to help the user find the correct one.

					//_ASSERTE(aisState.CalcCurrentBackgroundImageFullPath(pParentEMN->GetParentEMR()->GetPatientID(), strOrigBackgroundImageFilePath, eitOrigBackgroundImageType).IsEmpty());

					strDataOutput = "<Missing Image>";
				}				
			}
		}
		break;
	case eitSlider:
		strDataOutput = AsString(pDetail->GetState());
		break;
	case eitNarrative:
		if(bAllowHtml) {
			//TES 12/1/2006 - PLID 23385 - We need to make sure the NxRichTextEditor has been loaded, because only it can properly 
			// parse the merge fields.
			// (a.walling 2012-06-22 14:01) - PLID 51150 - Explicitly get the topic wnd
			pDetail->EnsureEmrItemAdvDlg(pDetail->m_pParentTopic->GetTopicWndRaw());

			CString strFilename;
			CString strRichText;
			CString strPlainText;
			CString strNewName = pDetail->GetMergeFieldOverride();
			if(strNewName.IsEmpty())
				strNewName = pDetail->GetLabelText();

			// (c.haag 2004-05-27 08:56) PLID 12614 - Treat field names consistently
			// with how we do it in EMR (See ConvertToHeaderName() in EMRDlg.cpp)
			// First replace every non-alphanumeric character with an underscore
			for (long i=0; i<strNewName.GetLength(); i++) {
				// (a.walling 2008-10-03 17:29) - PLID 31589 - ASSERTion if you try to pass a signed char to any ::is* functions.
				if (!isalnum(unsigned char(strNewName.GetAt(i)))) {
					strNewName.SetAt(i, '_');
				}
			}
			// Then make every sequence of more than one underscore into a single underscore
			while (strNewName.Replace("__", "_"));

			// (j.jones 2004-12-29 14:00) - PLID 15081 - if the narrative is empty, do not merge it
			//TES 7/11/2012 - PLID 51500 - Differentiate between RTF and HTML narratives
			if(IsRtfNarrative(VarString(pDetail->GetState(), ""))) {
				strPlainText = ConvertTextFormat(VarString(pDetail->GetState(), ""), tfNxRichText, tfPlainText);
			}
			else {
				strPlainText = ConvertTextFormat(VarString(pDetail->GetState(), ""), tfHTML, tfPlainText);
			}
			strPlainText.TrimRight("\r\n");

			if(!strPlainText.IsEmpty()) {

				// (c.haag 2007-06-04 10:12) - PLID 25242 - If pParentEMN is NULL by this point, we fail
				if (NULL == pParentEMN) {
					ASSERT(FALSE);
					ThrowNxException("::GetDataOutput was called for a narrative detail, but the parent EMN object was NULL!");
				}

				// Generate our temporary file
				//TES 7/11/2012 - PLID 51500 - Differentiate between RTF and HTML narratives
				CString strNxRichText = VarString(pDetail->GetState(), "");
				CString strFullPath;
				if(IsRtfNarrative(strNxRichText)) {
					strFilename.Format("MergeRTF_Narrative_EMR_%s_%d.rtf", strNewName, pParentEMN->GetID());
					strFullPath = GetNxTempPath() ^ strFilename;
					CStdioFile f(strFullPath, CFile::modeCreate | CFile::modeWrite | CFile::typeText | CFile::shareCompat);

					CString strNewRich = ConvertTextFormat(strNxRichText, tfNxRichText, tfRichTextForMerge);
					if(!UpdateNxRichText(strNxRichText)) {
						MsgBox("The text for the narrative item " + pDetail->GetLabelText() + " is of an outdated format.  The item may not merge correctly.\r\n"
							"To resolve this, please open the EMN being merged, go to the tab with that item, and click OK on the EMN.  The item will automatically be brought up to date.");
						strNewRich = ConvertTextFormat(strNxRichText, tfNxRichText, tfRichText);;
					}
					else {
						strNewRich = ConvertTextFormat(strNxRichText, tfNxRichText, tfRichTextForMerge);
					}
					f.WriteString(strNewRich);
					f.Close();

					//m.hancock - 12/1/2005 - PLID 18478 - Some RTF merge fields should retain their formatting.
					//Using a tag of NXRTFRETAINFORMATTING will allow the narrative to keep its own font attributes
					//rather than using the font attributes applied to the merge field on the template.
					//I've changed the tag from NXRTF to NXRTFRETAINFORMATTING
					strDataOutput = "{NXRTFRETAINFORMATTING " + strFullPath + "}";
				}
				else {
					strFilename.Format("MergeHTML_Narrative_EMR_%s_%d.htm", strNewName, pParentEMN->GetID());
					strFullPath = GetNxTempPath() ^ strFilename;
					CStdioFile f(strFullPath, CFile::modeCreate | CFile::modeWrite | CFile::typeText | CFile::shareCompat);

					CString strNewHTML = "<html><head></head><body>" + ConvertTextFormat(strNxRichText, tfHTML, tfHTMLForMerge) + "</body></html>";
					f.WriteString(strNewHTML);
					f.Close();
					//TES 7/11/2012 - PLID 51500 - Use NXRTF here, like other places that merge .htm files
					strDataOutput = "{NXRTF " + strFullPath + "}";
				}

				// Add to our temp file list
				saTempFiles.Add(strFullPath);
			}
			else {
				strDataOutput = "";
			}
		}
		else {
			// (a.walling 2007-08-08 12:33) - PLID 27017 - Unfortunately, the state is not up to date for narratives that have not had
			// their advdlg created yet, so we must create it to get the correct state.
			// (a.walling 2007-10-17 14:22) - PLID 27017 - For safety's sake, ensure that the parent topic is valid.
			// (a.walling 2012-06-22 14:01) - PLID 51150 - Explicitly get the topic wnd
			if (pDetail->m_pParentTopic->GetTopicWndRaw()) {
				pDetail->EnsureEmrItemAdvDlg(pDetail->m_pParentTopic->GetTopicWndRaw());
			}
			//TES 7/11/2012 - PLID 51500 - Differentiate between RTF and HTML narratives
			CString strDetailText = VarString(pDetail->GetState(), "");
			if(IsRtfNarrative(strDetailText)) {
				strDataOutput = ConvertTextFormat(ConvertTextFormat(strDetailText, tfNxRichText, tfRichTextForMerge), tfRichText, tfPlainText);
			}
			else {
				strDataOutput = ConvertTextFormat(ConvertTextFormat(strDetailText, tfHTML, tfHTMLForMerge), tfHTML, tfPlainText);
			}
		}
		break;
	case eitTable: {			

			EmrMultiSelectFormat emsf = (EmrMultiSelectFormat)pDetail->m_nDataFormat;
			CString strSeparator = pDetail->m_strDataSeparator;
			CString strSeparatorFinal = pDetail->m_strDataSeparatorFinal;

			// (j.jones 2004-12-20 15:10) - if the format or separator have changed, 
			// then we of course need to reload
			
			// (j.jones 2005-03-10 13:31) - we should reload every time if it is HTML
			// (b.cardillo 2005-03-22 17:49) - PLID 15008 - Include the "final" separator in the comparison as well
			// (a.walling 2010-03-30 13:48) - PLID 37923 - Don't used any cached data if we are limiting to a single table row
			if(pSingleTableRowID == NULL && pDetail->m_emsf == emsf && pDetail->m_strSeparator == strSeparator && pDetail->m_strSeparatorFinal == strSeparatorFinal && !bAllowHtml) {

				//jump out early if we already have a sentence
				if(bAllowHtml && !pDetail->m_strSentenceHTML.IsEmpty()) {
					strDataOutput = pDetail->m_strSentenceHTML;
					if(pbDataIsHtml)
						*pbDataIsHtml = TRUE;
					break;
				}
				
				if(!bAllowHtml && !pDetail->m_strSentenceNonHTML.IsEmpty()) {
					strDataOutput = pDetail->m_strSentenceNonHTML;
					if(pbDataIsHtml)
						*pbDataIsHtml = FALSE;
					break;
				}
			}

			//okay, we need to get the new table data output
			CString strHTML, strNonHTML;
			// (a.walling 2010-03-26 18:03) - PLID 37923 - Send in any single table row ID if we have one
			//TES 3/22/2012 - PLID 48203 - Pass in pParentEMN
			GetTableDataOutput(pDetail,emsf,strSeparator,strSeparatorFinal,strHTML,strNonHTML, pMi, bAllowHtml, saTempFiles, pbDataIsHtml, Format, lpCon, pSingleTableRowID, pParentEMN);

			// (a.walling 2010-03-26 18:10) - PLID 37923 - Don't save these if we are getting a single table row's output
			if (!pSingleTableRowID) {
				pDetail->m_strSentenceHTML = strHTML;
				pDetail->m_strSentenceNonHTML = strNonHTML;
				pDetail->m_emsf = emsf;
				pDetail->m_strSeparator = strSeparator;
				pDetail->m_strSeparatorFinal = strSeparatorFinal;
			}

			if(bAllowHtml) {
				if(pbDataIsHtml)
					*pbDataIsHtml = TRUE;
				strDataOutput = strHTML;
			}
			else {
				strDataOutput = strNonHTML;
			}
		}
		break;
	default:
		// unknown
		ASSERT(FALSE);
		break;
	}

	return strDataOutput;
}

// (z.manning 2010-08-18 11:33) - PLID 39842 - Note: This class is now declared in EmrUtils.h
// (c.haag 2007-08-20 11:02) - PLID 27121 - This class is an array of table elements that
// can be sorted by row order by column order. If you were to do:
//
// for (i=0; i < sortedtableelementarray.GetSize(); i++) { ... }
//
// It's no different than doing:
//
// for (iRow=0; iRow < nRows; iRow++) {
//		for (iCol=0; iCol < nCols; iCol++) {
//			GetTableElementIfAvailable(iRow,iCol);
// } }
//
// (c.haag 2008-10-22 12:35) - PLID 31762 - Added support for table flipping; meaning we
// effectively do this traversal:
//
// for (iCol=0; iCol < nCols; iCol++) {
//		for (iRow=0; iRow < nRows; iRow++) {
//			GetTableElementIfAvailable(iRow,iCol);
// } }
//
typedef int (__cdecl *EMRUTILS_GENERICCOMPAREFN)(const void * elem1, const void * elem2);
typedef int (__cdecl *EMRUTILS_SORTEDROWCOLCOMPAREFN)(const TableElement ** elem1, const TableElement ** elem2);

// These are static globals (local only to this source file) that we need for sorting
static CMap<TableRow*,TableRow*,int,int> l_mapTableRow;
static CMap<TableColumn*,TableColumn*,int,int> l_mapTableColumn;

void CSortedTableElementArray::Sort(CEMNDetail* pDetail)
{
	EMRUTILS_SORTEDROWCOLCOMPAREFN pfnCompare;
	if (pDetail->m_bTableRowsAsFields) {
		pfnCompare = CompareColumnsThenRows;
	} else {
		pfnCompare = CompareRowsThenColumns;
	}

	const int nRows = pDetail->GetRowCount();
	const int nCols = pDetail->GetColumnCount();
	int i;

	// First, build maps where the keys are TableRow and TableColumn pointers, 
	// and the values are indices to the detail's internal row and column arrays.
	// The purpose of this is to quickly map a pointer to an ordinal in the
	// already-sorted row and column arrays in the detail. By doing this, we can
	// order the table elements in this object by row then column. 
	for (i=0; i < nRows; i++) {
		l_mapTableRow.SetAt( pDetail->GetRowPtr(i), i );
	}
	for (i=0; i < nCols; i++) {
		l_mapTableColumn.SetAt( pDetail->GetColumnPtr(i), i );
	}
	
	// Now do the sorting
	TableElement** prgdtl = (TableElement**)GetData();
	qsort(prgdtl,GetSize(),sizeof(TableElement*),(EMRUTILS_GENERICCOMPAREFN)pfnCompare);

	// Cleanup
	l_mapTableRow.RemoveAll();
	l_mapTableColumn.RemoveAll();
}

int __cdecl CSortedTableElementArray::CompareRowsThenColumns(const TableElement** pp1, const TableElement** pp2)
{
	const TableElement* p1 = *pp1;
	const TableElement* p2 = *pp2;
	ASSERT(p1);
	ASSERT(p2);

	// First, compare on row indexes
	int nRowIndex1, nRowIndex2;
	if (!l_mapTableRow.Lookup(p1->m_pRow, nRowIndex1) ||
		!l_mapTableRow.Lookup(p2->m_pRow, nRowIndex2)) {
		ThrowNxException("CSortedTableElementArray was given an invalid table element!");
	}
	if (nRowIndex1 < nRowIndex2) {
		return -1;
	} else if (nRowIndex1 > nRowIndex2) {
		return 1;
	}

	// Now compare on column indexes
	int nColIndex1, nColIndex2;
	if (!l_mapTableColumn.Lookup(p1->m_pColumn, nColIndex1) ||
		!l_mapTableColumn.Lookup(p2->m_pColumn, nColIndex2)) {
		ThrowNxException("CSortedTableElementArray was given an invalid table element!");
	}
	if (nColIndex1 < nColIndex2) {
		return -1;
	} else if (nColIndex1 > nColIndex2) {
		return 1;
	}

	ASSERT(FALSE); // This should never happen!
	return 0;
}

// (c.haag 2008-10-22 12:35) - PLID 31762 - Goes in order of columns then rows instead of
// rows then columns
int __cdecl CSortedTableElementArray::CompareColumnsThenRows(const TableElement** pp1, const TableElement** pp2)
{
	const TableElement* p1 = *pp1;
	const TableElement* p2 = *pp2;
	ASSERT(p1);
	ASSERT(p2);

	// Compare on column indexes
	int nColIndex1, nColIndex2;
	if (!l_mapTableColumn.Lookup(p1->m_pColumn, nColIndex1) ||
		!l_mapTableColumn.Lookup(p2->m_pColumn, nColIndex2)) {
		ThrowNxException("CSortedTableElementArray was given an invalid table element!");
	}
	if (nColIndex1 < nColIndex2) {
		return -1;
	} else if (nColIndex1 > nColIndex2) {
		return 1;
	}

	// Compare on row indexes
	int nRowIndex1, nRowIndex2;
	if (!l_mapTableRow.Lookup(p1->m_pRow, nRowIndex1) ||
		!l_mapTableRow.Lookup(p2->m_pRow, nRowIndex2)) {
		ThrowNxException("CSortedTableElementArray was given an invalid table element!");
	}
	if (nRowIndex1 < nRowIndex2) {
		return -1;
	} else if (nRowIndex1 > nRowIndex2) {
		return 1;
	}

	ASSERT(FALSE); // This should never happen!
	return 0;
}


// (c.haag 2008-02-22 12:50) - PLID 29064 - Added optional connection pointer parameter
// (a.walling 2010-03-26 18:02) - PLID 37923 - Takes a TableRowID* to limit output to a specific table row
// (z.manning 2010-08-09 15:22) - PLID 39842 - Added bAllowHtml
//TES 3/22/2012 - PLID 48203 - Added an optional pParentEMN parameter
void GetTableDataOutput(CEMNDetail *pDetail, EmrMultiSelectFormat emsf, CString strSeparator, CString strSeparatorFinal, CString &strHTML, CString &strNonHTML, CMergeEngine *pMi, bool bAllowHtml, CStringArray &saTempFiles, OPTIONAL OUT bool *pbDataIsHtml /*= NULL*/, EmrCategoryFormat Format /*= ecfParagraph*/,
						OPTIONAL IN ADODB::_Connection *lpCon /*= NULL*/, TableRowID* pSingleTableRowID /*= NULL*/, CEMN *pParentEMN /*= NULL*/)
{
	//this function is going to calculate both the HTML and nonHTML strings at the same time

	// (a.walling 2010-04-14 11:33) - PLID 34406 - Also need to ensure any calculated fields are updated appropriately.
	// (z.manning 2012-03-28 12:41) - PLID 33710 - No need to do this as we now save calculated fields to data.
	//CEmrItemAdvTableBase::UpdateCalculatedFields(pDetail, NULL);

	CString strState = VarString(pDetail->GetState(), "");
	// (c.haag 2008-10-21 17:30) - PLID 31762 - Support for flipped tables
	const BOOL bTableRowsAsFields = pDetail->m_bTableRowsAsFields;

	strHTML = "";
	strNonHTML = "";

	{
		//for a table, we reinterpret emsfNumberList as a Table output
		BOOL bTable = emsf == emsfNumberList;
			
		if(emsf == emsfBulletList) {
			strHTML += "<UL>";
		}

		long nTableWidth = 100; //dynamically change the table width to account for bullets and numbers, because Word does not!
		if(Format == ecfBulletedList || Format == ecfNumberedList) {
			nTableWidth = 90;
		}

		//m.hancock - 4/9/2006 - PLID 20016 - This code used to query EmrInfoT to see if the detail should have a disabled
		//table border, but now the flag is loaded when the item is loaded, thus speeding up merges and tooltip creation.
		int nTableBorder = pDetail->m_bDisableTableBorder == true ? 0 : 1;

		//m.hancock - 2/28/2006 - PLID 15864 - Added variable to set the border for the HTML table
		CString strTableStart;
		strTableStart.Format("<table width=\"%li%%\" border=\"%li\" cellspacing=\"0\" frame=\"VOID\" RULES=\"NONE\">\r\n",nTableWidth, nTableBorder);

		//display the merge name
		/*
		CString str;
		str.Format("<caption>%s</caption>\r\n",pEMNDetail->m_strLabelText);
		strHTML += str;
		*/

		strTableStart += "<TR ALIGN=center>\r\n<TH>  </TH>";

		//determines if a row should be added at all
		BOOL bRowHasData = FALSE;

		//determines if a table should be added at all
		BOOL bTableHasData = FALSE;

		CString strRow = "";

		//Make sure this detail has loaded its content (if its already loaded, function will have no effect.
		pDetail->LoadContent(FALSE, NULL, lpCon);

		int i;

		// (c.haag 2007-08-21 9:17) - PLID 27121 - Now we will go through all the elements
		// and populate non-HTML and HTML data
		const void* pPrevHTMLRow = NULL; // The HTML row of the previous element
		CStringArray astrRowHTMLData; // The HTML data for the row; one entry per column
		CMap<const void*,const void*,int,int> mapHTMLColOrdinals; // Maps an HTML column pointer to its ordinal

		// (c.haag 2008-10-21 17:33) - PLID 31762 - Supports regular and flipped tables
		const int nDetailRows = pDetail->GetRowCount();
		const int nDetailColumns = pDetail->GetColumnCount();
		const int nHTMLRows = (bTableRowsAsFields) ? nDetailColumns : nDetailRows;
		const int nHTMLColumns = (bTableRowsAsFields) ? nDetailRows : nDetailColumns;
		// (z.manning 2010-05-03 12:16) - PLID 38175
		const CString strLabelStyle = " style=\"color:blue; background-color:#D6D6D6; text-align:center\"";

		// First, fill astrRowHTMLData with blank items (one for each HTML table column)
		for (i=0; i < nHTMLColumns; i++) {
			// (z.manning 2010-05-03 14:03) - PLID 38175 - Find out of this column is a label and if so, style accordingly
			BOOL bIsLabel = FALSE;
			if(bTableRowsAsFields) {
				bIsLabel = pDetail->GetRowPtr(i)->m_bIsLabel;
			}
			else {
				bIsLabel = pDetail->GetColumnPtr(i)->m_bIsLabel;
			}
			astrRowHTMLData.Add(FormatString("<TD ALIGN=center%s></TD>\r\n", bIsLabel ? strLabelStyle : ""));
		}
		// Now build the HTML table column ordinal map for later processing
		for (i=0; i < nHTMLColumns; i++) {
			mapHTMLColOrdinals.SetAt( (bTableRowsAsFields) ? (void*)pDetail->GetRowPtr(i) : (void*)pDetail->GetColumnPtr(i), i );
		}
		// Now build the header
		for (i=0; i < nHTMLColumns; i++) {
			strTableStart += "<TH";
			if (bTableRowsAsFields) {
				const TableRow* ptr = pDetail->GetRowPtr(i);
				if(ptr->m_bIsLabel) {
					// (z.manning 2010-05-03 12:17) - PLID 38175 - This is a label so style it accordingly
					strTableStart += strLabelStyle;
				}
				strTableStart += '>';
				strTableStart += ConvertToHTMLEmbeddable(ptr->strName);
			} else {
				const TableColumn* ptc = pDetail->GetColumnPtr(i);
				if(ptc->m_bIsLabel) {
					// (z.manning 2010-05-03 12:17) - PLID 38175 - This is a label so style it accordingly
					strTableStart += strLabelStyle;
				}
				strTableStart += '>';
				strTableStart += ConvertToHTMLEmbeddable(ptc->strName);
			}
			strTableStart += "</TH>\r\n";
		}

		CMap<__int64, __int64, int, int> mapTable; // This maps row/column combinations to elements
		pDetail->PopulateTableElementMap(mapTable);

		//first, the non-HTML
		// (z.manning 2010-07-30 16:23) - PLID 39842 - Moved this logic to a new function
		CString strDetailHTML;
		// (c.haag 2011-04-05) - PLID 43145 - Pass in NULL for the EmrDataIDs filter; we don't use it here
		// (c.haag 2011-04-12) - PLID 43245 - Pass in NULL for the table caption
		//TES 3/22/2012 - PLID 48203 - Pass in pParentEMN
		pDetail->GetTableDataOutputRaw(strDetailHTML, strNonHTML, pSingleTableRowID, NULL, NULL, NULL, pParentEMN);

		//now the HTML
		// (z.manning 2010-07-30 16:23) - PLID 39842 - We can't use pDetail->GetTableDataOutputHTMLRaw because it uses the
		// style sheet whereas we have to do it manually here.
		for(int i = 0; i < nHTMLRows; i++)
		{
			for(int j = 0; j < nHTMLColumns; j++)
			{
				// (c.haag 2008-10-22 11:47) - PLID 31760 - Get the row and column
				TableRow* ptr = pDetail->GetRowPtr( bTableRowsAsFields ? j : i );
				TableColumn* ptc = pDetail->GetColumnPtr ( bTableRowsAsFields ? i : j);

				// (z.manning 2010-04-14 10:20) - PLID 38175 - Keep track if this row is a label
				BOOL bRowIsLabel = bTableRowsAsFields ? ptc->m_bIsLabel : ptr->m_bIsLabel;
				BOOL bColumnIsLabel = bTableRowsAsFields ? ptr->m_bIsLabel : ptc->m_bIsLabel;

				//(e.lally 2011-12-08) PLID 46471 - We don't need to check for allergies and current medication 
				//	checked status because this is an overloaded function that will do it internally
				// (a.walling 2012-07-13 10:28) - PLID 51479 - Pass in calculated fallback parent EMN
				CString strDataHTML = pDetail->GetTableElementDataOutput(ptr, ptc, mapTable, true, false, pParentEMN);

				if(bTable) {

					//now output the row
					void* pCurHTMLRow = (bTableRowsAsFields) ? (void*)ptc : (void*)ptr;
					void* pCurHTMLColumn = (bTableRowsAsFields) ? (void*)ptr : (void*)ptc;
					if(pPrevHTMLRow != pCurHTMLRow) {
						//the row has changed
						// (c.haag 2007-08-21 9:20) - PLID 27121 - We now determine this by pointer comparison
						pPrevHTMLRow = pCurHTMLRow;

						//if the last row had data, output it
						if(bRowHasData) {

							// (c.haag 2007-08-21 9:22) - PLID 27121 - Build the row string using the
							// CStringArray which stores each HTML table element. The reason we do this
							// is to guarantee that every column in the row string has a table divider tag
							// in it. If we don't do that, the row data will always stack from the left,
							// regardless of column placement.
							int n;
							for (n=0; n < nHTMLColumns; n++) {
								strRow += astrRowHTMLData[n];
							}
							strRow += "</TR>"; // (a.walling 2007-04-03 15:00) - PLID 25484 - Close the tag
							strHTML += strRow;

							// (c.haag 2007-08-21 9:22) - PLID 27121 - Now reset the row data
							for (n=0; n < nHTMLColumns; n++) {
								// (z.manning 2010-05-03 14:27) - PLID 38175 - If it's a label, style it as such
								astrRowHTMLData[n] = FormatString("<TD ALIGN=center%s></TD>\r\n"
									, bRowIsLabel || bColumnIsLabel ? strLabelStyle : "");
							}
						}
					
						//now start a new row
						// (c.haag 2008-10-22 12:23) - PLID 31762 - Factor in table flipping when getting the row name
						CString strHTMLRowName = (bTableRowsAsFields) ? ptc->strName : ptr->strName;
						// (z.manning 2010-04-15 17:24) - PLID 38175 - Gray out label rows and make text color blue
						CString strCellStyle;
						if(bRowIsLabel || bColumnIsLabel) {
							strCellStyle = strLabelStyle;
						}
						bRowHasData = FALSE;
						strRow.Format("<TR>\r\n<TD%s><B>%s</B></TD>", strCellStyle, ConvertToHTMLEmbeddable(strHTMLRowName)); // (a.walling 2007-04-03 15:00) - PLID 25484 - Close the tag
						// (z.manning 2010-05-05 17:57) - PLID 38519 - We used to have a somewhat convaluted check to see if
						// were in the first row that would fail if the row name was blank. I fixed it to be much simpler and
						// work in all cases.
						if(i > 0) {
							strTableStart += "</TR>\r\n"; // (a.walling 2007-04-03 15:00) - PLID 25484 - Close the tag
						}
					}

					// (c.haag 2007-08-21 9:28) - PLID 27121 - We used to build the row string data here.
					// Instead, we build the string in astrRowHTMLData. The reason is that Word merges 
					// require every table column to have a TD tag. We accomplish that by storing the 
					// value in astrRowHTMLData, and building a fully filled row with that variable.
					int nColumnOrdinal = -1;
					if (!mapHTMLColOrdinals.Lookup(pCurHTMLColumn, nColumnOrdinal)) {
						ThrowNxException("GetTableDataOutput was called with at least one table element having an invalid column ordinal"); 
					}
					// (z.manning 2010-04-15 17:24) - PLID 38175 - Gray out label rows and make text color blue
					CString strCellStyle;
					if(bRowIsLabel || bColumnIsLabel) {
						strCellStyle = strLabelStyle;
					}
					astrRowHTMLData[ nColumnOrdinal ] = FormatString("<TD ALIGN=center%s>", strCellStyle);
					if(!strDataHTML.IsEmpty() || bRowIsLabel) {
						astrRowHTMLData[ nColumnOrdinal ] += strDataHTML;
						bRowHasData = TRUE;
						if(!bRowIsLabel) {
							// (z.manning 2010-04-14 10:22) - PLID 38175 - If this is a label we do not want to count this
							// as the table being non-empty.
							bTableHasData = TRUE;
						}
					}
					astrRowHTMLData[ nColumnOrdinal ] += "</TD>\r\n";			
				}
				else {
					strHTML = strDetailHTML;
					if(bAllowHtml && (emsf == emsfBulletList || emsf == emsfNumberList)) {
						if(pbDataIsHtml) { *pbDataIsHtml = true; }
					}
				}
			}
		}
		//finish up

		if(!bTable) {
			if(emsf == emsfText) {
				// (b.cardillo 2005-03-22 17:45) - PLID 15008 - Replace the last separator with strSeparatorFinal instead
				// (z.manning 2010-08-02 13:51) - PLID 39842 - This is now handled in GetTableDataOutputRaw
			}
			else if(emsf == emsfBulletList) {
				// (j.jones 2005-02-11 14:41) - PLID 15483 - bulleted/numbered lists require a <br>
				// after them to be more compliant with Word. Read this PL item to understand why.
				strHTML += "</UL>";
				if(strHTML != "<UL></UL>")
					strHTML += "<br>";
			}
			else if(emsf == emsfList) {
				strHTML.TrimRight("<br>");
			}
		}
		else {

			if(bRowHasData) {
				// (c.haag 2007-08-21 9:31) - PLID 27121 - Build the row string using the
				// CStringArray which stores each HTML table element
				for (int n=0; n < nHTMLColumns; n++) {
					strRow += astrRowHTMLData[n];
				}
				strRow += "</TR>"; // (a.walling 2007-04-03 15:00) - PLID 25484 - Close the tag
				strHTML += strRow;
			}

			CString strTableHtml = strTableStart + strHTML + "</table>";
			CString strReplacedLongForm = pDetail->m_strLongForm;
			// (z.manning 2012-03-20 12:02) - PLID 49038 - We need to update demographic fields for tables too.
			ReplaceLongFormDemographicFields(strReplacedLongForm, pDetail, pParentEMN != NULL ? pParentEMN : pDetail->GetParentEMN());
			strHTML = ConvertToHTMLEmbeddable(strReplacedLongForm);
			strHTML.Replace(ConvertToHTMLEmbeddable(DATA_FIELD), strTableHtml);
		}

		// (a.walling 2007-10-23 14:39) - PLID 27849 - Ensure a blank string is returned if there is no data, even if
		// we are not formatting as a table.
		// (z.manning 2010-07-30 17:27) - PLID 39842 - The HTML logic for non-table formats was moved to another function
		// so skip this check.
		if(bTable && !bTableHasData)
			strHTML = "";

		if(emsf == emsfText) {
			// (b.cardillo 2005-03-22 17:45) - PLID 15008 - Replace the last separator with strSeparatorFinal instead
			// (z.manning 2010-08-02 13:52) - PLID 39842 - This is now handled in GetTableDataOutputRaw
		}
		else if(emsf == emsfList) {
			strNonHTML.TrimRight("\r\n");
		}
	}
}

// (c.haag 2007-03-16 11:54) - PLID 25242 - We now support getting EMN's passed through to GetSentence
// (c.haag 2007-03-28 15:06) - PLID 25397 - We also optionally pass in the data output for optimizing the
// execution of functions that need to get both individually
// (c.haag 2008-02-22 13:00) - PLID 29064 - Added optional connection pointer parameter
CString GetSentence(long nEmrDetailID, CMergeEngine *pMi, bool bAllowHtml, bool bInHtml, CStringArray &saTempFiles,
					EmrCategoryFormat Format /*= ecfParagraph*/, CEMN* pParentEMN /*= NULL*/, LPCTSTR szDataOutput /*= NULL*/, CEmrPreviewImageInfo* pPreviewImageInfo /*= NULL*/,
					OPTIONAL IN ADODB::_Connection *lpCon /*= NULL*/)
{
	// (a.walling 2009-10-23 09:23) - PLID 36046 - Track construction in initial reference count
	CEMNDetail *pDetail = CEMNDetail::CreateDetail(NULL, "GetSentence local detail");
	// (a.walling 2012-06-22 14:01) - PLID 51150 - No parent window param
	pDetail->LoadFromDetailID(nEmrDetailID, lpCon);
	CString strRet = GetSentence(pDetail, pMi, bAllowHtml, bInHtml, saTempFiles, Format, pParentEMN, szDataOutput, pPreviewImageInfo, lpCon);
	// (a.walling 2009-10-12 17:20) - PLID 36024 - Properly release the detail
	pDetail->__QuietRelease();
	//delete pDetail;
	return strRet;
}

//TES 2/22/2010 - PLID 37463 - Shareable utility function for pulling the values for the various demographic fields.
// NOTE: This only includes those fields that are valid for both Smart Stamp tables and regular tables.  That is, it does not include
// DATA_FIELD (which Smart Stamps don't use), as well as the 4 fields for Smart Stamps.
CString GetLongFormField(CEMNDetail *pDetail, const CString &strField, CEMN *pParentEMN)
{
	//TES 2/22/2010 - PLID 37463 - Most of these fields will want the gender.
	BYTE bGender = pParentEMN->GetPatientGender();

	if(strField == AGE_FIELD) {
		// (z.manning 2010-01-14 09:29) - PLID 22672 - Age is now a string
		if(!pParentEMN->GetPatientAge().IsEmpty()) {
			return pParentEMN->GetPatientAge();
		}
	}
	else if(strField == GENDER_UPPER_FIELD) {
		switch(bGender) {
			case 1:
				return "Male";
				break;
			case 2:
				return "Female";
				break;
		}
	}
	else if(strField == GENDER_LOWER_FIELD) {
		switch(bGender) {
			case 1:
				return "male";
				break;
			case 2:
				return "female";
				break;
		}
	}
	else if(strField == SUBJ_UPPER_FIELD) {
		switch(bGender) {
			case 1:
				return "He";
				break;
			case 2:
				return "She";
				break;
		}
	}
	else if(strField == SUBJ_LOWER_FIELD) {
		switch(bGender) {
			case 1:
				return "he";
				break;
			case 2:
				// (a.walling 2010-11-12 12:26) - PLID 41465 - This was incorrectly using "her" for the subject tense!
				return "she";
				break;
		}
	}
	else if(strField == OBJ_UPPER_FIELD) {
		switch(bGender) {
			case 1:
				return "Him";
				break;
			case 2:
				return "Her";
				break;
		}
	}
	else if(strField == OBJ_LOWER_FIELD) {
		switch(bGender) {
			case 1:
				return "him";
				break;
			case 2:
				return "her";
				break;
		}
	}
	else if(strField == POSS_UPPER_FIELD) {
		switch(bGender) {
			case 1:
				return "His";
				break;
			case 2:
				return "Her";
				break;
		}
	}
	else if(strField == POSS_LOWER_FIELD) {
		switch(bGender) {
			case 1:
				return "his";
				break;
			case 2:
				return "her";
				break;
		}
	}
	else if(strField == SPAWNING_FIELD) {
		if(pDetail->GetSpawnedGroupID() != -1) {
			// (a.walling 2007-10-19 16:49) - PLID 27820 - This will be escaped immediately before we replace the DATA_FIELD
			//TES 3/18/2010 - PLID 37530 - Pass in the index of the spawning stamp, it's needed to calculate the name.
			
			// (a.walling 2010-05-19 15:15) - PLID 38750 - Pass in the detailstampID as well
			return GetEmrActionName(pDetail->GetSpawnedGroupID(), pDetail->GetSpawnedGroupStampIndex(), eaoInvalid, -1, pDetail->GetSpawnedGroupSourceActionInfo().GetDetailStampID());
		}
	}

	//TES 2/22/2010 - PLID 37463 - We default to blank
	return "";
}

// (z.manning 2010-07-29 16:20) - PLID 39842 - Function that will go through and replace all of the standard long form fields.
void ReplaceLongFormDemographicFields(IN OUT CString &strOutput, CEMNDetail *pDetail, CEMN *pParentEMN)
{
	strOutput.Replace(AGE_FIELD, GetLongFormField(pDetail, AGE_FIELD, pParentEMN));
	strOutput.Replace(GENDER_UPPER_FIELD, GetLongFormField(pDetail, GENDER_UPPER_FIELD, pParentEMN));
	strOutput.Replace(GENDER_LOWER_FIELD, GetLongFormField(pDetail, GENDER_LOWER_FIELD, pParentEMN));
	strOutput.Replace(SUBJ_UPPER_FIELD, GetLongFormField(pDetail, SUBJ_UPPER_FIELD, pParentEMN));
	strOutput.Replace(SUBJ_LOWER_FIELD, GetLongFormField(pDetail, SUBJ_LOWER_FIELD, pParentEMN));
	strOutput.Replace(OBJ_UPPER_FIELD, GetLongFormField(pDetail, OBJ_UPPER_FIELD, pParentEMN));
	strOutput.Replace(OBJ_LOWER_FIELD, GetLongFormField(pDetail, OBJ_LOWER_FIELD, pParentEMN));
	strOutput.Replace(POSS_UPPER_FIELD, GetLongFormField(pDetail, POSS_UPPER_FIELD, pParentEMN));
	strOutput.Replace(POSS_LOWER_FIELD, GetLongFormField(pDetail, POSS_LOWER_FIELD, pParentEMN));
	// (a.walling 2010-05-19 16:39) - PLID 38793 - Don't bother if SPAWNING_FIELD does not exist; otherwise it creates recordsets for no reason.
	if (strOutput.Find(SPAWNING_FIELD) != -1) {
		strOutput.Replace(SPAWNING_FIELD, GetLongFormField(pDetail, SPAWNING_FIELD, pParentEMN));
	}
}

// (z.manning 2010-08-02 16:10) - PLID 39842 - Function to replace the <Data> field in a table's sentence format with
// the appropriate default column-based sentence format.
void ReplaceTableLongFormDataField(IN OUT CString &strOutput, CEMNDetail *pDetail)
{
	CString strDefaultTableLongForm;
	for(int nColIndex = 0; nColIndex < pDetail->GetColumnCount(); nColIndex++)
	{
		TableColumn *ptc = pDetail->GetColumnPtr(nColIndex);
		strDefaultTableLongForm += "<" + ptc->strName + "> ";
	}
	strDefaultTableLongForm.TrimRight();

	strOutput.Replace(DATA_FIELD, strDefaultTableLongForm);
}

// (z.manning 2010-08-10 12:52) - PLID 39497 - Will substitute any table sentence format "groups" with 
// a placeholder for the text that should eventually go there. That replacement text is stored in
// arystrReplacementText.
// (a.walling 2011-06-20 17:57) - PLID 44215 - Need to keep track of set of data IDs to pass on to the recursive GetTableDataOutputRaw call
//TES 3/22/2012 - PLID 48203 - Added an optional pParentEMN parameter
BOOL ReplaceSentenceFormatGroupsWithPlaceholders(CEMNDetail *pDetail, IN OUT CString &strLongForm, bool bUseHTML, OUT CStringArray &arystrReplacementText, TableRowID* pSingleTableRowID, CArray<long,long>* paTableEmrDataIDs, CEMN *pParentEMN /*= NULL*/)
{
	int nPos = 0;
	BOOL bHasAtLeastOneGroup = FALSE;
	// (z.manning 2010-08-11 09:13) - PLID 39497 - Look for the field that denotes the beginning of a group
	int nGroupBegin = strLongForm.Find(GROUP_BEGIN_FIELD, nPos);
	while(nGroupBegin != -1)
	{
		// (z.manning 2010-08-11 09:15) - PLID 39497 - Now see if we can find a group end field that defines
		// a complete group. Note: We don't support group nesting at this point.
		int nGroupEnd = strLongForm.Find(GROUP_END_FIELD, nGroupBegin);
		if(nGroupEnd != -1)
		{
			bHasAtLeastOneGroup = TRUE;
			// (z.manning 2010-08-11 09:18) - PLID 39497 - Find the point immediately after the group begin field.
			int nStart = nGroupBegin + CString(GROUP_BEGIN_FIELD).GetLength();
			// (z.manning 2010-08-11 09:19) - PLID 39497 - Find the sentence format between the group begin and end tags
			CString strGroupLongForm = strLongForm.Mid(nStart, nGroupEnd - nStart);
			// (z.manning 2010-08-11 09:19) - PLID 39497 - Now get the data output for the entire table based only on 
			// this group within the sentence format.
			CString strGroupHTML, strGroupNonHTML;
			// (c.haag 2011-04-05) - PLID 43145 - Pass in NULL for the EmrDataIDs filter; we don't use it here
			// (c.haag 2011-04-12) - PLID 43245 - Pass in NULL for the table caption
			//TES 3/22/2012 - PLID 48203 - Pass in pParentEMN
			pDetail->GetTableDataOutputRaw(strGroupLongForm, strGroupHTML, strGroupNonHTML, pSingleTableRowID, paTableEmrDataIDs, NULL, NULL, pParentEMN);
			// (z.manning 2010-08-11 09:21) - PLID 39497 - Now remove this group from the overall sentence format,
			// including the group begin and end fields.
			strLongForm.Delete(nGroupBegin, CString(GROUP_BEGIN_FIELD).GetLength() + strGroupLongForm.GetLength() + CString(GROUP_END_FIELD).GetLength());
			// (z.manning 2010-08-11 09:21) - PLID 39497 - Now move are starting search position to the group we just handled.
			nPos = nGroupBegin;
			// (z.manning 2010-08-11 09:22) - PLID 39497 - Now insert a placeholder to where the group sentence format should go.
			// We can't simply insert the replacement text here because it may be formatted for HTML and the caller may have
			// more HTML escapting to do but this group HTML has already been esacaped and formatted properly.
			strLongForm.Insert(nPos, GROUP_PLACEHOLDER);
			// (z.manning 2010-08-11 09:24) - PLID 39497 - Instead, let's store the replacement text in a string array and
			// the caller can substitute it when it's ready.
			if(bUseHTML) {
				arystrReplacementText.Add(strGroupHTML);
			}
			else {
				arystrReplacementText.Add(strGroupNonHTML);
			}
		}
		else {
			// (z.manning 2010-08-11 09:26) - PLID 39497 - This group begin tag does not have a corresponding group end
			// tag so we can't possibly have any more groups so stop looping.
			break;
		}

		// (z.manning 2010-08-11 09:27) - PLID 39497 - See if we have another group
		nGroupBegin = strLongForm.Find(GROUP_BEGIN_FIELD, nPos);
	}

	return bHasAtLeastOneGroup;
}

// (c.haag 2007-03-16 11:54) - PLID 25242 - We now support getting EMN's passed through to GetSentence
// (c.haag 2007-03-28 15:06) - PLID 25397 - We also optionally pass in the data output for optimizing the
// execution of functions that need to get both individually
// (c.haag 2008-02-22 13:00) - PLID 29064 - Added optional connection pointer parameter
CString GetSentence(CEMNDetail *pDetail, CMergeEngine *pMi, bool bAllowHtml, bool bInHtml, CStringArray &saTempFiles,
					EmrCategoryFormat Format /*= ecfParagraph*/, CEMN* pParentEMN /*= NULL*/, LPCTSTR szDataOutput /*= NULL*/, CEmrPreviewImageInfo* pPreviewImageInfo /*= NULL*/,
					OPTIONAL IN ADODB::_Connection *lpCon /*= NULL*/)
{
	// (c.haag 2008-02-22 15:25) - PLID 29064 - Since we create recordsets in this function, we need to have a valid
	// connection pointer
	_ConnectionPtr pCon;
	if(lpCon) pCon = lpCon;
	else pCon = GetRemoteData();

	bool bDataIsHtml = false;
	// (c.haag 2007-03-16 11:58) - PLID 25242 - Assign the parent EMN if it's not already
	if (NULL == pParentEMN) {
		if (NULL != pDetail) {
			if (NULL != pDetail->m_pParentTopic) {
				pParentEMN = pDetail->m_pParentTopic->GetParentEMN();
			} else {
				// If we have a detail without a parent topic, then we have no parent EMN. This
				// is unexpected, but don't error out until the last second later on.
			}
		} else {
			// The detail is NULL; but that may be by intent. Don't error out until the last second 
			// later on.
		}
	}

	//TES 3/3/2010 - PLID 37463 - Moved this code up here, so that LoadContent would be called before GetDataOutput()
	CString strFormatted;

	if(pDetail) {
		//we cached the LongForm, so use that instead of accessing the database
		pDetail->LoadContent(FALSE, NULL, lpCon); //won't actually load anything if it has already been loaded
		strFormatted = pDetail->m_strLongForm;
	}
	else {
		//TES 2/22/2010 - PLID 37643 - If we're here, pDetail must be NULL!  So how can we be using its EMRInfoID?
		/*_RecordsetPtr rsSentence = CreateRecordset(pCon, "SELECT LongForm, UseSmartStampsLongForm, SmartStampsLongForm "
			"FROM EmrInfoT WHERE ID = %li", pDetail->m_nEMRInfoID);*/

		//if(rsSentence->eof) {
			//It's up to our caller to give us a valid id.
			ASSERT(FALSE);
			strFormatted = "<Data";
		/*}
		//It's up to our caller to give us a valid id.
		strFormatted = AdoFldString(rsSentence, "LongForm");*/
	}

	CString strDataOutput;
	// (c.haag 2007-03-28 15:54) - PLID 25397 - Use the data output string that we passed in if necessary
	if (NULL == szDataOutput) {
		strDataOutput = GetDataOutput(pDetail, pMi, bAllowHtml, bInHtml, saTempFiles, &bDataIsHtml, Format, pParentEMN, pPreviewImageInfo, lpCon);
	} else {
		strDataOutput = szDataOutput;
	}

	// (c.haag 2007-04-03 10:54) - PLID 25397 - This conditional should be after the data output calculation
	if(strDataOutput.IsEmpty()) {
		//TES 9/2/2004: Don't put the sentence around a blank, it looks silly.
		return strDataOutput;
	}

	if(pDetail->m_EMRInfoType == eitTable) {
		strFormatted = strDataOutput;
	}
	
	//Replace all "merge fields" with their actual values.
	// (a.walling 2007-10-19 16:48) - PLID 27820 - The data is now replaced last

	//TES 2/22/2010 - PLID 37463 - Moved all this code into a GetLongFormField() utility, that could also be used by CEMNDetail.
	strFormatted.Replace(AGE_FIELD, GetLongFormField(pDetail, AGE_FIELD, pParentEMN));
	strFormatted.Replace(GENDER_UPPER_FIELD, GetLongFormField(pDetail, GENDER_UPPER_FIELD, pParentEMN));
	strFormatted.Replace(GENDER_LOWER_FIELD, GetLongFormField(pDetail, GENDER_LOWER_FIELD, pParentEMN));
	strFormatted.Replace(SUBJ_UPPER_FIELD, GetLongFormField(pDetail, SUBJ_UPPER_FIELD, pParentEMN));
	strFormatted.Replace(SUBJ_LOWER_FIELD, GetLongFormField(pDetail, SUBJ_LOWER_FIELD, pParentEMN));
	strFormatted.Replace(OBJ_UPPER_FIELD, GetLongFormField(pDetail, OBJ_UPPER_FIELD, pParentEMN));
	strFormatted.Replace(OBJ_LOWER_FIELD, GetLongFormField(pDetail, OBJ_LOWER_FIELD, pParentEMN));
	strFormatted.Replace(POSS_UPPER_FIELD, GetLongFormField(pDetail, POSS_UPPER_FIELD, pParentEMN));
	strFormatted.Replace(POSS_LOWER_FIELD, GetLongFormField(pDetail, POSS_LOWER_FIELD, pParentEMN));
	// (a.walling 2010-05-19 16:39) - PLID 38793 - Don't bother if SPAWNING_FIELD does not exist; otherwise it creates recordsets for no reason.
	if (strFormatted.Find(SPAWNING_FIELD) != -1) {
		strFormatted.Replace(SPAWNING_FIELD, GetLongFormField(pDetail, SPAWNING_FIELD, pParentEMN));
	}

	// (a.walling 2007-10-19 16:46) - PLID 27820 - Now that all the "merge fields" have been replaced,
	// we need to escape and replace the data field
	// (a.walling 2007-10-23 12:42) - PLID 27820 - If we are in HTML, and allowing it, we need to escape
	// also escape if we are not in HTML and the data is HTML.
	if ( (bAllowHtml && bInHtml) || (bDataIsHtml && !bInHtml) ) {
		//strDataOutput has already been escaped.
		// (z.manning 2010-08-02 14:01) - PLID 39842 - Tables have already been escaped
		if(pDetail->m_EMRInfoType != eitTable) {
			strFormatted = ConvertToHTMLEmbeddable(strFormatted);
		}
		//Replace the escaped DATA_FIELD with the actual value.
		strFormatted.Replace(ConvertToHTMLEmbeddable(DATA_FIELD), strDataOutput);
	} else {
		//Replace <Data> with the actual value.
		strFormatted.Replace(DATA_FIELD, strDataOutput);
	}

	if(bDataIsHtml && !bInHtml) {
		//OK, we need to generate an HTML file.
		CString strFilename;
		strFilename.Format("MergeHTML_EMR_List_%s_%d.htm", pDetail->GetMergeFieldName(TRUE), pParentEMN->GetID());
		CString strFullPath = GetNxTempPath() ^ strFilename;
		CStdioFile f(strFullPath, CFile::modeCreate | CFile::modeWrite | CFile::typeText | CFile::shareCompat);
		CString strHTML = "<html><head></head><body>" + strFormatted + "</body></html>";
		f.WriteString(strHTML);
		f.Close();

		// Add to our temp file list
		saTempFiles.Add(strFullPath);

		//Now, return the HTML file as our data.
		strFormatted.Format("{NXRTF %s}", strFullPath);
	}

	return strFormatted;
}

CString ConvertToHeaderName(const CString &strPrefix, const CString &strHeaderBaseText, OPTIONAL OUT BOOL *pbTruncated /*= NULL*/)
{
	CString strAns(strHeaderBaseText);
	// First replace every non-alphanumeric character with an underscore
	for (long i=0; i<strAns.GetLength(); i++) {
		// (a.walling 2008-10-03 17:29) - PLID 31589 - ASSERTion if you try to pass a signed char to any ::is* functions.
		if (!isalnum(unsigned char(strAns.GetAt(i)))) {
			strAns.SetAt(i, '_');
		}
	}

	// (j.jones 2013-11-21 16:21) - PLID 59692 - moved this prefix code to be before the underscore
	// replace code, because otherwise if your item began with a character that turned into an
	// underscore, we would never fix the newly created double-underscore caused by the prefix
	strAns = strPrefix + "_" + strAns;

	// Then make every sequence of more than one underscore into a single underscore
	while (strAns.Replace("__", "_"));

	// And return	
	if (strAns.GetLength() > 39) {
		if (pbTruncated) {
			*pbTruncated = TRUE;
		}
		return strAns.Left(39);
	} else {
		if (pbTruncated) {
			*pbTruncated = FALSE;
		}
		return strAns;
	}
}


CString GenerateXMLFromSemiColonDelimitedIDList(const CString &strSemiColonDelimitedIDList)
{
	CString strAns(strSemiColonDelimitedIDList);
	strAns.Replace(" ", "");
	strAns.Replace(";", "\" /><P ID=\"");
	if (!strAns.IsEmpty()) {
		return "<ROOT><P ID=\"" + strAns + "\" /></ROOT>";
	} else {
		return "<ROOT></ROOT>";
	}
}

void FillArrayFromSemiColonDelimitedIDList(IN OUT CDWordArray &arydw, IN const CString &strSemiColonDelimitedIDList)
{
	// (z.manning, 01/23/2008) - PLID 28690 - I rewrote this function so that it handles all semicolon delimited ID lists.
	arydw.RemoveAll();
	CString strWorking = strSemiColonDelimitedIDList;
	while (!strWorking.IsEmpty())
	{
		int nSemicolon = strWorking.Find(';');
		if(nSemicolon == -1) {
			nSemicolon = strWorking.GetLength();
		}
		
		arydw.Add(AsLong(_bstr_t(strWorking.Left(nSemicolon))));
		if(nSemicolon >= strWorking.GetLength()) {
			strWorking.Empty();
		}
		else {
			strWorking.Delete(0, nSemicolon + 1);
		}
	}
}

BOOL IsIDInSemiColonDelimitedIDList(IN const CString &strSemiColonDelimitedIDList, IN const long nLookForID)
{
	long nPos = 0;
	while (nPos < strSemiColonDelimitedIDList.GetLength()) {
		// Get the current key and move the index to the beginning of the next key
		long nLen = strSemiColonDelimitedIDList.Find(";", nPos) - nPos;
		if (nLen < 0) {
			nLen = strSemiColonDelimitedIDList.GetLength() - nPos;
		}
		CString strCurID = strSemiColonDelimitedIDList.Mid(nPos, nLen);
		nPos = nPos + nLen + 2;
		// See if this is a match
		if (atol(strCurID) == nLookForID) {
			return TRUE;
		}
	}
	// If we made it here we found no match
	return FALSE;
}

long RemoveIDFromSemiColonDelimitedIDList(IN OUT CString &strSemiColonDelimitedIDList, IN const long nRemoveID)
{
	BOOL bAns = FALSE;

	long nPos = 0;
	while (nPos < strSemiColonDelimitedIDList.GetLength()) {
		// Get the current key and move the index to the beginning of the next key
		long nLen = strSemiColonDelimitedIDList.Find(";", nPos) - nPos;
		if (nLen < 0) {
			nLen = strSemiColonDelimitedIDList.GetLength() - nPos;
		}
		CString strCurID = strSemiColonDelimitedIDList.Mid(nPos, nLen);
		nPos = nPos + nLen + 2;
		// See if this is a match
		if (atol(strCurID) == nRemoveID) {
			// nPos - (nLen + 2) will be the beginning of our nRemoveID entry, BUT if we're past the end of 
			// the string, then nRemoveID was the last ID in the string, which means we also want to delete 
			// the semicolon preceding nRemoveID if there is one.
			if (nPos > strSemiColonDelimitedIDList.GetLength()) {
				// We past the end, which means it was either of the form "<id1>; <id2>; <nRemoveID>" 
				// or just "<nRemoveID>".  Figure out which it is.
				if (nLen == strSemiColonDelimitedIDList.GetLength()) {
					// It's of the form "<nRemoveID>" so just clear it
					nPos = 0;
					strSemiColonDelimitedIDList.Empty();
				} else {
					// It's of the form "<id1>; <id2>; <nRemoveID>" so we have to also remove the preceding "; "
					nPos -= (nLen + 2 + 2);
					strSemiColonDelimitedIDList.Delete(nPos, nLen + 2);
				}
			} else {
				// It's of the form "<nRemoveID>; <id1>; <id2>" or "<id1>; <nRemoveID>; <id2>"
				nPos -= (nLen + 2);
				strSemiColonDelimitedIDList.Delete(nPos, nLen + 2);
			}
			// Remember we deleted one
			bAns = TRUE;
		}
	}
	// Return whether we removed anything or not
	return bAns;
}

long FindInList(const CDWordArray &arydw, DWORD dwFindElement)
{
	for (long i=0; i<arydw.GetSize(); i++) {
		if (arydw.GetAt(i) == dwFindElement) {
			// Found it
			return i;
		}
	}
	// If we made it here we didn't find it
	return -1;
}

void CalcChangedDataIDFromState(const _variant_t &varOldState, const _variant_t &varNewState, OUT CArray<long, long> &aryNewlySelDataID, OUT CArray<long, long> &aryNewlyUnselDataID)
{
	// We expect the given state variants to be either strings or nothing
	ASSERT((varOldState.vt == VT_NULL || varOldState.vt == VT_BSTR) && (varNewState.vt == VT_NULL || varNewState.vt == VT_BSTR));

	// Fill an array of DataIDs based on the old state
	CDWordArray aryOld;
	FillArrayFromSemiColonDelimitedIDList(aryOld, VarString(varOldState, ""));

	// Fill an array of DataIDs based on the new state
	CDWordArray aryNew;
	FillArrayFromSemiColonDelimitedIDList(aryNew, VarString(varNewState, ""));

	// Now loop through the new until we find one that was not in the old
	{
		for (long i=0; i<aryNew.GetSize(); i++) {
			// See what ID we're dealing with
			DWORD dwID = aryNew.GetAt(i);
			// See if this id is in the old list
			if (FindInList(aryOld, dwID) == -1) {
				// This ID exists in new and not in old
				aryNewlySelDataID.Add(dwID);
			}
		}
	}

	// Then do the opposite: loop through the old until we find one that was not in the new
	{
		for (long i=0; i<aryOld.GetSize(); i++) {
			// See what ID we're dealing with
			DWORD dwID = aryOld.GetAt(i);
			// See if this id is in the new list
			if (FindInList(aryNew, dwID) == -1) {
				// This ID exists in old and not in new
				aryNewlyUnselDataID.Add(dwID);
			}
		}
	}

	// Done
}

//generates the #NewObjectsT table declaration that will be used in the EMR batch saving
// (a.walling 2014-01-30 00:00) - PLID 60546 - Quantize
Nx::Quantum::Batch GenerateEMRBatchSaveNewObjectTableDeclaration()
{
	//will return the declaration for the #NewObjectsT table
	//this table will store the new IDs created, the type of ID it is,
	//and the pointer to the object that will later need the new ID assigned to it

	//the Types are defined as an EmrSaveObjectType enum in EmrUtils.h
	Nx::Quantum::Batch strDeclaration;
	// (a.walling 2014-01-30 00:00) - PLID 60550 - Explicitly declare variables and types for the Quantum::Batch
	strDeclaration
		.Declare("@nNewObjectID", "INT")
		.Declare("@nEMRGroupID", "INT")
		.Declare("@nEMRTemplateID", "INT")
		.Declare("@nEMNID", "INT")
		.Declare("@nEMRTemplateTopicID", "INT")
		.Declare("@nEMRTopicID", "INT")
		.Declare("@nEMRTemplateDetailID", "INT")
		.Declare("@nEMRDetailID", "INT")
		.Declare("@nEMRDataID", "INT")
		.Declare("@nParentTopicID", "INT")
		.Declare("@nEMRProblemID", "INT")
		.Declare("@nEMRObjectIDToUpdate", "INT")
		.Declare("@nEMRSourceDetailID", "INT")
		.Declare("@strLabDetailText", "NVARCHAR(4000)") // (z.manning 2008-10-30 09:25) - PLID 31613
		.Declare("@nNewObjectIDForProblems", "INT") // (a.walling 2009-01-09 11:50) - PLID 32466
		.Declare("@nEMRChildEMRDetailID", "INT")	// (j.jones 2010-02-12 11:15) - PLID 37318
		.Declare("@nEMRChildEMRTemplateDetailID", "INT")
		.Declare("@nEmrDetailImageStampID", "INT") // (z.manning 2010-02-19 12:16) - PLID 37404
		.Declare("@nSourceDetailImageStampID", "INT") // (z.manning 2010-02-26 17:24) - PLID 37540
		// (a.walling 2013-04-17 16:46) - PLID 55652 - order of insertion is required on output; using ObjectPtr as primary key, InsertOrder as identity for the final select	
		// (a.walling 2014-01-30 00:00) - PLID 60541 - #NewObjectsT now a table so can be referenced by other sprocs
		.AddRaw("IF OBJECT_ID('tempdb..#NewObjectsT') IS NOT NULL DROP TABLE #NewObjectsT")
		.AddRaw("CREATE TABLE #NewObjectsT ( \r\n"
		"	InsertOrder int not null identity, \r\n"
		"	ID int not null, \r\n"
		"	Type int not null, \r\n"
		"	ObjectPtr int not null, \r\n"
		"   PRIMARY KEY (Type, ObjectPtr) \r\n"
		") \r\n")
	;

	return strDeclaration;
}

// (c.haag 2007-06-20 12:14) - PLID 26397 - We now store saved objects in a map for fast lookups
void AddNewEMRObjectToSqlBatch(IN OUT Nx::Quantum::Batch& strSaveString, EmrSaveObjectType esotSaveObject, long nObjectPtr, IN CMapPtrToPtr& mapSavedObjects)
{
	// (c.haag 2007-06-20 12:36) - PLID 26397 - Check if we've already added the object to the saved
	// object map. If so, that means we're about to generate a save string for the same object twice, and
	// possibly cause data corruption. Throw an exception so that we can avoid the save and avoid the corruption.
	if (mapSavedObjects[(LPVOID)nObjectPtr] != NULL) {
		ASSERT(FALSE);
		ThrowNxException("Called AddNewEMRObjectToSqlBatch for an object that already exists in the batch!");
	}
	// (a.walling 2014-01-30 00:00) - PLID 60541 - #NewObjectsT now a table so can be referenced by other sprocs
	AddStatementToSqlBatch(strSaveString, "INSERT INTO #NewObjectsT (ID, Type, ObjectPtr) VALUES (@nNewObjectID, %li, %li)", esotSaveObject, nObjectPtr);
}

// (a.walling 2013-03-14 11:05) - PLID 55652 - Get the object pointer as well since we use that as a primary key in #NewObjectsT
void AddDeletedEMRObjectToSqlBatch(IN OUT Nx::Quantum::Batch& strSaveString, EmrSaveObjectType esotSaveObject, long nObjectPtr, long nObjectID)
{
	// (a.walling 2014-01-30 00:00) - PLID 60541 - #NewObjectsT now a table so can be referenced by other sprocs
	AddStatementToSqlBatch(strSaveString, "INSERT INTO #NewObjectsT (ID, Type, ObjectPtr) VALUES (%li, %li, %li)", nObjectID, esotSaveObject, nObjectPtr);
}

//DRT 12/15/2005 - Moved from patientemrdlg.cpp.
// Prompts the user to enter a new collection name and keeps prompting if the user tries to 
// enter an invalid or already-existing name, then tries to create the record in the database.  
// Returns the new ID if the record is created successfully, or -1 if the user cancels.  Can 
// throw exceptions if there is some sort of database malfunction.
long CreateNewEMRCollection()
{
	CString strNewCollectionName;
	while (true) {
		//TES 9/15/2006 - Note: this column actually allows 500 characters, but it has a uniqueness key, which apparently only allows 450.
		int nResult = InputBoxLimited(NULL, "Enter a name for the new collection", strNewCollectionName, "",450,false,false,NULL);
		if (nResult == IDOK) {
			// Validate it
			strNewCollectionName.TrimLeft();
			strNewCollectionName.TrimRight();
			if (!strNewCollectionName.IsEmpty()) {
				if (strNewCollectionName.GetLength() <= 500) {
					// It's good text-wise, now make sure it doesn't already exist
					// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
					if (!ReturnsRecordsParam("SELECT * FROM EMRCollectionT WHERE Name = {STRING}", strNewCollectionName)) {
						// Create the new collection and get its id
						_RecordsetPtr prs = CreateRecordset(
							"SET NOCOUNT ON \r\n"
							"INSERT INTO EMRCollectionT (Name, MenuOrder) VALUES ('%s', %li) \r\n"
							"SET NOCOUNT OFF \r\n"
							"SELECT CONVERT(INT, SCOPE_IDENTITY()) AS NewID", _Q(strNewCollectionName), NewNumber("EmrCollectionT","MenuOrder"));
						return AdoFldLong(prs->GetFields(), "NewID");
					} else {
						// Already exists
						AfxMessageBox("The collection name already exists (possibly as an inactive collection).  Please enter a new collection name.", NULL, MB_ICONINFORMATION|MB_OK);
					}
				} else {
					// Too long
					AfxMessageBox("The collection name is too long.  Please enter a new collection name fewer than 500 characters long.", NULL, MB_ICONINFORMATION|MB_OK);
				}
			} else {
				// Nothing entered
				AfxMessageBox("The collection name is blank.  Please enter a new collection name.", NULL, MB_ICONINFORMATION|MB_OK);
			}
		} else {
			// The user canceled
			return -1;
		}
	}
}

/**************************************************************************************
// (c.haag 2007-02-07 09:24) - If you change this function you must also change
BranchCurrentMedicationsInfoItem here and in the EMR importer!!!!!!
**************************************************************************************/

BOOL CopyEmrInfoItem(IN long nInfoMasterID, IN const CString &strName, BOOL bSilent,
					 OPTIONAL OUT long * pnNewMasterID /*= NULL*/,
					 OPTIONAL OUT long * pnNewInfoID /*= NULL*/,
					 OPTIONAL OUT CString *pstrNewName /*= NULL*/)
{
	long nNewInfoID;
	long nNewMasterID;
	CString strNewName;

	// (c.haag 2007-01-30 17:18) - PLID 24423 - We must not be able to copy the
	// Current Medications item
	// (c.haag 2007-04-03 09:44) - PLID 25468 - We must not be able to copy the Allergies item.
	// I made the message be a bit vaguer because we still want to use only one query for efficiency
	// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
	if (ReturnsRecordsParam(FormatString("SELECT ID FROM EmrInfoMasterT WHERE ID = {INT} AND ActiveEmrInfoID IN "
		"(SELECT ID FROM EmrInfoT WHERE DataSubType IN (%d,%d))", eistCurrentMedicationsTable, eistAllergiesTable), 
		nInfoMasterID))
	{
		if (bSilent) {
			ThrowNxException("Tried to copy a system EMR item!");
		} else {
			AfxMessageBox("You may not copy a system EMR item.");
			return FALSE;
		}
	}

	//Figure out which EmrInfoT record we're copying.
	long nInfoID = VarLong(GetTableField("EmrInfoMasterT", "ActiveEmrInfoID", "ID", nInfoMasterID));

	// (c.haag 2010-07-10 12:07) - PLID 39467 - Don't reassign if there are any table dropdowns with
	// duplicate SortOrder values. That is considered bad data and we don't to propogate that.
	if (EmrItemHasTableDropdownsWithDuplicateSortOrders(nInfoID)) 
	{
		ThrowNxException("This EMR item has one or more table dropdowns with duplicate Sort Order values. Please contact NexTech support for assistance.");
	}

	if (bSilent) {
		// (c.haag 2006-02-24 11:07) - PLID 17195 - If we're silent, then just copy
		// the name verbatim
		strNewName = strName;
	} 
	else {
		BOOL bValid = FALSE;
		strNewName = "Copy of " + strName;

		while(!bValid) {
			long nRes = InputBoxLimited(NULL, "Enter a name for the new item", strNewName, "",255,false,false,NULL);
			if(nRes == IDCANCEL)
				return FALSE;

			//require non-empty
			strNewName.TrimRight();

			// (j.jones 2006-03-02 11:09) - PLID 19372 - if there is any other active EMR item with the same name,
			// and we are changing this item's name, then stop them from saving
			// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
			if(ReturnsRecordsParam("SELECT EmrInfoT.ID FROM EMRInfoT INNER JOIN EmrInfoMasterT ON EmrInfoT.ID = EmrInfoMasterT.ActiveEmrInfoID "
				"WHERE Name = {STRING} AND Inactive = 0", strNewName)) {
				if(IDNO == MessageBox(GetActiveWindow(), "There is already an active EMR item with this name.\n"
					"It is recommended that you enter unique names for EMR items to minimize confusion.\n\n"
					"Do you still wish to use this name?","Practice",MB_ICONQUESTION|MB_YESNO)) {
					bValid = FALSE;
					continue;
				}
			}
			
			if(!strNewName.IsEmpty())
				bValid = TRUE;
			else {
				AfxMessageBox("You must type in a name for this item.");
			}
		}
	}


	//
	//Now do the deed of copying all this stuff
	BOOL bSaved = FALSE;

	BEGIN_TRANS("EMRInfoCopy")
		//TES 12/7/2006 - PLID 23724 - We are copying a full EmrInfoMasterT record. So first, do that.
		// (r.gonet 2013-04-11 18:17) - PLID 52761 - Ensure that NOCOUNT has not accidentally been left on, otherwise we will be getting an access violation.
		_RecordsetPtr rsNewMasterID = CreateRecordset(
			"SET NOCOUNT OFF; "
			"INSERT INTO EmrInfoMasterT (ActiveEmrInfoID) VALUES (NULL); "
			"SELECT CONVERT(INT,SCOPE_IDENTITY()) AS NewID;");
		_variant_t v;
		rsNewMasterID = rsNewMasterID->NextRecordset(&v);
		nNewMasterID = AdoFldLong(rsNewMasterID, "NewID");

		// (c.haag 2007-01-31 09:21) - PLID 24376 - We now consider the DataSubType field
		// (a.walling 2008-08-12 12:25) - PLID 30570 - Copy the preview flags
		// (j.jones 2008-09-22 15:12) - PLID 31476 - supported RememberForEMR
		// (c.haag 2008-10-16 09:59) - PLID 31708 - Supported TableRowsAsFields
		// (z.manning 2010-02-25 14:53) - PLID 37228 - Added ChildEmrInfoMasterID, SmartStampsEnabled
		//TES 2/25/2010 - PLID 37535 - Added SmartStampsLongForm.  Note that we do NOT want to copy UseSmartStampsLongForm, because
		// this copy will not in fact be a "Smart Stamps" table unless and until it gets linked to an image.  In the meantime, we don't
		// want it to attempt to use the SmartStampsLongForm, though we still copy it so it will be pre-set when the time comes.
		// (z.manning 2010-07-26 14:46) - PLID 39848 - SmartStampsLongForm and UseSmartStampsLongForm are now deprecated anyway
		//TES 3/15/2011 - PLID 42757 - Added HasGlassesOrderData, GlassesOrderLens
		// (z.manning 2011-11-15 16:45) - PLID 46485 - Added InfoFlags
		//TES 4/6/2012 - PLID 49367 - Added HasContactLensData
		// (r.gonet 08/03/2012) - PLID 51948 - Added UseWithWoundCareCoding
		// (j.armen 2014-01-29 12:06) - PLID 60523 - Idenitate EMRInfoT
		_RecordsetPtr prs = CreateParamRecordset("SET NOCOUNT ON\r\n"
			"INSERT INTO EMRInfoT "
			"(Name, DataType, DataSubType, LongForm, BackgroundImageFilePath, BackgroundImageType, DefaultPenColor, "
			"DataFormat, DataSeparator, DefaultText, RememberForPatient, RememberForEMR, SliderMin, SliderMax, SliderInc, "
			"DataSeparatorFinal, OnePerEmn, AutoAlphabetizeListData, DisableTableBorder, PreviewFlags, EmrInfoMasterID, TableRowsAsFields, "
			"ChildEmrInfoMasterID, SmartStampsEnabled, HasGlassesOrderData, GlassesOrderLens, InfoFlags, HasContactLensData, "
			"UseWithWoundCareCoding) "

			"SELECT {STRING}, DataType, DataSubType, LongForm, BackgroundImageFilePath, BackgroundImageType, DefaultPenColor, "
			"DataFormat, DataSeparator, DefaultText, RememberForPatient, RememberForEMR, SliderMin, SliderMax, SliderInc, "
			"DataSeparatorFinal, OnePerEmn, AutoAlphabetizeListData, DisableTableBorder, PreviewFlags, {INT}, TableRowsAsFields, "
			"ChildEmrInfoMasterID, SmartStampsEnabled, HasGlassesOrderData, GlassesOrderLens, InfoFlags, HasContactLensData, "
			"UseWithWoundCareCoding "
			"FROM EMRInfoT WHERE ID = {INT}\r\n"

			"SET NOCOUNT OFF\r\n"
			"SELECT CONVERT(INT,SCOPE_IDENTITY()) AS EMRInfoID;"
			, strNewName, nNewMasterID, nInfoID);

		nNewInfoID = AdoFldLong(prs, "EMRInfoID");

		//We are the only, and therefore the active, record for our master.
		ExecuteParamSql("UPDATE EmrInfoMasterT SET ActiveEmrInfoID = {INT} WHERE ID = {INT}", nNewInfoID, nNewMasterID);

		// (z.manning 2011-10-24 15:48) - PLID 46082 - Copy stamp exclusions
		ExecuteParamSql(
			"INSERT INTO EmrInfoStampExclusionsT (EmrInfoMasterID, StampID) \r\n"
			"SELECT {INT}, StampID \r\n"
			"FROM EmrInfoStampExclusionsT \r\n"
			"WHERE EmrInfoMasterID = {INT} \r\n"
			, nNewMasterID, nInfoMasterID);

		//audit the creation
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(-1, "", nAuditID, aeiEMRItemCreated, nNewInfoID, "", strNewName, aepMedium, aetCreated);

		//1b.) TES 1/30/2007 - PLID 24377 - Copy all the links this item is a part of, and remember the mapping so that 
		// we can also copy the details.
		CMap<long,long,long,long&> mapLinkIDs;
		//DRT 8/30/2007 - PLID 27259 - Parameterized.
		_RecordsetPtr rsLinks = CreateParamRecordset("SELECT ID, Name FROM EmrItemLinksT WHERE ID IN "
			" (SELECT EmrLinkID FROM EmrItemLinkedDataT WHERE EmrDataID IN (SELECT ID FROM EmrDataT WHERE EmrInfoID = {INT}))",
			nInfoID);
		while(!rsLinks->eof) {
			long nOldID = AdoFldLong(rsLinks, "ID");
			CString strName = AdoFldString(rsLinks, "Name");
			long nNewID = NewNumber("EmrItemLinksT", "ID");
			mapLinkIDs.SetAt(nOldID, nNewID);
			ExecuteSql("INSERT INTO EmrItemLinksT (ID, Name) VALUES (%li, '%s')", nNewID, _Q(strName));
			//TES 1/30/2007 - PLID 24377 - Also, copy the other side of the link (whichever details are not part of this item.).
			ExecuteSql("INSERT INTO EmrItemLinkedDataT (EmrLinkID, EmrDataID) "
				"SELECT %li, EmrDataID FROM EmrItemLinkedDataT WHERE EmrLinkID = %li AND EmrDataID NOT IN "
				" (SELECT ID FROM EmrDataT WHERE EmrInfoID = %li)", nNewID, nOldID, nInfoID);
			rsLinks->MoveNext();
		}
		rsLinks->Close();

		//2)  Copy all the records in EMRDataT
		long nNewDataID = NewNumber("EMRDataT", "ID");

		// (j.jones 2007-08-14 16:02) - PLID 27053 - added EMRDataGroupsT
		long nEMRDataGroupID = NewNumber("EMRDataGroupsT", "ID");

		//DRT 8/30/2007 - PLID 27259 - Parameterized.
		_RecordsetPtr prsData = CreateParamRecordset("SELECT ID, EMRDataGroupID FROM EMRDataT WHERE EMRInfoID = {INT}", nInfoID);
		// (c.haag 2011-03-15) - PLID 42821 - We cannot copy System Info items, but I'm leaving this code intact in case
		// we ever introduce common list buttons in non-system tables.
		// (c.haag 2011-03-15) - PLID 42821 - Preserve a mapping of the Emr Data ID's. I know we simply increment the
		// nNewDataID index; but if we ever change that, I don't want it to break my code so easily!
		// (j.jones 2012-09-21 13:49) - PLID 52316 - restored the mapping
		// (j.gruber 2013-10-02 13:58) - PLID 58676 - added mapping for groupIDs
		CMap<long,long,long,long> mapOldToNewEmrDataID;
		CMap<long,long,long,long> mapOldToNewEmrDataGroupID;
		while(!prsData->eof) {
			long nOldDataID = AdoFldLong(prsData, "ID");
			long nOldDataGroupID = AdoFldLong(prsData, "EMRDataGroupID");

			// (j.jones 2007-08-14 16:02) - PLID 27053 - added EMRDataGroupsT
			ExecuteSql("INSERT INTO EMRDataGroupsT (ID) VALUES (%li)", nEMRDataGroupID);

			//Make a new record of this data in EMRDataT

			//TES 12/7/2006 - PLID 23766 - Set the EmrDataGroupID to an arbitrary new number, since we are breaking the tie
			// between this item and any previous versions of it.
			// (z.manning, 05/23/2008) - PLID 16443 - Added Formula
			// (z.manning 2009-01-15 17:08) - PLID 32724 - Added InputMask
			// (c.haag 2010-02-24 16:22) - PLID 21301 - AutoAlphabetizeDropdown
			// (z.manning 2010-02-25 14:49) - PLID 37324 - Added list sub type
			// (j.gruber 2010-04-27 09:43) - PLID 38336 - BOLD Codes
			// (z.manning 2010-08-11 17:20) - PLID 40074 - AutoNumber fields
			//TES 3/15/2011 - PLID 42757 - Added GlassesOrderDataType, GlassesOrderDataID
			// (z.manning 2011-03-21 10:11) - PLID 23662 - Added autofill type
			// (z.manning 2011-05-26 15:34) - PLID 43865 - DataFlags
			// (z.manning 2011-09-19 13:51) - PLID 41954 - Dropdown separators
			// (z.manning 2011-11-07 10:59) - PLID 46309 - SpawnedItemsSeparator
			// (r.gonet 08/03/2012) - PLID 51948 - WoundCareDataType
			// (j.gruber 2014-07-18 14:19) - PLID 62624 - Keyword Saving
			// (j.gruber 2014-12-05 16:47) - PLID 64289 - search queue - UseNameForKeyword Saving
			ExecuteSql("INSERT INTO EMRDataT (ID, EMRInfoID, Data, Inactive, LongForm, SortOrder, ListType, IsLabel, IsGrouped, EmrDataGroupID, Formula, DecimalPlaces, InputMask, AutoAlphabetizeDropdown, ListSubType, BOLDCode, AutoNumberType, AutoNumberPrefix, GlassesOrderDataType, GlassesOrderDataID, AutofillType, DataFlags, DropdownSeparator, DropdownSeparatorFinal, SpawnedItemsSeparator, WoundCareDataType, UseKeyword, KeywordOverride, UseNameForKeyword) "
				"SELECT %li, %li, Data, Inactive, LongForm, SortOrder, ListType, IsLabel, IsGrouped, %li, Formula, DecimalPlaces, InputMask, AutoAlphabetizeDropdown, ListSubType, BOLDCode, AutoNumberType, AutoNumberPrefix, GlassesOrderDataType, GlassesOrderDataID, AutofillType, DataFlags, DropdownSeparator, DropdownSeparatorFinal, SpawnedItemsSeparator, WoundCareDataType, UseKeyword, KeywordOverride, UseNameForKeyword "
				"FROM EMRDataT WHERE ID = %li", nNewDataID, nNewInfoID, nEMRDataGroupID, nOldDataID);
			// (c.haag 2011-03-15) - PLID 42821 - We cannot copy System Info items, but I'm leaving this code intact in case
			// we ever introduce common list buttons in non-system tables.
			// (j.jones 2012-09-21 13:49) - PLID 52316 - restored the mapping
			mapOldToNewEmrDataID.SetAt(nOldDataID, nNewDataID); // (c.haag 2011-03-15) - PLID 42821
			// (j.gruber 2013-10-02 14:00) - PLID 58676 - map data group IDs also
			mapOldToNewEmrDataGroupID.SetAt(nOldDataGroupID, nEMRDataGroupID);

			//2a)  We also need to copy anything in EMRDefaultsT.  This record will only exist for some data
			//	elements, so we only copy some of the time.
			// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
			if(ReturnsRecordsParam("SELECT * FROM EMRInfoDefaultsT WHERE EMRInfoID = {INT} AND EMRDataID = {INT}", nInfoID, nOldDataID)) {
				ExecuteSql("INSERT INTO EMRInfoDefaultsT (EMRInfoID, EMRDataID) values (%li, %li)", nNewInfoID, nNewDataID);
			}

			//2b)  We also need to copy any EMR actions that are specific to this data.  SourceType = 4 means
			//	the actions are for this specific EMRData object.
			//  (z.manning, 3/16/2006, PLID 19747) - There's no reason to copy deleted action items.
			{
				//DRT 1/18/2007 - PLID 24181 - We additionally need to copy the charge-specific data, so we cannot batch
				//	copy any longer.
				//We will batch the SQL for speed.
				CString strActionSql;

				// (c.haag 2008-08-22 15:19) - PLID 30221 - Need to store the new action ID
				strActionSql += "DECLARE @ActionID INT;\r\n";

				//DRT 8/30/2007 - PLID 27259 - Parameterized.
				_RecordsetPtr prsActions = CreateParamRecordset("SELECT ID, DestType FROM EMRActionsT WHERE SourceID = {INT} AND SourceType = 4 AND Deleted = 0", nOldDataID);
				while(!prsActions->eof) {
					long nActionID = AdoFldLong(prsActions, "ID");
					EmrActionObject eaoType = (EmrActionObject)AdoFldLong(prsActions, "DestType");

					//TES 1/30/2007 - PLID 24474 - Don't forget SpawnAsChild!
					strActionSql += FormatString("INSERT INTO EMRActionsT (SourceType, SourceID, DestType, DestID, SortOrder, Popup, SpawnAsChild) "
						"SELECT SourceType, %li, DestType, DestID, SortOrder, Popup, SpawnAsChild "
						"FROM EMRActionsT WHERE ID = %li \r\n"
						"SET @ActionID = SCOPE_IDENTITY();\r\n", nNewDataID, nActionID);

					//Only if charge type
					if(eaoType == eaoCpt) {
						strActionSql += FormatString("INSERT INTO EMRActionChargeDataT (ActionID, Prompt, DefaultQuantity, Modifier1Number, "
							"Modifier2Number, Modifier3Number, Modifier4Number) SELECT @ActionID, Prompt, DefaultQuantity, Modifier1Number, "
							"Modifier2Number, Modifier3Number, Modifier4Number FROM EMRActionChargeDataT WHERE ActionID = %li \r\n", nActionID);
					}

					// (c.haag 2008-08-22 15:12) - PLID 30221 - Copy Todo actions
					if (eaoTodo == eaoType) {
						strActionSql += FormatString("INSERT INTO EMRActionsTodoDataT (ActionID, "
							"RemindType, RemindInterval, DeadlineType, DeadlineInterval, Notes, Priority, Task, CategoryID) SELECT @ActionID, "
							"RemindType, RemindInterval, DeadlineType, DeadlineInterval, Notes, Priority, Task, CategoryID FROM EMRActionsTodoDataT WHERE ActionID = %li \r\n", nActionID);

						strActionSql += FormatString("INSERT INTO EMRActionsTodoAssignToT (ActionID, AssignTo) SELECT @ActionID, "
							"AssignTo FROM EMRActionsTodoAssignToT WHERE ActionID = %li \r\n", nActionID);
					}

					// (b.savon 2014-07-22 09:35) - PLID 62707 - Handle saving the new Diagnosis DestType action to EmrActionsT and EmrActionsDiagnosisDataT
					if (eaoType == eaoDiagnosis){
						strActionSql += FormatString(
							R"(
							INSERT INTO EmrActionDiagnosisDataT (EmrActionID, DiagCodeID_ICD9, DiagCodeID_ICD10)
							SELECT	@ActionID, DiagCodeID_ICD9, DiagCodeID_ICD10
							FROM	EmrActionDiagnosisDataT
							WHERE	EmrActionID = %li
							)",
							nActionID
						);
					}

					// (c.haag 2008-08-22 15:37) - PLID 30724 - Copy EMR problem actions
					// (c.haag 2014-07-22) - PLID 62789 - Added SNOMEDCodeID
					// (s.tullis 2015-03-05 15:34) - PLID 64724 -Added Do not show on ccda
					// (r.gonet 2015-03-10 14:48) - PLID 65013 - Added DoNotShowOnProblemPrompt.
					strActionSql += FormatString("INSERT INTO EMRProblemActionsT (EmrActionID, "
						"DefaultDescription, DefaultStatus, SpawnToSourceItem, SNOMEDCodeID, DoNotShowOnCCDA, DoNotShowOnProblemPrompt) SELECT @ActionID, "
						"DefaultDescription, DefaultStatus, SpawnToSourceItem, SNOMEDCodeID, DoNotShowOnCCDA, DoNotShowOnProblemPrompt FROM EMRProblemActionsT "
						"WHERE Inactive = 0 AND EmrActionID = %li \r\n", nActionID);

					prsActions->MoveNext();
				}

				//Execute all statements
				if(!strActionSql.IsEmpty())
					ExecuteSqlStd(strActionSql);
			}

			//2c)  If this is a table, we need to copy data from EMRTableDropdownInfoT
			long nNewDDID = NewNumber("EMRTableDropdownInfoT", "ID");
			//DRT 8/30/2007 - PLID 27259 - Parameterized.
			_RecordsetPtr prsDDInfo = CreateParamRecordset("SELECT ID FROM EMRTableDropdownInfoT WHERE EMRDataID = {INT}", nOldDataID);
			while(!prsDDInfo->eof) {
				long nOldDDID = AdoFldLong(prsDDInfo, "ID");

				//TES 12/7/2006 - PLID 23766 - Set the DropdownGroupID to an arbitrary new number, since we are breaking the tie
				// between this item and any previous versions of it.
				// (a.walling 2007-08-29 12:05) - PLID 27223 - No longer arbirtary; links to EMRTableDropdownGroupsT
				CString strDropdownBatchSql = BeginSqlBatch();
				AddStatementToSqlBatch(strDropdownBatchSql, 
					"DECLARE @nNewTableDropdownGroupID INT; "
					"SET @nNewTableDropdownGroupID = (SELECT COALESCE(Max(ID),0)+1 FROM EMRTableDropdownGroupsT); "
					"INSERT INTO EMRTableDropdownGroupsT(ID) VALUES(@nNewTableDropdownGroupID); ");
				//TES 3/15/2011 - PLID 42757 - Added GlassesOrderDataID
				// (j.gruber 2014-07-22 13:11) - PLID 62627 - keyword columns
				AddStatementToSqlBatch(strDropdownBatchSql, 
					"INSERT INTO EMRTableDropdownInfoT (ID, EMRDataID, Data, SortOrder, Inactive, GlassesOrderDataID, DropdownGroupID, UseKeyword, KeywordOverride) "
					"SELECT %li, %li, Data, SortOrder, Inactive, GlassesOrderDataID, @nNewTableDropdownGroupID, UseKeyword, KeywordOverride "
					"FROM EMRTableDropdownInfoT WHERE ID = %li", nNewDDID, nNewDataID, nOldDDID);
				// (z.manning 2011-09-30 12:50) - PLID 45729 - Handle dropdown stamp filter data
				AddStatementToSqlBatch(strDropdownBatchSql,
					"INSERT INTO EmrTableDropdownStampFilterT (EmrTableDropdownInfoID, StampID) \r\n"
					"SELECT %li, StampID \r\n"
					"FROM EmrTableDropdownStampFilterT \r\n"
					"WHERE EmrTableDropdownInfoID = %li "
					, nNewDDID, nOldDDID);
				// (j.jones 2012-11-26 15:11) - PLID 53144 - added EMRTableDropdownStampDefaultsT
				AddStatementToSqlBatch(strDropdownBatchSql,
					"INSERT INTO EMRTableDropdownStampDefaultsT (EmrTableDropdownInfoID, StampID) \r\n"
					"SELECT %li, StampID \r\n"
					"FROM EMRTableDropdownStampDefaultsT \r\n"
					"WHERE EmrTableDropdownInfoID = %li "
					, nNewDDID, nOldDDID);

				// (j.gruber 2013-10-02 13:24) - PLID 58675 - DropDownCodes
				AddStatementToSqlBatch(strDropdownBatchSql, 
					" INSERT INTO EmrTableDropdownGroupCodesT (EmrTableDropdownGroupID, CodeID) \r\n"
					" SELECT @nNewTableDropdownGroupID, CodeID \r\n"
					" FROM EmrTableDropdownGroupCodesT \r\n"
					" INNER JOIN EMRTableDropDownInfoT ON EMRTableDropDownGroupCodesT.EmrTableDropdownGroupID = EMRTableDropDownInfoT.DropDownGroupID \r\n"
					" WHERE EMRTableDropDownInfoT.ID = %li",nOldDDID);

				ExecuteSqlBatch(strDropdownBatchSql);

				// (z.manning 2009-02-26 10:26) - PLID 15971 - Dropdown items now have actions so we need to copy them too.
				CString strActionSql;
				strActionSql += "DECLARE @ActionID INT;\r\n";
				_RecordsetPtr prsActions = CreateParamRecordset(
					"SELECT ID, DestType FROM EMRActionsT "
					"WHERE SourceID = {INT} AND SourceType = {INT} AND Deleted = 0"
					, nOldDDID, eaoEmrTableDropDownItem);
				while(!prsActions->eof) {
					long nActionID = AdoFldLong(prsActions, "ID");
					EmrActionObject eaoType = (EmrActionObject)AdoFldLong(prsActions, "DestType");

					strActionSql += FormatString("INSERT INTO EMRActionsT (SourceType, SourceID, DestType, DestID, SortOrder, Popup, SpawnAsChild) "
						"SELECT SourceType, %li, DestType, DestID, SortOrder, Popup, SpawnAsChild "
						"FROM EMRActionsT WHERE ID = %li \r\n"
						"SET @ActionID = SCOPE_IDENTITY();\r\n", nNewDDID, nActionID);

					//Only if charge type
					if(eaoType == eaoCpt) {
						strActionSql += FormatString("INSERT INTO EMRActionChargeDataT (ActionID, Prompt, DefaultQuantity, Modifier1Number, "
							"Modifier2Number, Modifier3Number, Modifier4Number) SELECT @ActionID, Prompt, DefaultQuantity, Modifier1Number, "
							"Modifier2Number, Modifier3Number, Modifier4Number FROM EMRActionChargeDataT WHERE ActionID = %li \r\n", nActionID);
					}

					// (c.haag 2008-08-22 15:12) - PLID 30221 - Copy Todo actions
					if (eaoTodo == eaoType) {
						strActionSql += FormatString("INSERT INTO EMRActionsTodoDataT (ActionID, "
							"RemindType, RemindInterval, DeadlineType, DeadlineInterval, Notes, Priority, Task, CategoryID) SELECT @ActionID, "
							"RemindType, RemindInterval, DeadlineType, DeadlineInterval, Notes, Priority, Task, CategoryID FROM EMRActionsTodoDataT WHERE ActionID = %li \r\n", nActionID);

						strActionSql += FormatString("INSERT INTO EMRActionsTodoAssignToT (ActionID, AssignTo) SELECT @ActionID, "
							"AssignTo FROM EMRActionsTodoAssignToT WHERE ActionID = %li \r\n", nActionID);
					}

					// (b.savon 2014-07-22 09:35) - PLID 62707 - Handle saving the new Diagnosis DestType action to EmrActionsT and EmrActionsDiagnosisDataT
					if (eaoType == eaoDiagnosis){
						strActionSql += FormatString(
							R"(
							INSERT INTO EmrActionDiagnosisDataT (EmrActionID, DiagCodeID_ICD9, DiagCodeID_ICD10)
							SELECT	@ActionID, DiagCodeID_ICD9, DiagCodeID_ICD10
							FROM	EmrActionDiagnosisDataT
							WHERE	EmrActionID = %li
							)",
							nActionID
							);
					}

					// (c.haag 2008-08-22 15:37) - PLID 30724 - Copy EMR problem actions
					// (c.haag 2014-07-22) - PLID 62789 - Added SNOMEDCodeID
					// (s.tullis 2015-03-05 15:35) - PLID 64724 - Added Do not show on CCDA 
					// (r.gonet 2015-03-10 14:48) - PLID 65013 - Added DoNotShowOnProblemPrompt.
					strActionSql += FormatString("INSERT INTO EMRProblemActionsT (EmrActionID, "
						"DefaultDescription, DefaultStatus, SpawnToSourceItem, SNOMEDCodeID, DoNotShowOnCCDA, DoNotShowOnProblemPrompt) SELECT @ActionID, "
						"DefaultDescription, DefaultStatus, SpawnToSourceItem, SNOMEDCodeID, DoNotShowOnCCDA, DoNotShowOnProblemPrompt FROM EMRProblemActionsT "
						"WHERE Inactive = 0 AND EmrActionID = %li \r\n", nActionID);

					prsActions->MoveNext();
				}

				//Execute all statements
				if(!strActionSql.IsEmpty()) {
					ExecuteSqlStd(strActionSql);
				}

				//increment
				nNewDDID++;
				prsDDInfo->MoveNext();
			}

			//2d) TES 1/30/2007 - PLID 24377 - We also need to copy any links that this data item was a part of.
			//			Note that here, we make a whole new link, as opposed to the PrepareToReassign() code, because
			//			in that code it treats the old and new copy as the "same" item, whereas in this new code the old
			//			and new copies are actually different items.
			//DRT 8/30/2007 - PLID 27259 - Parameterized.
			_RecordsetPtr rsLinks = CreateParamRecordset("SELECT EmrLinkID FROM EmrItemLinkedDataT WHERE EmrDataID = {INT}", nOldDataID);
			while(!rsLinks->eof) {
				long nNewLinkID;
				if(!mapLinkIDs.Lookup(AdoFldLong(rsLinks, "EmrLinkID"), nNewLinkID)) {
					//This is impossible!  We added all the links that were attached to any of our EmrDataT records to the
					// map up above in 1b.)!
					ASSERT(FALSE);
					AfxThrowNxException("Failed to map EMR Link!");
				}
				ExecuteSql("INSERT INTO EmrItemLinkedDataT (EmrLinkID, EmrDataID) VALUES (%li, %li)", nNewLinkID, nNewDataID);
				rsLinks->MoveNext();
			}

			// (j.gruber 2013-10-02 13:28) - PLID 58674 - EMRDataGroupCodesT
			ExecuteSql("INSERT INTO EmrDataGroupCodesT (EMRDataGroupID, CodeID) \r\n"
				" SELECT %li, CodeID FROM EmrDataGroupCodesT \r\n"
				"INNER JOIN EMRDataT ON EMRDataGroupCodesT.EMRDataGroupID = EMRDataT.EMRDataGroupID \r\n"
				"WHERE EMRDataT.ID = %li",
			nEMRDataGroupID, nOldDataID);
			
			nNewDataID++;	//increment for loop
			nEMRDataGroupID++;
			prsData->MoveNext();
		}

		//3)  Copy any EMR actions for this whole detail, we can batch copy these.  SourceType = 3 means
		//	the actions are for the entire EMRInfo object.
		{
			//DRT 1/18/2007 - PLID 24181 - We need to copy charge-specific action data, thus cannot batch copy any longer.
			//	Put all into a string for speed.
			CString strActionSql;

			// (c.haag 2008-08-22 15:19) - PLID 30221 - Need to store the new action ID
			strActionSql += "DECLARE @ActionID INT;\r\n";

			//DRT 8/30/2007 - PLID 27259 - Parameterized.
			_RecordsetPtr prsActions = CreateParamRecordset("SELECT ID, DestType FROM EMRActionsT WHERE SourceID = {INT} AND SourceType = 3", nInfoID);
			while(!prsActions->eof) {
				long nActionID = AdoFldLong(prsActions, "ID");
				EmrActionObject eaoType = (EmrActionObject)AdoFldLong(prsActions, "DestType");

				//TES 1/30/2007 - PLID 24474 - Don't forget SpawnAsChild!
				strActionSql += FormatString("INSERT INTO EMRActionsT (SourceType, SourceID, DestType, DestID, SortOrder, Popup, SpawnAsChild) "
					"SELECT SourceType, %li, DestType, DestID, SortOrder, Popup, SpawnAsChild "
					"FROM EMRActionsT WHERE ID = %li \r\n"
					"SET @ActionID = SCOPE_IDENTITY();\r\n", nNewInfoID, nActionID);
				//Only if charge type
				if(eaoType == eaoCpt) {
					strActionSql += FormatString("INSERT INTO EMRActionChargeDataT (ActionID, Prompt, DefaultQuantity, Modifier1Number, "
						"Modifier2Number, Modifier3Number, Modifier4Number) SELECT @ActionID, Prompt, DefaultQuantity, Modifier1Number, "
						"Modifier2Number, Modifier3Number, Modifier4Number FROM EMRActionChargeDataT WHERE ActionID = %li", nActionID);
				}

				// (c.haag 2008-08-22 15:12) - PLID 30221 - Copy Todo actions
				if (eaoTodo == eaoType) {
					strActionSql += FormatString("INSERT INTO EMRActionsTodoDataT (ActionID, "
						"RemindType, RemindInterval, DeadlineType, DeadlineInterval, Notes, Priority, Task, CategoryID) SELECT @ActionID, "
						"RemindType, RemindInterval, DeadlineType, DeadlineInterval, Notes, Priority, Task, CategoryID FROM EMRActionsTodoDataT WHERE ActionID = %li \r\n", nActionID);

					strActionSql += FormatString("INSERT INTO EMRActionsTodoAssignToT (ActionID, AssignTo) SELECT @ActionID, "
						"AssignTo FROM EMRActionsTodoAssignToT WHERE ActionID = %li \r\n", nActionID);
				}

				// (b.savon 2014-07-22 09:35) - PLID 62707 - Handle saving the new Diagnosis DestType action to EmrActionsT and EmrActionsDiagnosisDataT
				if (eaoType == eaoDiagnosis){
					strActionSql += FormatString(
						R"(
							INSERT INTO EmrActionDiagnosisDataT (EmrActionID, DiagCodeID_ICD9, DiagCodeID_ICD10)
							SELECT	@ActionID, DiagCodeID_ICD9, DiagCodeID_ICD10
							FROM	EmrActionDiagnosisDataT
							WHERE	EmrActionID = %li
							)",
							nActionID
							);
				}

				// (c.haag 2008-08-22 15:37) - PLID 30724 - Copy EMR problem actions
				// (c.haag 2014-07-22) - PLID 62789 - Added SNOMEDCodeID
				// (s.tullis 2015-03-05 15:35) - PLID 64724 - Added Do not show on CCDA 
				// (r.gonet 2015-03-10 14:48) - PLID 65013 - Added DoNotShowOnProblemPrompt.
				strActionSql += FormatString("INSERT INTO EMRProblemActionsT (EmrActionID, "
					"DefaultDescription, DefaultStatus, SpawnToSourceItem, SNOMEDCodeID, DoNotShowOnCCDA, DoNotShowOnProblemPrompt) SELECT @ActionID, "
					"DefaultDescription, DefaultStatus, SpawnToSourceItem, SNOMEDCodeID, DoNotShowOnCCDA, DoNotShowOnProblemPrompt FROM EMRProblemActionsT "
					"WHERE Inactive = 0 AND EmrActionID = %li \r\n", nActionID);


				prsActions->MoveNext();
			}

			//execute 'em
			if(!strActionSql.IsEmpty())
				ExecuteSqlStd(strActionSql);
		}

		//4)  Copy EMRInfoCategoryT
		ExecuteSql("INSERT INTO EMRInfoCategoryT (EMRInfoID, EMRCategoryID) SELECT %li, EMRCategoryID FROM EMRInfoCategoryT WHERE EMRInfoID = %li", 
			nNewInfoID, nInfoID);

		

		// (z.manning 2009-12-16 15:18) - PLID 31152 - Copy hot spots!
		// (z.manning 2009-12-23 10:58) - PLID 31152 - Also handle hot spot actions
		//TES 2/11/2010 - PLID 37223 - Added Anatomic Location info for hotspots
		// (z.manning 2012-01-26 09:24) - PLID 47802 - Copy ImageHotSpotID field
		// (b.savon 2014-07-16 10:01) - PLID 62707 - Handle saving the new Diagnosis DestType action to EmrActionsT and EmrActionsDiagnosisDataT
		// (c.haag 2014-07-22) - PLID 62789 - Added SNOMEDCodeID
		// (s.tullis 2015-03-05 15:35) - PLID 64724 - Added Do not show on CCDA 
		// (r.gonet 2015-03-10 14:48) - PLID 65013 - Added DoNotShowOnProblemPrompt.
		ExecuteParamSql(
			"DECLARE @NextHotSpotID int, @NextHotSpotGroupID int, @OldHotSpotID int \r\n"
			"	, @OldActionID int, @NewActionID int \r\n"
			"DECLARE rsHotSpots CURSOR LOCAL FORWARD_ONLY READ_ONLY FOR \r\n"
			"	SELECT ID FROM EmrImageHotSpotsT WHERE EmrInfoID = {INT} \r\n"
			"OPEN rsHotSpots \r\n"
			"FETCH NEXT FROM rsHotSpots INTO @OldHotSpotID \r\n"
			"WHILE @@FETCH_STATUS = 0 BEGIN \r\n"
			"	SET @NextHotSpotID = (SELECT COALESCE(Max(ID), 0) + 1 FROM EmrImageHotSpotsT) \r\n"
			"	SET @NextHotSpotGroupID = (SELECT COALESCE(Max(EmrSpotGroupID), 0) + 1 FROM EmrImageHotSpotsT) \r\n"
			"	INSERT INTO EmrImageHotSpotsT (ID, EmrInfoID, Data, AnatomicLocationID, AnatomicQualifierID, AnatomySide, EmrSpotGroupID, ImageHotSpotID) \r\n"
			"	SELECT @NextHotSpotID, {INT}, Data, AnatomicLocationID, AnatomicQualifierID, AnatomySide, @NextHotSpotGroupID, ImageHotSpotID \r\n"
			"	FROM EmrImageHotSpotsT \r\n"
			"	WHERE ID = @OldHotSpotID \r\n"
			"	------- Begin Actions \r\n"
			"	DECLARE rsHotSpotActions CURSOR LOCAL FORWARD_ONLY READ_ONLY FOR \r\n"
			"		SELECT ID FROM EmrActionsT WHERE Deleted = 0 AND SourceID = @OldHotSpotID AND SourceType = {INT} \r\n"
			"	OPEN rsHotSpotActions \r\n"
			"	FETCH NEXT FROM rsHotSpotActions INTO @OldActionID \r\n"
			"	WHILE @@FETCH_STATUS = 0 BEGIN \r\n"
			"		INSERT INTO EmrActionsT (SourceType, SourceID, DestType, DestID, SortOrder, Popup, SpawnAsChild) \r\n"
			"		SELECT SourceType, @NextHotSpotID, DestType, DestID, SortOrder, Popup, SpawnAsChild \r\n"
			"		FROM EmrActionsT \r\n"
			"		WHERE ID = @OldActionID \r\n"
			"		SET @NewActionID = SCOPE_IDENTITY() \r\n"
			"		INSERT INTO EMRActionChargeDataT (ActionID, Prompt, DefaultQuantity, Modifier1Number, Modifier2Number, Modifier3Number, Modifier4Number) \r\n"
			"		SELECT @NewActionID, Prompt, DefaultQuantity, Modifier1Number, Modifier2Number, Modifier3Number, Modifier4Number \r\n"
			"		FROM EMRActionChargeDataT \r\n"
			"		WHERE ActionID = @OldActionID \r\n"
			"		INSERT INTO EMRActionsTodoDataT (ActionID, RemindType, RemindInterval, DeadlineType, DeadlineInterval, Notes, Priority, Task, CategoryID) \r\n"
			"		SELECT @NewActionID, RemindType, RemindInterval, DeadlineType, DeadlineInterval, Notes, Priority, Task, CategoryID \r\n"
			"		FROM EMRActionsTodoDataT \r\n"
			"		WHERE ActionID = @OldActionID \r\n"
			"		INSERT INTO EMRActionsTodoAssignToT (ActionID, AssignTo) \r\n"
			"		SELECT @NewActionID, AssignTo \r\n"
			"		FROM EMRActionsTodoAssignToT \r\n"
			"		WHERE ActionID = @OldActionID \r\n"
			"		INSERT INTO EmrActionDiagnosisDataT (EmrActionID, DiagCodeID_ICD9, DiagCodeID_ICD10) \r\n"
			"		SELECT @NewActionID, DiagCodeID_ICD9, DiagCodeID_ICD10 \r\n"
			"		FROM	EmrActionDiagnosisDataT \r\n"
			"		WHERE	EmrActionID = @OldActionID \r\n"
			"		INSERT INTO EMRProblemActionsT (EmrActionID, DefaultDescription, DefaultStatus, SpawnToSourceItem, SNOMEDCodeID, DoNotShowOnCCDA, DoNotShowOnProblemPrompt) \r\n"
			"		SELECT @NewActionID, DefaultDescription, DefaultStatus, SpawnToSourceItem, SNOMEDCodeID, DoNotShowOnCCDA, DoNotShowOnProblemPrompt  \r\n"
			"		FROM EMRProblemActionsT \r\n"
			"		WHERE Inactive = 0 AND EmrActionID = @OldActionID \r\n"
			"		FETCH NEXT FROM rsHotSpotActions INTO @OldActionID \r\n"
			"	END \r\n"
			"	CLOSE rsHotSpotActions \r\n"
			"	DEALLOCATE rsHotSpotActions \r\n"
			"	------- End Actions \r\n"
			"	FETCH NEXT FROM rsHotSpots INTO @OldHotSpotID \r\n"
			"END \r\n"
			"CLOSE rsHotSpots \r\n"
			"DEALLOCATE rsHotSpots \r\n"
			"\r\n"
			, nInfoID, nNewInfoID, eaoEmrImageHotSpot);

		// (c.haag 2010-07-10 11:19) - PLID 39467 - Ensure we did not create multiple values for a dropdown with the same sort order
		ExecuteSql(	"IF EXISTS(SELECT EMRDataID, SortOrder FROM EMRTableDropdownInfoT WHERE EMRDataID IN (SELECT ID FROM EMRDataT WHERE EMRInfoID = %d) GROUP BY EMRDataID, SortOrder HAVING Count(SortOrder) > 1) \r\n"
			"BEGIN RAISERROR('Duplicate SortOrder values detected in EMRTableDropdownInfoT!', 16, 1) RETURN END \r\n"
			,nNewInfoID);

		// (j.jones 2012-09-21 13:50) - PLID 52316 - apply ParentLabelIDs
		{
			CString strSqlBatch;
			CNxParamSqlArray aryParams;
			//no need to propagate parent labels on non-list rows or on inactive rows
			_RecordsetPtr rsParentLabels = CreateParamRecordset("SELECT EMRDataT.ID, EMRDataT.ParentLabelID "
				"FROM EMRDataT "
				"WHERE EMRDataT.ListType = 1 AND EMRDataT.EMRInfoID = {INT} "
				"AND EMRDataT.Inactive = 0 AND EMRDataT.ParentLabelID Is Not Null", nInfoID);
			while(!rsParentLabels->eof) {
				long nID = VarLong(rsParentLabels->Fields->Item["ID"]->Value);
				long nParentLabelID = VarLong(rsParentLabels->Fields->Item["ParentLabelID"]->Value);

				//get the mapped value from old to new
				long nNewID = -1;
				long nNewParentLabelID = -1;

				if (!mapOldToNewEmrDataID.Lookup(nID, nNewID))
				{
					ThrowNxException("Could not properly map old item ID %d for parent labels!", nID);
				}

				if (!mapOldToNewEmrDataID.Lookup(nParentLabelID, nNewParentLabelID))
				{
					ThrowNxException("Could not properly map old parent label ID %d!", nParentLabelID);
				}

				AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE EMRDataT SET ParentLabelID = {INT} WHERE ID = {INT}", nNewParentLabelID, nNewID);

				rsParentLabels->MoveNext();
			}
			rsParentLabels->Close();

			if(!strSqlBatch.IsEmpty()) {
				ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);
			}
		}


		// (j.gruber 2013-10-02 13:33) - PLID 58676 - Table Cell Codes
		{
			CString strSqlBatch;
			CNxParamSqlArray aryParams;			
			//if we grab the information from all rows, then we can lookup the column IDs as we go
			_RecordsetPtr rsCodeCells = CreateParamRecordset(" SELECT EMRDataGroupID_X, EMRDataGroupID_Y, CodeID, EMRDataT_Y.ID as Y_DataID, EMRDataT_X.ID as X_DataID  \r\n"
				" FROM EmrTableDataGroupCodesT \r\n"
				" INNER JOIN EMRDataGroupsT EMRDataGroupT_Y ON EmrTableDataGroupCodesT.EMRDataGroupID_Y = EMRDataGroupT_Y.ID \r\n"
				" INNER JOIN EMRDataGroupsT EMRDataGroupT_X ON EmrTableDataGroupCodesT.EMRDataGroupID_X = EMRDataGroupT_X.ID \r\n"
				" INNER JOIN EMRDataT EMRDataT_Y ON EMRDataGroupT_Y.ID = EMRDataT_Y.EMRDataGroupID \r\n"
				" INNER JOIN EMRDataT EMRDataT_X ON EMRDataGroupT_X.ID = EMRDataT_X.EMRDataGroupID \r\n"			
				" WHERE  EMRDataT_Y.EMRInfoID = {INT} AND EMRDataT_X.EMRInfoID = {INT} \r\n",
				nInfoID, nInfoID);
			while(!rsCodeCells->eof) {
				long nDataYGroupID = VarLong(rsCodeCells->Fields->Item["EMRDataGroupID_Y"]->Value);
				long nDataXGroupID = VarLong(rsCodeCells->Fields->Item["EMRDataGroupID_X"]->Value);				
				long nCodeID = VarLong(rsCodeCells->Fields->Item["CodeID"]->Value);

				//get the mapped value from old to new
				long nNewYGroupID = -1;
				long nNewXGroupID = -1;				

				if (!mapOldToNewEmrDataGroupID.Lookup(nDataYGroupID, nNewYGroupID))
				{
					ThrowNxException("Could not properly map old item row Group ID %d for table cell codes!", nDataYGroupID);
				}

				if (!mapOldToNewEmrDataGroupID.Lookup(nDataXGroupID, nNewXGroupID))
				{
					ThrowNxException("Could not properly map old item column Group ID %d for table cell codes!", nDataXGroupID);
				}

				AddParamStatementToSqlBatch(strSqlBatch, aryParams, 
					"INSERT INTO EmrTableDataGroupCodesT (EmrDataGroupID_X, EmrDataGroupID_Y, CodeID) \r\n"
					" VALUES ({INT}, {INT}, {INT})", nNewXGroupID, nNewYGroupID, nCodeID);

				rsCodeCells->MoveNext();
			}
			rsCodeCells->Close();

			if(!strSqlBatch.IsEmpty()) {
				ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);
			}
		}

		// (c.haag 2011-03-15) - PLID 42821 - We cannot copy System Info items, but I'm leaving this code intact in case
		// we ever introduce common list buttons in non-system tables.
		// (c.haag 2011-04-11) - PLID 43234 - And don't forget GroupOnPreviewPane if we did
		/*
		//5) - EmrInfoCommonListT
		{
			CString strprsCommonLists =
				"SET NOCOUNT ON \r\n"
				"DECLARE @nError INT \r\n"
				// (c.haag 2011-03-15) - PLID 42821 - A) - Create a table variable that maps old EmrInfoCommonListT.ID's to new EmrInfoCommonListT.ID's.
				"DECLARE @tNewCommonListT TABLE (RowNum INT IDENTITY NOT NULL PRIMARY KEY, OldID INT NOT NULL, NewID INT) \r\n"
				"INSERT INTO @tNewCommonListT (OldID) \r\n"
				" SELECT ID FROM EmrInfoCommonListT WHERE EMRInfoID = {INT} \r\n"
				"SET @nError = @@ERROR IF @nError <> 0 BEGIN RAISERROR('@@ERROR was %li!', 16, 1, @nError) ROLLBACK TRAN RETURN END\r\n"
				"UPDATE @tNewCommonListT SET NewID = RowNum + COALESCE((SELECT MAX(ID) FROM EmrInfoCommonListT), 0) \r\n"
				"SET @nError = @@ERROR IF @nError <> 0 BEGIN RAISERROR('@@ERROR was %li!', 16, 1, @nError) ROLLBACK TRAN RETURN END\r\n"

				// (c.haag 2011-03-15) - PLID 42821 - B) - Copy the contents of EmrInfoCommonListT for the old EmrInfoID to the new EmrInfoID
				"SET IDENTITY_INSERT EmrInfoCommonListT ON \r\n"
				"SET @nError = @@ERROR IF @nError <> 0 BEGIN RAISERROR('@@ERROR was %li!', 16, 1, @nError) ROLLBACK TRAN RETURN END\r\n"
				"INSERT INTO EmrInfoCommonListT (ID, EmrInfoID, Name, Color, Inactive, OrderID, GroupOnPreviewPane) \r\n"
				" SELECT NewID, {INT}, Name, Color, Inactive, OrderID, GroupOnPreviewPane \r\n"
				" FROM EmrInfoCommonListT INNER JOIN @tNewCommonListT ON ID = OldID \r\n"
				"SET @nError = @@ERROR IF @nError <> 0 BEGIN RAISERROR('@@ERROR was %li!', 16, 1, @nError) ROLLBACK TRAN RETURN END\r\n"
				"SET IDENTITY_INSERT EmrInfoCommonListT OFF \r\n"
				"SET @nError = @@ERROR IF @nError <> 0 BEGIN RAISERROR('@@ERROR was %li!', 16, 1, @nError) ROLLBACK TRAN RETURN END\r\n"

				// (c.haag 2011-03-15) - PLID 42821 - C) - Return the mapped common list ID's
				"SET NOCOUNT OFF \r\n"
				"SELECT OldID, NewID FROM @tNewCommonListT \r\n"
				;

			// This recordset returns the table variable that maps the old list ID's to the new list ID's
			_RecordsetPtr prsCommonListMap = CreateParamRecordset(strprsCommonLists,
				nInfoID, nNewInfoID);
			while (!prsCommonListMap->eof)
			{
				long nOldListID = AdoFldLong(prsCommonListMap->Fields, "OldID");
				long nNewListID = AdoFldLong(prsCommonListMap->Fields, "NewID");

				// Now copy the list details. First we query all of the individual items for this list, then one by
				// one we add a new item with the proper corresponding values.
				_RecordsetPtr prsCommonListDetails = CreateParamRecordset(
					"SELECT ID, EmrDataID FROM EmrInfoCommonListItemsT WHERE ListID = {INT}"
					,nOldListID);
				while (!prsCommonListDetails->eof)
				{
					// Figure out the ID's we're working with
					long nOldListItemID = AdoFldLong(prsCommonListDetails->Fields, "ID");
					long nOldEmrDataID = AdoFldLong(prsCommonListDetails->Fields, "EmrDataID");
					long nNewEmrDataID = -1;

					if (!mapOldToNewEmrDataID.Lookup(nOldEmrDataID, nNewEmrDataID))
					{
						ThrowNxException("Could not properly map old common list item ID %d!", nOldListItemID);
					}

					// Now add the new item to data
					ExecuteParamSql(
						"SET NOCOUNT ON \r\n"
						"DECLARE @nError INT \r\n"
						"INSERT INTO EmrInfoCommonListItemsT (ListID, EmrDataID) "
						"VALUES ({INT}, {INT}) \r\n "
						"SET @nError = @@ERROR IF @nError <> 0 BEGIN RAISERROR('@@ERROR was %li!', 16, 1, @nError) ROLLBACK TRAN RETURN END\r\n"
						"SET NOCOUNT OFF \r\n"
						"SELECT CONVERT(INT, SCOPE_IDENTITY()) AS NewID \r\n"
						,nNewListID, nNewEmrDataID
						);

					prsCommonListDetails->MoveNext();
				} // while (!prsCommonListDetails->eof)
				
				prsCommonListMap->MoveNext();
			} // while (!prsCommonListMap->eof)
		}*/

		bSaved = TRUE;

	END_TRANS_CATCH_ALL("EMRInfoCopy")

	if(bSaved) {
		if(pnNewMasterID) {
			*pnNewMasterID = nNewMasterID;
		}
		if(pnNewInfoID) {
			*pnNewInfoID = nNewInfoID;
		}
		if(pstrNewName) {
			*pstrNewName = strNewName;
		}
	}
	return bSaved;
}


/* (c.haag 2006-02-23 17:29) - PLID 17195 - This function may be called after
CopyEmrInfoItem to create relationships between nNewInfoID and the following tables:

DiagCodeToEMRInfoT
ProcedureToEMRInfoT

These relationships are parallel to the relationships existing between them and
nOldInfoID

*/

BOOL CopyEmrInfoItemRelationships(IN long nOldInfoID, IN long nNewInfoID)
{
	BOOL bSaved = FALSE;
	BEGIN_TRANS("CopyEmrInfoItemRelationships")	

		// 1) DiagCodeToEMRInfoT
		ExecuteSql("INSERT INTO DiagCodeToEMRInfoT (DiagCodeID, EMRInfoID, Required) "
			"SELECT DiagCodeID, %d, Required FROM DiagCodeToEMRInfoT "
			"WHERE EMRInfoID = %d", nNewInfoID, nOldInfoID);

		// 2) ProcedureToEMRInfoT
		ExecuteSql("INSERT INTO ProcedureToEMRInfoT (ProcedureID, EMRInfoID, Required) "
			"SELECT ProcedureID, %d, Required FROM ProcedureToEMRInfoT "
			"WHERE EMRInfoID = %d", nNewInfoID, nOldInfoID);

		bSaved = TRUE;

	END_TRANS_CATCH_ALL("CopyEmrInfoItemRelationships")

	return bSaved;
}

// (c.haag 2006-02-24 09:49) - PLID 17195 - This function inactivates an EMR info item
//TES 12/14/2006 - PLID 23792 - It is now EmrInfoMasterT records that are activated/inactivate.
BOOL InactivateEmrInfoMasterItem(IN long nID)
{
	try {
		ExecuteSql("UPDATE EMRInfoMasterT SET Inactive = 1 WHERE ID = %d", nID);

		CString strOldValue, strNewValue;
		CString strName;
		//DRT 8/30/2007 - PLID 27259 - Parameterized.
		_RecordsetPtr rs = CreateParamRecordset("SELECT Name FROM EMRInfoT WHERE ID = (SELECT ActiveEmrInfoID FROM EmrInfoMasterT WHERE ID = {INT})", nID);
		if(!rs->eof) {
			strName = AdoFldString(rs, "Name","");
		}
		rs->Close();
		
		strOldValue.Format("%s (Active)", strName);
		strNewValue.Format("%s (Inactive)", strName);

		long nAuditID = BeginNewAuditEvent();
		AuditEvent(-1, "", nAuditID, aeiEMRItemInactive, nID, strOldValue, strNewValue, aepMedium, aetChanged);

		return TRUE;
	}
	NxCatchAll("Failed to inactivate EMR item");
	return FALSE;
}

// (c.haag 2006-02-27 09:49) - PLID 17195 - This function inactivates an EMR info item
//TES 12/14/2006 - PLID 23792 - It is now EmrInfoMasterT records that are activated/inactivate.
BOOL ActivateEmrInfoMasterItem(IN long nID)
{
	try {
		ExecuteSql("UPDATE EMRInfoMasterT SET Inactive = 0 WHERE ID = %d", nID);

		CString strOldValue, strNewValue;
		CString strName;
		//DRT 8/30/2007 - PLID 27259 - Parameterized.
		_RecordsetPtr rs = CreateParamRecordset("SELECT Name FROM EMRInfoT WHERE ID = (SELECT ActiveEmrInfoID FROM EmrInfoMasterT WHERE ID = {INT})", nID);
		if(!rs->eof) {
			strName = AdoFldString(rs, "Name","");
		}
		rs->Close();
		
		strOldValue.Format("%s (Inactive)", strName);
		strNewValue.Format("%s (Active)", strName);

		long nAuditID = BeginNewAuditEvent();
		AuditEvent(-1, "", nAuditID, aeiEMRItemInactive, nID, strOldValue, strNewValue, aepMedium, aetChanged);

		return TRUE;
	}
	NxCatchAll("Failed to activate EMR item");
	return FALSE;
}

BOOL DeleteEmrInfoMasterItem(IN long nID)
{
	BOOL bDeleted = FALSE;

	try {

		//for auditing
		CString strName;
		//DRT 8/30/2007 - PLID 27259 - Parameterized.
		_RecordsetPtr rs = CreateParamRecordset("SELECT Name FROM EMRInfoT WHERE ID = (SELECT ActiveEmrInfoID FROM EmrInfoMasterT WHERE ID = {INT})", nID);
		if(!rs->eof) {
			strName = AdoFldString(rs, "Name","");
		}
		rs->Close();

		// (j.jones 2007-05-21 10:44) - PLID 26061 - converted to use batched statements
		CString strSqlBatch = BeginSqlBatch();

		// (j.jones 2006-09-20 09:27) - PLID 22363 - we can't delete from a patient EMN,
		// although the only place this function is called would never let us get here if it was
		// on a patient EMN
		//AddStatementToSqlBatch(strSqlBatch, "DELETE FROM EmrSelectT WHERE EmrDetailID IN (SELECT ID FROM EmrDetailsT WHERE EmrInfoID = %li)", nID);
		//AddStatementToSqlBatch(strSqlBatch, "DELETE FROM EMRTableColumnWidthsT WHERE EMRDetailID IN (SELECT ID FROM EmrDetailsT WHERE EMRInfoID = %li)", nID);
		//AddStatementToSqlBatch(strSqlBatch, "DELETE FROM EMRDetailsT WHERE EMRInfoID = %li", nID);
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM ProcedureToEMRInfoT WHERE EMRInfoID IN (SELECT ID FROM EmrInfoT WHERE EmrInfoMasterID = %li)", nID);
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM DiagCodeToEMRInfoT WHERE EMRInfoID IN (SELECT ID FROM EmrInfoT WHERE EmrInfoMasterID = %li)", nID);
		//DRT 8/30/2007 - PLID 27259 - Parameterized.
		//DRT 2/19/2008 - PLID 28602 - Delete hotspots
		// (c.haag 2008-06-20 13:55) - PLID 30460 - Added a 4th ID for the parameter query that was missing earlier on
		// (z.manning 2009-03-19 17:10) - PLID 15971 - Need to delete table dropdown item actions too
		_RecordsetPtr rsActions = CreateParamRecordset(FormatString(
			"SELECT ID FROM EMRActionsT \r\n"
			"WHERE (DestType = %li AND DestID = {INT}) \r\n"
			"	OR (SourceType = %li AND SourceID IN (SELECT ID FROM EmrInfoT WHERE EmrInfoMasterID = {INT})) \r\n"
			"	OR (SourceType = %li AND SourceID IN (SELECT ID FROM EMRImageHotSpotsT WHERE EMRInfoID IN (SELECT ID FROM EmrInfoT WHERE EmrInfoMasterID = {INT}))) \r\n"
			"	OR (SourceType = %li AND SourceID IN (SELECT ID FROM EMRDataT WHERE EMRInfoID IN (SELECT ID FROM EmrInfoT WHERE EmrInfoMasterID = {INT}))) \r\n"
			"	OR (SourceType = %li AND SourceID IN ( \r\n"
			"		SELECT EmrTableDropdownInfoT.ID \r\n"
			"		FROM EmrTableDropdownInfoT \r\n"
			"		LEFT JOIN EmrDataT ON EmrTableDropdownInfoT.EmrDataID = EmrDataT.ID \r\n"
			"		LEFT JOIN EmrInfoT ON EmrDataT.EmrInfoID = EmrInfoT.ID \r\n"
			"		WHERE EmrInfoMasterID = {INT} \r\n"
			"	)) \r\n"
			, eaoEmrItem, eaoEmrItem, eaoEmrImageHotSpot, eaoEmrDataItem, eaoEmrTableDropDownItem)
			, nID, nID, nID, nID, nID);
		while(!rsActions->eof) {
			// (j.jones 2013-07-22 17:23) - PLID 57277 - DeleteEMRAction returns a SQL fragment,
			// but this function doesn't support parameterization (yet) so it has to be flattened
			AddStatementToSqlBatch(strSqlBatch, "%s", DeleteEMRAction(AdoFldLong(rsActions, "ID")).Flatten());
			rsActions->MoveNext();
		}
		rsActions->Close();

		//TES 12/12/2007 - PLID 28340 - Moved this to delete the details BEFORE deleting these other tables, some of 
		// which, (like EmrDataT) are referenced by details.
		// (j.jones 2007-01-12 11:04) - PLID 24027 - if you delete details, you must delete everything they spawn,
		// which is handled in DeleteEMRTemplateDetail		
		//DRT 8/30/2007 - PLID 27259 - Parameterized.
		_RecordsetPtr rsDetails = CreateParamRecordset("SELECT ID FROM EmrTemplateDetailsT WHERE EMRInfoMasterID = {INT}", nID);
		while(!rsDetails->eof) {
			// (a.walling 2014-01-30 00:00) - PLID 60546 - Quantize
			Nx::Quantum::Batch strDeleteBatch;
			DeleteEMRTemplateDetail(AdoFldLong(rsDetails,"ID"), strDeleteBatch);
			strSqlBatch += strDeleteBatch.FlattenToEither();
			rsDetails->MoveNext();
		}
		rsDetails->Close();

		// (z.manning 2013-03-11 14:52) - PLID 55554 - Deleting EMR data records now in its own function
		CSqlFragment sqlDeleteData;
		GetDeleteEmrDataSql(CSqlFragment("SELECT D.ID FROM EMRDataT D INNER JOIN EmrInfoT I ON D.EmrInfoID = I.ID WHERE I.EmrInfoMasterID = {INT}", nID), sqlDeleteData);
		strSqlBatch += sqlDeleteData.Flatten();

		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM EMRInfoDefaultsT WHERE EMRInfoID IN (SELECT ID FROM EmrInfoT WHERE EmrInfoMasterID = %li)", nID);
		//DRT 2/19/2008 - PLID 28602 - Delete hotspots
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM EMRImageHotSpotsT WHERE EMRInfoID IN (SELECT ID FROM EmrInfoT WHERE EmrInfoMasterID = %li)", nID);
		// (z.manning 2011-04-06 10:51) - PLID 42337
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM EmrProviderFloatDataT WHERE EmrDataGroupID NOT IN (SELECT EMRDataT.EMRDataGroupID FROM EMRDataT)");
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM EMRInfoCategoryT WHERE EMRInfoID IN (SELECT ID FROM EmrInfoT WHERE EmrInfoMasterID = %li)", nID);
		//TES 9/8/2005 - They may have stored exports that reference this item.
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM ExportFieldsT WHERE FieldID = 480 AND DynamicID IN (SELECT ID FROM EmrInfoT WHERE EmrInfoMasterID = %li)", nID);
		// (j.jones 2008-10-22 17:32) - PLID 31692 - delete any EMR Analysis details that reference the master item,
		// but do not delete their master records, even if there are no details left in them
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM EMRAnalysisConfigDetailsT WHERE EMRInfoMasterID = %li", nID);
		// (z.manning 2011-10-24 12:51) - PLID 46082 - Handle stamp exclusions
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM EmrInfoStampExclusionsT WHERE EmrInfoMasterID = %li", nID);
		AddStatementToSqlBatch(strSqlBatch, "UPDATE EmrInfoMasterT SET ActiveEmrInfoID = NULL WHERE ID = %li", nID);
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM EMRInfoT WHERE EmrInfoMasterID = %li", nID);
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM EmrInfoMasterT WHERE ID = %li", nID);
	
		ExecuteSqlBatch(strSqlBatch);

		long nAuditID = BeginNewAuditEvent();
		AuditEvent(-1, "", nAuditID, aeiEMRMasterItemDeleted, nID, "", strName, aepHigh, aetDeleted);

		bDeleted = TRUE;

	}NxCatchAll("Error in DeleteEmrInfoMasterItem");

	return bDeleted;
}

// (z.manning 2013-03-11 10:42) - PLID 55554
void GetDeleteEmrDataSql(IN CSqlFragment &sqlEMRDataIDsForInClause, OUT CSqlFragment &sql)
{
	if(sqlEMRDataIDsForInClause.IsEmpty()) {
		return;
	}

	// (j.jones 2015-12-22 10:31) - PLID 67769 - Added a table for EMRDataIDs to delete,
	// so we only have one IN clause. All other queries have been rewritten to use joins
	// and not IN clauses.
	sql += CSqlFragment(
		"DECLARE @EmrDataIDsT TABLE (EMRDataID INT NOT NULL PRIMARY KEY) \r\n"
		"INSERT INTO @EmrDataIDsT (EMRDataID) \r\n"
		"SELECT DISTINCT EMRDataT.ID FROM EMRDataT \r\n"
		"WHERE EMRDataT.ID IN ({SQL}); \r\n"
		, sqlEMRDataIDsForInClause);

	// (z.manning 2013-03-11 12:43) - PLID 55554 - Find all the data group IDs that are being deleted including
	// whether or not we're deleting the final instance of the data group.
	sql += CSqlFragment(
		"DECLARE @dataGroupToDelete TABLE (ID INT NOT NULL, StillExists BIT NOT NULL) \r\n"
		"INSERT INTO @dataGroupToDelete (ID, StillExists) \r\n"
		"SELECT	DISTINCT DelData.EmrDataGroupID, CASE WHEN AllData.ID IS NULL THEN 0 ELSE 1 END \r\n"
		"FROM	EmrDataT DelData \r\n"
		"INNER JOIN @EmrDataIDsT ED ON DelData.ID = ED.EMRDataID \r\n"
		//AllData is present if more EMRDataT records exist for our group, and not in our temp table
		"LEFT JOIN ( \r\n"
		"	SELECT EmrDataT.ID, EMRDataT.EmrDataGroupID FROM EmrDataT \r\n"
		"	LEFT JOIN @EmrDataIDsT ED ON EmrDataT.ID = ED.EMRDataID \r\n"
		"	WHERE ED.EMRDataID Is Null \r\n"
		") AS AllData ON DelData.EmrDataGroupID = AllData.EmrDataGroupID \r\n");

	// (z.manning 2013-03-11 12:44) - PLID 55554 - Find all dropdown groups that are being completely deleted.
	sql += CSqlFragment(
		"DECLARE @dropdownGroupToDelete TABLE (ID INT NOT NULL) \r\n"
		"INSERT INTO @dropdownGroupToDelete (ID) \r\n"
		"SELECT	DISTINCT DelDropdown.DropdownGroupID \r\n"
		"FROM	EmrTableDropdownInfoT DelDropdown \r\n"
		"INNER JOIN @EmrDataIDsT ED ON DelDropdown.EMRDataID = ED.EMRDataID \r\n"
		"LEFT JOIN ( \r\n"
		"	SELECT EmrTableDropdownInfoT.ID, EmrTableDropdownInfoT.EmrDataID, EmrTableDropdownInfoT.DropdownGroupID FROM EmrTableDropdownInfoT \r\n"
		"	LEFT JOIN @EmrDataIDsT ED ON EmrTableDropdownInfoT.EmrDataID = ED.EMRDataID \r\n"
		"	WHERE ED.EMRDataID Is Null \r\n"
		") AS AllDropdown ON DelDropdown.DropdownGroupID = AllDropdown.DropdownGroupID \r\n"
		"WHERE AllDropdown.ID IS NULL \r\n");

	sql += CSqlFragment("DELETE OrderSetTemplateMedicationsT "
		"FROM OrderSetTemplateMedicationsT "
		"INNER JOIN DrugList ON OrderSetTemplateMedicationsT.MedicationID = DrugList.ID "
		"INNER JOIN @EmrDataIDsT ED ON DrugList.EmrDataID = ED.EMRDataID \r\n");

	sql += CSqlFragment("DELETE EmrTemplateSelectT "
		"FROM EmrTemplateSelectT "
		"INNER JOIN @EmrDataIDsT ED ON EmrTemplateSelectT.EmrDataID = ED.EMRDataID \r\n");

	sql += CSqlFragment("DELETE EMRInfoDefaultsT "
		"FROM EMRInfoDefaultsT "
		"INNER JOIN @EmrDataIDsT ED ON EMRInfoDefaultsT.EmrDataID = ED.EMRDataID \r\n");

	sql += CSqlFragment("UPDATE EmrActionsT SET Deleted = 1 "
		"FROM EMRActionsT "
		"INNER JOIN @EmrDataIDsT ED ON EMRActionsT.SourceID = ED.EMRDataID "
		"WHERE SourceType = {CONST_INT} \r\n", eaoEmrDataItem);

	sql += CSqlFragment("DELETE EMRTableColumnWidthsT "
		"FROM EMRTableColumnWidthsT "
		"INNER JOIN @EmrDataIDsT ED ON EMRTableColumnWidthsT.EMRDataID_Y = ED.EMRDataID \r\n");

	sql += CSqlFragment("DELETE EMRTableColumnWidthsT "
		"FROM EMRTableColumnWidthsT "
		"INNER JOIN @EmrDataIDsT ED ON EMRTableColumnWidthsT.EMRDataID_X = ED.EMRDataID \r\n");

	sql += CSqlFragment("DELETE EMRTemplateTableColumnWidthsT "
		"FROM EMRTemplateTableColumnWidthsT "
		"INNER JOIN @EmrDataIDsT ED ON EMRTemplateTableColumnWidthsT.EMRDataID_Y = ED.EMRDataID \r\n");

	sql += CSqlFragment("DELETE EMRTemplateTableColumnWidthsT "
		"FROM EMRTemplateTableColumnWidthsT "
		"INNER JOIN @EmrDataIDsT ED ON EMRTemplateTableColumnWidthsT.EMRDataID_X = ED.EMRDataID \r\n");

	sql += CSqlFragment("DELETE EmrTemplateTableDefaultsT "
		"FROM EmrTemplateTableDefaultsT "
		"INNER JOIN @EmrDataIDsT ED ON EmrTemplateTableDefaultsT.EMRDataID_Y = ED.EMRDataID \r\n");

	sql += CSqlFragment("DELETE EmrTemplateTableDefaultsT "
		"FROM EmrTemplateTableDefaultsT "
		"INNER JOIN @EmrDataIDsT ED ON EmrTemplateTableDefaultsT.EMRDataID_X = ED.EMRDataID \r\n");
	
	sql += CSqlFragment("UPDATE EMRTableDropdownInfoT SET CopiedFromDropdownID = NULL "
		"FROM EMRTableDropdownInfoT "
		"INNER JOIN EMRTableDropdownInfoT TD2 ON EMRTableDropdownInfoT.CopiedFromDropdownID = TD2.ID "
		"INNER JOIN @EmrDataIDsT ED ON TD2.EMRDataID = ED.EMRDataID \r\n");

	// (z.manning 2011-09-28 12:07) - PLID 45729 - Delete stamp filter
	sql += CSqlFragment("DELETE EmrTableDropdownStampFilterT "
		"FROM EmrTableDropdownStampFilterT "
		"INNER JOIN EMRTableDropdownInfoT TD ON EmrTableDropdownStampFilterT.EmrTableDropdownInfoID = TD.ID "
		"INNER JOIN @EmrDataIDsT ED ON TD.EMRDataID = ED.EMRDataID \r\n");

	// (j.jones 2012-11-27 09:45) - PLID 53144 - added EMRTableDropdownStampDefaultsT
	sql += CSqlFragment("DELETE EMRTableDropdownStampDefaultsT "
		"FROM EMRTableDropdownStampDefaultsT "
		"INNER JOIN EMRTableDropdownInfoT TD ON EMRTableDropdownStampDefaultsT.EmrTableDropdownInfoID = TD.ID "
		"INNER JOIN @EmrDataIDsT ED ON TD.EMRDataID = ED.EMRDataID \r\n");

	// (z.manning 2009-02-11 10:53) - PLID 33029 - Delete EMR actions associated with deleted dropdown items
	sql += CSqlFragment("UPDATE EmrActionsT SET Deleted = 1 "
		"FROM EmrActionsT "
		"INNER JOIN EMRTableDropdownInfoT TD ON EMRActionsT.SourceType = {CONST_INT} AND EMRActionsT.SourceID = TD.ID "
		"INNER JOIN @EmrDataIDsT ED ON TD.EMRDataID = ED.EMRDataID \r\n", eaoEmrTableDropDownItem);

	sql += CSqlFragment("DELETE EMRTableDropdownInfoT "
		"FROM EMRTableDropdownInfoT "
		"INNER JOIN @EmrDataIDsT ED ON EMRTableDropdownInfoT.EMRDataID = ED.EMRDataID \r\n");

	// (z.manning 2011-04-06 11:04) - PLID 42337
	sql += CSqlFragment("DELETE EmrProviderFloatTableDropdownT "
		"FROM EmrProviderFloatTableDropdownT "
		"INNER JOIN @dropdownGroupToDelete DG ON EmrProviderFloatTableDropdownT.EmrTableDropdownGroupID = DG.ID \r\n");

	// (j.gruber 2013-10-02 12:52) - PLID 58675 - EMR Drop Down Codes
	sql += CSqlFragment("DELETE EmrTableDropdownGroupCodesT "
		"FROM EmrTableDropdownGroupCodesT "
		"INNER JOIN @dropdownGroupToDelete DG ON EmrTableDropdownGroupCodesT.EmrTableDropdownGroupID = DG.ID \r\n");

	// (a.walling 2007-08-29 13:30) - PLID 27223 - Remove any orphaned dropdown groups.
	sql += CSqlFragment("DELETE EMRTableDropdownGroupsT "
		"FROM EMRTableDropdownGroupsT "
		"INNER JOIN @dropdownGroupToDelete DG ON EMRTableDropdownGroupsT.ID = DG.ID \r\n");

	sql += CSqlFragment("UPDATE EMRDataT SET CopiedFromDataID = NULL "
		"FROM EMRDataT "
		"INNER JOIN @EmrDataIDsT ED ON EMRDataT.CopiedFromDataID = ED.EMRDataID \r\n");

	// (c.haag 2007-04-11 12:06) - PLID 25575 - We used to do special handling for system EMR items, but it would occasionally
	// cause problems where the EmrDataT table and auxiliary table (DrugList) would go "out of sync". There is really no need
	// for any kind of special handling, because in the end, we get data ID's, and these statements will update the records that
	// need to be updated. We should also be consistent with what we know to be historically working code. Notice that we don't
	// delete from PatientAllergyT or PatientMedications; we want this to fail if records exist because we should not delete historic 
	// patient data
	sql += CSqlFragment("DELETE DrugAllergyT "
		"FROM DrugAllergyT "
		"INNER JOIN DrugList ON DrugAllergyT.DrugID = DrugList.ID "
		"INNER JOIN @EmrDataIDsT ED ON DrugList.EMRDataID = ED.EMRDataID \r\n");

	sql += CSqlFragment("DELETE DrugAllergyT "
		"FROM DrugAllergyT "
		"INNER JOIN AllergyT ON DrugAllergyT.AllergyID = AllergyT.ID "
		"INNER JOIN @EmrDataIDsT ED ON AllergyT.EMRDataID = ED.EMRDataID \r\n");

	// (c.haag 2009-03-12 09:49) - PLID 32589 - Delete all actions that spawn this drug
	sql += CSqlFragment("UPDATE EMRActionsT SET Deleted = 1 "
		"FROM EMRActionsT "
		"INNER JOIN DrugList ON EMRActionsT.DestType = {CONST_INT} AND EMRActionsT.DestID = DrugList.ID "
		"INNER JOIN @EmrDataIDsT ED ON DrugList.EMRDataID = ED.EMRDataID \r\n");

	sql += CSqlFragment("DELETE DrugList "
		"FROM DrugList "
		"INNER JOIN @EmrDataIDsT ED ON DrugList.EMRDataID = ED.EMRDataID \r\n");

	sql += CSqlFragment("DELETE AllergyT "
		"FROM AllergyT "
		"INNER JOIN @EmrDataIDsT ED ON AllergyT.EMRDataID = ED.EMRDataID \r\n");

	sql += CSqlFragment("DELETE EMRInfoCommonListItemsT "
		"FROM EMRInfoCommonListItemsT "
		"INNER JOIN @EmrDataIDsT ED ON EMRInfoCommonListItemsT.EMRDataID = ED.EMRDataID \r\n");

	//TES 1/30/2007 - PLID 24377 - Clear out any links.
	sql += CSqlFragment("DELETE EmrItemLinkedDataT "
		"FROM EmrItemLinkedDataT "
		"INNER JOIN @EmrDataIDsT ED ON EmrItemLinkedDataT.EMRDataID = ED.EMRDataID \r\n");

	// (j.jones 2012-09-25 09:16) - PLID 52316 - clear the parent label IDs referencing this EMRDataID
	sql += CSqlFragment("UPDATE EMRDataT SET ParentLabelID = NULL "
		"FROM EMRDataT "
		"INNER JOIN @EmrDataIDsT ED ON EMRDataT.ParentLabelID = ED.EMRDataID \r\n");

	// (j.dinatale 2012-09-10 10:57) - PLID 52551 - remove any OMR Detail values for data groups that do not exist before we delete from EMRDataGroupsT
	sql += CSqlFragment("DELETE OMRFormDetailT "
		"FROM OMRFormDetailT "
		"INNER JOIN @dataGroupToDelete DG ON OMRFormDetailT.EmrDataGroupID = DG.ID \r\n"); 

	// (z.manning 2009-03-17 11:48) - PLID 33242 - Clear out tables with a FK on EmrDataGroupsT
	// (z.manning 2009-03-17 11:42) - PLID 33242 - As of today, 11 tables have a SourceDataGroupID field
	// but these are the only ones where it is safe to clear it out because the others are associated
	// with patient EMR data and are thus subject to triggers.
	sql += CSqlFragment("UPDATE EMRTemplateDetailsT SET SourceDataGroupID = NULL "
		"FROM EMRTemplateDetailsT "
		"INNER JOIN EMRDataT ON EMRTemplateDetailsT.SourceDataGroupID = EMRDataT.EMRDataGroupID "
		"INNER JOIN @EmrDataIDsT ED ON EMRDataT.ID = ED.EMRDataID \r\n"); 

	sql += CSqlFragment("UPDATE EMRTemplateTopicsT SET SourceDataGroupID = NULL "
		"FROM EMRTemplateTopicsT "
		"INNER JOIN EMRDataT ON EMRTemplateTopicsT.SourceDataGroupID = EMRDataT.EMRDataGroupID "
		"INNER JOIN @EmrDataIDsT ED ON EMRDataT.ID = ED.EMRDataID \r\n"); 

	sql += CSqlFragment("UPDATE LabsT SET SourceDataGroupID = NULL "
		"FROM LabsT "
		"INNER JOIN EMRDataT ON LabsT.SourceDataGroupID = EMRDataT.EMRDataGroupID "
		"INNER JOIN @EmrDataIDsT ED ON EMRDataT.ID = ED.EMRDataID \r\n");

	sql += CSqlFragment("UPDATE EmrTodosT SET SourceDataGroupID = NULL "
		"FROM EmrTodosT "
		"INNER JOIN EMRDataT ON EmrTodosT.SourceDataGroupID = EMRDataT.EMRDataGroupID "
		"INNER JOIN @EmrDataIDsT ED ON EMRDataT.ID = ED.EMRDataID \r\n");

	sql += CSqlFragment("DELETE EMRDataT "
		"FROM EMRDataT "
		"INNER JOIN @EmrDataIDsT ED ON EMRDataT.ID = ED.EMRDataID \r\n");

	// (d.thompson 2010-01-27) - PLID 36927 - Clear out any Meaningful Use report configurations that are no longer in use due to this change.
	sql += CSqlFragment("DELETE ConfigRT "
		"FROM ConfigRT "
		"INNER JOIN @dataGroupToDelete DG ON DG.StillExists = 0 AND ConfigRT.Name LIKE 'CCHITReportInfo_%' AND ConfigRT.IntParam = DG.ID \r\n");
	
	// (j.jones 2008-10-22 17:32) - PLID 31692 - Delete any EMR Analysis details that reference these data items.
	// Do not delete their master records, even if there are no details left in them.
	sql += CSqlFragment("DELETE EMRAnalysisConfigDetailsT "
		"FROM EMRAnalysisConfigDetailsT "
		"INNER JOIN @dataGroupToDelete DG ON DG.StillExists = 0 AND EMRAnalysisConfigDetailsT.EmrDataGroupID = DG.ID \r\n");
	
	// (z.manning 2011-04-05 17:41) - PLID 43140 - Delete from EmrDetailListOrderT as a fail-safe
	sql += CSqlFragment("DELETE EmrDetailListOrderT \r\n"
		"FROM EmrDetailListOrderT "
		"INNER JOIN @dataGroupToDelete DG ON DG.StillExists = 0 AND EmrDetailListOrderT.EmrDataGroupID = DG.ID \r\n");
	
	// (z.manning 2011-04-06 10:48) - PLID 42337
	sql += CSqlFragment("DELETE EmrProviderFloatDataT "
		"FROM EmrProviderFloatDataT "
		"INNER JOIN @dataGroupToDelete DG ON DG.StillExists = 0 AND EmrProviderFloatDataT.EmrDataGroupID = DG.ID \r\n");
	
	// (j.gruber 2013-10-02 12:51) - PLID 58674 - EMR Data Codes
	sql += CSqlFragment("DELETE EMRDataGroupCodesT "
		"FROM EMRDataGroupCodesT "
		"INNER JOIN @dataGroupToDelete DG ON DG.StillExists = 0 AND EMRDataGroupCodesT.EmrDataGroupID = DG.ID \r\n");
	
	// (j.gruber 2013-10-02 12:55) - PLID 58676 - EMRTableGroupCodesT
	sql += CSqlFragment("DELETE EmrTableDataGroupCodesT "
		"FROM EmrTableDataGroupCodesT "
		"INNER JOIN @dataGroupToDelete DG ON DG.StillExists = 0 AND EmrTableDataGroupCodesT.EmrDataGroupID_Y = DG.ID \r\n");

	sql += CSqlFragment("DELETE EmrTableDataGroupCodesT "
		"FROM EmrTableDataGroupCodesT "
		"INNER JOIN @dataGroupToDelete DG ON DG.StillExists = 0 AND EmrTableDataGroupCodesT.EmrDataGroupID_X = DG.ID \r\n");
	
	// (j.jones 2007-08-14 15:39) - PLID 27053 - added EMRDataGroupsT, and thus we have to attempt to delete from it
	sql += CSqlFragment("DELETE EMRDataGroupsT "
		"FROM EMRDataGroupsT "
		"INNER JOIN @dataGroupToDelete DG ON DG.StillExists = 0 AND EMRDataGroupsT.ID = DG.ID \r\n");
}

// (j.jones 2009-08-12 08:55) - PLID 35189 - this takes in a datalist2 row now
BOOL CALLBACK AddItemToTabMultiSelectContextMenuProc(IN CMultiSelectDlg *pwndMultSelDlg, IN LPARAM pParam, IN NXDATALIST2Lib::IRowSettings *lpRow, IN CWnd* pContextWnd, IN const CPoint &point, IN OUT CArray<long, long> &m_aryOtherChangedMasterIDs)
{
	// The context menu for the data element list is based on the current selection
	if (lpRow) {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		// Build the menu for the current row
		CMenu mnu;
		mnu.CreatePopupMenu();
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, 1, "&Edit...");
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, 2, "&Copy...");

		// Pop up the menu and gather the immediate response
		CPoint pt = CalcContextMenuPos(pContextWnd, point);
		long nMenuResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD|TPM_TOPALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, pt.x, pt.y, pwndMultSelDlg);
		switch (nMenuResult) {
		case 1: // edit
			{
				//TES 12/7/2006 - PLID 23724 - This is now a list of EmrInfoMasterT records, not EmrInfoT records.
				long nInfoMasterID = VarLong(pRow->GetValue(CMultiSelectDlg::mslcID));
				// (c.haag 2006-04-04 11:08) - PLID 19890 - Check permissions
				if(!CheckCurrentUserPermissions(bioAdminEMR, sptRead)) {
					break;
				}
				CEmrItemEntryDlg dlg(pwndMultSelDlg);
				if(pParam) {
					CEmrTreeWnd* pTreeWnd = (CEmrTreeWnd*)pParam;
					if(IsWindow(pTreeWnd->GetSafeHwnd())) {
						dlg.SetCurrentEMN(pTreeWnd->GetLastEMN());
					}
				}
				if (dlg.OpenWithMasterID(nInfoMasterID) == IDOK) {

					CString strType = "";
					BOOL bInactive = TRUE;
					//DRT 8/30/2007 - PLID 27259 - Parameterized.
					_RecordsetPtr rs = CreateParamRecordset("SELECT Inactive, "
						"CASE WHEN DataType = 1 THEN 'Text' WHEN DataType = 2 THEN 'Single-Select List' "
						"WHEN DataType = 3 THEN 'Multi-Select List' WHEN DataType = 4 THEN 'Image' "
						"WHEN DataType = 5 THEN 'Slider' WHEN DataType = 6 THEN 'Narrative' "
						"WHEN DataType = 7 THEN 'Table' END AS Type FROM EMRInfoT INNER JOIN EmrInfoMasterT ON EmrInfoT.ID = "
						"EmrInfoMasterT.ActiveEmrInfoID WHERE EmrInfoMasterT.ID = {INT}", nInfoMasterID);
					if(!rs->eof) {
						strType = AdoFldString(rs, "Type","");
						bInactive = AdoFldBool(rs, "Inactive");
					}
					rs->Close();
					
					pRow->PutValue(CMultiSelectDlg::mslcSelected, _variant_t(VARIANT_TRUE, VT_BOOL));
					pRow->PutValue(CMultiSelectDlg::mslcName, (LPCTSTR)dlg.GetName());
					pRow->PutValue(CMultiSelectDlg::mslcName+1, (LPCTSTR)strType);

					if(bInactive) {
						// (c.haag 2006-02-27 12:08) - PLID 12763 - If an item is
						// inactive, then we need to hide it from the multi-select list
						// because the list should not have inactive items in it
						// (j.jones 2009-08-11 18:02) - PLID 35189 - converted the multi-select list into a datalist2
						pwndMultSelDlg->GetDataList2()->RemoveRow(pRow);

					} else {
						//add to m_aryOtherChangedMasterIDs
						BOOL bFound = FALSE;
						for(int i=0; i<m_aryOtherChangedMasterIDs.GetSize() && !bFound; i++) {
							if(m_aryOtherChangedMasterIDs.GetAt(i) == nInfoMasterID)
								bFound = TRUE;
						}
						if(!bFound)
							m_aryOtherChangedMasterIDs.Add(nInfoMasterID);

						// (z.manning 2010-03-22 14:36) - PLID 37228 - Also handle this for possible changes to the smart stamp
						// table detail.
						if(dlg.GetSmartStampTableMasterID() != -1) {
							bFound = FALSE;
							for(int i=0; i < m_aryOtherChangedMasterIDs.GetSize() && !bFound; i++) {
								if(m_aryOtherChangedMasterIDs.GetAt(i) == dlg.GetSmartStampTableMasterID()) {
									bFound = TRUE;
								}
							}
						}
						if(!bFound) {
							m_aryOtherChangedMasterIDs.Add(dlg.GetSmartStampTableMasterID());
						}


						//TES 12/7/2006 - PLID 23724 - The master ID stays the same.
						/*if(dlg.GetID() != nID) {
							//if the ID's didn't match, that means a new item was created, best refresh that too
							bFound = FALSE;
							for(int i=0; i<m_aryOtherIDs.GetSize() && !bFound; i++) {
								if(m_aryOtherIDs.GetAt(i) == dlg.GetID())
									bFound = TRUE;
							}

							if(!bFound)
								m_aryOtherIDs.Add(dlg.GetID());
						}*/
					}

					//TES 12/7/2006 - PLID 23724 - The master ID stays the same.
					/*if(dlg.GetID() != nID) {
						// (j.jones 2006-08-22 16:40) - PLID 22160 - if the ID's don't match,
						// that means a new item was created, and thus we need to requery
						pwndMultSelDlg->GetDatalist()->Requery();
						pwndMultSelDlg->GetDatalist()->SetSelByColumn(CMultiSelectDlg::mslcID, dlg.GetID());
					}*/
				}
			}
			break;
		case 2: // copy
			{
				//TES 12/7/2006 - PLID 23724 - This is now a list of EmrInfoMasterT records, not EmrInfoT records.
				long nInfoMasterID = VarLong(pRow->GetValue(CMultiSelectDlg::mslcID));

				// (j.jones 2010-06-07 09:39) - PLID 39029 - also load the DataSubType
				_RecordsetPtr rs = CreateParamRecordset("SELECT ActiveEmrInfoID, EMRInfoT.DataSubType FROM EmrInfoMasterT "
					"INNER JOIN EMRInfoT ON EMRInfoMasterT.ActiveEmrInfoID = EMRInfoT.ID "
					"WHERE EmrInfoMasterT.ID = {INT}", nInfoMasterID);
				if(rs->eof) {
					ThrowNxException("Could not load EMRInfoMasterID = %li", nInfoMasterID);
				}

				long nInfoID = AdoFldLong(rs, "ActiveEmrInfoID");
				if(nInfoID == EMR_BUILT_IN_INFO__IMAGE) {
					AfxMessageBox("The 'Image' type is built-in and cannot be copied.");
					break;
				}
				// (c.haag 2008-06-16 11:13) - PLID 30319 - Don't copy EMR text macros
				if(nInfoID == EMR_BUILT_IN_INFO__TEXT_MACRO) {
					AfxMessageBox("The EMR Text Macro item is built-in and cannot be copied.");
					break;
				}

				// (j.jones 2010-06-04 17:02) - PLID 39029 - same for generic tables
				long nDataSubType = AdoFldByte(rs, "DataSubType");
				if(nDataSubType == eistGenericTable) {
					AfxMessageBox("The EMR Generic Table item is built-in and cannot be copied.");
					break;
				}

				long nNewInfoMasterID = -1, nNewItemID = -1;
				CString strNewItemName = "";
				if(CopyEmrInfoItem(nInfoMasterID, VarString(pRow->GetValue(CMultiSelectDlg::mslcName)), FALSE, &nNewInfoMasterID, &nNewItemID, &strNewItemName)) {
					//Add the new row.
					ASSERT(nNewItemID != -1);
					// (j.jones 2009-08-11 18:02) - PLID 35189 - converted the multi-select list into a datalist2
					NXDATALIST2Lib::IRowSettingsPtr pNewRow = pwndMultSelDlg->GetDataList2()->GetNewRow();
					pNewRow->PutValue(CMultiSelectDlg::mslcID, nNewInfoMasterID);
					pNewRow->PutValue(CMultiSelectDlg::mslcSelected, COleVariant((short)FALSE,VT_BOOL));
					pNewRow->PutValue(CMultiSelectDlg::mslcName, (_bstr_t)strNewItemName);
					pNewRow->PutValue(CMultiSelectDlg::mslcName+1, pRow->GetValue(CMultiSelectDlg::mslcName+1));
					pwndMultSelDlg->GetDataList2()->AddRowSorted(pNewRow, NULL);
					pwndMultSelDlg->GetDataList2()->Sort();
					// (z.manning, 3/14/2006, PLID 19670) - Let's select the newly copied item.
					// (z.manning 2008-09-03 17:33) - PLID 31245 - Use the info master ID here now.
					pwndMultSelDlg->GetDataList2()->SetSelByColumn(CMultiSelectDlg::mslcID, nNewInfoMasterID);
				}
			}
			break;

		case 0:
			// The user canceled, do nothing
			break;
		default:
			// Unexpected response!
			ASSERT(FALSE);
			ThrowNxException("Unexpected return value %li from context menu!", nMenuResult);
		}
		return TRUE;
	} else {
		return FALSE;
	}
}

// (z.manning 2010-03-01 16:15) - PLID 37571
// (a.walling 2010-11-01 12:38) - PLID 40965 - Note that strWhere is inserted TWICE!
CSqlFragment GetLoadActionInfoQuery(CSqlFragment sqlWhere)
{
	// (z.manning 2011-06-28 14:12) - PLID 44347 - Added join to LabAnatomyID
	// (z.manning 2011-11-09 10:11) - PLID 46367 - Added source group fields
	// (c.haag 2014-07-23) - PLID 62788 - Added EMRProblemActionsT.SNOMEDCodeID
	// (a.walling 2014-08-04 09:39) - PLID 62684 - Loading EmrAction::filter data including EmrActionAnatomicLocationQualifiersT
	// (s.tullis 2015-03-05 15:35) - PLID 64724 - Added Do not show on CCDA 
	// (r.gonet 2015-03-10 14:48) - PLID 65013 - Added DoNotShowOnProblemPrompt.
	return CSqlFragment(
		"SELECT EMRActionsT.ID, EMRActionsT.Deleted, SourceID, SourceType, DestID, DestType, EMRActionsT.SortOrder, Popup, "
		"	SpawnAsChild, EMRActionChargeDataT.Prompt, EMRActionChargeDataT.DefaultQuantity, EMRActionChargeDataT.Modifier1Number, "
		"	EMRActionChargeDataT.Modifier2Number, EMRActionChargeDataT.Modifier3Number, EMRActionChargeDataT.Modifier4Number, "
		"	Convert(bit, (CASE WHEN EMRActionsT.DestType = {CONST} THEN Coalesce(DestEMRInfoT.OnePerEmn, 0) ELSE 0 END)) AS OnePerEmn, "
		"	CASE WHEN EmrActionsT.DestType = {CONST} THEN "
		"	(CASE WHEN EmrActionsT.SourceType = {CONST} THEN SourceEMRDataT.Data "
		"      WHEN EmrActionsT.SourceType = {CONST} THEN SourceEMRInfoT.Name "
		"      WHEN EmrActionsT.SourceType = {CONST} THEN SourceProcedureT.Name "
		"	ELSE '' END) ELSE '' END AS SourceName, "
		"	RemindType, RemindInterval, DeadlineType, DeadlineInterval, Notes, Priority, Task, CategoryID, \r\n"
		"	EmrActionAnatomicLocationsT.LabAnatomyID, EmrActionAnatomicLocationsT.AnyQualifierAnatomySide, EmrActionAnatomicLocationsT.NoQualifierAnatomySide, \r\n"
		"	EmrActionAnatomicLocationQualifiersT.AnatomicQualifierID, EmrActionAnatomicLocationQualifiersT.AnatomySide, \r\n"
		"	SourceEMRDataT.EmrDataGroupID AS SourceDataGroupID, \r\n"
		"	SourceEmrTableDropdownInfoT.DropdownGroupID AS SourceTableDropdownGroupID, SourceEmrImageHotSpotsT.EmrSpotGroupID AS SourceHotSpotGroupID, \r\n"
		//(s.dhole 7/17/2014 4:24 PM ) - PLID 62724 Added DiagCodeID_ICD10 , DiagCodeID_ICD9
		"  EmrActionDiagnosisDataT.DiagCodeID_ICD9, EmrActionDiagnosisDataT.DiagCodeID_ICD10  \r\n"

		"FROM EmrActionsT "
		"LEFT JOIN EMRActionChargeDataT ON EMRActionsT.ID = EMRActionChargeDataT.ActionID AND EMRActionsT.DestType = {CONST} "
		"LEFT JOIN EMRDataT SourceEMRDataT ON EMRActionsT.SourceID = SourceEMRDataT.ID AND EMRActionsT.SourceType = {CONST} "
		"LEFT JOIN EMRInfoT SourceEMRInfoT ON EMRActionsT.SourceID = SourceEMRInfoT.ID AND EMRActionsT.SourceType = {CONST} "		
		"LEFT JOIN ProcedureT SourceProcedureT ON EMRActionsT.SourceID = SourceProcedureT.ID AND EMRActionsT.SourceType = {CONST} "
		"LEFT JOIN EmrTableDropdownInfoT SourceEmrTableDropdownInfoT ON EmrActionsT.SourceID = SourceEmrTableDropdownInfoT.ID AND EmrActionsT.SourceType = {CONST} \r\n"
		"LEFT JOIN EmrImageHotSpotsT SourceEmrImageHotSpotsT ON EmrActionsT.SourceID = SourceEmrImageHotSpotsT.ID AND EmrActionsT.SourceType = {CONST} \r\n"
		"LEFT JOIN EmrInfoMasterT DestEmrInfoMasterT ON EMRActionsT.DestID = DestEmrInfoMasterT.ID AND EMRActionsT.DestType = {CONST} "
		"LEFT JOIN EMRActionsTodoDataT ON EMRActionsT.ID = EMRActionsTodoDataT.ActionID AND EMRActionsT.DestType = {CONST} "
		"LEFT JOIN EMRInfoT DestEMRInfoT ON DestEmrInfoMasterT.ActiveEmrInfoID = DestEMRInfoT.ID AND EMRActionsT.DestType = {CONST} "
		"LEFT JOIN EmrActionAnatomicLocationsT ON EmrActionsT.ID = EmrActionAnatomicLocationsT.EmrActionID \r\n"
		"LEFT JOIN EmrActionAnatomicLocationQualifiersT "
			"ON EmrActionAnatomicLocationsT.EmrActionID = EmrActionAnatomicLocationQualifiersT.EmrActionID "
			"AND EmrActionAnatomicLocationsT.LabAnatomyID = EmrActionAnatomicLocationQualifiersT.LabAnatomyID \r\n"
		//(s.dhole 7/17/2014 4:37 PM ) - PLID 
		" LEFT JOIN EmrActionDiagnosisDataT ON EMRActionsT.ID = EmrActionDiagnosisDataT.emrActionID "
		"{SQL} "
		"ORDER BY CASE WHEN DestType = {CONST} THEN -1*EMRActionsT.SortOrder ELSE EMRActionsT.SortOrder END, EMRActionsT.ID DESC\r\n"
		"SELECT EmrProblemActionsT.ID, EmrActionID, DefaultDescription, DefaultStatus, SpawnToSourceItem, SNOMEDCodeID, DoNotShowOnCCDA, DoNotShowOnProblemPrompt "
		"FROM (SELECT * FROM EmrProblemActionsT WHERE Inactive = 0) EmrProblemActionsT "
		"INNER JOIN EmrActionsT ON EmrActionsT.ID = EmrProblemActionsT.EmrActionID "
		"{SQL} ",
		eaoEmrItem,		//for the OnePerEmn CASE statement
		eaoMintItems,	//for the outer SourceName CASE statement
		eaoEmrDataItem, //for the inner SourceName CASE statement
		eaoEmrItem,		//for the inner SourceName CASE statement
		eaoProcedure,	//for the inner SourceName CASE statement
		eaoCpt,			//for the EMRActionChargeDataT join
		eaoEmrDataItem,	//for the SourceEMRDataT join
		eaoEmrItem,		//for the SourceEMRInfoT join
		eaoProcedure,	//for the SourceProcedureT join
		eaoEmrTableDropDownItem, //for the SourceEmrTableDropdownInfoT join
		eaoEmrImageHotSpot, //for the SourceEmrImageHotSpotsT join
		eaoEmrItem,		//for the DestEmrInfoMasterT join
		eaoTodo,		// (c.haag 2008-06-04 11:05) - PLID 30221 - For the EMRActionsTodoDataT join
		eaoEmrItem,		//for the DestEMRInfoT join
		sqlWhere,		//the where clause fragment of course!
		eaoMintItems,	//for the ORDER BY logic
		sqlWhere
		);
}

// (a.walling 2010-11-01 12:38) - PLID 40965 - Note that strWhere is inserted TWICE!
// (a.walling 2014-07-08 14:19) - PLID 62812 - Use MFCArray
void LoadActionInfo(CSqlFragment sqlWhere, MFCArray<EmrAction> &arActions, _ConnectionPtr pCon /* = GetRemoteData()*/)
{
	//TES 2/2/2006 - Process eaoMintItems actions in reverse order, that way, when they are inserted immediately after their
	//source detail, they will appear in the correct order on screen. 
	// (j.jones 2007-01-22 10:05) - PLID 24356 - changed query's ordering to first show
	// non-deleted items, then use the sort order (carried over from previous version of the query),
	// then by ID descending. That way if there are multiple copies of an identical action, the
	// newest non-deleted version is returned first
	// (a.wetta 2007-04-12 12:47) - PLID 25302 - I have changed the query below to only include actions that have not been deleted.
	// Previously deleted actions were being included and were consequently being processed when they should not have been.  There
	// is no reason that deleted actions need to be processed when processing or revoking because the action is cleaned up before
	// it is marked as deleted.  In addition to chaning the query below to not include deleted actions, I also changed the strWhere
	// parameter passed into this function every time it is called to also not include deleted actions.  But, of course, if this 
	// function is called in the future, someone may not add the don't include deleted actions clause to the strWhere parameter.  So,
	// if you ever make the query below include deleted actions, beware!  Make sure to check everytime that this function is called
	// and see if it was called under the assumption that deleted actions would not be included.
	// (c.haag 2007-04-25 11:08) - PLID 25774 - We now use a connection object
	// (j.jones 2007-07-16 15:44) - PLID 26694 - added OnePerEmn load
	// (j.jones 2007-07-31 15:59) - PLID 26898 - added SourceName as a field, only filled when we're spawning topics
	// (c.haag 2008-07-21 15:03) - PLID 30725 - Added support for EMR problem spawning
	// (c.haag 2008-08-05 09:17) - PLID 30941 - We now consider the inactive flag of problem actions
	// (r.gonet 09/12/2012) - PLID 51949 - We now allow deleted actions to be loaded if the source type is wound care condition.
	_RecordsetPtr rsInfo = CreateParamRecordset(pCon, "{SQL}",
		GetLoadActionInfoQuery(CSqlFragment("WHERE {SQL} AND (EMRActionsT.Deleted = 0 OR EMRActionsT.SourceType = {CONST}) ", sqlWhere, (long)eaoWoundCareCodingCondition)));
	FillActionArray(rsInfo, pCon, arActions);
}

// (a.walling 2014-08-04 09:39) - PLID 62684 - Loading EmrAction::filter data including EmrActionAnatomicLocationQualifiersT
Emr::ActionFilter FillActionFilter(ADODB::_RecordsetPtr prs)
{
	Emr::ActionFilter filter;

	const long nActionID = AdoFldLong(prs, "ID");

	if (AdoFldNull(prs, "LabAnatomyID")) {
		return filter;
	}

	for (;;) {
		long nAnatomicLocationID = AdoFldLong(prs, "LabAnatomyID");

		char anyQualifierSide = AdoFldByte(prs, "AnyQualifierAnatomySide", Emr::sideNull);
		char noQualifierSide = AdoFldByte(prs, "NoQualifierAnatomySide", Emr::sideNull);

		if (anyQualifierSide != Emr::sideNull) {
			filter.anatomicLocationFilters.emplace(nAnatomicLocationID, -1, anyQualifierSide);
		}
		if (noQualifierSide != Emr::sideNull) {
			filter.anatomicLocationFilters.emplace(nAnatomicLocationID, 0, noQualifierSide);
		}

		long qualifierID = AdoFldLong(prs, "AnatomicQualifierID", -1);
		if (qualifierID != -1) {
			char side = AdoFldByte(prs, "AnatomySide", -1);

			filter.anatomicLocationFilters.emplace(nAnatomicLocationID, qualifierID, side);
		}
		else if (anyQualifierSide == Emr::sideNull && noQualifierSide == Emr::sideNull) {
			// if no qualifiers and we have a NULL any qualifier side, we assume it means match anything
			filter.anatomicLocationFilters.emplace(nAnatomicLocationID, -1, anyQualifierSide);
		}
		
		prs->MoveNext();

		if (prs->eof) {
			break;
		}
		if (nActionID != AdoFldLong(prs, "ID")) {
			break;
		}
	}
	prs->MovePrevious();

	return filter;
}

// (z.manning 2010-03-01 16:50) - PLID 37571
// (a.walling 2014-07-08 14:19) - PLID 62812 - Use MFCArray
void FillActionArray(ADODB::_RecordsetPtr rsInfo, _ConnectionPtr pCon, OUT MFCArray<EmrAction> &arActions)
{
	for(; !rsInfo->eof; rsInfo->MoveNext())
	{
		FieldsPtr fields = rsInfo->Fields;

		EmrAction ea;

		ea.nID = AdoFldLong(fields, "ID");
		// (a.walling 2014-08-04 09:39) - PLID 62684 - Load EmrAction::filter data
		ea.filter = FillActionFilter(rsInfo);

		ea.eaoSourceType = (EmrActionObject)AdoFldLong(fields,"SourceType");
		ea.nSourceID = AdoFldLong(fields,"SourceID");
		ea.eaoDestType = (EmrActionObject)AdoFldLong(fields,"DestType");
		ea.nDestID = AdoFldLong(fields, "DestID");
		ea.nSortOrder = AdoFldLong(fields,"SortOrder");
		ea.bPopup = AdoFldBool(fields,"Popup");
		ea.bSpawnAsChild = AdoFldBool(fields,"SpawnAsChild");
		// (z.manning 2011-11-09 10:26) - PLID 46367 - Source group fields
		ea.nSourceDataGroupID = AdoFldLong(fields, "SourceDataGroupID", -1);
		ea.nSourceHotSpotGroupID = AdoFldLong(fields, "SourceHotSpotGroupID", -1);
		ea.nSourceTableDropdownGroupID = AdoFldLong(fields, "SourceTableDropdownGroupID", -1);

		//DRT 1/10/2007 - PLID 24182 - We now have a separate table for extra charge information.  This only
		//	applies to eaoCpt type items.
		//(s.dhole 7/17/2014 4:24 PM ) - PLID 62724 Added DiagCodeID_ICD10 , DiagCodeID_ICD9
		if (ea.eaoDestType == eaoDiagnosis) {
			ea.diaDiagnosis.nDiagCodeID_ICD9  = AdoFldLong(fields, "DiagCodeID_ICD9", -1);
			ea.diaDiagnosis.nDiagCodeID_ICD10  = AdoFldLong(fields, "DiagCodeID_ICD10", -1);
		}
		else if(ea.eaoDestType == eaoCpt) {
			//If you get a BadVariableType on either of the next fields, you have bad data.  Every EMRActionsT record
			//	with a DestType of 1 must have a corresponding EMRActionChargeDataT record.
			//However, in anticipation of such a possible event, I've written this code to gracefully handle bad data, 
			//	you'll get the bad var type error, but the prompt and default qty will be set to their default values, 
			//	and program execution will continue to load the action.
			try {
				ea.bPrompt = AdoFldBool(fields, "Prompt", 0);
				ea.dblDefaultQuantity = AdoFldDouble(fields, "DefaultQuantity");
			} NxCatchAllCall("Error in LoadActionInfo : EMRActionChargeData", {	ea.bPrompt = FALSE;	ea.dblDefaultQuantity = 1.0; }	);
			ea.strMod1 = AdoFldString(fields, "Modifier1Number", "");
			ea.strMod2 = AdoFldString(fields, "Modifier2Number", "");
			ea.strMod3 = AdoFldString(fields, "Modifier3Number", "");
			ea.strMod4 = AdoFldString(fields, "Modifier4Number", "");
		}
		else if (ea.eaoDestType == eaoTodo) {
			// (c.haag 2008-06-04 11:06) - PLID 30221 - Load todo information
			ea.nDestID = ea.nID;
			ea.nTodoRemindType = AdoFldLong(fields, "RemindType");
			ea.nTodoRemindInterval = AdoFldLong(fields, "RemindInterval");
			ea.nTodoDeadlineType = AdoFldLong(fields, "DeadlineType");
			ea.nTodoDeadlineInterval = AdoFldLong(fields, "DeadlineInterval");
			ea.strTodoNotes = AdoFldString(fields, "Notes", "");
			ea.nTodoPriority = AdoFldByte(fields, "Priority");
			ea.strTodoMethod = AdoFldString(fields, "Task");
			ea.nTodoCategoryID = AdoFldLong(fields, "CategoryID", -1);
#pragma TODO("Performance concern -- creates one recordset per Todo destination action")
			FillEmrActionWithTodoAssignTos(pCon, ea);
		}

		// (j.jones 2007-01-22 10:55) - PLID 24356 - see if the action already exists,
		// and if it is deleted and the new one is not, replace it with the new version
		// if it is not deleted and the new one is deleted, ignore the new version
		ea.bDeleted = AdoFldBool(fields, "Deleted", FALSE);

		// (j.jones 2007-07-16 15:44) - PLID 26694 - added OnePerEmn to the struct
		ea.bOnePerEmn = AdoFldBool(fields, "OnePerEmn", FALSE);

		// (j.jones 2007-07-31 15:59) - PLID 26898 - added strSourceName
		ea.strSourceName = AdoFldString(fields, "SourceName", "");

		BOOL bAdd = TRUE;
		
		for(int i=0;i<arActions.GetSize();i++) {
			// (j.jones 2013-07-15 10:51) - PLID 57243 - changed to a reference
			const EmrAction& eaCompare = arActions.GetAt(i);
			if(eaCompare.eaoSourceType == ea.eaoSourceType
				&& eaCompare.nSourceID == ea.nSourceID
				&& eaCompare.eaoDestType == ea.eaoDestType
				&& eaCompare.nDestID == ea.nDestID) {

				// (r.gonet 09/12/2012) - PLID 51949 - Load all of the wound care conditions since they are not versioned.
				if(eaCompare.eaoSourceType != eaoWoundCareCodingCondition) {
					if(eaCompare.bDeleted && !ea.bDeleted) {

						//it is the same, and the existing action is deleted,
						//so simply overwrite the value in the array with the non-deleted one
						
						//(e.lally 2011-12-02) PLID 46838 - Need to actually replace the action in the array with the current one.
						//	Also, we were more or less acting like a copy constructor here, only with a few hand-picked values.
						//	Since this code was never updating the array like it intended to, I think it is safe to say 
						//	we want all the actions elements updated and not a hand-picked few.
						arActions.ElementAt(i) = ea;
					}

					// (b.savon 2014-07-22 15:27) - PLID 62711 - check the actual codes now if diagnosis type, not just the -1 destID
					if (eaCompare.eaoDestType != eaoDiagnosis ||
						(eaCompare.eaoDestType == eaoDiagnosis &&
						eaCompare.diaDiagnosis.nDiagCodeID_ICD9 == ea.diaDiagnosis.nDiagCodeID_ICD9 &&
						eaCompare.diaDiagnosis.nDiagCodeID_ICD10 == ea.diaDiagnosis.nDiagCodeID_ICD10)
					   ){
						//remember to not add this action now
						bAdd = FALSE;
					}
				}
			}
		}

		if(bAdd) {
			arActions.Add(ea);
		}
	}

	// (c.haag 2008-07-21 15:06) - PLID 30725 - Read problem actions
	_RecordsetPtr prsProblems = rsInfo->NextRecordset(NULL);
	FieldsPtr fields = prsProblems->Fields;
	while (!prsProblems->eof) {
		const long nEmrActionID = AdoFldLong(fields, "EmrActionID");
		// Search for the emr action
		for (int i=0; i < arActions.GetSize(); i++) {
			// (j.jones 2013-07-15 10:51) - PLID 57243 - changed to a reference
			EmrAction& ea = arActions.GetAt(i);
			if (ea.nID == nEmrActionID) {
				// Found the action. Add the problem information to it
				EmrProblemAction epa;
				epa.nID = AdoFldLong(fields, "ID");
				epa.bSpawnToSourceItem = AdoFldBool(fields, "SpawnToSourceItem");
				epa.nStatus = AdoFldLong(fields, "DefaultStatus");
				epa.strDescription = AdoFldString(fields, "DefaultDescription");
				epa.nSNOMEDCodeID = AdoFldLong(fields, "SNOMEDCodeID", -1); // (c.haag 2014-07-22) - PLID 62789
				epa.bDoNotShowOnCCDA = AdoFldBool(fields, "DoNotShowOnCCDA", FALSE);// (s.tullis 2015-03-05 15:35) - PLID 64724 - Added Do not show on CCDA 
				// (r.gonet 2015-03-10 14:48) - PLID 65013 - Load DoNotShowOnProblemPrompt from the recordset.
				epa.bDoNotShowOnProblemPrompt = AdoFldBool(fields, "DoNotShowOnProblemPrompt", FALSE);
				// (r.farnworth 2014-08-19 14:51) - PLID 62787 - Link the Diagnosis Code to the Problem
				epa.nDiagICD9CodeID = ea.diaDiagnosis.nDiagCodeID_ICD9;
				epa.nDiagICD10CodeID = ea.diaDiagnosis.nDiagCodeID_ICD10;
				ea.aProblemActions.push_back(epa);
				// (j.jones 2013-07-15 10:51) - PLID 57243 - not needed, this is now a reference
				//arActions.SetAt(i, ea);
				break;
			}
		}
		prsProblems->MoveNext();
	}
}

// (j.jones 2007-05-21 10:54) - PLID 26061 - added a batch parameter
// (j.jones 2013-07-22 16:35) - PLID 57277 - this now returns a Sql fragment
CSqlFragment DeleteEMRAction(long nActionID)
{
	CSqlFragment sql("");
	sql += CSqlFragment("UPDATE EMRTemplateTopicsT SET SourceActionID = NULL WHERE SourceActionID = {INT}; ", nActionID);
	sql += CSqlFragment("UPDATE EMRTemplateDetailsT SET SourceActionID = NULL WHERE SourceActionID = {INT}; ", nActionID);
	//DRT 2/6/2007 - PLID 24181 - Need to clear out charge specific actions if they exist.
	sql += CSqlFragment("DELETE FROM EMRActionChargeDataT WHERE ActionID = {INT}; ", nActionID);
	// (c.haag 2008-06-20 12:27) - PLID 30221 - Delete todo actions
	sql += CSqlFragment("DELETE FROM EMRActionsTodoDataT WHERE ActionID = {INT}; ", nActionID);
	sql += CSqlFragment("DELETE FROM EMRActionsTodoAssignToT WHERE ActionID = {INT}; ", nActionID);
	// (c.haag 2008-07-10 15:54) - PLID 30674 - Delete from the EMR todo action table (but don't delete the todo task itself). I think this
	// could happen if you create a new EMR item, add it to an EMN, spawn a todo, and delete the master item without having saved the EMN first.
	sql += CSqlFragment("DELETE FROM EMRTodosT WHERE SourceActionID = {INT}; ", nActionID);
	// (c.haag 2008-08-22 11:52) - PLID 30724 - EMR Problem Actions
	sql += CSqlFragment("DELETE FROM EMRProblemActionsT WHERE EmrActionID = {INT}; ", nActionID);
	// (z.manning 2008-10-06 14:07) - PLID 21094 - Just to be safe, make sure this action isn't referenced
	// in a lab.
	sql += CSqlFragment("UPDATE LabsT SET SourceActionID = NULL WHERE SourceActionID = {INT}; ", nActionID);
	// (a.walling 2014-08-18 14:33) - PLID 62681 - Laterality - handle database changes for referential integrity
	sql += CSqlFragment("DELETE FROM EmrActionAnatomicLocationQualifiersT WHERE EmrActionID = {INT}; ", nActionID);
	// (z.manning 2011-06-28 12:13) - PLID 44347 - Handle EmrActionAnatomicLocationsT
	sql += CSqlFragment("DELETE FROM EmrActionAnatomicLocationsT WHERE EmrActionID = {INT}; ", nActionID);
	// (b.savon 2014-07-21 17:03) - PLID 62707 - Handle saving the new Diagnosis DestType action to EmrActionsT and EmrActionsDiagnosisDataT
	sql += CSqlFragment("DELETE FROM EmrActionDiagnosisDataT WHERE EmrActionID = {INT}; ", nActionID);

	sql += CSqlFragment("DELETE FROM EMRActionsT WHERE ID = {INT}; ", nActionID);

	return sql;
}

// (z.manning 2009-03-17 11:30) - PLID 33242
BOOL IsDataIDUsedBySpawnedObject(const CString strDataIDClause)
{
	// (c.haag 2010-07-14 11:30) - PLID 39587 - Don't use "CASE WHEN EXISTS" with a bunch of unions inside it as below. It will dramatically slow the query down.
	ADODB::_RecordsetPtr prs = CreateRecordset(
		//"SELECT CASE WHEN EXISTS ( \r\n"
		"	SELECT ID FROM EmrChargesT WHERE SourceDataGroupID IN (SELECT EmrDataGroupID FROM EmrDataT WHERE EmrDataT.ID IN (%s)) \r\n"
		"	UNION \r\n"
		"	SELECT ID FROM EmrDetailsT WHERE SourceDataGroupID IN (SELECT EmrDataGroupID FROM EmrDataT WHERE EmrDataT.ID IN (%s)) \r\n"
		"	UNION \r\n"
		"	SELECT ID FROM EmrDiagCodesT WHERE SourceDataGroupID IN (SELECT EmrDataGroupID FROM EmrDataT WHERE EmrDataT.ID IN (%s)) \r\n"
		"	UNION \r\n"
		"	SELECT ID FROM EmrMasterT WHERE SourceDataGroupID IN (SELECT EmrDataGroupID FROM EmrDataT WHERE EmrDataT.ID IN (%s)) \r\n"
		"	UNION \r\n"
		"	SELECT MedicationID FROM EmrMedicationsT WHERE SourceDataGroupID IN (SELECT EmrDataGroupID FROM EmrDataT WHERE EmrDataT.ID IN (%s)) \r\n"
		"	UNION \r\n"
		"	SELECT ProcedureID FROM EmrProcedureT WHERE SourceDataGroupID IN (SELECT EmrDataGroupID FROM EmrDataT WHERE EmrDataT.ID IN (%s)) \r\n"
		"	UNION \r\n"
		"	SELECT ID FROM EmrTopicsT WHERE SourceDataGroupID IN (SELECT EmrDataGroupID FROM EmrDataT WHERE EmrDataT.ID IN (%s)) \r\n"
		//"	) THEN convert(bit, 1) ELSE convert(bit, 0) END AS InUse \r\n"
		, strDataIDClause, strDataIDClause, strDataIDClause, strDataIDClause, strDataIDClause, strDataIDClause, strDataIDClause);
	
	//return AdoFldBool(prs->GetFields(), "InUse");
	if (!prs->eof) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

CMap<long,long&,CString,CString&> g_mapActionIDsToNames;

// (j.jones 2007-07-31 09:43) - PLID 26882 - added optional parameters for source type and source ID
//TES 3/18/2010 - PLID 37530 - Added a parameter for the index of the spawning stamp, it's needed to calculate the name.
// (a.walling 2010-05-19 16:02) - PLID 38750 - Pass in the detail stamp ID
CString GetEmrActionName(long nActionID, long nSourceStampIndex, OPTIONAL IN EmrActionObject eaoSourceType /*= eaoInvalid*/, OPTIONAL IN long nSourceID /*= -1*/, OPTIONAL IN long nSourceDetailStampID /*= -1*/)
{
	CString strName;
	if(g_mapActionIDsToNames.Lookup(nActionID, strName)) {
		return strName;
	}
	else {

		// (j.jones 2007-07-31 09:47) - PLID 26882 - if we have both optional parameters, there's no need for a recordset,
		// but if we have one and not the other, we have to query
		if(eaoSourceType == eaoInvalid || nSourceID == -1) {
			//DRT 8/30/2007 - PLID 27259 - Parameterized.
			_RecordsetPtr rsAction = CreateParamRecordset("SELECT SourceType, SourceID FROM EmrActionsT WHERE ID = {INT}", nActionID);
			if(!rsAction->eof) {
				eaoSourceType = (EmrActionObject)AdoFldLong(rsAction, "SourceType");
				nSourceID = AdoFldLong(rsAction, "SourceID");
			}
			else {
				return "";
			}
			rsAction->Close();
		}

		switch(eaoSourceType) {
		case eaoEmrItem:
			return VarString(GetTableField("EmrInfoT","Name","ID",nSourceID),"");
			break;
		case eaoEmrDataItem:
			return VarString(GetTableField("EmrDataT","Data","ID",nSourceID),"");
			break;
		case eaoProcedure:
			return VarString(GetTableField("ProcedureT","Name","ID",nSourceID),"");
			break;
		// (c.haag 2006-03-06 08:50) - PLID 19562 - If you add any new cases to this
		// statement, you must edit CEMRTopic::LoadFromTopicID because it calculates
		// EMR action names by itself. You should also look for any other places in
		// EMRTopic.cpp, and in the program, too. Test carefully!
		case eaoEmrImageHotSpot:
			//DRT 1/23/2008 - PLID 28690 - Image hot spots don' thave names.  I'm leaving this as blank for now
			//	but I'm really not sure how this is used.  We may need to change it.
			// (z.manning, 02/22/2008) - PLID 28690 - Action names are used in a few places such as when 2
			// different things spawn the same topics. Since hot spots do not yet have names there's really
			// nothing we can do. I started to implement this to use just the info item's name, but realized
			// from a user's perspective that giving the name of the item that spawned instead of the specific
			// spot really isn't useful.  I entered 29073 to one day give hot spots a name.
			//TES 2/16/2010 - PLID 37298 - HotSpots now have Anatomic Locations, we can use those as the name.
			{
				_RecordsetPtr rsSpotName = CreateParamRecordset("SELECT LabAnatomyT.Description AS Location, AnatomyQualifiersT.Name AS Qualifier, EmrImageHotSpotsT.AnatomySide "
					"FROM EmrImageHotSpotsT LEFT JOIN LabAnatomyT ON EmrImageHotSpotsT.AnatomicLocationID = LabAnatomyT.ID "
					"LEFT JOIN AnatomyQualifiersT ON EmrImageHotSpotsT.AnatomicQualifierID = AnatomyQualifiersT.ID "
					"WHERE EmrImageHotSpotsT.ID = {INT}", nSourceID);
				if(rsSpotName->eof) {
					return "";
				}
				else {
					CString strLocation = AdoFldString(rsSpotName, "Location", "");
					CString strQualifier = AdoFldString(rsSpotName, "Qualifier", "");
					AnatomySide as = (AnatomySide)AdoFldLong(rsSpotName, "AnatomySide", asNone);
					// (z.manning 2010-04-30 17:36) - PLID 37553 - We now have a function to format this
					return ::FormatAnatomicLocation(strLocation, strQualifier, as);
				}
			}
			break;
		// (z.manning 2009-02-10 15:34) - PLID 33026 - Added EMR table dropdown item based spawning
		case eaoEmrTableDropDownItem:
			return VarString(GetTableField("EMRTableDropdownInfoT", "Data", "ID", nSourceID), "");
			break;
		case eaoSmartStamp: // (z.manning 2010-02-15 11:07) - PLID 37226
			// (a.walling 2010-05-19 15:26) - PLID 38750 - Index is only valid on templates.
			if (nSourceStampIndex != -1) {			
				//TES 3/18/2010 - PLID 37530 - Append the index of the spawning stamp.
				return VarString(GetTableField("EMRImageStampsT", "TypeName", "ID", nSourceID), "") + " - " + AsString((long)nSourceStampIndex);
			} else if (nSourceDetailStampID != -1) {
				// (a.walling 2010-05-19 15:15) - PLID 38750 - The index is not always loaded. We could load the index, but it is only used for templates.
				// However we have all the information we need to handle it here.
				_RecordsetPtr prsStampSpawning = CreateParamRecordset(
					"SELECT EMRImageStampsT.TypeName, EmrDetailImageStampsT.OrderIndex "
					"FROM EmrDetailImageStampsT "
					"INNER JOIN EMRImageStampsT ON EmrDetailImageStampsT.EmrImageStampID = EMRImageStampsT.ID "
					"WHERE EmrDetailImageStampsT.ID = {INT}", nSourceDetailStampID);

				if (!prsStampSpawning->eof) {
					return FormatString("%s - %li", AdoFldString(prsStampSpawning, "TypeName", ""), AdoFldLong(prsStampSpawning, "OrderIndex", -1));
				} else {
					ASSERT(FALSE);
				}
			} else {
				// (a.walling 2010-05-19 16:05) - PLID 38750 - Invalid state; should have one or the other
				ASSERT(FALSE);
			}
			return "";
			break;
		// (r.gonet 08/03/2012) - PLID 51949 - Added for Wound Care Condition generated actions.
		case eaoWoundCareCodingCondition:
			return VarString(GetTableField("WoundCareConditionT","Name","ConditionID",nSourceID),"");
			break;

		default:
			ASSERT(FALSE);
			return "";
		}
	}
}

// (z.manning 2009-02-11 09:04) - PLID 33029 - Set the fields of an EmrAction from the given fields pointer.
void SetActionFromFields(IN ADODB::FieldsPtr pflds, OUT EmrAction &ea)
{
	FieldPtr fldID = pflds->GetItem("ID");
	FieldPtr fldSourceType = pflds->GetItem("SourceType");
	FieldPtr fldSourceID = pflds->GetItem("SourceID");
	FieldPtr fldDestType = pflds->GetItem("DestType");
	FieldPtr fldDestID = pflds->GetItem("DestID");
	FieldPtr fldSortOrder = pflds->GetItem("SortOrder");
	FieldPtr fldPopup = pflds->GetItem("Popup");
	FieldPtr fldSpawnAsChild = pflds->GetItem("SpawnAsChild");
	ea.nID = AdoFldLong(fldID);
	ea.eaoSourceType = (EmrActionObject)AdoFldLong(fldSourceType);
	ea.nSourceID = AdoFldLong(fldSourceID);
	ea.eaoDestType = (EmrActionObject)AdoFldLong(fldDestType);
	ea.nDestID = AdoFldLong(fldDestID);
	ea.nSortOrder = AdoFldLong(fldSortOrder);
	ea.bPopup = AdoFldBool(fldPopup);
	ea.bSpawnAsChild = AdoFldBool(fldSpawnAsChild);
	//DRT 1/10/2007 - PLID 24181 - We now have a separate table for extra charge information.  This only
	//	applies to eaoCpt type items.
	if(ea.eaoDestType == eaoCpt) {
		//If you get a BadVariableType on either of the next fields, you have bad data.  Every EMRActionsT record
		//	with a DestType of 1 must have a corresponding EMRActionChargeDataT record.
		//However, in anticipation of such a possible event, I've written this code to gracefully handle bad data, 
		//	you'll get the bad var type error, but the prompt and default qty will be set to their default values, 
		//	and program execution will continue to load the action.
		try {
			ea.bPrompt = AdoFldBool(pflds, "Prompt", 0);
			ea.dblDefaultQuantity = AdoFldDouble(pflds, "DefaultQuantity");
		} NxCatchAllCall("Error in LoadActionInfo : EMRActionChargeData", {	ea.bPrompt = FALSE;	ea.dblDefaultQuantity = 1.0; }	);
		ea.strMod1 = AdoFldString(pflds, "Modifier1Number", "");
		ea.strMod2 = AdoFldString(pflds, "Modifier2Number", "");
		ea.strMod3 = AdoFldString(pflds, "Modifier3Number", "");
		ea.strMod4 = AdoFldString(pflds, "Modifier4Number", "");
	}
	else if (ea.eaoDestType == eaoTodo) {
		// (c.haag 2008-06-04 11:27) - PLID 30221 - Load EMR todo action data
		ea.nDestID = ea.nID;
		ea.nTodoRemindType = AdoFldLong(pflds, "RemindType", -2);
		ea.nTodoRemindInterval = AdoFldLong(pflds, "RemindInterval", -2);
		ea.nTodoDeadlineType = AdoFldLong(pflds, "DeadlineType", -2);
		ea.nTodoDeadlineInterval = AdoFldLong(pflds, "DeadlineInterval", -2);
		ea.strTodoNotes = AdoFldString(pflds, "Notes", "");
		ea.nTodoPriority = AdoFldByte(pflds, "Priority", -2);
		ea.strTodoMethod = AdoFldString(pflds, "Task", "");
		ea.nTodoCategoryID = AdoFldLong(pflds, "CategoryID", -2);
		if (ea.nTodoRemindType > -2) {
			FillEmrActionWithTodoAssignTos(GetRemoteData(), ea);
		} else {
			// If we get here, it can only mean that this is not a todo action because RemindType cannot be NULL in data.
			// So, don't bother trying to load AssignTo values.
		}
	}
	//(s.dhole 7/17/2014 4:24 PM ) - PLID 62724 Added DiagCodeID_ICD10 , DiagCodeID_ICD9
	else if (ea.eaoDestType == eaoDiagnosis) {
		ea.diaDiagnosis.nDiagCodeID_ICD9 = AdoFldLong(pflds, "DiagCodeID_ICD9", -1);
		ea.diaDiagnosis.nDiagCodeID_ICD10 = AdoFldLong(pflds, "DiagCodeID_ICD10", -1);
	}


	// (j.jones 2007-07-16 16:16) - PLID 26694 - bOnePerEmn is a value of EmrAction for the
	// purposes of efficiency, but is not a property of the action, and isn't needed here
	// for future saving purposes.
}

void GetEmrItemCategories(IN long nEmrInfoID, OUT CArray<long,long> &arCategoryIDs)
{
	_RecordsetPtr rsCategories = CreateRecordset("SELECT EmrCategoryID FROM EmrInfoCategoryT WHERE EmrInfoID = %li", nEmrInfoID);
	while(!rsCategories->eof) {
		arCategoryIDs.Add(AdoFldLong(rsCategories, "EmrCategoryID"));
		rsCategories->MoveNext();
	}
}

BOOL ValidateEMRTimestamp(long nEMRID)
{
	//check the server time, and check the last audit time
	//for any object on this EMR, then give a warning and
	//return FALSE if they have changed the server time

	//we audit by EMN, not by EMR, so we need to validate per EMN,
	//but in the event that multiple EMNs in EMR fail the timestamp check,
	//we'll only warn once, thus we only need to select the first offending EMN
	//DRT 8/30/2007 - PLID 27259 - Parameterized.
	_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 Description, ModifiedDate, GetDate() AS CurDate "
		"FROM EMRMasterT WHERE Deleted = 0 AND ModifiedDate > GetDate() AND EMRGroupID = {INT}", nEMRID);
	if(!rs->eof) {		

		//we have a problem - the server time is before the last audit time!
		
		CString str;
		CString strDescription = AdoFldString(rs, "Description","");
		COleDateTime dtMaxAuditDate = AdoFldDateTime(rs, "ModifiedDate");
		COleDateTime dtCurServerDate = AdoFldDateTime(rs, "CurDate");

		str.Format("You cannot access this record because the EMN '%s'\n"
				   "was last modified on %s but the current server time is %s\n\n"
				   "For security purposes, you may not access any EMN if there is a discrepancy between the\n"
				   "last modified time and the current time.",
				   strDescription, FormatDateTimeForInterface(dtMaxAuditDate, NULL, dtoDateTime),
				   FormatDateTimeForInterface(dtCurServerDate, NULL, dtoDateTime));

		AfxMessageBox(str);
		return FALSE;
	}
	rs->Close();

	return TRUE;
}

BOOL ValidateEMNTimestamp(long nEMNID)
{
	//check the server time, and check the last audit time
	//for any object on this EMN, then give a warning and
	//return FALSE if they have changed the server time

	// (j.jones 2006-02-23 10:40) - PLID 19431 - we put the last audit time in EMRMasterT.ModifiedDate,
	// so just go off of that date

	_RecordsetPtr rs = CreateRecordset("SELECT Description, ModifiedDate, GetDate() AS CurDate "
		"FROM EMRMasterT WHERE ModifiedDate > GetDate() AND ID = %li", nEMNID);

	if(!rs->eof) {

		//we have a problem - the server time is before the last audit time!
		
		CString str;
		CString strDescription = AdoFldString(rs, "Description","");
		COleDateTime dtMaxAuditDate = AdoFldDateTime(rs, "ModifiedDate");
		COleDateTime dtCurServerDate = AdoFldDateTime(rs, "CurDate");

		str.Format("You cannot access this record because the EMN '%s'\n"
				   "was last modified on %s but the current server time is %s\n\n"
				   "For security purposes, you may not access any EMN if there is a discrepancy between the\n"
				   "last modified time and the current time.",
				   strDescription, FormatDateTimeForInterface(dtMaxAuditDate, NULL, dtoDateTime),
				   FormatDateTimeForInterface(dtCurServerDate, NULL, dtoDateTime));

		AfxMessageBox(str);
		return FALSE;
	}
	rs->Close();
	
	return TRUE;
}

void EnsureCompletelyLoadedOrThrowException(EmrSaveObjectType esotSaveType, CEMN* pEMN)
{
	// (c.haag 2007-08-27 08:58) - PLID 27185 - Ensure that the initial load is done. Throws an exception
	// if we cannot ensure this.
	if(pEMN) {
		// Check to see whether the initial load is in progress
		if (pEMN->IsLoading()) {
			// If we get here, it is still in progress. The calling code should have ensured that
			// this can't happen. Regardless, force it to finish.
			ASSERT(FALSE);
			pEMN->EnsureCompletelyLoaded();
			if (pEMN->IsLoading()) {
				// This should not happen unless EnsureCompletelyLoaded doesn't work.
				switch (esotSaveType) {
				case esotTopic:
					ThrowNxException("Attempted to save a topic while its parent EMN was still loading!");
					break;
				case esotEMN:
				case esotEMR:
				default:
					ThrowNxException("Attempted to save an EMN while it was still loading!");
					break;
				}
			}
		}
	} else {
		ThrowNxException("Called EnsureCompletelyLoadedOrThrowException with a NULL EMN!");
	}
}


EmrSaveStatus SaveEMRObject(EmrSaveObjectType esotSaveType, long nObjectPtr, BOOL bShowProgressBar)
{
	BOOL bDrugInteractionsChanged;
	CDWordArray arNewCDSInterventions;
	return SaveEMRObject(esotSaveType, nObjectPtr, bShowProgressBar, bDrugInteractionsChanged, arNewCDSInterventions);
}

// (j.jones 2012-09-27 15:11) - PLID 52820 - now we track a flag if something that contributed to drug interactions has changed,
// such as new or deleted prescriptions, or new or deleted diagnosis codes
//TES 10/31/2013 - PLID 59251 - Added arNewCDSInterventions, any interventions triggered in this function will be added to it
EmrSaveStatus SaveEMRObject(EmrSaveObjectType esotSaveType, long nObjectPtr, BOOL bShowProgressBar, OUT BOOL &bDrugInteractionsChanged, IN OUT CDWordArray &arNewCDSInterventions)
{
	CNxPerform nxp(__FUNCTION__);
	long nAuditTransactionID = -1;

	// (a.walling 2008-06-26 15:42) - PLID 30513 - Better handling of save errors due to multi-user EMN concurrency
	BOOL bFailedConcurrency = FALSE;

	CShowProgressFeedbackDlg *pProgressDlg = NULL;

	try {
		CWaitCursor pWait;

		long nCurrentProgress = 0;
		long nProgressMax = 100;

		// (a.walling 2007-09-07 09:00) - PLID 24371 - Get a parent window for the progress bar
		CWnd* pInterfaceWnd = NULL;
		CEMR *pEMR = NULL;
		switch(esotSaveType) {
			case esotTopic: {
				CEMRTopic *pTopic = (CEMRTopic*)nObjectPtr;
				pEMR = pTopic->GetParentEMN()->GetParentEMR();
				pInterfaceWnd = pEMR->GetInterface();
				break;
			}
			case esotEMN: {
				CEMN *pEMN = (CEMN*)nObjectPtr;
				pEMR = pEMN->GetParentEMR();
				pInterfaceWnd = pEMR->GetInterface();
				break;
			}
			case esotEMR:
			default: {
				pEMR = (CEMR*)nObjectPtr;
				pInterfaceWnd = pEMR->GetInterface();
				break;
			}
		}

		// (z.manning 2009-05-21 12:20) - PLID 34297 - Now that EMR problems can be associated with multiple
		// objects, if their are new problems anywhere in the current EMR, we silently force them to save
		// the entire EMR. The reason for this is that problems are no longer directly tied to an object.
		// Now we just have global problems that can be tied to one or more objects. So first off, 
		// we need to ensure all problems have been saved to data before saving anything that can be linked
		// to a problem in order to properly propagate the problem ID in the problem linking table. The same
		// problem could potentially be linked to different objects in different topics or even different EMNs.
		// Also, I think this makes more practically because let's say for example you edit a problem linked
		// to an item from that item. You then just save that topic. You then save just that topic but since
		// the problem isn't tied to a single object it would not save the problem even though it's linked to
		// something we did save. So by saving the entire EMR we avoid that potentially odd situation.
		//TES 6/3/2009 - PLID 34371 - As it happens, the wellness code will also need to know whether there were
		// problems changed.
		bool bUpdateWellnessForProblems = false;
		if(pEMR->DoesEmrOrChildrenHaveUnsavedProblems()) {
			esotSaveType = esotEMR;
			nObjectPtr = (long)pEMR;
			bUpdateWellnessForProblems = true;
		}

		if(esotSaveType == esotTopic)
		{
			CEMRTopic *pTopic = (CEMRTopic*)nObjectPtr;
			//TES 4/13/2012 - PLID 49482 - Likewise don't save a single topic if it contains a smart stamp image
			if(pTopic->ContainsSmartStampTable() || pTopic->ContainsSmartStampImage()) {
				// (z.manning 2011-03-02 11:40) - PLID 42638 - We do not currently support saving a single topic if
				// that topic contains a smart stamp table.
				CEMN *pEmn = pTopic->GetParentEMN();
				if(pEmn != NULL) {
					esotSaveType = esotEMN;
					nObjectPtr = (long)pEmn;
				}
			}
		}

		if(bShowProgressBar) {
			// (a.walling 2007-09-07 08:59) - PLID 24371 - send a valid parent window.
			pProgressDlg = new CShowProgressFeedbackDlg(1000, TRUE, FALSE, FALSE, pInterfaceWnd == NULL ? NULL : pInterfaceWnd->GetSafeHwnd());

			pProgressDlg->SetProgress(0, nProgressMax, nCurrentProgress);
			pProgressDlg->SetCaption("Saving EMR Changes - Preparing to Save...");
		}

		nxp.Tick("Preparing batch...");

		// (a.walling 2014-01-30 00:00) - PLID 60547 - Quantize EMR saving
		Nx::Quantum::Batch strSaveString;

		//used to track any errors that may occur while saving,
		//should only be filled when we try to write to locked EMNs
		CStringArray arystrErrors;
		//Used by the save code to prevent duplicated saving.
		// (c.haag 2007-06-20 12:14) - PLID 26397 - We now store saved objects in a map for fast lookups
		CMapPtrToPtr mapSavedObjects;

		// (a.walling 2007-04-17 12:28) - PLID 25454 - Keep track of EMNs which are unsaved and may need to have their preview regenerated.
		CMapPtrToWord mapUnsavedEMNs;
		CMapPtrToWord mapUpdatedEMNs; // keeps track of EMNs which have had IDs updated via PropagateNewID

		// (j.jones 2007-01-11 14:34) - PLID 24027 - the PostSaveSql is used by the save code
		// to handle statements that must occur only after the save is complete,
		// ie. once all the objects have been entered into the database and our #NewObjectsT
		// is filled out with the object IDs
		// (a.walling 2014-01-30 00:00) - PLID 60547 - Quantize EMR saving
		Nx::Quantum::Batch strPostSaveSql;

		// (j.jones 2006-02-08 10:12) - PLID 18946 - to support initializing a save from an object
		// other than the EMR as a whole, we can call GenerateSaveString from that object,
		// with TopLevelSave set to true
		// (b.cardillo 2009-06-03 13:44) - PLID 34370 - Collect a list of EMRDetailIDs that are being deleted
		CDWordArray arynAffectedDetailIDs;
		switch(esotSaveType) {
			case esotTopic: {
				CEMRTopic *pTopic = (CEMRTopic*)nObjectPtr;
				if(pTopic) {
					// (c.haag 2007-08-27 09:09) - PLID 27185 - Do not let Practice save a topic if its parent's
					// initial load is still in progress. This will throw an exception if we could not stop the
					// initial load.
					EnsureCompletelyLoadedOrThrowException(esotSaveType, pTopic->GetParentEMN());
					// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs
					strSaveString = pTopic->GenerateSaveString(nAuditTransactionID, strPostSaveSql, arystrErrors, mapSavedObjects, arynAffectedDetailIDs, bDrugInteractionsChanged, TRUE, FALSE);
				}
				else {
					ASSERT(FALSE);

					if(pProgressDlg) {
						delete pProgressDlg;
						pProgressDlg = NULL;
					}

					return essFailed;
				}
				break;
			}
			case esotEMN: {
				CEMN *pEMN = (CEMN*)nObjectPtr;
				if(pEMN) {
					// (c.haag 2007-08-27 09:09) - PLID 27185 - Do not let Practice save a topic if its parent's
					// initial load is still in progress.
					EnsureCompletelyLoadedOrThrowException(esotSaveType, pEMN);
					// (a.walling 2007-04-17 12:31) - 25454 - Save a reference to our unsaved EMN
					if (pEMN->IsUnsaved()) {
						mapUnsavedEMNs.SetAt((void*)pEMN, 0);
					}
					// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs
					strSaveString = pEMN->GenerateSaveString(pEMR->GetID(), nAuditTransactionID, strPostSaveSql, arystrErrors, mapSavedObjects, arynAffectedDetailIDs, bDrugInteractionsChanged, TRUE, FALSE);
				}
				else {
					ASSERT(FALSE);

					if(pProgressDlg) {
						delete pProgressDlg;
						pProgressDlg = NULL;
					}

					return essFailed;
				}
				break;
			}
			case esotEMR:
			default:
				// (a.walling 2007-04-17 12:31) - 25454 - Save a reference to our unsaved EMNs
				if (pEMR) {
					for (int i = 0; i < pEMR->GetEMNCount(); i++) {
						CEMN* pEMN = pEMR->GetEMN(i);
						if (pEMN) {
							// (c.haag 2007-08-27 09:09) - PLID 27185 - Do not let Practice save a topic if its parent's
							// initial load is still in progress.
							EnsureCompletelyLoadedOrThrowException(esotSaveType, pEMN);
						}
						if (pEMN && pEMN->IsUnsaved()) {
							mapUnsavedEMNs.SetAt((void*)pEMN, 0);
						}
					}
				}

				// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs
				strSaveString = pEMR->GenerateSaveString(nAuditTransactionID, strPostSaveSql, arystrErrors, mapSavedObjects, arynAffectedDetailIDs, bDrugInteractionsChanged, FALSE);
				break;				
		}

		nxp.Tick("Got save batch");

		// (j.jones 2007-01-11 14:36) - PLID 23027 - append strPostSaveSql to the strSaveString
		if(!strPostSaveSql.IsEmpty()) {
			strSaveString += strPostSaveSql;
		}

		nxp.Tick("Appended post-save batch");

		/*
		CMsgBox msg;
		msg.msg = strSaveString;
		msg.DoModal();
		*/

		if(pProgressDlg) {
			nCurrentProgress = 45;
			pProgressDlg->SetProgress(0, nProgressMax, nCurrentProgress);
			pProgressDlg->SetCaption("Saving EMR Changes...");
		}

		// (j.jones 2006-08-24 08:56) - PLID 22183 - if the arystrErrors has anything in it,
		// warn the user, log the errors, and then allow them to continue saving if they so desire
		if(arystrErrors.GetSize() > 0) {
			
			CString strWarning = "Some of your changes will not be saved, please review the following warnings:\n\n";

			Log("***Errors occurred saving an EMN***");
			Log("The messages returned were:");

			for(int i=0;i<arystrErrors.GetSize();i++) {
				strWarning += arystrErrors.GetAt(i);
				Log(arystrErrors.GetAt(i));
				strWarning += "\n";
			}

			Log("***Error list complete***");

			// (a.walling 2008-08-22 16:39) - PLID 30513 - Updated the wording to this message (if you _choose no_)
			strWarning += "\nYou can still save your remaining changes, and disregard the attempted changes listed here.\n"
				"Do you still wish to save the other changes?\n"
				"(If you choose 'No' and are closing the EMR, the EMR will not close.)";

			//remove the progress bar before warning
			if(pProgressDlg) {
				delete pProgressDlg;
				pProgressDlg = NULL;
			}
			
			if(IDNO == MessageBox(GetActiveWindow(), strWarning, "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
				RollbackAuditTransaction(nAuditTransactionID); // (a.walling 2008-07-07 14:26) - PLID 30513 - Memory leak
				return essFailed;
			} else {
				// (a.walling 2008-07-07 12:57) - PLID 30513 - They are OK with this, so we will set the ignore flag
				pEMR->SetIgnoreReadOnly(TRUE);
			}

			//if we continue, recreate the progress bar where we left off
			if(bShowProgressBar) {
				// (a.walling 2007-09-07 09:03) - PLID 24371 - send a valid parent window.
				pProgressDlg = new CShowProgressFeedbackDlg(1000, TRUE, FALSE, FALSE, pInterfaceWnd == NULL ? NULL : pInterfaceWnd->GetSafeHwnd());

				nCurrentProgress = 45;
				pProgressDlg->SetProgress(0, nProgressMax, nCurrentProgress);
				pProgressDlg->SetCaption("Saving EMR Changes...");
			}
		}

		// (j.jones 2006-04-11 14:51) - nothing is changing
		if(strSaveString.IsEmpty()) {

			if(pProgressDlg) {
				delete pProgressDlg;
				pProgressDlg = NULL;
			}

			return essSuccess;
		}

		// now run the statement (normally we'd call ExecuteSqlBatch() but we need the returned 
		// recordset and we want no recordsets for the non-select statements, so we put it in a transactiion 
		// and add the select statement ourselves, and call CreateRecordset())

		// (c.haag 2006-03-27 14:59) - PLID 19860 - We already know that this query is slow, so 
		// we need to suppress it from the log so that it's not cluttered with EMR saving queries.
		// (a.walling 2008-05-29 16:06) - PLID 22049 - Added code to update our internal revision
		// (z.manning 2012-09-11 14:32) - PLID 52543 - Added modified date to these queries
		CString strRevisionString;
		if (pEMR->GetIsTemplate()) {
			long nCount = pEMR->GetEMNCount();
			ASSERT(nCount == 1);
			if (nCount == 1) {
				strRevisionString.Format("SELECT ID, Revision, ModifiedDate FROM EMRTemplateT WHERE ID = %s;\r\n"
					, pEMR->GetEMN(0)->GetID() == -1 ? "@nEMRTemplateID" : AsString(pEMR->GetEMN(0)->GetID()));
			}
		}
		else {
			strRevisionString.Format("SELECT ID, Revision, ModifiedDate FROM EMRMasterT WHERE EMRGroupID = %s;\r\n"
				, pEMR->GetID() == -1 ? "@nEMRGroupID" : AsString(pEMR->GetID()));
		}
		// (z.manning 2009-10-27 17:13) - PLID 36064 - If you ever add any more SELECT queries to the end
		// of this batch then make sure you disconnect them from the connection right away the same
		// way that prsEMRResults and prsRevision do.

		// (a.walling 2014-01-30 00:00) - PLID 60547 - Prepare final batch
		// (a.walling 2014-05-01 15:29) - PLID 62008 - Optimized 'quantum' EMR save batches may potentially not rollback a transaction or partially commit a transaction due to RAISERROR not causing the batch to abort processing.
		Nx::Quantum::Batch finalBatch;
		finalBatch += GenerateEMRBatchSaveNewObjectTableDeclaration();
		finalBatch.AddRaw("SET NOCOUNT ON");
		finalBatch.AddRaw("BEGIN TRY"); // (a.walling 2014-05-01 15:29) - PLID 62008 - begin sql try / catch block
		finalBatch.AddRaw("BEGIN TRAN");
		finalBatch += strSaveString;
		finalBatch.AddRaw("COMMIT TRAN");
		finalBatch.AddRaw("SET NOCOUNT OFF");
		// (a.walling 2014-01-30 00:00) - PLID 60541 - #NewObjectsT now a table so can be referenced by other sprocs
		finalBatch.AddStatement("SELECT ID, Type, ObjectPtr FROM #NewObjectsT ORDER BY InsertOrder ASC"); // (a.walling 2013-04-17 16:46) - PLID 55652 - order of insertion is required on output
		finalBatch.AddStatement(strRevisionString);
		finalBatch.AddRaw("IF OBJECT_ID('tempdb..#NewObjectsT') IS NOT NULL DROP TABLE #NewObjectsT"); 
		// (a.walling 2014-05-01 15:29) - PLID 62008 - Ensure the transaction is rolled back and rethrow the exception.
		finalBatch.AddRaw("END TRY");
		finalBatch.AddRaw(
			"BEGIN CATCH \r\n"
			"IF @@TRANCOUNT > 0 BEGIN \r\n"
			"ROLLBACK TRANSACTION\r\n"
			"END\r\n"
			"SET NOCOUNT OFF\r\n"
			"DECLARE @errNumber INT, @errMessage NVARCHAR(4000), @errSeverity INT, @errState INT, @errProc NVARCHAR(128), @errLine INT \r\n"
			"SET @errNumber = ERROR_NUMBER(); SET @errMessage = ERROR_MESSAGE(); SET @errSeverity = ERROR_SEVERITY(); SET @errState = ERROR_STATE(); SET @errProc = ERROR_PROCEDURE(); SET @errLine = ERROR_LINE(); \r\n"
			"RAISERROR(N'Caught error %d from line %d in ''%s'': %s', @errSeverity, @errState, @errNumber, @errLine, @errProc, @errMessage) \r\n"
			"RETURN \r\n"
			"END CATCH"
		);

		nxp.Tick("Preparing final batch");
	

		//DRT 7/15/2008 - PLID 30743 (for 27964) - Since we cannot come up with any replication of the problem with the 
		//	triggers fire for updating locked EMNs, I am adding some logging that may help pin things down
		//	further.  If we get any failure message, then we will log the entire query to the log
		//	file.  We can then retrieve that and see if everyone is having the same problem or different ones.
		//If we solve this, you can remove the try/catch and everything in the catch here to go back to normal.
		_RecordsetPtr prsEMRResults = NULL;
		// (c.haag 2016-06-10 10:54) - PLID-66502 - TRUE if we logged EMR object data for objects we're saving
		BOOL bLoggedEMRObjectData = FALSE;

		nxp.Tick("Ready to execute");
		{
			// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
			NxAdo::PushPerformanceWarningLimit ppw(-1);

			try
			{
#if _DEBUG
				// (c.haag 2016-06-10 10:54) - PLID-66502 - If we're in debug mode, log the state of all related EMR objects to NxLog
				LogEmrObjectData(esotSaveType, nObjectPtr, finalBatch);
				bLoggedEMRObjectData = TRUE;
#endif
				// (a.walling 2014-02-05 11:23) - PLID 60546 - Execute the flattened batch according to UseAutoQuantumBatch, log if any errors
				prsEMRResults = ExecuteWithLog(finalBatch);
			}
			catch (...)
			{
				// (c.haag 2016-06-10 10:54) - PLID-66502 - Log the state of all related EMR objects to NxLog so we can get a better
				// picture of how we got the exception
				if (!bLoggedEMRObjectData)
				{
					LogEmrObjectData(esotSaveType, nObjectPtr, finalBatch);
					bLoggedEMRObjectData = TRUE;
				}
				throw;
			}
		}

		nxp.Tick("Exec complete!");
		
		// (z.manning 2009-10-27 17:06) - PLID 36064 - Let's go ahead and disconnect these recordsets
		// from the connection. This way if something happens to the connection during the massive
		// amount of EMR post save processing then these recordsets will still be perfectly valid.
		_RecordsetPtr prsRevision = prsEMRResults->NextRecordset(NULL);
		prsEMRResults->putref_ActiveConnection(NULL);
		prsRevision->putref_ActiveConnection(NULL);

		// (j.jones 2006-05-26 11:46) - really, this is the only significant wait in the save process,
		// so we previously showed the bar at 45, and then show 75 now, in order to give a good visual
		// effect. In many cases, the bar will disappear extremely soon after this step anyways,
		// with the exception being that sometimes the tracking changes are slow.
		if(pProgressDlg) {
			nCurrentProgress = 70;
			pProgressDlg->SetProgress(0, nProgressMax, nCurrentProgress);
			pProgressDlg->SetCaption("Saving EMR Changes - Updating Saved Status...");
		}

		//
		// (c.haag 2007-01-25 10:20) - PLID 24396 - Before we reset all the modified flags for all
		// the details being saved, we need to determine whether or not a Current Medications detail
		// has changed. If it has, then we will call PostSaveUpdateCurrentMedications later on to
		// notify the user if there are discrepancies between the most recently saved EMN and the
		// patient's current medications list.
		//
		// (c.haag 2007-04-06 09:38) - PLID 25525 - The same applies to allergies
		//
		// (j.jones 2009-09-21 10:37) - PLID 35367 - we will not update current meds or allergies
		// on the patient account unless this EMN is the latest EMN on the patient's account
		// (simply defined as there being no EMN with a later date)
		// so I renamed these booleans slightly
		BOOL bTryPostSaveUpdateCurMeds = FALSE;
		BOOL bTryPostSaveUpdateAllergies = FALSE;
		//DRT 8/15/2007 - PLID 26527 - We don't want to find out what EMR this detail is on just
		//	to look it up later -- give us the detail!  See more expanded comments in CEMR::PostSaveUpdateCurrentMedications()
		CEMNDetail *pPostSaveCurMedsDetail = NULL;
		// (c.haag 2007-08-15 18:07) - PLID 25525 - We don't want to find out what the Allergies detail
		// is now only to look it up later. Additional comments in CEMR::PostSaveUpdateCurrentAllergies
		// (and also CEMR::PostSaveUpdateCurrentMedications() where Don gives a more thorough explanation)
		CEMNDetail* pPostSaveAllergiesDetail = NULL;

		if(!pEMR->GetIsTemplate()) {

			switch(esotSaveType) {
				case esotTopic: {
					// (a.walling 2007-04-17 11:27) - PLID 25454 - If a topic is being saved, set NeedToSavePreview
					// to true. That way when closing, if the user decides not to save changes, we know whether we
					// at least need to generate a preview for the saved objects.
					CEMRTopic *pTopic = (CEMRTopic*)nObjectPtr;

					if (pTopic && pTopic->GetParentEMN()) {
						pTopic->GetParentEMN()->SetNeedToSavePreview(TRUE);
					}

					// (c.haag 2007-02-09 09:36) - If we are saving a topic, check if this topic
					// or any of its parents have a modified official Current Medications item. If so, 
					// then we need to do our post-save Current Medications processing.
					
					if(pTopic && !pTopic->IsTemplate()) {

						// (j.jones 2007-07-24 09:27) - PLID 26742 - the medications info ID is cached in CEMR
						long nActiveCurrentMedicationsInfoID = -2;
						//do memory checks
						if(pTopic->GetParentEMN()) {
							if(pTopic->GetParentEMN()->GetParentEMR()) {
								nActiveCurrentMedicationsInfoID = pTopic->GetParentEMN()->GetParentEMR()->GetCurrentMedicationsInfoID();
							}
						}

						if(nActiveCurrentMedicationsInfoID == -2) {
							//should only remain -2 if we have no EMR (-1 is bad data, but indicative that we did perform the check),
							//but why don't we have an EMR?
							ASSERT(FALSE);
							nActiveCurrentMedicationsInfoID = GetActiveCurrentMedicationsInfoID();
						}

						while (pTopic && !bTryPostSaveUpdateCurMeds) {
							const unsigned long nDetails = pTopic->GetEMNDetailCount();
							for (unsigned long n=0; n < nDetails && !bTryPostSaveUpdateCurMeds; n++) {
								CEMNDetail* pDetail = pTopic->GetDetailByIndex(n);
								if (nActiveCurrentMedicationsInfoID == pDetail->m_nEMRInfoID && pDetail->IsModified()) {
									bTryPostSaveUpdateCurMeds = TRUE;
									//DRT 8/15/2007 - PLID 26527 - We don't want to find out what EMR this detail is on just
									//	to look it up later -- give us the detail!
									pPostSaveCurMedsDetail = pDetail;
								}
							}

							pTopic = pTopic->GetParentTopic();
						}

						// (c.haag 2007-04-06 09:40) - PLID 25525 - Same with Allergies

						pTopic = (CEMRTopic*)nObjectPtr;

						// (j.jones 2007-07-24 09:27) - PLID 26742 - the Allergy Info ID is cached in CEMR
						long nActiveCurrentAllergiesInfoID = -2;
						//do memory checks
						if(pTopic->GetParentEMN()) {
							if(pTopic->GetParentEMN()->GetParentEMR()) {
								nActiveCurrentAllergiesInfoID = pTopic->GetParentEMN()->GetParentEMR()->GetCurrentAllergiesInfoID();
							}
						}

						if(nActiveCurrentAllergiesInfoID == -2) {
							//should only remain -2 if we have no EMR (-1 is bad data, but indicative that we did perform the check),
							//but why don't we have an EMR?
							ASSERT(FALSE);
							nActiveCurrentAllergiesInfoID = GetActiveAllergiesInfoID();
						}

						while (pTopic && !bTryPostSaveUpdateAllergies) {
							const unsigned long nDetails = pTopic->GetEMNDetailCount();
							for (unsigned long n=0; n < nDetails && !bTryPostSaveUpdateAllergies; n++) {
								CEMNDetail* pDetail = pTopic->GetDetailByIndex(n);
								if (nActiveCurrentAllergiesInfoID == pDetail->m_nEMRInfoID && pDetail->IsModified()) {
									bTryPostSaveUpdateAllergies = TRUE;
									// (c.haag 2007-08-15 18:09) - PLID 25525 - Retain the detail for PostSaveUpdateAllergies
									pPostSaveAllergiesDetail = pDetail;
								}
							}

							pTopic = pTopic->GetParentTopic();
						}

					}
				}
				break;

				case esotEMN: {

					// (c.haag 2007-02-09 09:43) - If we are saving an EMN, check if any of its
					// modified details are the official Current Medications item. If so, then we
					// need to do our post-save Current Medications processing.
					CEMN *pEMN = (CEMN*)nObjectPtr;

					// (j.jones 2007-07-23 10:39) - PLID 26742 - get the active medications info ID from the EMR if we can
					long nActiveCurrentMedicationsInfoID = -1;
					if(pEMN->GetParentEMR()) {
						nActiveCurrentMedicationsInfoID = pEMN->GetParentEMR()->GetCurrentMedicationsInfoID();
					}
					else {
						//why don't we have an EMR?
						ASSERT(FALSE);
						//get it the traditional way
						nActiveCurrentMedicationsInfoID = GetActiveCurrentMedicationsInfoID();
					}

					const unsigned long nDetails = pEMN->GetTotalDetailCount();
					unsigned long n;
					for (n=0; n < nDetails && !bTryPostSaveUpdateCurMeds; n++) {
						CEMNDetail* pDetail = pEMN->GetDetail(n);
						if (nActiveCurrentMedicationsInfoID == pDetail->m_nEMRInfoID && pDetail->IsModified()) {
							bTryPostSaveUpdateCurMeds = TRUE;
							//DRT 8/15/2007 - PLID 26527 - We don't want to find out what EMR this detail is on just
							//	to look it up later -- give us the detail!
							pPostSaveCurMedsDetail = pDetail;
						}
					}

					// (c.haag 2007-04-06 09:42) - PLID 25525 - Same with Allergies

					// (j.jones 2007-07-23 10:39) - PLID 26742 - get the active medications info ID from the EMR if we can
					long nActiveCurrentAllergiesInfoID = -1;
					if(pEMN->GetParentEMR()) {
						nActiveCurrentAllergiesInfoID = pEMN->GetParentEMR()->GetCurrentAllergiesInfoID();
					}
					else {
						//why don't we have an EMR?
						ASSERT(FALSE);
						//get it the traditional way
						nActiveCurrentAllergiesInfoID = GetActiveAllergiesInfoID();
					}

					for (n=0; n < nDetails && !bTryPostSaveUpdateAllergies; n++) {
						CEMNDetail* pDetail = pEMN->GetDetail(n);
						if (nActiveCurrentAllergiesInfoID == pDetail->m_nEMRInfoID && pDetail->IsModified()) {
							bTryPostSaveUpdateAllergies = TRUE;
							// (c.haag 2007-08-15 18:09) - PLID 25525 - Retain the detail for PostSaveUpdateAllergies
							pPostSaveAllergiesDetail = pDetail;
						}
					}
				}
				break;

				case esotEMR: {
					// (c.haag 2007-02-09 09:43) - If we are saving an EMR, check if any of its
					// details are modified official Current Medications items. If so, then we need 
					// to do our post-save Current Medications processing.
					CEMR *pEMR = (CEMR*)nObjectPtr;

					// (j.jones 2007-07-23 10:39) - PLID 26742 - get the active medications info ID from the EMR if we can
					long nActiveCurrentMedicationsInfoID = pEMR->GetCurrentMedicationsInfoID();

					const unsigned long nEMNCount = pEMR->GetEMNCount();
					unsigned long nEMN;
					for (nEMN=0; nEMN < nEMNCount && !bTryPostSaveUpdateCurMeds; nEMN++) {
						CEMN* pEMN = pEMR->GetEMN(nEMN);
						const unsigned long nDetails = pEMN->GetTotalDetailCount();
						for (unsigned long n=0; n < nDetails && !bTryPostSaveUpdateCurMeds; n++) {
							CEMNDetail* pDetail = pEMN->GetDetail(n);
							if (nActiveCurrentMedicationsInfoID == pDetail->m_nEMRInfoID && pDetail->IsModified()) {
								bTryPostSaveUpdateCurMeds = TRUE;
								//DRT 8/15/2007 - PLID 26527 - We don't want to find out what EMR this detail is on just
								//	to look it up later -- give us the detail!
								pPostSaveCurMedsDetail = pDetail;
							}
						}
					}

					// (c.haag 2007-04-06 09:42) - PLID 25525 - Same with Allergies

					// (j.jones 2007-07-23 10:39) - PLID 26742 - get the active medications info ID from the EMR if we can
					long nActiveCurrentAllergiesInfoID = pEMR->GetCurrentAllergiesInfoID();

					for (nEMN=0; nEMN < nEMNCount && !bTryPostSaveUpdateAllergies; nEMN++) {
						CEMN* pEMN = pEMR->GetEMN(nEMN);
						const unsigned long nDetails = pEMN->GetTotalDetailCount();
						for (unsigned long n=0; n < nDetails && !bTryPostSaveUpdateAllergies; n++) {
							CEMNDetail* pDetail = pEMN->GetDetail(n);
							if (nActiveCurrentAllergiesInfoID == pDetail->m_nEMRInfoID && pDetail->IsModified()) {
								bTryPostSaveUpdateAllergies = TRUE;
								// (c.haag 2007-08-15 18:09) - PLID 25525 - Retain the detail for PostSaveUpdateAllergies
								pPostSaveAllergiesDetail = pDetail;
							}
						}
					}
				}
				break;

				default:
					// (c.haag 2007-02-09 10:06) - It looks like esotDetail and esotDetailData are never 
					// called for this function. The rest of the save types have no relevance here
					break;
			}


		}

		//DRT 2/24/2006 - PLID 19465 - Once the save has been done, notify the various objects that the save is indeed complete.
		//	Note that this must be done before PropagateIDs, because we may need to know if our EMN or template is new.
		
		// (a.walling 2009-04-22 14:53) - PLID 33948 - Gather medications we want to send here, and actually send after propagating IDs
		CArray<EMNMedication*, EMNMedication*> arMedicationsToSubmitToSureScripts;
		if (esotSaveType == esotEMN || esotSaveType == esotEMR) {
			int iEmnIndex = 0;
			CEMN* pEMN = NULL;

			do {
				if (esotSaveType == esotEMN) {
					if (pEMN != NULL) {
						pEMN = NULL;
					} else {
						pEMN = (CEMN*)nObjectPtr;
					}
				} else if (esotSaveType == esotEMR) {
					CEMR* pEMR = (CEMR*)nObjectPtr;
					if (pEMR->GetEMNCount() > iEmnIndex) {
						pEMN = pEMR->GetEMN(iEmnIndex);
						iEmnIndex++;
					} else {
						pEMN = NULL;
					}
				}
				
				if (pEMN != NULL && !pEMN->IsTemplate()) {
					for (int iMedIndex = 0; iMedIndex < pEMN->GetMedicationCount(); iMedIndex++) {
						EMNMedication* pMedication = pEMN->GetMedicationPtr(iMedIndex);

						if (pMedication && pMedication->bEPrescribe && (pMedication->HasChanged_Deprecated() || pMedication->nID == -1) ) {
							arMedicationsToSubmitToSureScripts.Add(pMedication);
						}
					}
				}
			} while (pEMN);
		}

		// (a.walling 2007-10-18 16:29) - PLID 27664 - Pass in an array to gather topics that need to have their HTML generated
		CArray<CEMRTopic*, CEMRTopic*> arTopicsAffected;


		nxp.Tick("Ready for PostSaveUpdate");

		switch(esotSaveType) {
			case esotTopic: {
				//Tell the topic to update, and all of its subtopics and children, then its parent (but not the parents other topics)
				CEMRTopic *pTopic = (CEMRTopic*)nObjectPtr;
				if(pTopic) {
					CEMN *pParentEMN = pTopic->GetParentEMN();

					// (a.walling 2008-06-09 13:08) - PLID 22049 - Only set saved if we are writable
					if (pParentEMN && pParentEMN->IsWritable()) {
						pTopic->SetSaved();
						// (a.walling 2007-10-18 16:35) - PLID 27664 - Pass the array to gather all affected topics
						pTopic->PostSaveUpdate(TRUE, FALSE, &arTopicsAffected);
					}
				}
				else {
					ASSERT(FALSE);

					if(pProgressDlg) {
						delete pProgressDlg;
						pProgressDlg = NULL;
					}

					return essFailed;
				}
				break;
			}
			case esotEMN: {
				//Tell the EMN to update, and all of its children, and its parent (but not the parents other EMNs)
				CEMN *pEMN = (CEMN*)nObjectPtr;
				// (a.walling 2008-06-09 13:08) - PLID 22049 - Only set saved if we are writable
				if(pEMN) {
					if (pEMN->IsWritable()) {
						pEMN->SetSaved();
						// (a.walling 2007-10-18 16:35) - PLID 27664 - Pass the array to gather all affected topics
						pEMN->PostSaveUpdate(pProgressDlg, TRUE, FALSE, &arTopicsAffected);
					}
				}
				else {
					ASSERT(FALSE);

					if(pProgressDlg) {
						delete pProgressDlg;
						pProgressDlg = NULL;
					}

					return essFailed;
				}
				break;
			}
			case esotEMR:
			default:
				//Tell the EMR to update, and all of its children to update
				pEMR = (CEMR*)nObjectPtr;
				if(pEMR) {
					pEMR->SetSaved();
					// (a.walling 2007-10-18 16:35) - PLID 27664 - Pass the array to gather all affected topics
					pEMR->PostSaveUpdate(pProgressDlg, FALSE, &arTopicsAffected);
				}
				else {
					ASSERT(FALSE);

					if(pProgressDlg) {
						delete pProgressDlg;
						pProgressDlg = NULL;
					}

					return essFailed;
				}
				break;				
		}

		// (z.manning 2009-06-15 17:11) - PLID 34332 - Because the same problem can be linked with
		// multiple objects within the same EMR, we need to wait until all other EMR objects have had
		// post save update called on them before doing so on problems.
		pEMR->PostSaveUpdateProblems();

		if(pEMR->GetIsTemplate()) {
			// (c.haag 2007-10-02 08:31) - PLID 27024 - After a template is loaded within the template
			// editor, the EMNLoader object that was used to load it is what I refer to as "retired", and
			// saved in memory for the purpose of saving an extra query every time we spawn topics. Now that
			// we've saved the template, the "retired" loader becomes out of date. So, we have to delete it
			// so that it won't cause spawning problems.
			switch(esotSaveType) {
				case esotTopic: {
					CEMRTopic *pTopic = (CEMRTopic*)nObjectPtr;
					if(pTopic) {
						CEMN* pEMN = pTopic->GetParentEMN();
						if (NULL != pEMN) {
							pEMN->EnsureNotRetiredTemplateLoader();
						}
					}
					break;
				}
				case esotEMN: {
					CEMN *pEMN = (CEMN*)nObjectPtr;
					if (NULL != pEMN) {
						pEMN->EnsureNotRetiredTemplateLoader();
					}
					break;
				}
				case esotEMR: {
					pEMR = (CEMR*)nObjectPtr;
					if(pEMR) {
						const int nEMNs = pEMR->GetEMNCount();
						for (int i=0; i < nEMNs; i++) {
							CEMN* pEMN = pEMR->GetEMN(i);
							if (NULL != pEMN) {
								pEMN->EnsureNotRetiredTemplateLoader();
							}
						}
					}
					break;
				}
			}
		}

		nxp.Tick("Ready for tracking");

		if(pProgressDlg) {
			nCurrentProgress = 80;
			pProgressDlg->SetProgress(0, nProgressMax, nCurrentProgress);
			if(!pEMR->GetIsTemplate()) {
				//the only part that will be slow here is tracking
				pProgressDlg->SetCaption("Saving EMR Changes - Updating Tracking Ladders...");
			}
			else {
				//you aren't likely going to see this caption for long, if at all
				pProgressDlg->SetCaption("Saving EMR Changes - Propagating Changes...");
			}
		}

		//TES 2/8/2006 - Loop through to find any EMNs we should update Tracking for.
		//TES 5/28/2009 - PLID 34367 - We need to track the IDs of any new details that were added, so that we can
		// update Patient Wellness data as necessary.
		//TES 7/10/2009 - PLID 34534 - The Wellness logic has been reworked, so we no longer need this.
		//CArray<long,long> arNewDetails;
		while(!prsEMRResults->eof) {

			long ID = AdoFldLong(prsEMRResults, "ID");
			EmrSaveObjectType esotSavedType = (EmrSaveObjectType)AdoFldLong(prsEMRResults, "Type");
			long nSavedObjectPtr = AdoFldLong(prsEMRResults, "ObjectPtr");

			if(!pEMR->GetIsTemplate()) {
				if(esotSavedType == esotEMN) {
					PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_EMRCreated, pEMR->GetPatientID(), COleDateTime::GetCurrentTime(), ID, true, -1);
				}
				else if(esotSavedType == esotDeletedEMN) {
					PhaseTracking::UnapplyEvent(pEMR->GetPatientID(), PhaseTracking::ET_EMRCreated, ID, -1);
					// (c.haag 2008-07-15 16:52) - PLID 17244 - If this EMN was deleted, it may have been
					// related to one or more todo alarms. We need to post a universal todo list table checker
					// to make sure everyone is up to speed
					CClient::RefreshTable(NetUtils::TodoList);
				}
				else if(esotSavedType == esotDetail) {					
					// (b.cardillo 2009-06-03 15:07) - PLID 34370 - Our arynAffectedDetailIDs array already has the set of all 
					// existing details that were either modified or deleted; here we include any newly created details.
					arynAffectedDetailIDs.Add(ID);
				}
			}
			//now propagate IDs
			CEMN* pFoundOnEMN = NULL;
			BOOL bFound = pEMR->PropagateNewID(ID, esotSavedType, nSavedObjectPtr, nAuditTransactionID, &pFoundOnEMN);
			if (pFoundOnEMN) {
				WORD wordCount = 1;
				if (mapUpdatedEMNs.Lookup((void*)pFoundOnEMN, wordCount)) {
					// increase the count
					wordCount++;
				}

				mapUpdatedEMNs.SetAt((void*)pFoundOnEMN, wordCount);
			}

			//we should have found the object
			// (z.manning, 08/15/2007) - PLID 27078 - Unless we deleted it.
			ASSERT(bFound || esotSavedType == esotDeletedEMN);

			prsEMRResults->MoveNext();
		}
		// (z.manning 2009-10-27 17:12) - PLID 36064 - We are now completely done with this recordset
		// (which has been disconnected from its connection) so close it.
		prsEMRResults->Close();

		nxp.Tick("Ready for UpdateUnsavedTodos");
		// (c.haag 2012-10-17) - PLID 52863 - This must be called after PropagateNewID for all ID's so that we can assign ID values to EMRTodosT.
		pEMR->UpdateUnsavedTodos();

		nxp.Tick("Ready to update revision");
		// (a.walling 2008-05-29 16:05) - PLID 22049 - Update our internal revision
		for(; !prsRevision->eof; prsRevision->MoveNext())
		{
			long nID = AdoFldLong(prsRevision, "ID");
			_variant_t varRevision = prsRevision->Fields->Item["Revision"]->Value;

			CEMN* pSavedEMN = pEMR->GetEMNByID(nID);
			void* pDummy = NULL;

			if (pSavedEMN != NULL) {
				if (mapSavedObjects.Lookup(pSavedEMN, pDummy)) {
					// this EMN was saved. Update the Revision!
					pSavedEMN->SetRevision(varRevision);
					// (z.manning 2012-09-11 14:45) - PLID 52543 - Update the modified date
					pSavedEMN->SetEMNModifiedDate(AdoFldDateTime(prsRevision, "ModifiedDate", COleDateTime::GetCurrentTime()));
					// (a.walling 2008-06-02 12:24) - PLID 22049 - Notify the interface
					// (a.walling 2008-08-13 13:33) - PLID 22049 - This used to happen in PropagateNewID,
					// but it is more reasonable that it should happen here.
					if (pSavedEMN->GetInterface()) {						
						CNxPerform nxp(__FUNCTION__ " - Sending NXM_EMN_LOADSAVE_COMPLETE");
						// (a.walling 2008-08-12 16:43) - PLID 22049 - Send rather than Post the message
						pSavedEMN->GetInterface()->SendMessage(NXM_EMN_LOADSAVE_COMPLETE, (WPARAM)pSavedEMN, 1);
					}
				} else {
					// this EMN was not saved. Ensure the revision is still OK.
#ifdef _DEBUG
					CString strNewRevision = CreateByteStringFromSafeArrayVariant(varRevision);
					CString strExistingRevision = CreateByteStringFromSafeArrayVariant(pSavedEMN->GetRevision());

					ASSERT(strNewRevision == strExistingRevision);
#endif
				}
			}
		}
		// (z.manning 2009-10-27 17:12) - PLID 36064 - We are now completely done with this recordset
		// (which has been disconnected from its connection) so close it.
		prsRevision->Close();

		//TES 5/28/2009 - PLID 34367 - If there were any new details, update PatientWellnessCompletionItemT
		//TES 7/8/2009 - PLID 34534 - This is all handled now by UpdatePatientWellness_EMRDetails().

		// (b.cardillo 2009-05-29 11:52) - PLID 34370 - Update wellness qualification records
		// If any details were committed (created, updated, or deleted) we need to update our wellness 
		// qualification records
		if (arynAffectedDetailIDs.GetSize() != 0) {
			CNxPerform nxp(__FUNCTION__ " - UpdatePatientWellness");
			// (b.cardillo 2009-07-09 15:32) - PLID 34369 - We used to expect this function to know our 
			// currentuserid, but it can't know that on its own anymore, so we pass it in now.
			UpdatePatientWellness_EMRDetails(GetRemoteData(), arynAffectedDetailIDs, GetCurrentUserID());
			//TES 10/31/2013 - PLID 59251 - Pass in arNewCDSInterventions
			nxp.Tick("UpdateDecisionRules...");
			UpdateDecisionRules(GetRemoteData(), pEMR->GetPatientID(), arNewCDSInterventions);
		}

		//TES 6/3/2009 - PLID 34371 - Update wellness qualification records if any problems were changed.
		if(bUpdateWellnessForProblems) {
			CNxPerform nxp(__FUNCTION__ " - UpdatePatientWellnessForProblems");
			UpdatePatientWellnessQualification_EMRProblems(GetRemoteData(), pEMR->GetPatientID());
			// (c.haag 2010-09-21 11:35) - PLID 40612 - Create todo alarms for decisions
			//TES 10/31/2013 - PLID 59251 - Pass in arNewCDSInterventions
			nxp.Tick("UpdateDecisionRules...");
			UpdateDecisionRules(GetRemoteData(), pEMR->GetPatientID(), arNewCDSInterventions);
		}

		// (a.walling 2010-03-31 11:23) - PLID 38006 - Now we need to ensure any cross-EMN IDs are updated. This must occur after all the other
		// Propagate New ID calls.
		pEMR->PropagateCrossEMNIDs();

		BOOL bNotifyNxServer = FALSE;
		BOOL bSendSureScriptsTableChecker = FALSE;

		if (arMedicationsToSubmitToSureScripts.GetSize() > 0 && SureScripts::IsEnabled()) {
			CNxPerform nxp(__FUNCTION__ " - Generating E-Prescriptions");

			if(pProgressDlg) {
				nCurrentProgress = 84;
				pProgressDlg->SetProgress(0, nProgressMax, nCurrentProgress);
				pProgressDlg->SetCaption("Saving EMR Changes - Generating E-Prescriptions...");
			}

			long nCurrentPatientID = -1;
			if (esotSaveType == esotEMN) {
				nCurrentPatientID = ((CEMN*)nObjectPtr)->GetParentEMR()->GetPatientID();
			} else if (esotSaveType == esotEMR) {
				nCurrentPatientID = ((CEMR*)nObjectPtr)->GetPatientID();
			}
			

			CString strSureScriptsValidationError;
			// (a.walling 2009-05-01 16:37) - PLID 34144 - Keep a total of all errors for later display
			CString strTotalSureScriptsValidationErrors;
			BOOL bSomeSureScriptsActionsFailed = FALSE;
			for (int iMedIndex = 0; iMedIndex < arMedicationsToSubmitToSureScripts.GetSize(); iMedIndex++) {
				// (a.walling 2009-05-01 16:23) - PLID 34144 - We need to clear out the error message each iteration!
				strSureScriptsValidationError.Empty();

				EMNMedication* pMed = arMedicationsToSubmitToSureScripts[iMedIndex];

				// should always be true, doesn't hurt.
				if (pMed && pMed->bEPrescribe) {
					BOOL bSuccess = FALSE;
					try {
						if (pMed->bEPrescribeReviewed) {							
							SureScripts::Messages::NewRxMessage newRx;
							SureScripts::GenerateNewRxFromPatientMedication(GetRemoteData(), pMed->nID, "", newRx);
							// (a.walling 2009-04-22 15:27) - PLID 33948 - the param -2 tells the function to find any pending messages for us
							try {
								SureScripts::QueueOutgoingMessage(GetRemoteData(), newRx, nCurrentPatientID, FALSE, -2);

								bNotifyNxServer = TRUE;
								bSuccess = TRUE;
							} NxCatchAllCallThread("Error queueing pending EMR prescription", {bSomeSureScriptsActionsFailed = TRUE;});
						}
					} catch(CNxValidationException* e) {
						bSomeSureScriptsActionsFailed = TRUE;
						strSureScriptsValidationError += e->m_strDescription;
						strSureScriptsValidationError += "\r\n";
						e->Delete();
					} NxCatchAllCallThread("Error generating pending EMR prescription", {bSomeSureScriptsActionsFailed = TRUE;});

					if (!bSuccess) {
						// (a.walling 2009-04-22 13:14) - PLID 33948 - If this is set to ePrescribe and unreviewed, create a pending prescription
						// The only way an EPrescribe medication should be able to be edited is via the prescription editor, which
						// will warn them of the issues regarding electronic prescriptions submission if this is not a new prescription.

						// if this throws an exception, let it be thrown and handled normally, since it is nothing specific to e-prescribing.
						try {
							// (a.walling 2009-04-24 11:45) - PLID 34033 - Send the provider and the error message
							SureScripts::QueuePendingNewRx(GetRemoteData(), nCurrentPatientID, pMed->nID, pMed->nProviderID, pMed->m_strDrugName, strSureScriptsValidationError);
							bSendSureScriptsTableChecker = TRUE;
						} NxCatchAllThread("Error creating pending EMR prescription");
					}

					// (a.walling 2009-05-01 16:37) - PLID 34144 - Append to the total errors
					if (!strSureScriptsValidationError.IsEmpty()) {
						strTotalSureScriptsValidationErrors += pMed->m_strDrugName;
						strTotalSureScriptsValidationErrors += ": ";
						strTotalSureScriptsValidationErrors += strSureScriptsValidationError;
						strTotalSureScriptsValidationErrors += "\r\n\r\n";
					}
				}
			}

			// same way the errors are reported above. I'm not sure if this is the best approach.. but I havn't had any issues with it yet.

			// (a.walling 2009-05-01 16:37) - PLID 34144 - Display the total errors
			strTotalSureScriptsValidationErrors.TrimRight("\r\n");
			if (bSomeSureScriptsActionsFailed || !strTotalSureScriptsValidationErrors.IsEmpty()) {
				MessageBox(GetActiveWindow(), FormatString("The prescriptions were saved successully, but not all could not be electronically prescribed at this time.\r\n\r\n%s", strTotalSureScriptsValidationErrors), "Practice", MB_ICONINFORMATION);
			}
		}


		// (a.walling 2007-04-17 12:18) - PLID 25454
		if(pProgressDlg) {
			nCurrentProgress = 87;
			pProgressDlg->SetProgress(0, nProgressMax, nCurrentProgress);
			pProgressDlg->SetCaption("Saving EMR Changes - Generating Preview...");
		}

		// (a.walling 2013-03-13 08:40) - PLID 55632 - Ignore unsaved is no longer applicable for the preview pane

		// if we are saving the EMR or EMN, check to see if any IDs have been propogated,
		// and if so then save now.
		if ( (esotSaveType == esotEMN) || (esotSaveType == esotEMR) ) {
			CNxPerform nxp(__FUNCTION__ " - GenerateHTMLFile for updated EMNs");
			POSITION pos = mapUpdatedEMNs.GetStartPosition();
			while (pos) {
				void* vpChangedEMN;
				WORD nItemsChanged;			

				mapUpdatedEMNs.GetNextAssoc(pos, vpChangedEMN, nItemsChanged);
				CEMN* pChangedEMN = (CEMN*)vpChangedEMN;

				if (pChangedEMN) {
					pChangedEMN->GenerateHTMLFile(TRUE, TRUE);
					pChangedEMN->SetNeedToSavePreview(FALSE);
					nxp.Tick("Generated for 0x%08x / %li", pChangedEMN, pChangedEMN->GetID());
				}
			}
		}

		// (a.walling 2007-04-17 12:37) - 25454
		switch(esotSaveType) {
			// don't generate or update for topic-level or other object saves.
			case esotEMN:
				{
					// first check to see if we already saved due to propagating IDs.
					// if we have not, then we should only need to save if the EMN is marked
					// as unsaved or NeedToSavePreview (which is set whenever a topic is saved
					// independent of the EMN).
					CEMN *pEMN = (CEMN*)nObjectPtr;
					WORD wordCount = 1;
					if (mapUpdatedEMNs.Lookup((void*)pEMN, wordCount)) {
						// it's in the map, so it's already saved. Nothing to do here.
					} else {
						if (pEMN) {
							WORD wordDummy;
							BOOL bUnsaved = mapUnsavedEMNs.Lookup((void*)pEMN, wordDummy);

							if (pEMN->GetNeedToSavePreview() || bUnsaved) {
								CNxPerform nxp(__FUNCTION__ " - GenerateHTMLFile for other EMNs via esotEMN");
								// if this EMN was unsaved, or if it is marked need to save preview (meaning a topic was saved),
								// then we need to generate the HTML.
								pEMN->GenerateHTMLFile(TRUE, TRUE);
								pEMN->SetNeedToSavePreview(FALSE);
								nxp.Tick("Generated for 0x%08x / %li", pEMN, pEMN->GetID());
							}
						}
					}
					break;
				}
			case esotEMR:
				{
					CEMR* pEMR = (CEMR*)nObjectPtr;

					if (pEMR) {
						for (int i = 0; i < pEMR->GetEMNCount(); i++) {
							CEMN* pEMN = pEMR->GetEMN(i);
							WORD wordCount;

							if (mapUpdatedEMNs.Lookup((void*)pEMN, wordCount)) {
								// already saved, nothing to do here
							} else {
								if (pEMN) {
									WORD wordDummy;
									BOOL bUnsaved = mapUnsavedEMNs.Lookup((void*)pEMN, wordDummy);

									if (pEMN->GetNeedToSavePreview() || bUnsaved) {
										CNxPerform nxp(__FUNCTION__ " - GenerateHTMLFile for other EMNs via esotEMR");
										// if this EMN was unsaved, or if it is marked need to save preview (meaning a topic was saved),
										// then we need to generate the HTML.
										pEMN->GenerateHTMLFile(TRUE, TRUE);
										pEMN->SetNeedToSavePreview(FALSE);
										nxp.Tick("Generated for 0x%08x / %li", pEMN, pEMN->GetID());
									}
								}
							}
						}
					}
					break;
				}
		}

		if(pProgressDlg) {
			nCurrentProgress = 90;
			pProgressDlg->SetProgress(0, nProgressMax, nCurrentProgress);
			pProgressDlg->SetCaption("Saving EMR Changes - Auditing Changes...");
		}

		{
			CNxPerform nxp(__FUNCTION__ " - CommitAuditTransaction");
			CommitAuditTransaction(nAuditTransactionID);
		}

		// (j.jones 2006-02-08 10:12) - PLID 18946 - to support confirming a save from an object
		// other than the EMR as a whole, we can call SetSaved from that object

		// (c.haag 2006-03-01 12:09) - PLID 19208 - We now have table checker support here.
		// In the last case of an EMR, we send an EMRMasterT table checker with no ID. This
		// should cause the views to do a general refresh instead of refreshing just one
		// EMRMasterT entry.

		if(pProgressDlg) {
			nCurrentProgress = 95;
			pProgressDlg->SetProgress(0, nProgressMax, nCurrentProgress);
			pProgressDlg->SetCaption("Saving EMR Changes - Refreshing Practice...");
		}
		
		if (bSendSureScriptsTableChecker) {
			CClient::RefreshTable(NetUtils::SureScriptsMessagesT);
		}
		if (bNotifyNxServer) {
			SureScripts::NotifyNxServerOfNewMessages();
		}

		//
		switch(esotSaveType) {
			case esotTopic: {
				CNxPerform nxp(__FUNCTION__ " - CClient::RefreshTable esotTopic");
				CEMRTopic *pTopic = (CEMRTopic*)nObjectPtr;
				if(pTopic) {
					if(pTopic->IsTemplate()) {
						// (j.jones 2014-08-12 10:17) - PLID 63189 - we now call RefreshEMRTemplateTable
						// to send an EMRTemplateT tablechecker, which also tells some of our local
						// lists to refresh
						RefreshEMRTemplateTable();
					}
					else {
						CClient::RefreshTable(NetUtils::EMRMasterT, pTopic->GetParentEMN()->GetParentEMR()->GetPatientID());
						// (c.haag 2006-11-13 15:26) - PLID 23158 - Refresh problem records. Put a try-catch here because
						// the save was successful except for this problem, so we don't want the user to think the topic
						// wasn't saved. Even though the user may not have actually changed any problem records, it won't
						// hurt anything if we send the message anyway.
						try {
							nxp.Tick("Querying for problems...");
							//DRT 8/30/2007 - PLID 27259 - Parameterized
							// (j.jones 2008-07-23 16:22) - PLID 30823 - if there are problems, 
							// send the patient ID in the tablechecker, not the problem ID
							// (c.haag 2009-05-12 12:35) - PLID 34234 - Use the new problem link structure
							// (a.walling 2014-07-23 09:12) - PLID 63003 - Use CONST_INT for EMRProblemLinkT.EMRRegardingType enums
							// (a.walling 2014-07-23 09:09) - PLID 63003 - Filter on EMRProblemsT.PatientID when possible
							_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 ID FROM EMRProblemsT "
								"WHERE PatientID = {INT} AND ID IN (SELECT EMRProblemID FROM EMRProblemLinkT WHERE ("
								"(EMRRegardingType = {CONST_INT} AND EMRRegardingID IN (SELECT ID FROM EMRDetailsT WHERE EMRTopicID = {INT})) "
								"OR (EMRRegardingType = {CONST_INT} AND EMRRegardingID IN (SELECT ID FROM EMRDetailsT WHERE EMRTopicID = {INT}))"
								"OR (EMRRegardingType = {CONST_INT} AND EMRRegardingID = {INT})"
								"))",
								pTopic->GetParentEMR()->GetPatientID(),
								eprtEmrItem, pTopic->GetID(),
								eprtEmrDataItem, pTopic->GetID(),
								eprtEmrTopic, pTopic->GetID());
							if(!prs->eof) {
								nxp.Tick("Sending RefreshTable...");
								CClient::RefreshTable(NetUtils::EMRProblemsT, pTopic->GetParentEMN()->GetParentEMR()->GetPatientID());
							}
							prs->Close();
						}
						NxCatchAll("Error refreshing problem records");
					}
				}
				else {
					ASSERT(FALSE);

					if(pProgressDlg) {
						delete pProgressDlg;
						pProgressDlg = NULL;
					}

					return essFailed;
				}
				break;
			}
			case esotEMN: {
				CNxPerform nxp(__FUNCTION__ " - CClient::RefreshTable esotEMN");
				CEMN *pEMN = (CEMN*)nObjectPtr;
				if(pEMN) {
					if (pEMN->IsTemplate()) {
						// (j.jones 2014-08-12 10:17) - PLID 63189 - we now call RefreshEMRTemplateTable
						// to send an EMRTemplateT tablechecker, which also tells some of our local
						// lists to refresh
						RefreshEMRTemplateTable();
					} else {
						CClient::RefreshTable(NetUtils::EMRMasterT, pEMN->GetParentEMR()->GetPatientID());
						// (c.haag 2006-11-13 15:26) - PLID 23158 - Refresh problem records. Put a try-catch here because
						// the save was successful except for this problem, so we don't want the user to think the topic
						// wasn't saved. Even though the user may not have actually changed any problem records, it won't
						// hurt anything if we send the message anyway.
						//
						// (c.haag 2006-11-14 10:33) - I think this can only be fired from EmrTemplateManagerDlg, but I'm 
						// adding the code here for consistency
						//
						try {
							nxp.Tick("Querying for problems...");
							//DRT 8/30/2007 - PLID 27259 - Parameterized.
							// (j.jones 2008-07-23 16:22) - PLID 30823 - if there are problems, 
							// send the patient ID in the tablechecker, not the problem ID
							// (c.haag 2009-05-12 12:35) - PLID 34234 - Use the new problem link structure
							// (a.walling 2014-07-23 09:12) - PLID 63003 - Use CONST_INT for EMRProblemLinkT.EMRRegardingType enums
							// (a.walling 2014-07-23 09:09) - PLID 63003 - Filter on EMRProblemsT.PatientID when possible
							_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 ID FROM EMRProblemsT "
								"WHERE PatientID = {INT} AND ID IN (SELECT EMRProblemID FROM EMRProblemLinkT WHERE ("
								"(EMRRegardingType = {CONST_INT} AND EMRRegardingID IN (SELECT ID FROM EMRDetailsT WHERE EMRID = {INT})) "
								"OR (EMRRegardingType = {CONST_INT} AND EMRRegardingID IN (SELECT ID FROM EMRDetailsT WHERE EMRID = {INT})) "
								"OR (EMRRegardingType = {CONST_INT} AND EMRRegardingID IN (SELECT ID FROM EMRTopicsT WHERE EMRID = {INT})) "
								"OR (EMRRegardingType = {CONST_INT} AND EMRRegardingID = {INT}) "
								"OR (EMRRegardingType = {CONST_INT} AND EMRRegardingID IN (SELECT ID FROM EMRDiagCodesT WHERE EMRID = {INT})) "
								"OR (EMRRegardingType = {CONST_INT} AND EMRRegardingID IN (SELECT ID FROM EMRChargesT WHERE EMRID = {INT})) "
								"OR (EMRRegardingType = {CONST_INT} AND EMRRegardingID IN (SELECT MedicationID FROM EMRMedicationsT WHERE EMRID = {INT})) "
								"))",
								pEMN->GetParentEMR()->GetPatientID(),
								eprtEmrItem, pEMN->GetID(),
								eprtEmrDataItem, pEMN->GetID(),
								eprtEmrTopic, pEMN->GetID(),
								eprtEmrEMN, pEMN->GetID(),
								eprtEmrDiag, pEMN->GetID(),
								eprtEmrCharge, pEMN->GetID(),
								eprtEmrMedication, pEMN->GetID());
							if(!prs->eof) {
								nxp.Tick("Sending RefreshTable...");
								CClient::RefreshTable(NetUtils::EMRProblemsT, pEMN->GetParentEMR()->GetPatientID());
							}
							prs->Close();
						}
						NxCatchAll("Error refreshing problem records");
					}
				}
				else {
					ASSERT(FALSE);

					if(pProgressDlg) {
						delete pProgressDlg;
						pProgressDlg = NULL;
					}

					return essFailed;
				}
				break;
			}
			case esotEMR:
			default:
				if (pEMR->GetIsTemplate()) {
					// (j.jones 2014-08-12 10:17) - PLID 63189 - we now call RefreshEMRTemplateTable
					// to send an EMRTemplateT tablechecker, which also tells some of our local
					// lists to refresh
					RefreshEMRTemplateTable();
				} else {
					CNxPerform nxp(__FUNCTION__ " - CClient::RefreshTable esotEMR");
					CClient::RefreshTable(NetUtils::EMRMasterT, pEMR->GetPatientID());
					// (c.haag 2006-11-13 15:26) - PLID 23158 - Refresh problem records. Put a try-catch here because
					// the save was successful except for this problem, so we don't want the user to think the topic
					// wasn't saved. Even though the user may not have actually changed any problem records, it won't
					// hurt anything if we send the message anyway.
					try {
						nxp.Tick("Querying for problems...");
						//DRT 8/30/2007 - PLID 27259 - Parameterized.
						// (j.jones 2008-07-23 16:22) - PLID 30823 - if there are problems, 
						// send the patient ID in the tablechecker, not the problem ID
						// (c.haag 2009-05-12 12:35) - PLID 34234 - Use the new problem link structure
						// (a.walling 2014-07-23 09:12) - PLID 63003 - Use CONST_INT for EMRProblemLinkT.EMRRegardingType enums
						// (a.walling 2014-07-23 09:09) - PLID 63003 - Filter on EMRProblemsT.PatientID when possible
						_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 ID FROM EMRProblemsT "
								"WHERE PatientID = {INT} AND ID IN (SELECT EMRProblemID FROM EMRProblemLinkT WHERE ("
								"(EMRRegardingType = {CONST_INT} AND EMRRegardingID IN (SELECT ID FROM EMRDetailsT WHERE EMRID IN (SELECT ID FROM EMRMasterT WHERE EMRGroupID = {INT}))) "
								"OR (EMRRegardingType = {CONST_INT} AND EMRRegardingID IN (SELECT ID FROM EMRDetailsT WHERE EMRID IN (SELECT ID FROM EMRMasterT WHERE EMRGroupID = {INT}))) "
								"OR (EMRRegardingType = {CONST_INT} AND EMRRegardingID IN (SELECT ID FROM EMRTopicsT WHERE EMRID IN (SELECT ID FROM EMRMasterT WHERE EMRGroupID = {INT}))) "
								"OR (EMRRegardingType = {CONST_INT} AND EMRRegardingID = {INT}) "								
								"OR (EMRRegardingType = {CONST_INT} AND EMRRegardingID IN (SELECT ID FROM EMRMasterT WHERE EMRGroupID = {INT})) "								
								"OR (EMRRegardingType = {CONST_INT} AND EMRRegardingID IN (SELECT ID FROM EMRDiagCodesT WHERE EMRID IN (SELECT ID FROM EMRMasterT WHERE EMRGroupID = {INT}))) "
								"OR (EMRRegardingType = {CONST_INT} AND EMRRegardingID IN (SELECT ID FROM EMRChargesT WHERE EMRID IN (SELECT ID FROM EMRMasterT WHERE EMRGroupID = {INT}))) "
								"OR (EMRRegardingType = {CONST_INT} AND EMRRegardingID IN (SELECT MedicationID FROM EMRMedicationsT WHERE EMRID IN (SELECT ID FROM EMRMasterT WHERE EMRGroupID = {INT}))) "
								"))",
								pEMR->GetPatientID(),
								eprtEmrItem, pEMR->GetID(),
								eprtEmrDataItem, pEMR->GetID(),
								eprtEmrTopic, pEMR->GetID(),
								eprtEmrEMR, pEMR->GetID(),
								eprtEmrEMN, pEMR->GetID(),
								eprtEmrDiag, pEMR->GetID(),
								eprtEmrCharge, pEMR->GetID(),
								eprtEmrMedication, pEMR->GetID());
							if(!prs->eof) {
								nxp.Tick("Sending RefreshTable...");
								CClient::RefreshTable(NetUtils::EMRProblemsT, pEMR->GetPatientID());
							}
							prs->Close();
					}
					NxCatchAll("Error refreshing problem records");
				}
				break;	
		}

		// (j.jones 2009-09-21 10:46) - PLID 35367 - get the newest EMN date for the patient
		COleDateTime dtNewestEMNDate;
		dtNewestEMNDate.SetStatus(COleDateTime::invalid);
		if(bTryPostSaveUpdateCurMeds || bTryPostSaveUpdateAllergies) {
			CNxPerform nxp(__FUNCTION__ " - GetLatestEMNDateForPatient");
			//get the newest EMN date
			dtNewestEMNDate = GetLatestEMNDateForPatient(pEMR->GetPatientID());
		}

		// (c.haag 2007-02-09 09:49) - PLID 24396 - If necessary, compare the saved object's
		// Current Medication details with those in the Medications tab
		if (bTryPostSaveUpdateCurMeds) {
			if(pPostSaveCurMedsDetail == NULL)
				//Shouldn't be allowed
				ASSERT(FALSE);
			else {

				// (j.jones 2009-09-21 10:41) - PLID 35367 - We will not update patient allergies or current meds
				// unless there is no EMN on the patient's account with a newer date. If there was another EMN
				// with the same date, then that's fine. We're simply reducing this update ability to only occur
				// on the most recent EMNs.
				BOOL bCanSave = FALSE;
				if(dtNewestEMNDate.GetStatus() == COleDateTime::invalid) {
					//this means that we're saving the only EMN on the account
					bCanSave = TRUE;
				}
				else {
					COleDateTime dtEMNDate = pPostSaveCurMedsDetail->m_pParentTopic->GetParentEMN()->GetEMNDate();					
					//don't need to revert to date-only because it should already be that way and if it's the same date
					//and a greater time than the date-only dtNewestEMNDate, that's perfectly fine
					if(dtEMNDate >= dtNewestEMNDate) {
						bCanSave = TRUE;
					}
				}

				if(bCanSave) {
					//DRT 8/15/2007 - PLID 26527 - Call the PostSaveUpdate on the EMR for this detail
					// (j.jones 2012-10-03 15:47) - PLID 36220 - if current meds. change, this function will update bDrugInteractionsChanged
					//TES 10/31/2013 - PLID 59251 - Pass in arNewCDSInterventions
					CNxPerform nxp(__FUNCTION__ " - PostSaveUpdateCurrentMedications");
					pPostSaveCurMedsDetail->m_pParentTopic->GetParentEMN()->GetParentEMR()->PostSaveUpdateCurrentMedications(pProgressDlg, pPostSaveCurMedsDetail, bDrugInteractionsChanged, arNewCDSInterventions);
				}
			}
		}
		// (c.haag 2007-04-06 09:44) - PLID 25525 - If necessary, do the same with Allergies
		// (c.haag 2007-08-15 18:11) - PLID 25525 - Use the pPostSaveAllergiesDetail when doing
		// processing here
		if (bTryPostSaveUpdateAllergies) {
			if(pPostSaveAllergiesDetail == NULL) {
				ASSERT(FALSE); // This should never happen
			} else {

				// (j.jones 2009-09-21 10:41) - PLID 35367 - We will not update patient allergies or current meds
				// unless there is no EMN on the patient's account with a newer date. If there was another EMN
				// with the same date, then that's fine. We're simply reducing this update ability to only occur
				// on the most recent EMNs.
				BOOL bCanSave = FALSE;
				if(dtNewestEMNDate.GetStatus() == COleDateTime::invalid) {
					//this means that we're saving the only EMN on the account
					bCanSave = TRUE;
				}
				else {
					COleDateTime dtEMNDate = pPostSaveAllergiesDetail->m_pParentTopic->GetParentEMN()->GetEMNDate();
					//don't need to revert to date-only because it should already be that way and if it's the same date
					//and a greater time than the date-only dtNewestEMNDate, that's perfectly fine
					if(dtEMNDate >= dtNewestEMNDate) {
						bCanSave = TRUE;
					}
				}

				if(bCanSave) {
					//DRT 8/15/2007 - PLID 26527 - Call the PostSaveUpdate on the EMR for this detail
					// (j.jones 2012-10-03 15:47) - PLID 36220 - if allergies change, this function will update bDrugInteractionsChanged
					CNxPerform nxp(__FUNCTION__ " - PostSaveUpdateAllergies");
					pPostSaveAllergiesDetail->m_pParentTopic->GetParentEMN()->GetParentEMR()->PostSaveUpdateAllergies(pProgressDlg, pPostSaveAllergiesDetail, bDrugInteractionsChanged);
				}
			}
		}


		if(pProgressDlg) {
			nCurrentProgress = 100;
			pProgressDlg->SetProgress(0, nProgressMax, nCurrentProgress);
			pProgressDlg->SetCaption("Saving EMR Changes - Save Complete.");

			delete pProgressDlg;
			pProgressDlg = NULL;
		}

		//if there were warnings, and we bypassed them, confirm we did save,
		//but use the flag to tell the calling code we had warnings,
		//which likely means there are still some unsaved items
		if(arystrErrors.GetSize() > 0) {
			return essSuccessWithWarnings;
		}
		else {
			return essSuccess;
		}

	}
	catch (CException *e) {
		HandleException(e, "EMRUtils::SaveEMR : Failed to save EMR.", __LINE__, __FILE__);
		RollbackAuditTransaction(nAuditTransactionID);
	}
	catch (_com_error e) { 
		// (a.walling 2008-05-30 12:24) - PLID 22049 - Advanced SQL Error reporting - detect concurrency violations
		BOOL bHandledException = FALSE;
		// (a.walling 2008-08-27 14:23) - PLID 30855 - Use the shared function
		CSQLErrorInfo eSqlError;
		if (GetSQLErrorInfo(e, eSqlError) && eSqlError.IsNxRaisedError()) {
			switch(eSqlError.nState) {
			case 1:
				// attempt to write to locked EMN trigger
				HandleException(e, "EMRUtils::SaveEMR : Failed to save EMR. An attempt was made to write to a locked EMN.", __LINE__, __FILE__); 
				bHandledException = TRUE;
				break;
			case 61:
				// duplicate EMN template detail trigger
				HandleException(e, "EMRUtils::SaveEMR : Failed to save EMR. A duplicate EMN template detail was detected.", __LINE__, __FILE__); 
				bHandledException = TRUE;
				break;
			case 42:
				// revision is out of date
				HandleException(e, "Object could not be saved; it has been modified by another user since it was initially opened.", __LINE__, __FILE__); 
				bHandledException = TRUE;
				// (a.walling 2008-06-26 16:14) - PLID 30513 - Failed due to concurrency violations
				bFailedConcurrency = TRUE;
				break;
			case 43:
				// write token is missing or invalid
				HandleException(e, "Object could not be saved; another user has acquired write access.", __LINE__, __FILE__); 
				bHandledException = TRUE;
				// (a.walling 2008-06-26 16:14) - PLID 30513 - Failed due to concurrency violations
				bFailedConcurrency = TRUE;
				break;
			default:
				break;
			}
		}
		if (!bHandledException) {
			HandleException(e, "EMRUtils::SaveEMR : Failed to save EMR.", __LINE__, __FILE__); 
		}
		RollbackAuditTransaction(nAuditTransactionID); 
	}
	catch (...) { 
		HandleException(NULL, "EMRUtils::SaveEMR : Failed to save EMR.", __LINE__, __FILE__); 
		RollbackAuditTransaction(nAuditTransactionID);
	}

	if(pProgressDlg) {
		delete pProgressDlg;
		pProgressDlg = NULL;
	}
	// (a.walling 2008-06-26 16:20) - PLID 30513 - more info returned on failure
	return bFailedConcurrency ? essFailedConcurrency : essFailed;
}

// (c.haag 2016-06-09 14:54) - PLID-66502 - Gets the visible name of a save object
CString GetEmrSaveObjectTypeName(EmrSaveObjectType esotSaveObject)
{
	switch (esotSaveObject)
	{
	case esotEMR: return "EMR";
	case esotEMN: return "EMN";
	case esotCharge: return "Charge";
	case esotPrescription: return "Prescription";
	case esotTopic: return "Topic";
	case esotDetail: return "Detail";
	case esotTableDefault: return "Table Default";
	case esotDetailData: return "Detail Data";
	case esotDeletedEMN: return "Deleted EMN";
	case esotProblem: return "EMR Problem";
	case esotDiagCode: return "Diag Code";
	case esotProblemLink: return "Problem Link";
	case esotDetailImageStamp: return "Detail Image Stamp";
	default:
		// If you hit this assertion you need to add support for this type
		ASSERT(FALSE);
		return "";
	}
}

// (c.haag 2016-06-09 14:54) - PLID-66502 - Write pertinent information about the EMR object we are saving to NxLog
void LogEmrObjectData(EmrSaveObjectType esotSaveType, long nObjectPtr, const Nx::Quantum::Batch& finalBatch)
{
	Log(R"(============================== EMR SAVE AND OBJECT DUMP ==============================

)");
	bool bUseAutoQuantumBatch = Nx::Quantum::UseAutoQuantumBatch();
	CString strFinalRecordset = bUseAutoQuantumBatch ? finalBatch.FlattenToQuantum() : finalBatch.FlattenToClassic();

	// Log the save
	Log(R"(%s
)", strFinalRecordset);

	// Now log the content
	Log(R"(=== ALL RELATED OBJECTS ===

)");
	switch (esotSaveType)
	{
		case esotTopic: {
			CEMRTopic *pTopic = (CEMRTopic*)nObjectPtr;
			pTopic->LogEmrObjectData(0, FALSE);
			break;
		}
		case esotEMN: {
			CEMN *pEMN = (CEMN*)nObjectPtr;
			pEMN->LogEmrObjectData(0, FALSE, FALSE);
			break;
		}
		case esotEMR:
		default: {
			CEMR *pEMR = (CEMR*)nObjectPtr;
			pEMR->LogEmrObjectData(0, FALSE);
			break;
		}
	}

	Log(R"(============================== DUMP COMPLETE ==============================

)");
}

// (c.haag 2016-06-09 14:54) - PLID-66502 - Write pertinent information about this EMR object to NxLog. This is used to help pin down save errors.
// nIndent - The number of spaces to indent this log line
// nID - The ID of the object, or a non-positive number if it isn't saved yet
// pAddress - The memory address of the object
// esotSaveObject - The type of object
// bNew - True if the object is flagged as new
// bModified - True if the object is flagged as modified
// bDeleted - True if the object is flagged as deleted
// strName - The name of the object
// szExtraDesc - Additional information for this object
void LogEmrObjectData(int nIndent, int nID, void* pAddress, EmrSaveObjectType esotSaveObject, BOOL bNew, BOOL bModified, BOOL bDeleted, const CString& strName, LPCTSTR szExtraDesc, ...)
{
	CString strExtraDesc;
	va_list argList;
	va_start(argList, szExtraDesc);
	strExtraDesc.FormatV(szExtraDesc, argList);
	va_end(argList);

	// Calculate the indent string
	CString strIndent;
	for (int i = 0; i < nIndent; i++, strIndent += " ");

	// Log format: [ {ID} : {Address} ] { Object type } { NEW | MOD | DEL } { Name } { Additional info }
	Log("%s[%d : %li] %s - %s%s%s- (%s) %s"
		, strIndent
		, nID
		, pAddress
		, GetEmrSaveObjectTypeName(esotSaveObject)
		, bNew ? "NEW " : ""
		, bModified ? "MOD " : ""
		, bDeleted ? "DEL " : ""
		, strName
		, strExtraDesc);
}

// (z.manning 2011-10-21 15:17) - PLID 44649 - Added info sub type
void CleanDetailsForReload(CEMRTopic* pTopic, long nEMRInfoID, EmrInfoType eNewInfoType, EmrInfoSubType eNewInfoSubType,
						   _variant_t& vDefaultState, CPtrArray& apModifiedDetails)
{
	int nDetails = pTopic->GetEMNDetailCount();
	int nChildren = pTopic->GetSubTopicCount();
	int k;
	for (k=0; k < nDetails; k++) {
		CEMNDetail* pDetail = pTopic->GetDetailByIndex(k);
		if (pDetail->m_nEMRInfoID == nEMRInfoID) {
			//(e.lally 2007-06-26) PLID 25613 - This section of code was removing the detail dialog
				//so that it could be recreated when there does not appear to be a specific reason
				//for that behavior. Its lack of recreation was causing a section of code in EmrTopicWnd::
				//RepositionDetailsInTopic to not get executed. Inside the section of code that needs to be
				//executed to handle this situation is a call to CEMNDetail:: ReloadEMRItemAdvDlg which 
				//is a cleaner method of destroying and recreating the dialog.
			/*
			if (pDetail->m_pEmrItemAdvDlg) {
				pDetail->m_pEmrItemAdvDlg->DestroyWindow();
				delete pDetail->m_pEmrItemAdvDlg;
				pDetail->m_pEmrItemAdvDlg = NULL; 
			}
			*/
			//(e.lally 2007-06-26) PLID 25613 - In order for the section of code of that we need to execute,
				// -it expects this type value to still be the previous type.
				// -it expects the need content reload flag to be set to true.
			//pDetail->m_EMRInfoType = type;
			pDetail->SetNeedContentReload(TRUE);

			// If we changed from one type to another that shares the same state, we
			// have to clear the state. For example, a narrative state could look like
			// "}} \viewkind4\uc1\pard\lang1033\f0\fs24 \par }", but if we change the item
			// to a table, the table content will try to parse all that out into table
			// data. If this detail is saved in data, LoadContent will restore that state.

			// The reason we don't call pDetail->RequestStateChange is that RequestStateChange
			// does all sorts of stuff like LoadContent and RevokeEMRActions that we cannot
			// presently do because some of the details may still have mismatched info types
			// and content. To work around this, we'll set the state directly, and LoadContent
			// will be called naturally down the road when it needs to be.

			// (z.manning, 07/26/2007) - PLID 26574 - We may not want to reset the state. There are certain cases
			// (generally involving changing from single to multi or vice versa) where changing the info type
			// is perfectly valid.
			if(InfoTypeChangeRequiresDetailStateReset(pDetail->m_EMRInfoType, pDetail->m_EMRInfoSubType, eNewInfoType, eNewInfoSubType, pDetail)) {
				pDetail->SetState(vDefaultState);
			}

			// (c.haag 2007-01-03 16:59) - PLID 24091 - Update the topic's completion status
			pTopic->HandleDetailStateChange(pDetail);

			// (c.haag 2007-01-03 10:01) - PLID 22849 - We now need to update the state in data
			// as well. If we don't do this, and the user cancels out of the EMR, then the old
			// state from data is loaded the next time the user opens the EMR (as opposed to
			// vDefaultState)
			//
			// At this point, we build a list of details that exist in data but were modified here

			if (pDetail->m_nEMRDetailID > 0) {
				apModifiedDetails.Add(pDetail);
			}
		}
	}
	for (k=0; k < nChildren; k++) {
		CleanDetailsForReload(pTopic->GetSubTopic(k), nEMRInfoID, eNewInfoType, eNewInfoSubType, vDefaultState,
			apModifiedDetails);
	}
}

// (a.walling 2008-01-29 15:20) - PLID 14982 - Added variable for safety check
void PostEmrItemEntryDlgItemSaved(CEMNDetail* pSrcDetail, BOOL bFromMessage)
{
	try {
		CEMRTopic *pTopic = pSrcDetail->m_pParentTopic;
		CEMN* pEMN = (pTopic) ? pTopic->GetParentEMN() : NULL;
		CEMR* pEMR = (pEMN) ? pEMN->GetParentEMR() : NULL;

		// (c.haag 2006-04-04 13:32) - PLID 19883 - This is called as a result of a user
		// changing an EMR info item from an EMR or a template (please refer to
		// CEmrItemAdvDlg::OpenItemEntryDlg()). Once an item has been modified, it is
		// entirely possible that the type of the item has changed. If that happens, we
		// have to destroy the CEmrItemAdvDlg from which the user committed the edit from.
		// We obviously cannot destroy it from itself, so we got here as a result of a posted
		// message.
		//
		// This function does several things
		// - If the type has changed, destroy all corresponding CEmrItemAdvDlg detail members
		// - If the type has changed, update m_EMRInfoType in all corresponding details
		// - Executes carried over code related to repositioning details, loading content,
		// etc (this is not actually part of 19883; it's just copied over).
		//

		//
		// First, determine if the data type has changed. If so, set bInfoTypeChanged to true
		// and remember the new type
		//
		//DRT 8/30/2007 - PLID 27259 - Parameterized.
		// (z.manning 2011-10-21 14:42) - PLID 44649 - Added DataSubType
		_RecordsetPtr rsInfo = CreateParamRecordset("SELECT DataType, DataSubType FROM EMRInfoT WHERE ID = {INT}",
			pSrcDetail->m_nEMRInfoID);
		BOOL bInfoTypeChanged = FALSE;
		_variant_t vDefaultState;
		EmrInfoType eNewInfoType;
		EmrInfoSubType eNewInfoSubType;
		// (c.haag 2007-01-03 10:27) - PLID 22849 - Variables that track details that were modified
		// if the EMR info item type was modified
		CPtrArray apModifiedDetails;
		// (c.haag 2007-02-01 15:54) - PLID 22849 - We now track errors when generating the save string
		CStringArray arystrErrors;

		// (a.walling 2008-02-07 13:32) - PLID 28857 - Array to hold all items of this info type that may have change
		// (a.walling 2008-02-08 09:20) - PLID 28857 - Converted to a map to prevent duplicates
		CMap<CEMNDetail*, CEMNDetail*, BOOL, BOOL> mapDetailsToUpdate;

		// (a.walling 2008-03-24 16:05) - PLID 28811 - Map of all topics that need refreshing due to item type changes
		CMap<CEMRTopic*, CEMRTopic*, BOOL, BOOL> mapTopicsToRefresh;

		if (!rsInfo->eof)
		{
			eNewInfoType = (EmrInfoType)AdoFldByte(rsInfo, "DataType");
			eNewInfoSubType = (EmrInfoSubType)AdoFldByte(rsInfo, "DataSubType");
			// (z.manning 2011-10-21 15:00) - PLID 44649 - Changed this to factor in subtype as well
			if (!IsSameEmrInfoType(pSrcDetail->m_EMRInfoType, pSrcDetail->m_EMRInfoSubType, eNewInfoType, eNewInfoSubType))
			{
				bInfoTypeChanged = TRUE;

				mapTopicsToRefresh[pSrcDetail->m_pParentTopic] = TRUE;

				// (z.manning, 07/27/2007) - PLID 26574 - Loading the default state here was really shady.
				// For one, you could get into some really weird situations such as changine a multi-select
				// that had mutliple selections to a single where it would randomly select 1 option. Also,
				// CEmrTopicWnd::RepositionDetailsInTopicByInfoOrMasterID loads a blank state when it 
				// detects the info type has changed, so we should be consistent with that.
				//vDefaultState = LoadEMRDetailStateDefault(pSrcDetail->m_nEMRInfoID, pSrcDetail->m_pParentTopic->GetParentEMN()->GetParentEMR()->GetPatientID(), -1);
				// (z.manning 2011-10-21 17:13) - PLID 44649 - When loading the blank set set the image type
				// to undefined because that will ensure we use the image file from content (rather than the
				// one from state, which will be nothing since we're loading a blank state).
				vDefaultState = LoadEMRDetailStateBlank(eNewInfoType, TRUE);
			}
		}
		rsInfo->Close();

		
		// (a.walling 2008-02-07 13:36) - PLID 28857 - Moved from above since we will use this to find all details that may be affected
		if (pEMN) {
			long nDetails = pEMN->GetTotalDetailCount();

			for (int i = 0; i < nDetails; i++) {
				CEMNDetail* pDetail = pEMN->GetDetail(i);

				if (pDetail != pSrcDetail && pDetail->m_nEMRInfoID == pSrcDetail->m_nEMRInfoID) {
					// (a.walling 2008-02-07 13:37) - PLID 28857 - Keep track of this to update at the end of this function
					mapDetailsToUpdate[pDetail] = TRUE;

					if (pDetail->m_nEMRDetailID == -1) {
						mapTopicsToRefresh[pDetail->m_pParentTopic] = TRUE;
					}

					// (a.walling 2008-02-08 09:24) - PLID 28857 - Also keep track of any linked details
					CArray<CEMNDetail*, CEMNDetail*> arLinkedDetails;
					pDetail->GetLinkedDetails(arLinkedDetails, pEMN );

					for (int x = 0; x < arLinkedDetails.GetSize(); x++) {
						mapDetailsToUpdate[arLinkedDetails.GetAt(x)] = TRUE;
					}
				}
			}
		}

		//
		// If the type has changed, delete the CEmrItemAdvDlg out of every corresponding
		// detail and assign each detail the new type. If we do not do this, evil things
		// happen because the item type and detail type don't match, so improper data is
		// loaded. Even worse, the underlying EMR code expects details to have a certain
		// kind of CEmrItemAdvDlg based on their m_EMRInfoTypes. If there is a type mismatch,
		// a bad type cast will cause a crash.
		//
		if (bInfoTypeChanged) {
			// (a.walling 2008-01-29 15:22) - PLID 14982 - The info type should only change if we are calling in response to a message
			ASSERT(bFromMessage);
			if (!bFromMessage)
				ThrowNxException("Info type changed but not in response to a message!");

			{
				// (a.walling 2008-03-24 16:05) - PLID 28811 - We need to ensure the topic window has been created
				POSITION pos = mapTopicsToRefresh.GetStartPosition();
				while (pos) {
					BOOL bDummy;
					CEMRTopic* pTopic;
					mapTopicsToRefresh.GetNextAssoc(pos, pTopic, bDummy);

					CEmrTopicWndPtr pTopicWnd = pTopic->GetTopicWnd();
					if (!pTopicWnd) {
						CEmrTreeWnd* pTreeWnd = pEMN->GetParentEMR()->GetInterface();
						if (pTreeWnd) {
							NXDATALIST2Lib::IRowSettingsPtr pRow = pTreeWnd->FindInTreeByColumn(TREE_COLUMN_OBJ_PTR, _variant_t((long)pTopic), NULL);
							if (pRow) {
								pTreeWnd->EnsureTopicRow(pRow);
							}
						} else {
							ASSERT(FALSE);
						}
					}
				}
			}

			// (a.walling 2007-04-02 10:16) - PLID 20002 - We should revoke any actions that this item spawned if changing the type.
			// Exception 1: Going from a single-select to multi-select.
			// Exception 2: Going from a multi-select with one item selected to a single-select (with that item selected).
			//		nevermind; the selection is lost regardless. So this will revoke actions regardless to be consistent.
			// (z.manning, 07/27/2007) - PLID 26574 - Fixed it so the selection is not lost in the situations
			// Adam describes, so restored this functionality to only revoke actions if we're going to be
			// resetting the state.
			if(InfoTypeChangeRequiresDetailStateReset(pSrcDetail->m_EMRInfoType, pSrcDetail->m_EMRInfoSubType, eNewInfoType, eNewInfoSubType, pSrcDetail)) 
			{
				if(pEMR && (pSrcDetail->m_EMRInfoType == eitSingleList || pSrcDetail->m_EMRInfoType == eitMultiList)) 
				{
					// (a.walling 2007-06-18 13:57) - PLID 20002 - Simpler handling of revoking actions
					// which also handles previous versions of the data.
					//DRT 8/3/2007 - PLID 26938 - The old code here didn't work with the rules set forth in PLID 19099.  It also was 
					//	very inefficient, so I've changed it to use the CEMNUnspawner object instead of manually revoking each action.
					//	All we're really doing is revoking an entire detail without touching the info actions.  So I added an option
					//	to ignore all info actions

					//This is slightly shady.  Because the detail changed (and that change has been committed to data), before
					//	we get to this function, the pSrcDetail has been set as "needs content reloaded".  But we cannot reload the content
					//	until we've taken care of actions.  Because of this, we have to actually turn off the content reload need, do 
					//	our revoking work, then turn the flag back on.  In my opinion (noted in PLID 26993), we should entirely remove
					//	the detail and re-add the new one, there's really very little similar between the 2.
					BOOL bNeedReload = pSrcDetail->GetNeedContentReload();
					pSrcDetail->SetNeedContentReload(FALSE);
					//

					//Now unspawn all the data actions from this detail.
					CEMNUnspawner eu(pEMN);
					eu.SetOption(eUnspawnIgnoreInfoActions, true);
					CArray<CEMNDetail*, CEMNDetail*> aryDetails;
					aryDetails.Add(pSrcDetail);
					eu.RemoveActionsByDetails(&aryDetails);

					//DRT 9/12/2007 - PLID 26938 - This is goofy code.  The unspawner must flag our source detail for deletion, otherwise
					//	the spawning will not properly remove the spawned details (it will keep them around with saved positions, 
					//	which we do not want since this detail no longer spawns things).  So, we let the unspawner flag the detail
					//	as deleted, do the revoking, then when we come back here, we'll have to manually clear it.
					if(pTopic) {
						pTopic->RemoveDetailFromPendingDeletion(pSrcDetail);
					}
					else {
						//We lack a parent topic for this detail?  We're certain to have abandoned the detail in the pending
						//	deletion array, and you'll get an assertion when you close the current EMR.  Note:  That assertion has
						//	no bearing on anything.  It's just for information purposes, and causes no harm at all.
						//And so I'm going to put an assertion here too, so that you know to expect it.  There's nothing that can
						//	be done about these if you hit this case, sorry.
						ASSERT(FALSE);
					}

					//Undo our content reload change from above
					pSrcDetail->SetNeedContentReload(bNeedReload);
					//
				}
			}			

			if (NULL != pEMR && NULL != pEMN) {
				// (c.haag 2007-09-27 15:04) - PLID 27526 - A detail can only be on one and only one EMN, so there is
				// no reason to go through all the EMR's. In fact, doing so will cause ASSERT'ions to be fired, and
				// other unexpected problems. Another problem is that pEMN was assigned a value earlier, and it gets
				// overwritten here!
				/*int nEMNs = pEMR->GetEMNCount();
				int i;
				for (i=0; i < nEMNs; i++) {
					pEMN = pEMR->GetEMN(i);*/
					//TES 6/22/2006 - PLID 20006 - We need to remove this detail from narratives, and re-add it once its been reloaded.
					//TES 6/27/2006 - OK, here's the plan: we need to get a list of the elements to be removed, which are still
					//set in this detail's member variables.  But if we let it reload its content, it will clear out those 
					//member variables before we can read them.  So we set it to false.  But we can't call the 
					//RemoveNarrativeField() function until SetNeedContentReload(TRUE) is called; otherwise, the narrative will
					//immediately query the program for whether that list field exists, and since it is still loaded in the
					//member variables, the program will say that it does exist, and put it right back in.  This could be worked
					//around, but there's a lot of code that tries to keep the program and the narrative in sync, and that's a
					//noble goal for the most part.  So we'll do the absolute minimum we have to (which is read the names of all
					//the list elements) while SetNeedContentReload() is turned off.

					// (a.walling 2009-11-18 12:09) - PLID 36365 - No longer necessary
					/*
					pSrcDetail->SetNeedContentReload(FALSE);
					int nCount = pSrcDetail->GetListElementCount();
					CStringArray saFieldsToRemove;
					CString strDetailName = pSrcDetail->GetLabelText();
					for(int nElement = 0; nElement < nCount; nElement++) {
						ListElement le = pSrcDetail->GetListElement(nElement);
						saFieldsToRemove.Add(strDetailName + " - " + le.strName);
					}
					pSrcDetail->SetNeedContentReload(TRUE);
					for(nElement = 0; nElement < saFieldsToRemove.GetSize(); nElement++) {
						pEMN->RemoveNarrativeField(saFieldsToRemove[nElement]);
					}
					*/
					//TES 1/23/2008 - PLID 24157 - Renamed.
					pEMN->HandleDetailChange(pSrcDetail, TRUE);
					int nTopics = pEMN->GetTopicCount();
					for (int j=0; j < nTopics; j++) {
						CleanDetailsForReload(pEMN->GetTopic(j), pSrcDetail->m_nEMRInfoID, eNewInfoType, eNewInfoSubType,
							vDefaultState, apModifiedDetails);
					}
					//Now put it back.
					//TES 1/23/2008 - PLID 24157 - Renamed.
					pEMN->HandleDetailChange(pSrcDetail, FALSE);

				//} // for (i=0; i < nEMNs; i++) {

			} // if (NULL != pEMR && NULL != pEMN) {

		} // if (bInfoTypeChanged) {

		//
		// Begin legacy code carried over from CEMRItemAdvDlg not related to 19883
		//

		// (c.haag 2006-03-31 13:31) - PLID 19387 - Now reload the current detail from data so that the
		// content matches that of what the user changed, if anything.
		//
		// If the data type is 2 or 3, this is obviously redundant because we call ClearContent later
		// on. However, we must ensure that all variables are loaded. We can optimize this later by
		// not calling LoadContent() if the data type is 2 or 3; but instead call an overload called
		// LoadContent(CEmrItemEntryDlg*) that won't pull row-specific list content from data.
		//
		pSrcDetail->LoadContent();

		// (j.jones 2010-03-03 09:59) - PLID 37318 - if they edited a SmartStamp Image,
		// make sure the linked table is set up properly
		pSrcDetail->EnsureLinkedSmartStampTableValid();

		/*
		m_pDetail->ClearContent();

		m_pDetail->BeginAddListElements();
		for(int i = 0; i < naryDataIDs.GetSize(); i++) {

			ASSERT(naryDataIDs.GetAt(i) != -1);
			m_pDetail->AddListElement(naryDataIDs.GetAt(i), straryData.GetAt(i), straryLabels.GetAt(i) == "1", straryLongForms.GetAt(i));

			//mark the item changed wherever it may be loaded on the entire EMR,
			//not just at a topic or EMN level
			//
			// (c.haag 2006-03-31 14:31) - Don't do this becaause it sets m_bNeedToLoadContent to 1.
			// We already loaded it earlier. If LoadContent is called for the item, then we will
			// lose changes made to the active list.
			//
			//m_pParentEMN->GetParentEMR()->MarkItemChanged(nInfoID);
		}

		m_pDetail->EndAddListElements();*/

		//
		// More legacy code here
		//
		// (z.manning 2009-03-20 11:41) - PLID 15971 - I added tables here as table dropdown items can now
		// spawn things.  I also added images for hot spots which looks to have been missed previously.
		if (pSrcDetail->m_EMRInfoType == eitSingleList || pSrcDetail->m_EMRInfoType == eitMultiList ||
			pSrcDetail->m_EMRInfoType == eitTable || pSrcDetail->m_EMRInfoType == eitImage)
		{
			CEmrItemAdvDlg* pDlg = pSrcDetail->m_pEmrItemAdvDlg;

			if (pDlg) {
				CRect rWindow;
				pDlg->GetWindowRect(rWindow);
				pDlg->RepositionControls(CSize(rWindow.Width(),rWindow.Height()),FALSE);
			}

			//We need to first remove the detail from the narrative and then readd it so that all of the list fields will be correct
			//as we just reloaded the content for the list
			//TES 1/23/2008 - PLID 24157 - Renamed.
			pEMN->HandleDetailChange(pSrcDetail, TRUE);
			pEMN->HandleDetailChange(pSrcDetail);
		
			if(pEMN->IsTemplate()) {
				pEMN->RevokeDeletedActionsForInfoID(pSrcDetail->m_nEMRInfoID);
			}

		} else {
			// (a.walling 2010-05-17 14:19) - PLID 38000 - Ensure the output is reflected if any sentence format etc stuff has changed
			pEMN->HandleDetailChange(pSrcDetail);
		}

		{
			// (a.walling 2008-03-24 16:07) - PLID 28811 - Now force the topic windows to reposition their details, which will reposition
			// any changed types and update the item to the changed type immediately.
			POSITION pos = mapTopicsToRefresh.GetStartPosition();
			while (pos) {
				BOOL bDummy;
				CEMRTopic* pTopic;
				mapTopicsToRefresh.GetNextAssoc(pos, pTopic, bDummy);

				// (a.walling 2008-03-24 16:32) - PLID 28811 - Create or access the topic window; either way we end up calling
				// RepositionDetails in some fashion or another.
				CEmrTopicWndPtr pTopicWnd = pTopic->GetTopicWnd();
				if (pTopicWnd) {
					pTopicWnd->RepositionDetailsInTopicByInfoMasterID(pSrcDetail->m_nEMRInfoMasterID, FALSE, NULL, NULL, TRUE);
				} else {
					CEmrTreeWnd* pTreeWnd = pEMN->GetParentEMR()->GetInterface();
					if (pTreeWnd) {
						NXDATALIST2Lib::IRowSettingsPtr pRow = pTreeWnd->FindInTreeByColumn(TREE_COLUMN_OBJ_PTR, _variant_t((long)pTopic), NULL);
						if (pRow) {
							pTreeWnd->EnsureTopicRow(pRow);
							pTopicWnd = pTopic->GetTopicWnd();
							if (pTopicWnd) {
								pTopicWnd->RepositionDetailsInTopicByInfoMasterID(pSrcDetail->m_nEMRInfoMasterID, FALSE, NULL, NULL, TRUE);
							}
						}
					} else {
						ASSERT(FALSE);
					}
				}
			}
		}

		// (a.walling 2012-06-22 14:01) - PLID 51150 - Explicitly get the topic wnd
		if(pSrcDetail->m_pParentTopic->GetTopicWnd()) {
			// (j.jones 2007-07-26 09:31) - PLID 24686 - renamed this function
			// (c.haag 2011-11-01) - PLID 46223 - Make sure we don't unnecessarily reload the source detail's content or state
			pSrcDetail->m_pParentTopic->GetTopicWnd()->RepositionDetailsInTopicByInfoID(pSrcDetail->m_nEMRInfoID, FALSE, NULL, NULL, FALSE, pSrcDetail);
		}
		//
		// End legacy code
		//

#ifdef _DEBUG
		// (b.cardillo 2009-06-03 17:02) - Make sure all details are for the same patient.  See comments inside loop for more info
		if (apModifiedDetails.GetSize() != 0) {
			long nAllDetailsPatientID = ((CEMNDetail*)apModifiedDetails[0])->m_pParentTopic->GetParentEMN()->GetParentEMR()->GetPatientID();
			for (int i=1; i < apModifiedDetails.GetSize(); i++) {
				// Make sure all modified details are for the same patient, as we rely on that assumption below
				CEMNDetail* pDetail = (CEMNDetail*)apModifiedDetails[i];
				if (pDetail->m_pParentTopic->GetParentEMN()->GetParentEMR()->GetPatientID() != nAllDetailsPatientID) {
					// This should be impossible because, as the code is right now, all details in our array have to be 
					// from the same EMR, which means the same patient.  But it's pretty important for our use of the 
					// arynModifiedEMRDetailIDs array below that all entries be for the same patient, so I'm asserting 
					// here if they're not.  I would like to throw an exception, but we're already in a pretty crazily 
					// rare situation, so if I missed something in my assessment, I prefer to protect EMR data over 
					// wellness qualification data.
					ASSERT(FALSE);
					// Only need to bother the first time there's a mismatch
					break;
				}
			}
		}
#endif

		// (c.haag 2007-01-03 10:01) - PLID 22849 - We now need to update the state in data
		// as well. If we don't do this, and the user cancels out of the EMR, then the old
		// state from data is loaded the next time the user opens the EMR (as opposed to
		// vDefaultState)
		//
		// At this point, we take our detail list and generate a SQL batch out of them
		//
		CDWordArray arynModifiedEMRDetailIDs;
		Nx::Quantum::Batch strSaveString;
		for (int i=0; i < apModifiedDetails.GetSize(); i++) {
			CEMNDetail* pDetail = (CEMNDetail*)apModifiedDetails[i];
			
			// (b.cardillo 2009-06-03 16:47) - PLID 34370 - This detail is going to be re-written to data, so we need to 
			// track it for updating the applicable patient wellness qualification records.
			arynModifiedEMRDetailIDs.Add(pDetail->m_nEMRDetailID);
			
			// Prepare the actual detail data query
			CString strID;
			strID.Format("%d", pDetail->m_nEMRDetailID);
			// (z.manning 2010-02-18 14:56) - PLID 37404 - Added NULL parameter for saved object map as we don't need it for this save
			// (z.manning 2010-03-02 12:50) - PLID 37404 - Also added strPostSaveSql
			// (a.walling 2014-01-30 00:00) - PLID 60543 - Quantize
			Nx::Quantum::Batch strPostSaveSql;
			Nx::Quantum::Batch strDetailString = pDetail->GenerateSaveString_State(strID, arystrErrors, strPostSaveSql, NULL);
			strDetailString += strPostSaveSql;

			if (!strDetailString.IsEmpty()) {
				// Update the main query for saving
				if (strSaveString.IsEmpty()) {
					//TES 8/20/2007 - PLID 25913 - The EMR save code depends on certain variables being declared, so we
					// have to include the declarations in our save string.
					strSaveString += GenerateEMRBatchSaveNewObjectTableDeclaration();

					strSaveString.AddRaw("BEGIN TRY"); // (a.walling 2014-05-01 15:29) - PLID 62008 - begin sql try / catch block
				}

				strSaveString += strDetailString;
			}
		}
		// (c.haag 2007-01-03 10:42) - PLID 22849 - If we have a valid save string, then update the details
		// in data. Notice that there is no auditing here. This is for two reasons: One is that we already
		// audit an item type change in the audit trail in the administrator module. The second is that I
		// tried to use CEMNDetail::GetDetailNameAndDataForAudit on a copy of the detail loaded from data 
		// to get the old value, but the info type has changed, thus already affecting what the audit value
		// would return. That happens well before this function is even called.

		if(arystrErrors.GetSize() > 0) {
			//
			// (c.haag 2007-02-01 16:23) - PLID 22849 - After a very thorough discussion, Josh and I have decided
			// that if there are errors, then we should not concern the user with a warning message. An error simply
			// means we were going to change details on locked EMN's, but did not (as we should not). There's really
			// nothing we can say to the user that wouldn't make them panic for no reason. We will, however, raise
			// a debug assertion and log the error. This should never happen at all; and in my opinion, we need to
			// do a better job of checking whether details or EMN's are locked besides blindly ignoring attempts at
			// changing locked EMN's which should have never been made in the first place.
			//
			CString strWarning = "***An error occured in PostEmrItemEntryDlgItemSaved. Please review the following warnings:***\r\n\r\n";
			for(int i=0;i<arystrErrors.GetSize();i++) {
				strWarning += arystrErrors.GetAt(i);
				Log(arystrErrors.GetAt(i));
				strWarning += "\r\n";
			}
			Log(strWarning);
			Log("***Error list complete***");

			ASSERT(FALSE);
		}

		if (!strSaveString.IsEmpty()) {
			// (a.walling 2014-05-01 15:29) - PLID 62008 - Ensure the transaction is rolled back and rethrow the exception.
			strSaveString.AddRaw("END TRY");
			strSaveString.AddRaw(
				"BEGIN CATCH \r\n"
				"IF @@TRANCOUNT > 0 BEGIN \r\n"
				"ROLLBACK TRANSACTION\r\n"
				"END\r\n"
				"SET NOCOUNT OFF\r\n"
				"DECLARE @errNumber INT, @errMessage NVARCHAR(4000), @errSeverity INT, @errState INT, @errProc NVARCHAR(128), @errLine INT \r\n"
				"SET @errNumber = ERROR_NUMBER(); SET @errMessage = ERROR_MESSAGE(); SET @errSeverity = ERROR_SEVERITY(); SET @errState = ERROR_STATE(); SET @errProc = ERROR_PROCEDURE(); SET @errLine = ERROR_LINE(); \r\n"
				"RAISERROR(N'Caught error %d from line %d in ''%s'': %s', @errSeverity, @errState, @errNumber, @errLine, @errProc, @errMessage) \r\n"
				"RETURN \r\n"
				"END CATCH"
			);

			// Write to data
			// (j.jones 2010-04-20 08:48) - PLID 38273 - converted to a batch execute
			ExecuteWithLog(strSaveString);
			
			// (b.cardillo 2009-06-03 16:47) - PLID 34370 - Reflect any changes to patient wellness qualification records
			if (arynModifiedEMRDetailIDs.GetSize() != 0) {
				// (b.cardillo 2009-07-09 15:32) - PLID 34369 - We used to expect this function to know our 
				// currentuserid, but it can't know that on its own anymore, so we pass it in now.
				UpdatePatientWellness_EMRDetails(GetRemoteData(), arynModifiedEMRDetailIDs, GetCurrentUserID());
			}

			// Let others know this patient's emr has changed
			CClient::RefreshTable(NetUtils::EMRMasterT, pSrcDetail->m_pParentTopic->GetParentEMN()->GetParentEMR()->GetPatientID());
		}

		// (j.jones 2007-08-30 08:55) - PLID 27057 - the following functions need to handle
		//the case where the InterfaceWnd is NULL

		CEmrTreeWnd *pTreeWnd = NULL;
		CEmrTopicWndPtr pTopicWnd = pSrcDetail->m_pParentTopic->GetTopicWnd();
		if(pTopicWnd) {
			pTreeWnd = pTopicWnd->GetEmrTreeWnd();
		}
		else {
			CEMN *pEMN = pSrcDetail->m_pParentTopic->GetParentEMN();
			pTreeWnd = pEMN->GetInterface();
		}

		if(pTreeWnd == NULL) {
			//this should not be possible
			// (a.walling 2008-01-18 12:09) - PLID 14982 - This can be possible when called via a popped up detail
			//ASSERT(FALSE);
		} else {

			// (a.walling 2007-06-18 12:03) - PLID 25549 - Update this detail
			// narratives will be updated in an onstatechanged handler (since removing merge fields causes that event, the field is removed then added.)
			
			// (a.walling 2008-02-07 13:38) - PLID 28857 - At this point, arDetailsToUpdate has all details that may have been modified
			// since the change, except pSrcDetail, so we need to add that and then update them all
			mapDetailsToUpdate[pSrcDetail] = TRUE;

			CMap<CEMRTopic*, CEMRTopic*, BOOL, BOOL> mapTopicsToRefresh;

			POSITION pos = mapDetailsToUpdate.GetStartPosition();
			while (pos) {
				BOOL bDummy;
				CEMNDetail* pDetail;
				mapDetailsToUpdate.GetNextAssoc(pos, pDetail, bDummy);

				if (pDetail != pSrcDetail) {
					pDetail->LoadContent();
				}
				pTreeWnd->UpdateDetailPreview(pDetail);

				if (pDetail->m_pParentTopic) {
					mapTopicsToRefresh[pDetail->m_pParentTopic] = TRUE;
				}

				if (bInfoTypeChanged && (pDetail != pSrcDetail) && (pDetail->m_nEMRInfoID == pSrcDetail->m_nEMRInfoID) ) {
					// (a.walling 2008-03-17 10:40) - PLID 28857 - Set the need content reload flag!!
					pDetail->SetNeedContentReload(TRUE);
				}
			}

			// (a.walling 2008-02-08 09:27) - PLID 28857 - Now refresh visibility on all these topics
			pos = mapTopicsToRefresh.GetStartPosition();
			while (pos) {
				BOOL bDummy;
				CEMRTopic* pTopic;
				mapTopicsToRefresh.GetNextAssoc(pos, pTopic, bDummy);

				pTopic->RefreshHTMLVisibility();
			}

			// (c.haag 2006-04-04 17:28) - PLID 19883 - To make up for the fact we don't call
			// RequestStateChange, we have to explicitly ensure the topic background colors are
			// set.
			// (b.cardillo 2012-03-06 14:56) - PLID 48647 - Have it recalculate completion status, not just 
			// set the color based on existing or calculated status (because it will go with existing, which 
			// could now easily be wrong since we've just changed a bunch above without having called 
			// RequestStateChange()).
			pTreeWnd->EnsureAllTopicBackColors(TRUE);
		}
	}
	NxCatchAll("Error in PostEmrItemAdvDlgItemSaved");
}

//recursively deletes template topics and details
// (a.walling 2014-01-30 00:00) - PLID 60546 - Quantize
void DeleteEMRTemplateTopic(long nEMRTemplateTopicID, Nx::Quantum::Batch &strSqlBatch)
{
	//first delete the details
	// (j.jones 2007-01-12 11:02) - PLID 24027 - moved to DeleteEMRTemplateDetail
	//DRT 8/30/2007 - PLID 27259 - Parameterized.
	_RecordsetPtr rsDetails = CreateParamRecordset("SELECT ID FROM EmrTemplateDetailsT WHERE EMRTemplateTopicID = {INT}", nEMRTemplateTopicID);
	while(!rsDetails->eof) {
		DeleteEMRTemplateDetail(AdoFldLong(rsDetails,"ID"), strSqlBatch);
		rsDetails->MoveNext();
	}
	rsDetails->Close();

	//now the topic
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM EmrTemplateTopicToTopicLinkT WHERE EmrTemplateTopicID = %li", nEMRTemplateTopicID);

	// (j.jones 2006-02-24 09:55) - PLID 19416 - any spawned child template topics need to be deleted
	//TES 3/16/2006 - Clarification: This query is actually getting all topics which are actually just the remembered location
	//of this topic, embedded in some other template.  "Spawned child template topics are now taken care of just below.
	//DRT 8/30/2007 - PLID 27259 - Parameterized.
	_RecordsetPtr rsSpawnedTopics = CreateParamRecordset("SELECT ID FROM EmrTemplateTopicsT WHERE SourceTemplateTopicID = {INT} AND SourceActionID IS NOT NULL", nEMRTemplateTopicID);
	while(!rsSpawnedTopics->eof) {
		DeleteEMRTemplateTopic(AdoFldLong(rsSpawnedTopics,"ID"), strSqlBatch);
		rsSpawnedTopics->MoveNext();
	}
	rsSpawnedTopics->Close();

	//DRT 8/30/2007 - PLID 27259 - Parameterized.
	_RecordsetPtr rsChildTopics = CreateParamRecordset("SELECT ID FROM EmrTemplateTopicsT WHERE EmrParentTemplateTopicID = {INT}", nEMRTemplateTopicID);
	while(!rsChildTopics->eof) {
		DeleteEMRTemplateTopic(AdoFldLong(rsChildTopics,"ID"), strSqlBatch);
		rsChildTopics->MoveNext();
	}
	rsChildTopics->Close();
	
	AddStatementToSqlBatch(strSqlBatch, "UPDATE EmrTemplateTopicsT SET SourceTemplateTopicID = NULL WHERE SourceTemplateTopicID = %li AND SourceActionID Is Null", nEMRTemplateTopicID);
	
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM EmrTemplateTopicsT WHERE ID = %li", nEMRTemplateTopicID);
}

// (j.jones 2007-01-12 10:54) - PLID 24027 - recursively deletes details and topics
// (a.walling 2014-01-30 00:00) - PLID 60546 - Quantize
void DeleteEMRTemplateDetail(long nEMRTemplateDetailID, Nx::Quantum::Batch &strSqlBatch)
{
	//first delete any details that reference this detail as their source
	//DRT 8/30/2007 - PLID 27259 - Parameterized.
	_RecordsetPtr rsDetails = CreateParamRecordset("SELECT ID FROM EmrTemplateDetailsT WHERE SourceDetailID = {INT}", nEMRTemplateDetailID);
	while(!rsDetails->eof) {
		DeleteEMRTemplateDetail(AdoFldLong(rsDetails,"ID"), strSqlBatch);
		rsDetails->MoveNext();
	}
	rsDetails->Close();

	//and delete any topics that use this detail as a source
	//DRT 8/30/2007 - PLID 27259 - Parameterized.
	_RecordsetPtr rsTopics = CreateParamRecordset("SELECT ID FROM EmrTemplateTopicsT WHERE SourceDetailID = {INT}", nEMRTemplateDetailID);
	while(!rsTopics->eof) {
		DeleteEMRTemplateTopic(AdoFldLong(rsTopics,"ID"), strSqlBatch);
		rsTopics->MoveNext();
	}
	rsTopics->Close();

	// (j.dinatale 2012-09-10 14:29) - PLID 52551 - delete map entries when deleting an entire detail
	AddStatementToSqlBatch(strSqlBatch, 
		"DELETE OMRFormDetailT "
		"FROM OMRFormDetailT "
		"INNER JOIN OMRFormT ON OMRFormDetailT.OMRFormID = OMRFormT.ID "
		"WHERE OMRFormT.EmrTemplateID = (SELECT EmrTemplateDetailsT.TemplateID FROM EmrTemplateDetailsT WHERE EmrTemplateDetailsT.ID = %li) "
		"AND OMRFormDetailT.EmrDataGroupID IN ( "
			"SELECT EMRDataT.EMRDataGroupID "
			"FROM EMRTemplateDetailsT "
			"INNER JOIN EMRInfoMasterT ON EMRInfoMasterT.ID = EMRTemplateDetailsT.EmrInfoMasterID "
			"LEFT JOIN EMRInfoT ON EmrInfoMasterT.ActiveEMRInfoID = EmrInfoT.ID "
			"LEFT JOIN EMRDataT ON EmrInfoT.ID = EMRDataT.EMRInfoID "
			"WHERE EMRTemplateDetailsT.ID = %li "
		")", nEMRTemplateDetailID, nEMRTemplateDetailID);

	//now delete this detail
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM EMRTemplateDetailToDetailLinkT WHERE EmrTemplateDetailID = %li", nEMRTemplateDetailID);
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM EMRTemplateTableDefaultsT WHERE EmrTemplateDetailID = %li", nEMRTemplateDetailID);
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM EMRTemplateTableColumnWidthsT WHERE EmrDetailID = %li", nEMRTemplateDetailID);
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM EmrTemplateSelectT WHERE EmrTemplateDetailID = %li", nEMRTemplateDetailID);
	//DRT 1/23/2008 - PLID 28697 - Clear out hotspot data
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM EMRHotSpotTemplateSelectT WHERE EMRDetailID = %li", nEMRTemplateDetailID);
	// (j.jones 2010-02-12 10:52) - PLID 37318 - clear out ChildEMRTemplateDetailIDs referencing this item
	AddStatementToSqlBatch(strSqlBatch, "UPDATE EmrTemplateDetailsT SET ChildEMRTemplateDetailID = NULL WHERE ChildEMRTemplateDetailID = %li", nEMRTemplateDetailID);
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM EmrTemplateDetailsT WHERE ID = %li", nEMRTemplateDetailID);
}

// (a.walling 2009-06-05 12:59) - PLID 34496 - Include parent window
void PromptPatientEMRProblemWarning(long nPatientID, CWnd* pParent)
{
	if (!GetRemotePropertyInt("EMRProblemPromptUserWarning", 1, 0, GetCurrentUserName()))
		return;
	// (c.haag 2006-11-02 09:32) - PLID 21453 - We now require users to have read or read w/ pass
	// permissions to see these warnings
	if (!(GetCurrentUserPermissions(bioEMRProblems) & SPT__R_________ANDPASS))
		return;

	//CArray<CEMN*, CEMN*> aryEMNs;

	// (c.haag 2006-07-03 16:41) - PLID 19977 - I'm intentionally omitting a try/catch clause here
	// because the calling function is expected to catch exceptions over multiple messages
	// (c.haag 2006-10-19 12:08) - PLID 21454 - We now consider deleted problems
	//DRT 3/31/2008 - PLID 29489 - Parameterized with the other warnings.
	// (c.haag 2008-06-16 12:47) - PLID 30319 - Factor in text macro detail names
	// (j.jones 2008-07-16 09:39) - PLID 30739 - supported EMRRegardingType and EMRRegardingID
	// (j.jones 2008-10-30 14:48) - PLID 31869 - added EMNID and DataType
	// (c.haag 2008-12-15 16:17) - PLID 28496 - Added "<None>"
	// (c.haag 2008-12-15 16:51) - PLID 32461 - Added EMRRegardingType (I could have just used the
	// ProblemType field, but if someone changed the description of it, the developer could unwittingly
	// break my code changes)
	// (c.haag 2009-05-12 12:37) - PLID 34234 - Use the new problem link structure
	// (z.manning 2009-06-23 16:57) - PLID 34340 - Labs can now have problems
	// (j.jones 2014-03-24 08:40) - PLID 61506 - if a problem is linked to a diagnosis, show the 9 and 10 code if both exist
	// (a.walling 2014-07-23 09:12) - PLID 63003 - Use CONST_INT for EMRProblemLinkT.EMRRegardingType enums
	// (r.gonet 2015-03-16 13:57) - PLID 65017 - Don't select problems that are flagged as "Don't Show On Problem Prompt"
	_RecordsetPtr prs = CreateParamRecordset(
		"SELECT EMRDetailsT.ID AS EMRDetailID, EMRProblemsT.Description, EMRDetailsT.EMRID AS EMNID, EMRInfoT.DataType, EMRRegardingType, "
		""
		"CASE WHEN (EMRRegardingType = {CONST_INT} OR EMRRegardingType = {CONST_INT}) THEN EMRDetailMaster.Description "
		"WHEN EMRRegardingType = {CONST_INT} THEN EMRTopicMaster.Description "
		"WHEN EMRRegardingType = {CONST_INT} THEN EMRProblemMaster.Description "
		"WHEN EMRRegardingType = {CONST_INT} THEN EMRDiagMaster.Description "
		"WHEN EMRRegardingType = {CONST_INT} THEN EMRChargeMaster.Description "
		"WHEN EMRRegardingType = {CONST_INT} THEN EMRMedicationMaster.Description "
		"ELSE '<None>' END AS EMNName, "
		""
		"CASE WHEN EMRRegardingType = {CONST_INT} THEN 'EMR Item' "
		"WHEN EMRRegardingType = {CONST_INT} THEN 'List Item' "
		"WHEN EMRRegardingType = {CONST_INT} THEN 'Topic' "
		"WHEN EMRRegardingType = {CONST_INT} THEN 'EMN' "
		"WHEN EMRRegardingType = {CONST_INT} THEN 'EMR' "
		"WHEN EMRRegardingType = {CONST_INT} THEN 'Diag Code' "
		"WHEN EMRRegardingType = {CONST_INT} THEN 'Charge' "
		"WHEN EMRRegardingType = {CONST_INT} THEN 'Medication' "
		"WHEN EMRRegardingType = {CONST_INT} THEN 'Lab' "
		"ELSE 'Item' END "
		"AS ProblemType, "
		""
		"CASE WHEN (EMRRegardingType = {CONST_INT} OR EMRRegardingType = {CONST_INT}) THEN "
		"	CASE WHEN MergeOverride IS NULL THEN (CASE WHEN EmrInfoT.ID = " + AsString((long)EMR_BUILT_IN_INFO__TEXT_MACRO) + " THEN EmrDetailsT.MacroName ELSE EmrInfoT.Name END) ELSE MergeOverride END "
		"WHEN EMRRegardingType = {CONST_INT} THEN EMRProblemTopic.Name "
		"WHEN EMRRegardingType = {CONST_INT} THEN EMRProblemMaster.Description "
		"WHEN EMRRegardingType = {CONST_INT} THEN EMRProblemGroup.Description "
		"WHEN EMRRegardingType = {CONST_INT} THEN "
		"	CASE WHEN EMRDiagCodes9.ID Is Not Null AND EMRDiagCodes10.ID Is Not Null THEN EMRDiagCodes10.CodeNumber + ' (' + EMRDiagCodes9.CodeNumber + ')' "
		"	ELSE Coalesce(EMRDiagCodes10.CodeNumber, EMRDiagCodes9.CodeNumber) END "
		"WHEN EMRRegardingType = {CONST_INT} THEN EMRChargesT.Description "
		"WHEN EMRRegardingType = {CONST_INT} THEN DrugDataT.Data "
		"WHEN EMRRegardingType = {CONST_INT} THEN LabsT.FormNumberTextID "
		"WHEN EMRRegardingType = {CONST_INT} THEN '<None>' "
		"ELSE '<Unknown>' END AS ProblemItemName "
		"FROM EMRProblemsT "
		"INNER JOIN EMRProblemLinkT ON EMRProblemLinkT.EMRProblemID = EMRProblemsT.ID "
		""
		"LEFT JOIN EMRDetailsT ON EMRDetailsT.ID = EMRProblemLinkT.EMRRegardingID "
		"LEFT JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
		"LEFT JOIN EMRMasterT AS EMRDetailMaster ON EMRDetailMaster.ID = EMRDetailsT.EMRID "
		""
		"LEFT JOIN EMRTopicsT AS EMRProblemTopic ON EMRProblemLinkT.EMRRegardingID = EMRProblemTopic.ID "
		"LEFT JOIN EMRMasterT AS EMRTopicMaster ON EMRTopicMaster.ID = EMRProblemTopic.EMRID "
		""
		"LEFT JOIN EMRMasterT AS EMRProblemMaster ON EMRProblemMaster.ID = EMRProblemLinkT.EMRRegardingID "
		""
		"LEFT JOIN EMRGroupsT AS EMRProblemGroup ON EMRProblemGroup.ID = EMRProblemLinkT.EMRRegardingID "
		""			
		"LEFT JOIN EMRDiagCodesT ON EMRProblemLinkT.EMRRegardingID = EMRDiagCodesT.ID "
		"LEFT JOIN DiagCodes EMRDiagCodes9 ON EMRDiagCodes9.ID = EMRDiagCodesT.DiagCodeID "
		"LEFT JOIN DiagCodes EMRDiagCodes10 ON EMRDiagCodes10.ID = EMRDiagCodesT.DiagCodeID_ICD10 "
		"LEFT JOIN EMRMasterT AS EMRDiagMaster ON EMRDiagMaster.ID = EMRDiagCodesT.EMRID "
		""			
		"LEFT JOIN EMRChargesT ON EMRProblemLinkT.EMRRegardingID = EMRChargesT.ID "
		"LEFT JOIN EMRMasterT AS EMRChargeMaster ON EMRChargeMaster.ID = EMRChargesT.EMRID "
		""
		"LEFT JOIN EMRMedicationsT ON EMRProblemLinkT.EMRRegardingID = EMRMedicationsT.MedicationID "
		"LEFT JOIN PatientMedications ON EMRMedicationsT.MedicationID = PatientMedications.ID "
		"LEFT JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID "
		"LEFT JOIN EMRDataT AS DrugDataT ON DrugList.EMRDataID = DrugDataT.ID "
		"LEFT JOIN EMRMasterT AS EMRMedicationMaster ON EMRMedicationMaster.ID = EMRMedicationsT.EMRID "
		""
		"LEFT JOIN LabsT ON EmrProblemLinkT.EmrRegardingID = LabsT.ID "
		""
		"WHERE EMRProblemsT.PatientID = {INT} AND StatusID <> 2 AND EMRProblemsT.Deleted = 0 "
		"AND EMRProblemsT.DoNotShowOnProblemPrompt = 0 ",
		eprtEmrItem, eprtEmrDataItem, eprtEmrTopic, eprtEmrEMN, eprtEmrDiag, eprtEmrCharge, eprtEmrMedication,
		eprtEmrItem, eprtEmrDataItem, eprtEmrTopic, eprtEmrEMN, eprtEmrEMR, eprtEmrDiag, eprtEmrCharge, eprtEmrMedication, eprtLab,
		eprtEmrItem, eprtEmrDataItem, eprtEmrTopic, eprtEmrEMN, eprtEmrEMR, eprtEmrDiag, eprtEmrCharge, eprtEmrMedication, eprtLab, eprtUnassigned,
		nPatientID); // A status ID of 2 means closed
	if (!prs->eof) {
		CString strMessage = "This patient has the following EMR problems:\n\n";
		FieldsPtr f = prs->Fields;
		const long nMaxWarnings = 3;
		long i = 0;
		while (!prs->eof) {
			CString strEMNName = AdoFldString(f, "EMNName", "");
			CString strProblemType = AdoFldString(f, "ProblemType", "");
			CString strProblemItemName = AdoFldString(f, "ProblemItemName", "");
			CString strDescription = AdoFldString(f, "Description", "");
			EMRProblemRegardingTypes RegardingType = (EMRProblemRegardingTypes)AdoFldLong(f, "EMRRegardingType");
			CString strSentence;
			CString str;
			// Load the detail
			// (c.haag 2008-12-15 16:41) - PLID 32461 - Only load if the regarding ID points to a detail
			long nDetailID = (eprtEmrItem == RegardingType || eprtEmrDataItem == RegardingType) ? AdoFldLong(f, "EMRDetailID", -1) : -1;
			if(nDetailID != -1) {

				long nEMNID = AdoFldLong(f, "EMNID", -1);
				EmrInfoType eDataType = (EmrInfoType)AdoFldByte(f, "DataType", eitInvalid);

				// (j.jones 2008-10-30 14:52) - PLID 31869 - If we have a Narrative or a Table,
				// we need to load the entire EMN, otherwise our results won't be reliable
				// (narratives won't show their linked items, tables won't show linked details).
				// (j.jones 2009-06-19 15:57) - PLID 34596 - for the problem warning we are not
				// going to show the contents of narratives or tables
				
				/*
				//first see if we already have this EMN
				BOOL bFound = FALSE;
				CEMN *pEMN = NULL;
				for(i=0; i<aryEMNs.GetSize() && pEMN == NULL; i++) {
					CEMN *pEMNToCheck = (CEMN*)aryEMNs.GetAt(i);
					if(pEMNToCheck->GetID() == nEMNID) {
						pEMN = pEMNToCheck;
					}
				}

				//now if we do not have this EMN already, but this is a Narrative or Table,
				//create the EMN and add it to our array
				if(pEMN == NULL && (eDataType == eitNarrative || eDataType == eitTable)) {
					pEMN = new CEMN(NULL);
					pEMN->LoadFromEmnID(nEMNID);
					aryEMNs.Add(pEMN);
				}

				CEMNDetail *pDetail = NULL;
				// (j.jones 2008-10-30 14:52) - PLID 31869 - if we have an EMN, just find the already-loaded detail in it
				if(pEMN) {
					pDetail = pEMN->GetDetailByID(nDetailID);
				}
				*/

				CEMNDetail *pDetail = NULL;

				//if we have no detail, load it now, which is way faster
				//than always loading the EMN
				BOOL bIsLocal = FALSE;
				if(pDetail == NULL) {
					// (a.walling 2009-10-23 09:23) - PLID 36046 - Track construction in initial reference count
					pDetail = CEMNDetail::CreateDetail(NULL, "PromptPatientEMRProblemWarning local detail");
					// Load the detail
					pDetail->LoadFromDetailID(nDetailID, NULL);
					bIsLocal = TRUE;
				}

				// (m.hancock 2006-09-12 14:26) - PLID 22261 - Images marked as problems can show "<Missing Image>" as the merge name
				// when in reality, the "<Missing Image>" should not be displayed at all.  Furthermore, the merge name is included in
				// the field DetailName in the query to load the problems that should be displayed.  So, if we're displaying
				// information about an image detail, then we should not call GetSentence(), which will ultimately return 
				// "<Missing Image>" from GetDataOutput() because we are not attempting to render the image as we would under normal 
				// conditions for GetDataOutput().
				// (j.jones 2009-06-19 15:59) - PLID 34596 - if an image, table, or narrative, just show the type, and not content
				if(pDetail->m_EMRInfoType == eitImage) {
					strSentence = "<Image>";
				}
				else if(pDetail->m_EMRInfoType == eitNarrative) {
					strSentence = "<Narrative>";
				}
				else if(pDetail->m_EMRInfoType == eitTable) {
					strSentence = "<Table>";
				}
				else {			
					// Get the detail value in sentence form
					// (c.haag 2006-07-03 15:44) - Copied from CEmrItemAdvDlg::GetToolTipText()
					// (c.haag 2007-05-17 10:20) - PLID 26046 - Use GetStateVarType to get the detail state type
					if(pDetail->GetStateVarType() == VT_NULL || (pDetail->GetStateVarType() == VT_BSTR && VarString(pDetail->GetState()).IsEmpty())) {
						strSentence = pDetail->m_strLongForm;
					} else {
						CStringArray saDummy;
						strSentence = ::GetSentence(pDetail, NULL, false, false, saDummy, ecfParagraph);
					}
				}

				// (j.jones 2008-10-30 14:53) - PLID 31869 - We don't need to delete detail if it was from the EMN,
				// because it would be handled when the EMN is deleted. But we may have created this detail on our
				//own, and if so, we need to delete it now.
				// (j.jones 2009-06-19 15:58) - PLID 34596 - right now the detail is always local
				if(bIsLocal) {
					// (a.walling 2009-10-12 17:20) - PLID 36024 - Properly release the detail
					pDetail->__QuietRelease();
					//delete pDetail;
					pDetail = NULL;
				}
			}
			// Format the message
			// (c.haag 2008-12-15 16:45) - PLID 32461 - If this is an EMN problem, don't show the description twice!
			if (eprtEmrEMN == RegardingType) {
				str.Format("EMN: %s", strEMNName);
			}
			else if (eprtUnassigned == RegardingType) {
				// (c.haag 2008-12-17 16:16) - PLID 28496 - Special text for problems not assigned to EMR's
				str = "(EMR-Independent problem)";
			}
			else if (eprtLab == RegardingType) {
				// (z.manning 2009-06-23 17:04) - PLID 34340 - Lab problems
				str.Format("%s: %s", strProblemType, strProblemItemName);
			}
			else {
				str.Format("EMN: %s\n%s: %s", strEMNName, strProblemType, strProblemItemName);
			}

			if (!strSentence.IsEmpty()) {
				str += " - " + strSentence;
			} 
			if (!strDescription.IsEmpty()) {
				str += "\nDescription: " + strDescription;
			}
			strMessage += str + "\n\n";
			prs->MoveNext();

			if (++i == nMaxWarnings && !prs->eof) {
				strMessage += "<More problems omitted>\n";
				break;
			}
		}

		// (a.walling 2008-05-02 09:35) - PLID 29842 - Use the new problem flag icon in the messagebox
		// (a.walling 2009-06-05 12:59) - PLID 34496 - Include parent window
		MSGBOXPARAMS mp;
		::ZeroMemory(&mp, sizeof(MSGBOXPARAMS));
		mp.cbSize = sizeof(MSGBOXPARAMS);
		mp.hwndOwner = pParent->GetSafeHwnd() ? pParent->GetSafeHwnd() : AfxGetMainWnd()->GetSafeHwnd();
		mp.hInstance = AfxGetApp()->m_hInstance;
		mp.lpszText = strMessage;
		mp.lpszCaption = "EMR Problems";
		mp.dwStyle = MB_USERICON | MB_OK;
		mp.lpszIcon = MAKEINTRESOURCE(IDI_EMR_PROBLEM_FLAG);
		::MessageBoxIndirect((LPMSGBOXPARAMS)&mp);

		//::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), strMessage, "EMR Problems", MB_ICONINFORMATION);
	}

	// (j.jones 2008-10-30 14:54) - PLID 31869 - we now have to delete any EMNs we loaded
	// (j.jones 2009-06-19 16:00) - PLID 34596 - we don't load EMNs anymore
	/*
	int i=0;
	for(i=aryEMNs.GetSize() - 1; i>=0; i--) {
		CEMN *pEMN = (CEMN*)aryEMNs.GetAt(i);
		delete pEMN;
	}
	aryEMNs.RemoveAll();
	*/
}

//TES 12/5/2006 - PLID 23724 - This function is no longer needed with the new data structure, the query can just check EmrInfoMasterID.
//recursively generates a list of this Info ID plus all previous versions of that Info ID
/*CString GeneratePastEMRInfoIDs(long nEmrInfoID)
{	
	/*CString strPastIDs = AsString(nEmrInfoID);
	_RecordsetPtr rs = CreateRecordset("SELECT CopiedFromInfoID FROM EMRInfoT WHERE ID = %li AND CopiedFromInfoID Is Not Null", nEmrInfoID);
	if(!rs->eof) {
		long nCopiedFromInfoID = AdoFldLong(rs, "CopiedFromInfoID",-1);
		if(nCopiedFromInfoID != -1) {
			strPastIDs += ",";
			strPastIDs += GeneratePastEMRInfoIDs(nCopiedFromInfoID);
		}
	}
	rs->Close();

	return strPastIDs;
}
*/

//TES 12/6/2006 - PLID 23766 - This function is no longer needed with the new data structure, the query can just check EmrDataGroupID
//recursively generates a list of this Data ID plus all previous versions of that Data ID
/*CString GeneratePastEMRDataIDs(long nEmrDataID)
{
	CString strPastIDs = AsString(nEmrDataID);
	_RecordsetPtr rs = CreateRecordset("SELECT CopiedFromDataID FROM EMRDataT WHERE ID = %li AND CopiedFromDataID Is Not Null", nEmrDataID);
	if(!rs->eof) {
		long nCopiedFromDataID = AdoFldLong(rs, "CopiedFromDataID",-1);
		if(nCopiedFromDataID != -1) {
			strPastIDs += ",";
			strPastIDs += GeneratePastEMRDataIDs(nCopiedFromDataID);
		}
	}
	rs->Close();

	return strPastIDs;
}*/

//returns true if the datatypes are the same or if the types are both lists
// (j.jones 2007-07-23 09:16) - PLID 26773 - added connection pointer for use in threads
// (z.manning 2011-10-20 13:32) - PLID 44649 - Added data sub type param
BOOL AreInfoIDTypesCompatible(long nOldEMRInfoID, long nNewEMRInfoID, EmrInfoType eitNewInfoDataType, EmrInfoSubType eNewInfoDataSubType, OPTIONAL IN ADODB::_Connection *lpCon /*= NULL*/)
{
	_ConnectionPtr pCon;
	if(lpCon) pCon = lpCon;
	else pCon = GetRemoteData();

	long nOldDataType = -1;
	long nOldDataSubType = 1;

	//DRT 8/30/2007 - PLID 27259 - Parameterized.
	// (z.manning 2011-10-20 13:34) - PLID 44649 - Added data sub type
	_RecordsetPtr rs = CreateParamRecordset(pCon, "SELECT DataType, DataSubType FROM EMRInfoT WHERE ID = {INT}", nOldEMRInfoID);
	if(!rs->eof) {
		nOldDataType = AdoFldByte(rs, "DataType",-1);
		nOldDataSubType = AdoFldByte(rs, "DataSubType", -1);
	}
	rs->Close();

	//DRT 9/7/2007 - PLID 27330 - We already know the new data type before we ever started, no sense looking it up again.
	/*
	//DRT 8/30/2007 - PLID 27259 - Parameterized.
	rs = CreateParamRecordset(pCon, "SELECT DataType FROM EMRInfoT WHERE ID = {INT}", nNewEMRInfoID);
	if(!rs->eof) {
		nNewDataType = AdoFldByte(rs, "DataType",-1);
	}
	rs->Close();
	*/

	//if old is single-select and new is multi-select, then they are compatible
	if(eitNewInfoDataType == eitMultiList && nOldDataType == eitSingleList
		|| eitNewInfoDataType == eitSingleList && nOldDataType == eitMultiList) {
		return TRUE;
	}

	//otherwise, the types need to be identical
	return IsSameEmrInfoType((EmrInfoType)nOldDataType, (EmrInfoSubType)nOldDataSubType, eitNewInfoDataType, eNewInfoDataSubType);
}

// (z.manning 2011-10-21 14:44) - PLID 44649 - This determines if an info item is the same type (which is no
// longer as simple as comparing EmrInfoT.DataType). This is not the same as AreInfoIDTypesCompatiblel
BOOL IsSameEmrInfoType(const EmrInfoType eOldDataType, const EmrInfoSubType eOldDataSubType, const EmrInfoType eNewDataType, const EmrInfoSubType eNewDataSubType)
{
	if(eOldDataType == eNewDataType)
	{
		switch(eNewDataType)
		{
			case eitImage:
				// (z.manning 2011-10-20 13:39) - PLID 44649 - If this is an image then the data sub type must match too
				// since 2D and 3D images are not compatible.
				return (eNewDataSubType == eOldDataSubType);
				break;
		}

		return TRUE;
	}
	else {
		return FALSE;
	}
}

//given an EMRInfoID and data ID, map it up to the newest version of that Info ID and data ID
// (j.jones 2007-07-23 09:16) - PLID 26773 - added connection pointer for use in threads
long CalculateMappedDataID(long nOldEMRDataID, long nNewEMRInfoID, OPTIONAL IN ADODB::_Connection *lpCon /*= NULL*/)
{
	_ConnectionPtr pCon;
	if(lpCon) pCon = lpCon;
	else pCon = GetRemoteData();

	//TES 12/5/2006 - PLID 23766 - This is now a simple query.
	//DRT 8/30/2007 - PLID 27259 - Parameterized.
	_RecordsetPtr rsMappedID = CreateParamRecordset(pCon, "SELECT ID FROM EmrDataT WHERE EmrInfoID = {INT} "
		"AND EmrDataGroupID = (SELECT EmrDataGroupID FROM EmrDataT OldData WHERE OldData.ID = {INT})", nNewEMRInfoID, nOldEMRDataID);
	if(rsMappedID->eof) {
		//That data item no longer exists in this version.
		return -1;
	}
	else {
		return AdoFldLong(rsMappedID, "ID");
	}
	/*//this will recursively "upgrade" info IDs until we find the right one
	_RecordsetPtr rs = CreateRecordset("SELECT ID FROM EMRInfoT WHERE CopiedFromInfoID = %li", nOldEMRInfoID);
	if(!rs->eof) {

		if(rs->GetRecordCount() > 1) {
			//if more than one record matches, then multiple copies of the item was made
			//and we have to track the Data ID matching up the correct path

			CString strAllPreviousInfoIDs = GeneratePastEMRInfoIDs(nNewEMRInfoID);

			BOOL bFound = FALSE;

			while(!rs->eof && !bFound) {

				CString strPreviousInfoIDs = strAllPreviousInfoIDs;

				long nFoundEMRInfoID = AdoFldLong(rs, "ID",-1);
				while(!strPreviousInfoIDs.IsEmpty() && !bFound) {
					long nComma = strPreviousInfoIDs.Find(",");
					CString strID;
					if(nComma == -1) {
						strID = strPreviousInfoIDs;
						strPreviousInfoIDs = "";
					}
					else {
						strID = strPreviousInfoIDs.Left(nComma);
						strPreviousInfoIDs = strPreviousInfoIDs.Right(strPreviousInfoIDs.GetLength() - nComma - 1);
					}
					strPreviousInfoIDs.TrimLeft();
					strPreviousInfoIDs.TrimRight();

					if(atoi(strID) == nFoundEMRInfoID) {
						bFound = TRUE;
					}
				}

				if(!bFound) {
					rs->MoveNext();
				}
			}

			//presumably when we get here, we are currently looking at the next correct EMRInfoT record
			//in the path leading up to our "current" EMRInfoT record

			if(rs->eof) {
				//we should never get here
				ASSERT(FALSE);
				return -1;
			}
		}

		long nFoundEMRInfoID = AdoFldLong(rs, "ID",-1);

		//now map the data ID to the Found ID
		long nFoundEMRDataID = -1;
		_RecordsetPtr rsData = CreateRecordset("SELECT ID FROM EMRDataT WHERE CopiedFromDataID = %li AND EMRInfoID = %li", nOldEMRDataID, nFoundEMRInfoID);
		if(!rsData->eof) {
			nFoundEMRDataID = AdoFldLong(rsData, "ID",-1);
		}
		else {
			//we might get here if the selected item had been deleted between versions
			return -1;
		}
		rsData->Close();

		//now, is this the Info ID we want? if not, recurse
		if(nFoundEMRInfoID == nNewEMRInfoID) {
			return nFoundEMRDataID;
		}
		else {
			return CalculateMappedDataID(nFoundEMRInfoID, nFoundEMRDataID, nNewEMRInfoID);
		}

	}
	rs->Close();

	//we should never get here
	ASSERT(FALSE);
	return -1;*/
}

//given an EMRInfoID and table dropdown ID, map it up to the newest version of that Info ID and dropdown ID
// (j.jones 2007-07-23 09:16) - PLID 26773 - added connection pointer for use in threads
long CalculateMappedDropdownID(long nOldDropdownID, long nNewEMRDataID_Y, OPTIONAL IN ADODB::_Connection *lpCon /*= NULL*/)
{
	// (c.haag 2008-12-03 14:59) - PLID 32300 - Always return -1 if either ID is not positive (I don't see how
	// nNewEMRDataID_Y would ever fail this test, but better to be safe than sorry)
	if (nNewEMRDataID_Y <= 0 || nOldDropdownID <= 0) {
		return -1;
	}

	_ConnectionPtr pCon;
	if(lpCon) pCon = lpCon;
	else pCon = GetRemoteData();

	//TES 12/5/2006 - PLID 23766 - This is now a simple query.
	//DRT 8/30/2007 - PLID 27259 - Parameterized.
	_RecordsetPtr rsMappedID = CreateParamRecordset(pCon, "SELECT ID FROM EmrTableDropdownInfoT WHERE EmrDataID = {INT} "
		"AND DropdownGroupID = (SELECT DropdownGroupID FROM EmrTableDropdownInfoT OldDropdown WHERE OldDropdown.ID = {INT})", nNewEMRDataID_Y, nOldDropdownID);
	if(rsMappedID->eof) {
		//That data item no longer exists in this version.
		return -1;
	}
	else {
		return AdoFldLong(rsMappedID, "ID");
	}

	/*//this will recursively "upgrade" info IDs until we find the right one
	_RecordsetPtr rs = CreateRecordset("SELECT ID FROM EMRInfoT WHERE CopiedFromInfoID = %li", nOldEMRInfoID);
	if(!rs->eof) {

		if(rs->GetRecordCount() > 1) {
			//if more than one record matches, then multiple copies of the item was made
			//and we have to track the Data ID matching up the correct path

			CString strAllPreviousInfoIDs = GeneratePastEMRInfoIDs(nNewEMRInfoID);

			BOOL bFound = FALSE;

			while(!rs->eof && !bFound) {

				CString strPreviousInfoIDs = strAllPreviousInfoIDs;

				long nFoundEMRInfoID = AdoFldLong(rs, "ID",-1);
				while(!strPreviousInfoIDs.IsEmpty() && !bFound) {
					long nComma = strPreviousInfoIDs.Find(",");
					CString strID;
					if(nComma == -1) {
						strID = strPreviousInfoIDs;
						strPreviousInfoIDs = "";
					}
					else {
						strID = strPreviousInfoIDs.Left(nComma);
						strPreviousInfoIDs = strPreviousInfoIDs.Right(strPreviousInfoIDs.GetLength() - nComma - 1);
					}
					strPreviousInfoIDs.TrimLeft();
					strPreviousInfoIDs.TrimRight();

					if(atoi(strID) == nFoundEMRInfoID) {
						bFound = TRUE;
					}
				}

				if(!bFound) {
					rs->MoveNext();
				}
			}

			//presumably when we get here, we are currently looking at the next correct EMRInfoT record
			//in the path leading up to our "current" EMRInfoT record

			if(rs->eof) {
				//we should never get here
				ASSERT(FALSE);
				return -1;
			}
		}

		long nFoundEMRInfoID = AdoFldLong(rs, "ID",-1);

		//now map the data ID to the Found ID
		long nFoundEMRDataID = -1;
		_RecordsetPtr rsData = CreateRecordset("SELECT ID FROM EMRDataT WHERE CopiedFromDataID = %li AND EMRInfoID = %li", nOldEMRDataID_Y, nFoundEMRInfoID);
		if(!rsData->eof) {
			nFoundEMRDataID = AdoFldLong(rsData, "ID",-1);
		}
		else {
			//we might get here if the selected item had been deleted between versions
			return -1;
		}
		rsData->Close();

		//and map the dropdown ID to the Found ID
		long nFoundEMRDropdownID = -1;
		_RecordsetPtr rsDropdown = CreateRecordset("SELECT ID FROM EMRTableDropdownInfoT WHERE CopiedFromDropdownID = %li AND EMRDataID = %li", nOldDropdownID, nFoundEMRDataID);
		if(!rsDropdown->eof) {
			nFoundEMRDropdownID = AdoFldLong(rsDropdown, "ID",-1);
		}
		else {
			//we might get here if the selected dropdown item had been deleted between versions
			return -1;
		}
		rsDropdown->Close();

		//now, is this the Info ID we want? if not, recurse
		if(nFoundEMRInfoID == nNewEMRInfoID) {
			return nFoundEMRDropdownID;
		}
		else {
			return CalculateMappedDropdownID(nFoundEMRInfoID, nFoundEMRDataID, nFoundEMRDropdownID, nNewEMRInfoID, nNewEMRDataID_Y);
		}

	}
	rs->Close();

	//we should never get here
	ASSERT(FALSE);
	return -1;*/
}

// (b.cardillo 2006-11-20 10:37) - PLID 22565 - Determine the correct color associated with each possible 
// review state value, or -1 if there is no color (i.e. clear).
//DRT 6/4/2008 - PLID 30269 - GetReconstructedEMRDetailReviewStateBrush() now mirrors this, any changes
//	need applied there.
COLORREF GetReconstructedEMRDetailReviewStateColor(long nReviewStatus)
{
	COLORREF clrReview = -1;
	switch (nReviewStatus) {
	case -1:
		// Reconstructed, unverified
		// LIGHT PINK
		return PaletteColor(RGB(255,230,230));
		break;
	case -2:
		// Reconstructed, verified, and keep showing highlighted
		// WHITE
		return PaletteColor(RGB(255,255,255));
		break;
	case -3:
		// Reconstructed, verified, and no more reminder, so no more special color
		return -1;
		break;
	case 0:
		// Clean data, no reconstruction so no special color to begin with
		return -1;
		break;
	default:
		// This is unexpected so color it red because this situation is akin to an exception.
		ASSERT(FALSE);
		return PaletteColor(RGB(255,0,0));
		break;
	}
}

//DRT 6/4/2008 - PLID 30269 - Goes along with the above.  We need to store the brushes so we don't
//	leak resources if they are used.  Since these are extremely rarely used (only 3 or 4 clients can 
//	possibly have this data), I'm making 3 pointers to correspond with the brush colors above.
//	This way, memory is only allocated if we use it
CBrush g_brReviewStateLightPink(PaletteColor(RGB(255,230,230)));
CBrush g_brReviewStateWhite(PaletteColor(RGB(255,255,255)));
CBrush g_brReviewStateRed(PaletteColor(RGB(255,0,0)));
HBRUSH GetReconstructedEMRDetailReviewStateBrush(long nReviewStatus)
{
	//This follow the same as the GetReconstructedEMRDetailReviewStateColor(), any changes need applied to both.
	COLORREF clrReview = -1;
	switch (nReviewStatus) {
	case -1:
		// Reconstructed, unverified
		// LIGHT PINK
		return (HBRUSH)g_brReviewStateLightPink;
		break;
	case -2:
		// Reconstructed, verified, and keep showing highlighted
		// WHITE
		return (HBRUSH)g_brReviewStateWhite;
		break;
	case -3:
		// Reconstructed, verified, and no more reminder, so no more special color
		return (HBRUSH)GetStockObject(NULL_BRUSH);
		break;
	case 0:
		// Clean data, no reconstruction so no special color to begin with
		return (HBRUSH)GetStockObject(NULL_BRUSH);
		break;
	default:
		// This is unexpected so color it red because this situation is akin to an exception.
		ASSERT(FALSE);
		return (HBRUSH)g_brReviewStateRed;
		break;
	}
}

//determines if the source action ID from the detail is the same as the passed in action ID
//or whether the previous version of the action's spawning item is the item that spawns the detail
// (j.jones 2013-07-15 10:51) - PLID 57243 - changed the action to a reference
BOOL IsDetailSourceActionIDEquivalent(CEMNDetail *pDetail, const EmrAction &ea)
{	
	//first just see if the detail's action ID is correct
	if(pDetail->GetSourceActionID() == ea.nID)
		return TRUE;

	//does the detail even have an action ID?
	if(pDetail->GetSourceActionID() == -1)
		return FALSE;

	//if not, find out the source of this action
	if(ea.eaoSourceType == eaoEmrItem) {

		//TES 12/7/2006 - Hey!  This would be bad data! (eaoEmrItem SourceTypes can't spawn items!)
		/*//see if any previous version of that info item spawns this detail
		
		//TES 12/5/2006 - PLID 23724 - This can now be done within the query.
		*//*CString strPastItemIDs = GeneratePastEMRInfoIDs(ea.nSourceID);
		//if only one number, then there are no past versions to check
		if(strPastItemIDs.Find(",") == -1) {
			return FALSE;
		}*//*
		
		//if there is an action for a past version of the spawning item, that matches up with this
		//detail's source action ID, then return true
		return ReturnsRecords("SELECT ID FROM EMRActionsT WHERE SourceType = %li AND SourceID IN (SELECT ID FROM EmrInfoT "
			" WHERE EmrInfoMasterID = (SELECT EmrInfoMasterID FROM EmrInfoT SourceItem WHERE SourceItem.ID = %li) ) "
			"AND DestType = %li AND ID = %li", eaoEmrItem, ea.nSourceID, eaoEmrItem, pDetail->GetSourceActionID());*/
	}
	else if(ea.eaoSourceType == eaoEmrDataItem) {

		//see if any previous version of that data item spawns this detail
		
		//TES 12/5/2006 - PLID 23766 - This can now be done within the query.
		/*CString strPastItemIDs = GeneratePastEMRDataIDs(ea.nSourceID);
		//if only one number, then there are no past versions to check
		if(strPastItemIDs.Find(",") == -1) {
			return FALSE;
		}*/

		//if there is an action for a past version of the spawning item, that matches up with this
		//detail's source action ID, then return true
		// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
		return ReturnsRecordsParam(FormatString("SELECT ID FROM EMRActionsT WHERE SourceType = %li AND SourceID IN (SELECT ID FROM EmrDataT "
			" WHERE EmrDataGroupID = (SELECT EmrDataGroupID FROM EmrDataT SourceItem WHERE SourceItem.ID = {INT}) ) "
			"AND DestType = %li AND ID = {INT}", eaoEmrDataItem, eaoEmrItem), ea.nSourceID, pDetail->GetSourceActionID());
	}
	else {
		//sorry, can't help ya
		return FALSE;
	}

	return FALSE;
}

//determines if the source action ID from the topic is the same as the passed in action ID
//or whether the previous version of the action's spawning item is the item that spawns the topic
// (j.jones 2013-07-15 10:51) - PLID 57243 - changed the action to a reference
BOOL IsTopicSourceActionIDEquivalent(CEMRTopic *pTopic, const EmrAction &ea)
{
	//first just see if the topic's action ID is correct
	if(pTopic->GetSourceActionID() == ea.nID)
		return TRUE;

	//does the topic even have an action ID?
	if(pTopic->GetSourceActionID() == -1)
		return FALSE;

	//if not, find out the source of this action
	if(ea.eaoSourceType == eaoEmrItem) {

		//TES 12/7/2006 - Hey!  This would be bad data! (eaoEmrItem SourceTypes can't spawn topics!)
		/*//see if any previous version of that info item spawns this topic
		
		//TES 12/5/2006 - PLID 23724 - This can now be done within the query.
		*//*CString strPastItemIDs = GeneratePastEMRInfoIDs(ea.nSourceID);
		CString strPastItemIDs = GeneratePastEMRInfoIDs(ea.nSourceID);
		//if only one number, then there are no past versions to check
		if(strPastItemIDs.Find(",") == -1) {
			return FALSE;
		}*//*
		
		//if there is an action for a past version of the spawning item, that matches up with this
		//topic's source action ID, then return true
		return ReturnsRecords("SELECT ID FROM EMRActionsT WHERE SourceType = %li AND SourceID IN ((SELECT ID FROM EmrInfoT "
			" WHERE EmrInfoMasterID = (SELECT EmrInfoMasterID FROM EmrInfoT SourceItem WHERE SourceItem.ID = %li) ) "
			"AND DestType = %li AND ID = %li", eaoEmrItem, ea.nSourceID, eaoMintItems, pTopic->GetSourceActionID());*/
	}
	else if(ea.eaoSourceType == eaoEmrDataItem) {

		//see if any previous version of that data item spawns this topic
		
		//TES 12/5/2006 - PLID 23766 - This can now be done within the query.
		/*CString strPastItemIDs = GeneratePastEMRDataIDs(ea.nSourceID);
		//if only one number, then there are no past versions to check
		if(strPastItemIDs.Find(",") == -1) {
			return FALSE;
		}*/

		//if there is an action for a past version of the spawning item, that matches up with this
		//topic's source action ID, then return true
		// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
		return ReturnsRecordsParam(FormatString("SELECT ID FROM EMRActionsT WHERE SourceType = %li AND SourceID IN (SELECT ID FROM EmrDataT "
			" WHERE EmrDataGroupID = (SELECT EmrDataGroupID FROM EmrDataT SourceItem WHERE SourceItem.ID = {INT}) ) "
			"AND DestType = %li AND ID = {INT}", eaoEmrDataItem, eaoMintItems), ea.nSourceID, pTopic->GetSourceActionID());
	}
	else {
		//sorry, can't help ya
		return FALSE;
	}

	return FALSE;
}

//DRT 1/17/2008 - PLID 28602 - I pulled the central Action XML bits out of GenerateXML.  We now need to generate
//	this for both EMRDataT records and EMRImageHotSpotsT records.
// (z.manning 2009-02-11 15:18) - PLID 33029 - Moved to EmrUtils from EmrItemEntryDlg
// (a.walling 2014-07-08 14:19) - PLID 62812 - Use MFCArray
CString GenerateActionXml(IN const MFCArray<EmrAction> *pActions, IN OUT long &nArbitraryActionIDValue)
{
	CString strActionXml;
	for(int i = 0; i < pActions->GetSize(); i++) {
		//DRT 1/17/2007 - PLID 24181 - To put my charge action level in (labeled as 'C'), I had to change this from 
		//	<A ... /> to <A ... > </A>.  Also added the ArbVal type, similar to the one used in the EMRDataT record.  This
		//	value should only be used by sub-items (like charge actions)
		//TES 1/30/2007 - PLID 24474 - Don't forget SpawnAsChild!
		strActionXml += FormatString("<A ID=\"%li\" DestType=\"%li\" DestID=\"%li\" SortOrder=\"%li\" Popup=\"%i\" SpawnAsChild=\"%i\" ArbVal=\"%li\">\r\n", 
			pActions->GetAt(i).nID, pActions->GetAt(i).eaoDestType, pActions->GetAt(i).nDestID, pActions->GetAt(i).nSortOrder, pActions->GetAt(i).bPopup?1:0, pActions->GetAt(i).bSpawnAsChild?1:0, 
			nArbitraryActionIDValue);

		//DRT 1/17/2007 - PLID 24181 - We additionally need to generate the XML for EMRChargeActionDataT, ONLY if it's a charge type
		if(pActions->GetAt(i).eaoDestType == eaoCpt) {
			CString strMod1, strMod2, strMod3, strMod4;
			if(!pActions->GetAt(i).strMod1.IsEmpty()) strMod1 = FormatString("Mod1=\"%s\"", pActions->GetAt(i).strMod1);
			if(!pActions->GetAt(i).strMod2.IsEmpty()) strMod2 = FormatString("Mod2=\"%s\"", pActions->GetAt(i).strMod2);
			if(!pActions->GetAt(i).strMod3.IsEmpty()) strMod3 = FormatString("Mod3=\"%s\"", pActions->GetAt(i).strMod3);
			if(!pActions->GetAt(i).strMod4.IsEmpty()) strMod4 = FormatString("Mod4=\"%s\"", pActions->GetAt(i).strMod4);
			strActionXml += FormatString("<C ID=\"%li\" Prompt=\"%li\" Quantity=\"%g\" %s %s %s %s/>\r\n", pActions->GetAt(i).nID, pActions->GetAt(i).bPrompt, 
				pActions->GetAt(i).dblDefaultQuantity, strMod1, strMod2, strMod3, strMod4);
		}

		// (c.haag 2008-06-20 11:53) - PLID 30221 - Todo alarms
		if(pActions->GetAt(i).eaoDestType == eaoTodo) {
			CString strNotes;
			CString strCategoryID;
			const EmrAction& ea = pActions->GetAt(i);

			if(ea.strTodoNotes.IsEmpty()) { strNotes = "";	} else { strNotes.Format("Notes=\"%s\"", ConvertToQuotableXMLString(ea.strTodoNotes)); }
			if(-1 == ea.nTodoCategoryID) { strCategoryID = "";	} else { strCategoryID.Format("CategoryID=\"%d\"", ea.nTodoCategoryID); }

			strActionXml += FormatString("<Q ID=\"%li\" RemindType=\"%d\" RemindInterval=\"%d\" "
				"DeadlineType =\"%d\" DeadlineInterval =\"%d\" Priority =\"%d\" "
				"Task =\"%s\" %s %s />\r\n", 
				ea.nID, ea.nTodoRemindType, ea.nTodoRemindInterval,
				ea.nTodoDeadlineType, ea.nTodoDeadlineInterval, ea.nTodoPriority,
				ConvertToQuotableXMLString(ea.strTodoMethod), strNotes, strCategoryID);

			// (a.walling 2014-07-01 15:28) - PLID 62697
			for (long assignTo : ea.anTodoAssignTo) {
				strActionXml += FormatString("<R ID=\"%li\" AssignTo=\"%d\" />\r\n", ea.nID, assignTo);
			}
		}

		// (b.savon 2014-07-21 16:38) - PLID 62707 - Handle saving the new Diagnosis DestType action to EmrActionsT and EmrActionsDiagnosisDataT
		if (pActions->GetAt(i).eaoDestType == eaoDiagnosis){
			const EmrAction& ea = pActions->GetAt(i);
			strActionXml += FormatString("<DI ID=\"%li\" DiagCodeID_ICD9=\"%li\" DiagCodeID_ICD10=\"%li\" />\r\n", ea.nID, ea.diaDiagnosis.nDiagCodeID_ICD9, ea.diaDiagnosis.nDiagCodeID_ICD10);
		}

		// (c.haag 2008-07-17 16:59) - PLID 30724 - Emr problem actions
		const EmrAction& ea = pActions->GetAt(i);
		// (a.walling 2014-07-01 15:28) - PLID 62697
		for (const auto& epa : ea.aProblemActions) {
			// (a.walling 2008-10-02 09:18) - PLID 31564 - VS2008 - An element of a const array must also be const
			// (s.tullis 2015-03-13 16:32) - PLID 64724 - added do not show on ccda
			// (c.haag 2014-07-22) - PLID 62789 - Added SNOMEDCodeID
			// (r.gonet 2015-03-10 14:48) - PLID 65013 - Added DoNotShowOnProblemPrompt.
			strActionXml += FormatString("<P ID=\"%li\" DefaultDescription=\"%s\" DefaultStatus=\"%li\" SpawnToSourceItem=\"%li\" %s DoNotShowOnCCDA=\"%li\" DoNotShowOnProblemPrompt=\"%li\" />\r\n", 
				ea.nID, ConvertToQuotableXMLString(epa.strDescription), epa.nStatus, epa.bSpawnToSourceItem, epa.GetSNOMEDValueForXML(), epa.bDoNotShowOnCCDA, epa.bDoNotShowOnProblemPrompt);
		}

		//Increment the arbitrary value here, per-action.  Do this whether there is a charge action or not, because you 
		//	might have actions for 1 - charge, 2 - not charge, 3 - charge.
		nArbitraryActionIDValue++;

		//Append ending </A> because <C.../> is a subset of <A>
		strActionXml += "</A>\r\n";
	}

	return strActionXml;
}

/*//TES 12/5/2006 - PLID 23724 - This can now be done by a simple comparison of EmrInfoMasterID, so there's no need for this whole
// separate function.
//determines if the Info ID from the detail is the same as the passed in Info ID
//or whether a different version of the detail has the same Info ID
BOOL AreInfoIDsEquivalent(long nInfoID1, long nInfoID2)
{
	return ReturnsRecords("SELECT ID FROM EmrInfoT WHERE ID = %li AND EmrInfoMasterID = (SELECT EmrInfoMasterID FROM EmrInfoT Info2 WHERE Info2.ID = %li)", nInfoID1, nInfoID2);

	//first just see if the info IDs match
	if(nInfoID1 == nInfoID2)
		return TRUE;

	//if not, see if they are in the same version history
	CString strPastItemIDs1 = GeneratePastEMRInfoIDs(nInfoID1);
	CString strPastItemIDs2 = GeneratePastEMRInfoIDs(nInfoID2);
		
	//if only one number in either list, then there are no past versions to check
	if(strPastItemIDs1.Find(",") == -1 && strPastItemIDs2.Find(",") == -1) {
		return FALSE;
	}
	
	//now see if there is a number in list 1 that is also in list 2
	CString strList1Copy = strPastItemIDs1;	
	while(strList1Copy != "") {
		CString strID1;
		int nFind = strList1Copy.Find(",");
		if(nFind == -1) {
			strID1 = strList1Copy;
			strList1Copy = "";
		}
		else {
			strID1 = strList1Copy.Left(nFind);
			strList1Copy = strList1Copy.Right(strList1Copy.GetLength() - nFind - 1);
		}

		CString strList2Copy = strPastItemIDs2;
		while(strList2Copy != "") {
			CString strID2;
			int nFind2 = strList2Copy.Find(",");
			if(nFind2 == -1) {
				strID2 = strList2Copy;
				strList2Copy = "";
			}
			else {
				strID2 = strList2Copy.Left(nFind2);
				strList2Copy = strList2Copy.Right(strList2Copy.GetLength() - nFind2 - 1);
			}

			//if the IDs match, return TRUE
			if(strID1 == strID2) {
				return TRUE;
			}

			strList2Copy.TrimLeft();			
		}

		strList1Copy.TrimLeft();
	}

	return FALSE;
}*/

//DRT 1/11/2007 - PLID 24177
//Prompts the user with a multi select dialog containing all diagnosis codes already 
//	added to this EMN.
//This function will update the given EMNCharge object to have the new
//	set of diagnosis codes.
//Return value is TRUE if they pressed OK on the whichcodes popup (changed or not), and FALSE if they 
//	cancelled
//DRT 1/12/2007 - PLID 24178 - Prompts the user to link the given charge with
//	all diagnosis codes on the given EMN.
//DRT 1/12/2007 - PLID 24178 - I moved this to EMRUtils because it's needed
//	in the upper regions of the EMR structure, and is really a utility.  Changed the EMN
//	to be a parameter since it's no longer a member of the EMN.
// (j.jones 2007-01-18 08:55) - PLID 24180 - used preferences to control what items are preselected, as well as the prompt itself
BOOL PromptToLinkDiagCodesToCharge(EMNCharge *pCharge, CEMN* pEMN, BOOL bIsSpawning)
{
	//DRT 8/16/2007 - PLID 26495 - We do not want these to popup during an initial load.  Just skip and let nothing be selected.  We decided
	//	against having it auto-select based on the preferences, it's safer to have nothing select and let the user make their selections.
	//Note that in "real" data, auto-spawning charges / diag codes should never happen.
	if(pEMN->IsLoading())
		return FALSE;

	//Prompt the user to pick the diagnosis codes
	// (a.walling 2007-09-07 09:11) - PLID 24371 - Ensure the dialog has an appropriate parent window
	//TES 1/30/2008 - PLID 24157 - If the EMN has a multipopup dialog open, use that.
	CWnd *pParent = NULL;
	if(pEMN) {
		pParent = pEMN->GetOpenMultiPopupDlg();
		if(!pParent) {
			if(pEMN->GetParentEMR()) {
				pParent = pEMN->GetParentEMR()->GetInterface();
			}
		}
	}
	
	CString strFrom;
	CDWordArray dwPreSelectAllIDs;
	CDWordArray dwPreSelectSameTopicIDs;
	BOOL bPrompt = TRUE;

	// (j.jones 2011-04-07 09:32) - PLID 43166 - added EMR_SkipLinkingDiagsToNonBillableCPTs
	BOOL bSkipLinkingNonBillableCPTs = GetRemotePropertyInt("EMR_SkipLinkingDiagsToNonBillableCPTs", 1, 0, GetCurrentUserName(), true) == 1;

	// (j.jones 2011-04-07 09:39) - PLID 43166 - ignore the charge if
	// it is non-billable, based on our preference (only if spawning)
	if(bSkipLinkingNonBillableCPTs && !pCharge->bBillable && bIsSpawning) {
		return FALSE;
	}

	// (j.jones 2013-05-08 14:44) - PLID 44634 - The logic below was previously busted and the preference layout made no sense.
	// It's been improved to do the following:
	// - If we are not spawning, we do not try to automatically link anything, and we always prompt.
	// - If bLinkByTopic is on, we will only link to diag. codes that were spawned from the same topic,
	// then respect the setting whether or not to prompt to let the user tweak further.
	// - If bLinkByTopic is off, we look at the subpreferences to decide if they want to link to all diag. codes,
	// or link to no diag. codes, and then respect the setting whether or not to prompt.
	// - Turning off bLinkByTopic, choosing to not link to diag. codes, and choosing not to prompt means
	// that they will not get a prompt when spawning charges.

	// (j.jones 2007-01-18 10:10) - PLID 24180 - store this preference for later
	BOOL bLinkByTopic = GetRemotePropertyInt("EMR_LinkSpawnedChargesDiagsByTopic", 1, 0, GetCurrentUserName(), true) == 1;
	//TES 3/3/2014 - PLID 61080 - Track which codes exist in the list
	// (s.dhole 2014-03-05 09:39) - PLID 60916
	bool bHasICD9Code = false;
	bool bHasICD10Code = false;
	{
		//DRT 1/15/2007 - PLID 24178 - Before we do anything, check for no diag codes.  If no codes, no need to prompt.
		if(pEMN->GetDiagCodeCount() == 0)
			return FALSE;

		//Generate a from clause that just unions all the diag codes currently in the more info
		for(int i = 0; i < pEMN->GetDiagCodeCount(); i++) {
			EMNDiagCode* pDiag = pEMN->GetDiagCode(i);

			// (a.walling 2008-02-07 07:22) - PLID 28832 - Mind your Ps and _Q's!
			// (j.jones 2008-10-14 14:50) - PLID 28045 - display the code and description
			CString str, strICD9Value, strICD10Value;
			//TES 2/28/2014 - PLID 61080 - Display both the ICD-9 and ICD-10 codes to the user
			if(pDiag->nDiagCodeID != -1) {
				strICD9Value = FormatString("%s - %s", pDiag->strCode, pDiag->strCodeDesc);
				// (s.dhole 2014-03-05 09:39) - PLID 60916
				if (!pDiag->strCode.IsEmpty()){
					bHasICD9Code = true;
				}
			}
			else {
				strICD9Value = "<No Matching Code>";
				
			}
			if(pDiag->nDiagCodeID_ICD10 != -1) {
				strICD10Value = FormatString("%s - %s", pDiag->strCode_ICD10, pDiag->strCodeDesc_ICD10);
				// (s.dhole 2014-03-05 09:39) - PLID 60916
				bHasICD10Code = true;
			}
			else {
				strICD10Value = "<No Matching Code>";
				
			}
			//TES 2/28/2014 - PLID 61080 - Use the index as the ID, that's what pCharge actually stores now
			str.Format("UNION SELECT %li AS ID, '%s' AS ICD9Value, '%s' AS ICD10Value ", i, _Q(strICD9Value), _Q(strICD10Value));
			strFrom += str;

			dwPreSelectAllIDs.Add(i);

			if(bIsSpawning && bLinkByTopic) {
				//also track which diag codes have the same source topic as the charge
				CEMRTopic *pChargeTopic = NULL;
				if(pCharge->sai.pSourceDetail) {
					// (z.manning 2009-02-23 12:48) - PLID 33141 - Use the new source action info struct
					pChargeTopic = pCharge->sai.pSourceDetail->m_pParentTopic;
				}
				else if(pCharge->sai.nSourceDetailID != -1) {
					CEMNDetail *pDetail = pEMN->GetDetailByID(pCharge->sai.nSourceDetailID);
					// (c.haag 2007-06-21 17:14) - PLID 26421 - pDetail should not be NULL,
					// but we should always check for NULL's when we do anything.
					if (NULL != pDetail) {
						pChargeTopic = pDetail->m_pParentTopic;
					} else {
						// If we get here, it means the detail was probably deleted. At the
						// very least, it could not be loaded or doesn't exist for some reason.
					}
				}
				CEMRTopic *pDiagTopic = NULL;
				if(pDiag->sai.pSourceDetail) {
					pDiagTopic = pDiag->sai.pSourceDetail->m_pParentTopic;
				}
				else if(pDiag->sai.nSourceDetailID != -1) {
					CEMNDetail *pDetail = pEMN->GetDetailByID(pDiag->sai.nSourceDetailID);
					// (c.haag 2007-06-21 17:14) - PLID 26421 - pDetail should not be NULL,
					// but we should always check for NULL's when we do anything.
					if (NULL != pDetail) {
						pDiagTopic = pDetail->m_pParentTopic;
					} else {
						// If we get here, it means the detail was probably deleted. At the
						// very least, it could not be loaded or doesn't exist for some reason.
					}
				}

				//now, are they the same?
				if(pChargeTopic != NULL && pChargeTopic == pDiagTopic) {
					//they are, so track this diag code
					//TES 2/28/2014 - PLID 61080 - Use the index as the ID, that's what pCharge actually stores now
					dwPreSelectSameTopicIDs.Add(i);
				}
			}
		}

		//Trim off the opening 'UNION '
		strFrom = strFrom.Mid(6);

		//Format it as a subquery
		strFrom = "(" + strFrom + ") FromQ";
	}

	// (j.jones 2013-05-08 15:00) - PLID 44634 - we now track the IDs to pre-select,
	// rather than pre-filling the dialog, because now we support applying their
	// automated links without prompting first
	CDWordArray dwLinksToApply;

	if(bIsSpawning) {
		// (j.jones 2007-01-18 08:58) - PLID 24180 - check the spawning preferences;
		// at no point will we undo existing values in pCharge->strDiagList		

		//the logic here is that bLinkByTopic takes precedence,
		//and if not used then we look at nAutoSelect, and if that is set to 2
		//then and only then do we look at bDoNotPrompt
		if(bLinkByTopic) {
			//preselect any items spawned from the same topic, which we determined earlier
			dwLinksToApply.Append(dwPreSelectSameTopicIDs);
			// (j.jones 2013-05-08 14:57) - PLID 44634 - we now support configuring a prompt here (0 means prompt, 1 means do not)
			bPrompt = !(GetRemotePropertyInt("EMR_LinkSpawnedChargesDiagsByTopic_Prompt", 0, 0, GetCurrentUserName(), true) == 1);
		}
		else {
			//if not linking by source topic, then look at the all/none preferences
			long nAutoSelect = GetRemotePropertyInt("EMR_AutoLinkSpawnedChargesToDiagCodes", 2, 0, GetCurrentUserName(), true);
			//nAutoSelect: 1 - select all, 2 - select none
			if(nAutoSelect == 1) {
				//pre-select all codes
				dwLinksToApply.Append(dwPreSelectAllIDs);
				// (j.jones 2013-05-08 14:57) - PLID 44634 - we now support configuring a prompt here (0 means prompt, 1 means do not)
				bPrompt = !(GetRemotePropertyInt("EMR_SpawnedCharges_AutoLinkPrompt", 0, 0, GetCurrentUserName(), true) == 1);
			}
			else {
				//do not pre-select any codes

				//we need not do anything here except check the preference to not prompt (0 means prompt, 1 means do not)
				bPrompt = !(GetRemotePropertyInt("EMR_SpawnedCharges_NoLinkPrompt", 0, 0, GetCurrentUserName(), true) == 1);
			}
		}
	}

	//Preselect all the codes that are already selected (overrides preferences)
	// (j.jones 2009-01-02 09:57) - PLID 32601 - now use the DiagID list
	// (j.jones 2013-05-08 15:16) - PLID 44634 - don't call preselect, just add to dwLinksToApply
	//TES 2/28/2014 - PLID 61080 - Renamed to aryDiagIndexes
	for(int i = 0; i < pCharge->aryDiagIndexes.GetSize(); i++) {
		long nDiagIndex = (long)pCharge->aryDiagIndexes.GetAt(i);
		BOOL bFound = FALSE;
		for(int j = 0; j < dwLinksToApply.GetSize() && !bFound; j++) {
			if((long)dwLinksToApply.GetAt(j) == nDiagIndex) {
				//don't need to add this diagnosis code again, it's already in the list
				bFound = TRUE;
			}
		}
		if(!bFound) {
			dwLinksToApply.Add(nDiagIndex);
		}
	}

	//DRT 1/12/2007 - PLID 24178 - Make the popup tell them what code was just spawned.  If they import topics
	//	or have numerous codes spawn, they cannot guess.
	CString strLabel;
	if(!pCharge->strCode.IsEmpty())
		strLabel.Format("Choose Diagnosis Codes to Link to Code '%s'.", pCharge->strCode);
	else
		strLabel.Format("Choose Diagnosis Codes to Link to '%s'.", pCharge->strDescription);

	BOOL bRet = FALSE;
	// (j.jones 2013-05-08 15:00) - PLID 44634 - we might still apply links if they chose not to prompt
	if(bPrompt) {
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(pParent, "DiagCodes");
		
		// (j.jones 2013-05-08 15:00) - PLID 44634 - fill the dialog with dwLinksToApply,
		// then clear the list, it will be refilled only if they click OK
		dlg.PreSelect(dwLinksToApply);
		dwLinksToApply.RemoveAll();

		DiagCodeSearchStyle dcss = DiagSearchUtils::GetPreferenceSearchStyle();
		//TES 3/3/2014 - PLID 61080 - Show both styles if the preference is to show both, or if the prefence is to show one that we don't have
		BOOL bShowICD9 = (dcss == eICD9_10_Crosswalk || dcss == eManagedICD9_Search || bHasICD9Code);
		BOOL bShowICD10 = (dcss == eICD9_10_Crosswalk || dcss == eManagedICD10_Search || bHasICD10Code);
		UINT nDlgReturn = 0;
		if(bShowICD10 && bShowICD9) {
			CStringArray saExtraFields;
			// (s.dhole 2014-03-05 09:39) - PLID 60916  change column order
			saExtraFields.Add("ICD9Value");
			CStringArray saExtraDescriptions;
			saExtraDescriptions.Add("ICD-9");
			dlg.m_strNameColTitle = "ICD-10";
			// (s.dhole 2014-03-11 10:50) - PLID 61318
			dlg.m_bSetEqualColumnWidth = TRUE;
			nDlgReturn = dlg.Open(strFrom, "", "ID", "ICD10Value", strLabel, 0, -1, &saExtraFields, &saExtraDescriptions);
		}
		else if( bShowICD9){
			dlg.m_strNameColTitle = "ICD-9";
			nDlgReturn = dlg.Open(strFrom, "", "ID", "ICD9Value", strLabel);
		}
		else {
			ASSERT(bShowICD10);
			dlg.m_strNameColTitle = "ICD-10";
			nDlgReturn = dlg.Open(strFrom, "", "ID", "ICD10Value", strLabel);
		}
		if(nDlgReturn == IDOK) {

			// (j.jones 2013-05-08 15:00) - PLID 44634 - replace dwLinksToApply with what they chose			
			dlg.FillArrayWithIDs(dwLinksToApply);
			
			//If they just open it and hit OK, this will flag a change.  This
			//is consistent with the rest of the charge details.
			bRet = TRUE;
		}
		else {
			//Cancelled the dialog, just return false
			return FALSE;
		}
	}
	
	// (j.jones 2013-05-08 15:00) - PLID 44634 - dwLinksToApply will have content
	// if it was automatically filled and they chose not to prompt, or if they prompted
	// and made some selections. In either case, apply the links now.
	if(dwLinksToApply.GetSize() > 0 || bRet) {

		//Remember they may have had some links, been prompted,
		//and then cleared those links. If so, bRet would be true already.
		pCharge->aryDiagIndexes.RemoveAll();
		pCharge->strDiagCodeList = "";

		for(int i = 0; i < dwLinksToApply.GetSize(); i++) {

			//TES 2/26/2014 - PLID 60807 - With ICD10, there can now be multiple codes with the same DiagCodeID
			// (although WhichCodes hasn't been updated with ICD10 yet)
			//TES 2/28/2014 - PLID 61080 - Pull the EMNDiagCode pointer using the index they selected
			EMNDiagCode* pDiag = pEMN->GetDiagCode(dwLinksToApply[i]);
			pCharge->aryDiagIndexes.Add(dwLinksToApply[i]);
			if(!pCharge->strDiagCodeList.IsEmpty()) {
				pCharge->strDiagCodeList += ", ";
			}
			//TES 2/28/2014 - PLID 61080 - Use the audit value that includes both codes
			CString strCodeForAudit = GenerateEMRDiagCodesTAuditValue(pDiag->strCode, pDiag->strCode_ICD10);
			pCharge->strDiagCodeList += strCodeForAudit;
		}

		//Notify the more info dialog so it can update the interface.
		//Ensure our windows exist.
		CWnd* pInterfaceWnd = pEMN->GetInterface();
		if(pInterfaceWnd)
			SendMessage(pInterfaceWnd->GetSafeHwnd(), NXM_EMN_CHARGE_CHANGED, (WPARAM)pCharge, (LPARAM)pEMN);

		//since we definitely applied links, ensure bRet is true
		bRet = TRUE;
	}

	return bRet;
}

//DRT 1/15/2007 - PLID 24179 - Prompt the user to link existing charges to
//	a given diagnosis code.  This function will update all EMNCharge objects
//	with the diagnosis code after the user commits their selections.  No
//	changes are made to the EMNDiagCode objects.
//Return values are TRUE if they pressed OK on the popup dialog (changed or not), 
//	and FALSE if they hit cancel.
// (j.jones 2007-01-18 08:55) - PLID 24180 - used preferences to control what items are preselected, as well as the prompt itself
BOOL PromptToLinkChargesToDiagCode(EMNDiagCode *pDiag, CEMN* pEMN, BOOL bIsSpawning)
{
	//DRT 8/16/2007 - PLID 26495 - We do not want these to popup during an initial load.  Just skip and let nothing be selected.  We decided
	//	against having it auto-select based on the preferences, it's safer to have nothing select and let the user make their selections.
	//Note that in "real" data, auto-spawning charges / diag codes should never happen.
	if(pEMN->IsLoading())
		return FALSE;

	//Prompt the user to pick the diagnosis codes
	// (a.walling 2007-09-07 09:13) - PLID 24371 - Ensure the dialog has an appropriate parent window
	//TES 1/30/2008 - PLID 24157 - If the EMN has a multipopup dialog open, use that.
	CWnd *pParent = NULL;
	if(pEMN) {
		pParent = pEMN->GetOpenMultiPopupDlg();
		if(!pParent) {
			if(pEMN->GetParentEMR()) {
				pParent = pEMN->GetParentEMR()->GetInterface();
			}
		}
	}
	CString strFrom;
	CDWordArray dwPreSelectAllIDs;
	CDWordArray dwPreSelectSameTopicIDs;
	BOOL bPrompt = TRUE;

	// (j.jones 2011-04-07 09:32) - PLID 43166 - added EMR_SkipLinkingDiagsToNonBillableCPTs
	BOOL bSkipLinkingNonBillableCPTs = GetRemotePropertyInt("EMR_SkipLinkingDiagsToNonBillableCPTs", 1, 0, GetCurrentUserName(), true) == 1;

	// (j.jones 2013-05-08 14:44) - PLID 44634 - The logic below was previously busted and the preference layout made no sense.
	// It's been improved to do the following:
	// - If we are not spawning, we do not try to automatically link anything, and we always prompt.
	// - If bLinkByTopic is on, we will only link to charges that were spawned from the same topic,
	// then respect the setting whether or not to prompt to let the user tweak further.
	// - If bLinkByTopic is off, we look at the subpreferences to decide if they want to link to all charges,
	// or link to no charges, and then respect the setting whether or not to prompt.
	// - Turning off bLinkByTopic, choosing to not link to charges, and choosing not to prompt means
	// that they will not get a prompt when spawning diagnosis codes.

	// (j.jones 2007-01-18 10:14) - PLID 24180 - store this preference for later
	BOOL bLinkByTopic = GetRemotePropertyInt("EMR_LinkSpawnedChargesDiagsByTopic", 1, 0, GetCurrentUserName(), true) == 1;
	{
		//Before we do anything, check for no charges.  If no charges, no need to prompt.
		if(pEMN->GetChargeCount() == 0)
			return FALSE;

		BOOL bFoundOneCode = FALSE;

		//Generate a from clause that just unions all the charges currently in the more info
		for(int i = 0; i < pEMN->GetChargeCount(); i++) {
			EMNCharge *pCharge = pEMN->GetCharge(i);

			// (j.jones 2011-04-07 09:39) - PLID 43166 - ignore the charge if
			// it is non-billable, based on our preference
			if(bSkipLinkingNonBillableCPTs && !pCharge->bBillable) {
				continue;
			}

			bFoundOneCode = TRUE;

			//For charges, the CPT code is NOT unique, so we must use the pointer value of
			//	the EMNCharge* to uniquely identify things.
			// (a.walling 2008-02-07 07:22) - PLID 28832 - Mind your Ps and _Q's!
			// (j.jones 2008-10-14 14:50) - PLID 28045 - display the code, description, and modifier codes if we have them
			CString str, strCode, strDescription, strMods;

			strMods += pCharge->strMod1;

			if(!strMods.IsEmpty() && !pCharge->strMod2.IsEmpty()) {
				strMods += ", ";
			}
			strMods += pCharge->strMod2;

			if(!strMods.IsEmpty() && !pCharge->strMod3.IsEmpty()) {
				strMods += ", ";
			}
			strMods += pCharge->strMod3;

			if(!strMods.IsEmpty() && !pCharge->strMod4.IsEmpty()) {
				strMods += ", ";
			}
			strMods += pCharge->strMod4;

			if(!strMods.IsEmpty()) {
				strMods = " (" + strMods + ")";
			}

			if(pCharge->strCode.IsEmpty()) {
				//if no code, show the description first, mods second
				strCode.Format("%s%s", pCharge->strDescription, strMods);
			}
			else {
				//if we have a code, show the code, mods, then description
				strCode.Format("%s%s - %s", pCharge->strCode, strMods, pCharge->strDescription);
			}

			str.Format("UNION SELECT %li AS ID, '%s' AS Value ", (long)pCharge, _Q(strCode));
			strFrom += str;

			dwPreSelectAllIDs.Add((long)pCharge);

			if(bIsSpawning && bLinkByTopic) {
				//also track which charges codes have the same source topic as the diag code				
				CEMRTopic *pDiagTopic = NULL;
				if(pDiag->sai.pSourceDetail) {
					pDiagTopic = pDiag->sai.pSourceDetail->m_pParentTopic;
				}
				else if(pDiag->sai.nSourceDetailID != -1) {
					CEMNDetail *pDetail = pEMN->GetDetailByID(pDiag->sai.nSourceDetailID);
					// (c.haag 2007-06-29 10:03) - PLID 26421 - pDetail should not be NULL,
					// but we should always check for NULL's when we do anything.
					if (NULL != pDetail) {
						pDiagTopic = pDetail->m_pParentTopic;				
					} else {
						// If we get here, it means the detail was probably deleted. At the
						// very least, it could not be loaded or doesn't exist for some reason.
					}
				}
				CEMRTopic *pChargeTopic = NULL;
				if(pCharge->sai.pSourceDetail) {
					// (z.manning 2009-02-23 12:48) - PLID 33141 - Use the new source action info class
					pChargeTopic = pCharge->sai.pSourceDetail->m_pParentTopic;
				}
				else if(pCharge->sai.nSourceDetailID != -1) {
					CEMNDetail *pDetail = pEMN->GetDetailByID(pCharge->sai.nSourceDetailID);
					// (c.haag 2007-06-29 10:03) - PLID 26421 - pDetail should not be NULL,
					// but we should always check for NULL's when we do anything.
					if (NULL != pDetail) {
						pChargeTopic = pDetail->m_pParentTopic;				
					} else {
						// If we get here, it means the detail was probably deleted. At the
						// very least, it could not be loaded or doesn't exist for some reason.
					}
				}

				//now, are they the same?
				if(pDiagTopic != NULL && pDiagTopic == pChargeTopic) {
					//they are, so track this charge
					dwPreSelectSameTopicIDs.Add((long)pCharge);
				}
			}
		}

		// (j.jones 2011-04-07 09:39) - PLID 43166 - this will return FALSE
		// if all our charges were non-billable, and we skipped them
		if(!bFoundOneCode) {
			return FALSE;
		}

		//Trim off the opening 'UNION '
		strFrom = strFrom.Mid(6);

		//Format it as a subquery
		strFrom = "(" + strFrom + ") FromQ";
	}

	// (j.jones 2013-05-08 15:00) - PLID 44634 - we now track the IDs to pre-select,
	// rather than pre-filling the dialog, because now we support applying their
	// automated links without prompting first
	CDWordArray dwLinksToApply;

	if(bIsSpawning) {
		// (j.jones 2007-01-18 08:58) - PLID 24180 - check the spawning preferences		

		//the logic here is that bLinkByTopic takes precedence,
		//and if not used then we look at nAutoSelect, and if that is set to 2
		//then and only then do we look at bDoNotPrompt
		if(bLinkByTopic) {
			//preselect any items spawned from the same topic
			dwLinksToApply.Append(dwPreSelectSameTopicIDs);
			// (j.jones 2013-05-08 14:57) - PLID 44634 - we now support configuring a prompt here (0 means prompt, 1 means do not)
			bPrompt = !(GetRemotePropertyInt("EMR_LinkSpawnedChargesDiagsByTopic_Prompt", 0, 0, GetCurrentUserName(), true) == 1);
		}
		else {
			//if not linking by source topic, then look at the all/none preferences
			long nAutoSelect = GetRemotePropertyInt("EMR_AutoLinkSpawnedDiagCodesToCharges", 2, 0, GetCurrentUserName(), true);
			//nAutoSelect: 1 - select all, 2 - select none
			if(nAutoSelect == 1) {
				//pre-select all codes
				dwLinksToApply.Append(dwPreSelectAllIDs);
				// (j.jones 2013-05-08 14:57) - PLID 44634 - we now support configuring a prompt here (0 means prompt, 1 means do not)
				bPrompt = !(GetRemotePropertyInt("EMR_SpawnedDiagCodes_AutoLinkPrompt", 0, 0, GetCurrentUserName(), true) == 1);
			}
			else {
				//do not pre-select any codes

				//we need not do anything here except check the preference to not prompt (0 means prompt, 1 means do not)
				bPrompt = !(GetRemotePropertyInt("EMR_SpawnedDiagCodes_NoLinkPrompt", 0, 0, GetCurrentUserName(), true) == 1);
			}	
		}
	}

	//No further preselecting done

	//Inform the user what we're asking
	CString strLabel;
	strLabel.Format("Choose Charges to Link with Diagnosis '%s'.", pDiag->strCode);

	BOOL bRet = FALSE;
	// (j.jones 2013-05-08 15:00) - PLID 44634 - we might still apply links if they chose not to prompt
	if(bPrompt) {
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(pParent, "CPTCodeT");
		
		// (j.jones 2013-05-08 15:00) - PLID 44634 - fill the dialog with dwLinksToApply,
		// then clear the list, it will be refilled only if they click OK
		dlg.PreSelect(dwLinksToApply);
		dwLinksToApply.RemoveAll();

		if(dlg.Open(strFrom, "", "ID", "Value", strLabel) == IDOK) {
			// (j.jones 2013-05-08 15:00) - PLID 44634 - replace dwLinksToApply with what they chose			
			dlg.FillArrayWithIDs(dwLinksToApply);

			//If they just open it and hit OK, this will flag a change.  This
			//is consistent with the rest of the charge details.
			bRet = TRUE;
		}
		else {
			//Cancelled the dialog, just return false
			return FALSE;
		}
	}
	
	// (j.jones 2013-05-08 15:00) - PLID 44634 - dwLinksToApply will have content
	// if it was automatically filled and they chose not to prompt, or if they prompted
	// and made some selections. In either case, apply the links now.
	
	//Loop through all selections.  Remember that the ID column is actually a pointer to the EMNCharge
	//structure.  We just need to append the current diagnosis code to each of those charges.
	for(int i = 0; i < dwLinksToApply.GetSize(); i++) {
		EMNCharge *pCharge = (EMNCharge*)dwLinksToApply.GetAt(i);

		//The list of diagnosis codes is || delimited
		// (j.jones 2009-01-02 10:02) - PLID 32601 - track the IDs and Codes separately
		//TES 2/28/2014 - PLID 61080 - Get the index, that's what we store now
		long nIndex = pEMN->GetDiagCodeIndexByDiagID(pDiag->nDiagCodeID, pDiag->nDiagCodeID_ICD10);
		ASSERT(nIndex != -1);
		pCharge->aryDiagIndexes.Add(nIndex);
		if(!pCharge->strDiagCodeList.IsEmpty()) {
			pCharge->strDiagCodeList += ", ";
		}
		//TES 2/28/2014 - PLID 61080 - Use the audit value that checks both codes
		CString strCodeForAudit = GenerateEMRDiagCodesTAuditValue(pDiag->strCode, pDiag->strCode_ICD10);
		pCharge->strDiagCodeList += strCodeForAudit;

		//For each code that we change, we need to nofity the more info dialog so it can update the interface
		{
			//Ensure our windows exist.
			CWnd* pInterfaceWnd = pEMN->GetInterface();
			if(pInterfaceWnd)
				SendMessage(pInterfaceWnd->GetSafeHwnd(), NXM_EMN_CHARGE_CHANGED, (WPARAM)pCharge, (LPARAM)pEMN);
		}

		//since we definitely applied links, ensure bRet is true
		bRet = TRUE;
	}

	return bRet;
}

long GetActiveCurrentMedicationsInfoID(OPTIONAL IN ADODB::_Connection *lpCon /*= NULL*/)
{
	//
	// (c.haag 2007-01-25 10:27) - PLID 24396 - Returns the EmrInfoID of the active Current
	// Medications info item. Unfortunately, this requires a query.
	//
	_ConnectionPtr pCon;
	if(lpCon) pCon = lpCon;
	else pCon = GetRemoteData();

	// (j.jones 2009-09-18 11:07) - PLID 35367 - parameterized
	_RecordsetPtr prs = CreateParamRecordset(pCon, "SELECT ID FROM EmrInfoT WHERE DataSubType = {INT} AND ID IN "
		"(SELECT ActiveEmrInfoID FROM EMRInfoMasterT)",
		eistCurrentMedicationsTable);

	if (prs->eof) {
		// This is a wacky description to give to a user, but it helps support, and should never
		// happen anyway
		ThrowNxException("GetActiveCurrentMedicationsInfoID() could not find an EMR info record!");
	} else if (prs->RecordCount > 1) {
		// This is a wacky description to give to a user, but it helps support, and should never
		// happen anyway
		ThrowNxException("GetActiveCurrentMedicationsInfoID() found multiple EMR info records!");
	}

	return AdoFldLong(prs, "ID");
}

long GetActiveAllergiesInfoID(OPTIONAL IN ADODB::_Connection *lpCon /*= NULL*/)
{
	//
	// (c.haag 2007-04-02 16:08) - PLID 25465 - Returns the EmrInfoID of the active
	// Allergies info item
	//
	_ConnectionPtr pCon;
	if(lpCon) pCon = lpCon;
	else pCon = GetRemoteData();

	// (j.jones 2009-09-18 11:07) - PLID 35367 - parameterized
	_RecordsetPtr prs = CreateParamRecordset(pCon, "SELECT ID FROM EmrInfoT WHERE DataSubType = {INT} AND ID IN "
		"(SELECT ActiveEmrInfoID FROM EMRInfoMasterT)",
		eistAllergiesTable);

	if (prs->eof) {
		// This is a wacky description to give to a user, but it helps support, and should never
		// happen anyway
		ThrowNxException("GetActiveAllergiesInfoID() could not find an EMR info record!");
	} else if (prs->RecordCount > 1) {
		// This is a wacky description to give to a user, but it helps support, and should never
		// happen anyway
		ThrowNxException("GetActiveAllergiesInfoID() found multiple EMR info records!");
	}

	return AdoFldLong(prs, "ID");
}

// (j.jones 2010-06-21 11:30) - PLID 37981 - return the InfoID of the active generic table item
long GetActiveGenericTableInfoID(OPTIONAL IN ADODB::_Connection *lpCon /*= NULL*/)
{
	_ConnectionPtr pCon;
	if(lpCon) pCon = lpCon;
	else pCon = GetRemoteData();

	_RecordsetPtr prs = CreateParamRecordset(pCon, "SELECT ID FROM EmrInfoT WHERE DataSubType = {INT} AND ID IN "
		"(SELECT ActiveEmrInfoID FROM EMRInfoMasterT)",
		eistGenericTable);

	if (prs->eof) {
		ThrowNxException("GetActiveGenericTableInfoID() could not find an EMR info record!");
	} else if (prs->RecordCount > 1) {
		ThrowNxException("GetActiveGenericTableInfoID() found multiple EMR info records!");
	}

	return AdoFldLong(prs, "ID");
}

// (j.jones 2009-09-18 11:08) - PLID 35367 - Get the date of the latest EMN for the given patient
COleDateTime GetLatestEMNDateForPatient(long nPatientID)
{
	COleDateTime dt;
	dt.SetStatus(COleDateTime::invalid);

	//find the latest EMN date for this patient
	_RecordsetPtr rs = CreateParamRecordset("SELECT dbo.AsDateNoTime(Max(EMRMasterT.Date)) AS MaxDate "
		"FROM EMRMasterT "
		"WHERE EMRMasterT.PatientID = {INT} "
		"AND EMRMasterT.Deleted = 0",
		nPatientID);
	if(!rs->eof) {
		_variant_t varDate = rs->Fields->Item["MaxDate"]->Value;
		if(varDate.vt == VT_DATE) {
			dt = VarDateTime(varDate);
		}
	}
	rs->Close();

	return dt;
}

void GetCurrentMedicationTableElements(CArray<long, long>& anX, CArray<long, long>& anY,
									   CArray<long, long>& anYListType, CArray<long, long>& anYSortOrder,
									   long nCurMedEmrInfoID, OPTIONAL IN ADODB::_Connection *lpCon /*= NULL*/)
{
	//
	// (c.haag 2007-01-29 09:15) - PLID 24396 - This function populates two arrays
	// with the data (row and column) values of the active Current Medications table
	// info item.
	//
	// With this information, the caller can build the state value for a detail which
	// is based on the active Current Medications table info item
	//
	_ConnectionPtr pCon;
	if(lpCon) pCon = lpCon;
	else pCon = GetRemoteData();

	//DRT 8/30/2007 - PLID 27259 - Parameterized.
	_RecordsetPtr prsTable = CreateParamRecordset(pCon, "SELECT ID, ListType, SortOrder FROM EMRDataT WHERE EMRInfoID = {INT} ORDER BY SortOrder",
		nCurMedEmrInfoID);
	while (!prsTable->eof) {
		long nListType = AdoFldLong(prsTable, "ListType");
		long nID = AdoFldLong(prsTable, "ID");
		long nSortOrder = AdoFldLong(prsTable, "SortOrder");
		if(nListType == 2) { //Row
			anX.Add(nID);
		} else if(nListType >= 3) { //Column
			anYListType.Add(nListType);
			anYSortOrder.Add(nSortOrder);
			anY.Add(nID);
		} else {
			ASSERT(FALSE); // This should never happen for table items
		}
		prsTable->MoveNext();
	}
	prsTable->Close();
}

void GetAllergiesTableElements(CArray<long, long>& anX, CArray<long, long>& anY,
									   CArray<long, long>& anYListType, CArray<long, long>& anYSortOrder,
									   long nAllergiesEmrInfoID, OPTIONAL IN ADODB::_Connection *lpCon /*= NULL*/)
{
	//
	// (c.haag 2007-04-05 11:39) - PLID 25516 - This function populates two arrays
	// with the data (row and column) values of the active Allergies table
	// info item
	//
	// With this information, the caller can build the state value for a detail which
	// is based on the active Allergies table info item
	//
	_ConnectionPtr pCon;
	if(lpCon) pCon = lpCon;
	else pCon = GetRemoteData();

	//DRT 8/30/2007 - PLID 27259 - Parameterized.
	_RecordsetPtr prsTable = CreateParamRecordset(pCon, "SELECT ID, ListType, SortOrder FROM EMRDataT WHERE EMRInfoID = {INT} ORDER BY SortOrder",
		nAllergiesEmrInfoID);
	while (!prsTable->eof) {
		long nListType = AdoFldLong(prsTable, "ListType");
		long nID = AdoFldLong(prsTable, "ID");
		long nSortOrder = AdoFldLong(prsTable, "SortOrder");
		if(nListType == 2) { //Row
			anX.Add(nID);
		} else if(nListType >= 3) { //Column
			anYListType.Add(nListType);
			anYSortOrder.Add(nSortOrder);
			anY.Add(nID);
		} else {
			ASSERT(FALSE); // This should never happen for table items
		}
		prsTable->MoveNext();
	}
	prsTable->Close();
}

// (c.haag 2007-10-22 08:57) - PLID 27827 - Added nAuditTransactionID
// (c.haag 2011-01-25) - PLID 42222 - We now take in a map that maps current medication data ID's to NewCrop GUID's.
// (j.jones 2011-05-04 10:26) - PLID 43527 - added Sig maps, which maps a DataID
// to the Sig entered in the table or in the medications tab
// (j.jones 2012-10-17 13:44) - PLID 51713 - Added bCurHasNoMedsStatus, the current value of PatientsT.HasNoMeds.
// This function will clear that status if it adds a current medication.
CString GenerateCurrentMedsPropagationSaveString(CArray<long,long>& anEMNCurMedDataIDs,
												 CMap<long, long, CString, LPCTSTR> &mapEMNCurMedDataIDsToSig,
												 CArray<long,long>& anPtCurMedDataIDs,
												 CMap<long, long, CString, LPCTSTR> &mapPtCurMedDataIDsToSig,
												 long nPatientID,
												 long& nAuditTransactionID,
												 const CMap<long,long,CString,LPCTSTR>& mapNewEMRDataIDToNewCropGUID,
												 BOOL &bCurHasNoMedsStatus)
{
	// (c.haag 2007-01-23 14:36) - PLID 24396 - When an EMR with a Current Medications detail is saved,
	// we need to update the CurrentPatientMedsT table so that it is in perfect synchronization with the
	// detail. By design, we only need any one pSourceDetail to do this because all of the Current Medication
	// details in memory should always be in sync.
	//
	CString strSaveString;
	int i,j;

	// (c.haag 2007-01-23 14:52) - Make sure we have a patient ID
	if (-1 == nPatientID) {
		AfxThrowNxException("Tried to save current patient medication data without a patient!");
	}

	// (c.haag 2007-01-23 15:47) - Build a list of all current medication data id's selected in the
	// detail, but not assigned to the patient
	CDWordArray anMedsToAdd;
	CDWordArray anMedsToUpdate;
	for (i=0; i < anEMNCurMedDataIDs.GetSize(); i++) {
		BOOL bFound = FALSE;
		for (j=0; j < anPtCurMedDataIDs.GetSize() && !bFound; j++) {
			if (anPtCurMedDataIDs[j] == anEMNCurMedDataIDs[i]) {
				bFound = TRUE;

				// (j.jones 2011-05-06 11:30) - PLID 43527 - see if the Sigs match (case sensitive),
				// if not, we need to change the med
				CString strEMNSig = "";
				if(mapEMNCurMedDataIDsToSig.Lookup(anEMNCurMedDataIDs[i], strEMNSig)) {
					//only if we have a result (could be blank) should we
					//try to compare further
					CString strPatSig = "";
					if(mapPtCurMedDataIDsToSig.Lookup(anPtCurMedDataIDs[j], strPatSig)) {
						//only if we have a result (could be blank) should we
						//try to compare further
						if(strEMNSig != strPatSig) {
							//the Sigs are different (case sensitive), so we must change it
							anMedsToUpdate.Add(anEMNCurMedDataIDs[i]);
						}
					}
				}
			}
		}
		if (!bFound) {
			anMedsToAdd.Add(anEMNCurMedDataIDs[i]);
		}
	}

	// (c.haag 2007-01-23 15:50) - Build a list of all current medication data id's assigned to the
	// patient, but not selected
	CDWordArray anMedsToDiscontinue;
	for (i=0; i < anPtCurMedDataIDs.GetSize(); i++) {
		BOOL bFound = FALSE;
		for (j=0; j < anEMNCurMedDataIDs.GetSize() && !bFound; j++) {
			if (anEMNCurMedDataIDs[j] == anPtCurMedDataIDs[i]) {
				bFound = TRUE;
			}
		}
		if (!bFound) {
			anMedsToDiscontinue.Add(anPtCurMedDataIDs[i]);
		}
	}

	// (c.haag 2007-10-22 08:40) - PLID 27827 - Auditing
	// (a.walling 2013-07-02 09:02) - PLID 57407 - CMap's ARG_VALUE should be const CString& instead of LPCTSTR so it can use CString reference counting
	CMap<long,long,CString,const CString&> mapMedNames;
	CDWordArray anAllIDs;
	anAllIDs.Append(anMedsToAdd);
	anAllIDs.Append(anMedsToUpdate);
	anAllIDs.Append(anMedsToDiscontinue);
	if (anAllIDs.GetSize() > 0) {

		// (c.haag 2007-10-22 08:45) - PLID 27827 - Pull all the medication names which 
		// correspond to the ID's into a map for auditing reference

		// Not using a parameter query because the ID's may be too diverse, and the query called too infrequently for any real benefit
		_RecordsetPtr prsAuditNames = CreateRecordset("SELECT ID, Data FROM EMRDataT WHERE ID IN (%s)", ArrayAsString(anAllIDs, false));
		FieldsPtr f = prsAuditNames->Fields;
		while (!prsAuditNames->eof) {
			mapMedNames.SetAt( AdoFldLong(f, "ID"), AdoFldString(f, "Data", "") );
			prsAuditNames->MoveNext();
		}
		prsAuditNames->Close();
	}

	// (j.jones 2011-05-06 11:36) - PLID 43527 - we'll save (which is always an update or insert)
	// any new or changed med
	CDWordArray anMedsToAddOrUpdate;
	anMedsToAddOrUpdate.Append(anMedsToAdd);
	anMedsToAddOrUpdate.Append(anMedsToUpdate);
	for (i=0; i < anMedsToAddOrUpdate.GetSize(); i++) {

		// (j.jones 2011-05-04 10:26) - PLID 43527 - get the Sig from our maps
		CString strNewSig = "";
		CString strUpdateSigSql = "";
		BOOL bHasNewSig = FALSE;
		if(mapEMNCurMedDataIDsToSig.Lookup(anMedsToAddOrUpdate[i], strNewSig)) {
			//it may be blank, but if we found something, it means
			//there is a Sig column in this table
			bHasNewSig = TRUE;

			strUpdateSigSql.Format(", Sig = '%s'", _Q(strNewSig));
		}

		BOOL bHasOldSig = FALSE;
		CString strOldSig = "";
		if(mapPtCurMedDataIDsToSig.Lookup(anMedsToAddOrUpdate[i], strOldSig)) {
			//it may be blank, but if we found something, it means
			//there is an old sig on the patient med.
			bHasOldSig = TRUE;
		}

		// (c.haag 2011-01-25) - PLID 42222 - Also include a NewCrop GUID. Notice how if we did not find one, we do not
		// change the NewCropID for the medication.
		CString strNewCropGUID = "NULL";
		if (mapNewEMRDataIDToNewCropGUID.Lookup(anMedsToAddOrUpdate[i], strNewCropGUID))
		{
			CString strTemp = strNewCropGUID;
			strNewCropGUID.Format("'%s'", _Q(strTemp));
			// (c.haag 2010-09-15 11:00) - PLID 40215 - First check whether the medication already exists for the patient
			// as a discontinued value; if so, remove its discontinued status
			// (j.jones 2011-05-04 10:17) - PLID 43527 - support the Sig
			// (j.armen 2013-06-27 17:10) - PLID 57359 - Idenitate CurrentPatientMedsT
			AddStatementToSqlBatch(strSaveString, "IF EXISTS (SELECT MedicationID FROM CurrentPatientMedsT WHERE MedicationID IN (SELECT ID FROM DrugList WHERE EmrDataID = %d) AND PatientID = %d) BEGIN \r\n "
				"UPDATE CurrentPatientMedsT SET Discontinued = 0, DiscontinuedDate = NULL, NewCropGUID = %s %s WHERE MedicationID IN (SELECT ID FROM DrugList WHERE EmrDataID = %d) AND PatientID = %d \r\n"
				"END \r\n"
				"IF NOT EXISTS(SELECT MedicationID FROM CurrentPatientMedsT WHERE MedicationID IN (SELECT ID FROM DrugList WHERE EmrDataID = %d) AND PatientID = %d) BEGIN \r\n"
				"INSERT INTO CurrentPatientMedsT (PatientID, MedicationID, NewCropGUID, InputByUserID, Sig) "
				"SELECT %d, ID, %s, %li, '%s' "
				"FROM DrugList WHERE DrugList.EMRDataID = %d \r\n"
				"END \r\n"
				,anMedsToAddOrUpdate[i], nPatientID
				,strNewCropGUID, strUpdateSigSql, anMedsToAddOrUpdate[i], nPatientID
				,anMedsToAddOrUpdate[i], nPatientID,
				nPatientID, strNewCropGUID, GetCurrentUserID(), _Q(strNewSig), 
				anMedsToAddOrUpdate[i]);

			// (j.jones 2012-10-17 16:07) - PLID 51713 - since we added/updated medications, clear the HasNoMeds status,
			// but only if it is currently set (passed into this function through bCurHasNoMedsStatus)
			if(bCurHasNoMedsStatus) {

				if (-1 == nAuditTransactionID) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				AddStatementToSqlBatch(strSaveString, "UPDATE PatientsT SET HasNoMeds = 0 WHERE PersonID = %li", nPatientID);
				AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditTransactionID, aeiPatientHasNoMedicationsStatus, nPatientID, "'No Medications' Status Selected", "'No Medications' Status Cleared", aepMedium, aetChanged);

				//set the status to false, to reflect our change
				bCurHasNoMedsStatus = FALSE;
			}
		}
		else {
			// (c.haag 2010-09-15 11:00) - PLID 40215 - First check whether the medication already exists for the patient
			// as a discontinued value; if so, remove its discontinued status
			// (j.jones 2011-05-04 10:17) - PLID 43527 - support the Sig
			// (j.armen 2013-06-27 17:10) - PLID 57359 - Idenitate CurrentPatientMedsT
			AddStatementToSqlBatch(strSaveString, "IF EXISTS (SELECT MedicationID FROM CurrentPatientMedsT WHERE MedicationID IN (SELECT ID FROM DrugList WHERE EmrDataID = %d) AND PatientID = %d) BEGIN \r\n "
				"UPDATE CurrentPatientMedsT SET Discontinued = 0, DiscontinuedDate = NULL %s WHERE MedicationID IN (SELECT ID FROM DrugList WHERE EmrDataID = %d) AND PatientID = %d \r\n"
				"END \r\n"
				"IF NOT EXISTS(SELECT MedicationID FROM CurrentPatientMedsT WHERE MedicationID IN (SELECT ID FROM DrugList WHERE EmrDataID = %d) AND PatientID = %d) BEGIN \r\n"
				"INSERT INTO CurrentPatientMedsT (PatientID, MedicationID, InputByUserID, Sig) "
				"SELECT %d, ID, %li, '%s' "
				"FROM DrugList WHERE DrugList.EMRDataID = %d \r\n"
				"END \r\n"
				,anMedsToAddOrUpdate[i], nPatientID
				,strUpdateSigSql, anMedsToAddOrUpdate[i], nPatientID
				,anMedsToAddOrUpdate[i], nPatientID,
				nPatientID, GetCurrentUserID(), _Q(strNewSig), anMedsToAddOrUpdate[i]);

			// (j.jones 2012-10-17 16:07) - PLID 51713 - since we added/updated medications, clear the HasNoMeds status,
			// but only if it is currently set (passed into this function through bCurHasNoMedsStatus)
			if(bCurHasNoMedsStatus) {

				if (-1 == nAuditTransactionID) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				AddStatementToSqlBatch(strSaveString, "UPDATE PatientsT SET HasNoMeds = 0 WHERE PersonID = %li", nPatientID);
				AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditTransactionID, aeiPatientHasNoMedicationsStatus, nPatientID, "'No Medications' Status Selected", "'No Medications' Status Cleared", aepMedium, aetChanged);

				//set the status to false, to reflect our change
				bCurHasNoMedsStatus = FALSE;
			}
		}

		// (j.jones 2011-05-04 10:21) - PLID 43527 - audit the Sig only if it changed,
		// which would be if both bHasOldSig and bHasNewSig are true, and the Sigs are different
		if(bHasOldSig && bHasNewSig && strOldSig != strNewSig) {

			if (-1 == nAuditTransactionID) {
				nAuditTransactionID = BeginAuditTransaction();
			}

			CString strMedName;
			if (!mapMedNames.Lookup( anMedsToAddOrUpdate[i], strMedName )) {
				ThrowNxException("Attempted to audit a non-existent medication record! (1)");
			}

			CString strPatientName = GetExistingPatientName(nPatientID);
			CString strOldAudit;
			strOldAudit.Format("Medication: %s, Sig: %s", strMedName, strOldSig);
			AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiCurrentMedicationSig, nPatientID, strOldAudit, strNewSig, aepHigh, aetChanged);
		}
	}

	// (c.haag 2007-01-23 15:55) - Now build the save string based on medications we will
	// remove from CurrentPtMedsT
	// (j.jones 2013-08-19 13:53) - PLID 45864 - We no longer delete current meds from the syncing
	// of the blue tables. We only discontinue them.
	for (i=0; i < anMedsToDiscontinue.GetSize(); i++) {
		AddStatementToSqlBatch(strSaveString, "UPDATE CurrentPatientMedsT SET Discontinued = 1, DiscontinuedDate = GetDate() "
			"FROM CurrentPatientMedsT "
			"INNER JOIN DrugList ON CurrentPatientMedsT.MedicationID = DrugList.ID "
			"WHERE PatientID = %li AND DrugList.EMRDataID = %li", nPatientID, anMedsToDiscontinue[i]);
	}

	// (c.haag 2007-10-22 08:48) - PLID 27827 - Now build audit transactions for medications
	// we are adding
	const long nMedsToAdd = anMedsToAdd.GetSize();
	const long nMedsToDiscontinue = anMedsToDiscontinue.GetSize();
	if (nMedsToAdd > 0) {
		if (-1 == nAuditTransactionID) {
			nAuditTransactionID = BeginAuditTransaction();
		}
		for (i=0; i < nMedsToAdd; i++) {
			CString strMedName;
			if (!mapMedNames.Lookup( anMedsToAdd[i], strMedName )) {
				ThrowNxException("Attempted to audit a non-existent medication record! (2)");
			}
			AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditTransactionID, aeiCurrentPatientMedsAdd, anMedsToAdd[i], "", strMedName, aepHigh, aetCreated);
		}
	}

	// (c.haag 2007-10-22 08:53) - PLID 27827 - Now build audit transactions for medications
	// we are removing
	if (nMedsToDiscontinue > 0) {
		if (-1 == nAuditTransactionID) {
			nAuditTransactionID = BeginAuditTransaction();
		}
		for (i=0; i < nMedsToDiscontinue; i++) {
			CString strMedName;
			if (!mapMedNames.Lookup( anMedsToDiscontinue[i], strMedName )) {
				ThrowNxException("Attempted to audit a non-existent medication record! (3)");
			}
			// (j.jones 2013-08-19 13:53) - PLID 45864 - We no longer delete current meds from the syncing
			// of the blue tables. We only discontinue them.
			AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditTransactionID, aeiCurrentMedicationInactivated, anMedsToDiscontinue[i], strMedName, "<Discontinued>", aepHigh, aetChanged);
		}
	}

	return strSaveString;
}

// (c.haag 2007-10-22 09:20) - PLID 27822 - Added nAuditTransactionID
// (j.jones 2012-10-17 09:32) - PLID 53179 - Added bCurHasNoAllergiesStatus, the current value of PatientsT.HasNoAllergies.
// This function will clear that status if it adds an allergy.
CString GenerateAllergyPropagationSaveString(CArray<long,long> &anEMNAllergyDataIDs, CArray<long,long> &anPtAllergyDataIDs,
											 long nPatientID, long &nAuditTransactionID, BOOL &bCurHasNoAllergiesStatus,
 											 IN OUT CDWordArray &aryAllergyIngredToImport, 
											 IN OUT CDWordArray &aryAllergyIngredToDelete)
{
	// (c.haag 2007-04-06 09:21) - PLID 25525 - When an EMR with an Allergies detail is saved,
	// we need to update the PatientAllergyT table so that it is in perfect synchronization with the
	// detail. By design, we only need any one pSourceDetail to do this because all of the Allergies
	// in memory should always be in sync for an EMN
	//
	CString strSaveString;
	int i,j;

	// (c.haag 2007-01-23 14:52) - Make sure we have a patient ID
	if (-1 == nPatientID) {
		AfxThrowNxException("Tried to save current patient allergy data without a patient!");
	}

	// Build a list of all allergy data ID's selected in the detail, but not assigned to the patient
	// (j.luckoski 2012-11-20 10:23) - PLID 53825 - Add variables needed to import allergy ingredients
	CDWordArray anAllergiesToAdd;
	for (i=0; i < anEMNAllergyDataIDs.GetSize(); i++) {
		BOOL bFound = FALSE;
		for (j=0; j < anPtAllergyDataIDs.GetSize() && !bFound; j++) {
			if (anPtAllergyDataIDs[j] == anEMNAllergyDataIDs[i]) {
				bFound = TRUE;
			}
		}
		if (!bFound) {
			anAllergiesToAdd.Add(anEMNAllergyDataIDs[i]);
		}
	}

	// Build a list of all allergy data ID's assigned to the patient, but not selected in the detail
	CDWordArray anAllergiesToDiscontinue;
	for (i=0; i < anPtAllergyDataIDs.GetSize(); i++) {
		BOOL bFound = FALSE;
		for (j=0; j < anEMNAllergyDataIDs.GetSize() && !bFound; j++) {
			if (anEMNAllergyDataIDs[j] == anPtAllergyDataIDs[i]) {
				bFound = TRUE;
			}
		}
		if (!bFound) {
			anAllergiesToDiscontinue.Add(anPtAllergyDataIDs[i]);
		}
	}

	// Now build the save string based on allergies we will add to PatientAllergyT
	// (c.haag 2009-12-22 12:44) - PLID 35766 - Added EnteredDate
	BOOL bChangedSomething = FALSE;
	long nID = 0;
	for (i=0; i < anAllergiesToAdd.GetSize(); i++) {
		if(nID == 0) {
			_RecordsetPtr prs = CreateParamRecordset("SELECT (COALESCE(Max(ID),0) + 1) AS ID FROM PatientAllergyT");
			if (!prs->eof) {
				nID = AdoFldLong(prs, "ID");
			} else {
				ASSERT(false);
				return "";
			}
		} else {
			nID++;
		}
		
		CString strSql;
		strSql.Format("INSERT INTO PatientAllergyT (ID, AllergyID, PersonID, Description, EnteredDate) "
			"SELECT "
			"	%li, "
			"	ID, "
			"	%d, "
			"	'', "
			"	GetDate() "
			"FROM AllergyT WHERE AllergyT.EMRDataID = %d",
			nID, nPatientID, anAllergiesToAdd[i]);
		AddStatementToSqlBatch(strSaveString, strSql);

		bChangedSomething = TRUE;
		aryAllergyIngredToImport.Add((DWORD)nID); // (j.luckoski 2012-11-20 10:29) - PLID 53825 - Add to array for API import
	}

	// (j.jones 2012-10-17 09:28) - PLID 53179 - if we added allergies, clear the HasNoAllergies status,
	// but only if it is currently set (passed into this function through bCurHasNoAllergiesStatus)
	if(anAllergiesToAdd.GetSize() > 0 && bCurHasNoAllergiesStatus) {

		AddStatementToSqlBatch(strSaveString, "UPDATE PatientsT SET HasNoAllergies = 0 WHERE PersonID = %li", nPatientID);

		if(nAuditTransactionID == -1) {
			nAuditTransactionID = BeginAuditTransaction();
		}
		AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditTransactionID, aeiPatientHasNoAllergiesStatus, nPatientID, "'No Known Allergies' Status Selected", "'No Known Allergies' Status Cleared", aepMedium, aetChanged);

		//set the status to false, to reflect our change
		bCurHasNoAllergiesStatus = FALSE;
	}

	// Now build the save string based on allergies we will remove from PatientAllergyT
	// (j.jones 2013-08-19 13:53) - PLID 45864 - We no longer delete allergies from the syncing
	// of the blue tables. We only discontinue them. Discontinuing, however, DOES remove
	// from AllergyIngredientT.
	for (i=0; i < anAllergiesToDiscontinue.GetSize(); i++) {
	// (j.luckoski 2/26/16) - PLID 53825- Delete from AllergyIngredientT first
	// (j.luckoski 2013-03-22 14:03) - PLID 53825 - Slightly altered the allergyingredient table to be more time efficient as a request
		CString strAllergySql;
		strAllergySql.Format("DELETE AIT FROM AllergyIngredientT AIT INNER JOIN PatientAllergyT PAT ON PAT.ID = AIT.PatientAllergyID INNER JOIN AllergyT AL ON PAT.AllergyID = AL.ID WHERE PersonID = %li AND EMRDataID = %li",
				nPatientID, anAllergiesToDiscontinue[i]);
		AddStatementToSqlBatch(strSaveString, strAllergySql);
		CString strSql;
		strSql.Format("UPDATE PatientAllergyT SET Discontinued = 1, DiscontinuedDate = GetDate() "
			"FROM PatientAllergyT "
			"INNER JOIN AllergyT ON PatientAllergyT.AllergyID = AllergyT.ID "
			"WHERE PersonID = %li AND AllergyT.EMRDataID = %li", nPatientID, anAllergiesToDiscontinue[i]);
		AddStatementToSqlBatch(strSaveString, strSql);

		bChangedSomething = TRUE;
		aryAllergyIngredToDelete.Add((DWORD)anAllergiesToDiscontinue[i]); // (j.luckoski 2012-11-20 10:29) - PLID 53825 - Add to array for deletion
	}

	// (j.jones 2008-11-25 14:28) - PLID 28508 - if anything changed, clear our patient allergy review status
	if(bChangedSomething) {
		AddStatementToSqlBatch(strSaveString, "UPDATE PatientsT SET AllergiesReviewedOn = NULL, AllergiesReviewedBy = NULL WHERE PersonID = %li", nPatientID);

		if(nAuditTransactionID == -1) {
			nAuditTransactionID = BeginAuditTransaction();
		}
		AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditTransactionID, aeiPatientAllergiesReviewed, nPatientID, "", "Allergy information has not been reviewed.", aepMedium, aetChanged);
	}

	// (c.haag 2007-10-19 15:35) - PLID 27822 - Auditing
	// (a.walling 2013-07-02 09:02) - PLID 57407 - CMap's ARG_VALUE should be const CString& instead of LPCTSTR so it can use CString reference counting
	CMap<long,long,CString,const CString&> mapAllergyNames;
	CDWordArray anAllIDs;
	anAllIDs.Append(anAllergiesToAdd);
	anAllIDs.Append(anAllergiesToDiscontinue);
	if (anAllIDs.GetSize() > 0) {

		// (c.haag 2007-10-19 13:25) - PLID 27822 - Build a map of data ID's to names for
		// auditing purposes

		// Not using a parameter query because the ID's may be too diverse, and the query called too infrequently for any real benefit
		_RecordsetPtr prsAuditNames = CreateRecordset("SELECT ID, Data FROM EMRDataT WHERE ID IN (%s)", ArrayAsString(anAllIDs, false));
		FieldsPtr f = prsAuditNames->Fields;
		const long nAllergiesToAdd = anAllergiesToAdd.GetSize();
		const long nAllergiesToDiscontinue = anAllergiesToDiscontinue.GetSize();
		while (!prsAuditNames->eof) {
			mapAllergyNames.SetAt( AdoFldLong(f, "ID"), AdoFldString(f, "Data", "") );
			prsAuditNames->MoveNext();
		}
		prsAuditNames->Close();

		// (c.haag 2007-10-19 13:23) - PLID 27822 - Now build audit transactions for allergies
		// we are adding
		if (nAllergiesToAdd > 0) {
			if (-1 == nAuditTransactionID) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			for (i=0; i < nAllergiesToAdd; i++) {
				CString strAllergyName;
				if (!mapAllergyNames.Lookup( anAllergiesToAdd[i], strAllergyName )) {
					ThrowNxException("Attempted to audit a non-existent allergy record!");
				}
				AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditTransactionID, aeiPatientAllergyAdd, nPatientID, "", strAllergyName, aepMedium, aetCreated);
			}
		}

		// (c.haag 2007-10-19 13:23) - PLID 27822 - Now build audit transactions for allergies
		// we are removing
		if (nAllergiesToDiscontinue > 0) {
			if (-1 == nAuditTransactionID) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			for (i=0; i < nAllergiesToDiscontinue; i++) {
				CString strAllergyName;
				if (!mapAllergyNames.Lookup( anAllergiesToDiscontinue[i], strAllergyName )) {
					ThrowNxException("Attempted to audit a non-existent allergy record!");
				}
				// (j.jones 2013-08-19 13:53) - PLID 45864 - We no longer delete allergies from the syncing
				// of the blue tables. We only discontinue them.
				AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditTransactionID, aeiPatientInactivateAllergy, nPatientID, strAllergyName, "<Discontinued>", aepHigh, aetChanged);
			}
		}

		
	}

	return strSaveString;
}

BOOL IsEMRInfoItemInUse(long nEmrInfoID)
{
	//
	// (c.haag 2007-01-30 16:47) - PLID 24422 - Returns true if it's necessary to copy
	// an existing EMR info item if it is to be edited
	//
	// (c.haag 2007-02-12 12:45) - PLID 24422 - Took out the deleted check because deleted
	// details still "use" the info ID. If we leave them in, then we can get exceptions when
	// trying to delete info-related data.
	//
	// (b.cardillo 2007-09-06 15:58) - As a little added boost for PLID 27264 I changed this 
	// to use a parameterized query.  Added justification is that this is a utility function 
	// used in several places, so the fact that it will now skip the execution plan for all 
	// of them is valuable, even though the execution plan is rather simple.
	_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 1 FROM EMRDetailsT WHERE EMRInfoID = {INT}", nEmrInfoID);
	if (prs->eof) {
		return FALSE;
	} else {
		return TRUE;
	}
}

// (a.walling 2013-03-27 09:47) - PLID 55898 - Update EmrInfoT.ModifiedDate (and vicariously, the .Revision rowversion)
void TouchEMRInfoItem(long nEmrInfoID)
{
	ExecuteParamSql(
		"UPDATE EmrInfoT SET ModifiedDate = GETDATE() WHERE ID = {INT} "
		"AND ID NOT IN ("
			"SELECT EmrInfoID FROM EmrDetailsT INNER JOIN EmrMasterT ON EmrDetailsT.EMRID = EmrMasterT.ID WHERE EmrMasterT.Status = 2"
		")"
	, nEmrInfoID);
}

/**************************************************************************************
// (c.haag 2007-02-07 09:24) - If you change this function you must also change
CopyEmrInfoItem here and BranchCurrentMedicationsInfoItem the EMR importer!!!!!!
**************************************************************************************/

long BranchSystemInfoItem(EmrInfoSubType subtype)
{
	//
	// (c.haag 2007-01-30 17:30) - PLID 24422 - Generates a new Current Medications info item.
	// This should be called just prior to making changes to the official Current Medications
	// info item if the item already exists on EMN's.
	//
	// There are similarities between this function and CopyEmrInfoItem; in fact this function
	// was started by copying and pasting CopyEmrInfoItem. The key differences are:
	//
	// - This function does not generate a new master ID; it instead assigns the copied item the
	// master ID
	// - The DrugList table is updated
	// - There is no transaction
	//
	// In the future, we should make some kind of unified CopyEmrInfoItem or merge the like code
	// into a new function called something like CopyEmrInfoItemLow or InternalCopyEmrInfoItem
	// that can only be accessed in this source file
	//
	// (c.haag 2007-04-04 16:44) - PLID 25498 - We now support copying the master allergies
	// item, too.
	//
	long nNewInfoID;
	CString strNewName;

	switch (subtype) {
	case eistCurrentMedicationsTable:
		strNewName = "Current Medications";
		break;
	case eistAllergiesTable:
		strNewName = "Allergies";
		break;
	// (j.jones 2010-06-21 15:21) - PLID 37981 - supported generic tables
	case eistGenericTable:
		strNewName = "Generic Table";
		break;
	}

	//
	//Now do the deed of copying all this stuff
	BOOL bSaved = FALSE;

	//Figure out which EmrInfoT record we're copying.
	long nInfoID;
	switch (subtype) {
	case eistCurrentMedicationsTable:
		nInfoID = GetActiveCurrentMedicationsInfoID();
		break;
	case eistAllergiesTable:
		nInfoID = GetActiveAllergiesInfoID();
		break;
	// (j.jones 2010-06-21 15:21) - PLID 37981 - supported generic tables
	case eistGenericTable:
		nInfoID = GetActiveGenericTableInfoID();
		break;
	}

	// (c.haag 2010-07-10 12:07) - PLID 39467 - Don't reassign if there are any table dropdowns with
	// duplicate SortOrder values. That is considered bad data and we don't to propogate that.
	if (EmrItemHasTableDropdownsWithDuplicateSortOrders(nInfoID)) 
	{
		ThrowNxException("This EMR item has one or more table dropdowns with duplicate Sort Order values. Please contact NexTech support for assistance.");
	}

	// (a.walling 2008-08-12 12:45) - PLID 30570 - Copy the preview flags
	// (j.jones 2008-09-22 15:12) - PLID 31476 - supported RememberForEMR
	// (c.haag 2008-10-16 10:00) - PLID 31708 - Supported TableRowsAsFields
	//TES 3/15/2011 - PLID 42757 - Added HasGlassesOrderData, GlassesOrderLens
	// (z.manning 2011-07-01 17:21) - PLID 43910 - Added missing fields: DataCodeID, DataUnit, ChildEmrInfoMasterID, SmartStampsEnabled
	// (z.manning 2011-11-15 16:46) - PLID 46485 - Added InfoFlags
	//TES 4/6/2012 - PLID 49367 - Added HasContactLensData
	// (r.gonet 08/03/2012) - PLID 51948 - Added UseWithWoundCareCoding
	// (j.armen 2014-01-29 12:06) - PLID 60523 - Idenitate EMRInfoT
	_RecordsetPtr prs = CreateParamRecordset("SET NOCOUNT ON\r\n"
		"INSERT INTO EMRInfoT "
		"(Name, DataType, DataSubType, LongForm, BackgroundImageFilePath, BackgroundImageType, DefaultPenColor, "
		"DataFormat, DataSeparator, DefaultText, RememberForPatient, RememberForEMR, SliderMin, SliderMax, SliderInc, "
		"DataSeparatorFinal, OnePerEmn, AutoAlphabetizeListData, DisableTableBorder, PreviewFlags, EmrInfoMasterID, TableRowsAsFields, "
		"HasGlassesOrderData, GlassesOrderLens, DataCodeID, DataUnit, ChildEmrInfoMasterID, SmartStampsEnabled, InfoFlags, HasContactLensData, "
		"UseWithWoundCareCoding) "

		"SELECT {STRING}, DataType, DataSubType, LongForm, BackgroundImageFilePath, BackgroundImageType, DefaultPenColor, "
		"DataFormat, DataSeparator, DefaultText, RememberForPatient, RememberForEMR, SliderMin, SliderMax, SliderInc, "
		"DataSeparatorFinal, OnePerEmn, AutoAlphabetizeListData, DisableTableBorder, PreviewFlags, EmrInfoMasterID, TableRowsAsFields, "
		"HasGlassesOrderData, GlassesOrderLens, DataCodeID, DataUnit, ChildEmrInfoMasterID, SmartStampsEnabled, InfoFlags, HasContactLensData, "
		"UseWithWoundCareCoding "
		"FROM EMRInfoT WHERE ID = {INT}\r\n"
		"SET NOCOUNT OFF\r\n"
		"SELECT CONVERT(INT, SCOPE_IDENTITY()) AS EMRInfoID"
		, strNewName, nInfoID);

	nNewInfoID = AdoFldLong(prs, "EMRInfoID");

	//We are the only, and therefore the active, record for our master.
	ExecuteParamSql("UPDATE EmrInfoMasterT SET ActiveEmrInfoID = {INT} WHERE ActiveEmrInfoID = {INT}", nNewInfoID, nInfoID);

	//audit the creation
	long nAuditID = BeginNewAuditEvent();
	// (c.haag 2007-02-07 13:47) - PLID 24422 - We must use aeiEMRItemAutoCopied because we are retiring the
	// old item
	AuditEvent(-1, "", nAuditID, aeiEMRItemAutoCopied, nNewInfoID, "", strNewName, aepMedium, aetCreated);

	// (j.jones 2007-07-23 10:46) - PLID 26742 - if changing medications or allergies,
	// send the new InfoID in a tablechecker
	switch (subtype) {
	case eistCurrentMedicationsTable:
		CClient::RefreshTable(NetUtils::CurrentMedicationsEMRInfoID, nNewInfoID);
		break;
	case eistAllergiesTable:
		CClient::RefreshTable(NetUtils::CurrentAllergiesEMRInfoID, nNewInfoID);
		break;
	// (j.jones 2010-06-21 15:21) - PLID 37981 - supported generic tables
	case eistGenericTable:
		CClient::RefreshTable(NetUtils::CurrentGenericTableEMRInfoID, nNewInfoID);
		break;
	}

	// (b.cardillo 2007-09-05 14:49) - PLID 27264 - Made the bulk of this function into one big set-based ad-hoc proc 
	// for significant performance gains.  Note, this is a change in the implementation, and NOT AT ALL a change in 
	// logic.
	_RecordsetPtr prsAll = CreateParamRecordset(
		"SET NOCOUNT ON \r\n"
		"DECLARE @nInfoID INT \r\n"
		"SET @nInfoID = {INT} \r\n"
		"DECLARE @nNewInfoID INT \r\n"
		"SET @nNewInfoID = {INT} \r\n"
		

		//1b.) TES 1/30/2007 - PLID 24377 - Copy all the links this item is a part of, and remember the mapping so that 
		// we can also copy the details.
		//DRT 8/30/2007 - PLID 27259 - Parameterized.
		"DECLARE @tLinksToCopy TABLE (RowNum INT IDENTITY NOT NULL, OldID INT NOT NULL, NewID INT, Name NVARCHAR(255)) \r\n"
		"INSERT INTO @tLinksToCopy (OldID, Name) \r\n"
		" SELECT ID, Name FROM EmrItemLinksT WHERE ID IN \r\n"
		"  (SELECT EmrLinkID FROM EmrItemLinkedDataT WHERE EmrDataID IN (SELECT ID FROM EmrDataT WHERE EmrInfoID = @nInfoID)) \r\n"
		"UPDATE @tLinksToCopy SET NewID = COALESCE((SELECT MAX(ID) FROM EmrItemLinksT), 0) + RowNum \r\n"
		"INSERT INTO EmrItemLinksT (ID, Name) \r\n"
		" SELECT NewID, Name FROM @tLinksToCopy \r\n"
		//TES 1/30/2007 - PLID 24377 - Also, copy the other side of the link (whichever details are not part of this item.).
		"INSERT INTO EmrItemLinkedDataT (EmrLinkID, EmrDataID) \r\n"
		" SELECT NewID, EmrDataID FROM EmrItemLinkedDataT INNER JOIN @tLinksToCopy LTCT ON EmrItemLinkedDataT.EmrLinkID = LTCT.OldID AND EmrDataID NOT IN \r\n"
		"  (SELECT ID FROM EmrDataT WHERE EmrInfoID = @nInfoID) \r\n"

		//2)  Copy all the records in EMRDataT
		// (j.jones 2007-08-14 16:02) - PLID 27053 - added EMRDataGroupsT
		//DRT 8/30/2007 - PLID 27259 - Parameterized.
		"DECLARE @tDatasToCopyT TABLE (RowNum INT IDENTITY NOT NULL, OldID INT NOT NULL, NewID INT) \r\n"
		"INSERT INTO @tDatasToCopyT (OldID) \r\n"
		" SELECT ID FROM EMRDataT WHERE EMRInfoID = @nInfoID \r\n"
		"UPDATE @tDatasToCopyT SET \r\n"
		" NewID = COALESCE((SELECT MAX(ID) FROM EMRDataT), 0) + RowNum \r\n"
		//Make a new record of this data in EMRDataT
		//TES 12/7/2006 - PLID 23766 - Set the EmrDataGroupID to an arbitrary new number, since we are breaking the tie
		// between this item and any previous versions of it.
		//TES 6/4/2008 - PLID 29973 - NO, we are NOT breaking the tie between this item and any previous versions!  Darn
		// copy and paste.  Make sure and keep the same EmrDataGroupID.
		// (z.manning 2010-08-11 17:27) - PLID 40074 - Added AutoNumber fields
		//TES 3/15/2011 - PLID 42757 - Added GlassesOrderDataType, GlassesOrderDataID
		// (z.manning 2011-03-21 10:12) - PLID 23662 - Added AutofillType
		// (j.jones 2011-05-04 15:31) - PLID 43527 - added ListSubType
		// (z.manning 2011-06-01 09:29) - PLID 43865 - DataFlags
		// (z.manning 2011-07-01 17:24) - PLID 43910 - Added missing fields: AutoAlphabetizeDropDown, BOLDCode, DecimalPlaces, Formula, InputMask, CopiedFromDataID
		// (z.manning 2011-09-19 13:52) - PLID 41954 - Dropdown separators
		// (z.manning 2011-11-07 10:59) - PLID 46309 - SpawnedItemsSeparator
		// (r.gonet 08/03/2012) - PLID 51948 - WoundCareDataType
		// (j.gruber 2014-07-18 14:19) - PLID 62624 - Keyword Saving
		// (j.gruber 2014-12-05 16:47) - PLID 64289 - search queue - UseNameForKeyword Saving
		"INSERT INTO EMRDataT (ID, EMRInfoID, Data, Inactive, LongForm, SortOrder, ListType, ListSubType, IsLabel, IsGrouped, \r\n"
		"	EmrDataGroupID, AutoNumberType, AutoNumberPrefix, GlassesOrderDataType, GlassesOrderDataID, AutofillType, DataFlags, \r\n"
		"	AutoAlphabetizeDropDown, BOLDCode, DecimalPlaces, Formula, InputMask, CopiedFromDataID, DropdownSeparator, DropdownSeparatorFinal, SpawnedItemsSeparator, WoundCareDataType, UseKeyword, KeywordOverride, UseNameForKeyword) \r\n"
		" SELECT DTCT.NewID, @nNewInfoID, Data, Inactive, LongForm, SortOrder, ListType, ListSubType, IsLabel, IsGrouped, \r\n"
		"	EmrDataGroupID, AutoNumberType, AutoNumberPrefix, GlassesOrderDataType, GlassesOrderDataID, AutofillType, DataFlags, \r\n"
		"	AutoAlphabetizeDropDown, BOLDCode, DecimalPlaces, Formula, InputMask, EmrDataT.ID, DropdownSeparator, DropdownSeparatorFinal, SpawnedItemsSeparator, WoundCareDataType, UseKeyword, KeywordOverride, UseNameForKeyword \r\n"
		" FROM EMRDataT INNER JOIN @tDatasToCopyT DTCT ON EMRDataT.ID = DTCT.OldID \r\n"

		//2a)  We also need to copy anything in EMRDefaultsT.  This record will only exist for some data
		//	elements, so we only copy some of the time.
		"IF EXISTS(SELECT * FROM EMRInfoDefaultsT WHERE EMRInfoID = @nInfoID AND EMRDataID IN (SELECT OldID FROM @tDatasToCopyT)) BEGIN \r\n"
		"  INSERT INTO EMRInfoDefaultsT (EMRInfoID, EMRDataID) \r\n"
		"   SELECT @nNewInfoID, NewID FROM @tDatasToCopyT \r\n"
		"END \r\n"

		//2b)  We also need to copy any EMR actions that are specific to this data.  SourceType = 4 means
		//	the actions are for this specific EMRData object.
		//  (z.manning, 3/16/2006, PLID 19747) - There's no reason to copy deleted action items.
		"DECLARE @tActionsToCopyT TABLE (RowNum INT IDENTITY NOT NULL, OldID INT NOT NULL, NewID INT, DestType INT NOT NULL, NewDataID INT NOT NULL) \r\n"
		"INSERT INTO @tActionsToCopyT (OldID, DestType, NewDataID) \r\n"
		" SELECT ID, DestType, DTCT.NewID FROM EMRActionsT INNER JOIN @tDatasToCopyT DTCT ON SourceID = DTCT.OldID AND SourceType = 4 WHERE Deleted = 0 \r\n"
		"UPDATE @tActionsToCopyT SET NewID = COALESCE((SELECT MAX(ID) FROM EMRActionsT), 0) + RowNum \r\n"
		//TES 1/30/2007 - PLID 24474 - Don't forget SpawnAsChild!
		"SET IDENTITY_INSERT EMRActionsT ON \r\n"
		"INSERT INTO EMRActionsT (ID, SourceType, SourceID, DestType, DestID, SortOrder, Popup, SpawnAsChild) \r\n"
		" SELECT ATCT.NewID, SourceType, ATCT.NewDataID, EMRActionsT.DestType, DestID, SortOrder, Popup, SpawnAsChild \r\n"
		" FROM EMRActionsT INNER JOIN @tActionsToCopyT ATCT ON EMRActionsT.ID = ATCT.OldID \r\n"
		"SET IDENTITY_INSERT EMRActionsT OFF \r\n"
		//Only if charge type
		"INSERT INTO EMRActionChargeDataT (ActionID, Prompt, DefaultQuantity, Modifier1Number, Modifier2Number, Modifier3Number, Modifier4Number) \r\n"
		" SELECT ATCT.NewID, Prompt, DefaultQuantity, Modifier1Number, Modifier2Number, Modifier3Number, Modifier4Number \r\n"
		" FROM EMRActionChargeDataT INNER JOIN @tActionsToCopyT ATCT ON EMRActionChargeDataT.ActionID = ATCT.OldID \r\n"
		" WHERE ATCT.DestType = " + FormatString("%li", eaoCpt) + " \r\n"
		// (b.savon 2014-07-22 09:24) - PLID 62707 - Handle saving the new Diagnosis DestType action to EmrActionsT and EmrActionsDiagnosisDataT
		"INSERT INTO EMRActionDiagnosisDataT (EmrActionID, DiagCodeID_ICD9, DiagCodeID_ICD10) \r\n"
		" SELECT ATCT.NewID, DiagCodeID_ICD9, DiagCodeID_ICD10 \r\n"
		" FROM EMRActionDiagnosisDataT INNER JOIN @tActionsToCopyT ATCT ON EmrActionDiagnosisDataT.EmrActionID = ATCT.OldID \r\n"
		" WHERE ATCT.DestType = " + FormatString("%li", eaoDiagnosis) + " \r\n"

		

		//2c)  If this is a table, we need to copy data from EMRTableDropdownInfoT
		//DRT 8/30/2007 - PLID 27259 - Parameterized.
		"DECLARE @tDropdownInfoToCopyT TABLE (RowNum INT IDENTITY NOT NULL, OldID INT NOT NULL, NewID INT, NewDataID INT NOT NULL) \r\n"
		"INSERT INTO @tDropdownInfoToCopyT (OldID, NewDataID) \r\n"
		" SELECT ID, DTCT.NewID FROM EMRTableDropdownInfoT INNER JOIN @tDatasToCopyT DTCT ON EMRTableDropdownInfoT.EMRDataID = DTCT.OldID \r\n"
		"UPDATE @tDropdownInfoToCopyT SET \r\n"
		" NewID = COALESCE((SELECT Max(ID) FROM EMRTableDropdownInfoT), 0) + RowNum \r\n"
		//TES 12/7/2006 - PLID 23766 - Set the DropdownGroupID to an arbitrary new number, since we are breaking the tie
		// between this item and any previous versions of it.
		// (a.walling 2007-08-29 12:12) - PLID 27223 - No longer arbirtary; links to EMRTableDropdownGroupsT
		//TES 6/4/2008 - PLID 29973 - NO, we are NOT breaking the tie between this item and any previous versions!  Darn
		// copy and paste.  Make sure to keep the same DropdownGroupID.
		//TES 3/15/2011 - PLID 42757 - Added GlassesOrderDataID
		"INSERT INTO EMRTableDropdownInfoT (ID, EMRDataID, Data, SortOrder, Inactive, GlassesOrderDataID, DropdownGroupID) \r\n"
		" SELECT NewID, NewDataID, Data, SortOrder, Inactive, GlassesOrderDataID, DropdownGroupID \r\n"
		" FROM EMRTableDropdownInfoT INNER JOIN @tDropdownInfoToCopyT DDITCT ON EMRTableDropdownInfoT.ID = DDITCT.OldID \r\n"
		// (z.manning 2011-09-30 12:50) - PLID 45729 - Handle dropdown stamp filter data
		"INSERT INTO EmrTableDropdownStampFilterT (EmrTableDropdownInfoID, StampID) \r\n"
		"SELECT NewID, StampID \r\n"
		"FROM EmrTableDropdownStampFilterT INNER JOIN @tDropdownInfoToCopyT DDITCT ON EmrTableDropdownStampFilterT.EmrTableDropdownInfoID = DDITCT.OldID \r\n"
		// (j.jones 2012-11-26 15:11) - PLID 53144 - added EMRTableDropdownStampDefaultsT
		"INSERT INTO EMRTableDropdownStampDefaultsT (EmrTableDropdownInfoID, StampID) \r\n"
		"SELECT NewID, StampID \r\n"
		"FROM EMRTableDropdownStampDefaultsT INNER JOIN @tDropdownInfoToCopyT DDITCT ON EMRTableDropdownStampDefaultsT.EmrTableDropdownInfoID = DDITCT.OldID \r\n"

		// (c.haag 2010-07-10 11:19) - PLID 39467 - Ensure we did not create multiple values for a dropdown with the same sort order
		"IF EXISTS(SELECT EMRDataID, SortOrder FROM EMRTableDropdownInfoT WHERE EMRDataID IN (SELECT NewDataID FROM @tDropdownInfoToCopyT) GROUP BY EMRDataID, SortOrder HAVING Count(SortOrder) > 1) \r\n"
		"BEGIN RAISERROR('Duplicate SortOrder values detected in EMRTableDropdownInfoT!', 16, 1) RETURN END \r\n"

		//2d) TES 1/30/2007 - PLID 24377 - We also need to copy any links that this data item was a part of.
		//			Note that here, we make a whole new link, as opposed to the PrepareToReassign() code, because
		//			in that code it treats the old and new copy as the "same" item, whereas in this new code the old
		//			and new copies are actually different items.
		//DRT 8/30/2007 - PLID 27259 - Parameterized.
		"DECLARE @tReverseLinksToCopyT TABLE (RowNum INT IDENTITY NOT NULL, EmrLinkID INT NOT NULL, NewLinkID INT, NewDataID INT NOT NULL) \r\n"
		"INSERT INTO @tReverseLinksToCopyT (EmrLinkID, NewDataID) \r\n"
		" SELECT EmrLinkID, DTCT.NewID FROM EmrItemLinkedDataT INNER JOIN @tDatasToCopyT DTCT ON EmrItemLinkedDataT.EmrDataID = DTCT.OldID \r\n"
		"UPDATE @tReverseLinksToCopyT SET NewLinkID = LTC.NewID \r\n"
		"FROM @tReverseLinksToCopyT RLTCT LEFT JOIN @tLinksToCopy LTC ON RLTCT.EmrLinkID = LTC.OldID \r\n"
		"IF EXISTS(SELECT * FROM @tReverseLinksToCopyT WHERE NewLinkID IS NULL) BEGIN \r\n"
		"  RAISERROR('Failed to map EMR Link!', 16, 1) \r\n"
		"  RETURN \r\n"
		"END \r\n"
		"INSERT INTO EmrItemLinkedDataT (EmrLinkID, EmrDataID) \r\n"
		" SELECT RLTCT.NewLinkID, RLTCT.NewDataID FROM @tReverseLinksToCopyT RLTCT \r\n"
		
		//2e) (c.haag 2007-01-30 17:46) - PLID 24422 - And update the DrugList table
		// (c.haag 2007-04-04 16:43) - PLID 25498 - ...or the AllergyT table depending on what
		// kind of item we're branching
		"DECLARE @nSubType INT \r\n"
		"SET @nSubType = {INT} \r\n"
		"IF (@nSubType = " + FormatString("%li", eistCurrentMedicationsTable) + ") BEGIN \r\n"
		"  UPDATE DrugList SET EMRDataID = NewID \r\n"
		"  FROM DrugList INNER JOIN @tDatasToCopyT DTCT ON DrugList.EMRDataID = DTCT.OldID \r\n"
		"END ELSE IF (@nSubType = " + FormatString("%li", eistAllergiesTable) + ") BEGIN \r\n"
		"  UPDATE AllergyT SET EMRDataID = NewID \r\n"
		"  FROM AllergyT INNER JOIN @tDatasToCopyT DTCT ON AllergyT.EMRDataID = DTCT.OldID \r\n"
		"END ELSE IF (@nSubType != " + FormatString("%li", eistGenericTable) + ") BEGIN \r\n"
		"  RAISERROR('Unexpected subtype!', 16, 1) \r\n"
		"  RETURN \r\n"
		"END \r\n"

		//3) (c.haag 2011-03-15) - PLID 42821 - Copy common lists. Being consistent with the above code, we don't check
		//  every single call for errors
		//		a) Create a table variable that maps old EmrInfoCommonListT.ID's to new EmrInfoCommonListT.ID's.
		"DECLARE @tNewCommonListT TABLE (RowNum INT IDENTITY NOT NULL PRIMARY KEY, OldID INT NOT NULL, NewID INT) \r\n"
		"INSERT INTO @tNewCommonListT (OldID) \r\n"
		" SELECT ID FROM EmrInfoCommonListT WHERE EMRInfoID = {INT} \r\n"
		"UPDATE @tNewCommonListT SET NewID = RowNum + COALESCE((SELECT MAX(ID) FROM EmrInfoCommonListT), 0) \r\n"

		//		b) Copy the contents of EmrInfoCommonListT for the old EmrInfoID to the new EmrInfoID
		// (c.haag 2011-04-11) - PLID 43234 - Also copy GroupOnPreviewPane
		"SET IDENTITY_INSERT EmrInfoCommonListT ON \r\n"
		"INSERT INTO EmrInfoCommonListT (ID, EmrInfoID, Name, Color, Inactive, OrderID, GroupOnPreviewPane) \r\n"
		" SELECT NewID, {INT}, Name, Color, Inactive, OrderID, GroupOnPreviewPane \r\n"
		" FROM EmrInfoCommonListT INNER JOIN @tNewCommonListT ON ID = OldID \r\n"
		"SET IDENTITY_INSERT EmrInfoCommonListT OFF \r\n"

		//		c) Create a table variable that maps old EmrInfoCommonListItemsT.ID's to new EmrInfoCommonListItemsT.ID's.
		"DECLARE @tNewCommonListItemsT TABLE (RowNum INT IDENTITY NOT NULL PRIMARY KEY, OldID INT NOT NULL, NewID INT) \r\n"
		"INSERT INTO @tNewCommonListItemsT (OldID) \r\n"
		" SELECT ID FROM EmrInfoCommonListItemsT WHERE ListID IN (SELECT ID FROM EmrInfoCommonListT WHERE EMRInfoID = {INT}) \r\n"
		"UPDATE @tNewCommonListItemsT SET NewID = RowNum + COALESCE((SELECT MAX(ID) FROM EmrInfoCommonListItemsT), 0) \r\n"

		//		d) Copy the contents of EmrInfoCommonListItemsT for the old EmrInfoID to the new EmrInfoID
		"SET IDENTITY_INSERT EmrInfoCommonListItemsT ON \r\n"
		"INSERT INTO EmrInfoCommonListItemsT (ID, ListID, EmrDataID) \r\n"
		" SELECT T1.NewID, T2.NewID, T3.NewID  \r\n"
		" FROM EmrInfoCommonListItemsT E "
		"	INNER JOIN @tNewCommonListItemsT T1 ON E.ID = T1.OldID \r\n"
		"	INNER JOIN @tNewCommonListT T2 ON E.ListID = T2.OldID \r\n"
		"	INNER JOIN @tDatasToCopyT T3 ON E.EmrDataID = T3.OldID \r\n"
		"SET IDENTITY_INSERT EmrInfoCommonListItemsT OFF \r\n"

		"SET NOCOUNT OFF \r\n"
		,
		nInfoID, nNewInfoID, subtype
		//3) (c.haag 2011-03-15) - PLID 42821 
		,nInfoID // a)
		,nNewInfoID // b)
		,nInfoID // c)
		);


	//3)  Copy any EMR actions for this whole detail, we can batch copy these.  SourceType = 3 means
	//	the actions are for the entire EMRInfo object.
	{
		//DRT 1/18/2007 - PLID 24181 - We need to copy charge-specific action data, thus cannot batch copy any longer.
		//	Put all into a string for speed.
		CString strActionSql;
		//DRT 8/30/2007 - PLID 27259 - Parameterized.
		_RecordsetPtr prsActions = CreateParamRecordset("SELECT ID, DestType FROM EMRActionsT WHERE SourceID = {INT} AND SourceType = 3", nInfoID);
		while(!prsActions->eof) {
			long nActionID = AdoFldLong(prsActions, "ID");
			EmrActionObject eaoType = (EmrActionObject)AdoFldLong(prsActions, "DestType");

			//TES 1/30/2007 - PLID 24474 - Don't forget SpawnAsChild!
			strActionSql += FormatString("INSERT INTO EMRActionsT (SourceType, SourceID, DestType, DestID, SortOrder, Popup, SpawnAsChild) "
				"SELECT SourceType, %li, DestType, DestID, SortOrder, Popup, SpawnAsChild "
				"FROM EMRActionsT WHERE ID = %li", nNewInfoID, nActionID);

			// (b.savon 2014-07-22 09:24) - PLID 62707 - Create a var and use it for charge type and diagnosis type
			strActionSql += "DECLARE @InfoObject_EmrActionID INT; SET @InfoObject_EmrActionID = SCOPE_IDENTITY();";

			//Only if charge type
			if(eaoType == eaoCpt) {
				strActionSql += FormatString("INSERT INTO EMRActionChargeDataT (ActionID, Prompt, DefaultQuantity, Modifier1Number, "
					"Modifier2Number, Modifier3Number, Modifier4Number) SELECT @InfoObject_EmrActionID, Prompt, DefaultQuantity, Modifier1Number, "
					"Modifier2Number, Modifier3Number, Modifier4Number FROM EMRActionChargeDataT WHERE ActionID = %li", nActionID);
			}
			// (b.savon 2014-07-22 09:24) - PLID 62707 - Handle saving the new Diagnosis DestType action to EmrActionsT and EmrActionsDiagnosisDataT
			if (eaoType == eaoDiagnosis){
				strActionSql += FormatString(
					R"(
					INSERT INTO EmrActionDiagnosisDataT (EmrActionID, DiagCodeID_ICD9, DiagCodeID_ICD10)
					SELECT	@InfoObject_EmrActionID, DiagCodeID_ICD9, DiagCodeID_ICD10
					FROM	EmrActionDiagnosisDataT WHERE EmrActionID = %li"
					)",
					nActionID
				);
			}

			prsActions->MoveNext();
		}

		//execute 'em
		if(!strActionSql.IsEmpty())
			ExecuteSqlStd(strActionSql);
	}

	// (b.cardillo 2007-09-06 15:26) - PLID 27264 - Inexplicably this query adds a minimum of 350 ms to the 
	// above ad-hoc proc if it's in there (whether cached or not), but when executed separately usually takes 
	// less than 20 ms.  It used to take 400 ms separately but now that we disable the trigger, it goes much 
	// faster.  This is safe to do because we know that we're adding the category for a new (and therefore 
	// unused) info item.  This is a reliable conclusion because we know we are inside the same transaction 
	// with the creation of the info item itself.
	// (j.armen 2014-02-04 16:58) - PLID 60641 - We don't disable triggers anymore
	//	Instead, set the EMRTriggerState flag for this transaction
	//4)  Copy EMRInfoCategoryT
	ExecuteParamSql(
		"SET XACT_ABORT ON\r\n"
		"BEGIN TRAN\r\n"
		"SET NOCOUNT ON \r\n"
		"UPDATE EMRTriggerStateT SET Enabled = 0\r\n"
		"INSERT INTO EMRInfoCategoryT (EMRInfoID, EMRCategoryID) \r\n"
		" SELECT {INT}, EMRCategoryID FROM EMRInfoCategoryT WHERE EMRInfoID = {INT} \r\n"
		"UPDATE EMRTriggerStateT SET Enabled = 1\r\n"
		"SET NOCOUNT OFF \r\n"
		"COMMIT TRAN\r\n",
		nNewInfoID, nInfoID);

	return nNewInfoID;
}

long BranchCurrentMedicationsInfoItem()
{
	// (c.haag 2007-01-30 17:30) - PLID 24422 - Generates a new Current Medications info item.
	// This should be called just prior to making changes to the official Current Medications
	// info item if the item already exists on EMN's.
	return BranchSystemInfoItem(eistCurrentMedicationsTable);
}

long BranchAllergiesInfoItem()
{
	// (c.haag 2007-04-04 16:40) - PLID 25498 - Generates a new Allergies info item.
	// This should be called just prior to making changes to the official Allergies info item
	// if the item already exists on EMN's.
	return BranchSystemInfoItem(eistAllergiesTable);
}

// (j.jones 2010-06-21 15:20) - PLID 37981 - branch the generic table item
long BranchGenericTableItem()
{
	return BranchSystemInfoItem(eistGenericTable);
}

void WarnMedicationDiscrepanciesWithEMR(long nPatientID)
{
	// (c.haag 2007-02-07 13:29) - PLID 24420 - Go through every patient EMN and warn the user
	// if there are differences between the patient medications list and the official medications
	// detail of any EMN. The "official medications" detail of an EMN refers to any Current Medications
	// detail that corresponds to the latest version of the Current Medications info item. If
	// there are multiple, they will all have the same state.
	//
	// We've already checked the licensing by this point
	//
	try {
		CString strWarning = "The following medical records have medication-related information which differs from the patient's medication list:\n\n";
		BOOL bShowWarning = FALSE;
		CWaitCursor wc;

		// Load in the medication ID's for the patient
		// (c.haag 2007-02-02 17:26) - PLID 24561 - We now store medication names in EmrDataT.Data rather than DrugList.Name
		CArray<long,long> anPtDrugListIDs;
		CStringArray astrPtDrugListNames;
		//DRT 8/30/2007 - PLID 27259 - Parameterized.
		_RecordsetPtr prsPt = CreateParamRecordset("SELECT MedicationID, EMRDataT.Data AS Name FROM CurrentPatientMedsT "
			"LEFT JOIN DrugList ON DrugList.ID = CurrentPatientMedsT.MedicationID "
			"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
			"WHERE PatientID = {INT} "
			"ORDER BY Name ", nPatientID);
		while (!prsPt->eof) {
			anPtDrugListIDs.Add(AdoFldLong(prsPt, "MedicationID"));
			astrPtDrugListNames.Add(AdoFldString(prsPt, "Name"));
			prsPt->MoveNext();
		}
		prsPt->Close();

		// This query will return one detail for every unlocked, undeleted EMN that is rooted in the active
		// Current Medications info item. It doesn't matter which detail on an EMN we pull because they should
		// all have the same state.
		//DRT 8/30/2007 - PLID 27259 - Parameterized.
		_RecordsetPtr prsEMN = CreateParamRecordset(
			FormatString("SELECT EMRMasterT.ID, EMRMasterT.Description AS EMNName, EMRGroupsT.Description AS EMRName, Min(EMRDetailsT.ID) AS EMRDetailID "
			"FROM EMRMasterT "
			"LEFT JOIN EMRGroupsT ON EMRMasterT.EMRGroupID = EMRGroupsT.ID "
			"LEFT JOIN EMRDetailsT ON EMRDetailsT.EMRID = EMRMasterT.ID "
			"LEFT JOIN EMRInfoT ON EMRInfoT.ID = EMRDetailsT.EmrInfoID "
			"WHERE EMRMasterT.PatientID = {INT} AND EMRMasterT.Deleted = 0 AND EMRDetailsT.Deleted = 0 AND EMRGroupsT.Deleted = 0 AND EMRMasterT.Status <> 2 "
			"AND EMRInfoID IN (SELECT ID FROM EMRInfoT WHERE DataSubType = %li AND ID IN (SELECT ActiveEMRInfoID FROM EmrInfoMasterT)) "
			"GROUP BY EMRMasterT.ID, EMRMasterT.Description, EMRGroupsT.Description "
			, eistCurrentMedicationsTable), nPatientID);

		int nEMN = 1; // Keep a count of EMN's we're logging so that we don't overinflate the message box size
		while (!prsEMN->eof) {
			CString strEMRName = AdoFldString(prsEMN, "EMRName");
			CString strEMNName = AdoFldString(prsEMN, "EMNName");
			long nEMNID = AdoFldLong(prsEMN, "ID");
			long nDetailID = AdoFldLong(prsEMN, "EmrDetailID");
			CArray<long,long> anEMNDrugListIDs;

			// (c.haag 2007-08-18 12:55) - PLID 27115 - Run a query to figure out what Current Medications
			// are selected in the detail
			_RecordsetPtr prsDetailSel = CreateParamRecordset(
				FormatString("SELECT DrugList.ID "
					"FROM EmrDetailTableDataT "
					"LEFT JOIN DrugList ON DrugList.EmrDataID = EmrDataID_X "
					"WHERE EmrDetailID = {INT} and Data = '1' and EmrDataID_Y IN ("
					"	SELECT EmrDataT.ID FROM EmrInfoMasterT "
					"	LEFT JOIN EmrInfoT ON EmrInfoT.ID = EmrInfoMasterT.ActiveEmrInfoID "
					"	LEFT JOIN EmrDataT ON EmrDataT.EmrInfoID = EmrInfoMasterT.ActiveEmrInfoID "
					"WHERE EmrInfoT.DataSubType = %d AND EmrDataT.SortOrder = 1 AND EmrDataT.ListType = %d) ",
					eistCurrentMedicationsTable, LIST_TYPE_CHECKBOX)
				, nDetailID);
			while (!prsDetailSel->eof) {
				anEMNDrugListIDs.Add(AdoFldLong(prsDetailSel, "ID"));
				prsDetailSel->MoveNext();
			}

			// Now compare with those we got from the patient
			if (!AreArrayContentsMatched(anEMNDrugListIDs, anPtDrugListIDs)) {

				// If there is a mismatch, build a warning message
				CString str;
				if (strEMRName.IsEmpty()) strEMRName = "<no name>";
				if (strEMNName.IsEmpty()) strEMNName = "<no name>";
				str.Format("EMR: \"%s\" EMN: \"%s\"\n", strEMRName, strEMNName);
				strWarning += str;

				// Flag the fact we want to show the warning
				bShowWarning = TRUE;

				// Cap the message box so it doesn't get ridiculously big
				if (++nEMN == 20) {
					strWarning += "<more>\n";
					break;
				}
			}

			prsEMN->MoveNext();
		} // while (!prsEMN->eof) {

		// Don't keep the recordset open any longer than it has to
		prsEMN->Close();

		if (bShowWarning) {
			strWarning += "\nPlease ensure that the values of the Current Medications details on these EMNs are accurate.";
			// (c.haag 2007-02-14 09:42) - PLID 24420 - Don't use MsgBox because Practice will
			// try to format medication names with % signs in them
			AfxMessageBox(strWarning);
		}

	}
	NxCatchAll("Error in WarnMedicationDiscrepanciesWithEMR");
}

void WarnAllergyDiscrepanciesWithEMR(long nPatientID)
{
	//
	// (c.haag 2007-02-07 13:29) - PLID 25524 - Go through every patient EMN and warn the user
	// if there are differences between the patient allergy list and the allergy
	// detail of any EMN. The "official allergies" detail of an EMN refers to any Allergies
	// detail that corresponds to the latest version of the Allergies info item. If
	// there are multiple, they will all have the same state.
	//
	// We've already checked the licensing by this point
	//
	try {
		CString strWarning = "The following medical records have allergy-related information which differs from the patient's allergy list:\n\n";
		BOOL bShowWarning = FALSE;
		CWaitCursor wc;

		// Load in the allergy ID's for the patient
		CArray<long,long> anPtAllergyIDs;
		CStringArray astrPtAllergyNames;
		//DRT 8/30/2007 - PLID 27259 - Parameterized.
		_RecordsetPtr prsPt = CreateParamRecordset("SELECT AllergyID, EMRDataT.Data AS Name FROM PatientAllergyT "
			"LEFT JOIN AllergyT ON AllergyT.ID = PatientAllergyT.AllergyID "
			"LEFT JOIN EMRDataT ON AllergyT.EMRDataID = EMRDataT.ID "
			"WHERE PersonID = {INT} "
			"ORDER BY Name ", nPatientID);
		while (!prsPt->eof) {
			anPtAllergyIDs.Add(AdoFldLong(prsPt, "AllergyID"));
			astrPtAllergyNames.Add(AdoFldString(prsPt, "Name"));
			prsPt->MoveNext();
		}
		prsPt->Close();

		// This query will return one detail for every unlocked, undeleted EMN that is rooted in the active
		// Allergies info item. It doesn't matter which detail on an EMN we pull because they should
		// all have the same state.
		//DRT 8/30/2007 - PLID 27259 - Parameterized.
		_RecordsetPtr prsEMN = CreateParamRecordset(
			FormatString("SELECT EMRMasterT.ID, EMRMasterT.Description AS EMNName, EMRGroupsT.Description AS EMRName, Min(EMRDetailsT.ID) AS EMRDetailID "
			"FROM EMRMasterT "
			"LEFT JOIN EMRGroupsT ON EMRMasterT.EMRGroupID = EMRGroupsT.ID "
			"LEFT JOIN EMRDetailsT ON EMRDetailsT.EMRID = EMRMasterT.ID "
			"LEFT JOIN EMRInfoT ON EMRInfoT.ID = EMRDetailsT.EmrInfoID "
			"WHERE EMRMasterT.PatientID = {INT} AND EMRMasterT.Deleted = 0 AND EMRDetailsT.Deleted = 0 AND EMRGroupsT.Deleted = 0 AND EMRMasterT.Status <> 2 "
			"AND EMRInfoID IN (SELECT ID FROM EMRInfoT WHERE DataSubType = %d AND ID IN (SELECT ActiveEMRInfoID FROM EmrInfoMasterT)) "
			"GROUP BY EMRMasterT.ID, EMRMasterT.Description, EMRGroupsT.Description "
			, eistAllergiesTable), nPatientID);

		int nEMN = 1; // Keep a count of EMN's we're logging so that we don't overinflate the message box size
		while (!prsEMN->eof) {
			CString strEMRName = AdoFldString(prsEMN, "EMRName");
			CString strEMNName = AdoFldString(prsEMN, "EMNName");
			long nEMNID = AdoFldLong(prsEMN, "ID");
			long nDetailID = AdoFldLong(prsEMN, "EmrDetailID");
			CArray<long,long> anEMNAllergyIDs;

			// (c.haag 2007-08-18 13:35) - PLID 27116 - Run a query to figure out which allergies are
			// selected in the detail
			_RecordsetPtr prsDetailSel = CreateParamRecordset(
				FormatString("SELECT AllergyT.ID "
					"FROM EmrDetailTableDataT "
					"LEFT JOIN AllergyT ON AllergyT.EmrDataID = EmrDataID_X "
					"WHERE EmrDetailID = {INT} and Data = '1' and EmrDataID_Y IN ("
					"	SELECT EmrDataT.ID FROM EmrInfoMasterT "
					"	LEFT JOIN EmrInfoT ON EmrInfoT.ID = EmrInfoMasterT.ActiveEmrInfoID "
					"	LEFT JOIN EmrDataT ON EmrDataT.EmrInfoID = EmrInfoMasterT.ActiveEmrInfoID "
					"WHERE EmrInfoT.DataSubType = %d AND EmrDataT.SortOrder = 1 AND EmrDataT.ListType = %d) ",
					eistAllergiesTable, LIST_TYPE_CHECKBOX)
				, nDetailID);
			while (!prsDetailSel->eof) {
				anEMNAllergyIDs.Add(AdoFldLong(prsDetailSel, "ID"));
				prsDetailSel->MoveNext();
			}

			// Now compare with those we got from the patient
			if (!AreArrayContentsMatched(anEMNAllergyIDs, anPtAllergyIDs)) {

				// If there is a mismatch, build a warning message
				CString str;
				if (strEMRName.IsEmpty()) strEMRName = "<no name>";
				if (strEMNName.IsEmpty()) strEMNName = "<no name>";
				str.Format("EMR: \"%s\" EMN: \"%s\"\n", strEMRName, strEMNName);
				strWarning += str;

				// Flag the fact we want to show the warning
				bShowWarning = TRUE;

				// Cap the message box so it doesn't get ridiculously big
				if (++nEMN == 20) {
					strWarning += "<more>\n";
					break;
				}
			}

			prsEMN->MoveNext();
		} // while (!prsEMN->eof) {

		// Don't keep the recordset open any longer than it has to
		prsEMN->Close();

		if (bShowWarning) {
			strWarning += "\nPlease ensure that the values of the Allergies details on these EMNs are accurate.";
			AfxMessageBox(strWarning);
		}

	}
	NxCatchAll("Error in WarnAllergyDiscrepanciesWithEMR");
}

void WarnEmrDataDiscrepanciesWithDrugList()
{
	// (c.haag 2007-02-07 13:33) - PLID 24423 - Warns the user if EmrDataT and DrugList have referential
	// integrity failures. This is done in the form of an exception so that the user knows it's a critical
	// problem, and that we should be contacted right away
	try {
		//TES 10/12/2010 - PLID 40907 - Moved the query to a shared function, for use by NexEmrImporter
		BOOL bHasMislinkedDataRecords = FALSE, bHasMislinkedDrugRecords = FALSE;
		CheckDrugListDiscrepancies(GetRemoteData(), bHasMislinkedDataRecords, bHasMislinkedDrugRecords);

		if(bHasMislinkedDataRecords){
			ThrowNxException("Found items in EmrDataT not linked to DrugList!");
		}
		else if(bHasMislinkedDrugRecords){
			ThrowNxException("Found items in DrugList improperly linked to EmrDataT!");
		}
	}
	NxCatchAll("Error validating the master medications list");
}

void WarnEmrDataDiscrepanciesWithAllergyT()
{
	// (c.haag 2007-04-03 09:37) - PLID 25468 - Warn the user if EmrDataT and AllergyT have referential
	// integrity failures. This is done in the form of an exception so that the user knows it's a critical
	// problem, and that we should be contacted right away
	try {
		//TES 10/12/2010 - PLID 40907 - Moved the query to a shared function, for use by NexEmrImporter
		BOOL bHasMislinkedDataRecords = FALSE, bHasMislinkedAllergyRecords = FALSE;
		CheckAllergyDiscrepancies(GetRemoteData(), bHasMislinkedDataRecords, bHasMislinkedAllergyRecords);
		if(bHasMislinkedDataRecords) {
			ThrowNxException("Found items in EmrDataT not linked to AllergyT!");
		}
		else if(bHasMislinkedAllergyRecords){
			ThrowNxException("Found items in AllergyT improperly linked to EmrDataT!");
		}
	}
	NxCatchAll("Error validating the master allergy list");
}

// (z.manning 2011-11-01 09:21) - PLID 44594 - Added bSlient param
BOOL AreEmnProvidersLicensed(IN long nEmnID, const CString &strEmnDescription /*= ""*/, OPTIONAL IN CArray<long,long> *paryProviders /*= NULL*/, BOOL bSilent /* = FALSE */)
{
	CString strExtraDescription;
	if(!strEmnDescription.IsEmpty()) {
		strExtraDescription = " (" + strEmnDescription + ") ";
	}
	CArray<long,long> arProviderIDs;
	if(paryProviders) {
		for(int i = 0; i < paryProviders->GetSize(); i++) arProviderIDs.Add(paryProviders->GetAt(i));
	}
	else {
		//Load from data.
		//DRT 8/30/2007 - PLID 27259 - Parameterized.
		_RecordsetPtr rsProviders = CreateParamRecordset("SELECT ProviderID FROM EmrProvidersT WHERE EmrID = {INT} AND Deleted = 0", nEmnID);
		while(!rsProviders->eof) {
			arProviderIDs.Add(AdoFldLong(rsProviders, "ProviderID"));
			rsProviders->MoveNext();
		}
	}

	if(arProviderIDs.GetSize() == 0) {
		//Stop right there!
		if(!bSilent) {
		MsgBox("You must assign a licensed provider to this EMN%s before locking it.  Please go to the '<More Info>' topic and select a provider before continuing.", strExtraDescription);
		}
		return FALSE;
	}
	else
	{
		//Let's split this into two arrays, licensed and unlicensed.
		CArray<long,long> arLicensedProviders;
		CArray<long,long> arUnlicensedProviders;
		for(int i = 0; i < arProviderIDs.GetSize(); i++) {
			//Is this provider licensed?
			long nProviderID = arProviderIDs[i];
			// (a.wilson 2012-08-08 16:50) - PLID 52043 - new function to handle removed code.
			if (g_pLicense->IsProviderLicensed(nProviderID)) {
				arLicensedProviders.Add(nProviderID);
			} else {
				arUnlicensedProviders.Add(nProviderID);
			}
		}
		if(arUnlicensedProviders.GetSize())
		{
			//Our message is going to include the names of all the unlicensed providers.
			CString strUnlicensedIDs = ArrayAsString(arUnlicensedProviders, false);
			_RecordsetPtr rsUnlicensedNames = CreateRecordset("SELECT First + CASE WHEN Middle = '' THEN '' ELSE ' ' + Middle END + ' ' + Last AS Name "
				"FROM PersonT WHERE ID IN (%s)", strUnlicensedIDs);
			CString strUnlicensedProviderNames;
			while(!rsUnlicensedNames->eof) {
				strUnlicensedProviderNames += AdoFldString(rsUnlicensedNames, "Name") + "\r\n";
				rsUnlicensedNames->MoveNext();
			}
			strUnlicensedProviderNames.TrimRight("\r\n");
			//Do they have any licensed providers?
			if(arLicensedProviders.GetSize()) {
				//OK, just tell them to remove the unlicensed ones.
				if(!bSilent) {
				MsgBox("The following provider(s) are not licensed for your EMR.  You cannot lock an EMN which has unlicensed providers assigned to it.  Please remove these unlicensed providers from this EMN%s before continuing.\r\n%s", strExtraDescription, strUnlicensedProviderNames);
				}
				return FALSE;
			}
			else {
				//Is there just one unlicensed provider?
				if(arUnlicensedProviders.GetSize() == 1) {
					//They can't save, but maybe they can license this provider and then save.
					long nProviderCountRemaining = g_pLicense->GetEMRProvidersCountAllowed() - g_pLicense->GetEMRProvidersCountUsed();	
					if(nProviderCountRemaining > 0 ) {
						CString strMessage;
						if(nProviderCountRemaining == 1) {
							strMessage.Format("The provider assigned to this EMN%s, %s, is not one of your licensed EMR providers. "
								"However, you have 1 unused EMR Provider license.  Would you like to assign %s to this license now?\r\n\r\n"
								"If 'Yes', this provider will immediately be assigned as your final licensed EMR provider, leaving you with 0 available licenses, and the save will continue.  This cannot be undone.\r\n\r\n"
								"If 'No', the save will abort, and you must go to the <More Info> topic and assign a licensed provider to this EMN before you can lock it.",
								strExtraDescription, strUnlicensedProviderNames, strUnlicensedProviderNames); 
						}
						else {
							strMessage.Format("The provider assigned to this EMN%s, %s, is not one of your licensed EMR providers. "
								"However, you have %li unused EMR Provider licenses.  Would you like to assign %s to one of these licenses now?\r\n\r\n"
								"If 'Yes', this provider will immediately be assigned as one of your licensed EMR providers, leaving you with %li available license(s), and the save will continue.  This cannot be undone.\r\n\r\n"
								"If 'No', the save will abort, and you must go to the <More Info> topic and assign a licensed provider to this EMN before you can lock it.",
								strExtraDescription, strUnlicensedProviderNames, nProviderCountRemaining, strUnlicensedProviderNames, nProviderCountRemaining-1);
						}

						if(!bSilent && IDYES == MsgBox(MB_YESNO, "%s", strMessage)) {
							//OK, they want to try to license this person.
							if(!g_pLicense->RequestEMRProvider(arUnlicensedProviders[0])) {
								//It failed, and they were told why.  Reset the status on the more info screen and return.
								MsgBox("The EMN could not be locked");
								return FALSE;
							}
						}
						else {
							return FALSE;
						}
					}
					else {
						//Stop right there!
						if(!bSilent) {
						MsgBox("The provider %s is not one of your licensed EMR providers, and you have no available EMR Provider licenses.  You must assign a licensed provider to this EMN%s before locking it.  Please go to the '<More Info>' topic and select a provider before continuing.", strUnlicensedProviderNames, strExtraDescription);
						}
						return FALSE;
					}
				}
				else {
					//We don't want to mass-assign providers, so just tell them they can't continue.
					if(!bSilent) {
					MsgBox("You have selected the following providers, which are not licensed for your EMR.  You cannot lock an EMN which has unlicensed providers assigned to it.  Please remove these unlicensed providers from this EMN%s, and assign one or more licensed providers to it, before continuing.\r\n%s", strExtraDescription, strUnlicensedProviderNames);
					}
					return FALSE;
				}
			}
		}
		else {
			//If they don't have any unlicensed providers, then if we get here we know they've got at least one licensed
			// provider (otherwise arProviderIDs would have been empty).  So, they're OK to continue.
		}
	}

	//If we get down here, then they must be OK.
	return TRUE;
}

// (a.walling 2007-04-02 14:15) - PLID 20002 - Helper function to load (not charge specific) variables from a recordset (using ActionsT-derived names)
// (a.walling 2007-08-08 16:33) - PLID 20002 - Not used, removed
/*EmrAction::LoadFromRecordset(_RecordsetPtr prs)
{
	try {
		nID = AdoFldLong(prs, "ID");
		eaoSourceType = static_cast<EmrActionObject>(AdoFldLong(prs, "SourceType"));
		nSourceID = AdoFldLong(prs, "SourceID");
		eaoDestType = static_cast<EmrActionObject>(AdoFldLong(prs, "DestType"));
		nDestID = AdoFldLong(prs, "DestID");
		bPopup = AdoFldBool(prs, "PopUp");
		nSortOrder = AdoFldLong(prs, "SortOrder");
		bSpawnAsChild = AdoFldBool(prs, "SpawnAsChild");
		// (j.jones 2007-01-22 11:00) - PLID 24356 - added Deleted variable
		bDeleted = AdoFldBool(prs, "Deleted");
		// (j.jones 2007-07-16 16:16) - PLID 26694 - added bOnePerEmn
		bOnePerEmn = AdoFldBool(prs, "OnePerEmn");
	} NxCatchAllThrow("Error in EmrAction::LoadFromRecordset()"); // throw the exceptions to the caller rather than catch here and leave with a bad struct
}*/

// (a.walling 2007-04-25 13:57) - PLID 25549 - Moved from EMN.cpp
CRect GetDetailRectangle(CEMNDetail* pDetail)
{
	//
	// (c.haag 2007-03-27 15:06) - PLID 25374 - This function is used to calculate the bounds
	// of an EMN detail rectangle for list placement. This was copied out of the old implementation
	// of CEMN::SortAllDetails
	//

	BOOL bValidAdvDlg = (pDetail->m_pEmrItemAdvDlg != NULL && pDetail->m_pEmrItemAdvDlg->GetSafeHwnd() != NULL);
	if(!bValidAdvDlg && pDetail->m_rcDefaultClientArea.IsRectNull())
	{
		// (z.manning 2012-06-22 16:32) - PLID 48138 - If we get here it means that this detail does not have
		// a dialog nor does it have a valid default position. Basically we have a detail, but do not know
		// its position on its topic, which is exactly what this function is trying to return. Let's go ahead
		// and create its adv dialog so it has a valid position.
		pDetail->EnsureTopicWndAndItemAdvDlg();

		bValidAdvDlg = (pDetail->m_pEmrItemAdvDlg != NULL && pDetail->m_pEmrItemAdvDlg->GetSafeHwnd() != NULL);
	}

	CRect rc;
	if(bValidAdvDlg) {
		pDetail->m_pEmrItemAdvDlg->GetClientRect(&rc);
		pDetail->m_pEmrItemAdvDlg->ClientToScreen(&rc);
		pDetail->m_pEmrItemAdvDlg->GetParent()->SendMessage(NXM_CONVERT_RECT_FOR_DATA, (WPARAM)&rc);
	}
	else {
		rc = pDetail->m_rcDefaultClientArea;
	}
	return rc;
}

/////////////// (a.walling 2007-04-03 08:45) - PLID 25454 - Helper functions to sort topics and details (using CRT qsort)
// I was very tempted to derive a class from CArray for topics/details that will sort automatically when adding, but
// at the same time I feared places relying on adding to the tail. Since this will probably not be used often anyway, this
// should suffice. Simply pass in an array of detail or topic pointers and the array will be sorted.

// (a.walling 2013-10-01 11:45) - PLID 58829 - Simply returns -1 if null, or the topic order index if not
inline long SafeGetTopicOrderIndex(CEMRTopic* pTopic)
{
	if (!pTopic) {
		return -1;
	} else {
		return pTopic->GetTopicOrderIndex();
	}
}

// (a.walling 2007-04-25 10:54) - PLID 25454 - Compare the topics based on order index
// (a.walling 2013-10-01 11:45) - PLID 58829 - Now using a functor predicate
bool CompareTopicSortOrder::operator()(CEMRTopic* pTopicL, CEMRTopic* pTopicR) const
{
	if (pTopicL == pTopicR) {
		return false;
	}

	return SafeGetTopicOrderIndex(pTopicL) < SafeGetTopicOrderIndex(pTopicR);
}

// (a.walling 2013-10-01 11:45) - PLID 58829 - Now using a functor predicate
bool CompareRect::operator()(const RECT& l, const RECT& r) const
{
	// basically, we go top-down, and then left to right if there is equality.
	if (l.top < r.top) {
		return true;
	}
	if (l.top > r.top) {
		return false;
	}
	// otherwise tops are equal, compare lefts
	if (l.left < r.left) {
		return true;
	}
	if (l.left > r.left) {
		return false;
	}

	// top and left are equal, so we can compare width
	if (l.right < r.right) {
		return true;
	}
	if (l.right > r.right) {
		return false;
	}

	// and finally compare height
	if (l.bottom < r.bottom) {
		return true;
	}
	if (l.bottom > r.bottom) {
		return false;
	}

	return false;
}

// (a.walling 2007-04-25 10:54) - PLID 25454 - Compare details based on top, then left
// (a.walling 2013-10-01 11:45) - PLID 58829 - Now using a functor predicate
bool CompareDetailSortOrder::operator()(CEMNDetail* pDetailL, CEMNDetail* pDetailR) const
{
	//TRACE("Compare 0x%08x < 0x%08x\r\n", pDetailL, pDetailR);
	if (pDetailL == pDetailR) {
		return false;
	}

	if (pDetailL && pDetailR) {
		if (pDetailL->m_pParentTopic != pDetailR->m_pParentTopic) {
			// compare parent topics
			long topicOrderL = SafeGetTopicOrderIndex(pDetailL->m_pParentTopic);
			long topicOrderR = SafeGetTopicOrderIndex(pDetailR->m_pParentTopic);

			if (topicOrderL != topicOrderR) {
				//TRACE("Compare 0x%08x < 0x%08x : via topics 0x%08x, 0x%08x order index %li, %li\r\n", pDetailL, pDetailR, pDetailL->m_pParentTopic, pDetailR->m_pParentTopic, topicOrderL, topicOrderR);
				return topicOrderL < topicOrderR;
			}
		}

		// use GetDetailRectangle
		CRect rectL = GetDetailRectangle(pDetailL);
		CRect rectR = GetDetailRectangle(pDetailR);

		//TRACE("Compare rect(%li,%li)(%li,%li) < rect(%li,%li)(%li,%li)\r\n", rectL, rectR);
		CompareRect compareRect;

		if (compareRect(rectL, rectR)) {
			return true;
		}
		if (compareRect(rectR, rectL)) {
			return false;
		}

		// for consistent ordering, older details are < newer ones, based on ID
		if (pDetailL->GetID() == pDetailR->GetID()) {
			return false;
		}

		// if the right detail is -1, it is new, and should be higher
		if (pDetailR->GetID() == -1) {
			return true;
		}
		if (pDetailL->GetID() == -1) {
			return false;
		}

		// IDs are not equal, and not -1, so return based on ID
		return pDetailL->GetID() < pDetailR->GetID();
	} else {
		if (pDetailR) {
			// left is null, right is not, so we can say null < not-null
			
			//TRACE("Compare 0x%08x < 0x%08x : true via null\r\n", pDetailL, pDetailR);
			return true;
		} else {
			// either right is null or both are null. either way, null is not less than null, and null is not less than not null.
			//TRACE("Compare 0x%08x < 0x%08x : false via null\r\n", pDetailL, pDetailR);
			return false;
		}
	}
}

// (c.haag 2008-12-04 15:27) - PLID 31693 - This works like CompareDetailSortOrder but for LinkedDetailStruct objects
// (a.walling 2013-10-01 11:45) - PLID 58829 - Now using a functor predicate
bool CompareLinkedDetailStructSortOrder::operator()(LinkedDetailStruct& linkedDetailL, LinkedDetailStruct& linkedDetailR) const
{
	CompareDetailSortOrder compareDetails;
	return compareDetails(linkedDetailL.pDetail, linkedDetailR.pDetail);
}


// (a.walling 2013-10-01 11:45) - PLID 58829 - Now using a functor predicate
bool CompareDiagCodeOrderIndex::operator()(EMNDiagCode* pDiagL, EMNDiagCode* pDiagR) const
{
	if (pDiagL == pDiagR) {
		return false;
	}

	if (pDiagL && pDiagR) {
		return pDiagL->nOrderIndex < pDiagR->nOrderIndex;
	} else {
		if (pDiagR) {
			// left is null, right is not, so we can say null < not-null
			return true;
		} else {
			// either right is null or both are null. either way, null is not less than null, and null is not less than not null.
			return false;
		}
	}
}

// (a.walling 2007-04-25 10:54) - PLID 25454 - Simply call qsort to sort the topic array
void SortTopicArray(CArray<CEMRTopic*, CEMRTopic*> &arTopics)
{
	try {
		// (a.walling 2013-10-01 11:45) - PLID 58829 - Now using a functor predicate
		boost::sort(arTopics, CompareTopicSortOrder());
	} NxCatchAll("Error sorting topics"); // array should still be in a usable state
}

// (a.walling 2007-04-25 10:54) - PLID 25454 - Simply call qsort to sort the detail array
void SortDetailArray(CArray<CEMNDetail*, CEMNDetail*> &arDetails)
{
	try {
		// (a.walling 2013-10-01 11:45) - PLID 58829 - Now using a functor predicate
		boost::sort(arDetails, CompareDetailSortOrder());
	} NxCatchAll("Error sorting details"); // array should still be in a usable state
}

// (c.haag 2008-12-04 15:26) - PLID 31693 - Overload of SortDetailArray for LinkedDetailStruct objects
void SortDetailArray(CArray<LinkedDetailStruct,LinkedDetailStruct&> &arDetails)
{
	try {
		// (a.walling 2013-10-01 11:45) - PLID 58829 - Now using a functor predicate
		boost::sort(arDetails, CompareLinkedDetailStructSortOrder());
	} NxCatchAll("Error sorting linked details"); // array should still be in a usable state
}

// (a.walling 2007-09-26 11:46) - PLID 25548 - Calls qsort to sort the array of diag codes given
void SortDiagCodeArray(CArray<EMNDiagCode*, EMNDiagCode*> &arDiagCodes)
{
	try {
		// (a.walling 2013-10-01 11:45) - PLID 58829 - Now using a functor predicate
		boost::sort(arDiagCodes, CompareDiagCodeOrderIndex());
	} NxCatchAll("Error sorting diag codes");
}


//////////////// end sorting functions

// (j.jones 2007-05-15 16:44) - PLID 25431 - added function to check both
// Create & Write permissions on EMR, only prompting for a password once
BOOL CheckHasEMRCreateAndWritePermissions()
{
	//To create an EMR, you need to have Create AND Write permissions,
	//you can't have Create without Write, because EMR is a special beast.
	//But we need to be able to check both permissions, and if either one
	//is passworded, check for the password only once.

	// (a.walling 2007-11-28 11:28) - PLID 28044 - Fail if license expired
	// (z.manning, 05/06/2008) - PLID 29938 - Custom records use this function too so we shouldn't
	// be explicity checking for a value of 2 here.
	if (g_pLicense->HasEMR(CLicense::cflrSilent) == 0) {
		return FALSE;
	}

	//first check the create permission, warn if they fail
	if(!CheckCurrentUserPermissions(bioPatientEMR, sptCreate)) {
		//they either don't have a create permission or cancelled the password screen,
		//so return false as they can't go further
		return FALSE;
	}

	//if we get here, they have Create permission, so see if they have Write permission

	CPermissions permEMR = GetCurrentUserPermissions(bioPatientEMR);
	// Return TRUE if we have write access
	if(permEMR & sptWrite) { 
		return TRUE;
	}

	//If we don't have regular write permissions, we need to check the password.
	//But first, did we check the Create password?
	if(permEMR & sptCreateWithPass) {
		//they did have a password, which means they already entered it earlier,
		//so if we need a write password, don't bother stopping them
		if(permEMR & sptWriteWithPass) {
			//indeed, we needed a password to Write, but we don't need to prompt,
			//so successfully return silently
			return TRUE;
		}
	}
	//if they did not have a password, it means they were allowed this far
	//with no messaging, so check the write permission normally, prompting for
	//a password if needed
	else if(CheckCurrentUserPermissions(bioPatientEMR, sptWrite)) {
		//they passed the write permissions, they're good to go
		return TRUE;
	}

	//if we get here, they had Create permissions, and either did not have
	//write permissions or Create had no password, and Write did, and they
	//didn't enter the password
	return FALSE;
}

// (z.manning, 07/26/2007) - PLID 26574 - Based on the old and new info types, determines if
// the change requires we clear the state for any details for the info item.
// (z.manning 2011-10-21 15:02) - PLID 44649 - Added subtypes
BOOL InfoTypeChangeRequiresDetailStateReset(EmrInfoType eOldInfoType, EmrInfoSubType eOldInfoSubType, EmrInfoType eNewInfoType, EmrInfoSubType eNewInfoSubType, CEMNDetail *pDetail)
{
	// (z.manning 2011-10-21 15:03) - PLID 44649 - We need to check subtype here as well.
	if(IsSameEmrInfoType(eOldInfoType, eOldInfoSubType, eNewInfoType, eNewInfoSubType)) {
		return FALSE;
	}

	// (z.manning, 07/26/2007) - PLID 26574 - Valid info type changes:
	//  1. Changing from a single to multi select
	//  2. Changing from a single to multi if there's at most one selection
	switch(eNewInfoType)
	{
		case eitMultiList:
			if(eOldInfoType == eitSingleList) {
				return FALSE;
			}
			break;

		case eitSingleList:
			if(eOldInfoType == eitMultiList)
			{
				CDWordArray arydwSelections;
				pDetail->GetSelectedValues(arydwSelections);
				if(arydwSelections.GetSize() <= 1) {
					return FALSE;
				}
			}
			break;
	}

	return TRUE;
}

// (z.manning 2010-02-18 09:10) - PLID 37427 - Added nEmrDetailImageStampID
// (z.manning 2011-03-02 14:47) - PLID 42335 - Added nStampID
void AppendTableStateWithFormattedElement(CString& strState, long nRowID, long nColumnID, const CString& strFormattedData, const long nEmrDetailImageStampID, const long nEmrDetailImageStampPointer, const long nStampID)
{
	// (c.haag 2007-08-18 08:54) - PLID 27111 - This function appends a state string with a new element given
	// the table row ID, column ID, and formatted data
	// (c.haag 2007-08-18 15:01) - PLID 27118 - Only add non-empty elements
	if (!strFormattedData.IsEmpty()) {
		// (z.manning 2010-02-18 09:39) - PLID 37427 - We now include EmrDetailImageStampID and a pointer to it in the state
		// (z.manning 2011-03-02 14:48) - PLID 42335 - Include global stamp ID in the state
		strState += FormatString("%li;%li;%s;%li;%li;%li;", nRowID, nColumnID, strFormattedData, nEmrDetailImageStampID, nEmrDetailImageStampPointer, nStampID);
	}
}

// (z.manning 2010-02-18 09:10) - PLID 37427 - Added nEmrDetailImageStampID
// (z.manning 2011-03-02 14:47) - PLID 42335 - Added nStampID
inline void AppendTableStateWithUnformattedElement(CString& strState, long nRowID, long nColumnID, const CString& strUnformattedData, const long nEmrDetailImageStampID, const long nEmrDetailImageStampPointer, const long nStampID)
{
	// (c.haag 2007-08-18 08:54) - PLID 27111 - This function appends a state string with a new element given
	// the table row ID, column ID, and non-formatted data
	// (c.haag 2007-08-18 15:01) - PLID 27118 - Only add non-empty elements
	if (!strUnformattedData.IsEmpty()) {
		AppendTableStateWithFormattedElement(strState, nRowID, nColumnID, FormatForDelimitedField(strUnformattedData, ';', '\\'), nEmrDetailImageStampID, nEmrDetailImageStampPointer, nStampID);
	}
}

// (z.manning 2011-11-11 11:26) - PLID 37093
CString FormatNexFormsTextForNarrative(const CString &strNexFormsText)
{
	// (z.manning 2011-11-11 13:10) - PLID 37093 - Turns out narrative fields don't currently support rich text
	// within the actual field's text. For that reason, this feature ended up getting pushed back. So we'll need
	// to change this once we finally do release this feature.
	return ConvertTextFormat(strNexFormsText, tfRichText, tfPlainText);
}

// (j.jones 2008-01-16 09:55) - PLID 18709 - added NarrativeRequestLWMergeFieldData,
// so the OnRequestLWMergeFieldData() functions in CEmrItemAdvNarrativeDlg and
// CEMRItemAdvPopupWnd can share the same code base
// (a.walling 2009-11-18 12:09) - PLID 36365 - Pass in rich text
void NarrativeRequestLWMergeFieldData(CEMNDetail *pDetail, const CString& strRichText)
{
	//no exception handling, let it send errors to the calling function

	//return if no patient, or if this is a template (should always be the same)
	long nPatientID = pDetail->GetPatientID();
	if(nPatientID == -1 || pDetail->m_bIsTemplateDetail) {
		//no patient - do nothing
		return;
	}

	if (pDetail->m_pParentTopic == NULL || pDetail->m_pParentTopic->GetParentEMN() == NULL) {
		return;
	}

	CEMN* pEMN = pDetail->m_pParentTopic->GetParentEMN();

	//when Practice gets the RequestLWMergeFieldData message from the richedit control,
	//we can assume that the richedit control has set LWMergeDataNeeded = TRUE in the
	//narrative recordset for at least one record, possibly multiple

	//loop through the recordset, grab all the merge fields we need to load,
	//load them, populate the recordset, then set LWMergeDataNeeded = FALSE
	// (a.walling 2009-11-18 12:09) - PLID 36365 - Check the generic map
	CMap<CString, LPCTSTR, long, long&> mapFields;
	pDetail->GetNarrativeLWFields(mapFields, strRichText);

	// (a.walling 2009-11-18 12:09) - PLID 36365 - Refresh any LW fields
	long nRefreshed = pEMN->EnsureLWMergeFields(mapFields);

	/*
	ADODB::_RecordsetPtr prs = pDetail->GetNarrativeFieldRecordset();
	if(prs == NULL) {
		//should be impossible
		ASSERT(FALSE);
		return;
	}

	ADODB::FieldsPtr f = prs->Fields;

	//first pass through the recordset, get the names of all the merge fields we need filled in
	CStringSortedArrayNoCase aryFieldList;

	prs->MoveFirst();		
	while (!prs->eof) {

		//look only for merge fields that we requested be filled in
		if(AdoFldBool(f, "IsLWMergeField") && AdoFldBool(f, "LWMergeDataNeeded")) {
			
			//get the field name
			CString strField = AdoFldString(f, "Field");

			//put it into the array
			aryFieldList.Insert((LPCTSTR)strField);
		}

		prs->MoveNext();
	}

	//now pull the merge data
	if(aryFieldList.GetSize() > 0) {

		CMergeEngine mi;

		// Prepare the table with the patient id we're going to merge
		CString strMergeT;
		{
			CString strSql;				
			strSql.Format("SELECT ID FROM PersonT WHERE ID = %li", nPatientID);
			strMergeT = CreateTempIDTable(strSql, "ID");
		}

		CString strRecordSql = mi.GenerateMergeRecordsetSql(aryFieldList, strMergeT);
		if(!strRecordSql.IsEmpty()) {

			//combine the sender check and merge recordset into one database call -
			//which means the sender info will NOT be in the merge recordset,
			//and we will have to add special handling for those fields
			CString strName, strSql;
			strSql.Format("SELECT First, Middle, Last, Title, Email FROM PersonT WHERE ID = {INT}\r\n%s", strRecordSql);
			
			ADODB::_RecordsetPtr rsMergeInfo = CreateParamRecordset(strSql, GetCurrentUserID());
			if(!rsMergeInfo->eof) {
				mi.m_strSenderFirst = AdoFldString(rsMergeInfo, "First","");
				mi.m_strSenderMiddle = AdoFldString(rsMergeInfo, "Middle","");
				mi.m_strSenderLast = AdoFldString(rsMergeInfo, "Last","");
				strName = mi.m_strSenderFirst + (mi.m_strSenderMiddle.IsEmpty() ? "" : (" "+ mi.m_strSenderMiddle)) + " " + mi.m_strSenderLast;
				strName.TrimRight();
				mi.m_strSender = strName;
				mi.m_strSenderTitle = AdoFldString(rsMergeInfo, "Title","");
				mi.m_strSenderEmail = AdoFldString(rsMergeInfo, "Email","");
			}

			//now advance to the recordset with all our merge fields
			rsMergeInfo = rsMergeInfo->NextRecordset(NULL);

			if(!rsMergeInfo->eof) {

				//now run through our narrative recordset again and
				//find the fields we needed to load
				prs->MoveFirst();		
				while (!prs->eof) {

					//look only for merge fields that we requested be filled in
					if(AdoFldBool(f, "IsLWMergeField") && AdoFldBool(f, "LWMergeDataNeeded")) {
						
						//get the field name
						CString strField = AdoFldString(f, "Field");

						//now find that field in our returned recordset

						CString strValue = "";

						//but first check to see if the field is any of the sender information,
						//in which case the data is not in the recordset
						//(*** this would have been in the recordset if we populated the
						//mi.m_strSender fields prior to calling GenerateMergeRecordsetSql,
						//but that would have resulted in two separate recordsets, so to
						//conserve sql access, we have to implement this special handling)
						if(strField == "Sender") {
							strValue = mi.m_strSender;
						}
						else if(strField == "Sender_First") {
							strValue = mi.m_strSenderFirst;
						}
						else if(strField == "Sender_Middle") {
							strValue = mi.m_strSenderMiddle;
						}
						else if(strField == "Sender_Last") {
							strValue = mi.m_strSenderLast;
						}
						else if(strField == "Sender_Email") {
							strValue = mi.m_strSenderEmail;
						}
						else if(strField == "Sender_Title") {
							strValue = mi.m_strSenderTitle;
						}
						else if(!strField.IsEmpty()) {
							//if not sender information, it's a standard merge field
							//that should have been returned in the recordset
							_bstr_t strFieldName = (LPCTSTR)strField;
							_variant_t varValue = rsMergeInfo->Fields->Item[strFieldName]->Value;
							strValue = AsString(varValue);
						}

						//and update the recordset
						f->Item["Value"]->Value = (LPCTSTR)strValue;
						f->Item["SentenceForm"]->Value = (LPCTSTR)strValue;
						f->Item["Linkable"]->Value = _variant_t(VARIANT_FALSE, VT_BOOL);
						f->Item["ValueIsValid"]->Value = _variant_t(VARIANT_TRUE, VT_BOOL);
						f->Item["SentenceFormIsValid"]->Value = _variant_t(VARIANT_TRUE, VT_BOOL);
						f->Item["LWMergeDataNeeded"]->Value = _variant_t(VARIANT_FALSE, VT_BOOL);
						prs->Update();
					}

					prs->MoveNext();
				}

			}
		}
	}
	*/
}

// (a.walling 2008-01-16 15:34) - PLID 14982 - Made this global since it uses no member variables at all
void BuildCurrentTableStateArray(CEMRTopic* pTopic, long nEMRInfoID, CArray<CEMNDetail*,CEMNDetail*>& apDetails)
{
	//
	// (c.haag 2006-03-07 08:47) - PLID 19580 - Go through each topic looking for details with the same
	// EmrInfoID, and then add their state to astrState. This function is recursive since topics can have
	// subtopics.
	//
	int nDetails = pTopic->GetEMNDetailCount();
	int nChildren = pTopic->GetSubTopicCount();
	int k;
	for (k=0; k < nDetails; k++) {
		CEMNDetail* pDetail = pTopic->GetDetailByIndex(k);
		if (pDetail->m_nEMRInfoID == nEMRInfoID) {
			apDetails.Add(pDetail);
		}
	}
	for (k=0; k < nChildren; k++) {
		BuildCurrentTableStateArray(pTopic->GetSubTopic(k), nEMRInfoID, apDetails);
	}
}

//DRT 2/26/2008 - PLID 28603 - Same as BuildCurrentTableStateArray, but for images.  Traverse this topic
//	and add all details with the given nEMRInfoID to the array, then traverse all subtopics and do the same.
void BuildCurrentImageArray(CEMRTopic* pTopic, long nEMRInfoID, CArray<CEMNDetail*,CEMNDetail*>& apDetails)
{
	int nDetails = pTopic->GetEMNDetailCount();
	int nChildren = pTopic->GetSubTopicCount();
	int k;
	for (k=0; k < nDetails; k++) {
		CEMNDetail* pDetail = pTopic->GetDetailByIndex(k);
		if (pDetail->m_nEMRInfoID == nEMRInfoID) {
			apDetails.Add(pDetail);
		}
	}
	for (k=0; k < nChildren; k++) {
		BuildCurrentImageArray(pTopic->GetSubTopic(k), nEMRInfoID, apDetails);
	}
}

// (z.manning, 01/23/2008) - PLID 28690 - Clears out any existing hot spots and re-adds them to
// the ink pic control based on the array stored in pDetail.
void RefreshHotSpots(NXINKPICTURELib::_DNxInkPicturePtr pInkPicture, CEMNDetail *pDetail)
{
	//DRT 1/21/2008 - PLID 28603 - Write the hotspots to the ink control
	if(pInkPicture != NULL && pDetail != NULL) {
		pInkPicture->ClearAllHotSpots();
		for(int i = 0; i < pDetail->GetHotSpotArray()->GetSize(); i++) {
			// (z.manning 2011-07-25 12:22) - PLID 44649 - This is now a pointer
			CEMRHotSpot *pSpot(pDetail->GetHotSpotArray()->GetAt(i));

			pInkPicture->AddHotSpot(pSpot->GetID(), _bstr_t(pSpot->GetOutputPoints()));
			pInkPicture->SetHotSpotState(pSpot->GetID(), pSpot->GetSelected() ? VARIANT_TRUE : VARIANT_FALSE);
		}
	}
}

// (a.walling 2008-03-24 10:24) - PLID 28811 - Returns whether the vt is valid for the info type
BOOL DataTypeMatchesState(const EmrInfoType eit, const long vt) {
	switch (eit) {
	case eitText:
	case eitNarrative:
	case eitSingleList:
	case eitMultiList:
	case eitTable:
		return vt == VT_BSTR;
		break;

	case eitImage:
		return vt == (VT_ARRAY|VT_UI1);
		break;

	case eitSlider: 
		return (vt == VT_R8) || (vt == VT_R4); // (a.walling 2008-03-24 14:19) - PLID 28811 - Sliders are VT_R8 or VT_R4
		break;

	case eitInvalid:
	default:
		return FALSE;
	}

	return FALSE;
}

// (r.gonet 2013-04-08 17:48) - PLID 56150 - Returns the number of elements
//  in a safearray variant array. Argument should be a single dimension array containing bytes.
unsigned long GetElementCountFromSafeArrayVariant(const _variant_t &var)
{
	_variant_t varLocal(var);

	DWORD dwSize;

	if (varLocal.vt == VT_EMPTY || varLocal.vt == VT_NULL) {
		return 0;
	}
	if (!(varLocal.vt & VT_ARRAY)) {
		ASSERT(FALSE);
		return 0;
	}

	COleSafeArray sa;
	VARIANT varTempData = varLocal.Detach();
	try {
		sa.Attach(varTempData);
		long nLow = 0, nHigh = 0;
		sa.GetLBound(1, &nLow);
		sa.GetUBound(1, &nHigh);
		ASSERT(sa.GetDim() == 1);
		ASSERT(sa.GetElemSize() == 1);
		dwSize = nHigh - nLow + 1;
	} catch (...) {
		sa.Detach();
		varLocal.Attach(varTempData);
		throw;
	}
	varTempData = sa.Detach();
	varLocal.Attach(varTempData);

	return (unsigned long)dwSize;
}

// (z.manning 2008-06-06 09:14) - PLID 30155 - Returns the details value that should be used for
// any table calulations.
//TES 10/12/2012 - PLID 39000 - Added bValueHadUnusedNumbers.  This will be set to TRUE if one or more of the fields that was used for the 
// calculation was truncated, and the text that was truncated included digits.  So for example, if a value was 5'6", then it would be
// TRUE, but if the value was 66 in. it would be FALSE, because while the "in." is getting truncated, it does not include any numbers.
double GetValueForCalculation(CEMNDetail *pDetail, long nRow, short nCol, OUT BOOL &bValueWasBlank, OUT BOOL &bValueHadUnusedNumbers)
{
	// (a.walling 2011-08-24 12:35) - PLID 45171 - We only need to look for existing table elements
	TableElement te;
	if(!pDetail->GetExistingTableElement(&pDetail->GetRowPtr(nRow)->m_ID, pDetail->GetColumn(nCol).nID, te)) {
		// (z.manning 2008-06-05 16:19) - This table cell is blank
		bValueWasBlank = TRUE;
		return 0;
	}

	CString strValue;
	CStringArray arystrTemp;
	switch(te.m_pColumn->nType)
	{
		case LIST_TYPE_CHECKBOX:
		case LIST_TYPE_LINKED:
			// (z.manning 2008-06-12 10:08) - PLID 30155 - Not supported
			bValueWasBlank = TRUE;
			return 0;
			break;

		case LIST_TYPE_DROPDOWN:
			// (z.manning 2008-06-12 10:09) - PLID 30155 - If we have multiple selections here
			// then just return 0.
			if(te.m_anDropdownIDs.GetSize() > 1) {
				bValueWasBlank = FALSE;
				return 0;
			}
			// (z.manning 2008-06-12 10:10) - Intentionally no break here
		case LIST_TYPE_TEXT:
			// (a.walling 2011-05-31 11:56) - PLID 42448
			strValue = te.GetValueAsOutput(pDetail, false, arystrTemp, false);
			break;

		default:
			// (z.manning 2008-06-05 16:06) - Unknown type
			ASSERT(FALSE);
	}

	Trim(strValue);

	bValueWasBlank = strValue.IsEmpty();

	// (z.manning 2008-06-06 10:09) - PLID 30155 - Per Christina and EMR team, if a table value begins
	// with a number, then we should use that number at the beginning and just trim the string off of it.
	bValueHadUnusedNumbers = FALSE;
	char *szBadString;
	ASSERT(strValue.GetLength() < 4096);
	strtod(strValue.GetBuffer(4096), &szBadString);
	strValue.ReleaseBuffer();
	//TES 10/12/2012 - PLID 39000 - If any of the truncated characters was a numeric digit, set bValueHadUnusedNumbers
	if(CString(szBadString).FindOneOf("1234567890") != -1) {
		bValueHadUnusedNumbers = TRUE;
	}
	int nCharsFromEndToDelete = strlen(szBadString);
	if(nCharsFromEndToDelete != 0) {
		strValue.Delete(strValue.GetLength() - nCharsFromEndToDelete, nCharsFromEndToDelete);
	}

	if(strValue.IsEmpty()) {
		// (z.manning 2008-06-06 10:08) - PLID 30155 - Per Christina and the EMR team, if a cell does
		// not have a valid numeric value, we should use zero.
		return 0;
	}
	else {
		return AsDouble(_bstr_t(strValue));
	}
}

// (c.haag 2008-06-17 12:15) - PLID 17842 - This function takes an array of details, looks for
// any table details that have missing linked detail elements, and tries to fill them in. The
// motivation for this goes back to "Remember this detail (for the patient)". Table element
// states can remember details by their EmrDetailsT.ID. Unfortunately, that is useless to other
// EMN's that don't contain the same ID. So, we do the next best thing: Try to find linked details
// by the Emr Info name of that detail rather than the ID of the detail. bSearchState is TRUE if
// we want to search the state in addition to the table elements (slower for table details)
void DetectAndTryRepairMissingLinkedDetails(CArray<CEMNDetail*,CEMNDetail*>& apDetails, BOOL bSearchState)
{
	// (c.haag 2008-06-17 11:20) - PLID 17842 - Now apDetails contains a list of all details that were spawned.
	// Pass them into the utility function to search for table details that have linked detail columns
	const int nDetails = apDetails.GetSize();

	for (int i=0; i < nDetails; i++) {
		CEMNDetail* pDetail = apDetails[i];

		// Check to see if this is a non-template table with a non-blank state and a valid EMN.
		if (eitTable == pDetail->m_EMRInfoType && VT_BSTR == pDetail->GetStateVarType() &&
			NULL != pDetail->m_pParentTopic && NULL != pDetail->m_pParentTopic->GetParentEMN() &&
			!pDetail->m_bIsTemplateDetail && eistNone == pDetail->m_EMRInfoSubType) 
		{
			// Jackpot. Now, in most fix cases, the would-be-linked-detail was dropped from the state, and
			// one or more elements have m_strMissingLinkedDetailID assigned to them. However, if this detail
			// was spawned, then the "I:" flag may still be in the state, and none of the elements apparently
			// have missing linked details (even though they actually do). So, we need to check the state for
			// the "I:" flag. We do a quick string search because, while this can be fooled by putting I: within
			// a text cell, it's faster than doing a full-on state iteration (especially with big tables)
			if (bSearchState) {
				CString strState = VarString(pDetail->GetState());
				if (-1 != strState.Find("I:")) {
					// We *might* have a hit. We must call RequestStateChange on itself to synchronize
					// the state with the content.
					pDetail->RequestStateChange( pDetail->GetState() );
				}
			}
			
			// Now, look for all the elements that may correspond to a missing linked detail.
			const int nElements = pDetail->GetTableElementCount();
			BOOL bUpdateState = FALSE;
			for (int j=0; j < nElements; j++) {
				TableElement te;
				pDetail->GetTableElementByIndex(j,te);

				// See if this element has a missing linked detail				
				if (!te.m_strMissingLinkedDetailID.IsEmpty() &&	-1 != te.m_strMissingLinkedDetailID.Find("\r")) {
					CString strID = te.m_strMissingLinkedDetailID.Left(te.m_strMissingLinkedDetailID.Find("\r"));
					if(strID.GetLength() >= 2) {
						long nLinkedDetailID = atol(strID.Mid(2));
						// If we get here, the linked detail ID may be missing. What we need to do is get
						// the name of the EMR item corresponding to that detail, find the detail on the EMN,
						// and if it exists, update the table element (which in turn updates the state internally)

						// Get the name of the Emr Info item
						_RecordsetPtr prsName = CreateParamRecordset(
							"SELECT Name FROM EmrInfoT "
							"INNER JOIN EmrDetailsT ON EmrDetailsT.EmrInfoID = EmrInfoT.ID "
							"WHERE EmrDetailsT.ID = {INT}", nLinkedDetailID);
						CString strName;
						if (!prsName->eof) {
							// We found the name. Now reset the missing linked detail ID and reload
							// the table element with the EMR info name. It will search for a linked
							// detail, and update element data accordingly if it can find it.
							const CString& strName = AdoFldString(prsName, "Name");
							te.m_strMissingLinkedDetailID.Empty(); // Empty the missing linked detail string
																	// (I don't know why LoadValueFromString doesn't do this)
							te.LoadValueFromString(strName, pDetail->m_pParentTopic->GetParentEMN());
							pDetail->SetTableElement(te);
							
							// Flag the detail for update
							bUpdateState = TRUE;

						} else {
							// Name wasn't found. Just leave the cell alone since we don't
							// have a way to fill it in.
						}
					} // if (nLinkedDetailID > -1) {
				}
				// (c.haag 2008-08-15 17:49) - PLID 17842 - Check for an extreme case: The linked detail
				// may be stored as a name without a prefix code. In this case, you would be able to find
				// the name in the state, but not in the element value. You would, though, find it in
				// m_strMissingLinkedDetailID and m_strLinkedSentenceHtml. Refer to my notes in 
				// TableElement::LoadValueFromString where the ASSERT used to be. It applies not only to
				// deleted details, but details that are trying to be remembered like they are here.
				else if (te.m_pColumn != NULL && te.m_pColumn->nType == LIST_TYPE_LINKED &&
					te.m_pLinkedDetail == NULL && 
					te.m_strValue.IsEmpty() &&
					te.m_strMissingLinkedDetailID.IsEmpty() && 
					!te.m_strLinkedSentenceHtml.IsEmpty() && te.m_strLinkedSentenceNonHtml == te.m_strLinkedSentenceHtml)
				{
					// If we get here, it *might* be a candidate. We need to actually seek to the element in the state
					// and compare the element data with te.m_strLinkedSentenceHtml. If they match, then we know the table
					// did try to link itself to a detail for this element, and failed at the time.
					CString strState = VarString(pDetail->GetState());
					CEmrTableStateIterator etsi(strState);
					long nRow, nColumn, nEmrDetailImageStampID, nEmrDetailImageStampPointer, nStampID;
					CString strData;
					// (z.manning 2010-02-18 09:30) - PLID 37427 - Added EmrDetailImageStampID
					while (etsi.ReadNextElement(nRow, nColumn, strData, nEmrDetailImageStampID, nEmrDetailImageStampPointer, nStampID))
					{
						//TES 3/17/2010 - PLID 37530 - Pass in -1 for SourceStampID and SourceStampIndex
						TableRowID rowid(nRow,nEmrDetailImageStampID,nEmrDetailImageStampPointer,-1,-1);
						if (nColumn == te.m_pColumn->nID && rowid == te.m_pRow->m_ID)
						{
							if (strData == te.m_strLinkedSentenceHtml) {
								te.LoadValueFromString(strData, pDetail->m_pParentTopic->GetParentEMN());
								pDetail->SetTableElement(te);
								// Flag the detail for update
								bUpdateState = TRUE;

							}
							break;
						}
					}
				}

			} // for (int j=0; j < nElements; j++) {

			// If the following is true, it means we changed content, and now we need to update the state
			// accordingly.
			if (bUpdateState) {
				pDetail->RecreateStateFromContent();
				// RecreateStateFromContent doesn't actually update the visible dialog; we do that by
				// calling this function.
				pDetail->RequestStateChange( pDetail->GetState() );
			}

		} // if (eitTable == pDetail->m_EMRInfoType ...
	} // for (i=0; i < nDetails; i++) {

}

// (c.haag 2008-07-02 15:44) - PLID 30221 - Fills an Emr Action object with values from EMRActionsTodoAssignToT
void FillEmrActionWithTodoAssignTos(_ConnectionPtr& pCon, IN OUT EmrAction& ea)
{
	// (c.haag 2008-07-02 15:36) - We used to use prs->GetRecordCount(), but sometimes it returns -1 if
	// either it can't be calculated, or if the cursor is not adUseClient. So, just fill a local array
	// and copy it to the allocated array. I didn't catch this a month ago because GetRecordCount worked
	// for everyone every time, and it just failed today for Yazi for no apparent reason.

	// Read in the values
	_RecordsetPtr prs = CreateParamRecordset(pCon, "SELECT AssignTo FROM EMRActionsTodoAssignToT WHERE ActionID = {INT}",
		ea.nID);

	// (a.walling 2014-07-01 15:28) - PLID 62697	
	while (!prs->eof) {
		ea.anTodoAssignTo.push_back(AdoFldLong(prs, "AssignTo"));
		prs->MoveNext();
	}
	prs->Close();

	ASSERT(!ea.anTodoAssignTo.empty()); // This means bad data -- no assignees exist for this alarm
}

// (c.haag 2008-07-17 15:17) - PLID 30550 - This function returns a query that must be
// run upon locking an EMN to properly lock todo alarms
// (a.walling 2014-01-31 15:41) - PLID 60551 - NxAutoQuantum - This now returns a self-contained call to sp_executesql
CString GetEMRLockTodoAlarmSql(long nEMNID)
{
	// (z.manning 2016-01-13 11:34) - PLID 67778 - Refactored this code to not use a cursor though because
	// of the somewhat odd data structure with locked to-dos, I had to used identity insert which I deemed
	// the lesser of two evils.
	CString strInner = FormatString(R"(
DECLARE @lockedTask TABLE (
	TaskID INT,
	EMRDetailID INT,
	RegardingType INT,
	DetailName NVARCHAR(255),
	Assignees NVARCHAR(MAX),
	Deadline DATETIME,
	Done DATETIME,
	Notes NVARCHAR(2000),
	AssigneeXml XML,
	RowNum INT
	)
INSERT INTO @lockedTask (TaskID, EMRDetailID, RegardingType, DetailName, Assignees, Deadline, Done, Notes)
SELECT TodoList.TaskID, RegardingID, RegardingType, 
	CASE WHEN EmrInfoT.ID = %d THEN EMRDetailsT.MacroName ELSE EmrInfoT.Name END AS Name, 
	dbo.GetTodoAssignToNamesString(TaskID) AS AssignToNames, 
	Deadline, Done, 
	/* This big crazy conditional is TRUE if the note appears to be properly formatted in 
	the form EMN: ... Detail:... If it is, we can just strip that part out of the note because 
	the information it gives us is redundant */ 
	CASE WHEN CHARINDEX(char(10), Notes) > 0 
		AND CHARINDEX(char(10), Notes, CHARINDEX(char(10), Notes) + 1) > 0 
		AND Len(Notes) > 3 
		AND Left(Notes, 4) = 'EMN:' 
		AND SUBSTRING(Notes, CHARINDEX(char(10), Notes) + 1, 7) = 'Detail:' 
	THEN 
		Right(Notes, Len(Notes) - CHARINDEX(char(10), Notes, CHARINDEX(char(10), Notes) + 1)) 
	ELSE 
		Notes 
	END
FROM TodoList 
INNER JOIN EMRDetailsT ON EMRDetailsT.ID = TodoList.RegardingID 
INNER JOIN EmrInfoT ON EmrInfoT.ID = EMRDetailsT.EmrInfoID 
WHERE EMRDetailsT.EMRID = @nEMNID AND EMRDetailsT.Deleted = 0 
	AND TodoList.RegardingType = %d
UNION ALL
SELECT TodoList.TaskID, NULL, RegardingType, 
	Description AS Name, 
	dbo.GetTodoAssignToNamesString(TaskID) AS AssignToNames, 
	Deadline, Done, 
	/* This conditional is TRUE if the note appears to be properly formatted in 
	the form EMN: ... If it is, we can just strip that part out of the note because 
	the information it gives us is redundant */ 
	CASE WHEN CHARINDEX(char(10), Notes) > 0 
		AND Len(Notes) > 3 
		AND Left(Notes, 4) = 'EMN:' 
	THEN 
		Right(Notes, Len(Notes) - CHARINDEX(char(10), Notes)) 
	ELSE 
		Notes 
	END
FROM TodoList 
INNER JOIN EMRMasterT ON EMRMasterT.ID = TodoList.RegardingID 
WHERE EMRMasterT.ID = @nEMNID AND EMRMasterT.Deleted = 0 
	AND TodoList.RegardingType = %d

;WITH RowNums AS (
	SELECT T.TaskID, ROW_NUMBER() OVER(ORDER BY T.TaskID) AS Num
	FROM @lockedTask T
	)
UPDATE @lockedTask SET RowNum = RN.Num
	, AssigneeXml = (SELECT TA.AssignTo AS [@UserID] FROM TodoAssignToT TA WHERE TA.TaskID = T.TaskID FOR XML Path('Assignee'))
FROM @lockedTask T
INNER JOIN RowNums RN ON T.TaskID = RN.TaskID

DECLARE @maxLockedTaskID INT
SET @maxLockedTaskID = (SELECT ISNULL(MAX(LT.ID), 0) FROM EMRLockedTodosT LT WITH(UPDLOCK, HOLDLOCK))

SET IDENTITY_INSERT EMRLockedTodosT ON
INSERT INTO EMRLockedTodosT (ID, EMRMasterID, EMRDetailID, RegardingType, DetailName, Assignees, Deadline, Done, Notes)
SELECT @maxLockedTaskID + T.RowNum, @nEMNID, T.EMRDetailID, T.RegardingType, T.DetailName, T.Assignees, T.Deadline, T.Done, T.Notes
FROM @lockedTask T
SET IDENTITY_INSERT EMRLockedTodosT OFF

INSERT INTO EMRLockedTodoAssignToT (EMRLockedTodoID, AssignTo)
SELECT @maxLockedTaskID + T.RowNum, A.value('@UserID', 'INT')
FROM @lockedTask T
CROSS APPLY AssigneeXML.nodes('/Assignee') X(A)
)"
, EMR_BUILT_IN_INFO__TEXT_MACRO, ttEMNDetail, ttEMN);

	return FormatString("EXEC sp_executesql N'%s', N'@nEMNID INT', %li;", _Q(strInner), nEMNID);
}

// (c.haag 2008-07-17 17:46) - PLID 30775 - Compares the data content of two EMR actions
// and returns TRUE if they are the same, or FALSE if they differ. This function calls all
// the "drill-down" functions listed below.
BOOL DoesEmrActionContentMatch(const EmrAction& a, const EmrAction& b)
{
	if (!DoesEmrActionBasicContentMatch(a,b)) { return FALSE; }
	if (!DoesEmrActionChargeContentMatch(a,b)) { return FALSE; }
	if (!DoesEmrActionProblemContentMatch(a,b)) { return FALSE; }
	if (!DoesEmrActionTodoContentMatch(a,b)) { return FALSE; }
	if (!DoesEmrActionDiagContentMatch(a, b)) { return FALSE; }
	
	return TRUE;
}

// (c.haag 2008-07-17 17:16) - PLID 30775 - Compares the problem-related content of two
// EMR actions and returns TRUE if they are the same, or FALSE if they differ
BOOL DoesEmrActionProblemContentMatch(const EmrAction& a, const EmrAction& b)
{

	// (a.walling 2014-07-01 15:28) - PLID 62697	// Compare first by size
	if (a.aProblemActions.size() != b.aProblemActions.size()) {
		return FALSE;
	}

	// Now compare by content
	CProblemActionAry aryB = b.aProblemActions;
	for (const auto& probA : a.aProblemActions) {
		BOOL bFound = FALSE;
		for (size_t j=0; j < aryB.size(); j++) {
			const EmrProblemAction& probB = aryB[j];
			if (DoesEmrActionProblemContentMatch(probA, probB)) {
				bFound = TRUE; 
				aryB.erase(aryB.begin() + j);
				break;
			}
		}
		if (!bFound) {
			return FALSE;
		}
	}

	// Now if these two arrays matched perfectly, then aryB should be empty
	if (!aryB.empty()) {
		return FALSE;
	} else {
		return TRUE;
	}
}

// (c.haag 2008-07-18 09:21) - PLID 30775 - Compares two emr problem actions, and
// returns TRUE if they match, or FALSE if they don't.
BOOL DoesEmrActionProblemContentMatch(const EmrProblemAction& a, const EmrProblemAction& b)
{
	// (c.haag 2014-07-22) - PLID 62789 - Just use the helper function
	return a.DoesContentMatch(b);
}

// (c.haag 2008-07-17 17:16) - PLID 30775 - Compares the todo-related content of two
// EMR actions and returns TRUE if they are the same, or FALSE if they differ
extern int CompareTodoAssignToAryElements(const void *pa, const void *pb);
BOOL DoesEmrActionTodoContentMatch(const EmrAction& a, const EmrAction& b)
{
	if (a.nTodoCategoryID != b.nTodoCategoryID ||
		a.strTodoMethod != b.strTodoMethod ||
		a.strTodoNotes != b.strTodoNotes ||
		!DoEmrActionTodoAssigneesMatch(a,b) ||
		a.nTodoPriority != b.nTodoPriority ||
		a.nTodoRemindType != b.nTodoRemindType ||
		a.nTodoRemindInterval != b.nTodoRemindInterval ||
		a.nTodoDeadlineType != b.nTodoDeadlineType ||
		a.nTodoDeadlineInterval != b.nTodoDeadlineInterval)
	{
		return FALSE;
	}
	else {
		return TRUE;
	}
}

// (c.haag 2008-07-18 09:01) - PLID 30775 - Compares todo assign to values between
// two EMR actions and returns TRUE if they are the same, or FALSE if they differ
BOOL DoEmrActionTodoAssigneesMatch(const EmrAction& a, const EmrAction& b)
{
	// (a.walling 2014-07-01 15:28) - PLID 62697
	if (a.anTodoAssignTo.size() != b.anTodoAssignTo.size()) {
		return FALSE;
	}

	std::vector<long> aCopy = a.anTodoAssignTo;
	std::vector<long> bCopy = b.anTodoAssignTo;

	boost::sort(aCopy);
	boost::sort(bCopy);

	if (aCopy == bCopy) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

//(s.dhole 7/18/2014 1:38 PM ) - PLID 62724
// Diagnosis code actions and returns TRUE if they are the same, or FALSE if they differ
BOOL DoesEmrActionDiagContentMatch(const EmrAction& a, const EmrAction& b)
{
	if (a.diaDiagnosis.nDiagCodeID_ICD10 != b.diaDiagnosis.nDiagCodeID_ICD10  ||
		a.diaDiagnosis.nDiagCodeID_ICD9 != b.diaDiagnosis.nDiagCodeID_ICD9)
	{
		return FALSE;
	}
	else {
		return TRUE;
	}
}

// (c.haag 2008-07-17 17:54) - PLID 30775 - Compares the charge-related content of two
// EMR actions and returns TRUE if they are the same, or FALSE if they differ
BOOL DoesEmrActionChargeContentMatch(const EmrAction& a, const EmrAction& b)
{
	if (a.bPrompt != b.bPrompt ||
		(LooseCompareDouble(a.dblDefaultQuantity, b.dblDefaultQuantity, 0.001) != 0) ||
		a.strMod1 != b.strMod1 ||
		a.strMod2 != b.strMod2 ||
		a.strMod3 != b.strMod3 ||
		a.strMod4 != b.strMod4)
	{
		return FALSE;
	}
	else {
		return TRUE;
	}
}

// (c.haag 2008-07-17 17:54) - PLID 30775 - Compares the basic content of two
// EMR actions and returns TRUE if they are the same, or FALSE if they differ.
// Exceptions include: nID, nSourceID, nDestID, bDeleted
BOOL DoesEmrActionBasicContentMatch(const EmrAction& a, const EmrAction& b)
{
	// We don't compare strSourceName because none of the calling functions ever compared by it
	// (z.manning 2011-06-29 09:47) - PLID 44347 - Also compare the anatomic location ID arrays.
	if (a.eaoSourceType != b.eaoSourceType ||
		a.eaoDestType != b.eaoDestType || 
		a.bPopup != b.bPopup ||
		a.nSortOrder != b.nSortOrder ||
		a.bSpawnAsChild != b.bSpawnAsChild || 
		a.bOnePerEmn != b.bOnePerEmn ||
		a.filter.anatomicLocationFilters.size() != b.filter.anatomicLocationFilters.size()
		)
	{
		return FALSE;
	}

	// (a.walling 2014-07-01 15:28) - PLID 62697
	// (a.walling 2014-08-04 09:39) - PLID 62684 - compare manually to avoid std::equal warning

	auto itA = a.filter.anatomicLocationFilters.begin();
	auto itB = b.filter.anatomicLocationFilters.begin();

	while (itA != a.filter.anatomicLocationFilters.end()) {
		if (!(*itA == *itB)) {
		return FALSE;
	}
		++itA;
		++itB;
	}
	return TRUE;
}

// (c.haag 2008-07-18 09:08) - PLID 30724 - Given an array of Emr problem actions,
// determine which ones were created, which were deleted and which were modifed.
void GetEmrActionProblemDiffs(const EmrAction& eaOld, const EmrAction& eaNew,
							  OUT CProblemActionAry& aCreated, OUT CProblemActionAry& aDeleted,
							  OUT CProblemActionAry& aModified)
{
	// First, search for deleted and modified actions
	// (a.walling 2014-07-01 15:28) - PLID 62697
	for (const EmrProblemAction& oldProblem : eaOld.aProblemActions) {
		BOOL bFound = FALSE;
		ASSERT(oldProblem.nID > 0);
		for (const EmrProblemAction& newProblem : eaNew.aProblemActions) {
			if (newProblem.nID == oldProblem.nID) {
				bFound = TRUE;
				// Detect changed content
				if (!DoesEmrActionProblemContentMatch(oldProblem, newProblem)) {
					// Yes it changed...this one goes in the modified column
					aModified.push_back(newProblem);
				}
				break;
			}
		}

		if (!bFound) {
			// If we get here, the problem is in the old action, but not the new action.
			// So, it was clearly deleted
			aDeleted.push_back(oldProblem);
		}
	}

	// Now, search for new actions
	for (const EmrProblemAction& newProblem : eaNew.aProblemActions) {
		if (newProblem.nID == -1) {
			// Must be new
			aCreated.push_back(newProblem);
		}
	}
}

// (c.haag 2008-07-18 15:18) - PLID 30784 - This function returns the SQL text to use in a
// massive query for loading one or more EMNs
// (c.haag 2008-07-25 15:25) - PLID 30799 - Added EmrProblemActionID
// (a.walling 2009-05-04 10:52) - PLID 33751 - Added ChronicityID
// (a.walling 2009-05-04 10:51) - PLID 28495 - Added DiagCodeID
// (c.haag 2009-05-12 12:38) - PLID 34234 - Formatted all statements to use new EMR problem link structure
// (c.haag 2009-05-19 14:20) - PLID 34277 - Added EmrProblemLinkID
// (c.haag 2009-05-19 15:49) - PLID 34277 - Moved EMRDataT from old Emr Problems table to EMR problem link structure
// (z.manning 2009-05-27 11:12) - PLID 34297 - Added patient ID
// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
// (j.jones 2012-07-31 15:17) - PLID 51750 - added sqlEMNSortOrder
// (j.jones 2013-07-02 08:56) - PLID 57271 - removed sqlEMNSortOrder, it always sorts by EMR ID now
// (b.spivey, October 22, 2013) - PLID 58677 - Set codeID
// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs
// (s.tullis 2015-02-23 15:44) - PLID 64723 
// (r.gonet 2015-03-09 18:21) - PLID 65008 - Added DoNotShowOnProblemPrompt.
CSqlFragment GetEMNProblemSql(CSqlFragment sqlLoadIDs)
{
	// Usage of strLoadIDs is /* ID IN (strLoadIDs) */
	return 
		/* EMR Details */
		CSqlFragment(
		"SELECT ProblemSubQ.* FROM ("
		"  SELECT EMRProblemsT.ID, EMRProblemLinkT.ID AS EmrProblemLinkID, (SELECT EMRID FROM EmrDetailsT WHERE ID = EmrRegardingID) AS EMRID, Description, EnteredDate, ModifiedDate, StatusID, OnsetDate, EmrRegardingType, EmrRegardingID, EMRDataID, EmrProblemActionID, DiagCodeID, DiagCodeID_ICD10, ChronicityID, EmrProblemsT.PatientID, EMRProblemsT.CodeID,EmrProblemsT.DoNotShowOnCCDA, EmrProblemsT.DoNotShowOnProblemPrompt "
		"  FROM EMRProblemsT INNER JOIN EMRProblemLinkT ON EMRProblemLinkT.EMRProblemID = EMRProblemsT.ID WHERE Deleted = 0 AND EmrRegardingType = {CONST} AND EmrRegardingID IN  "
		"  (SELECT ID FROM EmrDetailsT WHERE Deleted = 0 AND EMRID IN (SELECT ID FROM EmrMasterT WHERE Deleted = 0 AND ID IN ({SQL}))) "
		, eprtEmrItem, sqlLoadIDs)
		/* EMR Detail Elements */
		+ CSqlFragment(
		"  UNION SELECT EMRProblemsT.ID, EMRProblemLinkT.ID AS EmrProblemLinkID, (SELECT EMRID FROM EmrDetailsT WHERE ID = EmrRegardingID) AS EMRID, Description, EnteredDate, ModifiedDate, StatusID, OnsetDate, EmrRegardingType, EmrRegardingID, EMRDataID, EmrProblemActionID, DiagCodeID, DiagCodeID_ICD10, ChronicityID, EmrProblemsT.PatientID, EMRProblemsT.CodeID, EmrProblemsT.DoNotShowOnCCDA, EmrProblemsT.DoNotShowOnProblemPrompt "
		"  FROM EMRProblemsT INNER JOIN EMRProblemLinkT ON EMRProblemLinkT.EMRProblemID = EMRProblemsT.ID WHERE Deleted = 0 AND EmrRegardingType = {CONST} AND EmrRegardingID IN  "
		"  (SELECT ID FROM EmrDetailsT WHERE Deleted = 0 AND EMRID IN (SELECT ID FROM EmrMasterT WHERE Deleted = 0 AND ID IN ({SQL}))) "
		, eprtEmrDataItem, sqlLoadIDs)
		/* EMR Topics */
		+ CSqlFragment(
		"  UNION SELECT EMRProblemsT.ID, EMRProblemLinkT.ID AS EmrProblemLinkID, (SELECT EMRID FROM EmrTopicsT WHERE ID = EmrRegardingID) AS EMRID, Description, EnteredDate, ModifiedDate, StatusID, OnsetDate, EmrRegardingType, EmrRegardingID, EMRDataID, EmrProblemActionID, DiagCodeID, DiagCodeID_ICD10, ChronicityID, EmrProblemsT.PatientID, EMRProblemsT.CodeID,EmrProblemsT.DoNotShowOnCCDA, EmrProblemsT.DoNotShowOnProblemPrompt "
		"  FROM EMRProblemsT INNER JOIN EMRProblemLinkT ON EMRProblemLinkT.EMRProblemID = EMRProblemsT.ID WHERE Deleted = 0 AND EmrRegardingType = {CONST} AND EmrRegardingID IN  "
		"  (SELECT ID FROM EmrTopicsT WHERE Deleted = 0 AND EMRID IN (SELECT ID FROM EmrMasterT WHERE Deleted = 0 AND ID IN ({SQL}))) "
		, eprtEmrTopic, sqlLoadIDs)
		/* EMNs */
		+ CSqlFragment(
		"  UNION SELECT EMRProblemsT.ID, EMRProblemLinkT.ID AS EmrProblemLinkID, EmrRegardingID AS EMRID, Description, EnteredDate, ModifiedDate, StatusID, OnsetDate, EmrRegardingType, EmrRegardingID, EMRDataID, EmrProblemActionID, DiagCodeID, DiagCodeID_ICD10, ChronicityID, EmrProblemsT.PatientID, EMRProblemsT.CodeID,EmrProblemsT.DoNotShowOnCCDA, EmrProblemsT.DoNotShowOnProblemPrompt "
		"  FROM EMRProblemsT INNER JOIN EMRProblemLinkT ON EMRProblemLinkT.EMRProblemID = EMRProblemsT.ID WHERE Deleted = 0 AND EmrRegardingType = {CONST} AND EmrRegardingID IN (SELECT ID FROM EmrMasterT WHERE Deleted = 0 AND ID IN ({SQL})) "
		, eprtEmrEMN, sqlLoadIDs)
		/* EMRs. The caller demands that we load EMR problem information from this query. So, we have to do left join on EmrMasterT to replicate the data
		for individual EMN's. The caller already anticipated this (prior to this change), and problems will not be duplicated despite the data. */

		+ CSqlFragment(
		"  UNION SELECT EMRProblemsT.ID, EMRProblemLinkT.ID AS EmrProblemLinkID, EmrMasterT.ID AS EMRID, EMRProblemsT.Description, EMRProblemsT.EnteredDate, EMRProblemsT.ModifiedDate, EMRProblemsT.StatusID, EMRProblemsT.OnsetDate, EMRProblemLinkT.EmrRegardingType, EMRProblemLinkT.EmrRegardingID, EMRProblemLinkT.EMRDataID, EmrProblemActionID, EMRProblemsT.DiagCodeID, EMRProblemsT.DiagCodeID_ICD10, EMRProblemsT.ChronicityID, EmrProblemsT.PatientID, EMRProblemsT.CodeID,EmrProblemsT.DoNotShowOnCCDA, EmrProblemsT.DoNotShowOnProblemPrompt "
		"  FROM EMRProblemsT "
		"  INNER JOIN EMRProblemLinkT ON EMRProblemLinkT.EMRProblemID = EMRProblemsT.ID "

		"  LEFT JOIN (SELECT * FROM EMRGroupsT WHERE Deleted = 0) AS EMRGroupsT ON EmrGroupsT.ID = EmrRegardingID "
		"  LEFT JOIN (SELECT * FROM EmrMasterT WHERE Deleted = 0) AS EMRMasterT ON EmrMasterT.EmrGroupID = EmrGroupsT.ID "
		"  WHERE EMRProblemsT.Deleted = 0 AND EmrRegardingType = {CONST} AND EmrRegardingID IN "
		"  (SELECT EmrGroupsT.ID FROM EmrGroupsT INNER JOIN EmrMasterT ON EmrMasterT.EmrGroupID = EmrGroupsT.ID WHERE EmrMasterT.Deleted = 0 AND EmrMasterT.ID IN ({SQL})) "
		, eprtEmrEMR, sqlLoadIDs)
		/* Diagnosis Codes */
		+ CSqlFragment(
		"  UNION SELECT EMRProblemsT.ID, EMRProblemLinkT.ID AS EmrProblemLinkID, (SELECT EMRID FROM EMRDiagCodesT WHERE ID = EmrRegardingID) AS EMRID, Description, EnteredDate, ModifiedDate, StatusID, OnsetDate, EmrRegardingType, EmrRegardingID, EMRDataID, EmrProblemActionID, DiagCodeID, DiagCodeID_ICD10, ChronicityID, EmrProblemsT.PatientID, EMRProblemsT.CodeID,EmrProblemsT.DoNotShowOnCCDA, EmrProblemsT.DoNotShowOnProblemPrompt "
		"  FROM EMRProblemsT INNER JOIN EMRProblemLinkT ON EMRProblemLinkT.EMRProblemID = EMRProblemsT.ID WHERE Deleted = 0 AND EmrRegardingType = {CONST} AND EmrRegardingID IN "
		"  (SELECT ID FROM EMRDiagCodesT WHERE Deleted = 0 AND EMRID IN (SELECT ID FROM EmrMasterT WHERE Deleted = 0 AND ID IN ({SQL}))) "
		, eprtEmrDiag, sqlLoadIDs)
		/* Charges */
		+ CSqlFragment(
		"  UNION SELECT EMRProblemsT.ID, EMRProblemLinkT.ID AS EmrProblemLinkID, (SELECT EMRID FROM EMRChargesT WHERE ID = EmrRegardingID) AS EMRID, Description, EnteredDate, ModifiedDate, StatusID, OnsetDate, EmrRegardingType, EmrRegardingID, EMRDataID, EmrProblemActionID, DiagCodeID, DiagCodeID_ICD10, ChronicityID, EmrProblemsT.PatientID, EMRProblemsT.CodeID,EmrProblemsT.DoNotShowOnCCDA, EmrProblemsT.DoNotShowOnProblemPrompt "
		"  FROM EMRProblemsT INNER JOIN EMRProblemLinkT ON EMRProblemLinkT.EMRProblemID = EMRProblemsT.ID WHERE Deleted = 0 AND EmrRegardingType = {CONST} AND EmrRegardingID IN "
		"  (SELECT ID FROM EMRChargesT WHERE Deleted = 0 AND EMRID IN (SELECT ID FROM EmrMasterT WHERE Deleted = 0 AND ID IN ({SQL}))) "
		, eprtEmrCharge, sqlLoadIDs)
		/* Medications */
		+ CSqlFragment(
		"  UNION SELECT EMRProblemsT.ID, EMRProblemLinkT.ID AS EmrProblemLinkID, (SELECT EMRID FROM EMRMedicationsT WHERE MedicationID = EmrRegardingID) AS EMRID, Description, EnteredDate, ModifiedDate, StatusID, OnsetDate, EmrRegardingType, EmrRegardingID, EMRDataID, EmrProblemActionID, DiagCodeID, DiagCodeID_ICD10, ChronicityID, EmrProblemsT.PatientID, EMRProblemsT.CodeID,EmrProblemsT.DoNotShowOnCCDA, EmrProblemsT.DoNotShowOnProblemPrompt "
		"  FROM EMRProblemsT INNER JOIN EMRProblemLinkT ON EMRProblemLinkT.EMRProblemID = EMRProblemsT.ID WHERE Deleted = 0 AND EmrRegardingType = {CONST} AND EmrRegardingID IN "
		"  (SELECT MedicationID FROM EMRMedicationsT WHERE Deleted = 0 AND EMRID IN (SELECT ID FROM EmrMasterT WHERE Deleted = 0 AND ID IN ({SQL}))) "
		, eprtEmrMedication, sqlLoadIDs)
		+ ") ProblemSubQ "
		"ORDER BY ProblemSubQ.EMRID\r\n";
}

// (j.jones 2014-02-24 16:49) - PLID 61010 - a number of places need the diagnosis code number,
// and rather than check one recordset per code, this function will get both
void GetICD9And10Codes(long nICD9CodeID, long nICD10CodeID, OUT CString &strICD9Code, OUT CString &strICD10Code)
{
	strICD9Code = "";
	strICD10Code = "";
	if (nICD9CodeID != -1 || nICD10CodeID != -1) {
		_RecordsetPtr rs = CreateParamRecordset("SELECT DiagCodes9.CodeNumber AS DiagICD9Code, DiagCodes10.CodeNumber AS DiagICD10Code "
			"FROM (SELECT {INT} AS DiagCode9ID, {INT} AS DiagCode10ID) AS BaseQ "
			"LEFT JOIN DiagCodes DiagCodes9 ON BaseQ.DiagCode9ID = DiagCodes9.ID "
			"LEFT JOIN DiagCodes DiagCodes10 ON BaseQ.DiagCode10ID = DiagCodes10.ID", nICD9CodeID, nICD10CodeID);
		if(!rs->eof) {
			strICD9Code = VarString(rs->Fields->Item["DiagICD9Code"]->Value, "");
			strICD10Code = VarString(rs->Fields->Item["DiagICD10Code"]->Value, "");
		}
		rs->Close();
	}
}

// (j.jones 2014-02-24 16:49) - PLID 61010 - unifies auditing for ICD-9/10 codes on EMR problems
void GetProblemICD910AuditDescriptions(const CString &strOldICD9Code, const CString &strOldICD10Code, const CString &strNewICD9Code, const CString &strNewICD10Code,
									   OUT CString &strOldDiagCodeAudit, OUT CString &strNewDiagCodeAudit)
{
	strOldDiagCodeAudit = "";
	strNewDiagCodeAudit = "";

	if(strOldICD9Code.IsEmpty() && strOldICD10Code.IsEmpty()) {
		strOldDiagCodeAudit = "<No Diagnosis Selected>";
	}
	if(!strOldICD9Code.IsEmpty()) {
		strOldDiagCodeAudit += ("ICD-9: " + strOldICD9Code);
	}
	if(!strOldICD10Code.IsEmpty()) {
		if(!strOldDiagCodeAudit.IsEmpty()) {
			strOldDiagCodeAudit += ", ";
		}
		strOldDiagCodeAudit += ("ICD-10: " + strOldICD10Code);
	}
	if(strNewICD9Code.IsEmpty() && strNewICD10Code.IsEmpty()) {
		strNewDiagCodeAudit = "<No Diagnosis Selected>";
	}
	if(!strNewICD9Code.IsEmpty()) {
		strNewDiagCodeAudit += ("ICD-9: " + strNewICD9Code);
	}
	if(!strNewICD10Code.IsEmpty()) {
		if(!strNewDiagCodeAudit.IsEmpty()) {
			strNewDiagCodeAudit += ", ";
		}
		strNewDiagCodeAudit += ("ICD-10: " + strNewICD10Code);
	}
}

// (j.jones 2008-07-22 14:23) - PLID 30792 - made saving (and deleting) code modular
// (c.haag 2008-07-25 15:23) - PLID 30799 - Added fields required for for unspawning
// (z.manning 2009-05-21 15:02) - PLID 34297 - Removed the object ID paremeter
// (c.haag 2009-05-28 13:08) - PLID 34277 - We now have a notification window parameter
// (a.walling 2014-01-30 00:00) - PLID 60546 - Quantize
// (z.manning 2016-04-12 13:59) - NX-100140 - Added problem IDs to ignore param
void SaveProblem(Nx::Quantum::Batch &strSqlBatch, long &nAuditTransactionID, CEmrProblem *pProblem, IN CMapPtrToPtr& mapSavedObjects
	, long nPatientID, CString strPatientName, CWnd* pWndNotify, std::set<long> setProblemIDsToIgnore)
{
	//this function throws any exceptions to the caller

	if(pProblem == NULL) {
		ThrowNxException("SaveProblem called with an invalid pProblem pointer!");
	}

	//TES 10/27/2008 - PLID 31269 - If this is a new spawned problem, check whether it's a duplicate of an existing
	// problem for this patient.
	if(pProblem->m_nID < 0 && pProblem->m_nEmrProblemActionID != -1 && !pProblem->m_bIsDeleted) {
		//TES 10/30/2008 - PLID 31269 - We need to determine whether there is an existing, non-deleted problem for this
		// patient that was spawned by the "same" action.  Since the actions still unfortunately don't have group IDs,
		// we have to compare the source and destination, the destination is simple because the ids don't have to change,
		// but for the source, we have to check not just whether the ids are the same, but whether they're part of the
		// same group.  We order by entered date to show the most recent one.
		// (z.manning 2009-03-20 09:18) - PLID 15971 - Added support for table dropdown item actions
		
		// (a.walling 2009-05-04 09:49) - PLID 28495 - Diag code - Not important enough to put in this warning, I believe
		// (a.walling 2009-05-04 09:50) - PLID 33751 - Chronicity - Not important enough to put in this warning, I believe
		// (c.haag 2009-05-28 09:46) - PLID 34277 - We now get the patient ID from the problem itself, not the problem's EMR
		// (z.manning 2010-03-02 16:05) - PLID 37571 - Handle smart stamp actions
		//TES 2/17/2015 - PLID 62544 - This query was never working, due to incorrect parameter ordering when implementing 29039
		// (r.gonet 2015-11-17) - PLID 67604 - Split the actions subquery up into separate temp tables which are then combined. 
		// The query optimizer does a better job at these smaller individual query plans than with one large complex plan.
		// (c.haag 2016-04-07) - NX-100089 - The query has been moved into the API
		//
		// (z.manning 2016-04-08) - NX-100090 - There is now a preference to control this behavior. The values are...
		//  1 - Always spawn
		//  2 - Prompt
		//  3 - Skip duplicates
		long nDuplicateSpawnedProblemPref = GetRemotePropertyInt("DuplicateSpawnedEmrProblemBehavior", 2);
		if (nDuplicateSpawnedProblemPref != 1)
		{
			bool bDeleteDuplicateProblem = false;
			if (nDuplicateSpawnedProblemPref == 3) // Skip duplicates
			{
				// (z.manning 2016-12-04 13:46) - NX-100140 - We have special handling when the preference is set to silently
				// skip duplicates. Basically, the original implemention of this feature ignored unsaved changes which has 
				// created all sorts of quirks. This fuction helps work around some of them by checking for problems that
				// have already been deleted.
				if (!ShouldSpawnEmrProblemForPatient(nPatientID, pProblem->m_nEmrProblemActionID, setProblemIDsToIgnore)) {
					bDeleteDuplicateProblem = true;
				}
			}
			else
			{
				NexTech_Accessor::_EMRProblemsPtr pDuplicateProblems = GetAPI()->GetExistingSpawnedPatientEMRProblems(GetAPISubkey(), GetAPILoginToken(), _bstr_t(pProblem->m_nEmrProblemActionID), _bstr_t(pProblem->m_nPatientID));
				Nx::SafeArray<IUnknown*> pFoundProblems = pDuplicateProblems->Problems;
				if (pFoundProblems.GetSize() > 0)
				{
					if (nDuplicateSpawnedProblemPref == 2) // Prompt
					{
						NexTech_Accessor::_EMRProblemPtr pFoundProblem = pFoundProblems.GetAt(0);

						//TES 10/30/2008 - PLID 31269 - OK, there is at least one matching problem.  Ask the user what they want to do.
						//Pull the status name for the prompt.
						_RecordsetPtr rsNewStatus = CreateParamRecordset("SELECT Name FROM EmrProblemStatusT WHERE ID = {INT}",
							pProblem->m_nStatusID);
						CString strNewStatus = AdoFldString(rsNewStatus, "Name");
						rsNewStatus->Close();
						if (IDYES != MsgBox(MB_YESNO, "You are saving a problem which is a duplicate of an already existing problem "
							"for this patient.\r\n\r\n"
							"New problem:\tDescription - %s, Status - %s, Entered On - %s\r\n\r\n"
							"Existing problem:\tDescription - %s, Status - %s, Entered On - %s\r\n\r\n"
							"Would you like to keep the new problem?  If Yes, both problems will appear in the patient's Problem List. "
							"If No, the new problem will be discarded, and all your changes will be lost.  In either case, the existing "
							"problem will not be modified.", pProblem->m_strDescription, strNewStatus, FormatDateTimeForInterface(pProblem->m_dtEnteredDate, NULL, dtoDate),
							(LPCTSTR)pFoundProblem->description, (LPCTSTR)pFoundProblem->status->Name,
							FormatDateTimeForInterface(pFoundProblem->EnteredDate, NULL, dtoDate))
							)
						{
							bDeleteDuplicateProblem = true;
						}
					}
					else
					{
						// (z.manning 2016-04-08) - NX-100090 - Invalid preference value, just proceed as normal and keep the duplicate
					}
				}
			}

			if (bDeleteDuplicateProblem)
			{
				//TES 10/30/2008 - PLID 31269 - OK, flag it as deleted
				pProblem->m_bIsDeleted = true;
				//TES 10/30/2008 - PLID 31269 - Now, tell the interface to refresh itself; it will note that the problem
				// is now deleted, and therefore remove it from the screen.
				// (c.haag 2009-05-28 13:10) - PLID 34277 - Post a message to the notification window that a problem changed
				if (NULL != pWndNotify && IsWindow(pWndNotify->GetSafeHwnd())) {
					PostMessage(pWndNotify->GetSafeHwnd(), NXM_EMR_PROBLEM_CHANGED, NULL, NULL);
				}
			}
		}
		else
		{
			// (z.manning 2016-04-12 13:45) - NX-100090 - Preference is set to always spawn so no proceed as normal
		}
	}

	CString strOnsetDate = "NULL";
	if(pProblem->m_dtOnsetDate.GetStatus() != COleDateTime::invalid) {
		strOnsetDate.Format("'%s'", FormatDateTimeForSql(pProblem->m_dtOnsetDate));
	}

	// (a.walling 2009-05-04 09:49) - PLID 28495 - Diag code
	// (j.jones 2014-02-24 15:28) - PLID 61010 - EMR problems now have
	// an ICD-9 code and/or an ICD-10 code, no more than one pair.
	CString strDiagCode9 = pProblem->m_nDiagICD9CodeID == -1 ? "NULL" : FormatString("%li", pProblem->m_nDiagICD9CodeID);
	CString strDiagCode10 = pProblem->m_nDiagICD10CodeID == -1 ? "NULL" : FormatString("%li", pProblem->m_nDiagICD10CodeID);

	// (a.walling 2009-05-04 09:50) - PLID 33751 - Chronicity
	CString strChronicity = pProblem->m_nChronicityID == -1 ? "NULL" : FormatString("%li", pProblem->m_nChronicityID);

	// (b.spivey, October 22, 2013) - PLID 58677 - Set codeID
	CString strCodeID = pProblem->m_nCodeID == -1 ? "NULL" : FormatString("%li", pProblem->m_nCodeID);
	// (s.tullis 2015-02-23 15:44) - PLID 64723
	BOOL bDoNotShowOnCCDA = pProblem->m_bDoNotShowOnCCDA;
	// (r.gonet 2015-03-09 18:21) - PLID 65008 - Assign DoNotShowOnProblemPrompt.
	BOOL bDoNotShowOnProblemPrompt = pProblem->m_bDoNotShowOnProblemPrompt;

	if(pProblem->m_nID < 0 && !pProblem->m_bIsDeleted)
	{
		// (a.walling 2009-05-04 09:49) - PLID 28495 - Diag code
		// (a.walling 2009-05-04 09:50) - PLID 33751 - Chronicity
		AddStatementToSqlBatch(strSqlBatch, "SET @nEMRProblemID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM EMRProblemsT WITH(UPDLOCK, HOLDLOCK))");		
		// (j.jones 2008-07-16 09:11) - PLID 30739 - supported EMRRegardingType and EMRRegardingID
		// for details only, we will need to change this further to support other types
		// (b.spivey, October 22, 2013) - PLID 58677 - Insert codeID
		// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs
		// (s.tullis 2015-02-23 15:44) - PLID 64723
		// (r.gonet 2015-03-09 18:21) - PLID 65008 - Added DoNotShowOnProblemPrompt.
		AddStatementToSqlBatch(strSqlBatch, 
			"INSERT INTO EMRProblemsT (ID, PatientID, Description, StatusID, "
				"OnsetDate, EmrProblemActionID, DiagCodeID, DiagCodeID_ICD10, "
				"ChronicityID, CodeID, DoNotShowOnCCDA, DoNotShowOnProblemPrompt) \r\n"
			"VALUES (@nEMRProblemID, %li, '%s', %d, "
				"%s, %s, %s, %s, "
				"%s, %s, %li, %li)",
			nPatientID, _Q(pProblem->m_strDescription), pProblem->m_nStatusID, 
			strOnsetDate, (-1 == pProblem->m_nEmrProblemActionID) ? "NULL" : AsString(pProblem->m_nEmrProblemActionID), strDiagCode9, strDiagCode10, 
			strChronicity, strCodeID, bDoNotShowOnCCDA, bDoNotShowOnProblemPrompt);
		AddStatementToSqlBatch(strSqlBatch, "INSERT INTO EMRProblemHistoryT (ProblemID, Description, StatusID, UserID, DiagCodeID, DiagCodeID_ICD10, ChronicityID) "
			"VALUES (@nEMRProblemID, '%s', %d, %d, %s, %s, %s)",
			_Q(pProblem->m_strDescription), pProblem->m_nStatusID, GetCurrentUserID(), strDiagCode9, strDiagCode10, strChronicity);

		AddStatementToSqlBatch(strSqlBatch, "SET @nNewObjectID = @nEMRProblemID");
		AddNewEMRObjectToSqlBatch(strSqlBatch, esotProblem, (long)pProblem, mapSavedObjects);		

	}
	else if(pProblem->m_nID > 0 && pProblem->m_bIsModified)
	{
		if(pProblem->m_bIsDeleted) {
			AddStatementToSqlBatch(strSqlBatch, "UPDATE EMRProblemsT SET Deleted = 1, DeletedBy = '%s', DeletedDate = GetDate() WHERE ID = %d", _Q(GetCurrentUserName()), pProblem->m_nID);
			// (j.jones 2009-06-03 14:20) - PLID 34301 - ensure all the links are deleted
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM EMRProblemLinkT WHERE EMRProblemID = %d", pProblem->m_nID);
			// (c.haag 2008-08-29 10:48) - PLID 31214 - Create an audit transaction ID if we don't have one
			if(nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			AuditEvent(nPatientID, strPatientName,nAuditTransactionID,aeiEMNProblemDeleted,pProblem->m_nID,"","<Deleted>",aepHigh,aetDeleted);
		}
		else {

			// (c.haag 2006-12-27 10:32) - PLID 23158 - If the problem was unexpectedly deleted, then do not run the update.
			// We will warn the user about this later in PostSaveUpdate.
			// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
			if (ReturnsRecordsParam("SELECT TOP 1 ID FROM EMRProblemsT WHERE ID = {INT} AND Deleted = 0", pProblem->m_nID)) {
				
				// (a.walling 2009-05-04 09:49) - PLID 28495 - Diag code
				// (a.walling 2009-05-04 09:50) - PLID 33751 - Chronicity
				// (c.haag 2009-05-11 17:35) - PLID 28494 - I'm removing the regarding field from this query because
				// you couldn't change the regarding ID or type of a problem record in the pre-problem-link-table structure
				// in the first place.
				// (b.spivey, October 22, 2013) - PLID 58677 - Set codeID
				// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs
				// (s.tullis 2015-02-23 15:44) - PLID 64723
				// (r.gonet 2015-03-09 18:21) - PLID 65008 - Added DoNotShowOnProblemPrompt.
				AddStatementToSqlBatch(strSqlBatch, "UPDATE EMRProblemsT SET ModifiedDate = GetDate(), Description = '%s', "
					"StatusID = %d, OnsetDate = %s, DiagCodeID = %s, DiagCodeID_ICD10 = %s, ChronicityID = %s, CodeID = %s, "
					"DoNotShowOnCCDA = %li, DoNotShowOnProblemPrompt = %li "
					"WHERE ID = %d ",
					_Q(pProblem->m_strDescription), 
					pProblem->m_nStatusID, strOnsetDate, strDiagCode9, strDiagCode10, strChronicity, strCodeID,
					pProblem->m_bDoNotShowOnCCDA, pProblem->m_bDoNotShowOnProblemPrompt,
					pProblem->m_nID);

				// (c.haag 2008-07-23 10:37) - PLID 30727 - Now write to the EMR problem history table if either the
				// status or description have changed. We don't need to log the onset date because that ideally should
				// never change once it's been entered -- it doesn't progress like the rest of the problem fields do.
				
				// (a.walling 2009-05-04 09:49) - PLID 28495 - Diag code
				// (a.walling 2009-05-04 09:50) - PLID 33751 - Chronicity
				// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs
				if(pProblem->m_strLastDescription != pProblem->m_strDescription || pProblem->m_nLastStatusID != pProblem->m_nStatusID
					|| pProblem->m_nLastDiagICD9CodeID != pProblem->m_nDiagICD9CodeID
					|| pProblem->m_nLastDiagICD10CodeID != pProblem->m_nDiagICD10CodeID
					|| pProblem->m_nLastChronicityID != pProblem->m_nChronicityID)
				{
					AddStatementToSqlBatch(strSqlBatch, "INSERT INTO EMRProblemHistoryT (ProblemID, Description, StatusID, UserID, DiagCodeID, DiagCodeID_ICD10, ChronicityID) "
						"VALUES (%d, '%s', %d, %d, %s, %s, %s)",
						pProblem->m_nID, _Q(pProblem->m_strDescription), pProblem->m_nStatusID, GetCurrentUserID(), strDiagCode9, strDiagCode10, strChronicity);
				}

				if(pProblem->m_strLastDescription != pProblem->m_strDescription) {
					if(nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					AuditEvent(nPatientID, strPatientName,nAuditTransactionID,aeiEMNProblemDesc,pProblem->m_nID, pProblem->m_strLastDescription, pProblem->m_strDescription, aepMedium,aetChanged);
				}
				if(pProblem->m_nLastStatusID != pProblem->m_nStatusID) {
					if(nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					CString strOldStatus = VarString(GetTableField("EMRProblemStatusT", "Name", "ID", pProblem->m_nLastStatusID), "");
					CString strNewStatus = VarString(GetTableField("EMRProblemStatusT", "Name", "ID", pProblem->m_nStatusID), "");
					AuditEvent(nPatientID, strPatientName,nAuditTransactionID,aeiEMNProblemStatus,pProblem->m_nID,strOldStatus,strNewStatus,aepMedium,aetChanged);
				}
				// (a.walling 2009-05-04 09:49) - PLID 28495 - Diag code
				// (j.jones 2014-02-24 11:49) - PLID 60781 - EMR problems now have
				// an ICD-9 code and/or an ICD-10 code, no more than one pair.
				if(pProblem->m_nLastDiagICD9CodeID != pProblem->m_nDiagICD9CodeID
					|| pProblem->m_nLastDiagICD10CodeID != pProblem->m_nDiagICD10CodeID) {
					if(nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					CString strOldDiagCode9, strNewDiagCode9;
					CString strOldDiagCode10, strNewDiagCode10;
					GetICD9And10Codes(pProblem->m_nLastDiagICD9CodeID, pProblem->m_nLastDiagICD10CodeID, strOldDiagCode9, strOldDiagCode10);
					GetICD9And10Codes(pProblem->m_nDiagICD9CodeID, pProblem->m_nDiagICD10CodeID, strNewDiagCode9, strNewDiagCode10);

					CString strOldDiagCodeAudit, strNewDiagCodeAudit;
					GetProblemICD910AuditDescriptions(strOldDiagCode9, strOldDiagCode10, strNewDiagCode9, strNewDiagCode10, strOldDiagCodeAudit, strNewDiagCodeAudit);

					AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiEMNProblemDiagCode, pProblem->m_nID, strOldDiagCodeAudit, strNewDiagCodeAudit, aepMedium, aetChanged);
				}
				// (a.walling 2009-05-04 09:50) - PLID 33751 - Chronicity
				if(pProblem->m_nLastChronicityID != pProblem->m_nChronicityID) {
					if(nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					CString strOldChronicity = VarString(GetTableField("EMRProblemChronicityT", "Name", "ID", pProblem->m_nLastChronicityID), "");
					CString strNewChronicity = VarString(GetTableField("EMRProblemChronicityT", "Name", "ID", pProblem->m_nChronicityID), "");
					AuditEvent(nPatientID, strPatientName,nAuditTransactionID,aeiEMNProblemChronicity,pProblem->m_nID,strOldChronicity,strNewChronicity,aepMedium,aetChanged);
				}
				if(pProblem->m_dtLastOnsetDate != pProblem->m_dtOnsetDate) {
					CString strOld = (pProblem->m_dtLastOnsetDate.GetStatus() == COleDateTime::invalid) ? "" : FormatDateTimeForInterface(pProblem->m_dtLastOnsetDate);
					CString strNew = (pProblem->m_dtOnsetDate.GetStatus() == COleDateTime::invalid) ? "" : FormatDateTimeForInterface(pProblem->m_dtOnsetDate);
					if(nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					AuditEvent(nPatientID, strPatientName,nAuditTransactionID,aeiEMNProblemOnsetDate,pProblem->m_nID,strOld,strNew,aepMedium,aetChanged);
				}

				// (b.spivey, October 22, 2013) - PLID 58677 - Set codeID
				if(pProblem->m_nLastCodeID != pProblem->m_nCodeID) {
					CString strOldProblemCode = VarString(GetTableField("CodesT", "Code", "ID", pProblem->m_nLastCodeID), "");
					CString strNewProblemCode = VarString(GetTableField("CodesT", "Code", "ID", pProblem->m_nCodeID), "");
					if(nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiEMNProblemSNOMEDCode, pProblem->m_nID, strOldProblemCode, strNewProblemCode, aepMedium, aetChanged); 
				}
				// (s.tullis 2015-03-11 10:09) - PLID 64723 - Audit DoNotshowonCCDA
				if (pProblem->m_bLastDoNotShowOnCCDA != pProblem->m_bDoNotShowOnCCDA) {
					AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiEMNProblemDoNotShowOnCCDA, pProblem->m_nID,
						pProblem->m_bLastDoNotShowOnCCDA ? "Checked" : "Unchecked", pProblem->m_bDoNotShowOnCCDA ? "Checked" : "Unchecked", aepMedium, aetChanged);
				}
				// (r.gonet 2015-03-06 10:16) - PLID 65008 - Audit any changes in the EMR Problem's Do Not Show On Problem Prompt flag.
				if (pProblem->m_bLastDoNotShowOnProblemPrompt != pProblem->m_bDoNotShowOnProblemPrompt) {
					AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiEMNProblemDoNotShowOnProblemPrompt, pProblem->m_nID, 
						pProblem->m_bLastDoNotShowOnProblemPrompt ? "Checked" : "Unchecked", pProblem->m_bDoNotShowOnProblemPrompt ? "Checked" : "Unchecked", aepMedium, aetChanged);
				}
			}
		}
	}
}

// (a.walling 2014-01-30 00:00) - PLID 60546 - Quantize
void SaveProblemLinkArray(Nx::Quantum::Batch &strSqlBatch, CArray<CEmrProblemLink*, CEmrProblemLink*> &aryProblemLinks, const CString strObjectID, IN CMapPtrToPtr &mapSavedObjects, long &nAuditTransactionID, const long nPatientID, const CString strPatientName)
{
	for(int nProblemLinkIndex = 0; nProblemLinkIndex < aryProblemLinks.GetSize(); nProblemLinkIndex++) {
		CEmrProblemLink *pProblemLink = aryProblemLinks.GetAt(nProblemLinkIndex);
		SaveProblemLink(strSqlBatch, pProblemLink, strObjectID, mapSavedObjects, nAuditTransactionID, nPatientID, strPatientName);
	}
}

// (z.manning 2009-05-21 15:51) - PLID 34297 - Save a problem link
// (a.walling 2014-01-30 00:00) - PLID 60546 - Quantize
void SaveProblemLink(Nx::Quantum::Batch &strSqlBatch, CEmrProblemLink *pProblemLink, const CString strObjectID, IN CMapPtrToPtr &mapSavedObjects, long &nAuditTransactionID, const long nPatientID, const CString strPatientName)
{
	if(pProblemLink->GetID() == -1 && !pProblemLink->GetIsDeleted() && !pProblemLink->GetProblem()->m_bIsDeleted)
	{
		// (z.manning 2009-05-21 17:07) - PLID 34297 - This a new, unsaved problem link.
		CString strDataID = "NULL";
		if(pProblemLink->GetDataID() != -1) {
			strDataID = AsString(pProblemLink->GetDataID());
		}

		if(pProblemLink->GetProblem()->m_nID == -1) {
			AddStatementToSqlBatch(strSqlBatch,"SET @nEMRProblemID = (SELECT ID FROM #NewObjectsT WHERE Type = %i AND ObjectPtr = %li);"
				, esotProblem, (long)pProblemLink->GetProblem());
		}
		else {
			AddStatementToSqlBatch(strSqlBatch,"SET @nEMRProblemID = %li;", pProblemLink->GetProblem()->m_nID);
		}

		// (z.manning 2009-05-21 15:58) - PLID 34297 - We have a newly added problem link, so save it.
		AddStatementToSqlBatch(strSqlBatch,
			"INSERT INTO EmrProblemLinkT (EmrProblemID, EmrRegardingType, EmrRegardingID, EmrDataID) \r\n"
			"VALUES (@nEMRProblemID, %i, %s, %s);"
			, pProblemLink->GetType(), strObjectID, strDataID);

		AddStatementToSqlBatch(strSqlBatch, "SET @nNewObjectID = (SELECT CONVERT(int, SCOPE_IDENTITY()));");

		AddNewEMRObjectToSqlBatch(strSqlBatch, esotProblemLink, (long)pProblemLink, mapSavedObjects);

	}
	else if(pProblemLink->GetID() != -1 && (pProblemLink->GetIsDeleted() || pProblemLink->GetProblem()->m_bIsDeleted))
	{
		// (z.manning 2009-05-21 17:01) - PLID 34297 - This is a saved problem that that is now deleted.
		AddStatementToSqlBatch(strSqlBatch,
			"DELETE FROM EmrProblemLinkT WHERE ID = %li; \r\n"
			, pProblemLink->GetID());

		pProblemLink->Audit(aeiEMNProblemLinkDeleted, nAuditTransactionID, strPatientName);
	}
	// (z.manning 2009-05-21 17:08) - PLID 34297 - The only other possible cases are existing, unsaved
	// problem links (impossible currently) and new, deleted problem links (don't need to do anything).
}

// (c.haag 2008-07-23 12:16) - PLID 30820 - Populate apProblems with a list of all deleted problems for a specified object and
// all its children. If a child or related EMR object is deleted, all its problems are considered deleted as well.
// (j.jones 2009-05-29 09:46) - PLID 34301 - returns links now, not problems
void GetAllDeletedEmrProblemLinks(EmrSaveObjectType esotSaveType, long nObjectPtr, CArray<CEmrProblemLink*,CEmrProblemLink*>& apProblemLinks, BOOL bIncludeThisObject)
{
	switch(esotSaveType) {
		case esotTopic: {
			CEMRTopic *pTopic = (CEMRTopic*)nObjectPtr;
			if (!pTopic->IsTemplate() && !pTopic->GetIsOnLockedAndSavedEMN()) {
				pTopic->GetAllDeletedEmrProblemLinks(apProblemLinks, bIncludeThisObject);
			}
			break;
		}
		case esotEMN: {
			CEMN *pEMN = (CEMN*)nObjectPtr;
			if (!pEMN->IsTemplate() && !pEMN->IsLockedAndSaved()) {
				pEMN->GetAllDeletedEmrProblemLinks(apProblemLinks, bIncludeThisObject);
			}
			break;
		}
		case esotEMR:
		default: {
			CEMR* pEMR = (CEMR*)nObjectPtr;
			if (!pEMR->GetIsTemplate()) {
				pEMR->GetAllDeletedEmrProblemLinks(apProblemLinks, bIncludeThisObject);
			}
			break;
		}
	}
}

// (c.haag 2008-07-23 13:28) - PLID 30820 - This function will detect if there are any problems which exist in data and were
// not manually deleted, yet they belong to an EMR object that is about to be deleted. The user may not be aware that these
// problems are about to be deleted by virtue of their owner EMR object being deleted. So, a warning will appear if that is the
// case. The user must confirm that they want the problems deleted.
BOOL CheckForDeletedEmrProblems(EmrSaveObjectType esotSaveType, long nObjectPtr, BOOL bIncludeThisObject)
{
	//first off, we must get the top-level EMR
	CEMR *pCurEMR = NULL;
	switch(esotSaveType) {
		case esotTopic: {
			CEMRTopic *pTopic = (CEMRTopic*)nObjectPtr;
			if(pTopic->GetParentEMN() && pTopic->GetParentEMN()->GetParentEMR()) {
				pCurEMR = pTopic->GetParentEMN()->GetParentEMR();
			}
			//if bIncludeThisObject is TRUE, it means we're deleting the topic,
			//but we're checking at the EMR level, so we don't care
			bIncludeThisObject = FALSE;
			break;
		}
		case esotEMN: {
			CEMN *pEMN = (CEMN*)nObjectPtr;
			if(pEMN->GetParentEMR()) {
				pCurEMR = pEMN->GetParentEMR();
			}
			//if bIncludeThisObject is TRUE, it means we're deleting the EMN,
			//but we're checking at the EMR level, so we don't care
			bIncludeThisObject = FALSE;
			break;
		}
		case esotEMR:
		default: {
			pCurEMR = (CEMR*)nObjectPtr;

			//if bIncludeThisObject is TRUE, it means we're deleting the EMR,
			//so in this case we want bIncludeThisObject to remain true
			break;
		}
	}

	ASSERT(pCurEMR);

	// (c.haag 2009-06-04 16:22) - PLID 34293 - Search for any problems that are not in data,
	// and are not linked with any non-deleted problem links. This can happen if someone spawns
	// and then unspawns a detail in the same session before saving. Without this check, the 
	// saving code would write the problem to data, but not write any links.
	{
		CArray<CEmrProblem*,CEmrProblem*> apProblems;
		CArray<CEmrProblemLink*,CEmrProblemLink*> apProblemLinks;
		pCurEMR->GetAllProblems(apProblems);
		pCurEMR->GetAllProblemLinks(apProblemLinks);
		int i;

		// Remove all problems that do not exist in data from apProblems
		for (i=0; i < apProblems.GetSize(); i++) {
			if (apProblems[i]->m_nID > -1) {
				apProblems.RemoveAt(i--);
			}
		}

		// Remove all problem links that do not exist in data from apProblemLinks
		for (i=0; i < apProblemLinks.GetSize(); i++) {
			if (apProblemLinks[i]->GetID() > -1) {
				apProblemLinks.RemoveAt(i--);
			}	
		}
		
		// Now...go through all the problem links again, and for any link that isn't
		// deleted, remove it from apProblems
		for (i=0; i < apProblemLinks.GetSize(); i++) {
			if (!apProblemLinks[i]->IsDeleted()) {
				for (int j=0; j < apProblems.GetSize(); j++) {
					if (apProblems[j] == apProblemLinks[i]->GetProblem()) {
						apProblems.RemoveAt(j--);
						break;
					}
				}
			}
		}

		// When we get here, anything left in apProblems are unsaved, undeleted EMR problems
		// that are not assigned to any undeleted links. We need to flag these problems as
		// deleted so that they don't get saved.
		for (i=0; i < apProblems.GetSize(); i++) {
			apProblems[i]->m_bIsDeleted = TRUE;
		}
	}

	// (j.jones 2009-05-29 10:21) - PLID 34301 - uses the new problem structure
	CArray<CEmrProblemLink*,CEmrProblemLink*> apProblemLinksToDelete;

	//we go from the EMR level because in SaveRowObject we will already be forcing a save of the whole EMR
	//if any problem within the EMR changed, so check for everything on the EMR right now
	GetAllDeletedEmrProblemLinks(esotEMR, (long)pCurEMR, apProblemLinksToDelete, bIncludeThisObject);

	//now we also need to find all problems marked deleted, AND all problems who have no links remaining
	CArray<CEmrProblem*,CEmrProblem*> apProblems;
	if (!pCurEMR->GetIsTemplate()) {
		pCurEMR->GetAllProblems(apProblems, TRUE);
	}

	CArray<CEmrProblem*,CEmrProblem*> apProblemsToDelete;

	// (c.haag 2009-06-08 09:25) - PLID 34398 - I moved the code to populate apProblemsToDelete into a
	// utility function in CEMR. The array will be filled with problems flagged as deleted; as well as
	// problems with no undeleted links
	if (!pCurEMR->GetIsTemplate()) {
		pCurEMR->FindEmrProblemsToDelete(apProblems, apProblemLinksToDelete, apProblemsToDelete, TRUE /* Only check problems already in data */);
	}
	
	// Invoke a dialog to display all the problem links and have the user either commit all changes or deny the commit
	CEmrProblemDeleteDlg dlg(pCurEMR->GetInterface());
	dlg.m_strTopCaption = "The following is a list of problem links that will be removed from your data. Entries in red represent problems that will no longer be linked to any areas in the system, and will be removed entirely.";
	
	//track whether links were added - not the same as apProblemLinksToDelete.GetSize()
	//because that may legitimately contain links with -1 IDs, which we do not need to warn
	//about
	BOOL bAddedProblemLinks = FALSE;
	
	int i;
	for (i=0; i < apProblemLinksToDelete.GetSize(); i++) {
		if (apProblemLinksToDelete[i]->GetID() > 0) {
			dlg.m_apProblemLinksToBeDeleted.Add(apProblemLinksToDelete[i]);
			bAddedProblemLinks = TRUE;
		} else {
			//This CAN happen now because GetAllDeletedEmrProblemLinks
			//intentionally does not filter out -1 ID links that were
			//created in memory but will not be saved
			//ASSERT(FALSE);
		}
	}
	for (i=0; i < apProblemsToDelete.GetSize(); i++) {
		if (apProblemsToDelete[i]->m_nID > 0) {
			dlg.m_apProblemsToBeDeleted.Add(apProblemsToDelete[i]);
		} else {
			ASSERT(FALSE); // This should never happen
		}
	}

	// Quit if no links were added - don't check problems because
	// we really only display links in the dialog
	// (example, if we unspawned a lab it would have already deleted the problem
	// in data, and potentially flagged the deleted problem in memory, but it
	// would have already been saved, and we wouldn't have any problem links)
	if(!bAddedProblemLinks) {
		return TRUE;
	}

	// Invoke the dialog
	if (IDOK == dlg.DoModal()) {
		// If the user clicked OK, we're free to delete the problems

		// (j.jones 2009-05-29 12:48) - PLID 34301 - any problems & problem links
		// that we calculated as being delete-able, mark them as deleted now
		for (i=0; i < apProblemLinksToDelete.GetSize(); i++) {
			apProblemLinksToDelete[i]->SetDeleted();
			//shouldn't need to update the interface here because
			//any links we are now forcing to be deleted only
			//do so because of changes we already made
		}

		for (i=0; i < apProblemsToDelete.GetSize(); i++) {
			apProblemsToDelete[i]->m_bIsDeleted = TRUE;
			apProblemsToDelete[i]->m_bIsModified = TRUE;
			//shouldn't need to update the interface here because
			//any problems we are now forcing to be deleted only
			//do so because of link changes we already made
		}

		return TRUE;
	} else {
		// If the user cancelled, then we return a failure code
		return FALSE;
	}
}

// (c.haag 2008-08-06 11:03) - PLID 30820 - Ensures a problem exists in a given array
void EnsureProblemInArray(CArray<CEmrProblem*,CEmrProblem*>& apProblems, CEmrProblem* pProblem)
{
	const int nProblems = apProblems.GetSize();
	for (int i=0; i < nProblems; i++) {
		if (apProblems[i] == pProblem) {
			return;
		}
	}
	apProblems.Add(pProblem);
}

// (j.jones 2009-05-29 09:53) - PLID 34301 - added EnsureProblemLinkInArray, same logic as EnsureProblemInArray
void EnsureProblemLinkInArray(CArray<CEmrProblemLink*,CEmrProblemLink*>& apProblemLinks, CEmrProblemLink* pProblemLink)
{
	const int nProblemLinks = apProblemLinks.GetSize();
	for (int i=0; i < nProblemLinks; i++) {
		if (apProblemLinks[i] == pProblemLink) {
			return;
		}
	}
	apProblemLinks.Add(pProblemLink);
}

// (c.haag 2009-06-08 09:49) - PLID 34398 - Like above, but we ensure all the problems in apProblemLinks also exist in apProblems
void EnsureProblemsInArray(const CArray<CEmrProblemLink*,CEmrProblemLink*>& apProblemLinks,
						   CArray<CEmrProblem*,CEmrProblem*>& apProblems)
{
	for (int i=0; i < apProblemLinks.GetSize(); i++) {
		CEmrProblem* p = apProblemLinks[i]->GetProblem();
		EnsureProblemInArray(apProblems, p);
	}
}

// (c.haag 2008-07-24 08:41) - PLID 30820 - This is the "from" clause for a datalist that uses the standard EMR problem
// list query. J.Jones is the original author, but I moved it to this function.
// (c.haag 2009-05-28 13:06) - PLID 34298 - We now get EMRDataID
CString GetEmrProblemListFromClause()
{
	// (j.jones 2008-07-23 08:20) - PLID 30731 - changed the from clause such
	// that we can reflect the object descriptions for each object type
	// (j.jones 2008-10-30 14:48) - PLID 31869 - added EMNID and DataType
	// (a.walling 2009-05-04 12:18) - PLID 33751, 28495 - Added DiagCodeName, ChronicityName
	// (c.haag 2009-05-12 12:43) - PLID 34234 - Now uses the new EMR problem link structure
	// (c.haag 2009-05-22 09:48) - PLID 34298 - Added EMRProblemLinkID
	// (z.manning 2009-05-28 12:37) - PLID 34345 - Handle lab problems
	// (z.manning 2010-07-28 12:03) - PLID 39874 - Added resolved, EMN ID, and EMR ID
	// (b.spivey, October 22, 2013) - PLID 58677 - Added codeID
	// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs
	// (j.jones 2014-02-26 11:27) - PLID 60763 - need extra fields for just ICD-9 and just ICD-10, in addition to the combination
	// (j.jones 2014-03-24 08:35) - PLID 61505 - if a problem is linked to a diagnosis, show the 9 and 10 code if both exist
	// (s.tullis 2015-02-23 15:44) - PLID 64723 
	// (r.gonet 2015-03-09 19:24) - PLID 64723 - Added DoNotShowOnProblemPrompt.
	 return FormatString("(SELECT EMRProblemsT.ID, EMRProblemLinkT.EMRRegardingType, EMRProblemLinkT.EMRRegardingID, "
		"EMRProblemsT.EnteredDate, EMRProblemsT.ModifiedDate, EMRProblemsT.OnsetDate, "
		"EMRProblemsT.PatientID, EMRProblemsT.Deleted, EMRProblemLinkT.ID AS EMRProblemLinkID, EMRProblemLinkT.EMRDataID, "
		"EMRProblemsT.StatusID, EMRProblemStatusT.Name AS StatusName, EMRProblemStatusT.Resolved, "
		"CASE WHEN DiagCodes9.CodeNumber Is Null AND DiagCodes10.CodeNumber Is Null THEN '<None>' "
		"	WHEN DiagCodes9.CodeNumber Is Not Null AND DiagCodes10.CodeNumber Is Null THEN DiagCodes9.CodeNumber "
		"	WHEN DiagCodes9.CodeNumber Is Null AND DiagCodes10.CodeNumber Is Not Null THEN DiagCodes10.CodeNumber "
		"	WHEN DiagCodes9.CodeNumber Is Not Null AND DiagCodes10.CodeNumber Is Not Null THEN DiagCodes9.CodeNumber + ' / ' + DiagCodes10.CodeNumber "
		"END AS DiagCodeName, "
		"COALESCE(DiagCodes9.CodeNumber, '<None>') AS ICD9Name, "
		"COALESCE(DiagCodes10.CodeNumber, '<None>') AS ICD10Name, "
		"COALESCE(EMRProblemChronicityT.Name, '<None>') AS ChronicityName, "
		"EMRProblemsT.Description AS ProblemDescription, "
		"EMRInfoT.DataType, EMRProblemsT.CodeID, EmrProblemsT.DoNotShowOnCCDA, EmrProblemsT.DoNotShowOnProblemPrompt, "
		""
		"CASE WHEN (EMRRegardingType = %li OR EMRRegardingType = %li) THEN EMRDetailsT.ID ELSE NULL END AS DetailID, "
		""
		"CASE WHEN (EMRRegardingType = %li OR EMRRegardingType = %li) THEN '' "
		"WHEN EMRRegardingType = %li THEN '<Diagnosis Code>' "
		"WHEN EMRRegardingType = %li THEN '<Charge>' "
		"WHEN EMRRegardingType = %li THEN '<Medication>' "
		"WHEN EMRRegardingType = %li THEN '<Topic>' "
		"WHEN EMRRegardingType = %li THEN '<EMN>' "
		"WHEN EMRRegardingType = %li THEN '<EMR>' "
		"WHEN EMRRegardingType = %li THEN '<Lab>' "
		"ELSE '' END AS DetailValue, "
		""
		"CASE WHEN (EMRRegardingType = %li OR EMRRegardingType = %li) THEN "
		"	CASE WHEN MergeOverride IS NULL THEN (CASE WHEN EMRInfoT.ID = -27 THEN MacroName ELSE EMRInfoT.Name END) ELSE MergeOverride END "
		"WHEN EMRRegardingType = %li THEN "
		"	CASE WHEN EMRDiagCodes9.ID Is Not Null AND EMRDiagCodes10.ID Is Not Null THEN EMRDiagCodes10.CodeNumber + ' - ' + EMRDiagCodes10.CodeDesc + ' (' + EMRDiagCodes9.CodeNumber + ' - ' + EMRDiagCodes9.CodeDesc + ')' "
		"	ELSE Coalesce(EMRDiagCodes10.CodeNumber, EMRDiagCodes9.CodeNumber) + ' - ' + Coalesce(EMRDiagCodes10.CodeDesc, EMRDiagCodes9.CodeDesc) END "
		"WHEN EMRRegardingType = %li THEN EMRChargesT.Description "
		"WHEN EMRRegardingType = %li THEN DrugDataT.Data "
		"WHEN EMRRegardingType = %li THEN EMRProblemTopic.Name "
		"WHEN EMRRegardingType = %li THEN EMRProblemMaster.Description "
		"WHEN EMRRegardingType = %li THEN EMRProblemGroup.Description "
		"WHEN EMRRegardingType = %li THEN COALESCE(FormNumberTextID, '') + '-' + COALESCE(Specimen, '') + ' - ' + CASE WHEN LabsT.Type = %li THEN COALESCE(LabAnatomyT.Description, '') ELSE ToBeOrdered END "
		"ELSE '' END AS DetailName, "

		"CASE WHEN (EMRRegardingType = %li OR EMRRegardingType = %li) THEN EMRDetailTopic.Name "
		"WHEN EMRRegardingType = %li THEN EMRProblemTopic.Name "
		"ELSE '' END "
		"AS TopicName, "
		""
		"CASE WHEN (EMRRegardingType = %li OR EMRRegardingType = %li) THEN EMRDetailMaster.Description "
		"WHEN EMRRegardingType = %li THEN EMRTopicMaster.Description "
		"WHEN EMRRegardingType = %li THEN EMRProblemMaster.Description "
		"WHEN EMRRegardingType = %li THEN EMRDiagMaster.Description "
		"WHEN EMRRegardingType = %li THEN EMRChargeMaster.Description "
		"WHEN EMRRegardingType = %li THEN EMRMedicationMaster.Description "
		"ELSE '' END "
		"AS EMNName, "
		""
		"CASE WHEN (EMRRegardingType = %li OR EMRRegardingType = %li) THEN EMRDetailMaster.ID "
		"WHEN EMRRegardingType = %li THEN EMRTopicMaster.ID "
		"WHEN EMRRegardingType = %li THEN EMRProblemMaster.ID "
		"WHEN EMRRegardingType = %li THEN EMRDiagMaster.ID "
		"WHEN EMRRegardingType = %li THEN EMRChargeMaster.ID "
		"WHEN EMRRegardingType = %li THEN EMRMedicationMaster.ID "
		"ELSE NULL END "
		"AS EmnID, "
		""
		"CASE WHEN (EMRRegardingType = %li OR EMRRegardingType = %li) THEN EMRDetailGroup.Description "
		"WHEN EMRRegardingType = %li THEN EMRTopicGroup.Description "
		"WHEN EMRRegardingType = %li THEN EMRMasterGroup.Description "
		"WHEN EMRRegardingType = %li THEN EMRProblemGroup.Description "
		"WHEN EMRRegardingType = %li THEN EMRDiagGroup.Description "
		"WHEN EMRRegardingType = %li THEN EMRChargeGroup.Description "
		"WHEN EMRRegardingType = %li THEN EMRMedicationGroup.Description "
		"ELSE '' END "
		"AS EMRName, " 
		""	
		"CASE WHEN (EMRRegardingType = %li OR EMRRegardingType = %li) THEN EMRDetailGroup.ID "
		"WHEN EMRRegardingType = %li THEN EMRTopicGroup.ID "
		"WHEN EMRRegardingType = %li THEN EMRMasterGroup.ID "
		"WHEN EMRRegardingType = %li THEN EMRProblemGroup.ID "
		"WHEN EMRRegardingType = %li THEN EMRDiagGroup.ID "
		"WHEN EMRRegardingType = %li THEN EMRChargeGroup.ID "
		"WHEN EMRRegardingType = %li THEN EMRMedicationGroup.ID "
		"ELSE NULL END "
		"AS EmrID " 
		""			
		"FROM EMRProblemsT "
		"INNER JOIN EMRProblemLinkT ON EMRProblemLinkT.EMRProblemID = EMRProblemsT.ID "
		""
		"LEFT JOIN EMRDetailsT ON EMRDetailsT.ID = EMRProblemLinkT.EMRRegardingID "
		"LEFT JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
		"LEFT JOIN EMRTopicsT AS EMRDetailTopic ON EMRDetailTopic.ID = EMRDetailsT.EMRTopicID "
		"LEFT JOIN EMRMasterT AS EMRDetailMaster ON EMRDetailMaster.ID = EMRDetailsT.EMRID "
		"LEFT JOIN EMRGroupsT AS EMRDetailGroup ON EMRDetailGroup.ID = EMRDetailMaster.EMRGroupID "
		""			
		"LEFT JOIN EMRTopicsT AS EMRProblemTopic ON EMRProblemLinkT.EMRRegardingID = EMRProblemTopic.ID "
		"LEFT JOIN EMRMasterT AS EMRTopicMaster ON EMRTopicMaster.ID = EMRProblemTopic.EMRID "
		"LEFT JOIN EMRGroupsT AS EMRTopicGroup ON EMRTopicGroup.ID = EMRTopicMaster.EMRGroupID "
		""
		"LEFT JOIN EMRMasterT AS EMRProblemMaster ON EMRProblemMaster.ID = EMRProblemLinkT.EMRRegardingID "
		"LEFT JOIN EMRGroupsT AS EMRMasterGroup ON EMRMasterGroup.ID = EMRProblemMaster.EMRGroupID "
		""			
		"LEFT JOIN EMRGroupsT AS EMRProblemGroup ON EMRProblemGroup.ID = EMRProblemLinkT.EMRRegardingID "
		""			
		"LEFT JOIN EMRDiagCodesT ON EMRProblemLinkT.EMRRegardingID = EMRDiagCodesT.ID "
		"LEFT JOIN DiagCodes EMRDiagCodes9 ON EMRDiagCodes9.ID = EMRDiagCodesT.DiagCodeID "
		"LEFT JOIN DiagCodes EMRDiagCodes10 ON EMRDiagCodes10.ID = EMRDiagCodesT.DiagCodeID_ICD10 "
		"LEFT JOIN EMRMasterT AS EMRDiagMaster ON EMRDiagMaster.ID = EMRDiagCodesT.EMRID "
		"LEFT JOIN EMRGroupsT AS EMRDiagGroup ON EMRDiagGroup.ID = EMRDiagMaster.EMRGroupID "
		""			
		"LEFT JOIN EMRChargesT ON EMRProblemLinkT.EMRRegardingID = EMRChargesT.ID "
		"LEFT JOIN EMRMasterT AS EMRChargeMaster ON EMRChargeMaster.ID = EMRChargesT.EMRID "
		"LEFT JOIN EMRGroupsT AS EMRChargeGroup ON EMRChargeGroup.ID = EMRChargeMaster.EMRGroupID "
		""
		"LEFT JOIN EMRMedicationsT ON EMRProblemLinkT.EMRRegardingID = EMRMedicationsT.MedicationID "
		"LEFT JOIN PatientMedications ON EMRMedicationsT.MedicationID = PatientMedications.ID "
		"LEFT JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID "
		"LEFT JOIN EMRDataT AS DrugDataT ON DrugList.EMRDataID = DrugDataT.ID "
		"LEFT JOIN EMRMasterT AS EMRMedicationMaster ON EMRMedicationMaster.ID = EMRMedicationsT.EMRID "
		"LEFT JOIN EMRGroupsT AS EMRMedicationGroup ON EMRMedicationGroup.ID = EMRMedicationMaster.EMRGroupID "
		""			
		"LEFT JOIN EMRProblemStatusT ON EMRProblemStatusT.ID = EMRProblemsT.StatusID "
		"LEFT JOIN EMRProblemChronicityT ON EMRProblemChronicityT.ID = EMRProblemsT.ChronicityID "
		"LEFT JOIN DiagCodes DiagCodes9 ON DiagCodes9.ID = EMRProblemsT.DiagCodeID "
		"LEFT JOIN DiagCodes DiagCodes10 ON DiagCodes10.ID = EMRProblemsT.DiagCodeID_ICD10 "
		""
		"LEFT JOIN LabsT ON EmrProblemLinkT.EmrRegardingID = LabsT.ID AND EmrProblemLinkT.EmrRegardingType = %li "
		"LEFT JOIN LabAnatomyT ON LabAnatomyT.ID = LabsT.AnatomyID "
		") AS ProblemsQ",
		eprtEmrItem, eprtEmrDataItem,
		eprtEmrItem, eprtEmrDataItem, eprtEmrDiag, eprtEmrCharge, eprtEmrMedication, eprtEmrTopic, eprtEmrEMN, eprtEmrEMR, eprtLab,
		eprtEmrItem, eprtEmrDataItem, eprtEmrDiag, eprtEmrCharge, eprtEmrMedication, eprtEmrTopic, eprtEmrEMN, eprtEmrEMR, eprtLab, ltBiopsy,
		eprtEmrItem, eprtEmrDataItem, eprtEmrTopic,
		eprtEmrItem, eprtEmrDataItem, eprtEmrTopic, eprtEmrEMN, eprtEmrDiag, eprtEmrCharge, eprtEmrMedication, 
		eprtEmrItem, eprtEmrDataItem, eprtEmrTopic, eprtEmrEMN, eprtEmrDiag, eprtEmrCharge, eprtEmrMedication, 
		eprtEmrItem, eprtEmrDataItem, eprtEmrTopic, eprtEmrEMN, eprtEmrEMR, eprtEmrDiag, eprtEmrCharge, eprtEmrMedication,
		eprtEmrItem, eprtEmrDataItem, eprtEmrTopic, eprtEmrEMN, eprtEmrEMR, eprtEmrDiag, eprtEmrCharge, eprtEmrMedication,
		eprtLab);

}

// (c.haag 2008-07-24 09:40) - PLID 30826 - Returns TRUE if the currently logged in user can delete Emr problems
BOOL CanCurrentUserDeleteEmrProblems()
{
	if (IsCurrentUserAdministrator()) {
		return TRUE;
	}
	else if (CheckCurrentUserPermissions(bioEMRProblems, sptDelete, FALSE, 0, TRUE)) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

// (c.haag 2008-08-04 15:25) - PLID 30942 - Fill a problem array with problems that are bound to unspawning actions. The general
// plan is to go through the doomed object array, add all problems that correspond to object pointers contained in the array, and
// also add problems bound to the source object that correspond to the same action.
// (c.haag 2009-05-29 16:16) - PLID 34293 - Converted all references of problems to problem links
void FillUnspawnedProblemLinksArray(IN const CArray<CDoomedEmrObject, CDoomedEmrObject&>& aryAllDoomedObjects, 
								OUT CArray<CEmrProblemLink*,CEmrProblemLink*>& aryOutProblemLinks)
{
	// Do for all doomed objects
	aryOutProblemLinks.RemoveAll();
	for (int i=0; i < aryAllDoomedObjects.GetSize(); i++) {

		// Collect information for the current doomed object
		CDoomedEmrObject deo = aryAllDoomedObjects[i];

		// (a.walling 2010-03-31 15:47) - PLID 38008 - Skip if set
		if (deo.GetIsSkipped()) {
			continue;
		}

		CActionAndSource aas = deo.m_aas;
		// (j.jones 2013-07-15 10:51) - PLID 57243 - changed to a reference
		const EmrAction &ea = aas.ea;
		CEMNDetail* pSourceDetail = aas.sai.pSourceDetail;

		// Gather a list of problems for the doomed object. This includes not only problems bound
		// to the object in question, but also to its children (such as details for an EMR topic)
		CArray<CEmrProblemLink*,CEmrProblemLink*> aryProblemLinks;
		switch (ea.eaoDestType) {
		case eaoCpt: aryProblemLinks.Copy(deo.GetCharge()->m_apEmrProblemLinks); break;
		//case eaoDiag: aryProblemLinks.Copy(deo.GetDiagCode()->m_apEmrProblemLinks); break; // (b.savon 2014-07-14 11:08) - PLID 62706 - Deprecate the old Diag DestType and handle the changes Practice wide
		case eaoDiagnosis: aryProblemLinks.Copy(deo.GetDiagCode()->m_apEmrProblemLinks); break;
		case eaoMedication: aryProblemLinks.Copy(deo.GetMedication()->m_apEmrProblemLinks); break;
		case eaoMintItems: deo.GetTopic()->GetAllProblemLinks(aryProblemLinks); break;
		case eaoEmrItem: aryProblemLinks.Copy(deo.GetDetail()->m_apEmrProblemLinks); break;
		case eaoMint: deo.GetEMN()->GetAllProblemLinks(aryProblemLinks); break;
		default:
			// This type cannot have problems
			break;
		}

		// Now gather problems bound to the source object which spawned all the doomed objects. We
		// must be careful to only get the ones which correspond to the same EMR actions..
		switch (ea.eaoSourceType) {
			case eaoEmrDataItem:
			case eaoEmrItem:
			case eaoEmrImageHotSpot:
			case eaoProcedure:
			case eaoEmrTableDropDownItem: // (z.manning 2009-02-23 16:24) - PLID 33138
			case eaoSmartStamp: // (z.manning 2010-03-02 16:10) - PLID 37571
			case eaoWoundCareCodingCondition: // (r.gonet 08/03/2012) - PLID 51949
				{
					// Do for all problem actions corresponding to the EMR action
					// (a.walling 2014-07-01 15:28) - PLID 62697
					for (const EmrProblemAction& epa : ea.aProblemActions) {
						// Only handle cases where we spawn a problem to the source object. Destination 
						// object problem spawning is handled on a case-by-case basis in the RevokeEmrActions_ 
						// function family.
						if (epa.bSpawnToSourceItem) {
							if (ea.eaoSourceType == eaoEmrDataItem || ea.eaoSourceType == eaoEmrItem) {
								EMRProblemRegardingTypes eprt = (ea.eaoSourceType == eaoEmrDataItem) ? eprtEmrDataItem : eprtEmrItem;
								// Do for all problems in the source detail
								const int nProblemLinks = pSourceDetail->m_apEmrProblemLinks.GetSize();
								for (int j=0; j < nProblemLinks; j++) {
									CEmrProblemLink* pLink = pSourceDetail->m_apEmrProblemLinks[j];
									if (pLink->GetDetail() == pSourceDetail &&
										pLink->GetProblem()->m_nEmrProblemActionID == epa.nID &&
										pLink->GetType() == eprt &&
										!pLink->GetProblem()->m_bIsDeleted && !pLink->IsDeleted()) 
									{
										// If we get here, the problem must correspond to this action.
										aryProblemLinks.Add(pLink);
									}
								}
							} else {
								// We don't support binding problems to source actions where the source is
								// not an Emr Item or an Emr List Selection.
								ASSERT(FALSE);
							}
						} else {
							// This was already handled by the body of code above encapsulated in "switch (ea.eaoDestType) {"
						}
					}
				}
				break;
			default:
				ASSERT(FALSE); // Not supported
				break;
		}

		for (int j=0; j < aryProblemLinks.GetSize(); j++) {
			EnsureProblemLinkInArray(aryOutProblemLinks, aryProblemLinks[j]);
		}

	} // for (int i=0; i < aryAllDoomedObjects.GetSize(); i++) {

	// By the time we get here, aryOutProblems is filled with all the problems that will be removed by the collective unspawning
	// about to happen.
}

// (c.haag 2008-08-04 17:18) - PLID 30492 - Invoke a dialog that warns the user of the problems that will be deleted as a result of
// the collective unspawning to be done by this class.
// (c.haag 2009-05-29 16:28) - PLID 34398 - We now take in problem links
void WarnOfUnspawningProblemLinks(const CArray<CEmrProblemLink*,CEmrProblemLink*>& aryProblemLinks)
{

	// Quit if there are no links
	if (0 == aryProblemLinks.GetSize()) {
		return;
	}
	// We require a valid EMR to display a warning
	else if (NULL == aryProblemLinks[0]->GetEMR()) {
		return;
	}

	CEMR* pEMR = aryProblemLinks[0]->GetEMR();

	CEmrProblemDeleteDlg dlg(pEMR->GetInterface());
	dlg.m_strSecondCaption = "Please review this list, and then press the Close button to dismiss this window.";

	// Find all the problems related to aryProblemLinks that will be deleted if the user saves the EMR
	CArray<CEmrProblem*,CEmrProblem*> apProblems, apDoomedProblems;
	EnsureProblemsInArray(aryProblemLinks, apProblems); // Fill apProblems with all the problems in aryProblemLinks
	pEMR->FindEmrProblemsToDelete(apProblems, aryProblemLinks, apDoomedProblems, FALSE /* Include problems saved in data and not saved in data */);
	// Copy all problems and all problem links
	dlg.m_apProblemLinksToBeDeleted.Copy(aryProblemLinks);
	dlg.m_apProblemsToBeDeleted.Copy(apDoomedProblems);
	dlg.m_bCloseButtonOnly = TRUE;
	dlg.DoModal();
}

// (a.walling 2008-08-12 12:35) - PLID 30570 - Get description for auditing
CString GetPreviewFlagsDescriptionForAudit(DWORD nPreviewFlags)
{
	CString str;
	if (nPreviewFlags == 0)
		return "None";

	if (nPreviewFlags & epfHideTitle)
		str += "Hide Title, ";
	if (nPreviewFlags & epfHideItem)
		str += "Hide Item, ";

	// (j.armen 2013-01-02 16:38) - PLID 54412 - Added Hide Item on iPad
	if (nPreviewFlags & epfHideOnIPad)
		str += "Hide Item on iPad, ";

	// (a.walling 2009-01-09 09:13) - PLID 32669 - Add audit descriptions
	// for new preview pane flags (subdetail, floats/clear, and align text)
	if (nPreviewFlags & epfSubDetail)
		str += "Show Under Spawning Detail, ";

	// (a.walling 2012-07-13 16:38) - PLID 48896
	if (nPreviewFlags & epfHideIfIndirect)
		str += "Hide if Indirectl&y Included, ";

	// (a.walling 2009-07-06 10:14) - PLID 34793 - epfFloatLeft is now epfColumnOne, epfFloatRight is now epfColumnTwo
	if (nPreviewFlags & epfColumnOne)
		str += "Column One, ";
	if (nPreviewFlags & epfColumnTwo)
		str += "Column Two, ";

	if (nPreviewFlags & epfTextRight)
		str += "Align Text Right, ";

	str.TrimRight(", ");

	return str;
}

// (j.armen 2013-05-14 12:15) - PLID 56680 - The Write Token Info object has most of what we need
//	let's just inherit and add description
struct CWriteTokenInfo_CheckEMNConcurrency : CWriteTokenInfo
{
	CString strDescription;
};

// (a.walling 2008-08-27 13:21) - PLID 30855 - Generic function to get info on who is currently holding an EMN(s)
// (z.manning 2009-07-01 12:10) - PLID 34765 - Added a parameter to make it optional to exclude the current user
// when checking concurrency issues.
// (j.armen 2013-05-14 12:17) - PLID 56680 - Passing back a recordset is never a good idea
std::vector<CWriteTokenInfo_CheckEMNConcurrency> CheckEMNConcurrency(const CSqlFragment& sqlEMNQuery, BOOL bExcludeCurrentUser)
{
	if (sqlEMNQuery.IsEmpty()) {
		return std::vector<CWriteTokenInfo_CheckEMNConcurrency>();
	}

	// (j.armen 2013-05-14 12:18) - PLID 56680 - EMN Access Refactoring
	CSqlFragment sqlWriteStatusQuery(
		"SELECT\r\n"
		"	M.Deleted, A.Date, T.UserID, U.UserName, T.DeviceInfo, A.UserLoginTokenID, M.Description,\r\n"
		"	CASE WHEN DeviceType = -2 THEN CONVERT(BIT, 0) ELSE CONVERT(BIT, 1) END AS IsExternal\r\n"
		"FROM EMRMasterT M\r\n"
		"INNER JOIN EMNAccessT A WITH(UPDLOCK, HOLDLOCK) ON M.ID = A.EmnID\r\n"
		"INNER JOIN UserLoginTokensT T ON A.UserLoginTokenID = T.ID\r\n"
		"INNER JOIN UsersT U ON T.UserID = U.PersonID\r\n"
		"WHERE M.ID IN ({SQL})",
		sqlEMNQuery);
	
	std::vector<CWriteTokenInfo_CheckEMNConcurrency> aryWriteTokens;

	for(_RecordsetPtr prs = CreateParamRecordset(sqlWriteStatusQuery); !prs->eof; prs->MoveNext())
	{
		if(!bExcludeCurrentUser || AdoFldLong(prs, "UserLoginTokenID") != GetAPIUserLoginTokenID())
		{
			CWriteTokenInfo_CheckEMNConcurrency wtInfo;
			wtInfo.bIsDeleted = AdoFldBool(prs, "Deleted");
			wtInfo.dtHeld = AdoFldDateTime(prs, "Date");
			wtInfo.nHeldByUserID = AdoFldLong(prs, "UserID");
			wtInfo.strHeldByUserName = AdoFldString(prs, "UserName");
			wtInfo.strDeviceInfo = AdoFldString(prs, "DeviceInfo");
			wtInfo.bIsExternal = AdoFldBool(prs, "IsExternal");
			wtInfo.strDescription = AdoFldString(prs, "Description");
			aryWriteTokens.push_back(wtInfo);
		}
	}

	return aryWriteTokens;
}

// (a.walling 2008-08-27 13:21) - PLID 30855 - Generic function for warning if concurrency issues may exist for an EMN(s)
// (z.manning 2009-07-01 12:10) - PLID 34765 - Added a parameter to make it optional to exclude the current user
// when checking concurrency issues.
// (j.armen 2013-05-14 12:19) - PLID 56680 - Take in a Sql Fragment instead of a CString
BOOL WarnIfEMNConcurrencyIssuesExist(CWnd* pWnd, const CSqlFragment& sqlEMNQuery, const CString& strMessage, BOOL bExcludeCurrentUser /* = FALSE */)
{
	if (sqlEMNQuery.IsEmpty()) {
		return FALSE;
	}

	// (j.armen 2013-05-14 12:19) - PLID 56680 - We now get back an array of tokens instead of a recordset
	std::vector<CWriteTokenInfo_CheckEMNConcurrency> aryWriteTokens = CheckEMNConcurrency(sqlEMNQuery, bExcludeCurrentUser);

	if (aryWriteTokens.empty()) {
		// no one has this held, awesome.
		return FALSE;
	} else {
		CString strWarningMessage;
		const unsigned int cnMaxDisplay = 10;

		for(unsigned int i = 0; i < aryWriteTokens.size() && i < cnMaxDisplay; i++)
		{
			const CWriteTokenInfo_CheckEMNConcurrency& wtInfo = aryWriteTokens[i];

			if (wtInfo.bIsExternal)
				strWarningMessage += FormatString("The EMN '%s' is currently held for editing by the user '%s' %s at %s (using an external device, identified as: %s).\r\n\r\n", wtInfo.strDescription, wtInfo.strHeldByUserName, FormatDateTimeForInterface(wtInfo.dtHeld, NULL, dtoDateWithToday), FormatDateTimeForInterface(wtInfo.dtHeld, 0, dtoTime), wtInfo.strDeviceInfo);
			else
				strWarningMessage += FormatString("The EMN '%s' is currently held for editing by the user '%s' %s at %s (using workstation %s).\r\n\r\n", wtInfo.strDescription, wtInfo.strHeldByUserName, FormatDateTimeForInterface(wtInfo.dtHeld, NULL, dtoDateWithToday), FormatDateTimeForInterface(wtInfo.dtHeld, 0, dtoTime), wtInfo.strDeviceInfo);
		}

		if (aryWriteTokens.size() > cnMaxDisplay) {
			strWarningMessage += FormatString("\r\n(and %li other%s...)\r\n", aryWriteTokens.size() - cnMaxDisplay, aryWriteTokens.size() - cnMaxDisplay > 1 ? "s" : "");
		}

		if (!strMessage.IsEmpty()) {
			strWarningMessage += CString("\r\n") + strMessage;
		}

		if (pWnd->GetSafeHwnd() && ::IsWindow(pWnd->GetSafeHwnd())) {
			pWnd->MessageBox(strWarningMessage, NULL, MB_OK | MB_ICONEXCLAMATION);
		} else {
			AfxMessageBox(strWarningMessage, MB_OK | MB_ICONEXCLAMATION);
		}

		return TRUE;
	}
}

// (z.manning 2008-11-14 12:42) - PLID 32035 - Added a function to sort topics in order of
// parent, meaning parent topics will always come before their children.
void SortTopicsByParent(IN OUT CArray<EmrTopicSortInfo,EmrTopicSortInfo&> &aryTopicSortInfo, OUT CArray<long,long> &arynSortedTopicIDs)
{
	// (z.manning 2008-11-14 12:50) - PLID 32035 - We are going to loop through once per topic level
	// but add this as a safety check since there's no way to have this many levels of topics.
	const int nMaxIterations = 10000;
	int nIteration = 0;
	const int nTopicCount = aryTopicSortInfo.GetSize();

	arynSortedTopicIDs.RemoveAll();
	CArray<long,long> arynIDsToAdd;
	while(aryTopicSortInfo.GetSize() > 0)
	{
		nIteration++;
		if(nIteration >= nMaxIterations) {
			// (z.manning 2008-11-14 12:54) - PLID 32035 - Either something went wrong or we have an absurd
			// number of topic levels.
			ASSERT(FALSE);
			for(int nSortInfoIndex = 0; nSortInfoIndex < aryTopicSortInfo.GetSize(); nSortInfoIndex++) {
				EmrTopicSortInfo sortinfo = aryTopicSortInfo.GetAt(nSortInfoIndex);
				arynSortedTopicIDs.Add(sortinfo.nTopicID);
			}
			break;
		}

		// (z.manning 2008-11-14 13:41) - PLID 32035 - Ok, let's loop through all of the available topics
		// and gather all of the ones that are either a top-level parent or their parent was already 
		// collected on a previous iteration of this loop.
		for(int nSortInfoIndex = 0; nSortInfoIndex < aryTopicSortInfo.GetSize(); nSortInfoIndex++)
		{
			EmrTopicSortInfo sortinfo = aryTopicSortInfo.GetAt(nSortInfoIndex);
			long nParentID = VarLong(sortinfo.varParentTopicID, -1);
			if(nParentID == -1 || IsIDInArray(nParentID, arynSortedTopicIDs)) {
				// (z.manning 2008-11-14 13:42) - PLID 32035 - This topic either has no parent or its
				// parent is already in the main topic array, so it's safe to add this topic.
				// We then remove it from the sort info array for efficiency purposes.
				arynIDsToAdd.Add(sortinfo.nTopicID);
				aryTopicSortInfo.RemoveAt(nSortInfoIndex);
				nSortInfoIndex--;
			}
		}

		// (z.manning 2008-11-14 13:43) - PLID 32035 - Now go through all the topics we just added
		// and add them to the main array.
		for(int nAddIndex = 0; nAddIndex < arynIDsToAdd.GetSize(); nAddIndex++) {
			arynSortedTopicIDs.Add(arynIDsToAdd.GetAt(nAddIndex));
		}
		arynIDsToAdd.RemoveAll();
	}

	// (z.manning 2008-11-14 15:15) - PLID 32035 - We had better return as many topics as we were given.
	ASSERT(arynSortedTopicIDs.GetSize() == nTopicCount);
}

// (j.jones 2008-11-25 14:20) - PLID 28508 - UpdatePatientAllergyReviewStatus will update
// the given patient's AllergiesReviewedOn and AllergiesReviewedBy fields, and audit the change
// The return value is the date that was stamped into data, will be invalid if bReviewedAllergies = FALSE.
COleDateTime UpdatePatientAllergyReviewStatus(long nPatientID, BOOL bReviewedAllergies)
{
	//throw exceptions to the caller

	COleDateTime dtReviewedOn;
	dtReviewedOn.SetStatus(COleDateTime::invalid);

	if(bReviewedAllergies) {
		//save the review info
		_RecordsetPtr rs = CreateParamRecordset("SET NOCOUNT ON \r\n"
						"DECLARE @dtNow datetime \r\n"
						"SET @dtNow = GetDate() \r\n"
						"UPDATE PatientsT SET AllergiesReviewedOn = @dtNow, AllergiesReviewedBy = {INT} WHERE PersonID = {INT} \r\n"
						"SET NOCOUNT OFF \r\n"
						"SELECT @dtNow AS CurDate", GetCurrentUserID(), nPatientID);
		if(!rs->eof) {
			dtReviewedOn = AdoFldDateTime(rs, "CurDate");
		}
		else {
			ThrowNxException("Failed to update patient allergy review status!");
		}
		rs->Close();
	}
	else {
		//clear the review info
		ExecuteParamSql("UPDATE PatientsT SET AllergiesReviewedOn = NULL, AllergiesReviewedBy = NULL WHERE PersonID = {INT}", nPatientID);
	}

	//now audit this
	long nAuditID = BeginNewAuditEvent();
	if(bReviewedAllergies) {
		CString strNew;
		strNew.Format("Allergy information has been reviewed by %s on %s.", GetCurrentUserName(), FormatDateTimeForInterface(dtReviewedOn, DTF_STRIP_SECONDS, dtoDateTime));
		AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditID, aeiPatientAllergiesReviewed, nPatientID, "", strNew, aepMedium, aetChanged);
	}
	else {
		AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditID, aeiPatientAllergiesReviewed, nPatientID, "", "Allergy information has not been reviewed.", aepMedium, aetChanged);
	}

	//do not send a tablechecker, the caller should be responsible for that

	return dtReviewedOn;
}

// (z.manning 2009-02-24 09:10) - PLID 33138 - This function is equivilent to logic in many of
// the CEMN::GetEmrObjectsToRevoke_ functions so I moved here to reduce the duplicated code.
// (j.jones 2013-07-15 10:51) - PLID 57243 - changed the action to a reference
BOOL IsSourceActionInfoOk(SourceActionInfo &saiSourceAction, SourceActionInfo &saiDestObject, const EmrAction &ea, long nDestObjectID)
{
	// Compare on object ID, source action ID and source detail. Altogether, they should define a uniquely spawned charge.
	return IsSourceActionInfoOK_Essentials(saiSourceAction, saiDestObject, ea) && (nDestObjectID == ea.nDestID);
}

// (b.savon 2014-07-22 16:38) - PLID 62996 - Split this out
BOOL IsSourceActionInfoOK_Essentials(SourceActionInfo &saiSourceAction, SourceActionInfo &saiDestObject, const EmrAction &ea)
{
	// (z.manning 2009-02-24 10:59) - PLID 26579 - For table dropdown item based spawning we use
	// the source table element instead of the source detail since the exact same dropdown elments
	// (and thus dropdown actions) exist within the same detail.
	// (b.savon 2014-07-22 16:38) - PLID 62996 - Split this out
	BOOL bSourceDataGroupIDOK = IsSourceDataGroupIDOK(saiSourceAction, saiDestObject, ea);

	// (z.manning 2010-03-02 14:58) - PLID 37571 - Special handling for smart stamp actions
	// (b.savon 2014-07-22 16:38) - PLID 62996 - Split this out
	BOOL bSourceSmartStampOk = IsSourceSmartStampOK(saiSourceAction, saiDestObject, ea);

	// (b.savon 2014-07-22 16:38) - PLID 62996 - Split this out
	BOOL bSourceDetailOK = IsSourceDetailOK(saiSourceAction, saiDestObject);

	BOOL bSourceActionOK = (saiDestObject.nSourceActionID == ea.nID);

	return bSourceDataGroupIDOK && bSourceSmartStampOk && bSourceDetailOK && bSourceActionOK;
}

// (b.savon 2014-07-22 16:38) - PLID 62996 - Split this out
BOOL IsSourceDetailOK(SourceActionInfo &saiSourceAction, SourceActionInfo &saiDestObject)
{
	// Do a source detail equivalency test
	if (saiSourceAction.pSourceDetail != NULL) {
		//if we were given a source detail (and we should have), see if
		//the object we are comparing to has the same source detail
		if (saiDestObject.pSourceDetail != NULL) {
			if (saiSourceAction.pSourceDetail == saiDestObject.pSourceDetail) {
				return TRUE;
			}
		}
		//if the object doesn't have a source detail pointer, compare on source detail IDs
		if (saiDestObject.nSourceDetailID != -1) {
			if (saiDestObject.nSourceDetailID == saiSourceAction.nSourceDetailID) {
				return TRUE;
			}
		}
	}
	else {
		return TRUE;
	}

	return FALSE;
}

// (b.savon 2014-07-22 16:38) - PLID 62996 - Split this out
BOOL IsSourceSmartStampOK(SourceActionInfo &saiSourceAction, SourceActionInfo &saiDestObject, const EmrAction &ea)
{
	if (ea.eaoSourceType == eaoSmartStamp)
	{
		if (saiSourceAction.GetTableRow()->m_ID == saiDestObject.GetTableRow()->m_ID) {
			return TRUE;
		}
	}
	else {
		// (z.manning 2010-03-02 15:06) - PLID 37571 - This doesn't matter on non smart stamp spawning
		return TRUE;
	}

	return FALSE;
}

// (b.savon 2014-07-22 16:38) - PLID 62996 - Split this out
BOOL IsSourceDataGroupIDOK(SourceActionInfo &saiSourceAction, SourceActionInfo &saiDestObject, const EmrAction &ea)
{
	if (ea.eaoSourceType == eaoEmrTableDropDownItem)
	{
		if (saiSourceAction.TableSourceMatches(saiDestObject)) {
			return TRUE;
		}
	}
	else {
		// (z.manning 2009-02-26 11:52) - PLID 33141 - No other spawning type requires we compare
		// source data ID.
		return TRUE;
	}

	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////
// (z.manning 2009-02-18 12:04) - PLID 33141 - We used to use a source detail to help uniquely
// identify an action when the same info item (and thus same actions) existed on an EMN
// more than once. However, for table dropdown spawning a source detail isn't enough because
// the same dropdown elements exist in the same detail multiple times. To be able to 
// uniquely identify these actions we use a SourceDataGroupID as well. Since EMR action
// source stuff is now more complicated, I added this class to help keep things organized.
SourceActionInfo::SourceActionInfo()
{
	Init(NULL, NULL, NULL, NULL);
}

SourceActionInfo::SourceActionInfo(EmrAction *pAction, CEMNDetail *pSrcDetail)
{
	Init(pAction, pSrcDetail, NULL, NULL);
}

SourceActionInfo::SourceActionInfo(EmrAction *pAction, CEMNDetail *pSrcDetail, TableRow *pSrcTableRow)
{
	Init(pAction, pSrcDetail, pSrcTableRow, NULL);
}

//TES 2/16/2010 - PLID 37375 - Overload that takes a source HotSpot
SourceActionInfo::SourceActionInfo(EmrAction *pAction, CEMNDetail *pSrcDetail, CEMRHotSpot *pSrcSpot, TableRow *pSrcTableRow)
{
	Init(pAction, pSrcDetail, pSrcTableRow, pSrcSpot);
}

SourceActionInfo::SourceActionInfo(EmrActionObject eaoType, const long nActionID, CEMNDetail *pSrcDetail, TableRow *pSrcTableRow)
{
	nSourceActionID = nActionID;
	eaoSourceType = eaoType;
	nSourceDetailID = -1;
	if(pSrcTableRow != NULL) {
		ptrSourceTableRow = new TableRow(*pSrcTableRow);
	}
	else {
		ptrSourceTableRow = new TableRow;
	}
	pSourceDetail = pSrcDetail;
	if(pSrcDetail != NULL) {
		nSourceDetailID = pSrcDetail->m_bIsTemplateDetail ? pSrcDetail->m_nEMRTemplateDetailID : pSrcDetail->m_nEMRDetailID;
	}
	//TES 2/16/2010 - PLID 37375 - Added a variable for the source HotSpot
	pSourceSpot = NULL;
}

// (z.manning 2009-03-04 15:35) - PLID 33338 - Yet another constructor
SourceActionInfo::SourceActionInfo(EmrActionObject eaoType, const long nActionID, const long nDetailID, TableRow *pSrcTableRow)
{
	nSourceActionID = nActionID;
	eaoSourceType = eaoType;
	nSourceDetailID = nDetailID;
	if(pSrcTableRow != NULL) {
		ptrSourceTableRow = new TableRow(*pSrcTableRow);
	}
	else {
		ptrSourceTableRow = new TableRow;
	}
	pSourceDetail = NULL;
	//TES 2/16/2010 - PLID 37375 - Added a variable for the source HotSpot
	pSourceSpot = NULL;
}

//TES 2/16/2010 - PLID 37375 - Need an explicit copy constructor, to maintain our own copy of pSourceSpot
SourceActionInfo::SourceActionInfo(const SourceActionInfo &saiSource)
{
	nSourceActionID = saiSource.nSourceActionID;
	eaoSourceType = saiSource.eaoSourceType;
	nSourceDetailID = saiSource.nSourceDetailID;
	pSourceDetail = saiSource.pSourceDetail;
	if(saiSource.pSourceSpot) {
		pSourceSpot = new CEMRHotSpot(*(saiSource.pSourceSpot));
	}
	else {
		pSourceSpot = NULL;
	}

	// (z.manning 2010-02-24 16:31) - PLID 37532 - Handle table row pointer
	ptrSourceTableRow = new TableRow;
	*ptrSourceTableRow = *(saiSource.ptrSourceTableRow);
}

//TES 2/16/2010 - PLID 37375 - Need an explicit = operator, to maintain our own copy of pSourceSpot
void SourceActionInfo::operator =(const SourceActionInfo &saiSource)
{
	if(pSourceSpot) {
		delete pSourceSpot;
		pSourceSpot = NULL;
	}
	nSourceActionID = saiSource.nSourceActionID;
	eaoSourceType = saiSource.eaoSourceType;
	nSourceDetailID = saiSource.nSourceDetailID;
	pSourceDetail = saiSource.pSourceDetail;
	if(saiSource.pSourceSpot) {
		pSourceSpot = new CEMRHotSpot(*(saiSource.pSourceSpot));
	}
	else {
		pSourceSpot = NULL;
	}

	// (z.manning 2010-02-24 16:31) - PLID 37532 - Handle table row pointer
	*ptrSourceTableRow = *(saiSource.ptrSourceTableRow);
}

// (z.manning 2011-11-02 16:55) - PLID 45993 - This function used to be nearly identical to HasSameSource.
// I changed it to now only check source details, as its name implies.
BOOL SourceActionInfo::HasSameSourceDetail(const SourceActionInfo *psai)
{
	if(pSourceDetail != NULL && pSourceDetail == psai->pSourceDetail) {
		return TRUE;
	}
	if(nSourceDetailID != -1 && nSourceDetailID == psai->nSourceDetailID) {
		return TRUE;
	}

	return FALSE;
}

// (z.manning 2009-03-23 15:44) - PLID 33089
// (z.manning 2011-11-02 16:44) - PLID 45993 - Added bCheckActionID
BOOL SourceActionInfo::HasSameSource(const SourceActionInfo& sai, BOOL bCheckActionID /* = FALSE */)
{
	if(bCheckActionID) {
		if(nSourceActionID == -1 || nSourceActionID != sai.nSourceActionID) {
			return FALSE;
		}
	}

	// (z.manning 2012-07-18 13:58) - PLID 51553 - Procedure based actions don't have source details so skip this check for them.
	if(eaoSourceType != eaoProcedure) {
		if(!HasSameSourceDetail(&sai)) {
			return FALSE;
		}
	}

	if(eaoSourceType == eaoEmrTableDropDownItem) {
		if(!this->TableSourceMatches(sai)) {
			return FALSE;
		}
	}

	// (z.manning 2010-03-02 16:11) - PLID 37571
	if(eaoSourceType == eaoSmartStamp) {
		if(this->GetTableRow() == NULL || this->GetTableRow()->m_ID != sai.ptrSourceTableRow->m_ID) {
			return FALSE;
		}
	}

	return TRUE;
}

// (z.manning 2010-03-04 09:11) - PLID 37571
BOOL SourceActionInfo::TableSourceMatches(const SourceActionInfo& sai)
{
	if((this->GetDataGroupID() != -1 && this->GetDataGroupID() == sai.ptrSourceTableRow->nGroupID) ||
		TableRowMatches(sai))
	{
		return TRUE;
	}
	return FALSE;
}

// (a.walling 2010-04-05 16:04) - PLID 38060
BOOL SourceActionInfo::TableRowMatches(const SourceActionInfo& sai)
{
	if (this->GetTableRow() != NULL && this->GetTableRow()->m_ID == sai.ptrSourceTableRow->m_ID)
	{
		return TRUE;
	}
	return FALSE;
}

// (a.walling 2009-04-23 09:14) - PLID 28957
BOOL SourceActionInfo::IsBlank()
{
	//TES 3/17/2010 - PLID 37530 - Check the global StampID as well.
	return (nSourceActionID == -1 && eaoSourceType == eaoInvalid && nSourceDetailID == -1 &&
		ptrSourceTableRow->nGroupID == -1 && pSourceDetail == NULL && GetDetailStampID() == -1 &&
		GetDetailStampPointer() == NULL && GetStampID() == -1);
}

//TES 2/16/2010 - PLID 37375 - Added a parameter for the source HotSpot
void SourceActionInfo::Init(EmrAction *pAction, CEMNDetail *pSrcDetail, TableRow *pSrcTableRow, CEMRHotSpot *pSrcSpot)
{
	nSourceActionID = -1;
	eaoSourceType = eaoInvalid;
	nSourceDetailID = -1;
	pSourceDetail = pSrcDetail;

	//TES 2/16/2010 - PLID 37375 - Keep our own copy of the source HotSpot, so that nobody has to worry about where this specific
	// memory address is.
	if(pSrcSpot) {
		pSourceSpot = new CEMRHotSpot(*pSrcSpot);
	}
	else {
		pSourceSpot = NULL;
	}

	// (z.manning 2010-02-24 16:33) - PLID 37532
	if(pSrcTableRow) {
		ptrSourceTableRow = new TableRow(*pSrcTableRow);
	}
	else {
		ptrSourceTableRow = new TableRow;
	}
	

	if(pAction != NULL) {
		nSourceActionID = pAction->nID;
		eaoSourceType = pAction->eaoSourceType;
	}

		if(pSrcDetail != NULL) {
			BOOL bIsTemplate = pSrcDetail->m_bIsTemplateDetail;
			nSourceDetailID = bIsTemplate ? pSrcDetail->m_nEMRTemplateDetailID : pSrcDetail->m_nEMRDetailID;
		}
}

// (z.manning 2010-03-11 15:07) - PLID 37571
void SourceActionInfo::UpdateDetailStampPointerIfMatch(EmrDetailImageStamp *pDetailStampToMatch, EmrDetailImageStamp *pNewDetailStamp)
{
	if(pDetailStampToMatch != NULL &&
		(GetDetailStampPointer() == pDetailStampToMatch || (
		pDetailStampToMatch->nID != -1 && GetDetailStampID() == pDetailStampToMatch->nID)))
	{
		SetDetailStampPointer(pNewDetailStamp);
		SetDetailStampID(pNewDetailStamp->nID);
	}
}

// (z.manning 2010-12-08 11:56) - PLID 41731 - Returns true if the source action info refers to a table
// row that was created by a smart stamp.
BOOL SourceActionInfo::IsSmartStampTableRowAction()
{
	if(eaoSourceType == eaoEmrTableDropDownItem)
	{
		if(GetDetailStampPointer() != NULL || GetDetailStampID() != -1) {
			// (z.manning 2010-12-08 13:33) - PLID 41731 - A table row can be from EmrDataT or from a smart stamp, but not both.
			ASSERT(GetDataGroupID() == -1);
			return TRUE;
		}
	}

	return FALSE;
}


// (z.manning 2011-11-04 16:40) - PLID 42765
CSpawningSourceInfo::CSpawningSourceInfo(SourceActionInfo *psai, CUnspawningSource *pus)
	: m_psai(psai), m_pus(pus)
{
}

// (z.manning 2011-11-04 17:28) - PLID 42765
BOOL CSpawningSourceInfo::SpawnedDetail(CEMNDetail *pDetail)
{
	// (z.manning 2011-11-04 17:29) - PLID 42765 - Check and see if the given detail matches the source
	// action info as well as the group ID that did the spawning.
	if(m_psai->HasSameSource(pDetail->GetSourceActionInfo()) && m_pus->HasSameGroupID(pDetail)) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}


// (z.manning 2010-02-24 18:10) - PLID 37532
TableRow* SourceActionInfo::GetTableRow()
{
	return ptrSourceTableRow;
}

// (j.jones 2012-11-15 10:51) - PLID 52819 - added GetSourceDetailID, which is more reliable
// if our source detail pointer has an ID but somehow our action does not
long SourceActionInfo::GetSourceDetailID()
{
	//if we have a source detail pointer, use its ID
	if(pSourceDetail != NULL) {
		long nDetailID = -1;
		if(pSourceDetail->m_bIsTemplateDetail) {
			nDetailID = pSourceDetail->GetTemplateDetailID();			
		}
		else {
			nDetailID = pSourceDetail->GetID();
		}
		if(nDetailID != -1) {
			if(nSourceDetailID != -1) {
				//If we do have both IDs filled, they should match.
				//If they do not match, find out why not!
				ASSERT(nSourceDetailID == nDetailID);
			}
			//use the detail pointer's ID
			return nDetailID;
		}
		else {
			//use our source detail ID
			return nSourceDetailID;
		}
	}

	//return the source detail ID
	return nSourceDetailID;
}

// (z.manning 2010-02-24 18:10) - PLID 37532
long SourceActionInfo::GetDataGroupID()
{
	if(ptrSourceTableRow == NULL) {
		return -1;
	}
	return ptrSourceTableRow->nGroupID;
}

// (z.manning 2010-02-24 18:10) - PLID 37532
void SourceActionInfo::SetDataGroupID(const long nDataGroupID)
{
	if(ptrSourceTableRow == NULL) {
		ptrSourceTableRow = new TableRow;
	}
	ptrSourceTableRow->nGroupID = nDataGroupID;
}

// (z.manning 2010-02-24 18:10) - PLID 37532
long SourceActionInfo::GetDetailStampID()
{
	if(ptrSourceTableRow == NULL) {
		return -1;
	}
	// (c.haag 2012-10-26) - PLID 53440 - Use the new getter function
	return ptrSourceTableRow->m_ID.GetDetailImageStampID();
}

//TES 3/17/2010 - PLID 37530
long SourceActionInfo::GetStampID()
{
	if(ptrSourceTableRow == NULL) {
		return -1;
	}
	// (c.haag 2012-10-26) - PLID 53440 - Use the new getter function
	return ptrSourceTableRow->m_ID.GetImageStampID();
}

//TES 3/17/2010 - PLID 37530
long SourceActionInfo::GetStampIndexInDetailByType()
{
	if(ptrSourceTableRow == NULL) {
		return -1;
	}
	return ptrSourceTableRow->m_ID.nStampIndexInDetailByType;
}

//TES 3/17/2010 - PLID 37530
void SourceActionInfo::SetGlobalStampIDAndIndex(const long nStampID, const long nStampIndex)
{
	if(ptrSourceTableRow == NULL) {
		if(nStampID == -1 && nStampIndex == -1) {
			return;
		}
		ptrSourceTableRow = new TableRow;
	}
	// (c.haag 2012-10-26) - PLID 53440 - Use the new setter function
	ptrSourceTableRow->m_ID.SetImageStampID(nStampID);
	ptrSourceTableRow->m_ID.nStampIndexInDetailByType = nStampIndex;
}

// (z.manning 2010-02-24 18:10) - PLID 37532
void SourceActionInfo::SetDetailStampID(const long nDetailStampID)
{
	if(ptrSourceTableRow == NULL) {
		ptrSourceTableRow = new TableRow;
	}
	// (c.haag 2012-10-26) - PLID 53440 - Use the new setter function
	ptrSourceTableRow->m_ID.SetDetailImageStampID(nDetailStampID);
}

// (z.manning 2010-02-26 17:14) - PLID 37540
EmrDetailImageStamp* SourceActionInfo::GetDetailStampPointer()
{
	if(ptrSourceTableRow == NULL) {
		return NULL;
	}
	// (c.haag 2012-10-26) - PLID 53440 - Use the new getter function
	return ptrSourceTableRow->m_ID.GetDetailImageStampObject();
}

// (z.manning 2010-02-26 17:14) - PLID 37540
void SourceActionInfo::SetDetailStampPointer(EmrDetailImageStamp *pSourceDetailStamp)
{
	if(ptrSourceTableRow == NULL) {
		ptrSourceTableRow = new TableRow;
	}

	// (c.haag 2012-10-26) - PLID 53440 - SetDetailImageStamp does all the reference count updating;
	// there's no reason to be doing it here.
	ptrSourceTableRow->m_ID.SetDetailImageStamp(pSourceDetailStamp);
/*	if(ptrSourceTableRow->m_ID.pDetailImageStamp != NULL) {
		ptrSourceTableRow->m_ID.pDetailImageStamp->Release();
	}
	ptrSourceTableRow->m_ID.pDetailImageStamp = pSourceDetailStamp;
	if(ptrSourceTableRow->m_ID.pDetailImageStamp != NULL) {
		ptrSourceTableRow->m_ID.pDetailImageStamp->AddRef();
	}*/
}

//TES 2/16/2010 - PLID 37375 - Free up our copy of the source HotSpot
SourceActionInfo::~SourceActionInfo()
{
	if(pSourceSpot) {
		delete pSourceSpot;
		pSourceSpot = NULL;
	}
	// (z.manning 2010-02-24 16:25) - PLID 37532 - Also free up our copy of the table row pointer
	if(ptrSourceTableRow != NULL) {
		delete ptrSourceTableRow;
		ptrSourceTableRow = NULL;
	}
}

// (z.manning 2010-12-08 14:03) - PLID 41731 - Generates a name for the action based on the action's source type.
// Note: Up until now action names had always been loaded along with the topic because up until now we had always
// used admin-level data for action names.
CString SourceActionInfo::GenerateEmrActionName(CEMN *pParentEmn)
{
	// (z.manning 2010-12-08 16:30) - PLID 41731 - First check and see if this source action info has a source
	// detail pointer and if not then attempt to find it on the given EMN.
	CEMNDetail *pLocalSourceDetail = this->pSourceDetail;
	if(pLocalSourceDetail == NULL && pParentEmn != NULL && nSourceDetailID != -1) {
		pLocalSourceDetail = pParentEmn->GetDetailByID(nSourceDetailID);
	}

	CString strActionName;
	switch(eaoSourceType)
	{
		case eaoEmrTableDropDownItem:
			// (z.manning 2010-12-08 16:31) - PLID 41731 - There are 2 different types of EMR table rows at this point.
			// We have regular tables where the rows are stored in EmrDataT and we also have rows that may have been
			// created by a smart stamp on a table's linked smart stamp image. We handle those type of rows differently
			// when it comes to action names.
			if(IsSmartStampTableRowAction())
			{
				if(pLocalSourceDetail == NULL) {
					// (z.manning 2010-12-08 16:33) - PLID 41731 - Somehow we don't have a source detail so we won't
					// be able to get an action name.
					return "";
				}

				// (z.manning 2010-12-08 16:37) - PLID 41731 - At this time, smart stamp rows do not have row names but
				// the auto-number column text is the closest thing to that so let's try and get it.
				CString strAutoNumberColumnName;
				TableColumn *ptc = pLocalSourceDetail->GetColumnByListSubType(lstSmartStampAutoNumber);
				if(ptc != NULL) {
					// (a.walling 2011-08-24 12:35) - PLID 45171 - We only need to look for existing table elements
					TableElement te;
					if(pLocalSourceDetail->GetExistingTableElement(&GetTableRow()->m_ID, ptc->nID, te)) {
						strAutoNumberColumnName = te.GetValueAsString();
					}
				}

				// (z.manning 2010-12-08 16:38) - PLID 41731 - Now try and get the short name of the stamp that
				// spawned the smart stamp row.
				CString strStampName;
				EmrDetailImageStamp *pDetailStamp = GetDetailStampPointer();
				if(pDetailStamp == NULL && GetDetailStampID() != -1) {
					pDetailStamp = pLocalSourceDetail->GetDetailImageStampByID(GetDetailStampID());
				}
				if(pDetailStamp != NULL) {
					EMRImageStamp *pStamp = GetMainFrame()->GetEMRImageStampByID(pDetailStamp->nStampID);
					if(pStamp != NULL) {
						strStampName = pStamp->strStampText;
					}
				}

				CString strSeparator;
				if(!strStampName.IsEmpty() && !strAutoNumberColumnName.IsEmpty()) {
					// (z.manning 2010-12-08 16:38) - PLID 41731 - If both text values are not blank then put a hyphen between them.
					strSeparator = " - ";
				}

				strActionName = strAutoNumberColumnName + strSeparator + strStampName;
			}
			else {
				// (z.manning 2010-12-08 14:07) - This is where we would handle action names for regular table rows.
				// We do not yet support that here.
				ASSERT(FALSE);
			}
			break;

		default:
			// (z.manning 2010-12-08 14:06) - Either the source type is invalid or it's not yet supported.
			ASSERT(FALSE);
			break;
	}

	return strActionName;
}

// End Class SourceActionInfo
//////////////////////////////////////////////////////////////////////////////////////////////

// (c.haag 2009-05-13 13:04) - PLID 34249 - This utility function populates anEMRProblemIDs with the ID's of all
// problems that are not linked with a given EMR-related item. Returns FALSE if there are no problems for the patient,
// or TRUE if there are problems for this patient. Even when this returns TRUE, anEMRProblemIDs can still be
// empty if all the problems for the patient are already linked with the item.
BOOL GetEmrProblemsNotLinkedWithItem(long nPatientID, EMRProblemRegardingTypes RegardingType, long nRegardingID, 
									 OUT CArray<long,long>& anEMRProblemIDs)
{
	// Clear the output
	anEMRProblemIDs.RemoveAll();

	// (a.walling 2014-07-23 09:12) - PLID 63003 - Use CONST_INT for EMRProblemLinkT.EMRRegardingType enums
	_RecordsetPtr prsPtProblems = CreateParamRecordset(
		/* List of all problems for the patient */
		"SELECT ID FROM EMRProblemsT "
		"WHERE EMRProblemsT.PatientID = {INT} "
		"AND EMRProblemsT.Deleted = 0;\r\n "
		""
		/* List of problems for the patient not associated with the EMR-related item */
		"SELECT ID FROM EMRProblemsT WHERE Deleted = 0 AND PatientID = {INT} AND ID NOT IN ( "
		"	SELECT EMRProblemID FROM EMRProblemLinkT WHERE EMRRegardingType = {CONST_INT} AND EMRRegardingID = {INT} "
		") "
		,nPatientID
		,nPatientID, (long)RegardingType, nRegardingID);
	if (prsPtProblems->eof) {
		// No problems exist for this patient. Always return FALSE in this case.
		return FALSE;
	}

	// Go to the next recordset and get the ID's of all problems not associated with the patient
	prsPtProblems = prsPtProblems->NextRecordset(NULL);
	FieldsPtr f = prsPtProblems->Fields;
	while (!prsPtProblems->eof) {
		anEMRProblemIDs.Add( AdoFldLong(f, "ID") );
		prsPtProblems->MoveNext();
	}

	// Always return TRUE because at least one problem exists for this patient
	return TRUE;
}

// (z.manning 2009-05-27 09:32) - PLID 34297
CString GetProblemTypeDescription(const EMRProblemRegardingTypes eType)
{
	CString strOwnerType;
	switch (eType)
	{
		case eprtEmrItem:
			strOwnerType = "EMR Item";
			break;
		case eprtEmrDataItem:
			strOwnerType = "EMR List Option";
			break;
		case eprtEmrTopic:
			strOwnerType = "EMR Topic";
			break;
		case eprtEmrEMN:
			strOwnerType = "EMN";
			break;
		case eprtEmrEMR:
			strOwnerType = "EMR";			
			break;
		case eprtEmrDiag:
			// (r.gonet 03/07/2014) - PLID 61191 - Reworded to remove reference to ICD-9
			strOwnerType = "Diagnosis Code";
			break;
		case eprtEmrCharge:
			strOwnerType = "Service Code";
			break;
		case eprtEmrMedication:
			strOwnerType = "Medication";					
			break;
		case eprtUnassigned:
			strOwnerType = "Patient";
			break;
		case eprtLab: // (z.manning 2009-05-26 14:53) - PLID 34345
			strOwnerType = "Lab";
			break;
		default:
			ASSERT(FALSE); // This type is not supported
			break;
	}

	return strOwnerType;
}

// (c.haag 2009-05-22 15:34) - PLID 34298 - Given an EMR problem and the EMR, this
// function will find all EMR problem links for the same problem in memory in the 
// same CEMR object. If the problem has a valid ID, it will also search data for 
// all problem links not already included in memory, and add those to both arrays.
// (j.jones 2009-05-29 12:22) - PLID 34301 - moved to EmrUtils
//TES 11/25/2009 - PLID 36191 - Changed from a CLabEntryDlg to a CLabRequisitionDlg
void PopulateProblemLinkArrays(CEmrProblem* pProblem, CEMR* pEMR,
	CArray<CEmrProblemLink*,CEmrProblemLink*>& apAllLinks,
	CArray<CEmrProblemLink*,CEmrProblemLink*>& apLinksFromData,
	NXDATALIST2Lib::_DNxDataListPtr pDataList /*= NULL*/,
	short nListProblemLinkPtrCol /*= -1*/,
	short nListProblemLinkIDCol /*= -1*/,
	short nListDetailNameCol /*= -1*/,
	short nListDetailValueCol /*= -1*/,
	CLabRequisitionDlg *pdlgLabRequisition /*= NULL*/)
{
	CArray<CEmrProblemLink*,CEmrProblemLink*> apAllLinksWithDeleted;

	// Get all the problem links; even deleted ones, because we need to make sure the
	// list is wholly inclusive when we query data
	CWaitCursor wc;
	if(pdlgLabRequisition != NULL) {
		// (j.jones 2009-05-29 12:22) - PLID 34301 - find all the links in memory, including deleted ones
		pdlgLabRequisition->GetAllProblemLinks(apAllLinksWithDeleted, pProblem, TRUE);
	}
	else if(NULL != pEMR) {
		// If we get here, it means the problem is definitely in memory; and we're
		// pulling from a CEMR in memory, too. Traverse the EMR for problems.

		// (j.jones 2009-05-29 12:22) - PLID 34301 - find all the links in memory, including deleted ones
		pEMR->GetAllProblemLinks(apAllLinksWithDeleted, pProblem, TRUE);
	}

	// (j.jones 2009-05-29 12:22) - PLID 34301 - now add just the non-deleted ones to apAllLinks,
	// which is the array that's returned to the caller
	int i=0;
	for (i=0; i < apAllLinksWithDeleted.GetSize(); i++) {
		CEmrProblemLink* pLink = apAllLinksWithDeleted[i];
		if(!pLink->GetIsDeleted()) {
			//apAllLinks will only track the non-deleted links
			apAllLinks.Add(pLink);
		}
	}

	// Now go through all the links in memory, and try to assign the detail name and value for each.
	for (i=0; i < apAllLinks.GetSize(); i++) {
		CEmrProblemLink* pLink = apAllLinks[i];
		if (!pLink->IsDeleted()) {
			NXDATALIST2Lib::IRowSettingsPtr pLinkRow = NULL;
			
			//were we passed in a datalist?
			if(pDataList != NULL && nListProblemLinkPtrCol != -1 && nListProblemLinkIDCol != -1) {
				// First, search by pointer
				if (NULL == (pLinkRow = pDataList->FindByColumn(nListProblemLinkPtrCol, (long)pLink, pDataList->GetFirstRow(), g_cvarFalse))) {
					// If we get here, we failed. Try again by ID.
					if (-1 != pLink->GetID()) {
						pLinkRow = pDataList->FindByColumn(nListProblemLinkIDCol, pLink->GetID(), pDataList->GetFirstRow(), g_cvarFalse);
					}
				}
			}

			if (NULL != pLinkRow) {
				// If we have a valid row, assign the detail name and value
				pLink->m_strDetailValue = VarString(pLinkRow->GetValue(nListDetailValueCol), "");
				pLink->m_strDetailName = VarString(pLinkRow->GetValue(nListDetailNameCol), "");
			}
			else {
				// If we don't have a valid row, get the name and value from memory
				CString strTopicName, strEMNName, strEMRName; //unused, but required parameters
				GetDetailNameAndValue(pLink, pLink->m_strDetailName, pLink->m_strDetailValue, strTopicName, strEMNName, strEMRName);
			}
		} // if (!pLink->IsDeleted()) {
	} // for (int i=0; i < apAllLinks.GetSize(); i++)

	if (-1 != pProblem->m_nID)
	{
		// Now look in data for problem links. 
		_RecordsetPtr prs = CreateParamRecordset(
			"SELECT * FROM " + GetEmrProblemListFromClause() + " WHERE ProblemsQ.ID = {INT} AND ProblemsQ.Deleted = 0",
			pProblem->m_nID);
		FieldsPtr f = prs->Fields;
		while (!prs->eof) {
			long nEMRProblemLinkID = AdoFldLong(f, "EMRProblemLinkID");
			BOOL bFound = FALSE;
			// (j.jones 2009-05-29 12:22) - PLID 34301 - compare to apAllLinksWithDeleted, not apAllLinks,
			// because we don't want to re-add from data a link that we are about to delete
			// (it is marked deleted in memory, just not saved yet)
			for (int i=0; i < apAllLinksWithDeleted.GetSize() && !bFound; i++) {
				if (apAllLinksWithDeleted[i]->GetID() == nEMRProblemLinkID) {
					bFound = TRUE;
				}
			}
			if (!bFound) {
				CEmrProblemLink* pNewLink = new CEmrProblemLink(
					pProblem,
					nEMRProblemLinkID,
					(EMRProblemRegardingTypes)AdoFldLong(f, "EmrRegardingType"),
					AdoFldLong(f, "EmrRegardingID"),
					AdoFldLong(f, "EmrDataID", -1));
				pNewLink->m_strDetailName = AdoFldString(f, "DetailName", "");
				pNewLink->m_strDetailValue = AdoFldString(f, "DetailValue", "");
				// Try to fill in the name and values from the datalist first. The primary motivation
				// for this is that to get the name and value of a narrative or table detail, we have to
				// actually open an EMN and parse out those values, which may have already been done in 
				// the list. The second motivation is, as a general rule, we'd like to pull information 
				// from memory before data where possible.
				NXDATALIST2Lib::IRowSettingsPtr pLinkRow;
				if (NULL != pDataList && NULL != (pLinkRow = pDataList->FindByColumn(nListProblemLinkIDCol, (long)nEMRProblemLinkID, pDataList->GetFirstRow(), g_cvarFalse))) {
					pNewLink->m_strDetailName = VarString(pLinkRow->GetValue(nListDetailNameCol));
					pNewLink->m_strDetailValue = VarString(pLinkRow->GetValue(nListDetailValueCol));
				} else {
					pNewLink->m_strDetailName = AdoFldString(f, "DetailName", "");
					pNewLink->m_strDetailValue = AdoFldString(f, "DetailValue", "");
				}
				apAllLinks.Add(pNewLink);
				apLinksFromData.Add(pNewLink);
			}
			prs->MoveNext();

		} // while (!prs->eof)
	} // if (-1 != pProblem->m_nID)
}

// (c.haag 2009-05-28 12:23) - PLID 34298 - This function will get the detail name and values given a problem link object
// (j.jones 2009-05-29 12:22) - PLID 34301 - moved to EmrUtils
void GetDetailNameAndValue(CEmrProblemLink* pLink, CString& strDetailName, CString& strDetailValue, CString& strTopicName, CString& strEMNName, CString& strEMRName)
{
	if(pLink->GetDetail()) {
		CEMNDetail* pDetail = pLink->GetDetail();
		if(pDetail->GetStateVarType() == VT_NULL || pDetail->GetStateVarType() == VT_BSTR && VarString(pDetail->GetState()).IsEmpty()) {
			//TES 2/26/2010 - PLID 37463 - Check whether to use the "Smart Stamps" long form
			// (z.manning 2010-07-26 15:25) - PLID 39848 - All tables now use the same long form
			strDetailValue = pDetail->m_strLongForm;
		} else {
			CStringArray saDummy;
			// (c.haag 2006-11-14 10:49) - PLID 23543 - If the info type is an image, we may get a debug assertion
			// failure when calling GetSentence. Rather than try to get a sentence, just return a sentinel value.
			if (eitImage == pDetail->m_EMRInfoType) {
				strDetailValue = "<image>";
			} else {
				// (c.haag 2008-02-22 13:53) - PLID 29064 - GetSentence may access the database when doing calculations on
				// dropdown table columns. Make sure we pass in our connection object so it won't try to use the global one
				// which belongs to the main thread.
				strDetailValue = ::GetSentence(pDetail, NULL, false, false, saDummy, ecfParagraph, NULL, NULL, NULL);
			}
		}
		strDetailName = pDetail->GetMergeFieldOverride().IsEmpty() ? pDetail->GetLabelText() : pDetail->GetMergeFieldOverride();
	}
	else if(pLink->GetDiagCode()) {
		strDetailValue = "<Diagnosis Code>";

		// (j.jones 2014-03-24 09:42) - PLID 61505 - if a problem is linked to a diagnosis, show the 9 and 10 code if both exist
		EMNDiagCode *pCode = pLink->GetDiagCode();
		if(pCode->nDiagCodeID != -1 && pCode->nDiagCodeID_ICD10 != -1) {
			strDetailName = pCode->strCode_ICD10 + " - " + pCode->strCodeDesc_ICD10 + " (" + pCode->strCode + " - " + pCode->strCodeDesc + ")";
		}
		else if(pCode->nDiagCodeID_ICD10 != -1) {
			strDetailName = pCode->strCode_ICD10 + " - " + pCode->strCodeDesc_ICD10;
		}
		else if(pCode->nDiagCodeID != -1) {
			strDetailName = pCode->strCode + " - " + pCode->strCodeDesc;
		}
		else {
			//this should be impossible
			ASSERT(FALSE);
			strDetailName = "<Unknown Diagnosis Code>";
		}
	}
	else if(pLink->GetCharge()) {
		strDetailValue = "<Charge>";
		strDetailName = pLink->GetCharge()->strDescription;
	}
	else if(pLink->GetMedication()) {
		strDetailValue = "<Medication>";
		strDetailName = pLink->GetMedication()->m_strDrugName;
	}

	if(pLink->GetTopic()) {
		strTopicName = pLink->GetTopic()->GetName();
	}
	if(pLink->GetEMN()) {
		strEMNName = pLink->GetEMN()->GetDescription();
	}
	if(pLink->GetEMR()) {
		strEMRName = pLink->GetEMR()->GetDescription();
	}

	if(pLink->GetType() == eprtEmrTopic) {
		strDetailValue = "<Topic>";
		strDetailName = strTopicName;
	}
	else if(pLink->GetType() == eprtEmrEMN) {
		strDetailValue = "<EMN>";
		strDetailName = strEMNName;
	}
	else if(pLink->GetType() == eprtEmrEMR) {
		strDetailValue = "<EMR>";
		strDetailName = strEMRName;
	}
	// (z.manning 2009-05-28 09:35) - PLID 34345
	else if(pLink->GetType() == eprtLab) {
		strDetailValue = "<Lab>";
		strDetailName = pLink->GetLabText();
	}
}

// (j.jones 2008-08-07 11:35) - PLID 30773 - this function will update the EMR appropriately
// when a problem memory object has been changed
// (c.haag 2009-05-26 15:51) - PLID 34298 - We now pass in problem links. We no longer need the EMN lock flag;
// we calculate it in the function now
// (j.jones 2009-06-05 09:16) - PLID 34487 - moved from EMRProblemListDlg
void UpdateEMRInterface(CWnd *pMessageWnd, CEmrProblemLink *pLink)
{
	try {

		//this function is called when a problem is changed (or deleted),
		//and does not modify the problem pointer, it only updates the
		//interface to reflect if an object was changed, or just updating
		//icons, taking the lock status into account
		CEMNDetail* pDetail = pLink->GetDetail();
		CEMRTopic* pTopic = pLink->GetTopic();
		CEMN* pEMN = pLink->GetEMN();
		EMNDiagCode* pDiagCode = pLink->GetDiagCode();
		EMNCharge* pCharge = pLink->GetCharge();
		EMNMedication* pMedication = pLink->GetMedication();
		CEMR* pEMR = pLink->GetEMR();

		// (c.haag 2009-05-26 15:52) - PLID 34298 - Determine whether the EMN is locked
		BOOL bEMNIsLocked = FALSE;
		if ((pEMN != NULL && pEMN->GetStatus() == 2)) {
			bEMNIsLocked = TRUE;
		} 

		if(pDetail != NULL) {

			if(!bEMNIsLocked) {
				pDetail->SetUnsaved();
				pDetail->m_pParentTopic->SetUnsaved();
			}

			if(pDetail->m_pEmrItemAdvDlg != NULL
				&& pDetail->m_pEmrItemAdvDlg->GetSafeHwnd()) {

				pDetail->m_pEmrItemAdvDlg->UpdateProblemStatusButtonAppearance();
			}

			if(pMessageWnd) {
				pMessageWnd->SendMessage(NXM_EMR_ITEM_CHANGED, (WPARAM)pDetail);								
			}
			else {
				//if pMessageWnd is NULL, find out why not,
				//and determine if there is a valid reason for it
				ASSERT(FALSE);
			}
		}
		//ensure we update if we changed a topic-level object
		else if(pTopic != NULL) {

			if(!bEMNIsLocked) {
				pTopic->SetUnsaved();
			}

			if(pMessageWnd) {

				pMessageWnd->SendMessage(NXM_EMR_TOPIC_CHANGED, (WPARAM)pTopic);
			}
			else {
				//if pMessageWnd is NULL, find out why not,
				//and determine if there is a valid reason for it
				ASSERT(FALSE);
			}
		}
		//ensure we update if we changed an EMN-level object
		else if(pEMN != NULL) {

			if(!bEMNIsLocked) {
				// (r.farnworth 2014-02-26 09:59) - PLID 60746 - Need to make sure we're only marking Codes unsaved here.
				pEMN->SetVariableOnlyUnsaved();
				//problems inside a more info don't update the icon,
				//so don't mark it as being changed if locked
				// (a.walling 2012-03-22 16:50) - PLID 49141 - notify the interface
				if (pMedication) {
					pEMN->SetMoreInfoUnsaved();
				}
				//TES 2/21/2014 - PLID 60972 - Diagnosis Codes and Charges are on the Codes topic now
				else if(pDiagCode || pCharge) {					
					pEMN->SetCodesUnsaved();
				}
			}

			if(pMessageWnd) {

				//if a diag, charge, or medication, update the more info, else just the EMN
				if(pDiagCode) {
					pMessageWnd->SendMessage(NXM_EMN_REFRESH_DIAG_CODES, (WPARAM)pEMN);
				}
				else if(pCharge) {
					pMessageWnd->SendMessage(NXM_EMN_REFRESH_CHARGES, (WPARAM)pEMN);
				}
				else if(pMedication) {
					pMessageWnd->SendMessage(NXM_EMN_REFRESH_PRESCRIPTIONS, (WPARAM)pEMN);
				}
				else {
					pMessageWnd->SendMessage(NXM_EMN_CHANGED, (WPARAM)pEMN);
				}
			}
			else {
				//if pMessageWnd is NULL, find out why not,
				//and determine if there is a valid reason for it
				ASSERT(FALSE);
			}
		}
		//if an EMR-level object changed, just change the saved status,
		//because the only message that needs sent is NXM_EMR_PROBLEM_CHANGED,
		//which is sent next
		// (j.jones 2009-05-29 08:58) - PLID 34301 - always mark the EMR as unsaved,
		// because it is responsible for saving all problems
		if(pEMR != NULL) {

			pEMR->SetUnsaved();
		}

		if(pMessageWnd) {
			pMessageWnd->SendMessage(NXM_EMR_PROBLEM_CHANGED);
		}
		else {
			//if m_pMessageWnd is NULL, find out why not,
			//and determine if there is a valid reason for it
			ASSERT(FALSE);
		}

	}NxCatchAll("Error in UpdateEMRInterface");
}

//TES 9/16/2009 - PLID 35495 - We now store 2 arrays, one for the 24x24 icons and one for the 32x32icons
//TES 9/9/2009 - PLID 35495 - Moved here from CEmrTreeWnd
HICON g_IconsSmall[EMR_TREE_ICON_COUNT];
HICON g_IconsLarge[EMR_TREE_ICON_COUNT];
//TES 9/9/2009 - PLID 35495 - Need to remember if we've initialized the (now) global variable.
BOOL g_bIconsSmallInitialized = FALSE, g_bIconsLargeInitialized = FALSE;

//TES 9/9/2009 - PLID 35495 - Moved here from CEmrTreeWnd::GetIcon()
//TES 9/16/2009 - PLID 35495 - This now takes a single boolean parameter, for whether we're using the 24x24 or 32x32 icons.
HICON GetEmrTreeIcon(EmrTreeIcon eti, bool bUseSmallIcons)
{
	//TES 9/9/2009 - PLID 35495 - If we haven't yet initialized the global array, do it.
	if(bUseSmallIcons) {
		if(!g_bIconsSmallInitialized) {
			::ZeroMemory(g_IconsSmall, sizeof(HICON) * EMR_TREE_ICON_COUNT);
			g_bIconsSmallInitialized = TRUE;
		}
	}
	else {
		if(!g_bIconsLargeInitialized) {
			::ZeroMemory(g_IconsLarge, sizeof(HICON) * EMR_TREE_ICON_COUNT);
			g_bIconsLargeInitialized = TRUE;
		}
	}
	// (a.walling 2008-04-29 09:19) - PLID 29815 - Support new icons and array of handles.
	WORD nResID = 0;

	switch(eti) {
	// (c.haag 2006-02-09 11:49) - PLID 19230 - EMN's and topics no longer
	// share the same icons. I'm keeping the folder icons as is because we
	// could use them elsewhere in the future.
	case etiOpenSavedTopic:
		nResID = IDI_TOPIC_OPEN;
		break;
	case etiOpenUnsavedTopic:
		nResID = IDI_TOPIC_OPEN_MOD;
		break;
	case etiClosedSavedTopic:
		nResID = IDI_TOPIC_CLOSED;
		break;
	case etiClosedUnsavedTopic:
		nResID = IDI_TOPIC_CLOSED_MOD;
		break;
	case etiLockedEmn:
		nResID = IDI_EMN_LOCKED;
		break;
	// (c.haag 2006-02-09 11:49) - PLID 19230 - Unless an EMN is locked,
	// we use a clipboard icon for it in the tree
	case etiOpenSavedEmn:
	case etiFinishedSavedEmn:
		nResID = IDI_EMN;
		break;
	case etiOpenUnsavedEmn:
	case etiFinishedUnsavedEmn:
		nResID = IDI_EMN_MOD;
		break;

	case etiReadOnlyEmn:
		nResID = IDI_EMN_RO;
		break;

	case etiReadOnlyUnsavedEmn:
		nResID = IDI_EMN_RO_MOD;
		break;

// (a.walling 2008-07-22 17:37) - PLID 30790 - new icons for EMNs with problems
	case etiLockedEmnProblems:
		nResID = IDI_EMN_LOCKED_PROBLEMS;
		break;
	case etiOpenSavedEmnProblems:
	case etiFinishedSavedEmnProblems:
		nResID = IDI_EMN_PROBLEMS;
		break;
	case etiOpenUnsavedEmnProblems:
	case etiFinishedUnsavedEmnProblems:
		nResID = IDI_EMN_MOD_PROBLEMS;
		break;

	case etiReadOnlyEmnProblems:
		nResID = IDI_EMN_RO_PROBLEMS;
		break;

	case etiReadOnlyUnsavedEmnProblems:
		nResID = IDI_EMN_RO_MOD_PROBLEMS;
		break;

	
	case etiLockedEmnClosedProblems:
		nResID = IDI_EMN_LOCKED_CLOSEDPROBLEMS;
		break;
	case etiOpenSavedEmnClosedProblems:
	case etiFinishedSavedEmnClosedProblems:
		nResID = IDI_EMN_CLOSEDPROBLEMS;
		break;
	case etiOpenUnsavedEmnClosedProblems:
	case etiFinishedUnsavedEmnClosedProblems:
		nResID = IDI_EMN_MOD_CLOSEDPROBLEMS;
		break;

	case etiReadOnlyEmnClosedProblems:
		nResID = IDI_EMN_RO_CLOSEDPROBLEMS;
		break;

	case etiReadOnlyUnsavedEmnClosedProblems:
		nResID = IDI_EMN_RO_MOD_CLOSEDPROBLEMS;
		break;

	case etiOpenSavedMoreInfo:
		nResID = IDI_MOREINFO_OPEN;
		break;
	case etiOpenUnsavedMoreInfo:
		nResID = IDI_MOREINFO_OPEN_MOD;
		break;
	case etiClosedSavedMoreInfo:
		nResID = IDI_MOREINFO_CLOSED;
		break;
	case etiClosedUnsavedMoreInfo:
		nResID = IDI_MOREINFO_CLOSED_MOD;
		break;

	//TES 2/12/2014 - PLID 60748 - Icons for the new Codes topic
	case etiOpenSavedCodes:
		nResID = IDI_CODES_OPEN;
		break;
	case etiOpenUnsavedCodes:
		nResID = IDI_CODES_OPEN_MOD;
		break;
	case etiClosedSavedCodes:
		nResID = IDI_CODES_CLOSED;
		break;
	case etiClosedUnsavedCodes:
		nResID = IDI_CODES_CLOSED_MOD;
		break;

	// (a.walling 2008-07-21 17:53) - PLID 30790 - New icons for topics with problems
	case etiOpenSavedTopicProblems:
		nResID = IDI_TOPIC_OPEN_PROBLEMS;
		break;
	case etiClosedSavedTopicProblems:
		nResID = IDI_TOPIC_CLOSED_PROBLEMS;
		break;
	case etiOpenUnsavedTopicProblems:
		nResID = IDI_TOPIC_OPEN_MOD_PROBLEMS;
		break;
	case etiClosedUnsavedTopicProblems:
		nResID = IDI_TOPIC_CLOSED_MOD_PROBLEMS;
		break;
	
	case etiOpenSavedTopicClosedProblems:
		nResID = IDI_TOPIC_OPEN_CLOSEDPROBLEMS;
		break;
	case etiClosedSavedTopicClosedProblems:
		nResID = IDI_TOPIC_CLOSED_CLOSEDPROBLEMS;
		break;
	case etiOpenUnsavedTopicClosedProblems:
		nResID = IDI_TOPIC_OPEN_MOD_CLOSEDPROBLEMS;
		break;
	case etiClosedUnsavedTopicClosedProblems:
		nResID = IDI_TOPIC_CLOSED_MOD_CLOSEDPROBLEMS;
		break;

	// (a.walling 2008-08-22 17:32) - PLID 23138 - New icons for locked readonly status
	case etiReadOnlyLockedEmnClosedProblems:
		nResID = IDI_EMN_LOCKED_RO_CLOSEDPROBLEMS;
		break;
	case etiReadOnlyLockedEmnProblems:
		nResID = IDI_EMN_LOCKED_RO_PROBLEMS;
		break;
	case etiReadOnlyLockedEmn:
		nResID = IDI_EMN_LOCKED_RO;
		break;
		
	default:
		ASSERT(FALSE);
		return NULL;
		break;
	}

	ASSERT(nResID != 0);

	if(bUseSmallIcons) {
		if (g_IconsSmall[eti] == 0) {
			g_IconsSmall[eti] = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(nResID), IMAGE_ICON, 24, 24, LR_SHARED);
		}

		return g_IconsSmall[eti];
	}
	else {
		if (g_IconsLarge[eti] == 0) {
			g_IconsLarge[eti] = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(nResID), IMAGE_ICON, 32, 32, LR_SHARED);
		}

		return g_IconsLarge[eti];
	}

}

CWnd*& GetHiddenDetailParentWndPtr()
{
	static CWnd* pHiddenParentWnd = NULL;
	return pHiddenParentWnd;
}

// (a.walling 2012-02-22 14:53) - PLID 48320 - Global hidden parent window for controls
CWnd& GetHiddenDetailParentWnd()
{
	CWnd*& pHiddenParentWnd = GetHiddenDetailParentWndPtr();
	if (!pHiddenParentWnd->GetSafeHwnd()) {
		if (!pHiddenParentWnd) {
			pHiddenParentWnd = new CWnd;
		}
		pHiddenParentWnd->CreateEx(WS_EX_CONTROLPARENT | WS_EX_NOPARENTNOTIFY | WS_EX_NOACTIVATE, AfxRegisterWndClass(0), "Hidden Parent", WS_POPUP, CRect(0,0,320,320), NULL, 0);
	}

	return *pHiddenParentWnd;
}

//TES 9/16/2009 - PLID 35529 - A single CEmrItemAdvNarrativeDlg that all narratives which are being loaded in the
// background can share.
// (a.walling 2009-12-08 15:22) - PLID 36225 - We now have two global CEmrItemAdvNarrativeDlgs, one for narrative loading, and
// the original one for windowless accessibility via the analysis and etc
CEmrItemAdvNarrativeDlg *g_paryEmrItemAdvNarrativeDlg[egiCount] = {NULL, NULL};
//TES 9/16/2009 - PLID 35529 - Returns a pointer to a shareable, global, CEmrItemAdvNarrativeDlg for background loading.
// If pDetail is not NULL, this function will create the dialog if necessary, and point it to the given detail.
// If pDetail is NULL, this will be treated as a simple accessor.
CEmrItemAdvNarrativeDlg* GetGlobalEmrItemAdvNarrativeDlg(CEMNDetail *pDetail, EGlobalNarrativeIndex eGlobalIndex /* = egiWindowless*/)
{
	// (a.walling 2009-12-08 15:22) - PLID 36225 - We now have two global CEmrItemAdvNarrativeDlgs
	if (eGlobalIndex >= egiCount || eGlobalIndex < egiWindowless) {
		ThrowNxException("Invalid global narrative index");
	}
	// Need to use a hidden parent window to prevent activation issues
	if(pDetail == NULL) {
		//TES 9/16/2009 - PLID 35529 - If the detail is NULL, just treat as an accessor
		return g_paryEmrItemAdvNarrativeDlg[eGlobalIndex];
	}
	else {
		if(g_paryEmrItemAdvNarrativeDlg[eGlobalIndex] == NULL) {
			//TES 9/16/2009 - PLID 35529 - Create it.
			g_paryEmrItemAdvNarrativeDlg[eGlobalIndex] = new CEmrItemAdvNarrativeDlg(pDetail);
			// (a.walling 2012-03-05 15:56) - PLID 46075 - Tell it not to auto-delete
			g_paryEmrItemAdvNarrativeDlg[eGlobalIndex]->SetAutoDelete(false);
			
			g_paryEmrItemAdvNarrativeDlg[eGlobalIndex]->SetIsTemplate(pDetail->m_bIsTemplateDetail);
			ASSERT(g_paryEmrItemAdvNarrativeDlg[eGlobalIndex]);
			if (!g_paryEmrItemAdvNarrativeDlg[eGlobalIndex]) {
				ThrowNxException("EMRUtils: Could not create global emr item advanced narrative dialog!");
			}
			
			//TES 9/16/2009 - PLID 35529 - Fill the data values
			pDetail->LoadContent(FALSE);
			//TES 9/16/2009 - PLID 35529 - Create the window
			// Need to use a hidden parent window to prevent activation issues
			// (a.walling 2012-02-22 14:53) - PLID 48320 - Global hidden parent window for controls
			g_paryEmrItemAdvNarrativeDlg[eGlobalIndex]->CreateWithClientArea(0, 0, CRect(0, 0, 320, 320), &GetHiddenDetailParentWnd());
			g_paryEmrItemAdvNarrativeDlg[eGlobalIndex]->ReflectCurrentState();
		}
		else {
			//TES 9/16/2009 - PLID 35529 - Just set the global dialog to point to the detail we were given.
			g_paryEmrItemAdvNarrativeDlg[eGlobalIndex]->SetDetail(pDetail);
		}
		return g_paryEmrItemAdvNarrativeDlg[eGlobalIndex];
	}
}

//TES 9/16/2009 - PLID 35529 - Destroys the global CEmrItemAdvNarrativeDlg
// (a.walling 2009-12-08 15:22) - PLID 36225 - Destroys all global CEmrItemAdvNarrativeDlgs
void EnsureNotGlobalEmrItemAdvNarrativeDlg()
{
	// (a.walling 2009-12-08 15:22) - PLID 36225 - We now have two global CEmrItemAdvNarrativeDlgs
	for (int i = 0; i < egiCount; i++) {
		if(g_paryEmrItemAdvNarrativeDlg[i]) {
			if(IsWindow(g_paryEmrItemAdvNarrativeDlg[i]->GetSafeHwnd())) {
				g_paryEmrItemAdvNarrativeDlg[i]->DestroyWindow();
			}
			delete g_paryEmrItemAdvNarrativeDlg[i];
			g_paryEmrItemAdvNarrativeDlg[i] = NULL;
		}
	}

	// (a.walling 2012-02-22 14:53) - PLID 48320 - Go ahead and destroy the hidden parent
	CWnd*& pHiddenDetailParentWndPtr = GetHiddenDetailParentWndPtr();
	if (pHiddenDetailParentWndPtr) {
		if (pHiddenDetailParentWndPtr->GetSafeHwnd()) {
			pHiddenDetailParentWndPtr->DestroyWindow();
		}

		delete pHiddenDetailParentWndPtr;
		pHiddenDetailParentWndPtr = NULL;
	}
}

//TES 9/2/2011 - PLID 37633 - We need a way that will ensure that the dialog is created, without passing in a detail, because otherwise
// we might end up creating the dialog in a thread, which causes issues.
void EnsureGlobalEmrItemAdvNarrativeDlg(EGlobalNarrativeIndex eGlobalIndex)
{
	CEMNDetail *pTmpDetail = CEMNDetail::__CreateDetail(NULL);
	pTmpDetail->m_EMRInfoType = eitNarrative;
	pTmpDetail->SetState(g_cvarNull);
	GetGlobalEmrItemAdvNarrativeDlg(pTmpDetail, egiLoading);
	pTmpDetail->__QuietRelease();
}

// (z.manning 2012-09-11 12:52) - PLID 52543 - Will load the given EMN from data and re-generate its preview
void RegenerateEmnPreviewFromData(const long nEmnID, BOOL bFailIfExists)
{
	CWaitCursor wc;
	CEMN* pLocalEmn = new CEMN(NULL);
	try
	{
		pLocalEmn->LoadFromEmnID(nEmnID);
		pLocalEmn->GenerateHTMLFile(FALSE, TRUE, FALSE, FALSE, bFailIfExists);
	}
	NxCatchAllCallThrow(__FUNCTION__, delete pLocalEmn;);
	delete pLocalEmn;
}

//(e.lally 2009-10-26) PLID 32503 - Opens the faxing dialogs for the passed in EMN.
//Assumes the EMN and the parent EMR have been pre-loaded.
void FaxEMNPreview(CEMN* pEMN)
{
	//(e.lally 2009-11-30) PLID 32503 - Be sure we are licensed to eFax before doing any work
	if(!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcFaxing, CLicense::cflrUse)) {
		return;
	}

	ASSERT(pEMN != NULL);

	//Force the refresh of the HTML file
	//(e.lally 2009-11-30) PLID 32503 - Set the flag to show the patient demographics 
	// (a.walling 2013-09-05 11:24) - PLID 58369 - GenerateHTMLFile returns path to newly-generated html file
	CString strHtmlFilePath = pEMN->GenerateHTMLFile(FALSE, FALSE, FALSE, TRUE);
	CEMR* pParentEmr = pEMN->GetParentEMR();
	ASSERT(pParentEmr);
	if(pParentEmr == NULL){
		return;
	}
	long nPatientID = pParentEmr->GetPatientID();
	CString strTempFaxFile;
	//(e.lally 2009-12-08) PLID 32503 - use nexemrt_Fax to stay in compliance with the mainframe cleanup of nexemrt_* files.
	strTempFaxFile.Format("nexemrt_Fax_EMN_%li.mht", pEMN->GetID());
	strTempFaxFile = GetNxTempPath() ^ strTempFaxFile;
	//Take the html file and create a temp MHT file, do NOT encrypt it though.
	if(NxCDO::CreateMHTFromFile(strHtmlFilePath, strTempFaxFile, AsSystemTime(pEMN->GetEMNModifiedDate()), FALSE)){

		//Pend a delete action in case of abnormal program termination.
		MoveFileEx(strTempFaxFile, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);

		CStringArray aryFilePaths;
		aryFilePaths.Add(strTempFaxFile);

		CFaxChooseRecipientDlg dlgRecip(pEMN->GetInterface());
		//Set the patient for the recipient selection screen
		dlgRecip.m_nPersonID = nPatientID;
		if(dlgRecip.DoModal() != IDOK) {
			//They cancelled the fax
			//(e.lally 2009-11-30) PLID 32503 - Cleanup our temp files before we quit. 
				//The .html and regular .mht files are cleaned up on their own. Any dependent picture files look to be getting
				//deleted automatically also with this .mht file.
			DeleteFileWhenPossible(strTempFaxFile);
			return;
		}
		//All set to send it
		CFaxSendDlg dlg(pEMN->GetInterface());
		//(e.lally 2011-10-31) PLID 41195 - Pass in the person ID and Pic ID for any mailsent entries
		long nPicID = -1;
		_RecordsetPtr rs = CreateParamRecordset("SELECT ID FROM PicT WHERE EMRGroupID = {INT}", pParentEmr->GetID());
		if(!rs->eof) {
			nPicID = AdoFldLong(rs, "ID",-1);
		}

		dlg.BeginFax(dlgRecip.m_strNumber, dlgRecip.m_strName, aryFilePaths, nPatientID, nPicID);
		//(e.lally 2009-11-30) PLID 32503 - Cleanup our temp files
			//The .html and regular .mht files are cleaned up on their own. Any dependent picture files look to be getting
			//deleted automatically also with this .mht file.
		DeleteFileWhenPossible(strTempFaxFile);
	}
}

// (a.walling 2011-09-14 15:51) - PLID 45498 - Get count of the stamps
// (a.wilson 2013-03-22 09:27) - PLID 55826 - pass in the detail so we can filter out excluded stamps.
long GetInkPictureStampCount(CEMNDetail *pDetail)
{
	//throw exceptions to the caller
	if (pDetail == NULL) {
		ThrowNxException("EMRUtils : GetInkPictureStampCount() was passed a NULL Detail.");
	}

	CMainFrame *pMainFrame = GetMainFrame();
	if(pMainFrame == NULL) {
		return 0;
	}

	// (a.walling 2015-03-19 15:30) - PLID 65082 - Signatures never have stamps
	if (pDetail->IsSignatureDetail()) {
		return 0;
	}

	//This function will only load once unless we force a reload,
	//which we will not do here. So we will load only when needed.
	pMainFrame->LoadEMRImageStamps();

	// Create a string of all the stamp information
	CString strStampString = "";

	long nStampCount = 0;

	for (int i = 0; i < pMainFrame->m_aryEMRImageStamps.GetSize(); i++) {
		EMRImageStamp *eis = (EMRImageStamp*)pMainFrame->m_aryEMRImageStamps.GetAt(i);
		// (j.jones 2010-02-16 15:32) - PLID 37377 - if the stamp is no longer active,
		// do not provide it as an available stamp in the ink picture
		// (a.wilson 2013-03-22 09:31) - PLID 55826 - check if the stamp is excluded before adding it to the count.
		if(!eis->bInactive && !pDetail->GetStampExclusions()->IsExcluded(eis->nID)) {
			nStampCount++;
		}
	}

	return nStampCount;
}

// (j.jones 2010-02-10 11:12) - PLID 37224 - parse EMR Image Stamps (loaded in MainFrame) into a string to send to NxInkPicture
CString GenerateInkPictureStampInfo(CEMNDetail *pDetail)
{
	//throw exceptions to the caller

	CMainFrame *pMainFrame = GetMainFrame();
	if(pMainFrame == NULL) {
		return "";
	}

	// (a.walling 2015-03-19 15:30) - PLID 65082 - Signatures never have stamps
	if (pDetail->IsSignatureDetail()) {
		return "";
	}

	//This function will only load once unless we force a reload,
	//which we will not do here. So we will load only when needed.
	pMainFrame->LoadEMRImageStamps();

	// (z.manning 2011-10-25 13:08) - PLID 39401 - We need to ensure the detail's content is loaded.
	pDetail->LoadContent();

	// Create a string of all the stamp information
	CString strStampString = "";

	long nCountIncludedStamps = 0;

	// (j.jones 2012-12-28 15:00) - PLID 54377 - We now allow unlimited stamps to be created,
	// but no more than MAX_NUM_STAMP_BUTTONS can be associated with a given image.
	// So if we have already added that many, do not add any more.
	for (int i = 0; i < pMainFrame->m_aryEMRImageStamps.GetSize() && nCountIncludedStamps < MAX_NUM_STAMP_BUTTONS; i++)
	{
		EMRImageStamp *eis = (EMRImageStamp*)pMainFrame->m_aryEMRImageStamps.GetAt(i);
		// (j.jones 2010-02-16 15:32) - PLID 37377 - if the stamp is no longer active,
		// do not provide it as an available stamp in the ink picture
		if(eis->bInactive) {
			continue;
		}

		// (z.manning 2011-10-25 12:59) - PLID 39401 - If this detail excludes this stamp then skip it.
		if(pDetail->GetStampExclusions()->IsExcluded(eis->nID)) {
			continue;
		}

		CString strStamp;
		CString strStampText = eis->strStampText;
		strStampText.Replace("\\","\\\\");
		strStampText.Replace("<", "\\<");
		strStampText.Replace(">", "\\>");
		strStampText.Replace(";", "\\;");

		CString strTypeName = eis->strTypeName;
		strTypeName.Replace("\\","\\\\");
		strTypeName.Replace("<", "\\<");
		strTypeName.Replace(">", "\\>");
		strTypeName.Replace(";", "\\;");

		// (r.gonet 05/02/2012) - PLID 49949 - Convert the image to a string of hex numbers. I don't like doing this, but this is the standard place for passing the stamp properties.
		CString strImageBytes = "";
		if(eis->m_ImageInfo.arImageBytes != NULL) {
			strImageBytes = GetByteArrayAsHexString(eis->m_ImageInfo.arImageBytes, eis->m_ImageInfo.nNumImageBytes, "");
		}

		// (z.manning 2010-02-15 16:29) - PLID 37226 - We now send the ID too
		// (z.manning 2012-01-26 17:54) - PLID 47592 - Added option to show dot
		// (r.gonet 05/02/2012) - PLID 49949 - Added image size and bytes
		strStamp.Format("<STAMP=%s;%li;%s;%li;%d;%d;%s;>", strStampText, eis->nTextColor, strTypeName, eis->nID, eis->bShowDot ? 1 : 0, 
			eis->m_ImageInfo.nNumImageBytes, strImageBytes);
		
		strStampString += strStamp;

		// (j.jones 2012-12-28 14:56) - PLID 54377 - track that we added this stamp,
		// we can only add a maximum of MAX_NUM_STAMP_BUTTONS (currently 2,000)
		nCountIncludedStamps++;
	}

	return strStampString;
}

// (z.manning 2010-02-12 11:23) - PLID 37320 - Returns true if the list sub type referes to a smart stamp table column
BOOL IsSmartStampListSubType(const BYTE nSubType)
{
	switch(nSubType)
	{
		case lstSmartStampType:
		case lstSmartStampLocation:
		case lstSmartStampQuantity:
		case lstSmartStampDescription:
		case lstSmartStampAutoNumber: // (z.manning 2010-08-11 10:58) - PLID 40074
		case lstSmartStampInitialType: //TES 3/8/2012 - PLID 48728
			return TRUE;
			break;
	}

	return FALSE;
}

// (j.jones 2011-05-03 16:07) - PLID 43527 - Returns true if the list sub type referes to a built-in current medication table column
BOOL IsCurrentMedicationListSubType(const BYTE nSubType)
{
	switch(nSubType)
	{
		case lstCurrentMedicationSig:
			return TRUE;
			break;
	}

	return FALSE;
}

// (z.manning 2010-02-16 18:04) - PLID 37230
// (z.manning 2010-03-03 11:10) - PLID 37230 - EMR team says we should format anatomic location as
// (j.armen 2012-06-14 15:54) - PLID 50554 - EMR wants left and right lower case
// <side> <qualifier> <location>
CString FormatAnatomicLocation(CString strLocation, CString strQualifier, AnatomySide as)
{
	CString strFullLocation;
	CString strSide;
	if(as == asLeft) { strSide = "left"; }
	else if(as == asRight) { strSide = "right"; }
	if(!strSide.IsEmpty()) {
		strFullLocation += strSide + " ";
	}
	if(!strQualifier.IsEmpty()) {
		strFullLocation += strQualifier + " ";
	}
	strFullLocation += strLocation;
	return strFullLocation;	
}

//TES 2/25/2010 - PLID 37535 - Can the detail be linked on either narratives or tables?
// Note that at the present time, details which can be linked to tables are a subset of those that can be linked to narratives.
BOOL IsDetailLinkable(CEMNDetail *pDetail)
{
	if(IsDetailLinkableOnTable(pDetail)) {
		return TRUE;
	}
	else {
		//TES 2/25/2010 - PLID 37535 - The only way it's linkable if it's not linkable on tables is if it's a table using the 
		// Smart Stamps version of the sentence format.
		// (z.manning 2010-07-26 15:26) - PLID 39848 - Now all tables are linkable always
		if(pDetail && pDetail->m_EMRInfoType == eitTable) {
			return TRUE;
		}
		else {
			return FALSE;
		}
	}
	
}

//TES 2/25/2010 - PLID 37535 - Can this detail be linked in tables?
BOOL IsDetailLinkableOnTable(CEMNDetail *pDetail)
{
	//TES 2/25/2010 - PLID 37535 - If this is not NULL, and is one of the "simple" types, then it can be linked.
	if(pDetail && 
		(pDetail->m_EMRInfoType == eitText || pDetail->m_EMRInfoType == eitSingleList || pDetail->m_EMRInfoType == eitMultiList ||
		pDetail->m_EMRInfoType == eitSlider)) {
			return TRUE;
	}
	else {
		return FALSE;
	}
}

//TES 4/15/2010 - PLID 24692 - Inserts the given entry into our linked list before the specified entry.  If it is already in the list,
// it will be detached from its current position and moved to the specified location.
//TES 4/19/2010 - PLID 24692 - Pass in the head of the list; the function will return the new head of the list, if for some reason
// this operation results in the head of thel ist being changed.
TopicPositionEntry* InsertTopicPositionEntry(TopicPositionEntry *pListHead, TopicPositionEntry *tpe, TopicPositionEntry *tpeInsertBefore)
{
	//TES 4/15/2010 - PLID 24692 - Do we have a list?
	if(pListHead) {
		//TES 4/15/2010 - PLID 24692 - Is this item in our list?  If so, we'll need to deatch it.
		pListHead = DetachTopicPositionEntry(pListHead, tpe);
	}
	//TES 4/15/2010 - PLID 24692 - Now it's detached, let's insert it.
	if(pListHead) {
		pListHead->InsertBefore(tpeInsertBefore, tpe);
	}
	else {
		//TES 4/15/2010 - PLID 24692 - Well, we'll just have to make this our new head.
		pListHead = tpe;
	}
	//TES 5/12/2010 - PLID 24692 - The list head may no longer be in from (the entry may have been inserted before it.  So, return the real
	// head of the list.
	return pListHead->GetListHead();
}

//TES 4/15/2010 - PLID 24692 - Adds the given entry to the end of the list whose parent is the specified topic (-1 for top-level).  If it is already in the list,
// it will be detached from its current position and moved to the specified location.
//TES 4/19/2010 - PLID 24692 - Pass in the head of the list; the function will return the new head of the list, if for some reason
// this operation results in the head of thel ist being changed.
TopicPositionEntry* AddTopicPositionEntryAtEnd(TopicPositionEntry *pListHead, TopicPositionEntry *tpe, long nParentTopicID)
{
	//TES 4/15/2010 - PLID 24692 - Do we have a list?
	if(pListHead) {
		//TES 4/15/2010 - PLID 24692 - Is the entry in our list?  If so, we need to detach it.
		pListHead = DetachTopicPositionEntry(pListHead, tpe);
	}
	if(pListHead) {
		//TES 4/15/2010 - PLID 24692 - Tell the appropriate child to add this entry at the end.
		if(nParentTopicID == -1) {
			pListHead->AddTail(tpe);
		}
		else {
			TopicPositionEntry *tpeParent = pListHead->GetEntryByID(nParentTopicID);
			if(tpeParent) {
				if(tpeParent->pChild) {
					tpeParent->pChild->AddTail(tpe);
				}
				else {
					tpeParent->pChild = tpe;
				}
			}
			else {
				//TES 4/15/2010 - PLID 24692 - The parent doesn't exist!  ASSERT, and add to the top level.
				ASSERT(FALSE);
				pListHead->AddTail(tpe);
			}
		}
	}
	else {
		//TES 4/15/2010 - PLID 24692 - Well, we don't have a list, so this entry will have to be it.
		pListHead = tpe;
	}
	return pListHead;
}

//TES 4/15/2010 - PLID 24692 - Adds the given entry to the end of the list whose parent is the specified topic (NULL for top-level).  If it is already in the list,
// it will be detached from its current position and moved to the specified location.
//TES 4/19/2010 - PLID 24692 - Pass in the head of the list; the function will return the new head of the list, if for some reason
// this operation results in the head of thel ist being changed.
TopicPositionEntry* AddTopicPositionEntryAtEnd(TopicPositionEntry *pListHead, TopicPositionEntry *tpe, TopicPositionEntry *pParent)
{
	//TES 4/15/2010 - PLID 24692 - Do we have a list?
	if(pListHead) {
		//TES 4/15/2010 - PLID 24692 - Is the entry already in our list?  If so, we need to detach it.
		pListHead = DetachTopicPositionEntry(pListHead, tpe);
	}
	//TES 4/15/2010 - PLID 24692 - Now, add to the appropriate parent.
	if(pListHead) {
		if(pParent == NULL) {
			pListHead->AddTail(pParent);
		}
		else {
			if(pParent->pChild) {
				pParent->pChild->AddTail(tpe);
			}
			else {
				pParent->pChild = tpe;
			}
		}
	}
	else {
		//TES 4/15/2010 - PLID 24692 - We didn't have a list, this entry will have to be it.
		pListHead = tpe;
	}
	return pListHead;
}

//TES 5/12/2010 - PLID 24692 - Detaches the given entry.  Pass in the head of the list; the function will return the new head of the list, 
// if this operation results in the head of the list being changed.
TopicPositionEntry* DetachTopicPositionEntry(TopicPositionEntry* pListHead, TopicPositionEntry* tpe)
{
	if(pListHead->HasEntry(tpe)) {
		if(pListHead == tpe) {
			//TES 4/15/2010 - PLID 24692 - It's the head of our list, so we need a new head (assuming there's anything else in the list).
			pListHead = pListHead->pNext;
			if(pListHead) {
				pListHead->pPrev = NULL;
			}
			//TES 4/15/2010 - PLID 24692 - Now detach
			tpe->pNext = tpe->pPrev = NULL;
		}
		else {
			//TES 4/15/2010 - PLID 24692 - Just tell our list to detach it.
			pListHead->Detach(tpe);
		}
	}
	return pListHead;
}

#ifdef _DEBUG_EMR_VERBOSE_LOG
// (c.haag 2010-05-19 9:04) - PLID 38759 - This is used for verbose EMR logging. The
// input string content will be formatted and sent to the Output window.
CCriticalSection g_csEmrLogVerbose;
int g_EmrLogIndent = 0;
void EmrLogVerbose(LPCTSTR szFmt, ...)
{
	// Lock the critical section in case logging is going on in multiple threads at once
	g_csEmrLogVerbose.Lock();

	CString strOutput;

	// Parse string inserting arguments where appropriate
	va_list argList;
	va_start(argList, szFmt);
	strOutput.FormatV(szFmt, argList);
	va_end(argList);

	// Send the string into the Output window followed by a CRLF so the caller doesn't
	// have to add them every time.
	CString strIndent;
	for (int i=0; i < g_EmrLogIndent; i++) {
		strIndent += "\t";
	}
	OutputDebugString(strIndent + strOutput + "\r\n");

	// Unlock the critical section
	g_csEmrLogVerbose.Unlock();
}

// (c.haag 2010-05-19 9:04) - PLID 38759 - This is used for verbose EMR logging. This
// function is identical to EmrLogVerbose except that the indentation of the logging will
// be shifted. If nIndent is positive, it will be shifted -after- the log is written. If nIndent
// is negative, it will be shifted -before- the log is written. This way, when you code
//
// void myfunc() {
// EMRLOGINDENT(1,"myfunc begin");
// EMRLOG("blah");
// EMRLOG("blah");
// EMRLOGINDENT(-1,"myfunc end");
// }
//
// the output looks like:
//
// myfunc begin
//    blah
//    blah
// myfunc end
//
void EmrLogVerboseIndent(int nIndent, LPCTSTR szFmt, ...)
{
	// Lock the critical section in case logging is going on in multiple threads at once
	g_csEmrLogVerbose.Lock();

	CString strOutput;

	// Parse string inserting arguments where appropriate
	va_list argList;
	va_start(argList, szFmt);
	strOutput.FormatV(szFmt, argList);
	va_end(argList);

	// Now update the indentation for successive calls
	if (nIndent < 0) {
		g_EmrLogIndent += nIndent;
	}

	// Send the string into the Output window followed by a CRLF so the caller doesn't
	// have to add them every time.
	CString strIndent;
	for (int i=0; i < g_EmrLogIndent; i++) {
		strIndent += "\t";
	}

	OutputDebugString(strIndent + strOutput + "\r\n");

	// Now update the indentation for successive calls
	if (nIndent > 0) {
		g_EmrLogIndent += nIndent;
	}

	// Unlock the critical section
	g_csEmrLogVerbose.Unlock();
}
#endif

// (j.jones 2010-06-21 14:49) - PLID 37981 - added ability to generate a state from a TableContent pointer
_variant_t GenerateStateFromGenericTableContent(long nEMRInfoID, DevicePluginUtils::TableContent *pGenericTableContent)
{
	CString strAns = "";

	if(pGenericTableContent == NULL || !pGenericTableContent->IsValid()) {
		return _bstr_t("");
	}

	if(pGenericTableContent->aryCells.GetSize() == 0) {
		//no need to build a table state if we were not given any cells
		return _bstr_t("");
	}

	long nRowsNeeded = pGenericTableContent->aryRows.GetSize();
	long nColumnsNeeded = pGenericTableContent->aryColumns.GetSize();

	if(nRowsNeeded == 0 || nColumnsNeeded == 0) {
		ThrowNxException("GenerateStateFromGenericTableContent was given invalid data!");
	}

	//EnsureGenericTableHasEnoughRecords should have been called BEFORE this function is called

	CMap<DevicePluginUtils::RowElement*, DevicePluginUtils::RowElement*, long, long> mapRowPointersToIDs;
	CMap<DevicePluginUtils::ColumnElement*, DevicePluginUtils::ColumnElement*, long, long> mapColumnPointersToIDs;

	_RecordsetPtr rs = CreateParamRecordset(FormatString(
		"SELECT TOP %li ID FROM EMRDataT WHERE Inactive = 0 AND ListType = 2 AND EMRInfoID = {INT} ORDER BY SortOrder\r\n"
		"SELECT TOP %li ID FROM EMRDataT WHERE Inactive = 0 AND ListType = 3 AND EMRInfoID = {INT} ORDER BY SortOrder",
		nRowsNeeded, nColumnsNeeded),
		nEMRInfoID, nEMRInfoID);

	if(rs->GetRecordCount() != nRowsNeeded) {
		ThrowNxException("GenerateStateFromGenericTableContent could not load %li rows for a generic table.", nRowsNeeded);		
	}

	for(int i=0;i<nRowsNeeded;i++) {
		DevicePluginUtils::RowElement *pRow = pGenericTableContent->aryRows.GetAt(i);
		mapRowPointersToIDs.SetAt(pRow, AdoFldLong(rs, "ID"));
		rs->MoveNext();
	}

	rs = rs->NextRecordset(NULL);

	if(rs->GetRecordCount() != nColumnsNeeded) {
		ThrowNxException("GenerateStateFromGenericTableContent could not load %li columns for a generic table.", nColumnsNeeded);		
	}

	for(int i=0;i<nColumnsNeeded;i++) {
		DevicePluginUtils::ColumnElement *pColumn = pGenericTableContent->aryColumns.GetAt(i);
		mapColumnPointersToIDs.SetAt(pColumn, AdoFldLong(rs, "ID"));
		rs->MoveNext();
	}

	rs->Close();

	//now build the content needed
	for(int i=0;i<pGenericTableContent->aryCells.GetSize();i++) {
		long nDataID_X = -1;
		long nDataID_Y = -1;

		DevicePluginUtils::CellElement *pCell = pGenericTableContent->aryCells.GetAt(i);
		mapRowPointersToIDs.Lookup(pCell->pRow, nDataID_X);
		mapColumnPointersToIDs.Lookup(pCell->pColumn, nDataID_Y);
		CString strData = pCell->strValue;

		if(nDataID_X != -1 && nDataID_Y != -1) {
			AppendTableStateWithUnformattedElement(strAns, nDataID_X, nDataID_Y, strData, -1, NULL, -1);
		}
	}

	return _variant_t(strAns);
}

// (j.jones 2010-06-21 14:56) - PLID 37981 - will branch and add more EMRDataT records if needed
void EnsureGenericTableHasEnoughRecords(long &nEMRInfoID, long nRowsNeeded, long nColumnsNeeded)
{
	long nRowsToCreate = 0;
	long nColumnsToCreate = 0;

	_RecordsetPtr rs = CreateParamRecordset(
		"SELECT Count(ID) AS CountRows FROM EMRDataT WHERE Inactive = 0 AND ListType = 2 AND EMRInfoID = {INT} \r\n"
		"SELECT Count(ID) AS CountColumns FROM EMRDataT WHERE Inactive = 0 AND ListType = 3 AND EMRInfoID = {INT}",
		nEMRInfoID, nEMRInfoID);
	if(!rs->eof) {
		long nActualRows = AdoFldLong(rs, "CountRows", 0);
		if(nActualRows < nRowsNeeded) {
			nRowsToCreate = nRowsNeeded - nActualRows;
		}
	}
	else {
		nRowsToCreate = nRowsNeeded;
	}

	rs = rs->NextRecordset(NULL);

	if(!rs->eof) {
		long nActualColumns = AdoFldLong(rs, "CountColumns", 0);
		if(nActualColumns < nColumnsNeeded) {
			nColumnsToCreate = nColumnsNeeded - nActualColumns;
		}
	}
	else {
		nColumnsToCreate = nColumnsNeeded;
	}

	rs->Close();

	if(nRowsToCreate == 0 && nColumnsToCreate == 0) {
		//we have enough records, leave now
		return;
	}

	//branch this item as needed
	if(IsEMRInfoItemInUse(nEMRInfoID)) {
		nEMRInfoID = BranchGenericTableItem();
	}

	//now add our rows & columns
	CString strSqlBatch;

	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @NewDataGroupID int;\r\n");
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @NewDataID int;\r\n");
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @NewSortOrder int;\r\n");

	for(int i=0;i<nRowsToCreate;i++) {
		AddStatementToSqlBatch(strSqlBatch, "SET @NewDataGroupID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM EMRDataGroupsT);\r\n");
		AddStatementToSqlBatch(strSqlBatch, "SET @NewDataID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM EMRDataT);\r\n");
		AddStatementToSqlBatch(strSqlBatch, "SET @NewSortOrder = (SELECT COALESCE(MAX(SortOrder), 0) + 1 FROM EMRDataT WHERE EMRInfoID = %li);\r\n", nEMRInfoID);

		AddStatementToSqlBatch(strSqlBatch, "INSERT INTO EMRDataGroupsT (ID) VALUES (@NewDataGroupID);\r\n");

		AddStatementToSqlBatch(strSqlBatch, 
			"INSERT INTO EMRDataT (ID, EMRInfoID, Data, SortOrder, ListType, EmrDataGroupID) "
			"VALUES (@NewDataID, %li, 'Generic Row ' + Convert(nvarchar, @NewSortOrder), "
			"@NewSortOrder, 2, @NewDataGroupID)",
			nEMRInfoID, nEMRInfoID);
	}

	for(int i=0;i<nColumnsToCreate;i++) {
		AddStatementToSqlBatch(strSqlBatch, "SET @NewDataGroupID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM EMRDataGroupsT);\r\n");
		AddStatementToSqlBatch(strSqlBatch, "SET @NewDataID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM EMRDataT);\r\n");
		AddStatementToSqlBatch(strSqlBatch, "SET @NewSortOrder = (SELECT COALESCE(MAX(SortOrder), 0) + 1 FROM EMRDataT WHERE EMRInfoID = %li);\r\n", nEMRInfoID);

		AddStatementToSqlBatch(strSqlBatch, "INSERT INTO EMRDataGroupsT (ID) VALUES (@NewDataGroupID);\r\n");

		AddStatementToSqlBatch(strSqlBatch, 
			"INSERT INTO EMRDataT (ID, EMRInfoID, Data, SortOrder, ListType, EmrDataGroupID) "
			"VALUES (@NewDataID, %li, 'Generic Column ' + Convert(nvarchar, @NewSortOrder), "
			"@NewSortOrder, 3, @NewDataGroupID)",
			nEMRInfoID, nEMRInfoID);
	}

	ExecuteSqlBatch(strSqlBatch);
}

// (c.haag 2010-07-10 12:07) - PLID 39467 - Returns TRUE if the given EMR info item has table dropdowns
// with duplicate SortOrder values.
BOOL EmrItemHasTableDropdownsWithDuplicateSortOrders(long nEmrInfoID)
{
	// (a.walling 2010-10-14 15:26) - PLID 40965 - Use ReturnsRecords since we don't need the actual record count, just whether it exists.
	// To that end, we can use the ReturnsRecordsParam.
	if (ReturnsRecordsParam("SELECT EMRDataID, SortOrder FROM EMRTableDropdownInfoT "
		"GROUP BY EMRDataID, SortOrder HAVING Count(SortOrder) > 1 "
		"AND EMRDataID IN (SELECT ID FROM EMRDataT WHERE EMRInfoID = {INT}) ", nEmrInfoID))
	{
		return TRUE;
	}
	else 
	{
		return FALSE;
	}
}

// (z.manning 2010-08-11 14:34) - PLID 40074
CString GetEmrAutoNumberTypeDescription(EEmrTableAutoNumberType eType)
{
	switch(eType)
	{
		case etantPerRow:
			return "Per-row";
			break;

		case etantPerStamp:
			return "Per-stamp";
			break;
	}

	return "";
}

// (a.walling 2013-06-27 13:15) - PLID 57348 - NxImageLib - More versatile replacement for g_EmrImageCache
// (j.jones 2011-03-09 09:05) - PLID 42283 - given a category ID, count of E/M elements, and a detail,
// track that information in an array
void AddEMElementsToCategoryArray(long nCategoryID, long nElementCount, CEMNDetail *pDetail,
						 CArray<ChecklistTrackedCategoryInfo*, ChecklistTrackedCategoryInfo*> &aryTrackedCategories)
{
	if(nCategoryID == -1 || pDetail == NULL) {
		//this function should never have been called
		ASSERT(FALSE);
		return;
	}

	if(nElementCount <= 0) {
		//no need to update anything
		return;
	}

	//find the category in our list, if we have one, and if not, add it

	ChecklistTrackedCategoryInfo *pCatInfo = NULL;
	for(int i=0; i<aryTrackedCategories.GetSize() && pCatInfo == NULL; i++) {
		ChecklistTrackedCategoryInfo *pCatInfoToCheck = (ChecklistTrackedCategoryInfo*)(aryTrackedCategories.GetAt(i));
		if(pCatInfoToCheck->nCategoryID == nCategoryID) {
			//found it
			pCatInfo = pCatInfoToCheck;
		}
	}

	//if we didn't find the category, create it
	if(pCatInfo == NULL) {
		pCatInfo = new ChecklistTrackedCategoryInfo;
		pCatInfo->nCategoryID = nCategoryID;
		pCatInfo->nTotalElementsFound = 0;
		aryTrackedCategories.Add(pCatInfo);
	}

	ChecklistTrackedCategoryDetailInfo *pDetailInfo = NULL;

	//it's possible the detail may already exist, if so, don't add it a second time
	for(int i=0; i<pCatInfo->aryDetailInfo.GetSize() && pDetailInfo == NULL; i++) {
		ChecklistTrackedCategoryDetailInfo *pCatInfoToCheck = (ChecklistTrackedCategoryDetailInfo*)(pCatInfo->aryDetailInfo.GetAt(i));
		if(pCatInfoToCheck->pDetail == pDetail) {
			//found it
			pDetailInfo = pCatInfoToCheck;
		}
	}

	//if we didn't find one, create one
	if(pDetailInfo == NULL) {
		pDetailInfo = new ChecklistTrackedCategoryDetailInfo;
		//now add the detail to the category
		pDetailInfo->pDetail = pDetail;
		pDetailInfo->nElementsFound = 0;
		pCatInfo->aryDetailInfo.Add(pDetailInfo);
	}

	//update our total count
	pCatInfo->nTotalElementsFound += nElementCount;
	
	//save the individual number of elements found, which then allows us to find
	//it later without calling pDetail->CalculateEMElements() again
	pDetailInfo->nElementsFound += nElementCount;
}

// (z.manning 2011-03-21 10:05) - PLID 23662
CString GetAutofillTypeDescription(EmrTableAutofillType eAutofillType)
{
	switch(eAutofillType)
	{
		case etatNone:
			return "None";
			break;

		case etatDateAndTime:
			return "Date and Time";
			break;

		case etatDate:
			return "Date";
			break;

		case etatTime:
			return "Time";
			break;

		default:
			// (z.manning 2011-03-21 10:07) - PLID 23662 - Unhandled type
			ASSERT(FALSE);
			break;
	}

	return "";
}

// (z.manning 2011-04-06 12:24) - PLID 43160 - Moved this here from CRoomManagerDlg
// (z.manning 2012-09-10 15:49) - PLID 52543 - Changed to an array of EmnPreviewPopup
void GetEmnIDsByPatientID(long nPatientID, CArray<EmnPreviewPopup, EmnPreviewPopup&> &arynEMNs)
{
	// (z.manning 2011-04-06 12:27) - PLID 43160 - This used to filter out locked EMNs for no good reason so now it doesn't.
	// (z.manning 2011-05-20 14:44) - PLID 33114 - This now filters out EMNs for the current user's chart permissions
	// (z.manning 2012-09-10 15:49) - PLID 52543 - Added modified date
	ADODB::_RecordsetPtr rs = CreateParamRecordset(
		"SELECT ID, ModifiedDate \r\n"
		"FROM EMRMasterT \r\n"
		"LEFT JOIN EmnTabChartsLinkT ON EmrMasterT.ID = EmnTabChartsLinkT.EmnID \r\n"
		"WHERE PatientID = {INT} AND Deleted = 0 {SQL} \r\n"
		"ORDER BY InputDate DESC \r\n"
		, nPatientID, GetEmrChartPermissionFilter());

	for(; !rs->eof; rs->MoveNext()) {
		arynEMNs.Add(EmnPreviewPopup(AdoFldLong(rs->GetFields(), "ID"), AdoFldDateTime(rs, "ModifiedDate")));
	}
}

// (z.manning 2011-04-06 15:17) - PLID 43140
// (j.jones 2012-07-31 15:17) - PLID 51750 - added sqlEMNSortOrder
// (j.jones 2013-07-02 08:56) - PLID 57271 - removed sqlEMNSortOrder, it always sorts by EMR ID now
CSqlFragment GetEmrDetailListOrderSql(CSqlFragment sqlEmnIDInClause)
{
	// (j.jones 2011-04-29 10:46) - PLID 43122 - added IsFloated
	// (j.jones 2012-07-31 15:17) - PLID 51750 - the sort is now defined by sqlEMNSortOrder, default is to ORDER BY EmrMasterT.ID
	// (j.jones 2013-07-02 08:56) - PLID 57271 - removed sqlEMNSortOrder, it always sorts by EMR ID now
	return CSqlFragment(
		"SELECT EmrDetailsT.EmrID, EmrDetailListOrderT.EmrDetailID, EmrDetailListOrderT.EmrDataGroupID, \r\n"
		"	EmrDetailListOrderT.OrderIndex, EmrDetailListOrderT.IsFloated \r\n"
		"FROM EmrDetailListOrderT \r\n"
		"INNER JOIN EmrDetailsT ON EmrDetailListOrderT.EmrDetailID = EmrDetailsT.ID \r\n"
		"INNER JOIN EmrTopicsT ON EmrDetailsT.EmrTopicID = EmrTopicsT.ID \r\n"
		"WHERE EmrDetailsT.EmrID IN ({SQL}) AND EmrDetailsT.Deleted = 0 AND EmrTopicsT.Deleted = 0 \r\n"
		"ORDER BY EmrDetailsT.EmrID, EmrDetailsT.ID, EmrDetailListOrderT.OrderIndex \r\n"
		, sqlEmnIDInClause);
}

// (z.manning 2011-05-19 14:36) - PLID 33114 - Gets the IDs of all EMR charts the current user is licensed to view
// Returns true if they can see all charts
BOOL PollEmrChartPermissions(OUT CArray<long,long> &arynPermissionedChartIDs)
{
	extern CArray<CBuiltInObject*,CBuiltInObject*> gc_aryUserDefinedObjects;;

	BOOL bAllChartsLicensed = TRUE;
	for(int nBuiltInObjectIndex = 0; nBuiltInObjectIndex < gc_aryUserDefinedObjects.GetSize(); nBuiltInObjectIndex++)
	{
		CBuiltInObject *pBuiltInObject = gc_aryUserDefinedObjects.GetAt(nBuiltInObjectIndex);
		if(pBuiltInObject->m_eParentID == bioEmrCharts) {
			if(CheckCurrentUserPermissions(bioEmrCharts, sptView, TRUE, pBuiltInObject->m_nObjectValue, TRUE)) {
				arynPermissionedChartIDs.Add(pBuiltInObject->m_nObjectValue);
			}
			else {
				bAllChartsLicensed = FALSE;
			}
		}
	}

	return bAllChartsLicensed;
}

// (z.manning 2011-05-19 14:48) - PLID 33114 - Function to return SQL meant to filter on only charts the current
// used is permissioned for. The filters are meant for use in the where clause and assume that the necessary table
// (usually EmnTabChartsLinkT) are already part of the from clause.
CSqlFragment GetEmrChartPermissionFilter(BOOL bIncludeAnd /* = TRUE */)
{
	return GetEmrChartPermissionFilter("EmnTabChartsLinkT.EmnTabChartID", bIncludeAnd);
}
CSqlFragment GetEmrChartPermissionFilter(LPCTSTR strFieldToFilterOn, BOOL bIncludeAnd /* = TRUE */)
{
	// (z.manning 2011-05-24 16:14) - PLID 33114 - Admins can always access all charts.
	if(IsCurrentUserAdministrator()) {
		return CSqlFragment();
	}

	CArray<long,long> arynPermissionedChartIDs;
	if(PollEmrChartPermissions(arynPermissionedChartIDs)) {
		// (z.manning 2011-05-24 16:15) - PLID 33114 - If we get here it means the user has permission to access
		// all charts so no need to return a filter.
		return CSqlFragment();
	}

	// (z.manning 2011-05-24 16:16) - PLID 33114 - If we got this far then there are charts that the user cannot access
	// so let's contruct a SQL filter for those. For starters, anyone can always access EMNs that do not have a chart.
	CSqlFragment sqlfragmentChartFilter(FormatString("%s(%s IS NULL", bIncludeAnd ? "AND " : "", strFieldToFilterOn));
	// (z.manning 2011-05-24 16:19) - PLID 33114 - Now filter on specific chart IDs if applicable.
	if(arynPermissionedChartIDs.GetSize() == 1) {
		sqlfragmentChartFilter += CSqlFragment(FormatString(" OR %s = {INT}", strFieldToFilterOn), arynPermissionedChartIDs.GetAt(0));
	}
	else if(arynPermissionedChartIDs.GetSize() > 1) {
		sqlfragmentChartFilter += CSqlFragment(FormatString(" OR %s IN ({INTARRAY})", strFieldToFilterOn), arynPermissionedChartIDs);
	}
	sqlfragmentChartFilter += CSqlFragment(")");

	return sqlfragmentChartFilter;
}

// (z.manning 2011-05-19 15:26) - PLID 33114
ADODB::_RecordsetPtr GetEmrChartRecordset()
{
	CSqlFragment sqlfragmentWhereClause = GetEmrChartPermissionFilter("EmnTabChartsT.ID", FALSE);
	if(!sqlfragmentWhereClause.IsEmpty()) {
		sqlfragmentWhereClause = CSqlFragment("WHERE ") + sqlfragmentWhereClause + CSqlFragment(" \r\n");
	}

	return CreateParamRecordset(
		"SELECT ID, Description, Color \r\n"
		"FROM EmnTabChartsT \r\n"
		"{SQL}"
		"ORDER BY PRIORITY DESC \r\n"
		, sqlfragmentWhereClause);
}

// (z.manning 2011-06-29 11:31) - PLID 37959
// (a.walling 2014-07-08 14:19) - PLID 62812 - Use MFCArray
void RemoveActionsBasedOnAnatomicLocation(IN OUT MFCArray<EmrAction> *paryActions, CEMNDetail *pImageDetail, EmrDetailImageStamp *pDetailStamp)
{
	for(int nActionIndex = paryActions->GetCount() - 1; nActionIndex >= 0; nActionIndex--)
	{
		// (j.jones 2013-07-15 10:51) - PLID 57243 - changed the to a reference
		EmrAction &ea = paryActions->GetAt(nActionIndex);
		CEMRHotSpot *pHotSpot = pImageDetail->GetHotSpotFromDetailStamp(pDetailStamp);
		if(!DoesActionApplyToHotSpot(&ea, pHotSpot)) {
			paryActions->RemoveAt(nActionIndex);
		}
	}
}

// (z.manning 2011-06-29 11:31) - PLID 37959
// (j.jones 2013-07-15 10:51) - PLID 57243 - this action is not modifiable,
// do not attempt to update this pointer, it is used for comparison purposes only
BOOL DoesActionApplyToHotSpot(EmrAction *pea, CEMRHotSpot *pHotSpot)
{
	if(!pea->filter) {
		// (z.manning 2011-06-29 11:40) - PLID 37959 - This action does not have an anatomic location filter so
		// it applies everywhere.
		return TRUE;
	}

	if(pHotSpot == NULL) {
		// (z.manning 2011-06-29 11:28) - PLID 37959 - They did not stamp on a hot spot so no way this action
		// (which is filtered on an anatomic location) can apply.
		return FALSE;
	}

	// (d.singleton 2014-08-06 12:17) - PLID 63180 - Laterality - filter actions using Qualifer and Side
	long nLocationID, nQualifierID;
	nLocationID = pHotSpot->GetAnatomicLocationID();
	nQualifierID = pHotSpot->GetAnatomicQualifierID();

	auto whichSide = Emr::ToWhichAnatomySide(pHotSpot->GetSide());

	auto range = pea->filter.equal_range(nLocationID);
	for (auto it = range.first; it != range.second; it++) {
		if (
			// any qualifier
			(it->qualifierID == -1)
			||
			// no qualifier, and no qualifier for hotspot
			(it->qualifierID == 0 && nQualifierID == -1)
			||
			// qualifier matches hotspot qualifier
			(it->qualifierID == nQualifierID)
			)
		{
			// (r.gonet 2015-02-11) - PLID 64859 - Check to see if the side of the filter includes the side of the hotspot. If the side 
			// is 0 (sideNull), that means the filter has no side setup. Interpret 0 as any side.
			if (it->anatomySide & whichSide || it->anatomySide == 0) {
				return TRUE;
			}
		}
	}
	return FALSE;
}

// (z.manning 2011-07-11 15:22) - PLID 44469 - For a given coding group range and group quantity, will return what
// the charges should be. Also takes a param for all loaded charges so we can easily keep track of them in memory.
void GetChargesByCodingGroupQuantity(CEmrCodingRange *pCodingRange, const long nGroupQuantity, IN CEMNChargeArray *parypAllLoadedCharges, OUT CEMNChargeArray *parypNewCharges)
{
	parypNewCharges->RemoveAll();
	for(int nCodingDetailIndex = 0; nCodingDetailIndex < pCodingRange->GetDetailCount(); nCodingDetailIndex++)
	{
		CEmrCodingGroupDetail *pCodingDetail = pCodingRange->GetDetailByIndex(nCodingDetailIndex);
		EMNCharge *pCharge = parypAllLoadedCharges->FindByServiceID(pCodingDetail->GetCptCodeID());
		if(pCharge == NULL)
		{
			_RecordsetPtr prsCpt = CreateParamRecordset(
				"SELECT ServiceT.ID, Code, SubCode, Name, Price, Billable \r\n"
				"FROM ServiceT \r\n"
				"LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID \r\n"
				"WHERE ServiceT.ID = {INT} \r\n"
				, pCodingDetail->GetCptCodeID());
			if(!prsCpt->eof) {		
				pCharge = new EMNCharge;
				pCharge->nID = -1;
				pCharge->nServiceID = pCodingDetail->GetCptCodeID();
				pCharge->strDescription = AdoFldString(prsCpt, "Name");
				pCharge->cyUnitCost = AdoFldCurrency(prsCpt, "Price", COleCurrency(0,0));
				pCharge->bChanged = TRUE;
				pCharge->strSubCode = AdoFldString(prsCpt, "SubCode", "");
				pCharge->strCode = AdoFldString(prsCpt, "Code", "");
				pCharge->strMod1 = "";
				pCharge->strMod2 = "";
				pCharge->strMod3 = "";
				pCharge->strMod4 = "";
				pCharge->bBillable = AdoFldBool(prsCpt, "Billable", TRUE);
			}
		}

		pCharge->dblQuantity = pCodingDetail->GetCptQuantityFromGroupQuantity(nGroupQuantity);
		parypNewCharges->Add(pCharge);
		if(parypAllLoadedCharges != NULL && !parypAllLoadedCharges->HasCharge(pCharge)) {
			parypAllLoadedCharges->Add(pCharge);
		}
	}
}

// (z.manning 2011-07-25 10:46) - PLID 44676
// (a.walling 2012-04-02 08:32) - PLID 46648 - Dialogs must set a parent!
void OpenHotSpotActionEditor(CWnd* pParent, CEMRHotSpot *pHotSpot, const long nInfoID, CString &strSourceItemName, CEMN *pCurrentEmn, CEMRHotSpotArray *parypChangedHotSpots)
{
	//We found a spot.  Create the action dialog and give it our known actions so the user can edit as
	//	they please.
	CWaitCursor wc;

	// (a.walling 2012-04-02 08:32) - PLID 46648 - Dialogs must set a parent!
	CEmrActionDlg dlg(pParent);
	if(pCurrentEmn != NULL) {
		dlg.SetCurrentEMN(pCurrentEmn);
	}
	dlg.m_SourceType = eaoEmrImageHotSpot;
	dlg.m_nSourceID = CEmrActionDlg::bisidNotBoundToData;
	dlg.m_strSourceObjectName = strSourceItemName;
	dlg.m_nOriginatingID = nInfoID;

	//Fill in the list of actions with those we already have.  Current only, in case they've already done
	//	this and made changes.
	for(int i = 0; i < pHotSpot->GetCurrentActionArray()->GetSize(); i++) {
		dlg.m_arActions.Add( pHotSpot->GetCurrentActionArray()->GetAt(i) );
	}

	if(dlg.DoModal() == IDOK)
	{
		//Get the actions back and save them.  Wipe out anything current and just recopy the whole thing
		pHotSpot->GetCurrentActionArray()->RemoveAll();
		for(i = 0; i < dlg.m_arActions.GetSize(); i++) {
			pHotSpot->GetCurrentActionArray()->Add( dlg.m_arActions.GetAt(i) );
		}

		//If this hotspot is existing (it has a valid positive ID number), then it is now changed.  We want to flag the hotspot so 
		//	that any changes can be applied when saving.  Note that if they open and just hit OK, the end saving code will just
		//	generate no differences.  No sense doing yet another array comparison here.
		if(pHotSpot->GetID() > 0)
		{
			//Things could change multiple times.  Make sure it doesn't already exist
			if(!parypChangedHotSpots->Contains(pHotSpot)) {
				parypChangedHotSpots->Add(pHotSpot);
			}
		}
		else {
			//Non-positive ID means that the hotspot is brand new.  Thus, it must already be in our m_aryNew..., and cannot
			//	be a changed spot.
		}
	}
}

// (z.manning 2011-09-02 17:54) - PLID 32123
BOOL GetEmnStatusName(const long nEmnStatusID, OUT CString &strEmnStatus)
{
	switch(nEmnStatusID)
	{
		case 0:
			strEmnStatus = "Open";
			break;
	
		case 1:
			strEmnStatus = "Finished";
			break;
	
		case 2:
			strEmnStatus = "Locked";
			break;

		default:
			_RecordsetPtr prs = CreateParamRecordset("SELECT Name FROM EMRStatusListT WHERE ID = {INT};", nEmnStatusID);
			if(prs->eof) {
				return FALSE;
			}
			strEmnStatus = AdoFldString(prs, "Name");
			break;
	}

	return TRUE;
}

// (z.manning 2011-09-28 11:57) - PLID 45729 - Function to return the list of text stamps associated with this dropdown item
// (j.jones 2012-11-26 17:31) - PLID 53144 - renamed to more accurately represent that this is the stamp filter only
CString CEmrTableDropDownItem::GetStampFilterText() const
{
	CString strStampText;
	// (a.walling 2014-06-30 10:21) - PLID 62497
	for (auto dropdownStampInfo : aryStampFilter) {
		long nStampID = dropdownStampInfo.nStampID;
		EMRImageStamp *pStamp = GetMainFrame()->GetEMRImageStampByID(nStampID);
		if(pStamp != NULL) {
			strStampText += pStamp->strTypeName + ", ";
		}
	}
	strStampText.TrimRight(", ");
	return strStampText;
}

// (j.jones 2012-11-27 11:08) - PLID 53144 - returns the list of stamps that use this dropdown by default
CString CEmrTableDropDownItem::GetStampDefaultsText() const
{
	CString strStampText;
	// (a.walling 2014-06-30 10:21) - PLID 62497
	for (auto dropdownStampInfo : aryStampDefaults) {
		long nStampID = dropdownStampInfo.nStampID;
		EMRImageStamp *pStamp = GetMainFrame()->GetEMRImageStampByID(nStampID);
		if(pStamp != NULL) {
			strStampText += pStamp->strTypeName + ", ";
		}
	}
	strStampText.TrimRight(", ");
	return strStampText;
}

// (j.jones 2012-11-27 11:08) - PLID 53144 - used for the hyperlink in the filter/defaults setup,
// returns the list of stamps in a filter and/or defaults, and < Choose Stamps >, < Choose Defaults > when empty
CString CEmrTableDropDownItem::GetStampFilterHyperlinkText() const
{
	// (a.walling 2014-06-30 10:21) - PLID 62497 - use .empty() for vectors
	CString strText = "< Choose Stamps >";
	if(aryStampFilter.empty() && aryStampDefaults.empty()) {
		//no filter, no defaults
		strText = "< Choose Stamps >";
	}
	else if (!aryStampFilter.empty() && aryStampDefaults.empty()) {
		//they have a filter but no defaults
		strText.Format("%s / < Choose Defaults >", GetStampFilterText());
	}
	else if (aryStampFilter.empty() && !aryStampDefaults.empty()) {
		//they have defaults but no filter
		strText.Format("< Choose Filter > / %s", GetStampDefaultsText());
	}
	else if (!aryStampFilter.empty() && !aryStampDefaults.empty()) {
		//they have both defaults and a filter
		strText.Format("%s / %s", GetStampFilterText(), GetStampDefaultsText());
	}

	return strText;
}


// (z.manning 2012-07-03 09:25) - PLID 49547 - Deprecated
/*
// (z.manning 2011-10-31 13:53) - PLID 44594
void SignEmns(CArray<EmnSignInfo,EmnSignInfo&> *paryEmnsToSign, CWnd *pwndParent)
{
	if(paryEmnsToSign->GetCount() == 0) {
		return;
	}

	BOOL bWillLockEmns = FALSE;
	int nEmrPostSignatureStatusPref = GetRemotePropertyInt("EmrPostSignatureInsertStatus", -1, 0, GetCurrentUserName());
	if(nEmrPostSignatureStatusPref == 2) {
		bWillLockEmns = TRUE;
	}

	CString strPrompt = FormatString("Are you sure you want to sign %d EMN(s)?", paryEmnsToSign->GetCount());
	if(bWillLockEmns) {
		strPrompt += "\r\n\r\nBecause of your preferences this will also attempt to lock the EMN(s).";
	}
	int nResult = pwndParent->MessageBox(strPrompt, NULL, MB_YESNO|MB_ICONQUESTION);
	if(nResult != IDYES) {
		return;
	}

	// (z.manning 2011-10-31 16:03) - PLID 44594 - First prompt for the user's signature.
	CSignatureDlg dlgSignature(pwndParent);
	dlgSignature.m_bRequireSignature = TRUE;
	dlgSignature.m_bAutoCommitIfSignature = TRUE;
	dlgSignature.m_bCheckPasswordOnLoad = (GetRemotePropertyInt("SignatureCheckPasswordEMR", 1, 0, GetCurrentUserName()) == 1);
	if(dlgSignature.DoModal() != IDOK) {
		return;
	}

	// (z.manning 2011-10-31 16:01) - PLID 44594 - I wanted to show a progress bar while doing this but it was
	// working very well with all the invisible PIC opening we do here. Perhaps down the road we can add one
	// so I left the code here commented out.
	//CShowProgressFeedbackDlg dlgProgress(500, TRUE, FALSE, FALSE, pwndParent->GetSafeHwnd());
	//int nProgressCount = 0;
	//int nEmnCount = parynEmnIDs->GetCount();
	//dlgProgress.SetProgress(nProgressCount, nEmnCount, nProgressCount);

	CString strFailures, strLockFailures;
	long nSignCount = 0;
	while(paryEmnsToSign->GetCount() > 0)
	{
		CWaitCursor wc;
		//nProgressCount++;
		//dlgProgress.SetCaption(FormatString("Signing EMN %li of %li", nProgressCount, nEmnCount));

		EmnSignInfo signInfo = paryEmnsToSign->GetAt(0);
		BOOL bSigned = FALSE;
		// (z.manning 2011-10-31 16:04) - PLID 44594 - Open the PIC for this EMN normally except we'll make the PIC container
		// dialog invisible.
		CPicContainerDlg *pdlgPic = GetMainFrame()->EditEmrRecord(signInfo.nPicID, signInfo.nEmnID, -1, TRUE);
		pdlgPic->ForceLoad();
		if(pdlgPic != NULL)
		{
			if(pdlgPic->GetEmr() != NULL)
			{
				CEMN *pEmn = pdlgPic->GetEmr()->GetEMNByID(signInfo.nEmnID);
				if(pEmn != NULL)
				{
					pEmn->EnsureCompletelyLoaded();
					CEmrTreeWnd *pTree = pEmn->GetInterface();
					// (z.manning 2011-10-31 16:06) - PLID 44594 - Use the same logic as the preview pane to determine the best
					// topic for the signature item.
					CEMRTopic *pTopic = pEmn->FindAppropriateSignatureTopic(NULL);
					if(pTopic != NULL && pTree != NULL && pEmn->IsWritable() && !pEmn->IsLockedAndSaved())
					{
						CEmrTopicWndPtr pTopicWnd = pTree->GetEmrFrameWnd()->GetEmrTopicView(pTopic->GetParentEMN())->EnsureTopicWnd(pTopic);
						if(pTopicWnd && pTopic == pTopicWnd->GetTopic()) {
							// (z.manning 2011-10-31 16:07) - PLID 44594 - Time to actually add the signature to the EMN.
							pTopicWnd->AddSignatureImage(&dlgSignature);
							nSignCount++;
							bSigned = TRUE;

							if(bWillLockEmns && !pEmn->IsLockedAndSaved()) {
								strLockFailures += signInfo.GetDescription() + "\r\n";
							}
						}
					}
				}
			}

			// TODO: We should one day consider optimiziting this a bit to search for multiple EMNs within the 
			// same EMR since we are loading an entire EMR each time. But given how delicate this code is, we
			// need to be careful when doing so.

			// (z.manning 2011-10-31 16:07) - PLID 44594 - Close our invisible PIC window. This will also save the entire EMR.
			pdlgPic->SendMessage(NXM_CLOSE_PIC, cprNone);
		}

		if(!bSigned) {
			strFailures += signInfo.GetDescription() + "\r\n";
		}

		//dlgProgress.SetProgress(nProgressCount, parynEmnIDs->GetCount(), nProgressCount);

		paryEmnsToSign->RemoveAt(0);
	}

	// (z.manning 2011-10-31 16:07) - PLID 44594 - Report the results to the user
	CString strMessage = AsString(nSignCount) + " EMN(s) were successfully signed.";
	if(!strFailures.IsEmpty()) {
		strMessage += "\r\n\r\nThe following EMN(s) could not be signed:\r\n" + strFailures;
	}
	if(!strLockFailures.IsEmpty()) {
		strMessage += "\r\n\r\nThe following EMN(s) could not be locked:\r\n" + strLockFailures;
	}
	pwndParent->MessageBox(strMessage, NULL, MB_OK|MB_ICONINFORMATION);
}
*/

// (z.manning 2011-11-07 17:24) - PLID 46309
CString GetActualSpawnedItemsSepator(LPCTSTR strSpawnedItemsSeparator)
{
	// (z.manning 2011-11-07 17:18) - PLID 46309 - Special handling for "<br>" as we allow EMR team to enter that for a newline
	// since you can't enter an actual return in the edit box.
	CString strActualSeparator = StringReplaceNoCase(strSpawnedItemsSeparator, "<br>", "\r\n");
	return strActualSeparator;
}

// (j.gruber 2012-05-30 13:38) - PLID 49046 - moved functionality so if can be used by NexEMR and PatientDashboard
BOOL CreateEMNFromTemplate(long nTemplateID, CWnd* pParent)
{
	//This row is an EMR Template, just simply launch the editor.

	// (j.jones 2009-08-28 11:33) - PLID 29185 - first prompt to link with an existing EMR
	if(GetRemotePropertyInt("PromptAddNewEMNsToEMR", 1, 0, GetCurrentUserName(), true) == 1) {
		//do they have existing EMRs?
		_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 EMRGroupsT.ID "
			"FROM EMRGroupsT "
			"INNER JOIN PicT ON EmrGroupsT.ID = PicT.EmrGroupID "
			"WHERE (PicT.IsCommitted IS NULL OR PicT.IsCommitted = 1) AND EMRGroupsT.Deleted = 0 "
			"AND EMRGroupsT.PatientID = {INT} "
			"ORDER BY EMRGroupsT.InputDate DESC", GetActivePatientID());
		if(!rs->eof) {
			//they have at least one EMR, so we will be prompting the user with all EMRs the patient has
			// (j.gruber 2010-01-19 09:12) - PLID 36963 - fixed the ID sent to CalcProcInfoName
			CString strFromClause;
			strFromClause.Format("(SELECT -1 AS PicID, ' {Create New EMR}' AS Description, "
				"'' AS ProcedureNames, NULL AS MinDate "
				""
				"UNION SELECT "
				"PicT.ID AS PicID, "
				"EMRGroupsT.Description, dbo.CalcProcInfoName(PicT.ProcInfoID) AS ProcedureNames, "
				"(SELECT Min(dbo.AsDateNoTime(Date)) AS Date FROM EMRMasterT WHERE Deleted = 0 AND EMRGroupID = EMRGroupsT.ID) AS MinDate "
				"FROM EMRGroupsT "
				"INNER JOIN PicT ON EmrGroupsT.ID = PicT.EmrGroupID "							
				"WHERE (PicT.IsCommitted IS NULL OR PicT.IsCommitted = 1) AND EMRGroupsT.Deleted = 0 "
				"AND EMRGroupsT.PatientID = %li) Q", GetActivePatientID());

			//there is at least one EMR, so let's give them the list of options
			CSelectDlg dlg(pParent);
			dlg.m_strTitle = "Add to an existing EMR";
			dlg.m_strCaption = "This patient has existing EMR records. "
				"Please select an existing EMR to associate your new EMN with, or select {Create New EMR} or cancel to create a new EMR record.";
			dlg.m_strFromClause = strFromClause;
			dlg.AddColumn("PicID", "PicID", FALSE, FALSE);
			dlg.AddColumn("Description", "EMR Description", TRUE, TRUE);
			dlg.AddColumn("ProcedureNames", "Procedures", TRUE, TRUE);
			dlg.AddColumn("MinDate", "First EMN Date", TRUE, FALSE, TRUE, 100, 0);
			if(dlg.DoModal() == IDOK) {
				long nPicID = VarLong(dlg.m_arSelectedValues[0]);
				if(nPicID != -1) {					
					GlobalLaunchPICEditorWithNewEMN(nPicID, nTemplateID);
					return TRUE;
				}
				//if they picked -1, they specifically want to make a new EMR,
				//so we will go on to the ladder check below
			}
			//if they cancelled, we will go on to the ladder check below
		}
		//if no EMRs exist, we will go on to the ladder check below
		rs->Close();
	}

	BOOL bEMRLaunched = FALSE;
	//TES 7/28/2006 - PLID 21662 - If they have ladders, prompt to use them.
	// (j.jones 2010-04-16 14:29) - PLID 38236 - do not prompt unless the preference to do so is enabled
	if(GetRemotePropertyInt("PromptAddNewEMNsToLadder", 1, 0, GetCurrentUserName(), true) == 1) {
		//TES 8/2/2006 - PLID 21687 - Check the preference to show completed ladders.
		CString strIncludeInactive = GetRemotePropertyInt("EMRIncludeInactiveLadders",0,0,"<None>",true) ? "1=1" : "(LaddersT.Status IN (SELECT ID FROM LadderStatusT WHERE IsActive = 1) OR LaddersT.ID Is Null)";
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		if(ReturnsRecordsParam(FormatString("SELECT PicT.ID FROM PicT INNER JOIN ProcInfoT ON PicT.ProcInfoID = ProcInfoT.ID "
			"LEFT JOIN LaddersT ON ProcInfoT.ID = LaddersT.ProcInfoID "
			"WHERE ProcInfoT.PatientID = {INT} AND PicT.EmrGroupID Is Null AND PicT.IsCommitted = 1 AND %s", strIncludeInactive), GetActivePatientID())) {
			CSelectDlg dlg(pParent);
			dlg.m_strTitle = "Select a ladder";
			dlg.m_strCaption = "This patient has existing NexTrack ladders.  Please select an existing ladder to associate your new EMN with, or cancel to create a new EMR record.";
			dlg.m_strFromClause = "ProcInfoT INNER JOIN PicT ON ProcInfoT.ID = PicT.ProcInfoID LEFT JOIN LaddersT ON ProcInfoT.ID = LaddersT.ProcInfoID";
			dlg.m_strWhereClause.Format("ProcInfoT.PatientID = %li AND PicT.EmrGroupID Is Null AND PicT.IsCommitted = 1 AND %s", GetActivePatientID(), strIncludeInactive);
			dlg.AddColumn("PicT.ID", "ID", FALSE, FALSE);
			dlg.AddColumn("dbo.CalcProcInfoName(ProcInfoT.ID)", "Procedure", TRUE, TRUE);
			if(dlg.DoModal() == IDOK) {
				ASSERT(dlg.m_arSelectedValues.GetSize() == 2);				
				GlobalLaunchPICEditorWithNewEMR(VarLong(dlg.m_arSelectedValues[0]), nTemplateID);
				bEMRLaunched = TRUE;
			}
		}
	}

	return bEMRLaunched;

}

// (j.gruber 2012-05-30 13:38) - PLID 49046 - moved functionality so if can be used by NexEMR and PatientDashboard
void GlobalLaunchPICEditorWithNewEMN(long nPicID, long nTemplateID)
{
	//TES 11/4/2009 - PLID 35807 - Do they want to be warned if this is the NexWeb template?
	if(GetRemotePropertyInt("WarnWhenCreatingNexWebEmn", 1, 0, GetCurrentUserName())) {
		//TES 11/4/2009 - PLID 35807 - They do, so check whether it is.
		//(e.lally 2011-05-04) PLID 43537 - Use new NexWebDisplayT structure
		_RecordsetPtr rsNexWebTemplate = CreateParamRecordset("SELECT EmrTemplateID FROM NexWebDisplayT "
			"WHERE EmrTemplateID = {INT} AND Visible = 1 ", nTemplateID);
		if(!rsNexWebTemplate->eof) {
			// (b.cardillo 2010-09-22 09:18) - PLID 39568 - Corrected wording now that we allow more than one NexWeb EMR template
			if(IDYES != MsgBox(MB_YESNO|MB_ICONEXCLAMATION, "Warning: This template is a NexWeb template.  It is designed "
				"to be created and filled out by your patients, through your website.  "
				"Are you sure you wish to continue creating an EMN based on this template?")) {
				return;
			}
		}
	}

	//Start a new EMN record with the given PicID (can be -1 for new) and TemplateID
	GetMainFrame()->EditEmrRecord(nPicID, -1, nTemplateID);

}

// (j.gruber 2012-05-30 13:38) - PLID 49046 - moved functionality so if can be used by NexEMR and PatientDashboard
void GlobalLaunchPICEditorWithNewEMR(long nPicID, long nTemplateID)
{
	//TES 11/4/2009 - PLID 35807 - Do they want to be warned if this is the NexWeb template?
	if(GetRemotePropertyInt("WarnWhenCreatingNexWebEmn", 1, 0, GetCurrentUserName())) {
		//TES 11/4/2009 - PLID 35807 - They do, so check whether it is.
		//(e.lally 2011-05-04) PLID 43537 - Use new NexWebDisplayT structure
		_RecordsetPtr rsNexWebTemplate = CreateParamRecordset("SELECT EmrTemplateID FROM NexWebDisplayT "
			"WHERE EmrTemplateID = {INT} AND Visible = 1 ", nTemplateID);
		if(!rsNexWebTemplate->eof) {
			// (b.cardillo 2010-09-22 09:18) - PLID 39568 - Corrected wording now that we allow more than one NexWeb EMR template
			if(IDYES != MsgBox(MB_YESNO|MB_ICONEXCLAMATION, "Warning: This template is a NexWeb template.  It is designed "
				"to be created and filled out by your patients, through your website.  "
				"Are you sure you wish to continue creating an EMN based on this template?")) {
				return;
			}
		}
	}

	//Start a new EMN record with the given PicID (can be -1 for new) and TemplateID
	GetMainFrame()->StartNewEMRRecord(GetActivePatientID(), nPicID, nTemplateID);
}

//DRT 12/15/2005 - Copied & modified from patientemrdlg.cpp
// (j.gruber 2012-05-31 15:15) - PLID 49054 - made a gloal function
void StartEMRTemplateWithCollection(int nEmrCollectionID, CWnd *pParent)
{
	try {
		long nEMRTemplateID = -1;
		BOOL bIsNew = FALSE;

		// Open the new emr using the above selected collection		
		if (nEmrCollectionID != -1) {
			// Prompt the user to select from among the available templates of this collection
			{
				// Prep the dialog object
				CSelectDlg dlgSelectMint(pParent);
				dlgSelectMint.m_strTitle = "Select EMN Template";
				dlgSelectMint.m_strCaption = "Please select the EMN Template you wish to open:";
				dlgSelectMint.m_strFromClause.Format("(SELECT ID, Name FROM EMRTemplateT WHERE Deleted = 0 AND CollectionID = %li UNION ALL SELECT -1, ' { New EMN Template }') Q", nEmrCollectionID);
				// Tell it to use the ID column
				int nIdColumnArrayIndex;
				{
					DatalistColumn dcID;
					dcID.strField = "ID";
					dcID.strTitle = "ID";
					dcID.nWidth = 0;
					dcID.nStyle = NXDATALISTLib::csVisible|NXDATALISTLib::csFixedWidth;		//the select list uses a v1 datalist
					dcID.nSortPriority = -1;
					dcID.bSortAsc = TRUE;
					nIdColumnArrayIndex = dlgSelectMint.m_arColumns.Add(dcID);
				}
				// Tell it to use the Name column
				{
					DatalistColumn dcName;
					dcName.strField = "Name";
					dcName.strTitle = "Template";
					dcName.nWidth = -1;
					dcName.nStyle = NXDATALISTLib::csVisible|NXDATALISTLib::csWidthAuto;	//the select list uses a v1 datalist
					dcName.nSortPriority = 0;
					dcName.bSortAsc = TRUE;
					//m.hancock - 3/24/2006 - PLID 19428 - Set the flag to allow text wrapping
					dcName.bWordWrap = TRUE;
					dlgSelectMint.m_arColumns.Add(dcName);
				}
				// Pop up the dialog
				if(dlgSelectMint.DoModal() == IDOK) {
					// The user made a selection
					nEMRTemplateID = VarLong(dlgSelectMint.m_arSelectedValues[nIdColumnArrayIndex]);
					if (nEMRTemplateID == -1) {
						bIsNew = TRUE;
					}
				} else {
					// The user cancelled
					return;
				}
			}
		} else {
			bIsNew = TRUE;
			nEMRTemplateID = -1;
		}

		// (a.walling 2012-02-29 06:50) - PLID 48469 - Handle new template editor
		if(nEMRTemplateID == -1) {
			CEmrTemplateFrameWnd::CreateNewTemplate(nEmrCollectionID);
		}
		else {
			// (a.walling 2012-04-10 15:56) - PLID 48469 - Moved the NexWeb template check into LaunchWithTemplate (was copy/pasted two other places)
			CEmrTemplateFrameWnd::LaunchWithTemplate(nEMRTemplateID);
		}

/*TODO
		//if (!IsWindow(GetSafeHwnd()))	{
		if (dlg.ClosedAllModalWindows()) {
			// (c.haag 12-27-04 9:45 AM) - PLID 15076 - If we are no longer a valid window handle,
			// it means the user wanted to view a report, or do something else that
			// ultimately requires all modal parents to close.
			// (c.haag 2005-08-25 18:30) - PLID 15076 - Change of plan: Have the main frame do this.
			GetMainFrame()->PostMessage(NXM_PRINT_EMN_REPORT, dlg.m_ID);
			return;
		}
		if(!bIsNew) {
			ReloadEMRList();
		}
*/
	} NxCatchAllSilentCallThrow({
//TODO		m_pEMRDlg = NULL;	//clear our pointer once the dialog is gone
	});
}

// (c.haag 2012-06-11) - PLID 50806 - This dialog prompts the user if they're sure they want to forcefully take
// write access of an EMN from another user
int DoForcedWriteAcquisitionPrompt(CWnd* pParentWnd, const CWriteTokenInfo& wtInfo)
{		
		// (a.walling 2010-01-13 14:08) - PLID 31253
		CASDDlg dlg(pParentWnd);
		dlg.SetParams(
			FormatString(
				"You are about to forcefully take write access from another user. Any changes that this user has made will not be saved! "
				"Rather than forcefully taking write access, it is strongly recommended that you personally contact this user (%s) and have "
				"them save and release write access.\r\n\r\n"
				"Please do not continue until you have contacted this user and verified that the EMN has been saved and is up to date. "
				"When forcefully taking write access, THERE IS ALWAYS THE POSSIBILITY THAT EXTREMELY IMPORTANT MEDICAL INFORMATION MAY HAVE "
				"BEEN ENTERED INTO THIS NOTE BUT NOT YET SAVED.\r\n\r\n", wtInfo.strHeldByUserName),
			"Requesting Write Access",
			"Please read and review this information carefully:",
			NULL);

		return dlg.DoModal();
}

// (j.luckoski 2012-11-20 10:33) - PLID 53825 - Import the ingredients from the API->ImportIngredient function, we do it silently
void ImportAllergyIngredientsFromFDB(IN const CDWordArray &aryIngredToImport) {
	try {
		CArray<NexTech_Accessor::_FDBIngredientImportInputPtr, NexTech_Accessor::_FDBIngredientImportInputPtr> aryIngredients;

		for(int i=0;i<aryIngredToImport.GetSize(); i++) {
			long nPatAllergyID = (long)aryIngredToImport[i];

			if(nPatAllergyID > 0) {
				NexTech_Accessor::_FDBIngredientImportInputPtr ingredient(__uuidof(NexTech_Accessor::FDBIngredientImportInput));
				ingredient->PatAllergyID = nPatAllergyID;
				aryIngredients.Add(ingredient);
			}
 
		}

		// (j.luckoski 2013-02-27 09:34) - PLID 53825 - Need to ensure firstdatabank and surescripts is enabled.
		if(FirstDataBank::EnsureDatabase(NULL, true) && SureScripts::IsEnabled() && aryIngredients.GetSize() > 0) {
			//	Create our SAFEARRAY to be passed to the IngredientImport function in the API
			Nx::SafeArray<IUnknown *> saryIngredients = Nx::SafeArray<IUnknown *>::From(aryIngredients);

			//	Call the API to import the ingredients and then convert the handed back SAFEARRAY to a CArray so we can do something useful with it.
			NexTech_Accessor::_FDBIngredientImportResultsArrayPtr importResults = GetAPI()->IngredientImport(GetAPISubkey(), GetAPILoginToken(), saryIngredients);
		}
	} NxCatchAll(__FUNCTION__);
}

// (j.luckoski 2012-11-20 10:34) - PLID 53825 - This merely executes a deletion from the AllergyIngredientT table
void DeleteAllergyIngredientsFromFDB(IN const CDWordArray &aryIngredToDelete, long nPatientID) {
	try {
		CParamSqlBatch sqlBatch;
		for(int i=0;i<aryIngredToDelete.GetSize(); i++) {
			long nAllergyDataID = (long)aryIngredToDelete[i];
			_RecordsetPtr prs = CreateParamRecordset("SELECT AllergyT.ID AS AllergyID from AllergyT "
				"INNER JOIN EMRDataT ON AllergyT.EMRDataID = EMRDataT.ID where "
				"EMRDataT.ID = {INT}" , nAllergyDataID);

			if(!prs->eof) {
				long nAllergyID = AdoFldLong(prs, "AllergyID", -1);

				sqlBatch.Add("Delete from AllergyIngredientT where PatientAllergyID IN (SELECT ID from PatientAllergyT where AllergyID = {INT} and PersonID = {INT})",
					nAllergyID, nPatientID);
			}
		}
		sqlBatch.Execute(GetRemoteConnection());
	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-10-26 09:35) - PLID 53322 - Typically called after an EMR closes and asks to prompt
// to update PatientsT.HasNoAllergies. Should only be called if we already know they have no allergies
// and know the "Has No Allergies" setting is not checked off.
void PromptPatientHasNoAllergies(long nPatientID)
{
	try {

		//prompt to toggle the "Has No Allergies" flag
		if(IDYES == MessageBox(GetActiveWindow(), "No allergies were entered for this patient.\n"
					"Do you wish to certify that the patient has no known allergies?\n\n"
					"If not, this can be selected later in the patient's Medications Tab.", "Practice", MB_YESNO|MB_ICONQUESTION)) {
						
			//save the change
			ExecuteParamSql("UPDATE PatientsT SET HasNoAllergies = 1 WHERE PersonID = {INT}", nPatientID);

			//audit
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditID, aeiPatientHasNoAllergiesStatus, nPatientID, "'No Known Allergies' Status Cleared", "'No Known Allergies' Status Selected", aepMedium, aetChanged);

			//send a tablechecker
			CClient::RefreshTable(NetUtils::PatientAllergyT, nPatientID);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-10-26 16:52) - PLID 53324 - Typically called after an EMR closes and asks to prompt
// to update PatientsT.HasNoMeds. Should only be called if we already know they have no current medications
// and know the "Has No Medications" setting is not checked off.
void PromptPatientHasNoCurrentMedications(long nPatientID)
{
	try {

		//prompt to toggle the "Has No Medications" flag
		if(IDYES == MessageBox(GetActiveWindow(), "No current medications were entered for this patient.\n"
					"Do you wish to certify that the patient has no known medications?\n\n"
					"If not, this can be selected later in the patient's Medications Tab.", "Practice", MB_YESNO|MB_ICONQUESTION)) {
						
			//save the change
			ExecuteParamSql("UPDATE PatientsT SET HasNoMeds = 1 WHERE PersonID = {INT}", nPatientID);

			//audit
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditID, aeiPatientHasNoMedicationsStatus, nPatientID, "'No Medications' Status Cleared", "'No Medications' Status Selected", aepMedium, aetChanged);

			//send a tablechecker
			CClient::RefreshTable(NetUtils::CurrentPatientMedsT, nPatientID);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-11-14 11:22) - PLID 52819 - Given an EMN ID, will return true if the
// current user has open, writeable access to that EMN at this time.
// The two OUT parameters pass the locked status and access status back to the caller
// for the purposes of context-sensitive messaging.
BOOL CanEditEMN(long nEMNID, OUT long &nStatus, OUT BOOL &bHasAccess)
{
	// (j.armen 2013-05-14 12:20) - PLID 56680 - EMN Access Refactoring
	_RecordsetPtr prs = CreateParamRecordset(
		"SELECT\r\n"
		"	M.Status, A.EMNID\r\n"
		"FROM EmrMasterT M\r\n"
		"LEFT JOIN EMNAccessT A WITH(UPDLOCK, HOLDLOCK) ON M.ID = A.EmnID AND UserLoginTokenID = {INT}\r\n"
		"WHERE M.ID = {INT}", GetAPIUserLoginTokenID(), nEMNID);

	if(!prs->eof) {
		nStatus = AdoFldLong(prs, "Status");
		bHasAccess = AdoFldLong(prs, "EMNID", -1) == nEMNID;
		//if the EMN is locked or not currently accessed, they cannot edit the EMN
		if(nStatus == 2 || !bHasAccess) {
			return FALSE;
		}
		else {
			return TRUE;
		}
	}

	//should be impossible unless the EMNID is invalid
	return FALSE;
}

// (z.manning 2012-11-19 09:46) - PLID 52262
BOOL IsSmartStampImageInUseOnEmnTemplate(const long nSmartStampImageInfoMasterID)
{
	// (z.manning 2012-11-19 12:55) - PLID 52262 - Check data to see if the given smart stamp image ID
	// is in use on a template and linked to the same table that the info item is linked to.
	return ReturnsRecordsParam(
		"SELECT ImageTD.EmrInfoMasterID \r\n"
		"FROM EmrTemplateDetailsT ImageTD \r\n"
		"INNER JOIN EmrInfoMasterT ImageMaster ON ImageTD.EmrInfoMasterID = ImageMaster.ID \r\n"
		"INNER JOIN EmrInfoT ImageInfo ON ImageMaster.ActiveEmrInfoID = ImageInfo.ID \r\n"
		"INNER JOIN EmrTemplateDetailsT TableTD ON ImageTD.ChildEmrTemplateDetailID = TableTD.ID \r\n"
		"WHERE ImageTD.EmrInfoMasterID = {INT} AND ImageInfo.ChildEmrInfoMasterID = TableTD.EmrInfoMasterID \r\n"
		, nSmartStampImageInfoMasterID);
}

// (a.walling 2013-01-22 10:00) - PLID 54762 - Emr Appointment linking - auto-assign appointment or prompt to choose
std::vector<long> FindEmnAppointments(long nPatientID, COleDateTime dtDate)
{
	std::vector<long> appts;

	ADODB::_RecordsetPtr prs = CreateParamRecordset(
		"SELECT AppointmentsT.ID "
		"FROM AppointmentsT "
		"WHERE AppointmentsT.StartTime > {OLEDATETIME} and AppointmentsT.StartTime <= DATEADD(d, 1, {OLEDATETIME}) "
		"AND AppointmentsT.PatientID = {INT} "
		"AND AppointmentsT.Status <> 4 "
		"AND AppointmentsT.ShowState <> 3 "
		"ORDER BY AppointmentsT.StartTime DESC"
		, AsDateNoTime(dtDate), AsDateNoTime(dtDate), nPatientID
	);

	while (!prs->eof) {
		appts.push_back(AdoFldLong(prs, "ID"));
		prs->MoveNext();
	}

	return appts;
}

// (a.walling 2013-01-22 10:00) - PLID 54762 - Emr Appointment linking - auto-assign appointment or prompt to choose
// (a.walling 2013-02-13 10:47) - PLID 55143 - Emr Appointment linking - UI - Added nExistingID for preselection
boost::optional<long> PromptForEmnAppointment(CWnd* pParent, long nPatientID, COleDateTime dtDate, long nExistingID)
{	
	// We're on a mint so just give the user the complete list of all drugs (ignore patient)
	CSelectDlg dlg(pParent);

	dlg.m_bAllowNoSelection = true;
	dlg.m_strTitle = "EMN Appointment";
	dlg.m_strCaption = "Please choose an appointment for this EMN";
	dlg.m_strFromClause.Format(
		"( "
			"SELECT "
				  "AppointmentsT.ID "
				", AppointmentsT.StartTime "
				", AptTypeT.Name AS Type "
				", dbo.GetResourceString(AppointmentsT.ID) AS Resources "
				", dbo.GetPurposeString(AppointmentsT.ID) AS Purposes "
				", AppointmentsT.Notes "
			"FROM AppointmentsT "
			"LEFT JOIN AptTypeT "
			"ON AppointmentsT.AptTypeID = AptTypeT.ID "
			"WHERE "
			"( "
				"AppointmentsT.StartTime > '%s' and AppointmentsT.StartTime < DATEADD(d, 1, '%s') "
				"AND AppointmentsT.PatientID = %li "
				"AND AppointmentsT.Status <> 4 "
				"AND AppointmentsT.ShowState <> 3 "
			") "
			"%s "
			 ""
			"UNION "
			 ""
			"SELECT "
				  "CONVERT(INT, -1) AS ID "
				", CONVERT(DATETIME, NULL) AS StartTime "
				", CONVERT(NVARCHAR(50), NULL) AS Type "
				", N'<No Appointment>' AS Resources "
				", CONVERT(NVARCHAR(50), NULL) AS Purposes "
				", CONVERT(NVARCHAR(50), NULL) AS Notes "
		") SubQ"
		, FormatDateTimeForSql(AsDateNoTime(dtDate))
		, FormatDateTimeForSql(AsDateNoTime(dtDate))
		, nPatientID
		, nExistingID == -1 ? "" : FormatString("OR AppointmentsT.ID = %li ", nExistingID)
	);

	{
		using namespace NXDATALISTLib;

		DatalistColumn dc;

		dc.strField = "ID";
		dc.strTitle = "ID";
		dc.nWidth = 0;
		dc.nStyle = csVisible|csFixedWidth;
		dc.nSortPriority = -1;
		dc.bSortAsc = TRUE;
		dlg.m_arColumns.Add(dc);

		dc.strField = "StartTime";
		dc.strTitle = "Time";
		dc.nWidth = 150;
		dc.nStyle = csVisible|csWidthData;
		dc.nSortPriority = 0;
		dc.bSortAsc = FALSE;
		dlg.m_arColumns.Add(dc);
		
		dc.nSortPriority = -1;
		dc.bSortAsc = TRUE;
		dc.nWidth = 75;

		dc.strField = "Resources";
		dc.strTitle = "Resources";
		dlg.m_arColumns.Add(dc);

		dc.strField = "Type";
		dc.strTitle = "Type";
		dlg.m_arColumns.Add(dc);		
		
		dc.strField = "Purposes";
		dc.strTitle = "Purposes";
		dlg.m_arColumns.Add(dc);
		
		dc.strField = "Notes";
		dc.strTitle = "Notes";
		dc.bWordWrap = TRUE;
		dc.nWidth = 150;
		dlg.m_arColumns.Add(dc);
	}

	// (a.walling 2013-02-13 10:47) - PLID 55143 - Emr Appointment linking - UI - Added nExistingID for preselection
	dlg.SetPreSelectedID(0, nExistingID);

	if (dlg.DoModal() == IDOK) {
		// The user made a selection
		return boost::optional<long>(VarLong(dlg.m_arSelectedValues[0], -1));
	} else {
		// The user cancelled
		return boost::optional<long>();
	}
}

// (a.walling 2013-02-15 11:11) - PLID 54651 - Check for any appointments linked to EMNs
CString GetLinkedEMNDescriptionsFromAppointment(long nApptID)
{
	CString strLinkedEMNs;

	ADODB::_RecordsetPtr prs = CreateParamRecordset(
		"SELECT EmrMasterT.Description, EmrMasterT.Date FROM EmrMasterT "
		"WHERE EmrMasterT.AppointmentID = {INT} "
		"ORDER BY EmrMasterT.Date DESC "
		, nApptID
	);

	for (; !prs->eof; prs->MoveNext()) {
		strLinkedEMNs.AppendFormat("\r\n%s - %s", FormatDateTimeForInterface(AdoFldDateTime(prs, "Date")), AdoFldString(prs, "Description"));
	}

	return strLinkedEMNs;
}

// (a.walling 2013-07-01 09:14) - PLID 57387 - CreateBlankMergeInfoFields can still take ~20-30ms itself even in release mode.
// Since the field list never actually changes, we can cache it.
namespace EMRLetterWritingMergeFieldCache
{
	CCriticalSection cs;
	std::vector<CiString> data;
}

// (c.haag 2013-03-19) - PLID 55697 - Populates a sorted string array with letter writing fields
void LoadEMRLetterWritingMergeFieldList(CStringSortedArrayNoCase &aryFieldList, OPTIONAL IN ADODB::_Connection *lpCon /*= NULL*/)
{	
	CSingleLock lock(&EMRLetterWritingMergeFieldCache::cs, TRUE);
	{
		// (a.walling 2013-07-01 09:14) - PLID 57387 - Cache results if necessary
		if (EMRLetterWritingMergeFieldCache::data.empty()) {			
			_ConnectionPtr pCon;
			if(lpCon) {
				pCon = lpCon;
			}
			else {
				pCon = GetRemoteData();
			}

			//this will load all the merge fields from letter writing, albeit excluding
			//prescription info, bill info, PIC info, and EMR info

			long nFlags = BMS_HIDE_ALL_DATA	| BMS_DEFAULT | BMS_HIDE_PRESCRIPTION_INFO
				| BMS_HIDE_BILL_INFO | BMS_HIDE_PROCEDURE_INFO | BMS_HIDE_EMR_INFO;

			//create a merge engine object
			CMergeEngine mi;

			// (a.walling 2013-05-08 12:13) - PLID 56601 - CreateBlankMergeInfoFields returns a set, so we don't have to execute the sql just to get the field names.
			std::set<CiString> fields = mi.CreateBlankMergeInfoFields(nFlags);
			std::vector<CiString> fieldArray(fields.begin(), fields.end());
			swap(fieldArray, EMRLetterWritingMergeFieldCache::data);
		}
	}

	// (a.walling 2013-07-01 09:14) - PLID 57387 - Otherwise copy results. Also removed the	
	// Find before Insert since that is already done with Insert; plus if it is empty we can simply
	// Add since the list is already sorted case insensitive.
	if (aryFieldList.IsEmpty()) {
		for each (const CiString& str in EMRLetterWritingMergeFieldCache::data) {
			// we already know it is sorted, just add
			aryFieldList.Add(str);
		}
	} else {
		for each (const CiString& str in EMRLetterWritingMergeFieldCache::data) {
			// Insert already does Find with CStringSortedArrayNoCase anyway
			aryFieldList.Insert(str);
		}
	}
}

// (z.manning 2010-11-29 14:42) - PLID 39025
// (j.gruber 2013-09-18 16:20) - PLID 58676 - moved here from EMRTableEditCalculatedFieldDlg
int CompareInfoDataElementsByVisibleIndex(const void *pDataElementA, const void *pDataElementB)
{
	CEmrInfoDataElement **ppeideA = (CEmrInfoDataElement**)pDataElementA;
	CEmrInfoDataElement **ppeideB = (CEmrInfoDataElement**)pDataElementB;
	CEmrInfoDataElement *peideA = *ppeideA;
	CEmrInfoDataElement *peideB = *ppeideB;

	if(peideA != NULL && peideB != NULL)
	{
		if(peideA->m_nVisibleIndex < peideB->m_nVisibleIndex) {
			return -1;
		}
		else if(peideA->m_nVisibleIndex == peideB->m_nVisibleIndex) {
			return 0;
		}
		else {
			ASSERT(peideA->m_nVisibleIndex > peideB->m_nVisibleIndex);
			return 1;
		}
	}
	
	ASSERT(FALSE);
	return 0;
}


CMapStringToString g_mapCancerDiags;
//TES 7/8/2013 - PLID 57415 - Determines whether the code should trigger a Cancer Case submission
BOOL IsCancerDiagnosis(const CString &strDiagCode)
{
	if(g_mapCancerDiags.GetCount() == 0) {
		//TES 4/24/2014 - PLID 61900 - The codes are now stored in a static table in the database
		_RecordsetPtr rsCodes = CreateRecordset("SELECT CodeNumber FROM CancerDiagCodesT");
		while(!rsCodes->eof) {
			g_mapCancerDiags.SetAt(AdoFldString(rsCodes,"CodeNumber"),"");
			rsCodes->MoveNext();
		}
		//TES 4/24/2014 - PLID 61900 - Left the below comments here for future reference

		//TES 7/8/2013 - PLID 57415 - This is the list of codes that should trigger a Cancer Case, the file with this definition can
		// be downloaded from http://phinvads.cdc.gov/vads/ViewView.action?name=Meaningful%20Use%20Healthcare%20Provider%20Reporting%20to%20Central%20Cancer%20Registries#
		// (j.jones 2014-02-27 14:41) - PLID 61069 - added ICD-10 codes to this list, also found at the above link
		//TES 11/25/2013 - PLID 57415 - 204.1 isn't actually in the list, because 204.1 shouldn't really be used (it should be 204.10, 204.11, or 204.12)
		// But the Cancer Case test scripts wants a case generated for 204.1, so, what are you gonna do.	
	}

	CString strTmp;
	return g_mapCancerDiags.Lookup(strDiagCode, strTmp);
}

// (b.savon 2014-02-27 07:25) - PLID 60808 - UPDATE - Write icd10 selections to data upon save of EMN
CString GenerateEMRDiagCodesTWhereCondition(const CString &strDiagCodeID, const CString &strDiagCodeID_ICD10)
{
	//Make sure we're clean
	CString strWhereCondition = "";

	//Do the ICD9 first
	if( strDiagCodeID.CompareNoCase("NULL") == 0 ){
		strWhereCondition += " DiagCodeID IS " + strDiagCodeID + " AND";
	}else{
		strWhereCondition += " DiagCodeID = " + strDiagCodeID + " AND";
	}

	//Do the ICD10 second
	if( strDiagCodeID_ICD10.CompareNoCase("NULL") == 0 ){
		strWhereCondition += " DiagCodeID_ICD10 IS " + strDiagCodeID_ICD10 + " ";
	}else{
		strWhereCondition += " DiagCodeID_ICD10 = " + strDiagCodeID_ICD10 + " ";
	}

	return strWhereCondition;
}

// (b.savon 2014-02-27 14:11) - PLID 61065 - UPDATE - Write icd10 selections to data upon save of EMN - Auditing
CString GenerateEMRDiagCodesTAuditValue(const CString &strDiagCode, const CString &strDiagCode_ICD10)
{
	//Make sure we're clean
	CString strAudit = "";

	if ( !strDiagCode.IsEmpty() && !strDiagCode_ICD10.IsEmpty()){
		//Both are present
		PopulateICD10AuditValue(strAudit, strDiagCode_ICD10);
		strAudit += " (";
		PopulateICD9AuditValue(strAudit, strDiagCode);
		strAudit += ")";
	}
	//If only ICD9 is present
	else if (!strDiagCode.IsEmpty()){
		PopulateICD9AuditValue(strAudit, strDiagCode);
	}
	//If only ICD10 is present
	else if (!strDiagCode_ICD10.IsEmpty()){
		PopulateICD10AuditValue(strAudit, strDiagCode_ICD10);
	}

	return strAudit;
}

// (b.savon 2014-02-27 14:16) - PLID 61065 - UPDATE - Write icd10 selections to data upon save of EMN - Auditing
void PopulateICD9AuditValue(CString &strAudit, const CString &strDiagCode)
{
	if(!strDiagCode.IsEmpty() && strDiagCode.CompareNoCase("NULL") != 0){
		strAudit += strDiagCode;
	}
}

// (b.savon 2014-02-27 14:16) - PLID 61065 - UPDATE - Write icd10 selections to data upon save of EMN - Auditing
void PopulateICD10AuditValue(CString &strAudit, const CString &strDiagCode_ICD10)
{
	if(!strDiagCode_ICD10.IsEmpty() && strDiagCode_ICD10.CompareNoCase("NULL") != 0){
		strAudit += strDiagCode_ICD10;
	}
}

// (r.farnworth 2014-03-06 11:53) - PLID 60820 - Takes the match status from the API and maps it to the enum we have in Practice
NexGEMMatchType MapMatchStatus(NexTech_Accessor::NexGEMDiagCode_MatchStatus matchStatus)
{
	switch(matchStatus)
	{
	case NexTech_Accessor::NexGEMDiagCode_MatchStatus_NoMatch:
		return nexgemtNoMatch;
	case NexTech_Accessor::NexGEMDiagCode_MatchStatus_ManyMatch:
		return nexgemtManyMatch;
	case NexTech_Accessor::NexGEMDiagCode_MatchStatus_Done:
	default:
		return nexgemtDone;
	}
}

// (b.savon 2014-03-06 09:24) - PLID 60824 - SPAWN - Set the colors and behavior of the ICD-10 columns for spawned diagnosis codes in the diag codes list in the <Codes> topic.
NexGEMMatchColor GetDiagnosisNexGEMMatchColor(long nDiagCodeID_ICD10, NexGEMMatchType nxgmMatchType)
{
	if( nDiagCodeID_ICD10 == -1 ){
		switch(nxgmMatchType){
			case nexgemtDone: //Done
				return nxgmDone;
			case nexgemtNoMatch: //NoMatch
				return nxgmNoMatch;
			case nexgemtManyMatch: //ManyMatch
				return nxgmManyMatch;		
		}
	}

	return nxgmDone;
}

// (b.savon 2014-03-06 09:24) - PLID 60824 - SPAWN - Set the colors and behavior of the ICD-10 columns for spawned diagnosis codes in the diag codes list in the <Codes> topic.
CString GetNexGEMDisplayText(long nDiagCodeID_ICD10, const CString &strDiagCodeDescription_ICD10, NexGEMMatchType nxgmMatchType)
{
	if( nDiagCodeID_ICD10 == -1 ){
		switch(nxgmMatchType){
			case nexgemtDone: //Done
				// (r.farnworth 2014-03-11 08:10) - PLID 61246 - Add a button, new link style, or auto load if a user adds codes in icd9 mode and another user loads in icd10 mode
				return "< Find ICD-10 Code >"; 
			case nexgemtNoMatch: //NoMatch
				return "< No Mapping - Open NexCode >";
			case nexgemtManyMatch: //ManyMatch
				return "< Select from possible matches >";
		}
	}

	return strDiagCodeDescription_ICD10;	
}

//TES 11/14/2013 - PLID 57415 - Creates a Cancer Case submission for the given diagnosis, unless one has been created for this PIC since dtLastModified
// (j.jones 2014-02-27 14:10) - PLID 61069 - this now takes in a diag code ID, in addition to the code and description
//TES 5/1/2014 - PLID 61916 - Moved here from PatientNexEmrDlg, added nPatientID and pMessageParent
//TES 5/1/2015 - PLID 61916 - This now returns the filename (if any) that was created
//TES 5/2/2014 - PLID 61855 - Added an output variable, pFailureInfo, for if the function succeeds (we assume this will be non-null if pMessageParent is null), 
// as well as strEmnDescriptionForUser, so that pop-up messages can distinguish which EMN they're referring to.
//TES 5/7/2015 - PLID 65969 - Added strAdditionalFolder. If set, the file will be saved there in addition to the patient's history folder
CString CreateNewCancerCaseDocument(long nPatientID, long nPicID, long nEMNID, const long &nDiagCodeID, const CString &strDiagCode, const CString &strDiagDesc, const COleDateTime &dtLastModified, const COleDateTime &dtEmnDate, CWnd *pMessageParent, CancerCaseFailureInfo *pFailureInfo, const CString &strEmnDescriptionForUser, const CString &strAdditionalFolder /*= ""*/)
{
	try {
		//TES 11/14/2013 - PLID 57415 - Find the last time we generated a cancer case for this PIC
		_RecordsetPtr rsLastCase = CreateParamRecordset("SELECT Max(Date) AS LastDate FROM MailSent WHERE Selection = {STRING} AND PicID = {INT}", SELECTION_CANCERCASE, nPicID);
		_variant_t var = rsLastCase->Fields->GetItem("LastDate")->Value;
		if (var.vt == VT_DATE && VarDateTime(var) >= dtLastModified) {
			//TES 11/14/2013 - PLID 57415 - OK, we've generated one since this was modified, no need for a new one now.
			//TES 11/25/2013 - PLID 57415 - Actually, maybe something else changed, so just warn them
			//TES 5/2/2014 - PLID 61855 - Use the description we were given
			if (IDYES != MsgBox(MB_YESNO, "There has been a Cancer Case submission generated for '" + strEmnDescriptionForUser + "' since the last time it was modified. "
				"Are you sure you wish to generate a new one?")) {
				//TES 5/2/2014 - PLID 61855 - Fill in pFailureInfo
				if (pFailureInfo) {
					pFailureInfo->nEmnID = nEMNID;
					pFailureInfo->strDisplayText = "Cancer Case already existed; user chose not to continue.";
					pFailureInfo->ccft = ccftWarning;
				}
				return "";
			}
		}

		//TES 11/14/2013 - PLID 57415 - We need to generate, so let's do that.
		//TES 7/8/2014 - PLID 62607 - Pass in a pointer to the API
		extern CPracticeApp theApp;
		CancerCaseDocument Document(GetRemoteData(), FALSE, GetCurrentUserID(), GetCurrentLocationID(), theApp.GetAPIObject());
		// (j.jones 2014-02-27 14:10) - PLID 61069 - this now takes in a diag code ID, not the code and description
		Document.Generate(nEMNID, nDiagCodeID);

		CString strTitle, strDisplay;
		CCD::CTimeRange tr;
		CString strDefault = "Cancer Case Submission";
		CString strDescription;

		COleDateTime dtDate;
		if (tr.GetSingleTime().GetStatus() == COleDateTime::valid) {
			dtDate = tr.GetSingleTime();
		}
		else {
			// (c.haag 2010-01-28 11:00) - PLID 37086 - We want to write the server's current time, not the
			// workstation's current time. So, set dtDate to null.
			dtDate.SetStatus(COleDateTime::null);
		}

		CString strWarnings = Document.GetWarnings();
		if (!strWarnings.IsEmpty()) {
			if (pMessageParent) {
				if (IDNO == pMessageParent->MessageBox(FormatString("The following warnings were encountered while generating the Cancer Case submission:\r\n\r\n%s\r\n\r\nDo you want to continue?", strWarnings), NULL, MB_YESNO)) {
					//TES 5/2/2014 - PLID 61855 - Fill in pFailureInfo
					if (pFailureInfo) {
						pFailureInfo->nEmnID = nEMNID;
						pFailureInfo->strDisplayText = strWarnings;
						pFailureInfo->ccft = ccftWarning;
					}
					return "";
				}
			}
			else {
				//TES 5/2/2014 - PLID 61855 - Fill in pFailureInfo
				if (pFailureInfo) {
					pFailureInfo->nEmnID = nEMNID;
					pFailureInfo->strDisplayText = strWarnings;
					pFailureInfo->ccft = ccftWarning;
					return "";
				}
			}
		}

		//TES 5/1/2014 - PLID 61916 - Cancer Cases have their own filename convention
		CString strFirst, strLast;
		long nUserDefinedID;
		GetPatientHistoryDemographicInfo(GetRemoteData(), nPatientID, &strFirst, &strLast, &nUserDefinedID);
		CString strFileName = strLast + "_" + strFirst + "_" + AsString(nUserDefinedID) + "_" + dtEmnDate.Format("%Y_%m_%d") + "_";
		//we need to replace the name information so, generate our replace string
		CString strReplaceString = strLast + "_" + strFirst + "_";
		CString strDocPath = GetPatientDocumentPath(nPatientID);
		strFileName = CalcPatientDocumentName(GetRemoteData(), nPatientID, strDocPath, "xml", strFileName, 4);
		if (strFileName.IsEmpty()) {
			ThrowNxException("Could not get filename to save to history");
		}

		CString strFilePath = GetPatientDocumentPath(nPatientID) ^ strFileName;

		Document.SaveToFile(strFilePath);

		//TES 5/7/2015 - PLID 65969 - If we were given an additional folder, save there as well
		if (!strAdditionalFolder.IsEmpty()) {
			CString strAdditionalFileName = strFileName;
			strAdditionalFileName.Replace(strReplaceString, "");
			Document.SaveToFile(strAdditionalFolder ^ strAdditionalFileName);
		}

		//TES 11/14/2013 - PLID 57415 - OK, now attach to our History tab
		// (d.singleton 2013-11-15 11:58) - PLID 59513 - need to insert the CCDAType when generating a CCDA
		CreateNewMailSentEntry(nPatientID, "Cancer Case: " + strDiagCode + " - " + strDiagDesc, SELECTION_CANCERCASE, strFileName, GetCurrentUserName(), "", GetCurrentLocationID(), dtDate, -1, -1, nPicID, -1, FALSE, -1, "", ctNone);

		//TES 11/14/2013 - PLID 57415 - And inform the user
		if (pMessageParent) {
			CString strMessage;
			strMessage.Format("A Cancer Case document has been successfully created and attached to history for this EMR, for submission to cancer registries.");
			pMessageParent->MessageBox(strMessage, "Practice", MB_ICONINFORMATION);
		}

		return strFilePath;
	}NxCatchAllCallIgnore({
		//TES 5/2/2014 - PLID 61855 - If we were given a pFailureInfo object, we need to pull the error message so we can pass it back to our caller.
		if (pFailureInfo) {
			CString strError;
			try {
				throw;
			}
			catch (CException* e) {
				e->GetErrorMessage(strError.GetBuffer(4096), 4095);
				strError.ReleaseBuffer();
			}
			catch (_com_error& e) {
				strError.Format("%s", e.ErrorMessage());
			}
			catch (std::exception& e) {
				strError = e.what();
			}
			catch (...) {
				strError = "Low level exception";
			}

			if (strError.IsEmpty()) {
				ASSERT(FALSE);
				strError = "Unknown Exception Type";
			}
			pFailureInfo->nEmnID = nEMNID;
			pFailureInfo->strDisplayText = strError;
			pFailureInfo->ccft = ccftError;
		}
		else {
			//TES 5/2/2014 - PLID 61855 - Just go ahead and use the usual exception handling
			try{ throw; }NxCatchAll(__FUNCTION__);
		}
	};
	);

	return "";
}

// (r.farnworth 2014-09-03 12:13) - PLID 63425 - When a spawned diagnosis code that is spawned linked with a problem is changed in the codes topic, we need to update that diagnosis code in the problem dialog
void LinkDiagnosisCodeToProblem(EMNDiagCode *pDiag)
{
	if (!pDiag->m_apEmrProblemLinks.IsEmpty())
	{
		// (r.farnworth 2014-09-03 15:53) - PLID 63425 - Only link based on preference
		DiagCodeSearchStyle dcss = DiagSearchUtils::GetPreferenceSearchStyle();
		BOOL bShowICD9 = (dcss == eICD9_10_Crosswalk || dcss == eManagedICD9_Search);
		BOOL bShowICD10 = (dcss == eICD9_10_Crosswalk || dcss == eManagedICD10_Search);
		bool updateProblem = false;

		// (r.farnworth 2014-09-03 14:57) - PLID 63425 - Changed to include a preference.
		// 0 - Don't update
		// 1 - Prompt
		// 2 - Automatically update
		switch (GetRemotePropertyInt("EMRUpdateDiagnosisCodeProblem", 0, 0, GetCurrentUserName(), true))
		{
		case 0:
			return;
		case 1:
			if (IDYES == MsgBox(MB_YESNO, "Would you like to update the linked EMR problem%s with the diagnosis code change? \r\n",
				pDiag->m_apEmrProblemLinks.GetSize() > 1 ? "s" : "")){
				updateProblem = true;
			}
			else {
				return;
			}
			break;
		case 2:
			updateProblem = true;
			break;
		default:
			break;
		}

		if (updateProblem) {
			for (int i = 0; i < pDiag->m_apEmrProblemLinks.GetSize(); i++) {
				CEmrProblemLink *pProblemLink = pDiag->m_apEmrProblemLinks.GetAt(i);
				CEmrProblem *pProblem = pProblemLink->GetProblem();
				pProblem->m_nDiagICD9CodeID = (bShowICD9) ? pDiag->nDiagCodeID : -1;
				pProblem->m_nDiagICD10CodeID = (bShowICD10) ? pDiag->nDiagCodeID_ICD10 : -1;
				pProblem->m_bIsModified = TRUE;
			}
		}
	}
}

// (j.jones 2014-08-12 09:41) - PLID 63189 - sends an EMRTemplateT tablechecker,
// and tells our NexEMR tab to refresh, if it exists and is active
void RefreshEMRTemplateTable()
{
	try {

		//first send our tablechecker
		CClient::RefreshTable(NetUtils::EMRTemplateT);

		//now find out if the active module is patients, and if the active tab is NexEMR,
		//and if so, refresh the template list
		if (GetMainFrame()) {
			if (GetMainFrame()->IsActiveView(PATIENT_MODULE_NAME)) {
				CPatientView* pView = (CPatientView *)GetMainFrame()->GetOpenView(PATIENT_MODULE_NAME);
				if (pView) {
					if (pView->GetActiveTab() == PatientsModule::NexEMRTab && pView->GetActiveSheet() != NULL) {
						((CPatientNexEMRDlg*)pView->GetActiveSheet())->RefreshNewTemplateList();
					}
				}
			}
			//also do this to the Admin EMR Setup screen
			else if (GetMainFrame()->IsActiveView(ADMIN_MODULE_NAME)) {
				CAdminView* pView = (CAdminView *)GetMainFrame()->GetOpenView(ADMIN_MODULE_NAME);
				if (pView) {
					if (pView->GetActiveTab() == AdminModule::EMRTab && pView->GetActiveSheet() != NULL) {
						pView->GetActiveSheet()->UpdateView();
					}
				}
			}
		}

	}NxCatchAll(__FUNCTION__);

}

// (b.savon 2015-12-29 10:18) - PLID 58470 - Do not allow an EMN to be locked if it has an incomplete Rx associated with it.
// ***Note to those who stumble upon this -- All NewCrop prescriptions are considered 'settled'
// (b.eyers 2016-02-05) - PLID 67980 - the Dispensed In-House prescription status allows emns to be locked
bool HasUnsettledPrescriptions(long emnID, bool showMessage /*= true*/)
{
	if (ReturnsRecordsParam(
		R"(
					SELECT	TOP 1 1
					FROM	EmrMedicationsT
					INNER JOIN PatientMedications ON EmrMedicationsT.MedicationID = PatientMedications.ID
					WHERE	EmrMedicationsT.Deleted = 0 AND EmrID = {INT}
					AND
					(
						PatientMedications.NewCropGUID IS NULL AND PatientMedications.QueueStatus IN(1, 5, 8, 10, 12, 15)
					) 
				)", emnID)
		) {

		if (showMessage) {
			MessageBox(NULL, "You currently have outstanding prescriptions for this EMN. \r\n\r\n"
				"All the prescriptions associated with this EMN and listed in 'More Info' must be from NewCrop, Printed, Sent Electronically, or Dispensed In-House before "
				"you may lock this EMN.", "Nextech", MB_OK | MB_ICONERROR);
		}

		return true;
	}
	else {
		// No unsettled prescriptions
		return false;
	}
}

// (z.manning 2016-04-11 13:39) - NX-100140 - Determines if we should spawn a problem (may want to skip if it's a duplicate)
bool ShouldSpawnEmrProblemForPatient(long nPatientPersonID, long nEmrProblemActionID, std::set<long> setProblemIDsToIgnore)
{
	long nDuplicateSpawnedProblemPref = GetRemotePropertyInt("DuplicateSpawnedEmrProblemBehavior", 2);
	if (nDuplicateSpawnedProblemPref == 3)
	{
		NexTech_Accessor::_EMRProblemsPtr pDuplicateProblems = GetAPI()->GetExistingSpawnedPatientEMRProblems(
			GetAPISubkey(), GetAPILoginToken(), _bstr_t(nEmrProblemActionID), _bstr_t(nPatientPersonID));
		Nx::SafeArray<IUnknown*> pFoundProblems = pDuplicateProblems->Problems;
		foreach(NexTech_Accessor::_EMRProblemPtr pProblem, pFoundProblems)
		{
			long nProblemID = AsLong(pProblem->ID);
			// (z.manning 2016-04-12 09:55) - NX-100140 - Check if this is a problem we should ignore
			if (setProblemIDsToIgnore.count(nProblemID) == 0) {
				// (z.manning 2016-04-12 09:57) - NX-100140 - We found a duplicate problem and it's not
				// one we're ignoring so we should not spawn this problem.
				return false;
			}
		}
	}

	return true;
}
