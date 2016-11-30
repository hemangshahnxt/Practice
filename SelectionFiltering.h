// (r.gonet 10/23/2013) - PLID 56236 - Added. Classes that deal with converting a letter writing filter into a SQL selection statement.

#pragma once

#include <stack>

// (r.gonet 10/23/2013) - PLID 56236 - Forward declarations
enum FilterBasedOnEnum;
class CFilter;

// (r.gonet 10/23/2013) - PLID 59833 - Describes a field that is in the select list of a CSubQuery object.
class CSelectedField
{
private:
	// (r.gonet 10/23/2013) - PLID 59833 - The internal name of the field. ie where the data comes from. Should be qualified to avoid ambiguity.
	CString m_strInternalFieldName;
	// (r.gonet 10/23/2013) - PLID 59833 - The unaffixed apparent name of the field. No prefix. No suffix.
	CString m_strUnaffixedFieldName;
	// (r.gonet 11/26/2013) - PLID 59833 - Suffix of the apparent field
	long m_nSuffixNumber;
	// (r.gonet 10/23/2013) - PLID 59833 - The filter field base type if this is from a CFilterFieldInfo. Defaults to fboPerson.
	FilterBasedOnEnum m_fboType;
	// (r.gonet 10/23/2013) - PLID 59833 - The CFilterFieldInfo info ID. -1 if not from a CFilterFieldInfo.
	long m_nFilterFieldInfoID;
	// (r.gonet 10/23/2013) - PLID 59833 - Whether the field is a key or not within the containing query. Key fields are not propegated up.
	bool m_bIsKey;
	// (r.gonet 10/23/2013) - PLID 59833 - What order this field is placed in the select list.
	long m_nSelectOrderIndex;

public:
	// (r.gonet 10/23/2013) - PLID 59833 - Constructs an empty CSelectedField object.
	CSelectedField();
	// (r.gonet 11/27/2013) - PLID 59833 - Constructs a new CSelectedField
	CSelectedField(CString strInternalFieldName, CString strUnaffixedFieldName, long nSuffixNumber, 
		FilterBasedOnEnum fboType, long nFilterFieldInfoID, bool bIsKey, long nSelectOrderIndex);
	// (r.gonet 10/23/2013) - PLID 59833 - Copy Constructor
	CSelectedField(const CSelectedField &other);
	// (r.gonet 11/27/2013) - PLID 59833 - Gets the internal name of the field. This may be a qualified database field name, or it might be something more complex like a case statement
	CString GetInternalFieldName();
	// (r.gonet 11/27/2013) - PLID 59833 - Sets the internal name of the selected field.
	void SetInternalFieldName(CString &strInternalFieldName);
	// (r.gonet 11/27/2013) - PLID 59833 - Gets the apparent field name without any prefix or suffix
	CString GetUnaffixedFieldName();
	// (r.gonet 11/27/2013) - PLID 59833 - Sets the apparent field name without any prefix or suffix
	void SetUnaffixedFieldName(CString &strUnaffixedFieldName);
	// (r.gonet 11/27/2013) - PLID 59833 - Gets the name of the field as it will appear in recordset. This may
	// be prefixed or suffixed to avoid ambiguity.
	CString GetApparentFieldName();
	// (r.gonet 11/27/2013) - PLID 59833 - Get the name of the base type
	CString GetPrefix();
	// A suffix of 0 is the default suffix and should not be displayed.
	long GetSuffixNumber();
	// A suffix of 0 is the default suffix and should not be displayed.
	void SetSuffixNumber(long nSuffixNumber);
	// (r.gonet 11/27/2013) - PLID 59833 - Gets the filter field base type that this selected field represents
	FilterBasedOnEnum GetBaseType();
	// (r.gonet 11/27/2013) - PLID 59833 - Sets the filter field base type that this selected field represents
	void SetBaseType(FilterBasedOnEnum fboType);
	// (r.gonet 11/27/2013) - PLID 59833 - Gets the id of the corresponding filter field
	long GetFilterFieldInfoID();
	// (r.gonet 11/27/2013) - PLID 59833 - Sets the id of the corresponding filter field
	void SetFilterFieldInfoID(long nFilterFieldInfoID);
	// (r.gonet 11/27/2013) - PLID 59833 - Returns true if this selected field is a key in the containing query. Keys are not propegated to the parent query's select list.
	bool GetIsKey();
	// (r.gonet 11/27/2013) - PLID 59833 - Sets whether this is a key in the containing query. Keys are not propegated to the parent query's select list.
	void SetIsKey(bool bIsKey);
	// (r.gonet 11/27/2013) - PLID 59833 - Gets the order in which this selected field appears in the select list
	long GetSelectOrderIndex();
	// (r.gonet 11/27/2013) - PLID 59833 - Sets the order in which tihs selected field appears in the select list
	void SetSelectOrderIndex(long nSelectOrderIndex);
};
typedef boost::shared_ptr<CSelectedField> CSelectedFieldPtr;

typedef CMap<CString, LPCTSTR, CSelectedFieldPtr, CSelectedFieldPtr> CMapStringToField;

class CSubQuery;
typedef boost::shared_ptr<CSubQuery> CSubQueryPtr;
typedef boost::shared_ptr<CFilter> CFilterPtr;
class CFilteredSelect;

// (r.gonet 11/27/2013) - PLID 59202 - An in-memory SQL query that is constructed from a CFilter object. Is used to convert a CFilter object to an SQL string.
class CSubQuery
{
public:
	// (r.gonet 11/27/2013) - PLID 59833 - Sort comparator for selected fields.
	struct SelectedFieldComparator
	{
		// (r.gonet 11/27/2013) - PLID 59833 - Sort comparator for selected fields.
		bool operator()(CSelectedFieldPtr &a, CSelectedFieldPtr &b);
		// (r.gonet 11/27/2013) - PLID 59833 - Gets whether one filter base type should come after the other.
		int GetTypeOrder(FilterBasedOnEnum fbo);
	};

	class CSqlPredicate;
	typedef boost::shared_ptr<CSqlPredicate> CSqlPredicatePtr;

	// (r.gonet 10/23/2013) - PLID 59833 - Describes a SQL predicate in the form:
	// [[LhsSchema.]LhsObject] [Operator] [RhsSchema.]RhsObject
	// or alternatively
	// [[LhsSchema.]LhsObject] [Operator] RhsAlias
	class CSqlPredicate
	{
	private:
		// (r.gonet 10/23/2013) - PLID 59833 - The name of the table source that qualifies the LHS object. eg "PatientsT"
		CString m_strLHSSchema;
		// (r.gonet 10/23/2013) - PLID 59833 - The name of the LHS object. eg "PersonID"
		CString m_strLHSObject;
		// (r.gonet 10/23/2013) - PLID 59833 - The operator of the predicate. eg "="
		CString m_strOperator;
		// (r.gonet 10/23/2013) - PLID 59833 - The name of the table source that qualifies the RHS object. eg "PersonT"
		CString m_strRHSSchema;
		// (r.gonet 10/23/2013) - PLID 59833 - The name of the RHS object. eg "ID"
		CString m_strRHSObject;
		// (r.gonet 10/23/2013) - PLID 59833 - The alias of the RHS. eg "ThePersonID". If present replaces [RhsSchema.]RhsObject in the output string.
		CString m_strRHSAlias;

	public:
		// (r.gonet 10/23/2013) - PLID 59833 - Constructs an empty CSqlPredicate
		CSqlPredicate();
		// (r.gonet 10/23/2013) - PLID 59833 - Constructs a CSqlPredicate of the form [[strLHSSchema.]strLHSObject] [strOperator] ([strRHSSchema.]strRHSObject | strRHSAlias)
		CSqlPredicate(CString strLHSSchema, CString strLHSObject, CString strOperator, CString strRHSSchema, CString strRHSObject, CString strRHSAlias);
		// (r.gonet 10/23/2013) - PLID 59833 - Constructs a CSqlPrediacte of the form [strOperator] strRHSObject
		CSqlPredicate(CString strOperator, CString strRHSObject);
		// (r.gonet 10/23/2013) - PLID 59833 - Constructs a CSqlPredicate from raw SQL. Will not be parsed into components.
		CSqlPredicate(CString strRawSql);
		// (r.gonet 10/23/2013) - PLID 59833 - Copy constuctor.
		CSqlPredicate(const CSqlPredicate &other);

		// (r.gonet 12/02/2013) - PLID 59820 - Formats the predicate into a SQL fragment string.
		CString ToString();

		// (r.gonet 10/23/2013) - PLID 59833 - Gets the LHS Schema
		CString GetLHSSchema();
		// (r.gonet 10/23/2013) - PLID 59833 - Sets the LHS Schema
		void SetLHSSchema(CString &strLHSSchema);
		// (r.gonet 10/23/2013) - PLID 59833 - Gets the LHS Object
		CString GetLHSObject();
		// (r.gonet 10/23/2013) - PLID 59833 - Sets the LHS Object
		void SetLHSObject(CString &strLHSObject);
		// (r.gonet 10/23/2013) - PLID 59833 - Gets the Operator
		CString GetOperator();
		// (r.gonet 10/23/2013) - PLID 59833 - Sets the Operator
		void SetOperator(CString &strOperator);
		// (r.gonet 10/23/2013) - PLID 59833 - Gets the RHS Schema
		CString GetRHSSchema();
		// (r.gonet 10/23/2013) - PLID 59833 - Sets the RHS Schema
		void SetRHSSchema(CString &strRHSSchema);
		// (r.gonet 10/23/2013) - PLID 59833 - Gets the RHS Object
		CString GetRHSObject();
		// (r.gonet 10/23/2013) - PLID 59833 - Sets the RHS Object
		void SetRHSObject(CString &strRHSObject);
		// (r.gonet 10/23/2013) - PLID 59833 - Gets the RHS Alias
		CString GetRHSAlias();
		// (r.gonet 10/23/2013) - PLID 59833 - Sets the RHS Alias
		void SetRHSAlias(CString &strRHSAlias);
	};

	class CSqlCondition;
	typedef boost::shared_ptr<CSqlCondition> CSqlConditionPtr;

	// (r.gonet 10/23/2013) - PLID 59833 - Describes a recursive SQL Condition grammatical structure.
	// Each CSqlCondition can contain either be an SQL Predicate or contain child SQL Conditions which are in either an ORed or ANDed together.
	class CSqlCondition
	{
	public:
		// (r.gonet 11/27/2013) - PLID 59833 - Enumeration type for the condition type
		enum ESqlConditionType
		{
			esctPredicate,
			esctAnd,
			esctOr,
		};

	private:
		// How the child conditions are aggregated.
		ESqlConditionType m_esctConditionType;
		// Iff this condition is a container for nested conditions, these are the child conditions.
		std::vector<CSqlConditionPtr> m_vecChildConditions;
		// Iff this condition is a leaf in the condition tree, then this will be non-null.
		CSqlPredicatePtr m_pPredicate;
		// Whether the contained conditions are negated.
		bool m_bNegated;

	public:
		// (r.gonet 11/27/2013) - PLID 59833 - Constructs a new CSqlCondition that just contains a single predicate
		CSqlCondition(CSqlPredicatePtr pPredicate);
		// (r.gonet 11/27/2013) - PLID 59833 - Constructs a new CSqlCondition that contains multiple subconditions joined by a certain operator
		// specified in the condition type
		CSqlCondition(ESqlConditionType esctConditionType);
		
		// (r.gonet 11/27/2013) - PLID 59833 - Adds a child condition node to this condition.
		bool AddChildCondition(CSqlConditionPtr pChildCondition);
		// (r.gonet 11/27/2013) - PLID 59833 - Adds a leaf condition that contains a predicate.
		bool AddPredicate(CSqlPredicatePtr pPredicate);
		// (r.gonet 12/02/2013) - PLID 59820 - Gets the SQL string representation of this condition. Walks the condition tree.
		CString ToString();
		// (r.gonet 11/27/2013) - PLID 59833 - Negates this current condition node
		void Negate();
	};

	// (r.gonet 10/23/2013) - PLID 59833 - Describes an SQL table source grammatical structure.
	class CSqlTableSource
	{
	public:
		// (r.gonet 11/27/2013) - PLID 59833 - Enumeration type for the join type
		enum ETableSourceJoinType
		{
			etsjtNone,
			etsjtInnerJoin,
			etsjtLeftJoin,
		};
	private:
		// (r.gonet 11/27/2013) - PLID 59833 - How this table source is joined to the query's table source list
		ETableSourceJoinType m_eJoinType;
		// (r.gonet 11/27/2013) - PLID 59833 - The name of the table in a simple table source
		CString m_strTableName;
		// (r.gonet 11/27/2013) - PLID 59833 - The subquery in a subquery table source
		CSubQueryPtr m_pSubQuery;
		// (r.gonet 11/27/2013) - PLID 59833 - The alias for the table source in the query
		CString m_strTableSourceAlias;
		// (r.gonet 11/27/2013) - PLID 59833 - A condition tree serving as the join condition
		CSqlConditionPtr m_pJoinCondition;

	public:
		// (r.gonet 11/27/2013) - PLID 59833 - Constructs a table source from a unstructured SQL that hopefully represents a subquery or table.
		CSqlTableSource(CString strRawSql);
		// (r.gonet 11/27/2013) - PLID 59833 - Constructs a table source that represents a simple joined table.
		explicit CSqlTableSource(ETableSourceJoinType eJoinType, CString strTableName, CString strTableSourceAlias, CSqlConditionPtr pJoinCondition);
		// (r.gonet 11/27/2013) - PLID 59833 - Constructs a table source that represents a joined subquery.
		explicit CSqlTableSource(ETableSourceJoinType eJoinType, CSubQueryPtr pSubQuery, CSqlConditionPtr pJoinCondition);
		// (r.gonet 12/02/2013) - PLID 59820 - Gets the SQL string that represents this table source.
		CString ToString();
	};
	typedef boost::shared_ptr<CSqlTableSource> CSqlTableSourcePtr;

	// (r.gonet 11/27/2013) - PLID 59869 - Class that stores the SQL information related to CFilterDetail objects from a CFilter object.
	class CFilterDetailSqlInfo
	{
	private:
		// (r.gonet 11/27/2013) - PLID 59869 - The ID field is the SQL field that will be used in the filter where conditions. It may just be a number though, so we have the m_strValueField in case there is a more user friendly field to select. 
		CString m_strIDField;
		// (r.gonet 11/27/2013) - PLID 59869 - The value field is the SQL field that will actually be selected in the select list and visible to the user. This may be the same as the m_strIDField.
		CString m_strValueField;
		// (r.gonet 11/27/2013) - PLID 59869 - If true, the detail's SQL has already been added to the select list. If false, the detail's SQL has not been added yet to the select list.
		bool m_bSelected;

	public:
		// (r.gonet 11/27/2013) - PLID 59869 - Constructs an empty CFilterDetailSqlInfo
		CFilterDetailSqlInfo();

		// (r.gonet 11/27/2013) - PLID 59869 - Gets the ID field
		CString GetIDField();
		// (r.gonet 11/27/2013) - PLID 59869 - Sets the ID field
		void SetIDField(CString &strIDField);
		// (r.gonet 11/27/2013) - PLID 59869 - Gets the value field
		CString GetValueField();
		// (r.gonet 11/27/2013) - PLID 59869 - Sets the value field
		void SetValueField(CString &strValueField);
		// (r.gonet 11/27/2013) - PLID 59869 - Returns true if the corresponding detail has been added to the select list. Returns false if the corresponding detail has not been added to the select list. 
		bool GetSelected();
		// (r.gonet 11/27/2013) - PLID 59869 - Sets whether the corresponding detail has been added to the select list. 
		// True if the corresponding detail has been added to the select list. False if the corresponding detail has not been added to the select list. 
		void SetSelected(bool bSelected);
	};
	typedef boost::shared_ptr<CFilterDetailSqlInfo> CFilterDetailSqlInfoPtr;

	// (r.gonet 11/27/2013) - PLID 59829 - Forward declaration
	class CSubFilterFrame;
	// (r.gonet 11/27/2013) - PLID 59829
	typedef boost::shared_ptr<CSubFilterFrame> CSubFilterFramePtr;

	// (r.gonet 11/27/2013) - PLID 59829 - Represents an evaluation context for the CSubQuery
	class CSubFilterFrame
	{
	private:
		// (r.gonet 11/27/2013) - PLID 59829 - The filter being evaluated
		CFilterPtr m_pFilter;
		// (r.gonet 11/27/2013) - PLID 59829 - Whether this filter is negated (such as by a NOT IN operator)
		bool m_bIsNegated;
		// (r.gonet 11/27/2013) - PLID 59829 - What subfilter frame contains this frame. Will be null if this is the top level frame.
		CSubFilterFramePtr m_pParentFrame;
		// (r.gonet 11/27/2013) - PLID 59829 - The CFilterDetail that contains this the subfilter referenced by m_pFilter. 
		// Will be null if this is the top level frame, since nothing contains the top level filter.
		CFilterDetail *m_pParentFilterDetail;
	public:
		// (r.gonet 11/27/2013) - PLID 59829 - Constructs a new CSubFilterFrame
		CSubFilterFrame(CFilterPtr pFilter);
		// (r.gonet 11/27/2013) - PLID 59829 - Constructs a new CSubFilterFrame
		CSubFilterFrame(CFilterPtr pFilter, bool bIsNegated, CSubFilterFramePtr pParentFrame, CFilterDetail *pParentFilterDetail);

		// (r.gonet 11/27/2013) - PLID 59829 - Gets the filter
		CFilterPtr GetFilter();
		// (r.gonet 11/27/2013) - PLID 59829 - Gets whether the frame is negated
		bool GetIsNegated();
		// (r.gonet 11/27/2013) - PLID 59829 - Gets the parent frame
		CSubFilterFramePtr GetParentFrame();
		// (r.gonet 11/27/2013) - PLID 59829 - Gets the parent filter detail
		CFilterDetail *GetParentFilterDetail();
	};
	
private:
	// (r.gonet 11/27/2013) - PLID 59824 - A reference to the SQL batch of statements that contains this query.
	CFilteredSelect& m_batch;
	// (r.gonet 11/27/2013) - PLID 59828 - The alias of this subquery in the context of the parent query.
	CString m_strAlias;
	// (r.gonet 11/27/2013) - PLID 59202 - A map of CFilterDetails' detail strings to SQL Info representing the fields in the query that correspond to the detail.
	CMap<CString, LPCTSTR, CFilterDetailSqlInfoPtr, CFilterDetailSqlInfoPtr> m_mapFilterDetailSqlInfo;
	// (r.gonet 11/27/2013) - PLID 59828 - A map of CFilterDetails' detail strings to subqueries. Such details are subfilter details.
	CMap<CString, LPCTSTR, CSubQueryPtr, CSubQueryPtr&> m_mapDetailStringToSubQuery;
	// (r.gonet 11/27/2013) - PLID 59869 - A map of field names to fields in the select list of this query.
	CMapStringToField m_mapSelectList;
	// (r.gonet 11/27/2013) - PLID 59202 - A list of the table sources of this query. Can be simple tables or subqueries.
	std::list<CSqlTableSourcePtr> m_lstTableSources;
	// (r.gonet 11/27/2013) - PLID 59869 - The where clause of this query.
	CSqlConditionPtr m_pWhereClause;
	// (r.gonet 11/27/2013) - PLID 59829 - A stack of subfilters that are being evaluated by this query. Though these are at 
	// separate hierarchies in the CFilter, they will all be flattened into the current query.
	std::stack<CSubFilterFramePtr> m_stackFrames;
public:
	// (r.gonet 11/27/2013) - PLID 59202 - Constructs a new CSubQuery with the intention of it being a top level query
	CSubQuery(CFilteredSelect& batch, CFilterPtr pFilter);
	// (r.gonet 11/27/2013) - PLID 59202 - Constructs a new CSubQuery with the intention of it being a child subquery
	CSubQuery(CFilteredSelect& batch, CFilterPtr pFilter, bool bIsNegated, CSubFilterFramePtr& pParentFrame, CFilterDetail *pParentFilterDetail, CString &strAlias);

	// (r.gonet 11/27/2013) - PLID 59202 - Creates an in memory query for a CFilter object passed in the constructor. Can later be converted to a SQL string.
	bool Create();
	// (r.gonet 11/27/2013) - PLID 59820 - Retrieves the columns that are defined as keys in this query's select list.
	void GetKeyColumns(OUT std::vector<CSelectedFieldPtr> &vecKeyColumns);
	// (r.gonet 12/02/2013) - PLID 59820 - Generate the SQL string for this query. If strOutputTempTableName is specified, then we'll output the records to a temp table.
	CString ToString(LPCTSTR strOutputTempTableName = NULL);
	// (r.gonet 11/27/2013) - PLID 58821 - Returns the map containing all the fields selected in the query
	CMapStringToField& GetSelectedFieldsMap();

private:
	// (r.gonet 11/27/2013) - PLID 59828 - Gets the alias of the current subquery within the parent query
	CString GetAlias();
	// (r.gonet 11/27/2013) - PLID 59829 - Gets the current subfilter evaluation context frame
	CSubFilterFramePtr GetCurrentFrame();
	// (r.gonet 11/27/2013) - PLID 59869 - Looks up SQL Info for a filter detail using the filter detail as a key
	CFilterDetailSqlInfoPtr GetFilterDetailSqlInfo(CFilterDetail *pFilterDetail);
	// (r.gonet 11/27/2013) - PLID 59869 - Caches SQL Info for a filter detail for later lookup using the filter detail as a key
	void StoreFilterDetailSqlInfo(CFilterDetail *pFilterDetail, CFilterDetailSqlInfoPtr pFilterDetailSqlInfo);
	// Looks up a subquery using the filter detail as a key
	CSubQueryPtr GetChildSubQuery(CFilterDetail *pFilterDetail);
	// Caches a subquery for later lookup using the filter detail as a key
	void StoreChildSubQuery(CFilterDetail *pFilterDetail, CSubQueryPtr pChildSubQuery);
	// (r.gonet 11/27/2013) - PLID 59202 - Adds a field to the select list of the query. The internal field name represents the source of the field. It may be qualified with a table name or even be an expression.
	// The apparent name is what we select as the alias for the field.
	void AddFieldToSelectList(CString strInternalFieldName, CString strApparentFieldName, long nFilterFieldInfoID, FilterBasedOnEnum fboType, bool bIsKey = false, long nSuffixNumber = 0);
	// Adds the select list from a subquery to the current query. Ensures the field names remain unique.
	void ImportSelectListFields(CSubQueryPtr pFromQuery);
	// (r.gonet 11/27/2013) - PLID 59869 - Adds a table source to the query's list of table sources.
	bool AddTableSource(CSqlTableSourcePtr pTableSource);
	// (r.gonet 11/27/2013) - PLID 59828 - Adds a subquery to the current query's list of table sources
	bool AddSubQuery(CSubQueryPtr pSubQuery);
	// (r.gonet 12/02/2013) - PLID 59823 - Add the always selected key fields, which are fields necessary to join this subquery with the outer query.
	void AddKeyFields();
	// (r.gonet 12/02/2013) - PLID 59823 - Certain fields we always want to select depending on the base type. These are things that better identify the record selected.
	void AddStaticFields();
	// (r.gonet 11/27/2013) - PLID 59828 - We need to know how to join a parent subquery to a child subquery of different base types.
	// These joins are defined nowhere in LW filtering. Define them here.
	CSqlPredicatePtr GetSubFilterJoinPredicate(FilterBasedOnEnum fboParent, FilterBasedOnEnum fboChild);
	// (r.gonet 11/27/2013) - PLID 59202 - Adds the select list from a subquery to the current query. Ensures the field names remain unique.
	void SortSelectList(OUT std::vector<CSelectedFieldPtr> &vecSortedSelectedFields);
	// (r.gonet 11/27/2013) - PLID 59829 - Creates a new subfilter frame from a filter detail. OK, here's the deal with subfilter frames. We don't have a
	// one to one relationship between SQL subqueries and subfilters. We add a new SQL subquery only if the subfilter
	// has a different base type than the containing filter. If they have the same base type, then we "flatten"
	// the subfilter into the parent. We do this to avoid repeating static fields. A subfilter frame represents the the evaluation
	// of a same type subfilter in the same SQL subquery's nest level.
	CSubFilterFramePtr NewFrame(CFilterDetail *pFilterDetail);
	// (r.gonet 11/27/2013) - PLID 59828 - Generates a child subquery that will represent a given filter detail
	CSubQueryPtr NewSubQuery(CFilterDetail *pFilterDetail);
	// (r.gonet 11/27/2013) - PLID 59869 - Gets the where clause of a SQL query.
	CSqlConditionPtr GetWhereClause();
	// (r.gonet 11/27/2013) - PLID 59869 - Sets the where clause of the SQL query.
	void SetWhereClause(CSqlConditionPtr pWhereClause);
	
	// (r.gonet 11/27/2013) - PLID 59823 - Takes a filter field and map of joins and adds to the query's table sources the dependencies of the filter field.
	bool MoveJoinFromMapToClause(const CFilterFieldInfo *pffi, CMapStringToPtr &mapJoins);
	// (r.gonet 11/27/2013) - PLID 59869 - Takes a filter in the sum of products form of A*B+A*C+A+B*C*D and splits it into its individual products A*B, A*C, A, B*C*D
	void SplitProducts(CFilterPtr pFilter, OUT std::list<CFilterPtr> &lstProductFilters);	
	// (r.gonet 11/27/2013) - PLID 59823 - Adds the current filter's base type's dependency table sources to the subquery's table sources.
	bool AddBaseTableSources();
	// (r.gonet 11/27/2013) - PLID 59826 - Adds the table source for combo values type fields
	void AddComboValuesTableSource(CFilterDetail *pFilterDetail);
	// (r.gonet 11/27/2013) - PLID 59824 - Create (or reuse) a temp table which will hold the dynamic ids and friendly names of values the user chose from when setting up the filter detail.
	CString GetComboSelectTempTable(CFilterDetail *pFilterDetail);
	// (r.gonet 11/27/2013) - PLID 59824 - Add the table sources required by combo select type fields such where the user has selected some value when setting up the filter detail from a dynamic set of values
	// retrieved by some SQL statement.
	void AddComboSelectTableSource(CFilterDetail *pFilterDetail);
	// (r.gonet 11/27/2013) - PLID 59827 - Add the table sources required by simple fields such as text, number, etc. that all depend on the base type's dependencies.
	void AddSimpleTypeTableSource(CFilterDetail *pFilterDetail);
	// (r.gonet 11/27/2013) - PLID 59828 - Add the table sources required by a subfilter type field.
	bool AddSubFilterTableSource(CFilterDetail *pFilterDetail);
	bool AddAdvancedFilterTableSource(CFilterDetail *pFilterDetail);
	// (r.gonet 11/27/2013) - PLID 59869 - Each detail references fields from some table(s), so add those tables to the query.
	bool AddTableSource(CFilterDetail *pFilterDetail);
	// (r.gonet 11/27/2013) - PLID 59869 - Adds the table sources that all the filter's details depend on.
	bool AddTableSources();

	// (r.gonet 12/02/2013) - PLID 59823 - Some fields are always selected when we filter on a particular base type. We always select key fields if this
	// query is a child subquery of another, which are fields necessary to join this subquery with the outer query.
	// We also select so-called static fields. Where as all the rest of the fields are dynamically added to the select
	// list if the filter field is being used by one of the filter's details, the static fields are always added to the
	// select list.
	void AddBaseSelectFields();
	// (r.gonet 11/27/2013) - PLID 59202 - Some filter fields have in their SQL definitions placeholders for DYNAMIC_NAME. Particularly EMR related fields.
	// The detail gives us the information necessary in order to get the real apparent name of the filter detail.
	// This function gets that dynamic name.
	CString GetDynamicFieldName(CFilterDetail *pFilterDetail);
	// (r.gonet 11/27/2013) - PLID 59827 - Populate the select list with the fields from the current filter's details.
	void AddSelectFields();

	// (r.gonet 11/27/2013) - PLID 59869 - Takes a filter detail belonging to some filter and creates the SQL where clause for the detail, then appends it to a growing where condition.
	void AppendWhereCondition(CFilterDetail *pFilterDetail, IN OUT CSqlConditionPtr pWhereCondition);
	// (r.gonet 11/27/2013) - PLID 59869 - Returns the where condition for the current frame's filter.
	CSqlConditionPtr CreateWhereCondition();
	// (r.gonet 11/27/2013) - PLID 59825 - Joins a filter field that uses an EXISTS clause which contains a subquery that references the outer query.
	// For Letter Writing filtering, that's fine since all that does not need to select any of the fields it is filtering on. But here we do need to
	// select all the fields we are filtering on. EXISTS is kind of black box though. It can be a query of its own. We need to somehow get 
	// the table sources used in the EXISTS and add them onto the current query (which LW does not do) as JOINs.
	void JoinExistsTableSource(CFilterDetail *pFilterDetail, OUT CString &strSubQueryAlias, OUT CString &strSubQueryObject);
	// (r.gonet 11/27/2013) - PLID 59869 - Returns true if a field should be excluded from the select list. False if it should not.
	bool ExcludeFromSelection(long nFieldInfoID);
	// (r.gonet 11/27/2013) - PLID 59828 - Gets a filter from a subfilter filter detail.
	CFilterPtr GetFilterFromDetail(CFilterDetail *pFilterDetail);
};

// (r.gonet 11/27/2013) - PLID 59202 - Convertor that transforms a Letter Writing Filter into an SQL selection statement.
// Differs from CFilter in that it selects all fields that were involved in the match and thus all the matching records.
class CFilteredSelect
{
private:
	// (r.gonet 11/27/2013) - PLID 59202 - SQL statements that are executed before the record selection query.
	CString m_strPreSelect;
	// (r.gonet 11/27/2013) - PLID 59202 - SQL statements that are executed after the record selection query.
	CString m_strPostSelect;
	// (r.gonet 11/27/2013) - PLID 59202 - A map that relates a detail to a temp table that is defined in the batch
	CMapStringToString m_mapTempTables;
	// (r.gonet 11/27/2013) - PLID 59202 - The top level query that selects the filtered records. 
	CSubQueryPtr m_pTopLevelQuery;
	
public:
	// (r.gonet 11/27/2013) - PLID 59202 - Constructs a new CFilteredSelect. pFilter is the root filter.
	CFilteredSelect(CFilterPtr pFilter);

	// (r.gonet 11/27/2013) - PLID 59202 - Creates a filtered select using the filter from the constructor.
	bool Create();
	// (r.gonet 12/02/2013) - PLID 59820 - Generates the SQL for the filtered select
	CString ToString();
	// (r.gonet 12/02/2013) - PLID 58821 - Gets a map of all the selected fields
	CMapStringToField& GetSelectedFieldsMap();
	// (r.gonet 11/27/2013) - PLID 59202 - Adds raw SQL to be executed before the selection query.
	void AppendPreSelect(CString strPreSelect);
	// (r.gonet 11/27/2013) - PLID 59202 - Adds raw SQL to be executed after the selection query.
	void PrependPostSelect(CString strPostSelect);
	// (r.gonet 11/27/2013) - PLID 59202 - Looks up a temp table by filter detail. Returns true if found and false if not found
	bool GetTempTableName(CFilterDetail *pFilterDetail, CString &strTempTableName);
	// (r.gonet 11/27/2013) - PLID 59202 - Caches a temp table name by a detail key.
	void StoreTempTableName(CFilterDetail *pFilterDetail, CString &strTempTableName);
};

// (r.gonet 12/02/2013) - PLID 59820 - Adds a number of tabs to each line in str
CString Tabify(CString str, int nNumTabs);