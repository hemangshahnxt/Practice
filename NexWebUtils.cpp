//NexWebUtils.cpp

#include "stdafx.h"
#include "Practice.h"
#include "nexwebutils.h"


using namespace ADODB;

CString GetGender(CString strGender) {
	if (strGender == "1") {
		return "Male";
	}
	else if (strGender == "2") {
		return "Female";
	}
	else {
		//(e.lally 2010-10-18) PLID 35603 - Make this "None" instead of "" for consistent auditing
		return "None";
	}
}

CString GetLocation (CString strLocation) {

	if (strLocation.IsEmpty() ) {
		return "";
	}
	else { 	
		_RecordsetPtr rs = CreateRecordset("SELECT Name From LocationsT WHERE ID = %s", strLocation);

		if (! rs->eof) {

			return AdoFldString(rs, "Name");
		}
		else {
			return "";
		}
	}
}

CString GetPatientType (CString strPatType) {
	
	if (strPatType.IsEmpty() ) {
		return "";
	}
	else {
	
		_RecordsetPtr rs = CreateRecordset("SELECT GroupName From GroupTypes WHERE TypeIndex = %s", strPatType);

		if (! rs->eof) {

			return AdoFldString(rs, "GroupName");
		}
		else {
			return "";
		}
	}
}

CString GetMaritalStatus (CString strMS) {

	if (strMS == "1") {
		return "Single";
	}
	else if (strMS == "2") {
		return "Married";
	}
	else if (strMS == "3") {
		return "Other";
	}
	else {
		return "";
	}
}
		

CString GetReferralSource (CString strRefSource) {
	
	if (strRefSource.IsEmpty() ) {
		return "";
	}
	else {
	
		_RecordsetPtr rs = CreateRecordset("SELECT Name From ReferralSourceT WHERE PersonID = %s", strRefSource);

		if (! rs->eof) {

			return AdoFldString(rs, "Name");
		}
		else {
			return "";
		}
	}
}
