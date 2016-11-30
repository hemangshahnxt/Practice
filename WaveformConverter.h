// WaveformConverter.h: interface for the CWaveformConverter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WAVEFORMCONVERTER_H__9A0AD9F0_D3B0_4832_86B3_FE4DD214E2EC__INCLUDED_)
#define AFX_WAVEFORMCONVERTER_H__9A0AD9F0_D3B0_4832_86B3_FE4DD214E2EC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <mmreg.h>
#include <mmsystem.h>
#include <msacm.h>

#ifndef SIZEOF_WAVEFORMATEX
#define SIZEOF_WAVEFORMATEX(pwfx)   ((WAVE_FORMAT_PCM==(pwfx)->wFormatTag)?sizeof(PCMWAVEFORMAT):(sizeof(WAVEFORMATEX)+(pwfx)->cbSize))
#endif

typedef struct tAACONVERTDESC
{
    HACMDRIVERID        hadid;
    HACMDRIVER          had;
    HACMSTREAM          has;
    DWORD               fdwOpen;

    HMMIO               hmmioSrc;
    HMMIO               hmmioDst;

    MMCKINFO            ckDst;
    MMCKINFO            ckDstRIFF;

    UINT                uBufferTimePerConvert;

    TCHAR               szFilePathSrc[MAX_PATH];
    LPWAVEFORMATEX      pwfxSrc;
    LPBYTE              pbSrc;
    DWORD               dwSrcSamples;
    DWORD               cbSrcData;
    DWORD               cbSrcReadSize;
    TCHAR               szSrcFormatTag[ACMFORMATTAGDETAILS_FORMATTAG_CHARS];
    TCHAR               szSrcFormat[ACMFORMATDETAILS_FORMAT_CHARS];

    TCHAR               szFilePathDst[MAX_PATH];
    LPWAVEFORMATEX      pwfxDst;
    LPBYTE              pbDst;
    DWORD               cbDstBufSize;
    TCHAR               szDstFormatTag[ACMFORMATTAGDETAILS_FORMATTAG_CHARS];
    TCHAR               szDstFormat[ACMFORMATDETAILS_FORMAT_CHARS];

    BOOL                fApplyFilter;
    LPWAVEFILTER        pwfltr;
    TCHAR               szFilterTag[ACMFILTERTAGDETAILS_FILTERTAG_CHARS];
    TCHAR               szFilter[ACMFILTERDETAILS_FILTER_CHARS];

    ACMSTREAMHEADER     ash;

    DWORD               cTotalConverts;
    DWORD               dwTimeTotal;
    DWORD               dwTimeShortest;
    DWORD               dwShortestConvert;
    DWORD               dwTimeLongest;
    DWORD               dwLongestConvert;

} AACONVERTDESC, *PAACONVERTDESC;

typedef UINT        WIOERR;

typedef struct tWAVEIOCB
{
    DWORD           dwFlags;
    HMMIO           hmmio;

    DWORD           dwDataOffset;
    DWORD           dwDataBytes;
    DWORD           dwDataSamples;

    LPWAVEFORMATEX  pwfx;

#if 0
    HWAVEOUT        hwo;
    DWORD           dwBytesLeft;
    DWORD           dwBytesPerBuffer;
    
    DISP FAR *      pDisp;
    INFOCHUNK FAR * pInfo;
#endif

} WAVEIOCB, *PWAVEIOCB, FAR *LPWAVEIOCB;

//
//  error returns from waveio functions
//
#define WIOERR_BASE             (0)
#define WIOERR_NOERROR          (0)
#define WIOERR_ERROR            (WIOERR_BASE+1)
#define WIOERR_BADHANDLE        (WIOERR_BASE+2)
#define WIOERR_BADFLAGS         (WIOERR_BASE+3)
#define WIOERR_BADPARAM         (WIOERR_BASE+4)
#define WIOERR_BADSIZE          (WIOERR_BASE+5)
#define WIOERR_FILEERROR        (WIOERR_BASE+6)
#define WIOERR_NOMEM            (WIOERR_BASE+7)
#define WIOERR_BADFILE          (WIOERR_BASE+8)
#define WIOERR_NODEVICE         (WIOERR_BASE+9)
#define WIOERR_BADFORMAT        (WIOERR_BASE+10)
#define WIOERR_ALLOCATED        (WIOERR_BASE+11)
#define WIOERR_NOTSUPPORTED     (WIOERR_BASE+12)

class CWaveformConverter  
{
protected:
	CString m_strSourceFile;
	CString m_strDestinationFile;

	BOOL GetSourceInfo(PAACONVERTDESC paacd);
	BOOL ConvertBegin(PAACONVERTDESC paacd);
	BOOL ConvertProcess(PAACONVERTDESC paacd);
	BOOL ConvertEnd(PAACONVERTDESC paacd);

protected:
	BOOL riffCopyChunk(HMMIO hmmioSrc, HMMIO hmmioDst, const LPMMCKINFO lpck);

public:
	CWaveformConverter();
	virtual ~CWaveformConverter();

	void SetSourceFile(const CString& strSource);
	void SetDestinationFile(const CString& strDest);
	BOOL Convert();
};

#endif // !defined(AFX_WAVEFORMCONVERTER_H__9A0AD9F0_D3B0_4832_86B3_FE4DD214E2EC__INCLUDED_)
