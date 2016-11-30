#ifndef PRACTICE_NXRPEMDICHILDWND_H
#define PRACTICE_NXRPEMDICHILDWND_H

#pragma once



#include "peplus.h"

// Declaration for our CNxRPEMDIChildWnd class, a wrapper for the peplus mdi child window
///////////////////////////////////////////////////////////////////////////////////////////


class CNxRPEMDIChildWnd : public CRPEMDIChildWnd
{
public:
	DECLARE_DYNCREATE(CNxRPEMDIChildWnd)

public:
	CNxRPEMDIChildWnd(CRPEJob *pJob = NULL);
	~CNxRPEMDIChildWnd();

public:
	afx_msg void OnFilePrint();
	afx_msg void OnClose();

protected:
	DECLARE_MESSAGE_MAP()

public:
	CRPEJob *m_pJob;
	CPrintInfo m_objPrintInfo;
};

#endif