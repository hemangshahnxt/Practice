#include "StdAfx.h"
#include "NxUTS.h"
#include <NxDataUtilitiesLib/iString.h>
#include <NxAlgorithm.h>
#include <boost/container/flat_map.hpp>
#include <NxPracticeSharedLib/NxXMLUtils.h>
#include "UMLSLoginDlg.h"

// (a.walling 2013-10-09 11:11) - PLID 58928 - Use UMLS' UTS web service to search for SNOMED codes, and lookup Concept IDs for them.

CString ThrowSOAPFault(MSXML2::IXMLDOMNodePtr nodeErr);


namespace Nx
{
namespace Soap
{
	// wrap the body and header in a soap:Envelop and soap:Body/Header etc
	inline CString Envelop(const CString& body, const CString& header = CString())
	{
		CString strXml;
		strXml.Format(
			"<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n" 
			"<soap:Envelope "
				"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" " 
				"xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" " 
				"xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n"
			"<soap:Header>\r\n" 
			"%s\r\n"
			"</soap:Header>\r\n"
			"<soap:Body>\r\n"
			"%s\r\n"
			"</soap:Body>\r\n" 
			"</soap:Envelope>"
			, header
			, body
		);
		return strXml;
	}

	// simpler way to format params, using operator() eg ("key", "value").
	// Constructor can take an element name to wrap it all in an element
	// operator() can take a Params() object too; using an element name in the constructor
	// lets you nest the params for more complex types
	class Params
	{
	public:
		Params(const CString& elementName = CString())
			: elementName(elementName)
		{}

		Params& operator()(const CString& name, const CString& value)
		{
			str.AppendFormat("<%s>%s</%s>\r\n", name, Xml::Escape(value), name);
			return *this;
		}

		Params& operator()(const Params& params)
		{
			str += params.Get();
			return *this;
		}

		operator CString() const
		{
			return Get();
		}

		CString Get() const
		{
			if (elementName.IsEmpty()) {
				return str;
			} else {
				CString wrapped;
				wrapped.Format(
					"<%s>\r\n"
					"%s"
					"</%s>\r\n"
					, elementName
					, str
					, elementName
				);
				return wrapped;
			}
		}

	protected:
		CString str;

		CString elementName;
	};
}
}


namespace Nx
{
namespace UTS
{	

// (a.walling 2014-07-31 13:50) - PLID 62911 - Use 2014AA root sources for UMLS
// (j.jones 2015-08-03 11:22) - PLID 66743 - updated to 2015AA
//(s.tullis 04/11/2016) NX-100079 - Removed hardcoded default version

static const char uriSchema[] = "http://webservice.uts.umls.nlm.nih.gov/";

static const char urlSecurity[] = "https://uts-ws.nlm.nih.gov/services/nwsSecurity";
static const char urlFinder[] = "https://uts-ws.nlm.nih.gov/services/nwsFinder";
static const char urlContent[] = "https://uts-ws.nlm.nih.gov/services/nwsContent";
static const char urlMetadata[] = "https://uts-ws.nlm.nih.gov/services/nwsMetadata";

static const char xmlNamespaces[] = 
			"xmlns:soap='http://schemas.xmlsoap.org/soap/envelope/' "
			"xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' " 
			"xmlns:xsd='http://www.w3.org/2001/XMLSchema' "
			"xmlns:uts='http://webservice.uts.umls.nlm.nih.gov/' "
;
//(s.tullis 04/11/2016) NX-100079 - Get Default Version from configrt
CString GetDefaultVersion()
{
	return GetRemotePropertyText("CurrentSnomedVersion", "2015AA", 0, "<None>", true);
}
// only one auth per database for now
Auth GetAuth()
{
	Auth auth = GetAuth_Internal();

	if (auth.user.IsEmpty() && auth.pass.IsEmpty()) {
		// (b.savon 2014-04-30 14:38) - PLID 61978 - Don't throw an exception when the UMLS login information is not configured.
		// Instead, give a message to the user on how to get a username and password; If the user can view the admin module, popup the UMLS login information screen
		BOOL bHasAdminModulePermissions = CanCurrentUserViewObject(bioAdministratorModule);
		CString strMessage;
		CString strFormatMessage = 
			"The UMLS login information is not set.  A UMLS account is required to search for codes.\r\n\r\n"
			"%s may configure the UMLS login information by navigating to the Administrator module.  "
			"Then, click the 'Tools' menu item and select 'UMLS Settings...'";
		strMessage.Format(strFormatMessage, (bHasAdminModulePermissions == FALSE ? "Your office administrator" : "You"));

		//This user can't create the UMLS login information, let them know what needs to happen and just return the empty auth.
		if ( bHasAdminModulePermissions == FALSE){
			GetMainFrame()->PopUpMessageBoxAsync(strMessage, MB_ICONINFORMATION | MB_OK);
			return auth;
		}

		// This user CAN set the login information; pop up the login box
		CUMLSLoginDlg dlg;
		if (IDOK == dlg.DoModal()){
			auth = GetAuth_Internal();
		}

		//If the user cancelled, or clicked OK without entering anything let them know why there wont be any search results and how to
		//setup the login information in the future.
		if (auth.user.IsEmpty() && auth.pass.IsEmpty()) {
			GetMainFrame()->PopUpMessageBoxAsync(strMessage, MB_ICONINFORMATION | MB_OK);
		}
	}

	return auth;
}

// (b.savon 2014-04-30 14:38) - PLID 61978 - added _Internal helper
Auth GetAuth_Internal()
{
	return 
		Auth(
			GetRemotePropertyText("UMLSLogin", "", 0, "<None>", true)
			, GetRemotePropertyText("UMLSPassword", "", 0, "<None>", true)
		);
}

///

CString GetSingleReturnString(MSXML2::IXMLDOMNodePtr pResponse)
{
	if (pResponse) {
		if (MSXML2::IXMLDOMNodePtr pReturn = pResponse->selectSingleNode("return")) {
			return (LPCTSTR)pReturn->text;
		}
	}
	return CString();
}

CString GetSingleReturnStringOrDie(MSXML2::IXMLDOMNodePtr pResponse)
{
	CString str = GetSingleReturnString(pResponse);
	if (str.IsEmpty()) {
		ThrowNxException("Failed to communicate with UTS service authentication");
	}
	return str;
}

// applies common namespaces to the document's selection namespaces property
void ApplyNamespaces(MSXML2::IXMLDOMNodePtr pNode)
{
	MSXML2::IXMLDOMDocument2Ptr pDoc = pNode;
	if (!pDoc && pNode) {
		pDoc = pNode->ownerDocument;
	}
	if (pDoc) {
		pDoc->setProperty("SelectionNamespaces", xmlNamespaces);
	}
}

// calls the soap function with given params; all UTS parameters have no namespace, while the function body itself is within the uts: namespace
// so we had to put the function body in a named namespace (uts:) and leave the params unqualified.
MSXML2::IXMLDOMNodePtr QueryService(const CString& endpoint, const CString& uri, const CString& functionName, const CString& params)
{
	MSXML2::IXMLHTTPRequestPtr req("Msxml2.ServerXMLHTTP.3.0");
	req->open("POST", (const char*)endpoint, false);
	req->setRequestHeader("Content-Type", "text/xml; charset=utf-8");
	req->setRequestHeader("SOAPAction", AsBstr(uri + "/" + functionName));

	CString strBody;
	strBody.Format(
		"<uts:%s xmlns:uts=\"%s\">\r\n"
		"%s\r\n"
		"</uts:%s>"
		, functionName, uri
		, params
		, functionName
	);

	CString strRequest = Soap::Envelop(strBody);
	
	req->send((LPCTSTR)strRequest);


	long status = req->Getstatus();
	CString statusText = (const char*)req->GetstatusText();
	MSXML2::IXMLDOMDocumentPtr response = req->GetresponseXML();
	ApplyNamespaces(response);

#ifdef _DEBUG
	CString strResponse = PrettyPrintXML(response);
#endif

	if (status >= 400) { // this is definitely an error
		throw new CSOAPFaultException(statusText);
	}
	MSXML2::IXMLDOMNodePtr body = response->selectSingleNode("/soap:Envelope/soap:Body");
	if (!body) {
		throw new CSOAPFaultException("No body returned in SOAP Envelope!");
	}

	if (MSXML2::IXMLDOMNodePtr faultNode = body->selectSingleNode("soap:Fault")) {
		ThrowSOAPFault(faultNode);
	}	
	// these services returns a *Response rather than *Result
	if (MSXML2::IXMLDOMNodePtr pResponse = body->selectSingleNode((const char*)("uts:" + functionName + "Response"))) {
		return pResponse;
	}
	if (MSXML2::IXMLDOMNodePtr pResult = body->selectSingleNode((const char*)("uts:" + functionName + "Result"))) {
		return pResult;
	}
	
	throw new CSOAPFaultException("No response found in SOAP Envelope!");

	return NULL;
}

namespace Security 
{		
	// cache the ProxyGrantTicket
	namespace ProxyGrantTickets
	{
		typedef boost::container::flat_map<CiString, CString> Map;
		Map g_map;
		CCriticalSection g_cs;
		
		CString Lookup(const CString& user)
		{
			CSingleLock lock(&g_cs, TRUE);
			return g_map[user];
		}

		void Update(const CString& user, const CString& ticket)
		{
			CSingleLock lock(&g_cs, TRUE);
			g_map[user] = ticket;
		}

		void Clear(const CString& user)
		{
			CSingleLock lock(&g_cs, TRUE);
			g_map[user] = "";
		}
	}

	// Get the cached ProxyGrantTicket, or renew if necessary
	CString GetProxyGrantTicket(const Auth& auth)
	{
		CString pgt = ProxyGrantTickets::Lookup(auth.user);
		if (!pgt.IsEmpty()) {
			return pgt;
		}
		pgt = GetSingleReturnStringOrDie(
			QueryService(urlSecurity, uriSchema, "getProxyGrantTicket", 
				Soap::Params()
					("user", auth.user)
					("pw", auth.pass)
			)
		);
		TRACE(__FUNCTION__" - Got proxy grant ticket `%s`\n", pgt);
		ProxyGrantTickets::Update(auth.user, pgt);
		return pgt;
	}

	// Gets the single-use ProxyTicket for a single call
	CString GetProxyTicket(const CString& proxyGrantTicket)
	{
		CString ticket = GetSingleReturnStringOrDie(
			QueryService(urlSecurity, uriSchema, "getProxyTicket",
			Soap::Params()
			("TGT", proxyGrantTicket)
			("service", "http://umlsks.nlm.nih.gov")
			)
			);
		TRACE(__FUNCTION__" - Got ticket `%s`\n", ticket);
		return ticket;
	}

	// Gets the single-use ProxyTicket for a single call
	// (j.jones 2015-03-30 14:49) - PLID 61540 - Changed this to give a clean message
	// if the login fails. This now returns true if we successfully got a proxy
	// ticket, false if the error was "Invalid Server Error".
	bool GetProxyTicket(IN const Auth& auth, OUT CString &strTicket)
	{
		CString pgt;

		try {
			pgt = GetProxyGrantTicket(auth);
		}
		catch (CSOAPFaultException *e) {
			// (j.jones 2015-03-30 15:15) - PLID 61540 - Give a clean earning if "Internal Server Error"
			// comes up, since that occurs for invalid logins.
			CString strErr;
			if (e->GetErrorMessage(strErr.GetBuffer(4096), 4095, NULL)) {
				strErr.ReleaseBuffer();
				if (strErr == "Internal Server Error") {
					MessageBox(GetActiveWindow(), "Failed to login to UMLS.\n\n"
						"Please confirm your UMLS username and password are correct under Tools->UMLS Settings...", "Practice", MB_ICONEXCLAMATION | MB_OK);
					e->Delete();
					return false;
				}
			}
			//if we're still here, throw the exception
			throw e;
		}

		try {
			strTicket = GetProxyTicket(pgt);
			return true;
		} NxCatchAllIgnore();

		// if that failed, renew the ProxyGrantTicket and try again. The error will bubble up to the caller if it fails this time.
		ProxyGrantTickets::Clear(auth.user);
		pgt = GetProxyGrantTicket(auth);
		strTicket = GetProxyTicket(pgt);
		return true;
	}

	// (j.jones 2015-03-30 15:46) - PLID 61540 - Checks to see if a provided
	// username & password can acquire a valid login.
	// Returns true/false on success/fail. No messaging is shown.
	bool ValidateLogin(CString strUserName, CString strPassword)
	{
		CString pgt;

		try {
			Auth auth = Auth(strUserName, strPassword);
			
			//clear any cached tickets for this user
			ProxyGrantTickets::Clear(auth.user);
			
			//now try to get a new ticket
			pgt = GetProxyGrantTicket(auth);
			return true;
		}
		catch (CSOAPFaultException *e) {
			CString strErr;
			if (e->GetErrorMessage(strErr.GetBuffer(4096), 4095, NULL)) {
				strErr.ReleaseBuffer();
				if (strErr == "Internal Server Error") {
					//no warning, just return false
					e->Delete();
					return false;
				}
			}
			//if we're still here, throw the exception
			throw e;
		}

		//it should be impossible to get here, as an exception would have been thrown
		ASSERT(FALSE);
		return false;
	}
}

namespace Finder
{
	// search for SNOMEDCT codes given a string, version, and search type
	// (a.walling 2013-10-18 10:23) - PLID 59096 - UTS search should support other vocabularies beyond SNOMEDCT
	std::vector<Code> FindSourceConcepts(const CString& str, const CString& rootSource, const CString& searchType, const CString& searchTarget, const CString& version)
	{
		std::vector<Code> codes;
		Auth auth = GetAuth();
		// (b.savon 2014-04-30 14:38) - PLID 61978 - Don't proceed if we have no login information
		if (auth.pass.IsEmpty() && auth.user.IsEmpty())
			return codes;

		// (j.jones 2015-03-30 15:02) - PLID 61540 - This will cleanly fail and return false
		// if their login is invalid. They will already have been warned if this is the case.
		CString ticket = "";
		if (!Security::GetProxyTicket(auth, ticket)) {
			return codes;
		}
		
		MSXML2::IXMLDOMNodePtr pResponse = QueryService(urlFinder, uriSchema, "findSourceConcepts", 
			Soap::Params()
				("ticket", ticket)
				("version", version) 
				("target", searchTarget) 
				("string", str) 
				("searchType", searchType) 
				(Soap::Params("psf")
					("caseSensitive", "0")
					("includeObsolete", "0")
					("includeSuppressible", "0")
					("includedSources", rootSource)
					("pageLn", "256")
					("pageNum", "0")
					("paging", "0")
				) 
		);

		for each (MSXML2::IXMLDOMNodePtr pCode in Xml::NodeList(pResponse->selectNodes("return"))) {
			codes.push_back(Code(GetXMLNodeText(pCode, "label"), GetXMLNodeText(pCode, "rootSource"), GetXMLNodeText(pCode, "ui")));
		}

		return codes;
	}
	//(s.tullis 04/11/2016) NX-100079 - Get Default Version from configrt
	// (a.walling 2013-10-18 10:23) - PLID 59096 - UTS search should support other vocabularies beyond SNOMEDCT
	std::vector<Code> FindSourceConcepts(const CString& str, const CString& rootSource, const CString& searchType, const CString& searchTarget)
	{
		return FindSourceConcepts(str, rootSource, searchType, searchTarget, GetDefaultVersion());
	}

}


namespace Content
{
	// lookup the UMLS Concept ID given a code and rootSource (eg SNOMEDCT)
	CString LookupConceptID(const CString& code, const CString& rootSource, const CString& version)
	{
		Auth auth = GetAuth();
		// (b.savon 2014-04-30 14:38) - PLID 61978 - Don't proceed if we have no login information
		if (auth.pass.IsEmpty() && auth.user.IsEmpty())
			return "";

		// (j.jones 2015-03-30 15:02) - PLID 61540 - This will cleanly fail and return false
		// if their login is invalid. They will already have been warned if this is the case.
		CString ticket = "";
		if (!Security::GetProxyTicket(auth, ticket)) {
			return "";
		}
		
		MSXML2::IXMLDOMNodePtr pResponse = QueryService(urlContent, uriSchema, "getDefaultPreferredAtom", 
			Soap::Params()
				("ticket", ticket)
				("version", version) 
				("atomClusterId", code) 
				("rootSource", rootSource) 
		);

		CString str = GetTextFromXPath(pResponse, "return/concept/ui");
		// (a.walling 2013-10-21 11:11) - PLID 58928 - Not really an exception, just no concept found for this atom
		//if (str.IsEmpty()) {
		//	ThrowNxException("Could not find concept ID for code `%s` using source `%s` version %s", code, rootSource, version);
		//}

		return str;
	}
	//(s.tullis 04/11/2016) NX-100079 - Get Default Version from configrt
	CString LookupConceptID(const CString& code, const CString& rootSource)
	{
		return LookupConceptID(code, rootSource, GetDefaultVersion());
	}
}

namespace Metadata
{
	namespace RootSources
	{		
		std::vector<RootSource> g_rootSources;
		CCriticalSection g_cs;
	}

	// (a.walling 2013-10-18 10:23) - PLID 59096 - Cache list of sources
	std::vector<RootSource> GetRootSources(const CString& version)
	{
		using namespace RootSources;

		{
			CSingleLock lock(&g_cs, TRUE);
			if (!g_rootSources.empty()) {
				return g_rootSources;
			}
		}

		std::set<RootSource> rootSources;

		// (a.walling 2013-10-23 10:16) - PLID 59149 - Need more information on UMLS vocabs / root sources, specifically family
		// Now cached to database, updated if necessary

		// don't autocreate, explicitly not cached
		CString lastVersion = GetRemotePropertyText("UMLSRootSourcesVersion", "", 0, "<None>", false);

		// (j.jones 2015-08-03 11:42) - PLID 66743 - if the version changed, don't load the existing sources,
		// we're going to perform an update
		if (lastVersion == version)
		{
			for (
				ADODB::_RecordsetPtr prs = CreateParamRecordset(GetRemoteDataSnapshot(), 
					"SELECT Name, Family, Description FROM VocabFamilyT"
				);
				!prs->eof;
				prs->MoveNext()
			)
			{
				rootSources.insert(RootSource(
					  AdoFldString(prs, "Name", "")
					, AdoFldString(prs, "Family", "")
					, AdoFldString(prs, "Description", "")
				));
			}
		}

		// (j.jones 2015-08-03 11:42) - PLID 66743 - The root sources will be empty if we have never
		// used the UMLS search before, or if the version has changed and this is the first time
		// we've entered the screen since then. This will do a rull reload of available Vocab families.
		if(rootSources.empty()) {

			{
				Auth auth = GetAuth();
				// (b.savon 2014-04-30 14:38) - PLID 61978 - Don't proceed if we have no login information
				if (auth.pass.IsEmpty() && auth.user.IsEmpty())
				{
					std::vector<RootSource> rootSourcesArray(rootSources.begin(), rootSources.end());

					{
						CSingleLock lock(&g_cs, TRUE);
						g_rootSources = rootSourcesArray;
					}
					return rootSourcesArray;
				}

				// (j.jones 2015-03-30 15:02) - PLID 61540 - This will cleanly fail and return false
				// if their login is invalid. They will already have been warned if this is the case.
				CString ticket = "";
				if (!Security::GetProxyTicket(auth, ticket)) {
					std::vector<RootSource> rootSourcesArray(rootSources.begin(), rootSources.end());

					{
						CSingleLock lock(&g_cs, TRUE);
						g_rootSources = rootSourcesArray;
					}
					return rootSourcesArray;
				}
				
				MSXML2::IXMLDOMNodePtr pResponse = QueryService(urlMetadata, uriSchema, "getAllRootSources", 
					Soap::Params()
						("ticket", ticket)
						("version", version) 
				);

				for each (MSXML2::IXMLDOMNodePtr pSource in Xml::NodeList(pResponse->selectNodes("return"))) {
					rootSources.insert(RootSource(
						  GetXMLNodeText(pSource, "abbreviation")
						, GetXMLNodeText(pSource, "family")
						, GetXMLNodeText(pSource, "expandedForm")
					));
				}
			}

			rootSources.insert(RootSource("", "", "Manually-added"));

			// (a.walling 2013-10-23 10:16) - PLID 59149 - Update or insert into database
			CParamSqlBatch batch;
			batch.Declare("DECLARE @Upsert NVARCHAR(4000);");
			batch.Declare("DECLARE @Params NVARCHAR(255);");
			batch.Declare(
				"SET @Upsert = N'"
					"UPDATE VocabFamilyT WITH (UPDLOCK,SERIALIZABLE) "
						"SET Family = @family, Description = @desc "
						"WHERE Name = @name; "
					"\r\n"
					"IF @@ROWCOUNT = 0 "
						"INSERT INTO VocabFamilyT(Name, Family, Description) "
						"VALUES(@name, @family, @desc); "
				"';"
			);
			batch.Declare("SET @Params = N'@name NVARCHAR(24), @family NVARCHAR(24), @desc NVARCHAR(1024)';");

			for each (const RootSource& source in rootSources) {
				batch.AddFormat(
					"EXEC sp_executesql @Upsert, @Params, '%s', '%s', '%s';"
					, _Q(source.name), _Q(source.family), _Q(source.description)
				);
			}

			try {
				NxAdo::PushPerformanceWarningLimit ppw(-1);
				batch.Execute(GetRemoteData());
				SetRemotePropertyText("UMLSRootSourcesVersion", version, 0, "<None>");
			} NxCatchAll(__FUNCTION__);
		}

		std::vector<RootSource> rootSourcesArray(rootSources.begin(), rootSources.end());

		{
			CSingleLock lock(&g_cs, TRUE);
			g_rootSources = rootSourcesArray;
		}

		return rootSourcesArray;
	}
	//(s.tullis 04/11/2016) NX-100079 - Get Default Version from configrt
	// (a.walling 2013-10-18 10:23) - PLID 59096 - This vector copies very quickly
	std::vector<RootSource> GetRootSources()
	{
		return GetRootSources(GetDefaultVersion());
	}
}

}
}
