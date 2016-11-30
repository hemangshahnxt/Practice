// FormDate.h: interface for the FormDate class.
//An edit box that formats as a date, and optionally checks birthdays
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FORMDATE_H__D37485A6_EBF6_11D2_9354_00104B318376__INCLUDED_)
#define AFX_FORMDATE_H__D37485A6_EBF6_11D2_9354_00104B318376__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FormEdit.h"
#include "FormFormat.h"

class FormDate : public FormEdit  
{
public:
								FormDate				();
								FormDate				(FormControl *control);
	virtual						~FormDate				();
			int					Birthdate				();
			COleDateTime		GetDateValue			();
			void				SetDateValue			(COleDateTime dt);
	virtual	BOOL				Create					(CWnd *parent, int id);
	virtual void				PrintOut				(CDC *pDC);
//	virtual void				DeleteThis				(void){delete this;}
	virtual BOOL Refresh(FormLayer* layer);
	virtual void Save(int iDocumentID);

protected:
	virtual	BOOL				PreTranslateMessage	(MSG* pMsg);
	int							Format					();
};

#endif // !defined(AFX_FORMDATE_H__D37485A6_EBF6_11D2_9354_00104B318376__INCLUDED_)
