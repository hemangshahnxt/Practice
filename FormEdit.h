#pragma once

#include "FormControl.h"

/////////////////////////////////////////////////////////////////////////////
// FormEdit window

class FormEdit : public FormControl, public CEdit
{
public:
	DECLARE_DYNAMIC(FormEdit)

	FormEdit();
	FormEdit(FormControl *control);
	virtual BOOL Create (int flags, RECT &rect, CWnd *parent);
	virtual void PrintOut(CDC *pDC);
	virtual bool Collide (int x, int y);
//	virtual void DeleteThis(void){delete this;}
	virtual BOOL Refresh(FormLayer* layer);
	virtual void Save(int iDocumentID);
	void Capitalize();
	void UnPunctuate();
	void ResetFont();

	//TES 5/13/2008 - PLID 24675 - We need this so that when the text is manually set (by a Refresh() or whatever), we don't
	// flag ourselves as edited when we get the EN_CHANGED message.
	virtual void SetWindowText(LPCTSTR lpszString);

	void * operator new(size_t nSize)
	{
		return CEdit::operator new(nSize);
	}

	void operator delete( void* p )
	{
		CEdit::operator delete(p);
	}

#ifdef _DEBUG
	void * operator new(size_t nSize, LPCSTR lpszFileName, int nLine)
	{
		return CEdit::operator new(nSize, lpszFileName, nLine);
	}

	void operator delete(void *p, LPCSTR lpszFileName, int nLine)
	{
		CEdit::operator delete(p, lpszFileName, nLine);
	}
#endif
};