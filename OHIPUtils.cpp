//OHIPUtils.cpp
//

#include "stdafx.h"

// (d.thompson 2009-03-19) - PLID 33585 - This flag tells whether the user wants to be
//	an OHIP site or not.
bool UseOHIP()
{
	long nOHIP = GetRemotePropertyInt("UseOHIP", 0, 0, "<None>", true);
	if(nOHIP == 0)
		return false;
	else
		return true;
}

// (z.manning 2010-10-28 12:40) - PLID 41129
CString GetOHIPHealthTitle()
{
	CString strOHIPHealthTitle = "Health Card Num.";
	//if enabled, get the actual name of the custom field
	long nHealthNumberCustomField = GetRemotePropertyInt("OHIP_HealthNumberCustomField", 1, 0, "<None>", true);
	ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT Name FROM CustomFieldsT WHERE ID = {INT}", nHealthNumberCustomField);
	if(!rs->eof) {
		CString str = AdoFldString(rs, "Name", "");
		str.TrimLeft();
		str.TrimRight();
		if(!str.IsEmpty()) {
			strOHIPHealthTitle = str;
		}
	}
	rs->Close();
	return strOHIPHealthTitle;
}