#if !defined(AFX_EYEGRAPHCTRL_H__DF82A9C9_107B_4D13_8A6D_56A6A21E241A__INCLUDED_)
#define AFX_EYEGRAPHCTRL_H__DF82A9C9_107B_4D13_8A6D_56A6A21E241A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EyeGraphCtrl.h : header file
//


//***********************************************************
//*                                                         *
//*  OK, here's the deal, we've got 1000x1000 to work with  *
//*  50 pixel margin around everything.                     *
//*  The main title goes from 900-950 vert, and 50-950 horiz*
//*  The x-axis caption goes from 840-890 vert, and 50-950  *
//*  The scaling for the x-axis goes from 780-830 vert, and *
//*  horiz as needed.                                       *
//*  The caption for the y-axis goes from 50-950 vert, and  *
//*  50-100 horiz.                                          *
//*  The scaling for the y-axis goes from 110-160 horiz, and*
//*  vert as needed.                                        *
//*  The y-axis itself goes from 50-770 vert, at 170 horiz, *
//*  with 10 unit wide hash marks.                          *
//*  The x-axis itself goes from 170-890 horiz, at 770 vert,*
//*  with 10 unit tall hash marks.                          *
//*  The data is entered in a 720x720 rectangle, as shown.  *
//*                                                         *
//***********************************************************

/////////////////////////////////////////////////////////////////////////////
// EyeGraphCtrl window

//
struct DataPoint {
	double dLogicalX;
	double dLogicalY;
};

class CEyeGraphCtrl : public CStatic
{
public:
//OK, for the moment, the only type of graph is a scatter graph of attempted/actual spherical refraction.
typedef enum GraphType
{
	SphereScatter = 0,
	DefocusScatter,
} GraphType;


// Construction
public:
	CEyeGraphCtrl();

// Attributes
public:
	GraphType m_Type;
	CArray<DataPoint, DataPoint&> *m_pPointArray;
	CString m_strCaption; //This has defaults for each type, but can be overidden.
protected:
	
// Operations
public:

protected:
	void DrawAxes(CPaintDC *pDC);
	void DrawCaptions(CPaintDC *pDC);
	void DrawPoints(CPaintDC *pDC);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEyeGraphCtrl)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CEyeGraphCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CEyeGraphCtrl)
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EYEGRAPHCTRL_H__DF82A9C9_107B_4D13_8A6D_56A6A21E241A__INCLUDED_)
