#pragma once

// (j.armen 2014-03-20 09:47) - PLID 60943 - Class to describe a generic DiagCode
class DiagCode {
public:
	_bstr_t strCode;
	_bstr_t strDescription;
	long nDiagCodesID;
	long nBackColor;
};

// (j.armen 2014-03-20 09:47) - PLID 60943 - Describes a ICD-9 Code
class DiagCode9 : public DiagCode {
public:
};

// (j.armen 2014-03-20 09:47) - PLID 60943 - Describes a ICD-10 Code
class DiagCode10 : public DiagCode {
public:
	VARIANT_BOOL vbPCS;
};

// (j.armen 2014-03-20 09:47) - PLID 60943 - Diag Search Types supported by this framework
enum eDiagSearchType {
	INVALID,
	FullICD10Only,
	ManagedICD9Only,
};

// Async Search Query Structure
struct AsyncDiagSearchQuery
{
	// (a.walling 2014-04-24 12:00) - VS2013 - no using std in global headers
	std::vector<std::pair<shared_ptr<DiagCode9>, shared_ptr<DiagCode10>>> aryCodes;	// List of codes.  Populated by Run
	void Run(HWND hwndNotify, const CString& strSearchText, const eDiagSearchType& eDiagSearchType); // Bind and run this in a thread
	static UINT NotifyMessage; // Registered Message
};