// FloatingImageWnd.h: interface for the CFloatingImageWnd class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FLOATINGIMAGEWND_H__B73A7935_F287_497B_8138_F6D2E0810AA8__INCLUDED_)
#define AFX_FLOATINGIMAGEWND_H__B73A7935_F287_497B_8138_F6D2E0810AA8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CFloatingImageWnd : public CWnd  
{
public:
	CFloatingImageWnd();
	virtual ~CFloatingImageWnd();

	HBITMAP m_hImage;

protected:
	//{{AFX_MSG(CPhotoViewerCtrl)
	afx_msg void OnPaint();
	afx_msg void OnMove(int x, int y);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_FLOATINGIMAGEWND_H__B73A7935_F287_497B_8138_F6D2E0810AA8__INCLUDED_)
