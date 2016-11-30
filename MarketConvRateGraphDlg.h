#include "marketutils.h"
#include "MultiSelectDlg.h"
#include "GraphDescript.h"
#if !defined(AFX_CMarketConvRateGraphDlg_H__3EE09E74_7337_11D2_B386_00001B4B970B__INCLUDED_)
#define AFX_CMarketConvRateGraphDlg_H__3EE09E74_7337_11D2_B386_00001B4B970B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MarketConvRateGraphDlg.h : header file
//


// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace NxTab;
//(e.lally 2009-09-11) This needs moved into a better location. Maybe even cleanup the namespace naming better
#import "stdole2.tlb" rename_namespace("ColumnGraph") exclude("GUID", "DISPPARAMS", "EXCEPINFO", \
	"OLE_XPOS_PIXELS", "OLE_YPOS_PIXELS", "OLE_XSIZE_PIXELS", "OLE_YSIZE_PIXELS", "OLE_XPOS_HIMETRIC", \
	"OLE_YPOS_HIMETRIC", "OLE_XSIZE_HIMETRIC", "OLE_YSIZE_HIMETRIC", "OLE_XPOS_CONTAINER", \
	"OLE_YPOS_CONTAINER", "OLE_XSIZE_CONTAINER", "OLE_YSIZE_CONTAINER", "OLE_HANDLE", "OLE_OPTEXCLUSIVE", \
	"OLE_CANCELBOOL", "OLE_ENABLEDEFAULTBOOL", "FONTSIZE", "OLE_COLOR", \
	"IUnknown", "IDispatch", "IEnumVARIANT", "IFont", "IPicture")

#import "nxcolumngraph.tlb" rename_namespace("ColumnGraph")

/////////////////////////////////////////////////////////////////////////////
// CMarketConvRateGraphDlg dialog

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
					long operation = GD_ADD)

		{	m_labels.Add(label);
			m_fields.Add(field);
			m_fields2.Add(field2);
			m_colors.Add(color);
			m_ops.Add(operation);
		}

		unsigned short Size()
		{
			//aribitrary, they should all be the same
			return m_colors.GetSize();
		}

		unsigned int Color(unsigned int index)
		{
			ASSERT (index < Size());
			return m_colors[index];
		}

		CString Label(unsigned int index)
		{
			ASSERT (index < Size());
			return m_labels[index];
		}

		CString Field(unsigned int index)
		{
			ASSERT (index < Size());
			return m_fields[index];
		}

		CString Field2(unsigned int index)
		{
			ASSERT (index < Size());
			return m_fields2[index];
		}

		GD_OP Op(unsigned int index)
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
};*/

// (a.walling 2008-05-28 14:01) - PLID 27591 - Use CDateTimePicker
//
//
//(e.lally 2009-09-11) - Dead code
//
//
class CMarketConvRateGraphDlg : public CNxDialog
{
// Construction
public: 
	CMarketConvRateGraphDlg(CWnd* pParent);   // standard constructor
	virtual ~CMarketConvRateGraphDlg();
/*
	virtual void UpdateView();
	NXDATALISTLib::_DNxDataListPtr  m_pApptPurposeList;
	CString m_strPurposeList;
	NXDATALISTLib::_DNxDataListPtr  m_pYearFilter;
	void RefreshConversionDateGraph();
	void SelectMultiPurposes();
	void CheckDataList(CMultiSelectDlg *dlg);
	void Refresh();
	void GraphPatientCoordinators(GraphDescript &desc);
	void GraphProcedures(GraphDescript &desc);

	NXDATALISTLib::_DNxDataListPtr  m_pLocationsList;
	NXDATALISTLib::_DNxDataListPtr  m_pProvidersList;
	NXDATALISTLib::_DNxDataListPtr  m_pDateOptionsList;
	NXDATALISTLib::_DNxDataListPtr  m_pPatCoordList;
	CBrush m_brush;


// Dialog Data
	//{{AFX_DATA(CMarketConvRateGraphDlg)
	enum { IDD = IDD_MARKET_CONVERSION_RATE };
	CNxIconButton		m_up;
	CProgressCtrl		m_progress;
	ColumnGraph::_DNxColumnGraphPtr	m_graph;
	CDateTimePicker	m_ToDate;
	CDateTimePicker	m_FromDate;
	CNxStatic	m_nxstatic1;
	CNxStatic	m_nxstatic2;
	CNxStatic	m_nxstatic3;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMarketConvRateGraphDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void	Graph(GraphDescript &desc);
	void	GetParameters(CString &from, CString &to, int &prov, int &loc, int &id, int &nPatCoord, CString &strDateField);
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

	NxButton			m_conversionRad;
	NxButton			m_ConvDateRad;
	NxButton			m_InqToConsByProc;
	NxButton			m_ProsToConsByProc;
	NxButton			m_ProsToSurgByProc;
	NxButton			m_InqToConsByStaff;
	NxButton			m_ConsToSurgByPatCoord;
	NxButton			m_ShowAllRad;
	NxButton			m_ShowNumberRad;
	NxButton			m_ShowPercentRad;
	NxButton			m_ShowAllProspects;
	NxButton			m_ShowProsConsOnly;
	//NxButton			m_InqToConsByStaff2;


	// Generated message map functions
	//{{AFX_MSG(CMarketConvRateGraphDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeFrom(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeTo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCloseUpFrom(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCloseUpTo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnUp();
	afx_msg void OnClickColumnGraph(short Row, short Column);
	afx_msg void OnMouseMoveColumnGraph(short Row, short Column);
	afx_msg void OnConversionRad();
	afx_msg void OnCompletedOnly();
	afx_msg void OnConversionDateRad();
	afx_msg void OnConfigureApptTypes();
	afx_msg void OnSelChosenApptPurposeList(long nRow);
	afx_msg void OnSelChosenYearFilter(long nRow);
	afx_msg void OnShowAllColumns();
	afx_msg void OnShowNumbersOnly();
	afx_msg void OnShowPercentagesOnly();
	afx_msg void OnConsToSurgByPatCoord();
	afx_msg void OnInqToConsByProc();
	afx_msg void OnInqToConsByStaff();
	afx_msg void OnProsToConsByProc();
	afx_msg void OnProsToSurgByProc();
	afx_msg void OnShowConsProspects();
	afx_msg void OnShowAllProspects();
	afx_msg void OnSelChosenDateOptionList(long nRow);
	afx_msg void OnChangeFilteredFromDate();
	afx_msg void OnSelChosenFilteredLocationList(long nRow);
	afx_msg void OnSelChosenFilteredProviderList(long nRow);
	afx_msg void OnChangeFilteredToDate();
	afx_msg void OnSelChosenPatCoordList(long nRow);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//afx_msg void OnInqToConsByStaff2();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// Generated OLE dispatch map functions
	//{{AFX_DISPATCH(CMarketConvRateGraphDlg)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	*/
};

////{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CMarketConvRateGraphDlg_H__3EE09E74_7337_11D2_B386_00001B4B970B__INCLUDED_)
