#pragma once

class CEmrChildWnd : public CMDIChildWndEx
{
	DECLARE_DYNCREATE(CEmrChildWnd)
public:
	CEmrChildWnd()
	{
	}

// Attributes
public:

// Operations
public:

// Overrides
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CEmrChildWnd()
	{
	}

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

	// (a.walling 2012-07-05 08:40) - PLID 51335 - Override to use NxSetWindowText
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);
	void CMDIChildWnd_OnUpdateFrameTitle(BOOL bAddToTitle);
};
