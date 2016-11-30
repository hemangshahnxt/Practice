#ifndef WHERE_CLAUSE_H
#define WHERE_CLAUSE_H

#pragma once



// A self-managing string tree
class CStringTree
{
public:
	CStringTree();
	~CStringTree();
public:
	CStringTree *m_pLeft;
	CStringTree *m_pRight;
	CString m_strValue;
public:
	long GetStringCount(const CString &strCompare);
};

// Given the open parentheses/quotation mark/bracket/squiggly bracket 
// at the specified location, this finds the closing one
long FindMatching(const CString &strString, long nPos);

// Tells whether the character at the specified position is enclosed in 
// quotes or any type of brackets (including parentheses) at any depth
bool IsEmbeddedOrCommented(const CString &strString, long nPos);

// Given the string, if the operator text (strOpText) is found after nStart, nOutOp will 
// be set to nInOp and nNewPos will be set to the position of its first occurrence
void CheckFirstOp(const CString &strString, const CString &strOpText, int nInOp, int &nOutOp, long &nNewPos, long nStart = 0);

// If an unembedded operator is found after nStart in strString, its position is returned and nOutOp 
// is set to the enum value corresponding to it.  Otherwise, nOutOp is ignored and -1 is returned
long FindOperator(const CString &strString, int &nOutOp, long nStart = 0);

// Returns the length of the given enum operator
long GetOpLen(int nOp);

// Converts a where clause into a string tree (each node has a left tree, a right tree, and an operator)
void ParseWhereClauseSimple(const CString &strWhere, CStringTree &treeOut);
void ParseWhereClause(const CString &strWhere, CStringTree &treeOut);
CString TrimParentheses(const CString &strClause);

// Given a string (an operator, a field name, etc.), this returns a corresponding 
// string more conducive to human understanding.  The optional strField may specify 
// some related text to help decide what the str string might mean
CString CALLBACK HumanizeString(const CString &str, LPCTSTR strField = NULL);
CString CALLBACK GetString(const CString &str, LPCTSTR strField = NULL);

// This walks the given tree, creating a reasonable string that can be understoon by the user.  The 
// optional strField may specify some related text to help decide what the given tree might mean
CString HumanizeWhereClause(const CStringTree *pTree, LPCTSTR strField = NULL);

// This walks the given tree, creating the exact string build from the given level down
CString GetWhereClause(const CStringTree *pTree);

// CString gives a reverse find but doesn't allow NoCase, doesn't 
// allow finding substrings, and doesn't allow starting somewhere
// This function assumes all those (nStart == -1 assumes starting 
// at the end)
long ReverseFindNoCase(IN LPCTSTR strFindInString, IN LPCTSTR strFindSubString, IN long nStart = -1);

// This is a particularly useful function.  It is just like 
// ReverseFindNoCase but it always searches backward from the 
// very end and it ignores everything that's in quotes, 
// parentheses, brackets, etc.
long ReverseFindNoCaseNotEmbedded(IN LPCTSTR strQuery, IN LPCTSTR strClause, IN long nStart = -1);

enum EnumFindClause {
	fcWhere,
	fcOrderBy,
	fcGroupBy,
	fcHaving,
};

// This does a fairly reliable job of finding an any existing 
// clause inside an SQL statement (I haven't been able to break it 
// yet 11-29-2000)
// nClausePos will have the position of the requested clause inside 
// strQuery or -1 if there wasn't one
// nNext will have the position of the clause following the requested
// clause.  If there was no clause of the requested type , this is the position of 
// the clause that WOULD follow the requested clause.  If there are no 
// clauses in strQuery that WOULD follow the requested clause then this 
// is either the position of the terminating semi-colon or the 
// position of the string's terminating NULL character (i.e. its 
// value will be equal to the length of the string).
void FindClause(IN LPCTSTR strQuery, EnumFindClause eFindClause, OUT long &nClause, OUT long &nNext);

// Refer to comment for FindClause
void FindWhereClause(IN LPCTSTR strQuery, OUT long &nWhere, OUT long &nNext);

// Pass an existing sql statement and the filter to add (a filter 
// in this context is simply a WHERE clause without the "WHERE")
// This works even if there wasn't a WHERE clause in the first place
// bAndOperator only applies if the sql already had a WHERE clause
// strAliasQ only applies if the sql is a UNION query and therefore 
// must be encapsulated in an aliased query
void AddFilter(IN OUT CString &strSql, IN const CString &strAddFilter, IN BOOL bAndOperator = TRUE, IN const CString &strAliasQ = "DoNotUseAliasQ");

// Pass a sql statement and an field alias, and if the field alias 
// exists, strField will be set to the actual field definition
// This function returns TRUE if the alias was found, false if it 
// was not found
BOOL GetFieldByAlias(IN const CString &strSql, IN const CString &strAlias, OUT CString &strField);

//TES 3/10/2004: The inverse of GetFieldByAlias
BOOL GetAliasByField(IN const CString &strSql, IN const CString &strField, OUT CString &strAlias);
#endif