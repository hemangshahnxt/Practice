// FormQuery.cpp: implementation of the CFormQuery class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "practice.h"
#include "FormQuery.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CFormQuery::CFormQuery()
{

}

CFormQuery::CFormQuery(long QueryID,CString SqlString,long Color,long FontType,long StaticFontType)
{
	ID = QueryID;
	sql = SqlString;
	BGColor  = Color;
	Font = FontType;
	StaticFont = StaticFontType;
}

void CFormQuery::operator = (CFormQuery fq) {
	this->ID = fq.ID;
	this->sql = fq.sql;
	this->BGColor = fq.BGColor;
	this->Font = fq.Font;
	this->StaticFont = fq.StaticFont;
}

CFormQuery::~CFormQuery()
{

}
