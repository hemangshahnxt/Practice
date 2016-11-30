#include "StdAfx.h"
#include "NxAutoQuantum.h"
#include <NxDataUtilitiesLib/NxSubString.h>
#include <NxDataUtilitiesLib/NxSqlParser.h>

#include <boost/container/flat_set.hpp>
#include <boost/container/flat_map.hpp>

#include <NxDataUtilitiesLib/vaUtils.h>

#include <NxDataUtilitiesLib/GlobalStringUtils.h>

#pragma push_macro("max")
#pragma push_macro("min")

#undef max
#undef min

namespace Nx
{

namespace Quantum
{

// (a.walling 2014-01-30 00:00) - PLID 60548 - Find a format token
struct TokenParser
{
	TokenParser(SubString src)
		: src(src)
		, pos(src.begin())
	{}

	SubString src;
	const char* pos;

	bool isEof() const
	{ return pos >= src.end(); }

	const char* Next()
	{
		while (pos < src.end()) {
			switch (*pos) {
				case '%':
					if (src.deref(pos + 1) == '%') {
						++pos;
					} else {
						return pos;
					}
					break;
				case '\0':
					return pos;
			};

			++pos;
		}

		return pos;
	}
};

// (a.walling 2014-01-30 00:00) - PLID 60548
struct FormatToken
{
	SubString token;
	SubString formatToken;

	unsigned char dataLength;
	char flag;
	char type;

	FormatToken()
		: dataLength(0)
		, flag('\0') // technically its the 'size' of the format specifier, but that is confusing considering length and dataLength
		, type('\0')
	{}


	bool IsValid() const
	{ return dataLength != 0; }
	
	// safe bool
	typedef void (FormatToken::*unspecified_bool_type)() const;
	operator unspecified_bool_type() const
	{
		return IsValid() ? &FormatToken::bool_helper : NULL;
	}

	bool operator!() const
	{ return !IsValid(); }

protected:
	void bool_helper() const {}
};

namespace detail {	
	inline bool IsFlagChar(char c)
	{
		switch (c)
		{
			case ' ':
			case '#':
			case '+':
			case '-':
				return true;
			default:
				return false;
		}
	}

	inline bool IsVariableChar(char c)
	{
		if (::isalnum((unsigned char)c)) {
			return true;
		}
		switch (c)
		{
			case '_':
			case '-':
			case '$':
			case '#':
				return true;
			default:
				return false;
		}
	}

	inline bool HasBalancedQuotes(SubString sz)
	{
		return 0 == (std::count(sz.begin(), sz.end(), '\'') % 2);
	}
}


SubString ParseVariable(SubString src, const char* pos)
{
	const char* begin = pos;

	_ASSERTE(src.deref(pos) == '@');
	++pos;

	while (src.deref(pos) && detail::IsVariableChar(*pos)) {
		++pos;
	}

	return SubString(begin, pos);
}

bool IsValidUnquotedValue(iSubString str)
{
	str = str.trim();

	if (str.empty()) {
		return false;
	}

	if (str == "null") {
		return true;
	}

	if (str[0] == '\'' || str.left(2) == "N'") {
		const char* pos = str.begin();
		if (str.deref(pos) == 'N') {
			++pos;
		}
		if (str.deref(pos) == '\'') {
			++pos;
		}
		while (pos < str.end()) {
			if (*pos == '\'') {
				if (str.deref(pos + 1) == '\'') {
					++pos;
				} else {
					break;
				}
			}

			++pos;
		}

		return pos == (str.end() - 1);
	} else if (::isdigit((unsigned char)str[0]) || str[0] == '-' || str[0] == '+' || str[0] == '.') {
		const char* pos = str.begin();
		++pos;
		if (str.deref(pos) == 'x' || str.deref(pos) == 'X') {
			++pos;
			while (pos < str.end() && ::isxdigit((unsigned char)*pos)) {
				++pos;
			}
		} else {
			while (pos < str.end() && (::isdigit((unsigned char)*pos) || *pos == 'e' || *pos == 'E' || *pos == '.' || *pos == '-' || *pos == '+')) {
				++pos;
			}
		}

		return pos == str.end();
	} else if (str[0] == '@') {
		SubString var = ParseVariable(str, str.begin());
		return var.end() == str.end();
	} else {
		return false;
	}
}

// (a.walling 2014-01-30 00:00) - PLID 60548 - Parse a format token
FormatToken ParseFormatToken(SubString src, const char* pos)
{
	FormatToken tok;
	tok.token = SubString(pos, pos);

	_ASSERTE(*pos == '%');
	++pos;

	// skip flags
	while (src.deref(pos) && detail::IsFlagChar(*pos)) {
		++pos;
	}

	// width (ignoring *)
	while (src.deref(pos) && ::isdigit((unsigned char)*pos)) {
		++pos;
	}

	// precision (ignoring *)
	if (src.deref(pos) == '.') {
		++pos;
		while (src.deref(pos) && ::isdigit((unsigned char)*pos)) {
			++pos;
		}
	}


	// size {h | l | ll | w | I | I32 | I64}
	switch (src.deref(pos)) {
		case 'l':
			if (src.deref(pos + 1) == 'l') {
				tok.flag = 'w';
				pos += 2;
			} else {
				++pos;
			}
			break;
		case 'I':
			if (src.deref(pos + 1) == '6' && src.deref(pos + 2) == '4') {
				tok.flag = 'w';
				pos += 3;
			} else if (src.deref(pos + 1) == '3' && src.deref(pos + 2) == '2') {
				pos += 3;
			} else {
				++pos;
			}
			break;
		case 'h':
			break;
		case 'w':
			tok.flag = 'w';
			++pos;
			break;
	}

	// type int{cCdiouxXp}, double{eEfgGaA}, bad{n}, string{sSZ}
	switch (src.deref(pos))
	{
		case 'c':
		case 'C':
		case 'd':
		case 'i':
		case 'o':
		case 'u':
		case 'x':
		case 'X':
		case 'p':
			// int
			tok.type = 'i';
			// always promoted to int (or __int64)
			if (tok.flag != 'w') {
				tok.dataLength = 4;
			} else {
				tok.dataLength = 8;
			}
			break;
		case 'a':
		case 'A':
		case 'e':
		case 'E':
		case 'g':
		case 'G':
			tok.flag = 'w';
		case 'f':
			// float / double
			tok.type = 'f';
			tok.dataLength = 8; // always promoted to double
			break;
		case 'S':
			tok.flag = 'w';
		case 's':
			// string
			tok.type = 's';
			tok.dataLength = sizeof(void*);
			break;
		case 'n':
		case 'Z': // we are not using any NT strings (ANSI_STRING, UNICODE_STRING structs)
			ASSERT(false);
			tok.flag = '\0';
			tok.type = *pos;
			break;
		default:
			ASSERT(false);
			tok.flag = '\0';
			tok.type = *pos;
			pos = tok.token.begin(); // just the '%';
			break;
	}

	++pos;

	tok.token.end_ = pos;
	tok.formatToken = tok.token;

	if ( (src.deref(tok.token.begin() - 1) == '\'') && (src.deref(tok.token.end()) == '\'') ) {
		--tok.token.begin_;
		++tok.token.end_;

		if (src.deref(tok.token.begin() - 1) == 'N') {
			--tok.token.begin_;
		}
	}

	return tok;
}

// (a.walling 2014-01-30 00:00) - PLID 60548 - Guess type from value
const char* GetDataTypeFromValue(SubString ssvalue)
{
	if (ssvalue[0] == '\'' || (ssvalue[0] == 'N' && ssvalue[1] == '\'') ) {
		int extraQuotes = std::count(ssvalue.begin(), ssvalue.end(), '\'') / 2;
		extraQuotes += 1; // for the begin/end pair of quotes
		int dataLen = ssvalue.length() - extraQuotes;
		if (ssvalue[0] == 'N') {
			dataLen -= 1;
		}
		if (dataLen <= 4000) {
			return "NVARCHAR(4000)";
		} else {
			return "NVARCHAR(MAX)";
		}
	} else if (ssvalue[0] == '0' && ssvalue[1] == 'x') {
		int dataLen = (ssvalue.length() - 2) / 2;
		if (dataLen <= 8000) {
			return "VARBINARY(8000)";
		} else {
			return "VARBINARY(MAX)";
		}
	} else if (Nx::Text::IsDigit(ssvalue[0])) {
		const char* endPtr = NULL;
		__int64 val = _strtoi64(ssvalue.begin(), (char**)&endPtr, 10);
		if (endPtr == ssvalue.end()) {
			if (val <= INT_MAX && val >= INT_MIN) {
				return "INT";
			} else {
				return "BIGINT";
			}
		} else if (ssvalue.deref(endPtr) == '.' || ssvalue.deref(endPtr) == 'e' || ssvalue.deref(endPtr) == 'E') {
			// float value, or money. Either way will convert to and from nvarchar implicitly, huzzah
			// this is already the 'default' if this function returns NULL but does not hurt to explicitly handle the case.
			return "NVARCHAR(64)";
		}
	}

	return NULL;
}

// (a.walling 2014-01-30 00:00) - PLID 60548 - Guess type from value
const char* GetFormatTokenDataType(char type, char flag, const CString& value)
{
	switch (type) {
		case 'i':
			if (flag == 'w') {
				return "BIGINT";
			} else {
				return "INT";
			}
			break;
		case 'f':
			return "FLOAT";
			break;
		case 's':
		default:
			{
				if (const char* valueTypeHint = GetDataTypeFromValue(value)) {
					return valueTypeHint;
				} else {
					return NULL; // can't deduce
				}
			}
			break;
	};
}

// (a.walling 2014-01-30 00:00) - PLID 60549
const char* GetNxParamSqlDataType(const CNxParamSql& param)
{
	switch (param.m_eadodtDataType) {
		case ADODB::adInteger:
			return "INT";
		case ADODB::adVarWChar:
			if (param.m_nParamSize > 0) {
				return "NVARCHAR(4000)";
			} else {
				return "NVARCHAR(MAX)";
			}
		case ADODB::adBinary:
		case ADODB::adVarBinary:
			if (param.m_nParamSize > 0) {
				return "VARBINARY(8000)";
			} else {
				return "VARBINARY(MAX)";
			}			
		case ADODB::adSingle:
		case ADODB::adDouble:
			return "FLOAT";
		case ADODB::adDate:
			return "DATETIME";
		case ADODB::adBoolean:
			return "BIT";
		case ADODB::adCurrency:
			return "MONEY";
		case ADODB::adBigInt:
			return "BIGINT";
		case ADODB::adDecimal:
			return "DECIMAL";
		case ADODB::adEmpty:
		default:
			ASSERT(FALSE);
			return "NVARCHAR(64)";
	}
}

// (a.walling 2014-01-30 00:00) - PLID 60549 - NxAutoQuantum - Parse CSqlFragment / ParamSql and quantize
Statement Statement::ParseParamQuery(const CString& strFinalSql, const CNxParamSqlArray& aryParams)
{
	Statement s;

	s.text.Preallocate(strFinalSql.GetLength() + (aryParams.GetSize() * 5));
	s.raw.Preallocate(strFinalSql.GetLength() + (aryParams.GetSize() * 10));
	s.args.reserve(aryParams.GetSize());
	
	SubString src = strFinalSql;

	using namespace Nx::Sql::Parser;

	const char* pos = src.begin();
	Token tok;

	// (a.walling 2014-01-30 00:00) - PLID 60538 - Use Nx::Sql::Parser
	while (tok = NextToken(src, pos)) {
		pos = tok.Text().end();

		if (tok.Type() == Token::Escape && tok.Text() == "{?}") {
			if ((int)s.args.size() >= aryParams.GetSize()) {
				ThrowNxException(__FUNCTION__": More parameter tokens than parameters!");
			}

			const CNxParamSql& param = aryParams.GetAt((int)s.args.size());

			Parameter arg(
				FormatString("@p%02li", s.args.size() + 1)
				, AsQuotedStringForSql(param.m_varParam)
				, GetNxParamSqlDataType(param)
			);

			s.args.push_back(arg);

			s.text += arg.name;
			s.raw += arg.value;
		} else {
			tok.Text().append_to(s.text);
			tok.Text().append_to(s.raw);
		}
	}

	_ASSERTE(s.args.size() == aryParams.GetSize());

	return s;
}

// (a.walling 2014-01-30 00:00) - PLID 60548 - NxAutoQuantum - Parse format strings and quantize
Statement Statement::ParseFormatV(LPCTSTR strStatementFmt, va_list args)
{
	va_list curArg;

	curArg = args;

	SubString src(strStatementFmt, strlen(strStatementFmt));

	Statement s;

	s.raw.FormatV(strStatementFmt, args);
	s.text.Preallocate(std::max(s.raw.GetLength(), (int)src.length()));

#ifdef _DEBUG
	CString strActualResult;
	strActualResult.Preallocate(s.raw.GetLength());
#endif

	TokenParser parseState(src);

	CString strTokenText;

	const char* lastPos = src.begin();
	while (!parseState.isEof()) {
		const char* pos = parseState.Next();
		if (!src.deref(pos)) {
			break;
		}

		s.text.Append(lastPos, pos - lastPos);
#ifdef _DEBUG
		strActualResult.Append(lastPos, pos - lastPos);
#endif
		lastPos = pos;

		if (*pos  == '%') {
			FormatToken tok = ParseFormatToken(src, pos);
			_ASSERTE(tok.token.length() > 0);

			_ASSERTE(tok.IsValid());

			if (tok.IsValid()) {
				bool isQuoted = (tok.token != tok.formatToken);
				if (isQuoted) {
					// trim any prefix chars here
					s.text.GetBuffer();
					s.text.ReleaseBufferSetLength(s.text.GetLength() - (tok.formatToken.begin() - tok.token.begin()));
	#ifdef _DEBUG
					strActualResult.GetBuffer();
					strActualResult.ReleaseBufferSetLength(strActualResult.GetLength() - (tok.formatToken.begin() - tok.token.begin()));
	#endif
				}

				tok.token.to(strTokenText);

				Parameter arg(
					FormatString("@p%02li", s.args.size() + 1)
					, FormatStringV(strTokenText, curArg)
				);
				arg.typeHint = GetFormatTokenDataType(tok.type, tok.flag, arg.value);

				_ASSERTE(detail::HasBalancedQuotes(arg.value));

				if (tok.type == 's' && !isQuoted && !IsValidUnquotedValue(arg.value)) {
					TRACE(__FUNCTION__" invalid unquoted value `%s`\n", arg.value);
					//_ASSERTE(IsValidUnquotedValue(arg.value));
					// this has served its purpose for now
					s.text.Append(arg.value);
				} else {

					s.args.push_back(arg);

					s.text.Append(arg.name);
				}

#ifdef _DEBUG
				strActualResult.Append(arg.value);
#endif

				curArg += tok.dataLength;		
			}

			pos = tok.token.end();
		} else {
			// nothing!
			ASSERT(FALSE);
			if (pos < src.end()) {
				++pos;
			}
		}

		_ASSERTE(parseState.pos != pos);
		parseState.pos = pos;
		lastPos = pos;
	}
	
	s.text.Append(lastPos, src.end() - lastPos);

#ifdef _DEBUG
	strActualResult.Append(lastPos, src.end() - lastPos);
#endif _DEBUG

	_ASSERTE(strActualResult == s.raw);

	return s;
}

// (a.walling 2014-01-30 00:00) - PLID 60550 - NxAutoQuantum - Scan quantized statements for external @variables and scope_identity() and convert to output variables and @@identity
static void ProcessStatementText(const Statement& statement, boost::container::flat_set<CiString>& references, CString& final)
{
	final.Preallocate(statement.text.GetLength());
	SubString src = statement.text;

	using namespace Nx::Sql::Parser;

	const char* pos = src.begin();
	Token tok;

	// (a.walling 2014-01-30 00:00) - PLID 60538 - Nx::Sql::Parser
	while (tok = NextToken(src, pos)) {
		pos = tok.Text().end();

		if (tok.Type() == Token::Word && tok.Text() == "SCOPE_IDENTITY") {
			_ASSERTE(src.deref(pos) == '(' && src.deref(pos + 1) == ')');
			if (src.deref(pos) == '(' && src.deref(pos + 1) == ')') {
				pos += 2;
				SubString("@@IDENTITY").append_to(final);
				continue;
			}
		}
			
		tok.Text().append_to(final);

		if (tok.Type() == Token::Variable) {

			if (tok.Text().at(1) == '@') {
				// ignoring @@variable names
				continue;
			}

			if ( (tok.Text().at(1) == 'p' || tok.Text().at(1) == 'P') && Nx::Text::IsDigit(tok.Text().at(2))) {
				// ignoring @p[0-9] variable names
				continue;
			}

			CiString varName = tok.Text();

			references.insert(varName);
		}
	}
}

const char* Parameter::GetDataType() const
{
	if (typeHint) {
		return typeHint;
	} else {
		if (const char* valueTypeHint = GetDataTypeFromValue(value)) {
			return valueTypeHint;
		} else {
			return NULL; // can't deduce
		}
	}
}

// (a.walling 2014-07-11 14:29) - PLID 62205 - If we don't have a data type, and the value is a @variable, we can deduce that type!
CString CalcDataType(const Parameter& arg, const Batch& batch)
{
	const char* typeHint = arg.GetDataType();
	if (typeHint && *typeHint) {
		return typeHint;
	}

	if (!arg.value.IsEmpty() && arg.value[0] == '@') {
		Batch::VariableMap::const_iterator it = batch.variables_.find(arg.value);

		if (it != batch.variables_.end()) {
			return it->second;
		}
	}
	return "NVARCHAR(64)";
}

struct FinalStatement
{
	CString text;
	CString params;
	CString values;
};

// (a.walling 2014-01-30 00:00) - PLID 60536
FinalStatement Finalize(const Statement& statement, const Batch& batch)
{
	FinalStatement final;

	if (statement.text.IsEmpty()) {
		final.text = statement.raw;
		return final;
	}

	// (a.walling 2014-01-30 00:00) - PLID 60550 - NxAutoQuantum - Scan quantized statements for external @variables and scope_identity() and convert to output variables and @@identity
	boost::container::flat_set<CiString> externalVars;
	ProcessStatementText(statement, externalVars, final.text);

	// first external vars
	{
		for each (const CiString& varName in externalVars) {
			_ASSERTE(batch.variables_.count(varName));

			CString varType;

			// (a.walling 2014-01-31 15:43) - PLID 60551 - If not found in global vars, just ignore, since it is likely one declared within this batch.
			Batch::VariableMap::const_iterator it = batch.variables_.find(varName);

			if (it != batch.variables_.end()) {
				varType = it->second;

				final.params.AppendFormat("%s %s OUTPUT, ", varName, varType);
				final.values.AppendFormat("%s OUTPUT, ", varName);
			}
		}
	}

	// then the arguments
	{
		for each (const Parameter& arg in statement.args) {
			final.params.AppendFormat("%s %s, ", arg.name, CalcDataType(arg, batch));
			final.values.AppendFormat("%s, ", arg.value);
		}
	}

	final.params.TrimRight(", ");
	final.values.TrimRight(", ");
	
	return final;
}

namespace {
	template<typename Generator>
	CString GenerateFlatBatch(const Batch& batch, Generator gen)
	{
		CString header;
		CString body;

		//

		header.Preallocate(batch.variables_.size() * 64);
		for each (const Batch::VariableMap::value_type& val in batch.variables_) {		
			header.AppendFormat("DECLARE %s %s;\r\n", val.first, val.second);
		}

		//

		body.Preallocate( (batch.statements_.size() * 128) );
		for each (const Statement& statement in batch.statements_) {
			if (statement.text.IsEmpty()) {
				body.AppendFormat("%s\r\n", statement.raw);
			} else {
				FinalStatement final = Finalize(statement, batch);
				
				gen(header, body, statement, final);
			}
		}

		//

		return header + body;
	}

	namespace Style
	{
		struct Classic
		{
			void operator()(CString& header, CString& body, const Statement& statement, const FinalStatement& final)
			{
				if (!statement.raw.IsEmpty()) {
					body.AppendFormat("%s\r\n", statement.raw);
				} else if (!final.text.IsEmpty() && final.params.IsEmpty() && final.values.IsEmpty()) {
					body.AppendFormat("%s\r\n", final.text);
				}
			}
		};
		
		struct Sproc
		{
			void operator()(CString& header, CString& body, const Statement& statement, const FinalStatement& final)
			{
				if (final.params.IsEmpty() && final.values.IsEmpty()) {
					body.AppendFormat("EXEC('%s');\r\n", _Q(final.text));
					return;
				}
				body.AppendFormat(
					"EXEC sp_executesql N'%s', N'%s', %s;\r\n"
					, _Q(final.text)
					, _Q(final.params)
					, final.values
				);
			}
		};
		
		struct Odbc
		{
			void operator()(CString& header, CString& body, const Statement& statement, const FinalStatement& final)
			{
				if (final.params.IsEmpty() && final.values.IsEmpty()) {
					body.AppendFormat("EXEC('%s');\r\n", _Q(final.text));
					return;
				}
				body.AppendFormat(
					"{call sp_executesql(N'%s', N'%s', %s)}\r\n"
					, _Q(final.text)
					, _Q(final.params)
					, final.values
				);
			}
		};

		template<typename DerivedStyleType>
		struct MinimalBase
		{
			boost::container::flat_map<CString, long> mapQueryText;
			boost::container::flat_map<CString, long> mapQueryParams;
			
			void operator()(CString& header, CString& body, const Statement& statement, const FinalStatement& final)
			{
				if (final.params.IsEmpty() && final.values.IsEmpty()) {
					body.AppendFormat("EXEC('%s');\r\n", _Q(final.text));
					return;
				}

				long& queryTextId = mapQueryText[final.text];
				long& queryParamsId = mapQueryParams[final.params];

				if (!queryTextId) {
					queryTextId = (long)mapQueryText.size();
					header.AppendFormat(
						"DECLARE @q%02li NVARCHAR(MAX);\r\n"
						"SET @q%02li = '%s';\r\n"
						, queryTextId
						, queryTextId
						, _Q(final.text)
					);
				}
				if (!queryParamsId) {
					queryParamsId = (long)mapQueryParams.size();
					header.AppendFormat(
						"DECLARE @a%02li NVARCHAR(MAX);\r\n"
						"SET @a%02li = '%s';\r\n"
						, queryParamsId
						, queryParamsId
						, _Q(final.params)
					);
				}
					
				static_cast<DerivedStyleType*>(this)->AddMinimalStatement(body, queryTextId, queryParamsId, final.values);
			}			
		};

		struct MinimalSproc : public MinimalBase<MinimalSproc>
		{
			void AddMinimalStatement(CString& body, long queryTextId, long queryParamsId, const CString& values)
			{
				body.AppendFormat(
					"exec sp_executesql @q%02li, @a%02li, %s;\r\n"
					, queryTextId
					, queryParamsId
					, values
				);
			}
		};

		struct MinimalOdbc : public MinimalBase<MinimalOdbc>
		{
			void AddMinimalStatement(CString& body, long queryTextId, long queryParamsId, const CString& values)
			{
				body.AppendFormat(
					"{call sp_executesql(@q%02li, @a%02li, %s)}\r\n"
					, queryTextId
					, queryParamsId
					, values
				);
			}
		};
	}
}

CString Batch::FlattenToQuantum() const
{
//#ifdef NXQUANTUM_EMR_DEBUG
//	CString strClassic = GenerateFlatBatch(*this, Style::Classic());
//	CString strSproc = GenerateFlatBatch(*this, Style::Sproc());
//	CString strOdbc = GenerateFlatBatch(*this, Style::Odbc());
//	CString strMinimalSproc = GenerateFlatBatch(*this, Style::MinimalSproc());
//	CString strMinimalOdbc = GenerateFlatBatch(*this, Style::MinimalOdbc());
//	CString strFinalBatch = GenerateFlatBatch(*this, Style::Sproc());
//
//	return strFinalBatch;
//#endif
#ifdef NXQUANTUM_EMR_DEBUG
	CString strClassic = GenerateFlatBatch(*this, Style::Classic());
	CString strSproc = GenerateFlatBatch(*this, Style::Sproc());

	return strSproc;
#else
	return GenerateFlatBatch(*this, Style::Sproc());
#endif
}

CString Batch::FlattenToClassic() const
{
	return GenerateFlatBatch(*this, Style::Classic());
}

// (a.walling 2014-02-05 11:23) - PLID 60546 - Flatten to sproc batch or classic batch, depending on UseAutoQuantumBatch
CString Batch::FlattenToEither() const
{
	if (UseAutoQuantumBatch()) {
		return FlattenToQuantum();
	} else {
		return FlattenToClassic();
	}
}

///

#ifdef NXQUANTUM_EMR_DEBUG
// (a.walling 2014-01-30 00:00) - PLID 60547 - Debug log file for Auto Quantum

struct CSessionPathThreadsafeLogFile
	: public CThreadsafeLogFile
{
	CSessionPathThreadsafeLogFile(const CString& strFileNameOrPath, bool bAppendTimestampAndProcessID = false)
		: CThreadsafeLogFile(strFileNameOrPath, bAppendTimestampAndProcessID)
	{}

	CSessionPathThreadsafeLogFile(void)
	{}

	virtual void EnsureLogFileHandle() override
	{
		if (!m_hLogFile) {
			CSingleLock sl(&m_cs, TRUE);
			
			if (m_strLogFile.FindOneOf("\\/") == -1) {
				m_strLogFile = GetPracPath(PracPath::SessionPath) ^ m_strLogFile;
			}
		}
		CThreadsafeLogFile::EnsureLogFileHandle();
	}
};

static CSessionPathThreadsafeLogFile LogFile("QuantumEmr.log", true);
CLogger Logger(LogFile, CLogger::eLogThreadID | CLogger::eLogFormattedTime | CLogger::eLogTicks);

#endif

// (a.walling 2014-01-30 00:00) - PLID 60547 - registry key to switch between quantum and classic
bool UseAutoQuantumBatch()
{
	static const char szLM[] = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Nextech\\Emr_UseAutoQuantumBatch";
	static const char szCU[] = "HKEY_CURRENT_USER\\SOFTWARE\\Nextech\\Emr_UseAutoQuantumBatch";

	// (a.walling 2014-01-30 00:00) - PLID 60547 - defaults to quantum in debug mode
#ifdef _DEBUG
	long defaultVal = 1;
#else
	long defaultVal = 1;
#endif
	
	long val = 2;

	if (val == 2) {
		val = NxRegUtils::ReadLong(szCU, 2);
	}
	if (val == 2) {
		val = NxRegUtils::ReadLong(szLM, 2);
	}
	
	if (val == 2) {
		// (a.walling 2014-02-06 09:26) - PLID 60547 - NxAutoQuantum - Check configrt global enable state
		static long configVal = -1;
		if (-1 == configVal) {
			configVal = GetRemotePropertyInt("Emr_UseAutoQuantumBatch", 2, 0, "<None>", true);
		}
		val = configVal;
	}

	if (val == 2) {
		val = defaultVal;
	}

	return !!val;
}

// (a.walling 2014-02-05 11:23) - PLID 60546 - Execute the flattened batch according to UseAutoQuantumBatch, log if any errors
ADODB::_RecordsetPtr ExecuteWithLog(const Batch& batch, ADODB::_ConnectionPtr pCon, ADODB::CursorLocationEnum cursorLocation)
{
	// (a.walling 2014-01-30 00:00) - PLID 60547 - Use the quantized EMR batch for execute; generate both classic and quantized batches; registry key to switch between the two
	bool bUseAutoQuantumBatch = UseAutoQuantumBatch();

	CString strFinalRecordset = bUseAutoQuantumBatch ? batch.FlattenToQuantum() : batch.FlattenToClassic();
	
	if (!pCon) {
		pCon = GetRemoteData();
	}

	try {	
		return CreateRecordsetStd(pCon, strFinalRecordset,
			ADODB::adOpenForwardOnly, ADODB::adLockReadOnly, ADODB::adCmdText, cursorLocation);
	} catch (_com_error e) {
		// (a.walling 2014-01-30 00:00) - PLID 60547 - Log the batch if there are errors
		try {
			Log("%s", strFinalRecordset);

			CString strErrorMessage = NxCatch::ComErrorInfo(e, __FUNCTION__, __LINE__, __FILE__, NxCatch::eIgnore, NULL).GetErrorMessage();

			Log("%s", strErrorMessage);

#ifdef NXQUANTUM_EMR_DEBUG
			Logger.LogStd(strFinalRecordset);
			Logger.LogStd(strErrorMessage);
#endif

			if (bUseAutoQuantumBatch) {
				CString strOtherRecordset = batch.FlattenToClassic();
				Log("%s", strOtherRecordset);

#ifdef NXQUANTUM_EMR_DEBUG
				Logger.LogStd(strOtherRecordset);
#endif
			} else {
#ifdef NXQUANTUM_EMR_DEBUG
				CString strOtherRecordset = batch.FlattenToQuantum();
				Logger.LogStd(strOtherRecordset);
#endif
			}
		} NxCatchAllIgnore();

		//In any case, re-throw the error to maintain existing behavior.
		throw e;
	}
}

}

}

#pragma pop_macro("min")
#pragma pop_macro("max")
