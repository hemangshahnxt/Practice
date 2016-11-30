// NxRichEditCtrl.h: interface for the CNxRichEditCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NXRICHEDITCTRL_H__71EF6313_7940_41D8_AC9E_413D98438BA0__INCLUDED_)
#define AFX_NXRICHEDITCTRL_H__71EF6313_7940_41D8_AC9E_413D98438BA0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CNxRichEditCtrl : public CRichEditCtrl  
{
public:
	CNxRichEditCtrl();
	virtual ~CNxRichEditCtrl();

	void SetRichText(const CString &strRichText);
	void SetPlainText(const CString &strPlainText);

	void GetRichText(CString &strRichText);
	void GetPlainText(CString &strPlainText);

	DECLARE_MESSAGE_MAP()
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNxRichEditCtrl)
	virtual void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

};

#endif // !defined(AFX_NXRICHEDITCTRL_H__71EF6313_7940_41D8_AC9E_413D98438BA0__INCLUDED_)
