// NxFolderField.h: interface for the CNxFolderField class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NXFOLDERFIELD_H__907016C5_3D51_43B4_BB98_C7440FE2FD51__INCLUDED_)
#define AFX_NXFOLDERFIELD_H__907016C5_3D51_43B4_BB98_C7440FE2FD51__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>

class CNxFolderField  
{
protected:
	CString m_strField;
	CString m_strAlias;

public:
	CNxFolderField();
	CNxFolderField(const CString& strField, const CString& strAlias);
	virtual ~CNxFolderField();

public:
	const CNxFolderField& operator=(const CNxFolderField&);
	
public:
	CString GetField() const;
	CString GetAlias() const;
};

typedef CArray<CNxFolderField, CNxFolderField> CNxFolderFieldArray;

#endif // !defined(AFX_NXFOLDERFIELD_H__907016C5_3D51_43B4_BB98_C7440FE2FD51__INCLUDED_)
