// DoesExist.h: interface for the CDoesExist class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DOESEXIST_H__76663761_E33B_4928_8EF3_C9D51484786B__INCLUDED_)
#define AFX_DOESEXIST_H__76663761_E33B_4928_8EF3_C9D51484786B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDoesExist  
{
	CString m_strFilePath; // The name of the file we are testing for existence
	CWinThread* m_pThread;

public:
	typedef enum { eNoAction, ePending, eFailed, eSuccess } EStatus;
	CDoesExist();
	CDoesExist(CString strFilePath, BOOL bTryAccess = TRUE);
	virtual ~CDoesExist();

	void SetPath(const CString& strPath);
	EStatus GetStatus();

	void TryAccess();
	EStatus WaitForAccess(const CString& strWaitMessage);
	void Stop();
};

#endif // !defined(AFX_DOESEXIST_H__76663761_E33B_4928_8EF3_C9D51484786B__INCLUDED_)
