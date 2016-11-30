#pragma once

class FormControl : public CObject
{
public:
	DECLARE_DYNAMIC(FormControl)

	// (a.walling 2014-03-13 09:01) - PLID 61359 - Show the source in debug mode, otherwise blank
	static bool ShouldShowEmptyControlSource(CWnd* pWnd, const CString& sourceText);
	template<typename T>
	static void ShowEmptyControlSource(T* pControl, const CString& sourceText)
	{
		if (ShouldShowEmptyControlSource(pControl, sourceText)) {
			pControl->SetWindowText(sourceText);
		} else {
			pControl->SetWindowText("");
		}
	}

	FormControl (FormControl *control);
	FormControl *CreateControl (class FormLayer *layer, long nSetupGroupID = -1);
					FormControl();
	virtual			~FormControl();
	virtual void	PrintOut (CDC *pDC){}
//	virtual void	DeleteThis (void) {delete this;}
	virtual	void	Draw (CDC *dc){}
	virtual void	ResetFont();
	long			format,
					x,
					y,
					width,
					height,
					value,
					nID,
					id,
					form,
					color;
	CString			source;
	CFont*			m_pOldFont;

	//TES 4/30/2008 - PLID 26475 - Made the m_bIsEdited variable protected.
protected:
	// (j.jones 2007-06-22 09:31) - PLID 25665 - track whether the control is edited
	BOOL m_bIsEdited;

	//TES 4/30/2008 - PLID 26475 - The control sets this when it's changing its own text (formatting it as a date, for
	// example), so that it knows not to flag itself as edited when the EN_CHANGE is fired.
	BOOL m_bSelfEditing;
public:
	//TES 4/30/2008 - PLID 26475 - Accessor functions for the m_bIsEdited variable.
	BOOL GetEdited() {return m_bIsEdited;}
	//TES 4/30/2008 - PLID 26475 - Note that this will have no effect if m_bSelfEditing is TRUE.
	void SetEdited(BOOL bEdited);

	void * operator new(size_t nSize)
	{
		return CObject::operator new(nSize);
	}

	void operator delete (void * p )
	{	
		CObject::operator delete(p);
	}

#ifdef _DEBUG
	
	void * operator new(size_t nSize, LPCSTR lpszFileName, int nLine)
	{
		return CObject::operator new(nSize, lpszFileName, nLine);
	}

	void operator delete(void *p, LPCSTR lpszFileName, int nLine)
	{
		CObject::operator delete(p);
	}

#endif
};