#pragma once

#include "FormControl.h"

class FormCheck : public CButton, public FormControl
{
public:
	DECLARE_DYNAMIC(FormCheck)

	long m_nSetupGroupID;

	FormCheck(int SetX, int SetY, int SetWidth, int SetHeight, int SetFormat, CString SetSource);
	FormCheck(FormControl *control);
	virtual			BOOL Create	(CWnd* pParentWnd, long nSetupGroupID);
	virtual			~FormCheck	();
	virtual void	PrintOut	(CDC *pDC);
	virtual void	DrawItem	(LPDRAWITEMSTRUCT lpDrawItemStruct);
	void			SetCheck	(int nCheck){check = nCheck;CButton::SetCheck(nCheck);}
	virtual BOOL	PreTranslateMessage( MSG* pMsg );
	virtual void	OnPaint		();
//	virtual void	DeleteThis	(void)	{delete this;}
	void * operator new(size_t nSize)
	{
		return CButton::operator new(nSize);
	}
	
	void operator delete( void* p )
	{	
		CButton::operator delete(p);
	}

#ifdef _DEBUG
	void * operator new(size_t nSize, LPCSTR lpszFileName, int nLine)
	{
		return CButton::operator new(nSize, lpszFileName, nLine);
	}

	void operator delete(void *p, LPCSTR lpszFileName, int nLine)
	{
		CButton::operator delete(p, lpszFileName, nLine);
	}
#endif

	int				check;

	virtual BOOL Refresh(FormLayer* layer);
	virtual void Save(int iDocumentID);
};