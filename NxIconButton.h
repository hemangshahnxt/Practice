#if !defined(AFX_NXICONBUTTON_H__54908E63_AF28_11D3_AD88_00104B318376__INCLUDED_)
#define AFX_NXICONBUTTON_H__54908E63_AF28_11D3_AD88_00104B318376__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NxIconButton.h : header file
//

// m.hancock - As of 9/9/2005 and PLID 16872, this file is shared with the NxInkPicture control.
//DRT - Split most code to NexTechIconButton, which is now shared, not this file.


/*
	DRT 4/14/2008 - PLID 29650
	NxIconButton has been shared for a while with NxInkPicture.  Unfortunately the cpp file has references to resource 
	files (IDI_whatever) all through it.  We should never share files that reference resources, it's just impossible 
	to keep up to date when icons change or are added.
 
	So, I'm going to split it into 2 classes.
	CNexTechIconButton (for lack of a better name) - This will take all the current function except the AutoSet function.  
	Will handle all the drawing, manipulation, etc.
	CNxIconButton (so I don't have to rename 10,000 buttons in Practice) - This will derive from CNexTechIconButton, and 
	implement an AutoSet() function, which can then freely reference resources in the current project.
 
	If you ever need to share source for buttons, share CNexTechIconButton, and either manually do SetIcon with your local 
	resources, or derive your own in your local project.
	You should never share NxIconButton.
*/


//#include <tmschmea.h>

// (c.haag 2016-05-31 11:39) - NX-100789 -  NXB_TYPE has been moved to NexTechIconButton.h

/////////////////////////////////////////////////////////////////////////////
// CNxIconButton window

class CNxIconButton : public CNexTechIconButton
{
// Construction
public:
	// (a.walling 2010-07-02 14:23) - PLID 39498 - CNxIconButton is now only a wrapper for AutoSet	
	//CNxIconButton();
	//DECLARE_DYNAMIC(CNxIconButton)

	// (c.haag 2016-05-31 11:49) - NX-100789 - Now handled by CNexTechIconButton
	//bool AutoSet(NXB_TYPE type, DWORD attributes = NXIB_ALL);

//private:
	//DRT 3/31/2008 - PLID 29469 - We now remember the auto set type, if this button is of
	//	the AutoSet variety.  This *only* applies to buttons setup with AutoSet().
	// (a.walling 2010-07-02 14:23) - PLID 39498 - No longer remembered -- was never used anywhere anyway
	//NXB_TYPE m_ntAutoSetType;
	//DWORD m_dwAutoSetAttr;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NXICONBUTTON_H__54908E63_AF28_11D3_AD88_00104B318376__INCLUDED_)
