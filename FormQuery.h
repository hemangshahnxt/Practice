// FormQuery.h: interface for the CFormQuery class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FORMQUERY_H__2F671141_B3C5_11D4_A760_0001024317D6__INCLUDED_)
#define AFX_FORMQUERY_H__2F671141_B3C5_11D4_A760_0001024317D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CFormQuery  
{
public:
	CFormQuery();
	CFormQuery(long QueryID,CString SqlString,long Color,long FontType,long StaticFontType);
	virtual ~CFormQuery();
	void operator = (CFormQuery fq);
	long ID;
	CString sql;
	long BGColor;
	long Font;
	long StaticFont;

};

#endif // !defined(AFX_FORMQUERY_H__2F671141_B3C5_11D4_A760_0001024317D6__INCLUDED_)
