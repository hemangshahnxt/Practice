#pragma once

#include "NxAutoQuantumFwd.h"

#include <vector>
#include <boost/container/flat_map.hpp>

#include <NxDataUtilitiesLib/iString.h>

#include <NxDataUtilitiesLib/vaUtils.h>
#include <NxSystemUtilitiesLib/LoggingUtils.h>

#ifdef _DEBUG
#define NXQUANTUM_EMR_DEBUG
#endif

// (a.walling 2014-01-30 00:00) - PLID 60536 - NxAutoQuantum - auto parameterize / quantize legacy batches eg for the EMR save

namespace Nx
{

namespace Quantum
{

	// (a.walling 2014-01-30 00:00) - PLID 60536 - Parameter with name, value, and type hint. nvarchar will implicitly convert to pretty much everything except binary
	struct Parameter
	{
		CiString name;
		CString value;
		const char* typeHint;

		Parameter(const CString& name = CString(), const CString& value = CString(), const char* typeHint = NULL)
			: name(name)
			, value(value)
			, typeHint(typeHint)
		{}

		const char* GetDataType() const;

		friend bool operator<(const Parameter& l, const Parameter& r)
		{
			return l.name < r.name;
		}
	};

	// (a.walling 2014-01-30 00:00) - PLID 60536 - Holds the parameterized text and args, along with the raw text for classic output / raw output
	struct Statement
	{
		CString text;
		CString raw;
		std::vector<Parameter> args;

		Statement()
		{}

		static Statement MakeRaw(const CString& str)
		{
			Statement s;
			s.raw = str;
			return s;
		}

		// (a.walling 2014-01-30 00:00) - PLID 60549 - NxAutoQuantum - Parse CSqlFragment / ParamSql and quantize
		static Statement ParseParamQuery(const CString& strFinalSql, const CNxParamSqlArray& aryParams);

		// (a.walling 2014-01-30 00:00) - PLID 60548 - NxAutoQuantum - Parse format strings and quantize
		static Statement ParseFormatV(LPCTSTR strStatementFmt, va_list args);

		//

		static Statement ParseFragment(CSqlFragment sql)
		{ return ParseParamQuery(sql->m_strSql, sql->m_aryParams); }

		static Statement ParseFormat(LPCTSTR strStatementFmt, ...)
		{ return ParseFormatV(strStatementFmt, va_rval(strStatementFmt)); }
		
		static Statement ParseParam(LPCTSTR strStatementFmt, ...)
		{ return ParseParamV(strStatementFmt, va_rval(strStatementFmt)); }

		static Statement ParseParamV(LPCTSTR strStatementFmt, va_list args)
		{ CSqlFragment sql; return ParseFragment(sql.CreateV(strStatementFmt, args)); }

		//

		explicit Statement(const CString& text)
			: text(text)
			, raw(text)
		{}
	};

	// (a.walling 2014-01-30 00:00) - PLID 60536 - Keeps track of variables and statements and the final batch generation.
	struct Batch
	{
		typedef boost::container::flat_map<CiString, CString> VariableMap;

		std::vector<Statement> statements_;
		VariableMap variables_;

		///

		CString FlattenToQuantum() const;
		CString FlattenToClassic() const;

		// (a.walling 2014-02-05 11:23) - PLID 60546 - Flatten to sproc batch or classic batch, depending on UseAutoQuantumBatch
		CString FlattenToEither() const;

		///
		Batch& AddStatement(const Statement& s)
		{ statements_.push_back(s); return *this; }

		Batch& AddStatement(const CString& str)
		{ return AddStatement(Statement(str)); }

		Batch& AddFragment(CSqlFragment sql)
		{ return AddStatement(Statement::ParseFragment(sql)); }

		Batch& AddRaw(const CString& str)
		{ statements_.push_back(Statement::MakeRaw(str)); return *this; }

		// (a.walling 2014-01-30 00:00) - PLID 60550 - Declare variables and types explicitly so we don't have to rely on guessing the type for output params etc
		Batch& Declare(const CiString& name, const CString& type)
		{ variables_.insert(std::make_pair(name, type)); return *this; }

		///

		Batch& operator+=(const Batch& r)
		{
			statements_.insert(statements_.end(), r.statements_.begin(), r.statements_.end());
			variables_.insert(r.variables_.begin(), r.variables_.end());
			return *this;
		}

		Batch operator+(const Batch& r) const
		{
			Batch l;
			l.statements_.reserve(statements_.size() + r.statements_.size());
			l += *this;
			l += r;
			return l;
		}

		bool IsEmpty() const
		{ return statements_.empty(); }
	};

	
	inline void AddStatementToSqlBatch(IN OUT Batch& batch, LPCTSTR strStatementFmt, ...)
	{ batch.AddStatement(Statement::ParseFormatV(strStatementFmt, va_rval(strStatementFmt))); }
	
	inline void AddParamStatementToSqlBatch(IN OUT Batch& batch, LPCTSTR strStatementFmt, ...)
	{ batch.AddStatement(Statement::ParseParamV(strStatementFmt, va_rval(strStatementFmt))); }

	inline void AddParamStatementToSqlBatch(IN OUT Batch& batch, CSqlFragment sql)
	{ batch.AddFragment(sql); }

	inline void AddStatementToSqlBatch(IN OUT Batch& batch, CSqlFragment sql)
	{ batch.AddFragment(sql); }

	inline void AddStatementToSqlBatch(IN OUT Batch& batch, const Batch& r)
	{ batch += r; }
	inline void AddSqlBatchToSqlBatch(IN OUT Batch& batch, IN const Batch& r)
	{ batch += r; }

	///

	// (a.walling 2014-01-30 00:00) - PLID 60547 - registry key to switch between quantum and classic
	bool UseAutoQuantumBatch();

	// (a.walling 2014-02-05 11:23) - PLID 60546 - Execute the flattened batch according to UseAutoQuantumBatch, log if any errors
	ADODB::_RecordsetPtr ExecuteWithLog(const Batch& batch, ADODB::_ConnectionPtr pCon = NULL, ADODB::CursorLocationEnum cursorLocation = ADODB::adUseClient);
	
#ifdef NXQUANTUM_EMR_DEBUG
	// (a.walling 2014-01-30 00:00) - PLID 60547 - Debug log file for Auto Quantum
	extern CLogger Logger;
#endif

}

}