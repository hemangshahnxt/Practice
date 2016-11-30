// NxIconButton.cpp : implementation file
//

// m.hancock - As of 9/9/2005 and PLID 16872, this file is shared with the NxInkPicture control.

#include "stdafx.h"
#include "NxIconButton.h"
#include "PracticeRc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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

//DRT 5/16/2008 - PLID 29469 - I reduced the "max size" parameter of several of the new icons to 32x, since we never draw them 
//	as larger than that.  This will cut down (several hundred KB per icon) on the compiled executable size.

// (a.walling 2010-07-02 14:23) - PLID 39498 - CNxIconButton is now only a wrapper for AutoSet
//IMPLEMENT_DYNAMIC(CNxIconButton, CNexTechIconButton)

//CNxIconButton::CNxIconButton()
//{
	//DRT 3/31/2008 - PLID 29469 - Setup defaults for not in use.
	//m_ntAutoSetType = NXB_NOTUSED;
	//m_dwAutoSetAttr = NXIB_ALL;
//}

// (c.haag 2016-05-31 11:49) - NX-100789 - AutoSet is now defined in CNexTechIconButton