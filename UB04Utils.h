#pragma once

#include <afxstr.h>
#include <afxole.h>

#include <vector>

namespace UB04
{
	// (a.walling 2016-03-03 10:06) - PLID 68500 - UB04 Enhancements - C++ structures

	struct Value
	{
		CString code;									// 2 chars
		COleCurrency amount;

		bool operator==(const Value& r) const
		{
			return code == r.code && amount == r.amount;
		}
	};

	struct Occurrence
	{
		CString code;									// 2 chars
		COleDateTime date;

		bool operator==(const Occurrence& r) const
		{
			return code == r.code && date == r.date;
		}
	};

	struct OccurrenceSpan
	{
		CString code;									// 2 chars
		COleDateTime from;
		COleDateTime to;

		bool operator==(const OccurrenceSpan& r) const
		{
			return code == r.code && from == r.from && to == r.to;
		}
	};
	
	struct ClaimInfo
	{
		std::vector<Value> values;						// box (39 + 40 + 41) * abcd
		std::vector<CString> conditions;				// box 18 - 28
		std::vector<Occurrence> occurrences;			// box (31 + 32 + 33 + 34) * ab
		std::vector<OccurrenceSpan> occurrenceSpans;	// box (35 + 36) * ab
		
		CString remarks;								// box 80

		bool IsEmpty() const
		{
			return values.empty() && conditions.empty() && occurrences.empty() && occurrenceSpans.empty() && remarks.IsEmpty();
		}

		bool operator==(const ClaimInfo& r) const
		{
			return remarks == r.remarks && values == r.values && conditions == r.conditions && occurrences == r.occurrences && occurrenceSpans == r.occurrenceSpans;
		}

		bool operator!=(const ClaimInfo& r) const
		{
			return !(*this == r);
		}

		CString GetAuditDescription() const; // (a.walling 2016-03-07 08:53) - PLID 68499 - UB04 Enhancements - Auditing

		CString ToXml() const;
		static ClaimInfo FromXml(const char* szXml); // (a.walling 2016-03-07 08:55) - PLID 68496 - UB04 Enhancements - Load from database
	};
}

