// NxExpression.h: interface for the NxMath class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NXEXPRESSION_H__15087668_066D_4E8E_9A68_08412395DA9E__INCLUDED_)
#define AFX_NXEXPRESSION_H__15087668_066D_4E8E_9A68_08412395DA9E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


enum EOperatorPrecedence
{
	opInvalid = 0,
	opAddSubtract,
	opMultiplyDivide,
	opExponent,
	opParentheses,
};

// (z.manning, 05/23/2008) - PLID 30155 - Created NxExpression class
class CNxExpression
{
public:
	// (r.gonet 08/03/2012) - PLID 30514 - Contains information about a cycle found during flattening.
	struct HasCycle {
		// The field that as assigned the expression containing the field that completes the cycle.
		//  ie for the cycle C1 -> C2 -> C3 -> C1. The strCycleContainer is C3.
		CString strCycleContainer;
		// The field that completes the cycle. In the example above, this is C1.
		CString strCycleField;

		HasCycle(CString _strCycleContainer, CString _strCycleField) 
			: strCycleContainer(_strCycleContainer), strCycleField(_strCycleField)
		{ };
	};

	// (r.gonet 08/03/2012) - PLID 30514 - Contains information about an unknown symbol found during flattening.
	//  This occurs when Flatten is called on a field that is not in m_mapFormulas.
	struct UnknownSymbol {
		// The field that is assigned the expression containing the symbol that is unknown.
		CString strContainerField;
		// The symbol that we could not expand.
		CString strUnknownSymbol;

		UnknownSymbol(CString _strContainerField, CString _strUnknownSymbol)
			: strContainerField(_strContainerField), strUnknownSymbol(_strUnknownSymbol)
		{ };
	};

	CNxExpression(CString strInfixExpression);
	// (r.gonet 08/03/2012) - PLID 30514 - constructs a new expression with strInfixExpression
	//  being assigned to strField, ie strField = strInfixExpression.
	CNxExpression(CString strField, CString strInfixExpression);

	void SetExpression(const CString strInfixExpression);

	void AddOperandReplacement(CString strOld, CString strNew);
	// (r.gonet 08/03/2012) - PLID 30514 - Adds a formula to the list of all known formulas.
	//  strExpression is assigned to strField, ie strField = strExpression.
	//  Used during flattening of expressions.
	void AddFormula(CString strField, CString strExpression);

	double Evaluate();
	double Evaluate(BOOL bIsParseFunction, BOOL bSkipParseBool, CString strInExpression); // (j.luckoski 2012-08-02 10:36) - PLID 51900

	CString GetError();

	// (z.manning, 05/30/2008) - PLID 16443 - Returns the infix representation of the expression,
	// including any operand replacements that may have been made.
	CString GetInfixExpression();

	// (z.manning, 05/30/2008) - PLID 16443 - Returns true if the given operand is in the expression,
	// false otherwise.
	BOOL HasOperand(CString strOperand);

	void GetTokens(OUT CStringArray &arystrTokens);

protected:

	struct InfixToken
	{
		CString strToken;
		int nStartPosition;
	};

	// (r.gonet 08/03/2012) - PLID 30514 - Just the left hand side that is assigned m_strInfixExpression,
	//  I found this necessary for detecting cycles.
	CString m_strField;
	CString m_strInfixExpression;
	CStringArray m_arystrPostfixExpression;
	// (z.manning, 05/30/2008) - PLID 16443 - Keep track of the the position of all tokens in the
	// normal (infix) expression;
	CArray<InfixToken, InfixToken&> m_aryInfixTokens;

	CString m_strFirstParseError;

	BOOL m_bParsed;

	CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapOperands;

	// (r.gonet 08/03/2012) - PLID 30514 - The knowledgebase of all left-hand-sides to expressions so we can flatten the expression.
	CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapFormulas;
	// (r.gonet 08/03/2012) - PLID 30514 - The cache of already flattened expressions. Takes a field name and returns the flattened expression.
	CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapCache;

	BOOL Parse(BOOL bReplaceOperands);
	BOOL Parse(BOOL bReplaceOperands, BOOL bIsParseFunction, BOOL bSkipParseBool, BOOL bUserEvaluate,CString strInExpression); // (j.luckoski 2012-08-02 10:37) - PLID 51900

	void SetError(const CString strError);

	BOOL IsOperator(char ch);
	BOOL IsValidOperandChar(char ch);

	EOperatorPrecedence GetOperatorPrecedence(char chOperator);

	double PerformOperation(_variant_t varLeftHandSide, char chOperator, _variant_t varRightHandSide);

	// (r.gonet 08/03/2012) - PLID 30514 - Documentation for Flatten
	// Flatten takes the m_strInfixExpression and 
	//  recursively substitutes for each formula filled field referenced in m_strInfixExpression
	//  its corresponding expression until the point where all referenced fields are fields that
	//  are not formula filled. Fields that are not formula filled are filled by the user. By
	//  referenced fields, I mean identifiers that stand for other expressions, such as C1, C2, or R2.
	//  By referenced, I mean either directly, literally in the m_strInfixExpression, or indirectly, 
	//  not literally in the m_strInfixExpression but somewhere down the line after performing enough
	//  substitution of fields for their expressions. 
	// The Flatten algorithm references the m_mapFormulas map to know which symbols that appear in the
	//  m_strInfixExpression are 1) fields and 2) have formulas it can substitute. The recursiveness
	//  could be exponential in complexity but for the fact that once some referenced field's expression
	//  is flattened, then the flattened expression is added to m_mapCache, so we just need to look up
	//  what we alreday figured out if we encounter that referenced field again elsewhere.
	// Flatten detects cycles during its recursive descent through referenced fields by maintaining
	//  a visited array and by looking through that each time it sees a new referenced field
	//  to see if the algorithm is already in the process of flattening that symbol. Thus it avoids
	//  infinite loops. If a cycle is detected, a HasCycle struct is thrown with information on where
	//  the cycle occurred.
	// It can also throw a UnknownSymbol struct if it is asked to flatten a symbol that it doesn't have
	//  an m_mapFormulas entry for, though as implemented currently, this doesn't ever occur.
	// If an expression being flattened contains fields that are not contained in m_mapFormulas,
	//  Flatten will ignore these since it doesn't know they are fields, leaving them in the resulting
	//  expression. This is desirable in EMR tables for non-formula filled fields and for transform-
	//  only fields.
	// Example: 
	//  We want to flatten the formula C1 = C2 + C3 * C4
	//  Our m_mapFormulas contains 
	//   C2 = C5 - C3 and
	//   C3 = C6 / 2
	//  Flattening C1 gets us
	//   C1 = (C5 - (C6 / 2)) + (C6 / 2) * C4
	//  Notice that the second occurence of C3 makes use of the m_mapCache. All of remaining
	//  fields here will presumably be replaced during Evaluate() by elements of m_mapOperands.

	// (r.gonet 08/03/2012) - PLID 30514 - lattens m_strInfixExpression. Uses the m_strField value if it is set to assign the m_strInfixExpression
	//  to so circular dependencies of the form m_strField -> C2 -> C5 -> m_strField can be detected.
	//  If m_strField is blank, then a temporary left-hand-side field is created for use in flattening.
	// Returns the flattened expression.
	CString Flatten();
	// (r.gonet 08/03/2012) - PLID 30514 - Flattens strExpression. Assigns strExpression to strField so that strField = strExpression for
	//  the purpose of finding circular dependencies of the form strField -> C2 -> C5 -> strField.
	// Returns the flattened expression.
	CString Flatten(CString strField, CString strExpression);
	// (r.gonet 08/03/2012) - PLID 30514 - Should not be called directly. Recursively called function that flattens the expression assigned 
	//  to strField. Maintains a visited array containing the fields being flattened during this descent in
	//  order to detect circular dependencies. aryVisited should contain no elements upon the first call of
	//  this function and will contain no elements upon its top most return.
	// Returns the flattened expression assigned to strField.
	CString Flatten(CString strField, CArray<CString, CString> &aryVisited);
};

#endif // !defined(AFX_NXEXPRESSION_H__15087668_066D_4E8E_9A68_08412395DA9E__INCLUDED_)
