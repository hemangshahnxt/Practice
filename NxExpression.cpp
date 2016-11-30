// NxExpression.cpp: implementation of the NxExpression class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NxExpression.h"
#include "MiscSystemUtils.h" // (r.gonet 08/03/2012) - PLID 30514 - For NewUUID

// (z.manning, 05/23/2008) - PLID 30155 - Created NxExpression class

CNxExpression::CNxExpression(CString strInfixExpression)
{
	// (r.gonet 08/03/2012) - PLID 30514 - Initialize the field to nothing since apparently we don't care about it.
	m_strField = "";
	m_strInfixExpression = strInfixExpression;
	m_bParsed = FALSE;
}

// (r.gonet 08/03/2012) - PLID 30514 - Constructs a new expression with an expression string and also a field name
//  so that circular dependecies can be checked for.
CNxExpression::CNxExpression(CString strField, CString strInfixExpression)
{
	m_strField = strField;
	m_strInfixExpression = strInfixExpression;
	m_bParsed = FALSE;
}

// (j.luckoski 2012-08-02 09:54) - PLID 51900 - All default PARSE functions call the new PARSE function
BOOL CNxExpression::Parse(BOOL bReplaceOperands)
{
	return Parse(bReplaceOperands, FALSE, FALSE, FALSE, m_strInfixExpression);
}

// (j.luckoski 2012-08-02 09:57) - PLID 51900 - Handle multiple bools for maximum recursiveness
BOOL CNxExpression::Parse(BOOL bReplaceOperands, BOOL bIsParseFunction, BOOL bSkipParseBool, BOOL bUserEvaluate, CString strInExpression)
{
	// (r.gonet 08/03/2012) - PLID 30514 - Attempt to flatten the expression, ie make it so
	// that every referenced column in the expression is only one step away from the raw data.
	try {
		CString strResult;
		if(strInExpression == m_strInfixExpression) {
			// This is equal to at least the field we are evaluating,
			//  so assume that it is the field. It all works out the same anyway. 
			strResult = Flatten();
		} else {
			// (r.gonet 08/03/2012) - PLID 30514 - We were passed some part of the member expression
			//  We don't have a left hand side for this expression, so just make one up that is unique.
			strResult = Flatten(FormatString("$%s$", NewUUID(true)), strInExpression);
		}
		if(strResult != strInExpression) {
			strInExpression = strResult;
		}
	} catch(HasCycle hasCycle) {
		SetError(FormatString("A cycle was found in your formula. Expressions cannot be self referencing, neither directly nor indirectly through other formulas."));
		return FALSE;
	} catch(UnknownSymbol unknownSymbol) {
		SetError(FormatString("An unknown symbol %s was found in %s. Practice cannot parse the expression.", unknownSymbol.strUnknownSymbol, unknownSymbol.strContainerField));
		return FALSE;
	}

	m_arystrPostfixExpression.RemoveAll();
	m_aryInfixTokens.RemoveAll();

	// (j.luckoski 2012-08-02 09:59) - PLID 51900- Handle the scoped arrays so that recursions can copy values over to evaluate
	CStringArray arystrPostFixExpressions;
	CArray<InfixToken, InfixToken&> aryInfixTokens;

	if(strInExpression.IsEmpty()) {
		SetError("The expression is blank.");
		return FALSE;
	}


	CString strInfix = strInExpression;
	int nOriginalStringLength = strInfix.GetLength();
	strInfix.MakeUpper();
	strInfix.TrimLeft();
	strInfix.Trim(" ");

	// (j.luckoski 2012-08-02 10:00) - PLID 51900 - We only want to run this part if we are not actively seperating arguements
	// of a function and if another function calls this besides evaluate such as HasOperand.
	if(!bIsParseFunction || !bUserEvaluate) {

		// (z.manning, 05/29/2008) - The purpose of Parse() is to convert a basic math expression from infix
		// notation (which is the standard notation we're all familiar with) to postfix notation. There is a ton
		// of documentation all over the Internet about it and converting between the two.
		// I use arrays to simulate stacks where InsertAt(0) is "push" and GetAt(0) then RemoveAt(0) is "pop."
		// Here is the basic algorithm...
		//  1. Scan the Infix string from left to right.
		//	2. Initialise an empty stack.
		//	3. If the scannned character is an operand, add it to the Postfix string. If the scanned character is 
		//		an operator and if the stack is empty Push the character to stack.
		//	4. If the scanned character is an Operator and the stack is not empty, compare the precedence of the 
		//		character with the element on top of the stack (topStack). If topStack has higher precedence over 
		//		the scanned character Pop the stack else Push the scanned character to stack. Repeat this step as 
		//		long as stack is not empty and topStack has precedence over the character.
		//	5. Repeat this step till all the characters are scanned.
		//	6. After all characters are scanned, we have to add any character that the stack may have to the 
		//		Postfix string. If stack is not empty add topStack to Postfix string and Pop the stack. Repeat this 
		//		step as long as stack is not empty. 
		int nOperatorCount = 0, nOperandCount = 0;
		CArray<char,char> arychOperatorStack;
		BOOL bExpectOperator = FALSE;
		while(strInfix.GetLength() > 0)
		{
			InfixToken token;
			token.nStartPosition = nOriginalStringLength - strInfix.GetLength();
			char ch = strInfix.GetAt(0);
			BOOL bIsSingleCharToken = TRUE;
			if(IsValidOperandChar(ch) || (ch == '-' && !bExpectOperator))
			{
				CString strOperand = ch;
				strInfix.Delete(0);
				// (z.manning, 05/29/2008) - Operand may be more than one character so read until we reach
				// a non-operand character.
				while(strInfix.GetLength() > 0 && IsValidOperandChar(strInfix.GetAt(0))) {
					strOperand += strInfix.GetAt(0);
					strInfix.Delete(0);
				}

				// (j.luckoski 2012-08-02 10:01) - PLID 51900 - If the operand is the function handle it
				// by evaluating and passing the value back as the operand so MAX(1,2,3) becomes 3.
				if(strOperand == "MAX" && strInfix.GetLength() == 0 && bUserEvaluate) {
					SetError("Incomplete function at '" + strOperand + "'");
				} else if(strOperand == "MAX" && strInfix.GetAt(0) == '(' && bUserEvaluate) {
					CString strTempOperand;
					strTempOperand += strInfix.GetAt(0);
					strInfix.Delete(0);

					int nParenCount = 1;

					while(strInfix.GetLength() > 0 && nParenCount > 0) {
						strTempOperand += strInfix.GetAt(0);
						if(strInfix.GetAt(0) == '(') { nParenCount++; }
						if(strInfix.GetAt(0) == ')') { nParenCount--; }
						strInfix.Delete(0);
					}

					if(nParenCount == 0) {
						strOperand += strTempOperand;
						strTempOperand.Format("%g",Evaluate(TRUE,TRUE,strOperand));
						strOperand = strTempOperand;
					} else {
						SetError("Incomplete function at '" + strOperand + "'");
						strInfix += strTempOperand;
					}
				} else if(strOperand == "MAX" && strInfix.GetAt(0) != '(' && bUserEvaluate) {
					SetError("Incomplete function at '" + strOperand + "'");
				}

				if(strOperand.Find('.') != strOperand.ReverseFind('.')) {
					SetError("Unknown symbol '" + strOperand + "'");
				}

				if(bExpectOperator) {
					SetError("Expected operator before '" + strOperand + "'");
				}

				bExpectOperator = TRUE;

				if(bReplaceOperands) {
					// (z.manning, 05/28/2008) - See if we're supposed to replace this operand
					CString strReplacement;
					if(m_mapOperands.Lookup(strOperand, strReplacement)) {
						strOperand = strReplacement;
					}
				}

				// (j.luckoski 2012-08-02 10:09) - PLID 51900 - Changed array to scoped one
				arystrPostFixExpressions.Add(strOperand);


				nOperandCount++;
				bIsSingleCharToken = FALSE;
				token.strToken = strOperand;
			}
			else if(IsOperator(ch))
			{
				while(arychOperatorStack.GetSize() > 0 && arychOperatorStack.GetAt(0) != '(' && GetOperatorPrecedence(arychOperatorStack.GetAt(0)) >= GetOperatorPrecedence(ch)) {
					// (j.luckoski 2012-08-02 10:09) - PLID 51900 - Changed array to scoped one
					arystrPostFixExpressions.Add(arychOperatorStack.GetAt(0));
					arychOperatorStack.RemoveAt(0);
				}
				arychOperatorStack.InsertAt(0, ch);
				strInfix.Delete(0);
				nOperatorCount++;

				if(!bExpectOperator) {
					SetError("Expected operand before '" + CString(ch) + "'");
				}
				bExpectOperator = FALSE;
			}
			else if(ch == '(')
			{
				arychOperatorStack.InsertAt(0, ch);
				strInfix.Delete(0);
			}
			else if(ch == ')')
			{
				while(arychOperatorStack.GetSize() > 0 && arychOperatorStack.GetAt(0) != '(') {
					// (j.luckoski 2012-08-02 10:09) - PLID 51900 - Changed array to scoped one
					arystrPostFixExpressions.Add(arychOperatorStack.GetAt(0));
					arychOperatorStack.RemoveAt(0);
				}
				if(arychOperatorStack.GetSize() > 0) {
					if(arychOperatorStack.GetAt(0) == '(') {
						arychOperatorStack.RemoveAt(0);
					}
					else {
						ASSERT(FALSE);
					}
				}
				else {
					SetError("Too many closing parentheses");
				}
				strInfix.Delete(0);
			}
			else
			{
				// Invalid character
				SetError("Invalid character '" + CString(ch) + "'");
				// (j.luckoski 2012-08-02 10:09) - PLID 51900 - Changed array to scoped one
				arystrPostFixExpressions.Add(ch);
				strInfix.Delete(0);
			}

			if(bIsSingleCharToken) {
				token.strToken = ch;
			}

			// (j.luckoski 2012-08-02 10:09) - PLID 51900 - Changed array to scoped one
			aryInfixTokens.Add(token);


			strInfix.TrimLeft();
		}

		while(arychOperatorStack.GetSize() > 0) {
			arystrPostFixExpressions.Add(arychOperatorStack.GetAt(0));
			arychOperatorStack.RemoveAt(0);
		}

		// (z.manning, 05/29/2008) - We should have one more operand than operator
		if(nOperatorCount < nOperandCount - 1) {
			SetError("There are not enough operators in your expression.");
		}
		else if(nOperatorCount > nOperandCount - 1) {
			// (z.manning, 05/29/2008) - This will be detected in Evaluate, which will be able
			// to provider a more descriptive error message.
		}

		// (z.manning, 05/29/2008) - This was very useful when debugging, so I left it in in case
		// anyone changes something here and wants to be able to easily view the postfix expression
		// when debugging.
#ifdef _DEBUG
		CString strPostfix;
		// (j.luckoski 2012-08-02 10:09) - PLID 51900 - Changed array to scoped one
		for(int i = 0; i < arystrPostFixExpressions.GetSize(); i++) {
			strPostfix += arystrPostFixExpressions.GetAt(i) + ' ';
		}
#endif

		BOOL bValid = GetError().IsEmpty();
		if(bValid && !bSkipParseBool) {
			m_bParsed = TRUE;
		}

		m_arystrPostfixExpression.Copy(arystrPostFixExpressions);
		m_aryInfixTokens.Copy(aryInfixTokens);
		

		return bValid;

	} else {   // (j.luckoski 2012-08-02 10:20) - PLID 51900 - This section is strictly for seperating the comma delimited expressions

		int nStartLocation = strInfix.Find('(');
		int nParenCount = 1;
		CStringArray arystrFunctionExpression;
		CString strExpressions, strTempInfix;

		strTempInfix = strInfix.Mid(nStartLocation + 1);

		while(strTempInfix.GetLength() > 0) {
			if((strTempInfix.GetAt(0) == ',' || strTempInfix.GetAt(0) == ')') && nParenCount == 1) {
				if(strExpressions.GetLength() == 0) {
					SetError("Function didn't contain a valid arguement");
					break;
				} else {
					CString strDoubEval;
					if(strTempInfix.GetAt(0) == ')') { nParenCount--; }
					strTempInfix.Delete(0);
					strDoubEval.Format("%g",Evaluate(FALSE,TRUE,strExpressions));
					arystrFunctionExpression.Add(strDoubEval);
					strExpressions = "";
				}
			} else {
				strExpressions += strTempInfix.GetAt(0);
				if(strTempInfix.GetAt(0) == '(') { nParenCount++; }
				if(strTempInfix.GetAt(0) == ')') { nParenCount--; }
				strTempInfix.Delete(0);
				strTempInfix.TrimLeft();
			}

			if(strInfix.Left(nStartLocation) == "MAX" && nParenCount == 0) {
				double nMax = -DBL_MAX;
				for(int i = 0; i < arystrFunctionExpression.GetSize(); i++)
				{
					double val = atof(arystrFunctionExpression[i]);
					if (val > nMax) {
						nMax = val;
					}
				}

				arystrPostFixExpressions.RemoveAll();
				CString strMax;
				strMax.Format("%g",nMax);
				arystrPostFixExpressions.Add(strMax);

			}

		}

		BOOL bValid = GetError().IsEmpty();

		m_arystrPostfixExpression.Copy(arystrPostFixExpressions);
		m_aryInfixTokens.Copy(aryInfixTokens);

		return bValid;
	}
}

// (j.luckoski 2012-08-02 10:34) - PLID 51900 - Call the default Evaluate at the top level if this is called
double CNxExpression::Evaluate()
{
	return Evaluate(FALSE, FALSE, m_strInfixExpression);
}

// (j.luckoski 2012-08-02 10:35) - PLID 51900 - Handle recursive descent.
double CNxExpression::Evaluate(BOOL bIsParseFunction, BOOL bSkipParseBool, CString strInExpression)
{
	if(!m_bParsed) {
		Parse(FALSE, bIsParseFunction, bSkipParseBool, TRUE,strInExpression);
	}

	CStringArray arystrPostFixExpression;
	CVariantArray aryvarOperandStack;

	// (j.luckoski 2012-08-02 10:35) - PLID 51900 - Depending on the bool, decide whether to draw from the main top level member array or the sub level array
	
	arystrPostFixExpression.Copy(m_arystrPostfixExpression);
	

	// (z.manning, 05/29/2008) -  I use arrays to simulate stacks where InsertAt(0) is "push"
	// and GetAt(0) then RemoveAt(0) is "pop."
	// The algorithm for evaulating a postfix expression is as follows...
	//  1. Scan the Postfix string from left to right.
	//	2. Initialise an empty stack.
    //	3. If the scannned character is an operand, add it to the stack. If the scanned character is 
	//		an operator, there will be at least two operands in the stack.
	//	4. If the scanned character is an Operator, then we store the top most element of the stack(topStack) 
	//		in a variable temp. Pop the stack. Now evaluate topStack(Operator)temp. Let the result of this 
	//		operation be retVal. Pop the stack and Push retVal into the stack.
    //	5. Repeat this step till all the characters are scanned.
    //	6. After all characters are scanned, we will have only one element in the
    //      stack. Return topStack as result
	
	for(int nTokenIndex = 0; nTokenIndex < arystrPostFixExpression.GetSize(); nTokenIndex++)
	{
		CString strToken = arystrPostFixExpression.GetAt(nTokenIndex);
		if(strToken.GetLength() <= 0) {
			// (z.manning, 05/23/2008) - There's a bug in Parse() if we get here
			ASSERT(FALSE);
			return 0.0;
		}

		if(IsOperator(strToken.GetAt(0)) && !(strToken.GetAt(0) == '-' && strToken.GetLength() > 1))
		{
			ASSERT(strToken.GetLength() == 1);
			if(aryvarOperandStack.GetSize() < 2) {
				// (z.manning, 05/28/2008) - If we have an operator but do not have at least 2 operands
				// remaining then there is incorrect syntax in the expression.
				CString strError = "Incorrect syntax near '" + CString(strToken.GetAt(0)) + "'";
				SetError(strError);
				return 0.0;
			}

			// (z.manning, 05/28/2008) - Make sure our operands are numbers
			_variant_t varLeftOperand = aryvarOperandStack.GetAt(1);
			if(!IsValidDouble(AsString(varLeftOperand))) {
				SetError("Unknown symbol '" + AsString(varLeftOperand) + "'");
				return 0.0;
			}
			_variant_t varRightOperand = aryvarOperandStack.GetAt(0);
			if(!IsValidDouble(AsString(varRightOperand))) {
				SetError("Unknown symbol '" + AsString(varRightOperand) + "'");
				return 0.0;
			}

			// (z.manning, 05/28/2008) - Make sure our operator is something we handle
			if(!IsOperator(strToken.GetAt(0))) {
				// (r.gonet 08/03/2012) - PLID 30514 - Fixed a misspelling
				SetError("Invalid character '" + CString(strToken.GetAt(0)) + "'");
				return 0.0;
			}

			double dblResult = PerformOperation(varLeftOperand, strToken.GetAt(0), varRightOperand);
			aryvarOperandStack.RemoveAt(0);
			aryvarOperandStack.RemoveAt(0);
			aryvarOperandStack.InsertAt(0, _variant_t(dblResult));
		}
		else
		{
			// (z.manning, 05/28/2008) - See if we're supposed to replace this operand
			CString strReplacement;
			if(m_mapOperands.Lookup(strToken, strReplacement)) {
				strToken = strReplacement;
			}

			// (z.manning, 05/23/2008) - We assume this to be an operand
			aryvarOperandStack.InsertAt(0, _variant_t(strToken));
		}
	}

	// (z.manning, 05/28/2008) - If everything was valid then we should only have 1 element left in the
	// stack, which is our result.
	if(aryvarOperandStack.GetSize() == 0) {
		// (z.manning, 05/29/2008) - The formula is likely empty
		SetError("Invalid formula");
		return 0.0;
	}
	else if(aryvarOperandStack.GetSize() > 1) {
		SetError("Incorrect syntax near '" + AsString(aryvarOperandStack.GetAt(0)) + "'");
		return 0.0;
	}

	_variant_t varResult = aryvarOperandStack.GetAt(0);

	// (z.manning, 05/29/2008) - Make sure the result is a number
	if(!IsValidDouble(AsString(varResult))) {
		SetError("Unknown symbol '" + AsString(varResult) + "'");
		return 0.0;
	}

	return AsDouble(varResult);
}

void CNxExpression::SetError(const CString strError)
{
	if(m_strFirstParseError.IsEmpty()) {
		m_strFirstParseError = strError;
	}
}

CString CNxExpression::GetError()
{
	return m_strFirstParseError;
}

void CNxExpression::SetExpression(const CString strInfixExpression)
{
	m_bParsed = FALSE;
	m_strFirstParseError.Empty();
	m_strInfixExpression = strInfixExpression;
}

// (r.gonet 08/03/2012) - PLID 30514 - Add a formula to the expression's context of knowledge.
//  This will enable it to flatten expressions that reference other fields that are formula filled.
void CNxExpression::AddFormula(CString strField, CString strExpression)
{
	CString strOldExpression;
	if(m_mapFormulas.Lookup(strField, strOldExpression)) {
		if(strOldExpression == strExpression) {
			// No need to update or invalidate the cache
			return;
		}
	}

	m_mapFormulas.SetAt(strField, strExpression);
	m_mapCache.RemoveAll();
	// Parsed needs unset because if this formula needs to be flattened, it could result in a different expression.
	m_bParsed = FALSE;
}

BOOL CNxExpression::IsOperator(char ch)
{
	switch(ch)
	{
		case '+':
		case '-':
		case '*':
		case '/':
		case '^':
		case '%': // (z.manning 2011-05-26 10:01) - PLID 43851 - Support modulo
			return TRUE;
			break;
	}

	return FALSE;
}

BOOL CNxExpression::IsValidOperandChar(char ch)
{
	if(ch >= 'A' && ch <= 'Z') {
		return TRUE;
	}
	else if(ch >= 'A' && ch <= 'Z') {
		return TRUE;
	}
	else if(ch >= '0' && ch <= '9') {
		return TRUE;
	}
	else if(ch == '.') {
		return TRUE;
	}

	return FALSE;
}

EOperatorPrecedence CNxExpression::GetOperatorPrecedence(char chOperator)
{
	switch(chOperator)
	{
		case '+':
		case '-':
			return opAddSubtract;
			break;

		case '*':
		case '/':
		case '%': // (z.manning 2011-05-26 09:58) - PLID 43851 - Support modulo
			return opMultiplyDivide;
			break;

		case '^':
			return opExponent;
			break;

		case '(':
		case ')':
			return opParentheses;
			break;

		default:
			ASSERT(FALSE);
			return opInvalid;
			break;
	}
}

double CNxExpression::PerformOperation(_variant_t varLeftHandSide, char chOperator, _variant_t varRightHandSide)
{
	double dblLeft = AsDouble(varLeftHandSide);
	double dblRight = AsDouble(varRightHandSide);

	// (z.manning 2010-11-08 15:44) - PLID 41377 - This was not actually checking for what type of operation
	// is being performed!
	// (z.manning 2011-06-01 14:20) - PLID 43851 - Do the same thing for mod by zero.
	if(dblRight == 0 && (chOperator == '/' || chOperator == '%')) {
		// (z.manning 2010-11-08 15:45) - PLID 41377 - We used to set an error here, however, it doesn't really
		// make sense to do so because the errors are only meant to be for determing syntax errors whereas
		// division by zero is a execution-time error.
		//SetError("Division by zero");
		// (z.manning 2010-11-08 15:47) - PLID 41377 - Additionaly, we previously had just continued on here
		// and let the division by zero operation be performed anyway. Let's just return zero rather than
		// perform an invalid operation.
		return 0.0;
	}

	switch(chOperator)
	{
		case '+':
			return dblLeft + dblRight;
			break;

		case '-':
			return dblLeft - dblRight;
			break;

		case '*':
			return dblLeft * dblRight;
			break;

		case '/':
			return dblLeft / dblRight;
			break;

		case '^':
			return pow(dblLeft, dblRight);
			break;

		// (z.manning 2011-05-26 09:59) - PLID 43851 - Added support for modulo
		case '%':
			return fmod(dblLeft, dblRight);
			break;

		default:
			ASSERT(FALSE);
			return 0.0;
			break;
	}
}

void CNxExpression::AddOperandReplacement(CString strOld, CString strNew)
{
	strOld.MakeUpper();
	strNew.MakeUpper();
	m_mapOperands.SetAt(strOld, strNew);
}

// (z.manning, 05/30/2008) - PLID 16443 - Returns the infix representation of the expression,
// including any operand replacements that may have been made.
CString CNxExpression::GetInfixExpression()
{
	if(!m_bParsed) {
		Parse(TRUE);
	}

	CString strInfix;
	for(int nIndex = 0; nIndex < m_aryInfixTokens.GetSize(); nIndex++)
	{
		InfixToken token = m_aryInfixTokens.GetAt(nIndex);
		while(strInfix.GetLength() < token.nStartPosition) {
			strInfix += ' ';
		}
		strInfix += token.strToken;
	}

	return strInfix;
}

// (z.manning, 05/30/2008) - PLID 16443 - Returns true if the given operand is in the expression,
// false otherwise.
BOOL CNxExpression::HasOperand(CString strOperand)
{
	if(!m_bParsed) {
		Parse(FALSE);
	}

	strOperand.MakeUpper();
	for(int nIndex = 0; nIndex < m_arystrPostfixExpression.GetSize(); nIndex++)
	{
		if(m_arystrPostfixExpression.GetAt(nIndex) == strOperand) {
			return TRUE;
		}
	}

	return FALSE;
}

void CNxExpression::GetTokens(OUT CStringArray &arystrTokens)
{
	if(!m_bParsed) {
		Parse(FALSE);
	}

	arystrTokens.RemoveAll();
	arystrTokens.Append(m_arystrPostfixExpression);
}

// (r.gonet 08/03/2012) - PLID 30514 - Flattens the m_strInfixExpression
CString CNxExpression::Flatten()
{
	CString strField = m_strField;
	if(m_strField.IsEmpty()) {
		// (r.gonet 08/03/2012) - PLID 30514 - There is no field associated with this expression! But we need one!
		strField = FormatString("$%s$", NewUUID(true));
	}
	return Flatten(strField, m_strInfixExpression);
}

// (r.gonet 08/03/2012) - PLID 30514 - Flattens an expression given the expression and a field that the expression is assigned to.
CString CNxExpression::Flatten(CString strField, CString strExpression)
{
	bool bAddedFormula = false;
	// Temporarily add ourself into the formulas map (if we aren't in there already) so we can detect cycles
	if(!m_mapFormulas.PLookup(strField)) {
		bAddedFormula = true;
		m_mapFormulas.SetAt(strField, strExpression);
	}
	CArray<CString, CString> aryVisited;
	CString strResult = Flatten(strField, aryVisited);

	if(bAddedFormula) {
		// Remove the temporary identifier from the formulas map
		m_mapFormulas.RemoveKey(strField);
	}

	return strResult;
}

// (r.gonet 08/03/2012) - PLID 30514 - Recursive Flatten. Should not be called directly.
//  Maintains a visited list to detect circular references.
CString CNxExpression::Flatten(CString strField, CArray<CString, CString> &aryVisited)
{
	// (r.gonet 08/03/2012) - PLID 30514 - Push the field on to the stack.
	aryVisited.Add(strField);
	CString strExpr;
	if(!m_mapFormulas.Lookup(strField, strExpr)) {
		// (r.gonet 08/03/2012) - PLID 30514 - Couldn't find the symbol we were instructed to evaluate?
		UnknownSymbol us("", strField);
		if(aryVisited.GetSize() > 1) {
			us.strContainerField = aryVisited[aryVisited.GetSize() - 2];
		}
		throw us;
	}

	// (r.gonet 08/03/2012) - PLID 30514 - Iterate through all the formula filled fields
	//  and see if there are any in expression we are flattening. If there are, then flatten
	//  the expressions that those fields reference and do substitution.
	POSITION pos = m_mapFormulas.GetStartPosition();
	while(pos) {
		CString strField2;
		CString strExpr2;
		m_mapFormulas.GetNextAssoc(pos, strField2, strExpr2);

		// (r.gonet 08/03/2012) - PLID 30514 - While replacing, we only need to check for the cycle once.
		bool bCheckForCycle = true;
		int nIndex = 0;
		while((nIndex = strExpr.Find(strField2, nIndex)) != -1) {
			// (r.gonet 08/03/2012) - PLID 30514 - This could have matched a different field, like C1 could match C19, so expand our find to the left and to the right
			int nStartIndex = nIndex;
			while(nStartIndex > 0) {
				nStartIndex--;
				if(!isalpha(strExpr[nStartIndex]) && !isdigit(strExpr[nStartIndex])) {
					nStartIndex++;
					break;
				}
			}
			int nEndIndex = nIndex + strField2.GetLength() - 1;
			while(nEndIndex < strExpr.GetLength()) {
				nEndIndex++;
				if(!isalpha(strExpr[nEndIndex]) && !isdigit(strExpr[nEndIndex])) {
					nEndIndex--;
					break;
				}
			}
			CString strMatchedField = strExpr.Mid(nStartIndex, nEndIndex - nStartIndex + 1);
			if(strMatchedField != strField2) {
				// Was a partial match, so continue looping
				nIndex++; // So we don't repeat the same mistake
				continue;
			}

			if(bCheckForCycle) {
				// Check for cycles
				for(int i = 0; i < aryVisited.GetSize(); i++) {
					if(aryVisited[i] == strField2) {
						// There is a cycle, we can't keep flattening or we would go on forever
						HasCycle hasCycle(strField, strField2);
						throw hasCycle;
					}
				}

				bCheckForCycle = false;
			}

			CString strFlat;
			if(!m_mapCache.Lookup(strField2, strFlat)) {
				// (r.gonet 08/03/2012) - PLID 30514 - We haven't flattened this field before. Do so.
				strFlat = Flatten(strField2, aryVisited);
				// Now remember what we got so we don't need to flatten it again.
				m_mapCache.SetAt(strField2, strFlat);
			} else {
				// (r.gonet 08/03/2012) - PLID 30514 - We've already flattened this field's expression,
				//  look up the result in the cache.
			}
			// (r.gonet 08/03/2012) - PLID 30514 - Now do the substitution. Remember the parentheses.
			strExpr = FormatString("%s(%s)%s", 
				strExpr.Left(nStartIndex),
				strFlat,
				strExpr.Right(strExpr.GetLength() - (nEndIndex + 1)));
			// (r.gonet 08/03/2012) - PLID 30514 - I am aware that this is modifying the string we are looping over and that is usually not
			//  a good thing to do, but in this case it is fine since the part before the found index doesn't change, only after.
		}
	}
	// (r.gonet 08/03/2012) - PLID 30514 - Pop the current field from the stack.
	aryVisited.RemoveAt(aryVisited.GetSize() - 1);

	return strExpr;
}