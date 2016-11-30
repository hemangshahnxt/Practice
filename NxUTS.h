#pragma once

#include <vector>

// (a.walling 2013-10-09 11:11) - PLID 58928 - Use UMLS' UTS web service to search for SNOMED codes, and lookup Concept IDs for them.

namespace Nx
{
namespace UTS
{
struct Auth
{
	CString user;
	CString pass;

	Auth()
	{}

	Auth(const CString& user, const CString& pass)
		: user(user)
		, pass(pass)
	{}
};

Auth GetAuth();
// (b.savon 2014-04-30 14:38) - PLID 61978 - added _Internal helper
Auth GetAuth_Internal();

namespace Security
{
	// (j.jones 2015-03-30 14:49) - PLID 61540 - Changed this to give a clean message
	// if the login fails. This now returns true if we successfully got a proxy
	// ticket, false if the error was "Invalid Server Error".
	bool GetProxyTicket(IN const Auth& auth, OUT CString &strTicket);

	// (j.jones 2015-03-30 15:46) - PLID 61540 - Checks to see if a provided
	// username & password can acquire a valid login.
	// Returns true/false on success/fail. No messaging is shown.
	bool ValidateLogin(CString strUserName, CString strPassword);
}

namespace Finder
{
	struct Code
	{
		CString label;		// name
		CString rootSource; // source (eg SNOMEDCT)
		CString ui;			// actual code

		Code()
		{}

		Code(const CString& label, const CString& rootSource, const CString& ui)
			: label(label)
			, rootSource(rootSource)
			, ui(ui)
		{}
	};


	// search for SNOMEDCT codes given a string, version, and search type
	std::vector<Code> FindSourceConcepts(const CString& str, const CString& rootSource, const CString& searchType, const CString& searchTarget, const CString& version);
	std::vector<Code> FindSourceConcepts(const CString& str, const CString& rootSource, const CString& searchType, const CString& searchTarget = "atom");
}

namespace Content
{
	// lookup the UMLS Concept ID given a code and rootSource (eg SNOMEDCT)
	CString LookupConceptID(const CString& code, const CString& rootSource);
	CString LookupConceptID(const CString& code, const CString& rootSource, const CString& version);
}

// (a.walling 2013-10-18 10:23) - PLID 59096 - UTS search should support other vocabularies beyond SNOMEDCT
namespace Metadata
{
	struct RootSource
	{
		CString name; // this is what you would use
		CString family;
		CString description;

		RootSource()
		{}

		RootSource(const CString& name, const CString& family, const CString& description)
			: name(name)
			, family(family)
			, description(description)
		{}

		friend bool operator<(const RootSource& l, const RootSource& r)
		{
			int cmp;

			cmp = l.family.Compare(r.family);
			if (cmp < 0) return true;
			if (cmp > 0) return false;

			cmp = l.name.Compare(r.name);
			if (cmp < 0) return true;
			if (cmp > 0) return false;

			cmp = l.description.Compare(r.description);
			if (cmp < 0) return true;
			if (cmp > 0) return false;

			return false;
		}
	};

	// (a.walling 2013-10-18 10:22) - PLID 59096 - Gather supported root sources
	std::vector<RootSource> GetRootSources(const CString& version);
	std::vector<RootSource> GetRootSources();
}

}
}
