// PatientDialog.h: interface for the CPatientDialog class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PATIENTDIALOG_H__A3347D13_AF60_11D3_A3C0_00C04F42E33B__INCLUDED_)
#define AFX_PATIENTDIALOG_H__A3347D13_AF60_11D3_A3C0_00C04F42E33B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CPatientDialog : public CNxDialog  
{
public:
	CPatientDialog(int IDD, CWnd* pParent);
	virtual ~CPatientDialog();
	virtual void SetColor(OLE_COLOR nNewColor);
	DWORD GetCurrentColor();
protected:
	DWORD m_lastColor;
};

#endif // !defined(AFX_PATIENTDIALOG_H__A3347D13_AF60_11D3_A3C0_00C04F42E33B__INCLUDED_)
