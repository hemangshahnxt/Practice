#include "stdafx.h"
#include "EmrGraphingCtrl.h"
#include "EmrGraphSetupDlg.h"
#include <foreach.h>
#include <NxAlgorithm.h>
#include <boost/range/adaptor/uniqued.hpp>
#include "PicContainerDlg.h"
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <string>

enum DataTypes {
	dtText = 1,
	dtSingleSelect = 2,
	dtSlider = 5,
	dtTable = 7,
};

#define SELECT_ALL 0

// (a.walling 2012-07-12 10:34) - PLID 49684 - Basic point for a piece of data
struct DataPoint
{
	DataPoint()
		: dt(g_cdtNull)
		, val(0)
	{
	}

	DataPoint(const COleDateTime& dt, double val)
		: dt(dt)
		, val(val)
	{
	}

	COleDateTime dt;
	double val;

	__int64 DateVal() const
	{
		static COleDateTime epoch(__time64_t(0));
		COleDateTimeSpan dts = dt - epoch;
		return static_cast<__int64>(dts.GetTotalSeconds() * 1000);
	}

	bool operator<(const DataPoint& r) const
	{
		if (dt < r.dt) {
			return true;
		}
		if (dt > r.dt) {
			return false;
		}
		return val < r.val;
	}

	bool operator==(const DataPoint& r) const
	{
		return dt == r.dt && val == r.val;
	}
};

// (a.walling 2012-07-12 10:34) - PLID 49684 - Line detail within a graph; basically a source of the data
struct LineDetail
{
	LineDetail(long nLineID, long nItemID, long nEMRInfoMasterID, long nDataType, long nColumnID, long nRowID, long nDateID) 
		: nLineID(nLineID)
		, nItemID(nItemID)
		, nEMRInfoMasterID(nEMRInfoMasterID)
		, nDataType(nDataType)
		, nColumnID(nColumnID)
		, nRowID(nRowID)
		, nDateID(nDateID)
	{
	}

	long nLineID;
	long nItemID;
	long nEMRInfoMasterID;
	long nDataType;
	long nColumnID;
	long nRowID;
	long nDateID;

	//(a.wilson 2012-5-3) PLID 49704 - generate all the data necessary for the generation of data points.
	void GetData(long nPatientID, std::vector<DataPoint>& data)
	{
		//ensure the item is still a valid datatype before allowing data generation.
		if (nDataType != dtText && nDataType != dtSingleSelect && nDataType != dtSlider && nDataType != dtTable)
			return;

		CSqlFragment sqlFrag(
			//Declare constant values.
			"DECLARE @PatientID INT \r\n"
			"DECLARE @EMRInfoMasterID INT \r\n"
			"DECLARE @DataType INT \r\n"
			"\r\n"
			"SET @PatientID = {INT} \r\n"
			"SET @EMRInfoMasterID = {INT} \r\n"
			"SET @DataType = {INT} \r\n"
			"\r\n", nPatientID, nEMRInfoMasterID, nDataType);

		//Start setup.
		sqlFrag += CSqlFragment("SELECT * FROM (\r\n"
			"SELECT EMRInfoQ.EMRInfoID, EMRGroupID, \r\n"
			"Date AS EMNDate, DetailID, \r\n");

		if (nDataType == dtTable) { //needs these fields if its a table.
			// (j.jones 2013-03-12 13:27) - PLID 55591 - need X and Y values
			sqlFrag += CSqlFragment("DataID_X, DataID_Y, EMRDataGroupID_X, EMRDataGroupID_Y, ListType_Y AS ListType, \r\n");
		} else if (nDataType == dtSingleSelect) {
			sqlFrag += CSqlFragment("EMRDataQ.Data, \r\n");
		}

		sqlFrag += CSqlFragment("IsFlipped, Text, SliderValue FROM \r\n"
			"\r\n"
			"(SELECT ID AS MasterID, PatientID, EMRGroupID, TemplateID, Date \r\n"
			"FROM EMRMasterT WHERE PatientID = @PatientID AND Deleted = 0) EMRMasterQ \r\n"
			"\r\n"
			"INNER JOIN \r\n"
			"(SELECT ID AS DetailID, EMRID, EMRInfoID, SourceTemplateID, Text, SliderValue \r\n"
			"FROM EMRDetailsT WHERE Deleted = 0) EMRDetailsQ \r\n"
			"ON EMRMasterQ.MasterID = EMRDetailsQ.EMRID \r\n"
			"\r\n"
			"INNER JOIN \r\n"
			"(SELECT EMRInfoMasterID, ID AS EMRInfoID, Name, DataType, TableRowsAsFields AS IsFlipped \r\n"
			"FROM EMRInfoT WHERE EMRInfoMasterID = @EMRInfoMasterID AND DataType = @DataType) EMRInfoQ \r\n"
			"ON EMRInfoQ.EMRInfoID = EMRDetailsQ.EMRInfoID \r\n");
		//complete the query if its a text box or a slider.
		if (nDataType == dtText || nDataType == dtSlider) {
			sqlFrag += CSqlFragment(")\r\n SetupQ \r\n");
		//specific query to generate single select results.
		} else if (nDataType == dtSingleSelect) {
			sqlFrag += CSqlFragment(
				"\r\n"
				"INNER JOIN \r\n"
				"(SELECT EMRDetailID, EMRDataID FROM EMRSelectT) EMRSelectQ \r\n"
				"ON EMRSelectQ.EMRDetailID = EMRDetailsQ.DetailID \r\n"
				"\r\n"
				"INNER JOIN \r\n"
				"(SELECT ID AS DataID, Data FROM EMRDataT) EMRDataQ \r\n"
				"ON EMRDataQ.DataID = EMRSelectQ.EMRDataID) SetupQ\r\n");
		//specific query to generate table results.
		} else if (nDataType == dtTable) {
			// (j.jones 2013-03-12 13:27) - PLID 55591 - need X and Y values
			sqlFrag += CSqlFragment(
				"\r\n"
				"INNER JOIN \r\n"
				"(SELECT ID AS DataID_X, EMRInfoID AS EMRInfoID_X, Data AS Data_X, EMRDataGroupID AS EMRDataGroupID_X, ListType AS ListType_X FROM EMRDataT) EMRDataXQ \r\n"
				"ON EMRDataXQ.EMRInfoID_X = EMRInfoQ.EMRInfoID \r\n"
				"\r\n"
				"INNER JOIN \r\n"
				"(SELECT ID AS DataID_Y, EMRInfoID AS EMRInfoID_Y, Data AS Data_Y, EMRDataGroupID AS EMRDataGroupID_Y, ListType AS ListType_Y FROM EMRDataT) EMRDataYQ \r\n"
				"ON EMRDataYQ.EMRInfoID_Y = EMRInfoQ.EMRInfoID \r\n"
				")SetupQ\r\n");

			if (nColumnID == SELECT_ALL) {
				sqlFrag += CSqlFragment(
					"INNER JOIN (SELECT EMRDetailID, EMRDataID_X, EMRDataID_Y, Data AS [Value], ID FROM EMRDetailTableDataT) EMRDetailTableDataQ ON \r\n"
					"EMRDetailTableDataQ.EMRDataID_X = SetupQ.DataID_X AND EMRDetailTableDataQ.EMRDetailID = SetupQ.DetailID \r\n"
					"\r\n"
					//Include dropdown fields.
					"LEFT JOIN (SELECT ID, Data AS DropdownValue FROM EMRTableDropdownInfoT) EMRTableDropdownInfoQ ON \r\n"
					// (a.walling 2013-09-27 09:58) - PLID 58801 - EMR graphing causing full table scan of EMRTableDropdownInfoT; fixed predicate to not convert EmrTableDropdownInfoT.ID to NVARCHAR
					// (d.singleton 2014-02-18 15:03) - PLID 60823 - Getting errors trying to graph decimals, isnumeric retuns true for decimals while convert will blow up
					// if you add '.e0' to the end, anything that already had a decimal point now has two decimal points, causing IsNumeric to be false, 
					// and anything already expressed in scientific notation is invalidated by the e0
					"ISNUMERIC(EMRDetailTableDataQ.Value + '.e0') = 1 AND \r\n"
					"EMRTableDropdownInfoQ.ID = CASE ISNUMERIC(EMRDetailTableDataQ.Value + '.e0') WHEN 1 THEN CONVERT(INT, EMRDetailTableDataQ.Value) ELSE NULL END AND \r\n"
					"SetupQ.ListType = 4 \r\n"
					"\r\n"
					"LEFT JOIN (\r\n"
					"SELECT * FROM (\r\n" 
					"SELECT EMRInfoQ.EMRInfoID, EMRGroupID, DetailID, DataID, EMRDataGroupID FROM  \r\n"
					"(SELECT ID AS MasterID, PatientID, EMRGroupID, TemplateID, Date, Deleted FROM EMRMasterT WHERE PatientID = @PatientID AND Deleted = 0) EMRMasterQ  \r\n"
					"INNER JOIN  \r\n"
					"(SELECT ID AS DetailID, EMRID, EMRInfoID, SourceTemplateID, Deleted FROM EMRDetailsT WHERE Deleted = 0) EMRDetailsQ  \r\n"
					"ON EMRMasterQ.MasterID = EMRDetailsQ.EMRID \r\n"
					"INNER JOIN  \r\n"
					"(SELECT EMRInfoMasterID, ID AS EMRInfoID, Name, DataType, TableRowsAsFields AS IsFlipped FROM EMRInfoT WHERE EMRInfoMasterID = @EMRInfoMasterID) EMRInfoQ  \r\n"
					"ON EMRInfoQ.EMRInfoID = EMRDetailsQ.EMRInfoID \r\n"
					"INNER JOIN  \r\n"
					"(SELECT ID AS DataID, EMRInfoID, Data, EMRDataGroupID FROM EMRDataT) EMRDataQ  \r\n"
					"ON EMRDataQ.EMRInfoID = EMRInfoQ.EMRInfoID) SetupQ  \r\n"
					"INNER JOIN (SELECT EMRDetailID, EMRDataID_X, EMRDataID_Y, Data AS [Date], ID FROM EMRDetailTableDataT) EMRDetailTableDataQ ON  \r\n"
					"EMRDetailTableDataQ.EMRDataID_X = SetupQ.DataID AND EMRDetailTableDataQ.EMRDetailID = SetupQ.DetailID  \r\n"
					"WHERE EMRDataGroupID = {INT}) SubQ ON EMRDetailTableDataQ.EMRDataID_Y = SubQ.EMRDataID_Y AND  \r\n"
					"EMRDetailTableDataQ.EMRDataID_X <> SubQ.EMRDataID_X AND SetupQ.DetailID = SubQ.EMRDetailID \r\n"
					"\r\n"
					"WHERE SetupQ.EMRDataGroupID_X = {INT} ORDER BY SetupQ.DetailID ", nDateID, nRowID);
			}
			else if (nRowID == SELECT_ALL) {
				sqlFrag += CSqlFragment(
					"INNER JOIN (SELECT EMRDetailID, EMRDataID_X, EMRDataID_Y, Data AS [Value], ID FROM EMRDetailTableDataT) EMRDetailTableDataQ ON \r\n"
					"EMRDetailTableDataQ.EMRDataID_Y = SetupQ.DataID_Y AND EMRDetailTableDataQ.EMRDetailID = SetupQ.DetailID \r\n"
					"\r\n"
					//Include dropdown fields.
					"LEFT JOIN (SELECT ID, Data AS DropdownValue FROM EMRTableDropdownInfoT) EMRTableDropdownInfoQ ON \r\n"
					// (a.walling 2013-09-27 09:58) - PLID 58801 - EMR graphing causing full table scan of EMRTableDropdownInfoT; fixed predicate to not convert EmrTableDropdownInfoT.ID to NVARCHAR
					// (d.singleton 2014-02-18 15:03) - PLID 60823 - Getting errors trying to graph decimals, isnumeric retuns true for decimals while convert will blow up
					// if you add '.e0' to the end, anything that already had a decimal point now has two decimal points, causing IsNumeric to be false, 
					// and anything already expressed in scientific notation is invalidated by the e0
					"ISNUMERIC(EMRDetailTableDataQ.Value + '.e0') = 1 AND \r\n"
					"EMRTableDropdownInfoQ.ID = CASE ISNUMERIC(EMRDetailTableDataQ.Value + '.e0') WHEN 1 THEN CONVERT(INT, EMRDetailTableDataQ.Value) ELSE NULL END AND \r\n"
					"SetupQ.ListType = 4 \r\n"
					"\r\n"
					"LEFT JOIN (\r\n"
					"SELECT * FROM (\r\n" 
					"SELECT EMRInfoQ.EMRInfoID, EMRGroupID, DetailID, DataID, EMRDataGroupID FROM  \r\n"
					"(SELECT ID AS MasterID, PatientID, EMRGroupID, TemplateID, Date, Deleted FROM EMRMasterT WHERE PatientID = @PatientID AND Deleted = 0) EMRMasterQ  \r\n"
					"INNER JOIN  \r\n"
					"(SELECT ID AS DetailID, EMRID, EMRInfoID, SourceTemplateID, Deleted FROM EMRDetailsT WHERE Deleted = 0) EMRDetailsQ  \r\n"
					"ON EMRMasterQ.MasterID = EMRDetailsQ.EMRID  \r\n"
					"INNER JOIN  \r\n"
					"(SELECT EMRInfoMasterID, ID AS EMRInfoID, Name, DataType, TableRowsAsFields AS IsFlipped FROM EMRInfoT WHERE EMRInfoMasterID = @EMRInfoMasterID) EMRInfoQ  \r\n"
					"ON EMRInfoQ.EMRInfoID = EMRDetailsQ.EMRInfoID  \r\n"
					"INNER JOIN  \r\n"
					"(SELECT ID AS DataID, EMRInfoID, Data, EMRDataGroupID FROM EMRDataT) EMRDataQ  \r\n"
					"ON EMRDataQ.EMRInfoID = EMRInfoQ.EMRInfoID) SetupQ  \r\n"
					"INNER JOIN (SELECT EMRDetailID, EMRDataID_X, EMRDataID_Y, Data AS [Date], ID FROM EMRDetailTableDataT) EMRDetailTableDataQ ON  \r\n"
					"EMRDetailTableDataQ.EMRDataID_Y = SetupQ.DataID AND EMRDetailTableDataQ.EMRDetailID = SetupQ.DetailID  \r\n"
					"WHERE EMRDataGroupID = {INT}) SubQ ON EMRDetailTableDataQ.EMRDataID_X = SubQ.EMRDataID_X AND  \r\n"
					"EMRDetailTableDataQ.EMRDataID_Y <> SubQ.EMRDataID_Y AND SetupQ.DetailID = SubQ.EMRDetailID \r\n"
					"\r\n"
					"WHERE SetupQ.EMRDataGroupID_Y = {INT} ORDER BY SetupQ.DetailID ", nDateID, nColumnID);
			}
			else {
				// (j.jones 2013-03-12 12:58) - PLID 55591 - fixed this query to properly handle graphing just one cell,
				// as we should only be pulling the table data for the cell at X and Y, not all X and all Y
				sqlFrag += CSqlFragment(
					"INNER JOIN (SELECT EMRDetailID, EMRDataID_X, EMRDataID_Y, Data AS [Value], ID FROM EMRDetailTableDataT) EMRDetailTableDataQ ON \r\n"
					"EMRDetailTableDataQ.EMRDetailID = SetupQ.DetailID AND \r\n"
					"SetupQ.DataID_X = EMRDetailTableDataQ.EMRDataID_X AND SetupQ.DataID_Y = EMRDetailTableDataQ.EMRDataID_Y "
					"\r\n"
					//Include dropdowns
					"LEFT JOIN (SELECT ID, Data AS DropdownValue FROM EMRTableDropdownInfoT) EMRTableDropdownInfoQ ON \r\n"
					// (a.walling 2013-09-27 09:58) - PLID 58801 - EMR graphing causing full table scan of EMRTableDropdownInfoT; fixed predicate to not convert EmrTableDropdownInfoT.ID to NVARCHAR
					// (d.singleton 2014-02-18 15:03) - PLID 60823 - Getting errors trying to graph decimals, isnumeric retuns true for decimals while convert will blow up
					// if you add '.e0' to the end, anything that already had a decimal point now has two decimal points, causing IsNumeric to be false, 
					// and anything already expressed in scientific notation is invalidated by the e0
					"ISNUMERIC(EMRDetailTableDataQ.Value + '.e0') = 1 AND \r\n"
					"EMRTableDropdownInfoQ.ID = CASE ISNUMERIC(EMRDetailTableDataQ.Value + '.e0') WHEN 1 THEN CONVERT(INT, EMRDetailTableDataQ.Value) ELSE NULL END AND \r\n"
					"SetupQ.ListType = 4 \r\n"
					"\r\n"
					"WHERE SetupQ.EMRDataGroupID_X = {INT} AND SetupQ.EMRDataGroupID_Y = {INT} ORDER BY DetailID \r\n", nRowID, nColumnID);
			}
		}

		CString strSQL = sqlFrag.Flatten();
		// (a.walling 2013-09-27 09:58) - PLID 58801 - Use snapshot connection
		ADODB::_RecordsetPtr prs = CreateParamRecordset(GetRemoteDataSnapshot(), sqlFrag);
		
		for (; !prs->eof; prs->MoveNext()) {
			COleDateTime dtDate, dtFullDate;
			CString strValue, strField;
			double dValue = 0;
			
			if (nDataType == dtTable) {
				if (nDateID == 0) dtFullDate = AdoFldDateTime(prs, "EMNDate");
				else dtFullDate.ParseDateTime(AdoFldString(prs, "Date", ""));
			} else {
				dtFullDate = AdoFldDateTime(prs, "EMNDate");
			}
			//if (nDateID == 0) dtFullDate = AdoFldDateTime(prs, "EMNDate");
			//else dtFullDate.ParseDateTime(AdoFldString(prs, "Date", ""));
			//check for a valid date, if not valid then we skip this data point.
			if (dtFullDate.GetStatus() == COleDateTime::invalid) continue;
			dtDate.SetDateTime(dtFullDate.GetYear(), dtFullDate.GetMonth(), dtFullDate.GetDay(), 
				dtFullDate.GetHour(), dtFullDate.GetMinute(), dtFullDate.GetSecond());

			switch (nDataType) {
				case dtText:
					strField = "Text";
					break;
				case dtSlider:
					dValue = AdoFldDouble(prs, "SliderValue", 0.00);
					break;
				case dtSingleSelect:
					strField = "Data";
					break;
				case dtTable:
					//check if the listtype is a dropdown.
					if (AdoFldLong(prs, "ListType") == 4) strField = "DropdownValue";
					else strField = "Value";
					break;
				default:
					continue;
			}
			if (!strField.IsEmpty()) {
				strValue = AdoFldString(prs, strField, "");
				HRESULT nResult = VarR8FromStr(_bstr_t(strValue), LOCALE_USER_DEFAULT, 0, &dValue);

				if (!SUCCEEDED(nResult)) {
					nResult = VarR8FromStr(_bstr_t(strValue), LOCALE_NEUTRAL, 0, &dValue);
				}

				if (!SUCCEEDED(nResult)) {
					continue;
				}
			}			
			data.push_back(DataPoint(dtDate, dValue));
		}
	}
};

// (a.walling 2012-07-12 10:34) - PLID 49684 - Line within a graph
struct Line
{
	Line()
		: nID(-1)
	{
	}

	Line(long nID, const CString& strName, long nColor)
		: nID(nID), strName(strName), nColor(nColor)
	{
		this->strName.Replace("\n", " ");
		this->strName.Replace("\r", "");
	}

	long nID;
	CString strName;
	long nColor;

	std::vector<DataPoint> data;

	CString GetGraphDataJson()
	{
		CString str;
		str.Append(" { ");

		str.AppendFormat("label: \"%s\", ", ConvertToHTMLEmbeddable(strName));
		str.Append("data: [ ");

		foreach (const DataPoint& point, data | boost::adaptors::uniqued) {
			
			if (fabs(point.val - (int)point.val) == 0)
				str.AppendFormat("[%I64i, %.0f], ", point.DateVal(), point.val);
			else	
				str.AppendFormat("[%I64i, %.2f], ", point.DateVal(), point.val);

		}

		str.TrimRight(",");

		str.Append(" ] ");

		str.Append(" } ");

		return str;
	}
};

// (a.walling 2012-07-12 10:34) - PLID 49684 - Graph itself contains data and other properties
struct Graph
{
	Graph(long nID, const CString& strName, bool bArchived)
		: nID(nID)
		, strName(strName)
		, bArchived(bArchived)
	{
	}

	long nID;
	CString strName;
	bool bArchived;

	typedef std::map<long, Line> LineMap;
	typedef std::vector<LineDetail> LineDetails;
	
	LineMap m_lines;
	LineDetails m_lineDetails;
	//(a.wilson 5-1-2012) PLID 50075 - this generates the overall setup of the graphs, lines, and items within the setup.
	void LoadLines()
	{
		ADODB::_RecordsetPtr prs = CreateParamRecordset(
			"SELECT EMRGraphsT.ID AS GraphID, EMRGraphLinesT.ID AS LineID, EMRGraphLineDetailsT.ID AS ItemID, \r\n"
			"EMRGraphsT.Name AS GraphName, EMRGraphLinesT.Name AS LineName, EMRGraphLinesT.Color, DataTypeQ.DataType, \r\n"
			"EMRGraphLineDetailsT.EMRInfoMasterID, EMRGraphLineDetailsT.ColumnID, EMRGraphLineDetailsT.RowID, EMRGraphLineDetailsT.DateID \r\n"
			"FROM EMRGraphsT \r\n"
			"INNER JOIN EMRGraphLinesT ON EMRGraphsT.ID = EMRGraphLinesT.GraphID \r\n"
			"INNER JOIN EMRGraphLineDetailsT ON EMRGraphLinesT.ID = EMRGraphLineDetailsT.LineID \r\n"
			"INNER JOIN (SELECT EMRInfoMasterT.ID, CAST(DataType AS INT) AS DataType FROM EMRInfoMasterT INNER JOIN EMRInfoT \r\n"
			"ON EMRInfoMasterT.ActiveEMRInfoID = EMRInfoT.ID) DataTypeQ ON EMRGraphLineDetailsT.EMRInfoMasterID = DataTypeQ.ID \r\n"
			"WHERE EMRGraphsT.ID = {INT}", nID);

		//(a.wilson 5-1-2012) PLID 50080 - add the assignment of colors to the line.
		for (; !prs->eof; prs->MoveNext()) {
			long nLineID = AdoFldLong(prs, "LineID");
			if (!m_lines.count(nLineID)) {
				m_lines.insert(std::make_pair(AdoFldLong(prs, "LineID"), Line(AdoFldLong(prs, "LineID"), 
					AdoFldString(prs, "LineName"), AdoFldLong(prs, "Color"))));
			}

			//collect each item for the graph
			m_lineDetails.push_back(
				LineDetail(AdoFldLong(prs, "LineID"), AdoFldLong(prs, "ItemID"), AdoFldLong(prs, "EMRInfoMasterID"), 
				AdoFldLong(prs, "DataType"), AdoFldLong(prs, "ColumnID", 0), AdoFldLong(prs, "RowID", 0), AdoFldLong(prs, "DateID", 0))
			);
		}
	}

	bool IsDataEmpty()
	{
		foreach (Line& line, m_lines | boost::adaptors::map_values) {
			if (!line.data.empty()) {
				return false;
			}
		}

		return true;
	}

	void LoadPatientData(long nPatientID)
	{
		Clear();
		LoadLines();

		foreach (LineDetail& lineDetail, m_lineDetails) {
			lineDetail.GetData(nPatientID, m_lines[lineDetail.nLineID].data);
		}

		foreach (Line& line, m_lines | boost::adaptors::map_values) {
			boost::sort(line.data);
		}
	}

	void Clear()
	{
		m_lineDetails.clear();
		m_lines.clear();
	}

	CString GetGraphDataJson()
	{
		CString strGraphData;

		strGraphData.Append(" [ ");

		long nValid = 0;
		foreach (Line& line, m_lines | boost::adaptors::map_values) {
			if (line.data.empty()) {
				continue;
			}
			strGraphData.Append(line.GetGraphDataJson());
			strGraphData.Append(", ");
		}

		strGraphData.TrimRight(", ");

		strGraphData.Append(" ] ");

		return strGraphData;
	}
	//(a.wilson 5-1-2012) PLID 50080 - get all the lines colors to display in the graph.
	CString GetGraphLineColors()
	{
		CString strLineColors;

		foreach (Line& line, m_lines | boost::adaptors::map_values) {
			if (line.data.empty()) {
				continue;
			}
			COLORREF rgb;
			OleTranslateColor(line.nColor, NULL, &rgb);
			//convert the colors into format that javascript can use.
			CString strColor = FormatString("\"#%02x%02x%02x\", ", GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));
			strLineColors.Append(strColor);

		}
		strLineColors.TrimRight(", ");

		return strLineColors; 
	}
};

// (a.walling 2012-07-12 10:34) - PLID 49684 - Graph info contains all graphs and parameters
struct GraphInfo
{
	GraphInfo()
	{
	}

	std::vector<shared_ptr<Graph>> m_graphs;

	std::vector<shared_ptr<Graph>>& Graphs()
	{
		return m_graphs;
	}

	shared_ptr<Graph> GetGraph(long nGraphID)
	{
		foreach (shared_ptr<Graph> pGraph, m_graphs) {
			if (nGraphID == pGraph->nID) {
				return pGraph;
			}
		}

		return shared_ptr<Graph>();
	}

	void Load()
	{
		try {
			ADODB::_RecordsetPtr prs = CreateRecordsetStd(
				"SELECT ID, Name, Archived FROM EmrGraphsT WHERE Archived = 0 ORDER BY Name"
			);

			for(; !prs->eof; prs->MoveNext()) {
				m_graphs.push_back(make_shared<Graph>(AdoFldLong(prs, "ID"), AdoFldString(prs, "Name"), !!AdoFldBool(prs, "Archived")));
			}
		} NxCatchAllThread(__FUNCTION__);
	}
};


//////

// (a.walling 2012-07-12 10:34) - PLID 49684 - Graph data object, handles callbacks with IDispatch when data is ready
class CEmrGraphData : public CCmdTarget
{
	DECLARE_DISPATCH_MAP();
public:
	CEmrGraphData(shared_ptr<Graph> pGraph, long nPatientID) 
		: m_pGraph(pGraph)
		, m_nPatientID(nPatientID)
	{
		EnableAutomation();
	}

	virtual ~CEmrGraphData()
	{
	}

protected:

	long m_nPatientID;

	shared_ptr<Graph> m_pGraph;

	// (a.walling 2012-04-26 09:12) - PLID 49998 - wrapper for handling script callbacks
	CNxScriptCallback m_callbackWhenDataReady;
	CNxScriptCallback m_callbackWhenDataEmpty;

	void ResetCallbacks()
	{
		m_callbackWhenDataReady.Clear();
		m_callbackWhenDataEmpty.Clear();
	}

	void InternalDataReady()
	{
		CNxScriptCallback callback;

		if (m_pGraph->IsDataEmpty()) {
			callback = m_callbackWhenDataEmpty;
		} else {
			callback = m_callbackWhenDataReady;
		}

		ResetCallbacks();

		callback.Invoke(this);
	}

	BSTR GetGraphDataJson()
	{
		CString str = m_pGraph->GetGraphDataJson();
		return str.AllocSysString();
	}
	//(a.wilson 5-1-2012) PLID 50080 - add the assignment of the line colors.
	BSTR GetGraphOptionsJson()
	{
		CString str;
		str.Format(
			"{ " 
				"series: { "
					"lines: { show: true }, "
					"points: { show: true, radius: 3 } "
				"}, "
				"colors: [%s], "
				"grid: { hoverable: true }, "
				"yaxis: { mode: null, tickDecimals: 0 }, "
				"xaxis: { mode: \"time\", timeformat: \"%%m/%%d/%%y\"} "
			"}", m_pGraph->GetGraphLineColors());

		return str.AllocSysString();
	}

	void Load()
	{
		try {
			m_pGraph->LoadPatientData(m_nPatientID);
		} NxCatchAllThread(__FUNCTION__);
		InternalDataReady();
	}
};

BEGIN_DISPATCH_MAP(CEmrGraphData, CCmdTarget)
	//{{AFX_DISPATCH_MAP(CEmrGraphData)
	DISP_FUNCTION(CEmrGraphData, "Load", Load, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(CEmrGraphData, "GetGraphDataJson", GetGraphDataJson, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CEmrGraphData, "GetGraphOptionsJson", GetGraphOptionsJson, VT_BSTR, VTS_NONE)
	// (a.walling 2012-04-26 09:12) - PLID 49998 - macros for handling script callbacks
	DISP_CALLBACK(CEmrGraphData, "WhenDataReady", m_callbackWhenDataReady)
	DISP_CALLBACK(CEmrGraphData, "WhenDataEmpty", m_callbackWhenDataEmpty)
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

/////////////////



struct GraphPoint {
	long nLineID;
	COleDateTime dtDate;
	long nValue;

	GraphPoint() { }
	GraphPoint(long _nLineID, COleDateTime _dtDate, long _nValue) {
		nLineID = _nLineID;
		dtDate = _dtDate;
		nValue = _nValue;
	}
};



// (a.walling 2012-04-12 14:42) - PLID 49657 - EMR Graphing control

CEmrGraphingCtrl::CEmrGraphingCtrl()
{
	EnableAutomation();
}

CEmrGraphingCtrl::~CEmrGraphingCtrl()
{
}

// (a.walling 2012-05-08 16:04) - PLID 50105
BEGIN_MESSAGE_MAP(CEmrGraphingCtrl, CNxHtmlControl)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_FILE_PRINT, &CEmrGraphingCtrl::OnPrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CEmrGraphingCtrl::OnPrintPreview)
END_MESSAGE_MAP()

void CEmrGraphingCtrl::OnInitializing()
{
	//CNxHtmlControl::OnInitializing(); // navigates to about:blank

	// (a.walling 2012-04-25 17:49) - PLID 49996 - Now we can just use an nxres URL eg nxres://0/[type/]resource
	LoadUrl("nxres://0/EmrGraphing.html");
}

// (a.walling 2012-07-12 10:34) - PLID 49684 - Callbacks and etc for interaction with HTML
BEGIN_DISPATCH_MAP(CEmrGraphingCtrl, CNxHtmlControl)
	//{{AFX_DISPATCH_MAP(CEmrGraphingCtrl)
	DISP_FUNCTION(CEmrGraphingCtrl, "Configure", Configure, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(CEmrGraphingCtrl, "PrintPreview", OnPrintPreview, VT_EMPTY, VTS_NONE) // (a.walling 2012-05-08 16:04) - PLID 50105 - Printing
	DISP_FUNCTION(CEmrGraphingCtrl, "GetAvailableGraphsJson", GetAvailableGraphsJson, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CEmrGraphingCtrl, "LoadGraphData", LoadGraphData, VT_VARIANT, VTS_I4)
	DISP_FUNCTION(CEmrGraphingCtrl, "UpdateVisibleGraphs", UpdateVisibleGraphs, VT_EMPTY, VTS_BSTR)
	DISP_FUNCTION(CEmrGraphingCtrl, "UpdateHiddenGraphs", UpdateHiddenGraphs, VT_EMPTY, VTS_BSTR)
	DISP_FUNCTION(CEmrGraphingCtrl, "UpdateGraphSortOrder", UpdateGraphSortOrder, VT_EMPTY, VTS_BSTR)
	DISP_FUNCTION(CEmrGraphingCtrl, "GetVisibleGraphs", GetVisibleGraphs, VT_BSTR, VTS_NONE)
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// (a.wilson 2012-4-30) PLID 49479 - create the emr graphing setup dialog.
void CEmrGraphingCtrl::Configure()
{
	try {
		CEmrGraphSetupDlg dlg(GetTopLevelFrame());
		dlg.DoModal();

		InvalidateGraphInfo();
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-05-08 16:04) - PLID 50105 - Suppress the context menu and send our usual WM_CONTEXTMENU message
HRESULT CEmrGraphingCtrl::OnShowContextMenu(DWORD dwID, LPPOINT ppt,
	LPUNKNOWN pcmdtReserved, LPDISPATCH pdispReserved)
{
	static POINT undefinedPt = {-1, -1};
	if (!ppt) {
		ppt = &undefinedPt;
	}

	SendMessage(WM_CONTEXTMENU, (WPARAM)GetSafeHwnd(), MAKELPARAM(ppt->x, ppt->y));

	return S_OK;
}

// (a.walling 2012-05-08 16:04) - PLID 50105 - Context menu
void CEmrGraphingCtrl::OnContextMenu(CWnd* pWnd, CPoint pos)
{
	CNxMenu mnu;
	mnu.CreatePopupMenu();

	mnu.AppendMenu(MF_BYPOSITION, ID_FILE_PRINT, "&Print...");
	mnu.AppendMenu(MF_BYPOSITION, ID_FILE_PRINT_PREVIEW, "Print Previe&w...");

	mnu.ShowPopupMenu(pos, this, TRUE);
}

// (a.walling 2012-05-08 16:04) - PLID 50105 - Print handling
void CEmrGraphingCtrl::OnPrint()
{
	DoPrint(false);
}

void CEmrGraphingCtrl::OnPrintPreview()
{
	DoPrint(true);
}

// (a.walling 2012-05-08 16:04) - PLID 50105 - Print handling - Prepare the print template and open up the preview or print direct
void CEmrGraphingCtrl::DoPrint(bool bPreview)
{
	try {
		NxPrintTemplate::DocInfo printInfo;

		printInfo
			.SetHeaderText("NexTech EMR Graphing")
			.SetFooterInnerHTML("Page <SPAN class=hfPage>{{NXPAGENUM}}</SPAN> of <SPAN class=hfPageTotal>Y</SPAN>");

		CString strPrintTemplate = printInfo.PreparePrintTemplate();
		_variant_t varTemplate = _bstr_t(strPrintTemplate);

		DWORD dwCmdID = bPreview ? IDM_PRINTPREVIEW : IDM_PRINT;

		GetOleCommandTarget()->Exec(&CGID_MSHTML, 
			dwCmdID, 
			OLECMDEXECOPT_PROMPTUSER, 
			strPrintTemplate.IsEmpty() ? NULL : &varTemplate,
			NULL);

	} NxCatchAll(__FUNCTION__);
}

void CEmrGraphingCtrl::InvalidateGraphInfo()
{
	m_pGraphInfo.reset();
}

// (a.walling 2012-07-12 10:34) - PLID 49684 - Load the graph info & parameters if necessary
shared_ptr<GraphInfo> CEmrGraphingCtrl::GetGraphInfo()
{
	if (!m_pGraphInfo) {
		m_pGraphInfo = make_shared<GraphInfo>();
		m_pGraphInfo->Load();
	}

	return m_pGraphInfo;
}

// (a.walling 2012-05-01 09:41) - PLID 50090 - Return the available graphs
BSTR CEmrGraphingCtrl::GetAvailableGraphsJson()
{
	shared_ptr<GraphInfo> pGraphInfo = GetGraphInfo();

	CString strGraphs;

	// (a.walling 2012-05-01 09:41) - PLID 50090 - Just use tokenizer for easier parsing of csv lists
	typedef boost::tokenizer<boost::escaped_list_separator<char>> listTokens;
	using std::string;

	// (a.walling 2012-05-02 12:17) - PLID 50156 - Gather all hidden graphs
	std::set<long> hiddenGraphs;
	{
		string strHiddenList = GetRemotePropertyText("EMRGraphing_HiddenGraphs", "", 0, "<None>");

		foreach (string strID, listTokens(strHiddenList)) {
			int nID = atol(strID.c_str() + 1);

			shared_ptr<Graph> pGraph = pGraphInfo->GetGraph(nID);
			if (pGraph) {
				hiddenGraphs.insert(nID);
			}
		}
	}
	
	strGraphs.Preallocate(pGraphInfo->Graphs().size() * 128);
	
	strGraphs.AppendChar('[');

	std::vector<shared_ptr<Graph>> orderedGraphs;
	
	// (a.walling 2012-05-02 18:12) - PLID 50157 - Populate orderedGraphs in order according to the sort order, then fill anything we missed at the end
	{
		orderedGraphs.reserve(pGraphInfo->Graphs().size());
		std::set<long> existingGraphs;

		string strOrderList = GetRemotePropertyText("EMRGraphing_SortOrder", "", 0, "<None>");

		foreach (string strID, listTokens(strOrderList)) {
			int nID = atol(strID.c_str() + 1);

			shared_ptr<Graph> pGraph = pGraphInfo->GetGraph(nID);
			if (pGraph) {
				orderedGraphs.push_back(pGraph);
				existingGraphs.insert(nID);
			}
		}

		foreach (shared_ptr<Graph>& pGraph, pGraphInfo->Graphs()) {
			if (!existingGraphs.count(pGraph->nID)) {
				orderedGraphs.push_back(pGraph);
				existingGraphs.insert(pGraph->nID);
			}
		}
	}

	ASSERT(orderedGraphs.size() == pGraphInfo->Graphs().size());

	// (a.walling 2012-05-01 09:41) - PLID 50090 - Creating a json document to describe the id and name of the graph
	foreach (shared_ptr<Graph>& pGraph, orderedGraphs) {
		strGraphs.AppendFormat(
			"{ "
				"id: %li, "
				"name: %s, "
				"isHidden: %s " // (a.walling 2012-05-02 12:17) - PLID 50156 - Include hidden state
			"},\r\n"
			, pGraph->nID
			, js::Stringify(pGraph->strName)
			, js::Boolify(!!hiddenGraphs.count(pGraph->nID)) 
		);
	}
	
	strGraphs.TrimRight(",\r\n");
	
	strGraphs.AppendChar(']');

	return strGraphs.AllocSysString();
}

// (a.walling 2012-05-01 09:41) - PLID 50090 - Notified when the list of visible graphs changes
void CEmrGraphingCtrl::UpdateVisibleGraphs(LPCTSTR szGraphIDs)
{
	try {
		SetRemotePropertyText("EMRGraphing_VisibleGraphs", szGraphIDs, 0, "<None>");
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-05-02 12:17) - PLID 50156 - Now we keep track of hidden / closed graphs as compared to collapsed graphs
void CEmrGraphingCtrl::UpdateHiddenGraphs(LPCTSTR szGraphIDs)
{
	try {
		SetRemotePropertyText("EMRGraphing_HiddenGraphs", szGraphIDs, 0, "<None>");
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-05-02 18:12) - PLID 50157 - Now we also keep track of the sort order
void CEmrGraphingCtrl::UpdateGraphSortOrder(LPCTSTR szGraphIDs)
{
	try {
		SetRemotePropertyText("EMRGraphing_SortOrder", szGraphIDs, 0, "<None>");
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-05-01 09:41) - PLID 50090 - Return the list of visible graphs
BSTR CEmrGraphingCtrl::GetVisibleGraphs()
{
	CString str;

	try {
		str = GetRemotePropertyText("EMRGraphing_VisibleGraphs", "", 0, "<None>");
	} NxCatchAll(__FUNCTION__);

	return str.AllocSysString();
}

// (a.walling 2012-07-12 10:34) - PLID 49684 - Load the graph data
VARIANT CEmrGraphingCtrl::LoadGraphData(long nGraphID)
{
	shared_ptr<Graph> pGraph = GetGraphInfo()->GetGraph(nGraphID);

	CPicContainerDlg* pPic = dynamic_cast<CPicContainerDlg*>(GetTopLevelFrame());

	long nPatientID = pPic ? pPic->GetPatientID() : -1;

	if (!pGraph || -1 == nPatientID) {
		return g_cvarNull;
	}

	CEmrGraphData* pGraphData = new CEmrGraphData(pGraph, nPatientID);

	_variant_t varGraphData(pGraphData->GetIDispatch(FALSE), false);

	return varGraphData.Detach();
}

