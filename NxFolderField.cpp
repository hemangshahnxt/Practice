// NxFolderField.cpp: implementation of the CNxFolderField class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NxFolderField.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNxFolderField::CNxFolderField()
{
}

CNxFolderField::CNxFolderField(const CString& strField, const CString& strAlias):
m_strField(strField),m_strAlias(strAlias)
{
}

CNxFolderField::~CNxFolderField()
{
}

const CNxFolderField& CNxFolderField::operator=(const CNxFolderField& src)
{
	m_strField = src.m_strField;
	m_strAlias = src.m_strAlias;
	return *this;
}

CString CNxFolderField::GetField() const
{
	return m_strField;
}

CString CNxFolderField::GetAlias() const
{
	return m_strAlias;
}