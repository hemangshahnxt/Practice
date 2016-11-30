//
//WebFaxUtils.h

#ifndef WEBFAXUTILS_H
#define WEBFAXUTILS_H

#pragma once





//DRT 6/26/2008 - PLID 30524
//As we add further fax support, add enums here, and add them to LoadServiceList().
//	These IDs are saved in data, so do not change them.
enum eSupportedFaxServices {
	esfsUnknown = -1,
	esfsMyFax = 1,
};

//DRT 6/27/2008 - PLID 30524 - Current settings for sending faxes.  Fill this and send it
//	as a parameter to your fax object.
class CFaxSettings {
public:
	CString m_strUser;
	CString m_strPassword;
	CString m_strFromName;
	CString m_strResolution;
	CString m_strRecipNumber;
	CString m_strRecipName;
	CStringArray m_aryDocPaths;
	// (z.manning 2009-09-29 12:32) - PLID 32472 - Added cover page subject
	CString m_strCoverPageSubject;

	//Default constructor
	CFaxSettings() { }

	//Copy constructor
	CFaxSettings(CFaxSettings &newSettings)
	{
		m_strUser = newSettings.m_strUser;
		m_strPassword = newSettings.m_strPassword;
		m_strFromName = newSettings.m_strFromName;
		m_strResolution = newSettings.m_strResolution ;
		m_strRecipNumber = newSettings.m_strRecipNumber;
		m_strRecipName = newSettings.m_strRecipName;
		m_strCoverPageSubject = newSettings.m_strCoverPageSubject;

		for(int i = 0; i < newSettings.m_aryDocPaths.GetSize(); i++) {
			m_aryDocPaths.Add(newSettings.m_aryDocPaths.GetAt(i));
		}
	}
};


///////////////////////////////////////////////////////////////
//DRT 6/27/2008 - PLID 30524 - Fax setups.  Implement any new fax
//	integrations here.  The functionality should handle all sending
//	of the fax, saving any traces of the fax, and reporting any errors.
//	These are mostly called from CFaxSendDlg.


//DRT 6/27/2008 - PLID 30524 - Fax sending class for MyFax integration.  All logic that is
//	specific to MyFax should be kept in this class.
class CMyFaxSend {
public:
	//(e.lally 2011-10-31) PLID 41195 - Added Person ID and Pic ID for any mailsent entries
	bool SendFax(CFaxSettings settings, long nPersonID = -1, long nPicID = -1);
	bool GenerateSendSingleFaxDocument(IN CFaxSettings settings, OUT CString &strXmlDocument);
	bool GenerateDocumentXML(IN CString strDocPath, OUT CString &strDocumentXML);
};

//Loads a hardcoded list of available services into a datalist2.  The datalist must have 2 columns, an ID for the type, 
//	and a string for the text.
void LoadServiceList(NXDATALIST2Lib::_DNxDataListPtr pList);

//Loads the available document resolutions.  As of 2008-06-26, we only have myFax, and honestly
//	I don't see why you'd ever send it as a low resolution.  But high & low are there, so we 
//	support the options.
//The datalist must have 1 column, a string for the text.
void LoadResolutions(NXDATALIST2Lib::_DNxDataListPtr pList);

//TES 3/4/2009 - PLID 32078 - Moved here from SOAPUtils.
CString GetWebContentType(CString strExt);


#endif	//WEBFAXUTILS_H