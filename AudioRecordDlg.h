#if !defined(AFX_AUDIORECORDDLG_H__1A9F0B20_5235_4816_865C_6135FBC0202F__INCLUDED_)
#define AFX_AUDIORECORDDLG_H__1A9F0B20_5235_4816_865C_6135FBC0202F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AudioRecordDlg.h : header file
//
// (c.haag 2006-09-22 17:43) - PLID 21327 - Initial implementation

#include "nxaudiocapture.h"
#include "wmpeventsink.h"
#include "practicerc.h"
// (a.walling 2014-04-30 15:19) - PLID 61989 - import a typelibrary rather than a dll
#import "wmp.tlb"

class CEMN;

/////////////////////////////////////////////////////////////////////////////
// CAudioRecordDlg dialog

class CAudioRecordDlg : public CNxDialog
{
// Construction
public:
	// (a.walling 2010-04-13 14:18) - PLID 36821 - Create a modeless audio record dialog, or bring up the existing one if it exists
	// (a.walling 2010-07-27 16:39) - PLID 39433 - Support attaching to an EMN
	static void DoAudioRecord(CWnd* pParent, long nPatientID, bool bIsPatient, long nPicID, CEMN* pEmn); // (a.walling 2010-04-13 14:18) - PLID 36821
	static CAudioRecordDlg* GetCurrentInstance();

public:
	// (a.walling 2010-07-27 16:24) - PLID 39433 - Helpers
	void BringToTop();
	bool IsEmpty();

private:
	CAudioRecordDlg(CWnd* pParent);   // standard constructor

	// (a.walling 2010-04-13 14:18) - PLID 36821 - The existing instance
	static CAudioRecordDlg* m_pCurrentInstance;

// Dialog Data
	// (a.walling 2008-10-27 08:27) - PLID 31827 - CWnd m_wndWMP for windowless activex control
	//{{AFX_DATA(CAudioRecordDlg)
	enum { IDD = IDD_AUDIO_RECORD_DIALOG };
	CSliderCtrl	m_Progress;
	CButton	m_btnStop;
	CButton	m_btnRec;
	CButton	m_btnPlay;
	CButton m_btnPause;
	CNxStatic	m_nxstaticAudioStatus;
	CNxStatic	m_nxstaticRecPos;
	CNxStatic	m_nxstaticRecLength;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CWnd	m_wndWMP;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAudioRecordDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
private:
	CNxAudioCapture m_NxAudioCapture;
	WMPLib::IWMPPlayer4Ptr m_WMP;

private:
	CWMPEventSink m_WMPSink;

private:
	DWORD m_nTotalTime;
	DWORD m_nTotalBytes;

private:
	BOOL m_bIsPlaying;
	BOOL m_bIsPaused;

private:
	CString m_strTempOutputFileName;	// The location of the temporary file that the
										// NxAudioCapture object records to

	long m_nPatientID;					// The ID of the patient we are ultimately attaching
										// the recording to

	bool m_bIsPatient;					// (a.walling 2010-04-13 14:55) - PLID 36821 - Might be a contact

	long m_nPicID;						// The ID of the PIC we retain for history attaching

	CEMN* m_pEmn;						// (a.walling 2010-07-27 16:40) - PLID 39433 - Support attaching to an EMN
	

public:
	long GetPatientID() const;
	//void SetPatientID(long nID);

	long GetPicID() const;
	//void SetPicID(long nID);

	// (a.walling 2010-07-27 16:40) - PLID 39433
	CEMN* GetEmn() const;
	void ResetEmn();

private:
	int ConvertTimeToProgressValue(DWORD nMilliseconds);
	DWORD ConvertProgressToTimeValue(int nPos);

private:
	void SetStatusText(const CString&);
	void SetPositionText(DWORD nMilliseconds);
	void SetLengthText(DWORD nMilliseconds, DWORD nBytes);

protected:
	// Generated message map functions
	//{{AFX_MSG(CAudioRecordDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnBtnRec();
	afx_msg void OnBtnStop();
	afx_msg void OnBtnPlay();
	afx_msg void OnBtnPause();
	afx_msg LRESULT OnWMPPlayStateChange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnRecordingTerminated(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnRecordingProgress(WPARAM wParam, LPARAM lParam);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar);
	virtual void PostNcDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AUDIORECORDDLG_H__1A9F0B20_5235_4816_865C_6135FBC0202F__INCLUDED_)
