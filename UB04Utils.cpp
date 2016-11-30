#include "StdAfx.h"

#include "UB04Utils.h"

#include "pugixml/pugixml.hpp"

namespace UB04
{
	// (a.walling 2016-03-07 08:53) - PLID 68499 - UB04 Enhancements - Auditing
	CString ClaimInfo::GetAuditDescription() const
	{
		CString desc;

		if (!values.empty()) {
			desc += "Values: ";
			for (auto&& val : values) {
				desc.AppendFormat("%s - %s, ", val.code, FormatCurrencyForInterface(val.amount));
			}
			desc.TrimRight(", ");
			desc += "\r\n";
		}

		if (!conditions.empty()) {
			desc += "Conditions: ";
			for (auto&& val : conditions) {
				desc.AppendFormat("%s, ", val);
			}
			desc.TrimRight(", ");
			desc += "\r\n";
		}

		if (!occurrences.empty()) {
			desc += "Occurrences: ";
			for (auto&& val : occurrences) {
				desc.AppendFormat("%s - %s, ", val.code, FormatDateTimeForInterface(val.date, dtoDate));
			}
			desc.TrimRight(", ");
			desc += "\r\n";
		}

		if (!occurrenceSpans.empty()) {
			desc += "Occurrence spans: ";
			for (auto&& val : occurrenceSpans) {
				desc.AppendFormat("%s - %s to %s, ", val.code, FormatDateTimeForInterface(val.from, dtoDate), FormatDateTimeForInterface(val.to, dtoDate));
			}
			desc.TrimRight(", ");
			desc += "\r\n";
		}

		if (!remarks.IsEmpty()) {
			desc.AppendFormat("Remarks: %s\r\n", remarks);
		}

		desc.TrimRight("\r\n");

		return desc;
	}

	CString ClaimInfo::ToXml() const
	{
		CString xml;

		xml += "<claimInfo>";

		if (!values.empty()) {
			xml += "<values>";
			for (auto&& val : values) {
				xml.AppendFormat(R"(<value code="%s" amount="%lli"/>)"
					, ConvertToQuotableXMLString(val.code)
					, val.amount.m_cur.int64
				);
			}
			xml += "</values>";
		}

		if (!conditions.empty()) {
			xml += "<conditions>";
			for (auto&& val : conditions) {
				xml.AppendFormat(R"(<condition code="%s"/>)"
					, ConvertToQuotableXMLString(val)
				);
			}
			xml += "</conditions>";
		}

		if (!occurrences.empty()) {
			xml += "<occurrences>";
			for (auto&& val : occurrences) {
				xml.AppendFormat(R"(<occurrence code="%s" date="%s"/>)"
					, ConvertToQuotableXMLString(val.code)
					, val.date.Format("%Y-%m-%dZ")
				);
			}
			xml += "</occurrences>";
		}

		if (!occurrenceSpans.empty()) {
			xml += "<occurrenceSpans>";
			for (auto&& val : occurrenceSpans) {
				xml.AppendFormat(R"(<occurrenceSpan code="%s" from="%s" to="%s"/>)"
					, ConvertToQuotableXMLString(val.code)
					, val.from.Format("%Y-%m-%dZ")
					, val.to.Format("%Y-%m-%dZ")
				);
			}
			xml += "</occurrenceSpans>";
		}

		if (!remarks.IsEmpty()) {
			xml.AppendFormat("<remarks>%s</remarks>", ConvertToQuotableXMLString(remarks));
		}

		xml += "</claimInfo>";

		return xml;
	}

	// (a.walling 2016-03-07 08:55) - PLID 68496 - UB04 Enhancements - Load from database
	ClaimInfo ClaimInfo::FromXml(const char* szXml)
	{
		ClaimInfo ci;

		if (!szXml || !szXml[0]) {
			return ci;
		}

		pugi::xml_document doc;

		// do not normalize \r\n to \n, so turn off the parse_eol flag
		// turns out SQL server normalizes this to \n within its own xml handling, so this is irrelevant
		auto result = doc.load(szXml, pugi::parse_default /* & ~pugi::parse_eol*/);

		if (!result) {
			ThrowNxException("Invalid ClaimInfo xml: `%s` at offset %li within `%s`", result.description(), result.offset, szXml);
		}

		auto claimInfo = doc.child("claimInfo");
		if (!claimInfo) {
			return ci;
		}

		for (auto node : claimInfo.children()) {
			if (!strcmp(node.name(), "values")) {
				for (auto val : node.children("value")) {
					auto code = val.attribute("code").as_string();
					auto amount = val.attribute("amount").as_llong();

					COleCurrency cyAmount;
					cyAmount.m_cur.int64 = amount;

					ci.values.push_back({ code, cyAmount });
				}
			} else if (!strcmp(node.name(), "conditions")) {
				for (auto val : node.children("condition")) {
					auto code = val.attribute("code").as_string();

					ci.conditions.push_back(code);
				}
			} else if (!strcmp(node.name(), "occurrences")) {
				for (auto val : node.children("occurrence")) {
					auto code = val.attribute("code").as_string();
					CString date = val.attribute("date").as_string();
					COleDateTime dtDate;
					dtDate.ParseDateTime(date.TrimRight("Z"));

					ci.occurrences.push_back({ code, dtDate });
				}
			} else if (!strcmp(node.name(), "occurrenceSpans")) {
				for (auto val : node.children("occurrenceSpan")) {
					auto code = val.attribute("code").as_string();
					CString from = val.attribute("from").as_string();
					CString to = val.attribute("to").as_string();
					COleDateTime dtFrom;
					COleDateTime dtTo;
					dtFrom.ParseDateTime(from.TrimRight("Z"));
					dtTo.ParseDateTime(to.TrimRight("Z"));

					ci.occurrenceSpans.push_back({ code, dtFrom, dtTo });
				}
			} else if (!strcmp(node.name(), "remarks")) {
				ci.remarks = node.text().as_string();
				ci.remarks.Replace("\r", "");
				ci.remarks.Replace("\n", "\r\n");
			}
		}

		return ci;
	}
}
