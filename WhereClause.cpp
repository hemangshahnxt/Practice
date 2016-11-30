#include "stdafx.h"
#include "WhereClause.h"
#include "NxStandard.h"

#define STR_PARAMETER_LIST			"={ParameterList}="

CStringTree::CStringTree()
{
	m_pLeft = NULL;
	m_pRight = NULL;
	m_strValue.Empty();
}

CStringTree::~CStringTree()
{
	if (m_pLeft) {
		delete m_pLeft;
	}
	if (m_pRight) {
		delete m_pRight;
	}
}

long CStringTree::GetStringCount(const CString &strCompare)
{
	if (this) {
		long nAns = m_pLeft->GetStringCount(strCompare) + m_pRight->GetStringCount(strCompare);
		if (m_strValue.CompareNoCase(strCompare) == 0) {
			nAns++;
		}
		return nAns;
	} else {
		return 0;
	}
}

// Important Enumerated type that defines all the possible operators
enum eOperators {
	eAndOp, eOrOp, eLikeOp, eNotOp, eGEOp, 
	eLEOp, eNEOp, eEqOp, eGTOp, eLTOp, eParenOp, 
	eCommaOp, eNoOp
};

bool IsEmbeddor(TCHAR ch)
{
	switch (ch) {
		case '\"':
		case '\'':
		case '#':
		case '(':
		case '[':
		case '{':
			return true;
			break;
		default:
			return false;
			break;
	}
}

// Given the open parentheses/quotation mark/bracket/squiggly bracket 
// at the specified location, this finds the closing one
long FindMatching(const CString &strString, long nPos)
{
	bool bInComment = false;
	bool bInDQuotes = false;
	bool bInSQuotes = false;
	bool bInPounds = false;
	long nInParens = 0;
	long nInBrackets = 0;
	long nInSquigs = 0;

	long nLen = strString.GetLength();
	long i=nPos;
	do {
		switch (strString.GetAt(i)) {
		case '\"':
			if (!bInSQuotes && !bInComment) {
				if ((i < nLen-1) && (strString.GetAt(i+1) == '\"')) {
					i++;
				} else {
					bInDQuotes = !bInDQuotes;
				}
			}
			break;
		case '\'':
			if (!bInDQuotes && !bInComment) {
				if ((i < nLen-1) && (strString.GetAt(i+1) == '\'')) {
					i++;
				} else {
					bInSQuotes = !bInSQuotes;
				}
			}
			break;
		case '#':
			if (!bInDQuotes && !bInSQuotes && !bInComment) bInPounds = !bInPounds;
			break;
		case '(':
			if (!bInDQuotes && !bInSQuotes && !bInComment) nInParens++;
			break;
		case '[':
			if (!bInDQuotes && !bInSQuotes && !bInComment) nInBrackets++;
			break;
		case '{':
			if (!bInDQuotes && !bInSQuotes && !bInComment) nInSquigs++;
			break;
		case ')':
			if (!bInDQuotes && !bInSQuotes && !bInComment) nInParens--;
			break;
		case ']':
			if (!bInDQuotes && !bInSQuotes && !bInComment) nInBrackets--;
			break;
		case '}':
			if (!bInDQuotes && !bInSQuotes && !bInComment) nInSquigs--;
			break;
		case '/':
			if (!bInDQuotes && !bInSQuotes && !bInComment) {
				if ((i+1) < nLen && strString.GetAt(i+1) == '*') {
					bInComment = true;
					i++;
				}
			}
			break;
		case '*':
			if (bInComment) {
				if ((i+1) < nLen && strString.GetAt(i+1) == '/') {
					bInComment = false;
					i++;
				}
			}
			break;
		default:
			break;
		}
		
		// If we've reached stasis return our current position
		if ((!bInDQuotes) && (!bInSQuotes) && (!bInPounds) && (!bInComment) && (nInParens == 0) && (nInBrackets == 0) && (nInSquigs == 0)) {
			return i;
		}
		
		// Otherwise carry on
		i++;
	} while (i<nLen);

	// Failure
	return -1;
}

// Tells whether the character at the specified position is enclosed in 
// quotes or any type of brackets (including parentheses) at any depth
bool IsEmbeddedOrCommented(const CString &strString, long nPos)
{
	bool bInComments = false;
	bool bInDQuotes = false;
	bool bInSQuotes = false;
	long bInPounds = false;
	long nInParens = 0;
	long nInBrackets = 0;
	long nInSquigs = 0;

	for (long i=0; i<nPos; i++) {
		switch (strString.GetAt(i)) {
		case '\"':
			if (!bInSQuotes && !bInComments) bInDQuotes = !bInDQuotes;
			break;
		case '\'':
			if (!bInDQuotes && !bInComments) bInSQuotes = !bInSQuotes;
			break;
		case '#':
			if (!bInDQuotes && !bInSQuotes && !bInComments) {
				// (a.walling 2008-10-03 17:29) - PLID 31589 - ASSERTion if you try to pass a signed char to any ::is* functions.
				if (!bInPounds && (i<strString.GetLength()-1) && isdigit(unsigned char(strString.GetAt(i+1)))) {
					// We're not in pounds and the next character is a number so we're starting pounds
					bInPounds = true;
				} else if (bInPounds) {
					// We're in pounds so if we encounter any pound sign, get us out
					// (a.walling 2008-10-03 17:29) - PLID 31589 - ASSERTion if you try to pass a signed char to any ::is* functions.
					ASSERT((i>0) && isdigit(unsigned char(strString.GetAt(i-1)))); // Shouldn't the character just prior to the # be a digit?
					bInPounds = false;
				}
			}
			break;
		case '(':
			if (!bInDQuotes && !bInSQuotes && !bInComments) nInParens++;
			break;
		case '[':
			if (!bInDQuotes && !bInSQuotes && !bInComments) nInBrackets++;
			break;
		case '{':
			if (!bInDQuotes && !bInSQuotes && !bInComments) nInSquigs++;
			break;
		case ')':
			if (!bInDQuotes && !bInSQuotes && !bInComments) nInParens--;
			break;
		case ']':
			if (!bInDQuotes && !bInSQuotes && !bInComments) nInBrackets--;
			break;
		case '}':
			if (!bInDQuotes && !bInSQuotes && !bInComments) nInSquigs--;
			break;
		case '/':
			if (!bInDQuotes && !bInSQuotes && !bInComments) {
				if ((i+1) < nPos && strString.GetAt(i+1) == '*') {
					bInComments = true;
					i++;
				}
			}
			break;
		case '*':
			if (bInComments) {
				if ((i+1) < nPos && strString.GetAt(i+1) == '/') {
					bInComments = false;
					i++;
				}
			}
			break;
		default:
			break;
		}
	}
	if ((bInDQuotes) || (bInSQuotes) || (bInPounds) || (bInComments) || (nInParens > 0) || (nInBrackets > 0) || (nInSquigs > 0)) {
		return true;
	} else {
		return false;
	}
}

// Given the string, if the operator text (strOpText) is found after nStart, nOutOp will 
// be set to nInOp and nNewPos will be set to the position of its first occurrence
void CheckFirstOp(const CString &strString, const CString &strOpText, int nInOp, int &nOutOp, long &nNewPos, long nStart /*= 0*/)
{
	int nPos = strString.Find(strOpText, nStart);
	while ((nPos >= 0) && IsEmbeddedOrCommented(strString, nPos)) {
		nPos = strString.Find(strOpText, nPos + 1);
	}

	if (nPos >= 0) {
		if ((nNewPos == -1) || (nPos < nNewPos)) {
			nNewPos = nPos;
			nOutOp = nInOp;
		}
	}
}

// Returns the length of the given enum operator
long GetOpLen(int nOp)
{
	switch (nOp) {
	case eLikeOp:
		return 4;
		break;
	case eAndOp:
	case eNotOp:
		return 3;
		break;
	case eOrOp:
	case eGEOp:
	case eLEOp:
	case eNEOp:
		return 2;
		break;
	case eGTOp:
	case eLTOp:
	case eParenOp:
	case eCommaOp:
	case eEqOp:
		return 1;
		break;
	case eNoOp:
	default:
		return 0;
		break;
	}
}

#define CHECK_FIRST_OP(strOpText, eOpNum)			CheckFirstOp(strString, (strOpText), (eOpNum), nOp, nPos, nStart)

// If an unembedded conjunction ("And" "Or" ",") is found after nStart in strString, its position is returned and nOutOp 
// is set to the enum value corresponding to it.  Otherwise, nOutOp is ignored and -1 is returned
long FindConjunction(const CString &strString, int &nOutConj, long nStart /*= 0*/)
{
	long nPos = -1;
	int nOp = eNoOp;
	CHECK_FIRST_OP("AND", eAndOp);
	CHECK_FIRST_OP("ANd", eAndOp);
	CHECK_FIRST_OP("AnD", eAndOp);
	CHECK_FIRST_OP("And", eAndOp);
	CHECK_FIRST_OP("aND", eAndOp);
	CHECK_FIRST_OP("aNd", eAndOp);
	CHECK_FIRST_OP("anD", eAndOp);
	CHECK_FIRST_OP("and", eAndOp);
	CHECK_FIRST_OP("OR", eOrOp);
	CHECK_FIRST_OP("Or", eOrOp);
	CHECK_FIRST_OP("oR", eOrOp);
	CHECK_FIRST_OP("or", eOrOp);

	nOutConj = nOp;
	return nPos;
}

// If an unembedded operator is found after nStart in strString, its position is returned and nOutOp 
// is set to the enum value corresponding to it.  Otherwise, nOutOp is ignored and -1 is returned
long FindOperator(const CString &strString, int &nOutOp, long nStart /*= 0*/)
{
	long nPos = -1;
	int nOp = eNoOp;
	CHECK_FIRST_OP("AND", eAndOp);
	CHECK_FIRST_OP("ANd", eAndOp);
	CHECK_FIRST_OP("AnD", eAndOp);
	CHECK_FIRST_OP("And", eAndOp);
	CHECK_FIRST_OP("aND", eAndOp);
	CHECK_FIRST_OP("aNd", eAndOp);
	CHECK_FIRST_OP("anD", eAndOp);
	CHECK_FIRST_OP("and", eAndOp);
	CHECK_FIRST_OP("OR", eOrOp);
	CHECK_FIRST_OP("Or", eOrOp);
	CHECK_FIRST_OP("oR", eOrOp);
	CHECK_FIRST_OP("or", eOrOp);
	CHECK_FIRST_OP("LIKE", eLikeOp);
	CHECK_FIRST_OP("LIKe", eLikeOp);
	CHECK_FIRST_OP("LIkE", eLikeOp);
	CHECK_FIRST_OP("LIke", eLikeOp);
	CHECK_FIRST_OP("LiKE", eLikeOp);
	CHECK_FIRST_OP("LiKe", eLikeOp);
	CHECK_FIRST_OP("LikE", eLikeOp);
	CHECK_FIRST_OP("Like", eLikeOp);
	CHECK_FIRST_OP("lIKE", eLikeOp);
	CHECK_FIRST_OP("lIKe", eLikeOp);
	CHECK_FIRST_OP("lIkE", eLikeOp);
	CHECK_FIRST_OP("lIke", eLikeOp);
	CHECK_FIRST_OP("liKE", eLikeOp);
	CHECK_FIRST_OP("liKe", eLikeOp);
	CHECK_FIRST_OP("likE", eLikeOp);
	CHECK_FIRST_OP("like", eLikeOp);
	CHECK_FIRST_OP("NOT", eNotOp);
	CHECK_FIRST_OP("NOt", eNotOp);
	CHECK_FIRST_OP("NoT", eNotOp);
	CHECK_FIRST_OP("Not", eNotOp);
	CHECK_FIRST_OP("nOT", eNotOp);
	CHECK_FIRST_OP("nOt", eNotOp);
	CHECK_FIRST_OP("noT", eNotOp);
	CHECK_FIRST_OP("not", eNotOp);
	CHECK_FIRST_OP(">=", eGEOp);
	CHECK_FIRST_OP("<=", eLEOp);
	CHECK_FIRST_OP("=>", eGEOp);
	CHECK_FIRST_OP("=<", eLEOp);
	CHECK_FIRST_OP("<>", eNEOp);
	CHECK_FIRST_OP("=", eEqOp);
	CHECK_FIRST_OP(">", eGTOp);
	CHECK_FIRST_OP("<", eLTOp);
	CHECK_FIRST_OP("(", eParenOp);
	CHECK_FIRST_OP(",", eCommaOp);

	nOutOp = nOp;
	return nPos;
}

/*// TODO: For this function to work properly, in needs to handle when the sql text has comments in it
CString TrimParentheses(const CString &strClause)
{
	CString strAns(strClause);
	strAns.TrimLeft(); strAns.TrimRight();
	bool bContinue = true;
	while (bContinue) {
		bContinue = false;
		if (!strAns.IsEmpty() && strAns.GetAt(0) == '(') {
			long nClose = FindMatching(strAns, 0);
			if (nClose > 0) {
				if (nClose == (strAns.GetLength() - 1)) {
					// The entire thing is surrounded by parentheses so parse as though it were not
					strAns = strAns.Mid(1, nClose-1);
					bContinue = true;
				}
			}
		}
	}
	return strAns;
}

// Converts a where clause into a string tree (each node has a left tree, a right tree, and an operator)
void ParseWhereClauseSimple(const CString &strWhere, CStringTree &treeOut)
{
	if (treeOut.m_pLeft) { delete treeOut.m_pLeft; treeOut.m_pLeft = NULL; }
	if (treeOut.m_pRight) { delete treeOut.m_pRight; treeOut.m_pRight = NULL; }
	CString strAns = strWhere;
	long nPos = -1;
	int nOp;
	nPos = FindConjunction(strAns, nOp, 0);
	if (nPos >= 0) {
		long nOpLen = GetOpLen(nOp);
		treeOut.m_strValue = strAns.Mid(nPos, nOpLen);
		
		if (treeOut.m_pLeft) delete treeOut.m_pLeft;
		treeOut.m_pLeft = new CStringTree;
		ParseWhereClauseSimple(strAns.Mid(0, nPos), *(treeOut.m_pLeft));

		if (treeOut.m_pRight) delete treeOut.m_pRight;
		treeOut.m_pRight = new CStringTree;
		ParseWhereClauseSimple(strAns.Mid(nPos + nOpLen), *(treeOut.m_pRight));
	} else {
		treeOut.m_strValue = TrimParentheses(strAns);
	}
}
*/

// Converts a where clause into a string tree (each node has a left tree, a right tree, and an operator)
/*// TODO: For this function to work properly, in needs to handle when the sql text has comments in it
void ParseWhereClause(const CString &strWhere, CStringTree &treeOut)
{
	if (treeOut.m_pLeft) { delete treeOut.m_pLeft; treeOut.m_pLeft = NULL; }
	if (treeOut.m_pRight) { delete treeOut.m_pRight; treeOut.m_pRight = NULL; }
	CString strAns = ExtractWhereClause(strWhere);
	long nPos = -1;
	int nOp;
	nPos = FindOperator(strAns, nOp);
	if (nPos >= 0) {
		switch (nOp) {
		case eParenOp:
			{
				long nOpen = nPos;
				long nClose = FindMatching(strAns, nOpen);
				if (nClose > nOpen) {
					if (nOpen == 0) {
						if (nClose == (strAns.GetLength() - 1)) {
							// The entire thing is surrounded by parentheses so parse as though it were not
							ParseWhereClause(strAns.Mid(1, strAns.GetLength()-2), treeOut);
						} else {
							// Parse left side
							treeOut.m_pLeft = new CStringTree;
							ParseWhereClause(strAns.Mid(nOpen+1, nClose-nOpen-1), *(treeOut.m_pLeft));

							// Find operator
							nPos = FindOperator(strAns, nOp, nClose+1);
							if (nPos >= 0) {
								treeOut.m_strValue = strAns.Mid(nPos, GetOpLen(nOp));

								// Parse right side
								treeOut.m_pRight = new CStringTree;
								ParseWhereClause(strAns.Mid(nPos+treeOut.m_strValue.GetLength()), *(treeOut.m_pRight));
							}
						}
					} else {
						int nNextOp;
						long nNextOpPos = FindOperator(strAns, nNextOp, nClose+1);
						if (nNextOpPos >= 0) {
							// Parse left side
							treeOut.m_pLeft = new CStringTree;
							ParseWhereClause(strAns.Left(nClose+1), *(treeOut.m_pLeft));

							// Find operator
							long nNextOpLen = GetOpLen(nNextOp);
							treeOut.m_strValue = strAns.Mid(nNextOpPos, nNextOpLen);

							// Parse right side
							treeOut.m_pRight = new CStringTree;
							ParseWhereClause(strAns.Mid(nNextOpPos+nNextOpLen), *(treeOut.m_pRight));
						} else {
							// Parse left side
							treeOut.m_pLeft = new CStringTree;
							ParseWhereClause(strAns.Left(nOpen), *(treeOut.m_pLeft));

							// Find operator
							treeOut.m_strValue = STR_PARAMETER_LIST;

							// Parse right side
							treeOut.m_pRight = new CStringTree;
							ParseWhereClause(strAns.Mid(nOpen+1, nClose-nOpen-1), *(treeOut.m_pRight));
						}
					}
				}
			}
			break;
		case eAndOp:
		case eOrOp:
		case eLikeOp:
		case eGEOp:
		case eLEOp:
		case eNEOp:
		case eEqOp:
		case eGTOp:
		case eLTOp:
		case eCommaOp:
			{
				// Find next conjunctive operator ("," "AND" "OR")
				int nTrueOp;
				long nTrueOpPos = FindConjunction(strAns, nTrueOp, 0);
				if (nTrueOpPos == -1) {
					nTrueOp = nOp;
					nTrueOpPos = nPos;
				}

				long nOpLen = GetOpLen(nTrueOp);
				treeOut.m_strValue = strAns.Mid(nTrueOpPos, nOpLen);
				
				if (treeOut.m_pLeft) delete treeOut.m_pLeft;
				treeOut.m_pLeft = new CStringTree;
				ParseWhereClause(strAns.Mid(0, nTrueOpPos), *(treeOut.m_pLeft));

				if (treeOut.m_pRight) delete treeOut.m_pRight;
				treeOut.m_pRight = new CStringTree;
				ParseWhereClause(strAns.Mid(nTrueOpPos + nOpLen), *(treeOut.m_pRight));
			}
			break;
		default:
			treeOut.m_strValue = strAns;
			break;
		}
	} else {
		treeOut.m_strValue = strAns;
	}
}
*/


typedef CString (CALLBACK *LPTRAVERSECALLBACK) (const CString &str, LPCTSTR strField /*= NULL*/);

CString TraverseWhereClause(const CStringTree *pTree, LPTRAVERSECALLBACK pCallback, LPCTSTR strField = NULL);

CString TraverseWhereClause(const CStringTree *pTree, LPTRAVERSECALLBACK pCallback, LPCTSTR strField /*= NULL*/)
{
	CString strAns;
	if (pTree) {
		if (pTree->m_strValue == STR_PARAMETER_LIST) {
			strAns = TraverseWhereClause(pTree->m_pLeft, pCallback) + "(" + TraverseWhereClause(pTree->m_pRight, pCallback) + ")";
		} else if (pTree->m_strValue == ",") {
			strAns = TraverseWhereClause(pTree->m_pLeft, pCallback) + ", " + TraverseWhereClause(pTree->m_pRight, pCallback);
		} else {
			// Get trimmed strings
			CString strLeft, strOp, strRight;
			if ((pTree->m_pLeft && pTree->m_pLeft->m_strValue.CompareNoCase("And")==0) ||
				 (pTree->m_pLeft && pTree->m_pLeft->m_strValue.CompareNoCase("Or")==0)) {
				strLeft = "(" + TraverseWhereClause(pTree->m_pLeft, pCallback) + ")";
			} else {
				strLeft = TraverseWhereClause(pTree->m_pLeft, pCallback);
			}
			strOp = (*pCallback)(pTree->m_strValue, strField);
			strRight = TraverseWhereClause(pTree->m_pRight, pCallback, strLeft);
			strLeft.TrimLeft(); strLeft.TrimRight();
			strOp.TrimLeft(); strOp.TrimRight();
			strRight.TrimLeft(); strOp.TrimRight();
			
			strAns.Empty();
			if (strLeft.GetLength()) strAns += (strAns.IsEmpty() ? "" : " ") + /*CString("(") + */strLeft;
			if (strOp.GetLength()) strAns += (strAns.IsEmpty() ? "" : " ") + strOp;
			if (strRight.GetLength()) strAns += (strAns.IsEmpty() ? "" : " ") + strRight /*+ ")"*/;
		}
	}
	return strAns;
}

#define HUMANIZE_STRING_START()						if (false) {
#define HUMANIZE_STRING(strString, strHuman)		} else if (strAns.CompareNoCase(strString) == 0) { return (strHuman);
#define HUMANIZE_STRING_END()				} else { if (strAns.GetLength()) { strAns.MakeLower(); strAns.SetAt(0, toupper(strAns.GetAt(0)));} return strAns; }

// Given a string (an operator, a field name, etc.), this returns a corresponding 
// string more conducive to human understanding.  The optional strField may specify 
// some related text to help decide what the str string might mean
/* // TODO: For this function to work properly, in needs to handle when the sql text has comments in it
CString CALLBACK HumanizeString(const CString &str, LPCTSTR strField /*= NULL* /)
{
	CString strAns(str);
	strAns.TrimLeft(" "); strAns.TrimRight(" ;");

	if (strField) {
		if (stricmp(strField, "Gender") == 0) {
			if ((str == "\"1\"") || (str == "'1'") || (str == "1")) {
				return "Male";
			} else if ((str == "\"2\"") || (str == "'2'") || (str == "2")) {
				return "Female";
			}
		}
	}
	
	if (strAns.GetLength() > 0) {
		TCHAR ch = strAns.GetAt(0);
		// Should we handle sql comments?
		if (IsEmbeddor(ch) && ch != '\'' && ch != '\"') {
			int nClose = FindMatching(strAns, 0);
			if (nClose >= 0) {
				strAns.Delete(nClose);
				strAns.Delete(0);
			}
		}
	}

	HUMANIZE_STRING_START();
	
	HUMANIZE_STRING("Id", "ID");
	HUMANIZE_STRING(">", "is greater than");
	HUMANIZE_STRING("<", "is less than");
	HUMANIZE_STRING(">=", "is greater than or equal to");
	HUMANIZE_STRING("=>", "is greater than or equal to");
	HUMANIZE_STRING("<=", "is less than or equal to");
	HUMANIZE_STRING("=<", "is less than or equal to");
	HUMANIZE_STRING("=", "is");
	HUMANIZE_STRING("<>", "is not");
	HUMANIZE_STRING("Is", "is");
	HUMANIZE_STRING("Not", "not");
	HUMANIZE_STRING("Null", "null");
	HUMANIZE_STRING("Like", "is like");
	HUMANIZE_STRING("And", "and");
	HUMANIZE_STRING("Or", "or");
	HUMANIZE_STRING("Not", "not");
	HUMANIZE_STRING(STR_PARAMETER_LIST, "");

	HUMANIZE_STRING_END();
}

CString HumanizeWhereClause(const CStringTree *pTree, LPCTSTR strField /*= NULL* /)
{
	return TraverseWhereClause(pTree, HumanizeString);
}
*/

CString CALLBACK GetString(const CString &str, LPCTSTR strField /*= NULL*/)
{
	return str;
}

CString GetWhereClause(const CStringTree *pTree)
{
	return TraverseWhereClause(pTree, GetString);
}

// CString gives a reverse find but doesn't allow NoCase, doesn't 
// allow finding substrings, and doesn't allow starting somewhere
// This function assumes all those (nStart == -1 assumes starting 
// at the end)
long ReverseFindNoCase(IN LPCTSTR strFindInString, IN LPCTSTR strFindSubString, IN long nStart /*= -1*/)
{
	long nInLen = strlen(strFindInString);
	long nSubLen = strlen(strFindSubString);
	// Obviously we have no chance of finding strFindSubString INSIDE 
	// strFindIn unless the length of strFindSubString is less than or 
	// equal to the length of strFindIn
	if (nSubLen <= nInLen) {
		// Initialize nPos (if nLastPos is not given, just use the last 
		// possible character in the string; if it IS given, use it but 
		// subtract the length of the substring from it)
		long nPos;
		if (nStart == -1) {
			nPos = nInLen - nSubLen;
		} else {
			nPos = nStart;
		}
		// Loop backwards starting at nPos until either you find the 
		// substring, or you get to the beginning of the string
		for ( ; nPos >= 0; nPos--) {
			if (strnicmp(strFindInString+nPos, strFindSubString, nSubLen) == 0) {
				// Found it
				return nPos;
			}
		}
		// If we made it here, the string wasn't found
		return -1;
	} else {
		// If strFindSubString is longer than strInSubString
		return -1;
	}
}

// This is a particularly useful function.  It is just like 
// ReverseFindNoCase but it always searches backward from the 
// very end and it ignores everything that's in quotes, 
// parentheses, brackets, etc.
long ReverseFindNoCaseNotEmbedded(IN LPCTSTR strQuery, IN LPCTSTR strClause, IN long nStart /*= -1*/)
{
	// Find the last occurrence of the string strClause
	long nClauseLen = strlen(strClause);
	long nPosFirst = ReverseFindNoCase(strQuery, strClause, nStart);
	while (nPosFirst >= 0 && IsEmbeddedOrCommented(strQuery, nPosFirst)) {
		// Keep searching backwards until we find one that's not inside quotes or anything
		nPosFirst = ReverseFindNoCase(strQuery, strClause, nPosFirst - nClauseLen);
	}
	return nPosFirst;
}

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
void FindClause(IN LPCTSTR strQuery, EnumFindClause eFindClause, OUT long &nClause, OUT long &nNext)
{
	// Find the beginning of each clause (this part works pretty 
	// well but I don't think it's one hundred percent perfect)
	long nPosWhere = ReverseFindNoCaseNotEmbedded(strQuery, "WHERE ");
	long nPosOrderBy = ReverseFindNoCaseNotEmbedded(strQuery, "ORDER BY ");
	long nPosGroupBy = ReverseFindNoCaseNotEmbedded(strQuery, "GROUP BY ");
	long nPosHaving = ReverseFindNoCaseNotEmbedded(strQuery, "HAVING ");
	long nPosSemiColon = ReverseFindNoCaseNotEmbedded(strQuery, ";");
	
	// Initialize nPosNext to the size of the string
	long nPosNext = strlen(strQuery);

	long nPosCheck;
	switch (eFindClause) {
	case fcWhere:
		nPosCheck = nPosWhere;
		break;
	case fcOrderBy:
		nPosCheck = nPosOrderBy;
		break;
	case fcGroupBy:
		nPosCheck = nPosGroupBy;
		break;
	case fcHaving:
		nPosCheck = nPosHaving;
		break;
	default:
		nPosCheck = -1;
		return;
	}
	// See if anything was found past the requested clause
	// The order of these tests doesn't matter
	if (nPosOrderBy		> nPosCheck && nPosOrderBy		< nPosNext) nPosNext = nPosOrderBy;
	if (nPosGroupBy		> nPosCheck && nPosGroupBy		< nPosNext) nPosNext = nPosGroupBy;
	if (nPosHaving		> nPosCheck && nPosHaving		< nPosNext) nPosNext = nPosHaving;
	if (nPosSemiColon	> nPosCheck && nPosSemiColon	< nPosNext) nPosNext = nPosSemiColon;
	
	nClause = nPosCheck;
	nNext = nPosNext;
}

// Refer to comment for FindClause
void FindWhereClause(IN LPCTSTR strQuery, OUT long &nWhere, OUT long &nNext)
{
	FindClause(strQuery, fcWhere, nWhere, nNext);
}

// Pass an existing sql statement and the filter to add (a filter 
// in this context is simply a WHERE clause without the "WHERE")
// This works even if there wasn't a WHERE clause in the first place
// bAndOperator only applies if the sql already had a WHERE clause
void AddFilter(IN OUT CString &strSql, IN const CString &strAddFilter, IN BOOL bAndOperator /*= TRUE*/, IN const CString &strAliasQ /*= "DoNotUseAliasQ"*/)
{
	// First if this is a UNION query it is the one case we know of that must be enclosed 
	// in an Aliased query (i.e. "SELECT * FROM (strSql) AliasQ WHERE strAddFilter")
	if (ReverseFindNoCaseNotEmbedded(strSql, "UNION ") != -1) {
		// Found UNION so it must be a union query
		strSql.TrimRight(" ;");
		strSql = "SELECT * FROM (" + strSql + ") " + strAliasQ;
	}
	// Find the where clause boundaries
	long nWhereClause, nNextClause;
	FindWhereClause(strSql, nWhereClause, nNextClause);
	// Get the where clause using those boundaries
	// (a.walling 2008-10-03 17:29) - PLID 31589 - ASSERTion if you try to pass a signed char to any ::is* functions.
	CString strPostSpace = (isspace(unsigned char(strSql.GetAt(nNextClause-1))) ? " " : "");
	if (nWhereClause == -1) {
		// There is no existing WHERE clause
		// (a.walling 2008-10-03 17:29) - PLID 31589 - ASSERTion if you try to pass a signed char to any ::is* functions.
		CString strPreSpace = (isspace(unsigned char(strSql.GetAt(nNextClause-1))) ? "" : " ");
		strSql = strSql.Left(nNextClause) + strPreSpace + "WHERE (" + strAddFilter + ")" + strPostSpace + strSql.Mid(nNextClause);
	} else {
		// There is an existing WHERE clause
		strSql = strSql.Left(nWhereClause) + // Up to current WHERE clause
			"WHERE (" + strSql.Mid(nWhereClause+6, nNextClause-nWhereClause-6) + ") " + // Add existing WHERE clause
			(bAndOperator?"AND":"OR") + " (" + strAddFilter + ")" + strPostSpace + // Add new filter
			strSql.Mid(nNextClause); // Add the rest of the sql
	}
}

BOOL GetFieldByAlias(IN const CString &strSql, IN const CString &strAlias, OUT CString &strField)
{
	long nAsAlias = ReverseFindNoCaseNotEmbedded(strSql, " AS " + strAlias);
	if (nAsAlias >= 0) {
		// Alias was found
		// Search for the first non-embedded comma prior to the "AS Alias" clause
		long nPreComma = ReverseFindNoCaseNotEmbedded(strSql, ",", nAsAlias);
		if (nPreComma >= 0) {
			// The comma was found so we have our limits
			strField = strSql.Mid(nPreComma+1, nAsAlias - (nPreComma+1));
			strField.TrimLeft("\t\r\n ");
			strField.TrimRight("\t\r\n ");
			return TRUE;
		} else {
			// The comma wasn't found so we're assuming this is the first field
			long nPreSelect = ReverseFindNoCaseNotEmbedded(strSql, "SELECT ", nAsAlias);
			if (nPreSelect >= 0) {
				// Found the SELECT
				strField = strSql.Mid(nPreSelect+7, nAsAlias - (nPreSelect+7));
				strField.TrimLeft("\t\r\n ");
				strField.TrimRight("\t\r\n ");
				return TRUE;
			} else {
				// The given query is of some unexpected format
				ASSERT(FALSE);
				return FALSE;
			}
		}
	} else {
		// Alias wasn't found
		return FALSE;
	}
}

BOOL GetAliasByField(IN const CString &strSql, IN const CString &strField, OUT CString &strAlias)
{
	long nFieldAs = ReverseFindNoCaseNotEmbedded(strSql, strField + " AS ");
	if (nFieldAs >= 0) {
		// Field was found
		// Search for the first non-embedded comma after the "Field AS" clause
		long nPreComma = strSql.Find(",", nFieldAs);
		//Also, have we hit our FROM clause?
		int nFrom = strSql.Find("FROM ", nFieldAs);
		if (nPreComma >= 0) {
			if(nFrom != -1 && strSql.Find("FROM ", nFieldAs) < nPreComma) {
				//This must have been the last field
				strAlias = strSql.Mid((nFieldAs + strField.GetLength() + 4), nFrom-(nFieldAs + strField.GetLength() + 4));
			}
			else {
				// The comma was found so we have our limits
				strAlias = strSql.Mid((nFieldAs + strField.GetLength() + 4), nPreComma-(nFieldAs + strField.GetLength() + 4));
			}
			strAlias.TrimLeft("\t\r\n ");
			strAlias.TrimRight("\t\r\n ");
			return TRUE;
		} else {
			// The comma wasn't found so we're assuming this is the last field
			if (nFrom >= 0) {
				// Found the FROM
				strAlias = strSql.Mid((nFieldAs + strField.GetLength() + 4), nFrom-(nFieldAs + strField.GetLength() + 4));
				strAlias.TrimLeft("\t\r\n ");
				strAlias.TrimRight("\t\r\n ");
				return TRUE;
			} else {
				// The given query is of some unexpected format
				ASSERT(FALSE);
				return FALSE;
			}
		}
	} else {
		// Alias wasn't found
		return FALSE;
	}
}