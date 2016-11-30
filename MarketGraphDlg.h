#include "marketutils.h"
#include "MultiSelectDlg.h"
#if !defined(AFX_CMarketGraphDlg_H__3EE09E74_7337_11D2_B386_00001B4B970B__INCLUDED_)
#define AFX_CMarketGraphDlg_H__3EE09E74_7337_11D2_B386_00001B4B970B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MarketGraphDlg.h : header file
//
// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace NxTab;
#import "stdole2.tlb" rename_namespace("ColumnGraph") exclude("GUID", "DISPPARAMS", "EXCEPINFO", \
	"OLE_XPOS_PIXELS", "OLE_YPOS_PIXELS", "OLE_XSIZE_PIXELS", "OLE_YSIZE_PIXELS", "OLE_XPOS_HIMETRIC", \
	"OLE_YPOS_HIMETRIC", "OLE_XSIZE_HIMETRIC", "OLE_YSIZE_HIMETRIC", "OLE_XPOS_CONTAINER", \
	"OLE_YPOS_CONTAINER", "OLE_XSIZE_CONTAINER", "OLE_YSIZE_CONTAINER", "OLE_HANDLE", "OLE_OPTEXCLUSIVE", \
	"OLE_CANCELBOOL", "OLE_ENABLEDEFAULTBOOL", "FONTSIZE", "OLE_COLOR", \
	"IUnknown", "IDispatch", "IEnumVARIANT", "IFont", "IPicture")

#import "nxcolumngraph.tlb" rename_namespace("ColumnGraph")

// using namespace ColumnGraph;
// using namespace ADODB;
/////////////////////////////////////////////////////////////////////////////
// CMarketGraphDlg dialog

/*class GraphDescript
{
	public:
		typedef enum
		{
			GD_ADD,
			GD_AVG,
			GD_DIV
		} GD_OP;

		GraphDescript (const LPCSTR sql = NULL)
		{
			m_sql = sql;
		}

		void Add (	const LPCSTR label, 
					const LPCSTR field, 
					unsigned long color, 
					const LPCSTR field2 = "", 
					long operation = GD_ADD,
					const double dblTotal = 0,
					const double dblTotal2 = 0)

		{	m_labels.Add(label);
			m_fields.Add(field);
			m_fields2.Add(field2);
			m_colors.Add(color);
			m_ops.Add(operation);
			m_dblTotals.Add(dblTotal);
			m_dblTotals2.Add(dblTotal2);

		}

		unsigned short Size() const
		{
			//aribitrary, they should all be the same
			return m_colors.GetSize();
		}

		unsigned int Color(unsigned int index) const
		{
			ASSERT (index < Size());
			return m_colors[index];
		}

		CString Label(unsigned int index) const
		{
			ASSERT (index < Size());
			return m_labels[index];
		}

		CString Field(unsigned int index) const
		{
			ASSERT (index < Size());
			return m_fields[index];
		}

		CString Field2(unsigned int index) const
		{
			if (index > Size()) {
				CString str;
				str.Format("Index: %li, Size: %li", index, Size());
				AfxMessageBox(str);
			}
			return m_fields2[index];
		}
		double dblTotal(unsigned int index) 
		{
			ASSERT(index < Size());
			return m_dblTotals[index];
		}
		double dblTotal2(unsigned int index) 
		{
			ASSERT(index < Size());
			return m_dblTotals2[index];
		}
		void AddToTotal(unsigned int index, double dblValToAdd) {
			ASSERT(index < Size());
			m_dblTotals[index] += dblValToAdd;
		}
		void AddToTotal2(unsigned int index, double dblValToAdd) {
			ASSERT(index < Size());
			m_dblTotals2[index] += dblValToAdd;
		}

		GD_OP Op(unsigned int index) const
		{
			ASSERT (index < Size());
			return (GD_OP)m_ops[index];
		}

	CString		 m_sql;

	protected:
		CStringArray m_labels;
		CStringArray m_fields;
		CStringArray m_fields2;
		CArray<unsigned int, unsigned int> m_ops;
		CArray<unsigned int, unsigned int> m_colors;
		CArray<double, double> m_dblTotals;
		CArray<double, double> m_dblTotals2;
};*/




/////////////////////////////////////////////////////////////////
// (b.cardillo 2004-08-05 18:19) - PLID 13747 - Various utility classes used by 
// several of the marketing tabs' graphs for high-performance functionality.
/////////////////////////////////////////////////////////////////

class GraphDescript; // forward-declaration

class CGraphValues
{
public:
	CGraphValues() : m_dblFieldValue(0.0), m_dblField2Value(0.0) { };

public:
	double m_dblFieldValue;
	double m_dblField2Value;
};

class CMapLongToGraphValuesArray : public CMap<long,long,CGraphValues *,CGraphValues *>
{
public:
	CMapLongToGraphValuesArray();
	~CMapLongToGraphValuesArray();

public:
	// Various handy ways of calling ScanRecordsetIntoMap()
	void ScanRecordsetIntoMap(IN ADODB::_RecordsetPtr rs, IN const CString &strIDFieldName, IN const GraphDescript &desc);
	void ScanRecordsetIntoMap(IN ADODB::_RecordsetPtr rs, IN const CString &strIDFieldName, IN long nIDDefault, IN const GraphDescript &desc);

protected:
	// Takes a recordset, the recordset's id field name, and a graph descriptor, and adds all the 
	// records' desc.Field and desc.Field2 values into the appropriate spot in an array of desc.Size() 
	// CGraphValues objects based on the strIDFieldName ID for each record.
	void ScanRecordsetIntoMap(IN ADODB::_RecordsetPtr rs, IN const CString &strIDFieldName, IN BOOL bUseDefaultIDForNULL, IN long nIDDefault, IN const GraphDescript &desc);
};

class CED_GraphCalcColumnTotals_Info
{
public:
	CED_GraphCalcColumnTotals_Info(ADODB::_RecordsetPtr prs, const CString &strIDFieldName, const GraphDescript &gdDescript, long nDescriptIndex, double dblTotal, double dblTotal2);
	CED_GraphCalcColumnTotals_Info(CMapLongToGraphValuesArray *pmapp, const GraphDescript &gdDescript, long nDescriptIndex, double dblTotal, double dblTotal2);

public: // IN member variables
	ADODB::_RecordsetPtr m_rs;
	CString m_strIDFieldName; // Only required if m_rs is used.
	CMapLongToGraphValuesArray *m_pmapp;
	const GraphDescript &m_gdDescript;
	long m_nDescriptIndex;

public: // IN/OUT member variables
	double m_dblTotal;
	double m_dblTotal2;

public:
	BOOL ProcessGraphValues(long nID);

public:
	static BOOL CALLBACK CallbackProcessGraphValues(long nID, LPVOID pParam);
};

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////




// (c.haag 2008-05-15 13:06) - PLID 30068 - This dialog has been deprecated
/*
class CMarketGraphDlg : public CNxDialog
{
// Construction
public: 
	CMarketGraphDlg(CWnd* pParent);   // standard constructor
	virtual ~CMarketGraphDlg();

	virtual void UpdateView();
	NXDATALISTLib::_DNxDataListPtr  m_pApptPurposeList;
	CString m_strPurposeList;
	NXDATALISTLib::_DNxDataListPtr  m_pYearFilter;
	void RefreshConversionDateGraph();
	void SelectMultiPurposes();
	void CheckDataList(CMultiSelectDlg *dlg);
	void Refresh();


// Dialog Data
	//{{AFX_DATA(CMarketGraphDlg)
	enum { IDD = IDD_MARKET_REFERRAL };
	CNxIconButton		m_up;
	CProgressCtrl		m_progress;
	ColumnGraph::_DNxColumnGraphPtr	m_graph;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMarketGraphDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void	Graph(GraphDescript &desc);
	void	GetParameters(CString &from, CString &to, int &prov, int &loc, int &id);
// Added by Bob.  Brad, what the heck are you checking this kinda crap into my source safe for?  :)
// Added by Brad, Bob, its not done yet
///////////////////////////////////////////////////////////////////////////////////////////////////
	int		MaxReferrals();
	void	Up();
	int		GetCurrentID();
	void	SetChart(int columns);

	bool				initialized;
	CString				m_referral;
	CArray<long, long>	m_arClickableRows;
	int					GetRow();
	HCURSOR				m_cursor;
	HCURSOR				m_oldCursor;

	NxButton			m_numberRad;
	NxButton			m_conversionRad;
	NxButton			m_costRad;
	NxButton			m_revenueRad;
	NxButton			m_ConvDateRad;
	NxButton			m_ShowAllRad;
	NxButton			m_ShowNumberRad;
	NxButton			m_ShowPercentRad;


	// Generated message map functions
	//{{AFX_MSG(CMarketGraphDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeFrom();
	afx_msg void OnChangeTo();
	afx_msg void OnCloseUpFrom();
	afx_msg void OnCloseUpTo();
	afx_msg void OnUp();
	afx_msg void OnMouseMoveReferralChart(short Button, short Shift, long X, long Y);
	afx_msg void OnClickColumnGraph(short Row, short Column);
	afx_msg void OnMouseMoveColumnGraph(short Row, short Column);
	afx_msg void OnConversionRad();
	afx_msg void OnCostsRad();
	afx_msg void OnRevenueRad();
	afx_msg void OnNumberRad();
	afx_msg void OnCompletedOnly();
	afx_msg void OnConversionDateRad();
	afx_msg void OnConfigureApptTypes();
	afx_msg void OnSelChosenApptPurposeList(long nRow);
	afx_msg void OnSelChosenYearFilter(long nRow);
	afx_msg void OnShowAllColumns();
	afx_msg void OnShowNumbersOnly();
	afx_msg void OnShowPercentagesOnly();
	
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// Generated OLE dispatch map functions
	//{{AFX_DISPATCH(CMarketGraphDlg)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
};*/

////{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CMarketGraphDlg_H__3EE09E74_7337_11D2_B386_00001B4B970B__INCLUDED_)
