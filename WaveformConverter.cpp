// WaveformConverter.cpp: implementation of the CWaveformConverter class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WaveformConverter.h"

// (a.walling 2009-10-13 10:01) - PLID 35930
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define GlobalAllocPtr(cb) (new char[cb])
#define GlobalFreePtr(cb) {delete cb;}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWaveformConverter::CWaveformConverter()
{

}

CWaveformConverter::~CWaveformConverter()
{

}

void CWaveformConverter::SetSourceFile(const CString& strSource)
{
	m_strSourceFile = strSource;
}

void CWaveformConverter::SetDestinationFile(const CString& strDest)
{
	m_strDestinationFile = strDest;
}

BOOL CWaveformConverter::Convert()
{
	AACONVERTDESC aacd;
	memset(&aacd, 0, sizeof(aacd));
	strncpy(aacd.szFilePathSrc, m_strSourceFile, MAX_PATH);
	strncpy(aacd.szFilePathDst, m_strDestinationFile, MAX_PATH);

	// Get source waveform parameters
	if (!GetSourceInfo(&aacd)) return FALSE;
	// Assign destination waveform parameters
	aacd.pwfxDst = (LPWAVEFORMATEX)GlobalAllocPtr(sizeof(WAVEFORMATEX));
	memset(aacd.pwfxDst, 0, sizeof(WAVEFORMATEX));
	aacd.pwfxDst->wFormatTag = 1;
	aacd.pwfxDst->nChannels = 1;
	aacd.pwfxDst->nSamplesPerSec = 11025;
	aacd.pwfxDst->nAvgBytesPerSec = 22050;
	aacd.pwfxDst->nBlockAlign = 2;
	aacd.pwfxDst->wBitsPerSample = 16;

	// Convert the source file into the destination file
	if (!ConvertBegin(&aacd)) return FALSE;
	if (!ConvertProcess(&aacd)) return FALSE;
	if (!ConvertEnd(&aacd)) return FALSE;

	return TRUE;
}

BOOL CWaveformConverter::ConvertBegin(PAACONVERTDESC paacd)
{
    MMRESULT            mmr;
    MMCKINFO            ckSrcRIFF;
    MMCKINFO            ck;
    DWORD               dw;
    LPACMSTREAMHEADER   pash;
    LPWAVEFILTER        pwfltr;



    //
    //
    //
    if (NULL != paacd->hadid)
    {
        mmr = acmDriverOpen(&paacd->had, paacd->hadid, 0L);
        if (MMSYSERR_NOERROR != mmr)
        {
            MsgBox(MB_OK | MB_ICONEXCLAMATION, "The selected driver (hadid=%.04Xh) cannot be opened. Error = %u",
				paacd->hadid, mmr);
            return (FALSE);
        }
    }


    //
    //
    //
    //
    pwfltr = paacd->fApplyFilter ? paacd->pwfltr : (LPWAVEFILTER)NULL;

    mmr = acmStreamOpen(&paacd->has,
                        paacd->had,
                        paacd->pwfxSrc,
                        paacd->pwfxDst,
                        pwfltr,
                        0L,
                        0L,
                        paacd->fdwOpen);

    if (MMSYSERR_NOERROR != mmr)
    {
        MsgBox(MB_OK | MB_ICONEXCLAMATION, "acmStreamOpen() failed.  Error = %u", mmr);
        return (FALSE);
    }


    //
    //
    //
    mmr = acmStreamSize(paacd->has,
                        paacd->cbSrcReadSize,
                        &paacd->cbDstBufSize,
                        ACM_STREAMSIZEF_SOURCE);

    if (MMSYSERR_NOERROR != mmr)
    {
        MsgBox(MB_OK | MB_ICONEXCLAMATION, "acmStreamSize() failed. Error = %u", mmr);
        return (FALSE);
    }



    //
    //  first try to open the file, etc.. open the given file for reading
    //  using buffered I/O
    //
    paacd->hmmioSrc = mmioOpen(paacd->szFilePathSrc,
                               NULL,
                               MMIO_READ | MMIO_DENYWRITE | MMIO_ALLOCBUF);
    if (NULL == paacd->hmmioSrc)
        goto aacb_Error;

    //
    //
    //
    paacd->hmmioDst = mmioOpen(paacd->szFilePathDst,
                               NULL,
                               MMIO_CREATE | MMIO_WRITE | MMIO_EXCLUSIVE | MMIO_ALLOCBUF);
    if (NULL == paacd->hmmioDst)
        goto aacb_Error;



    //
    //
    //
    //
    pash = &paacd->ash;
    pash->fdwStatus = 0L;


    //
    //  allocate the src and dst buffers for reading/converting data
    //
    paacd->pbSrc = (unsigned char*)GlobalAllocPtr(paacd->cbSrcReadSize);
    if (NULL == paacd->pbSrc)
        goto aacb_Error;
    
    paacd->pbDst = (unsigned char*)GlobalAllocPtr(paacd->cbDstBufSize);
    if (NULL == paacd->pbDst)
        goto aacb_Error;


    //
    //
    //
    //
    pash->cbStruct          = sizeof(*pash);
    pash->fdwStatus         = 0L;
    pash->dwUser            = 0L;
    pash->pbSrc             = paacd->pbSrc;
    pash->cbSrcLength       = paacd->cbSrcReadSize;
    pash->cbSrcLengthUsed   = 0L;
    pash->dwSrcUser         = paacd->cbSrcReadSize;
    pash->pbDst             = paacd->pbDst;
    pash->cbDstLength       = paacd->cbDstBufSize;
    pash->cbDstLengthUsed   = 0L;
    pash->dwDstUser         = paacd->cbDstBufSize;

    mmr = acmStreamPrepareHeader(paacd->has, pash, 0L);
    if (MMSYSERR_NOERROR != mmr)
    {
        MsgBox(MB_OK | MB_ICONEXCLAMATION, "acmStreamPrepareHeader() failed. Error = %u", mmr);
        goto aacb_Error;
    }                          



    //
    //  create the RIFF chunk of form type 'WAVE'
    //
    //
    paacd->ckDstRIFF.fccType = mmioFOURCC('W', 'A', 'V', 'E');
    paacd->ckDstRIFF.cksize  = 0L;
    if (mmioCreateChunk(paacd->hmmioDst, &paacd->ckDstRIFF, MMIO_CREATERIFF))
        goto aacb_Error;

    //
    //  locate a 'WAVE' form type in a 'RIFF' thing...
    //
    ckSrcRIFF.fccType = mmioFOURCC('W', 'A', 'V', 'E');
    if (mmioDescend(paacd->hmmioSrc, (LPMMCKINFO)&ckSrcRIFF, NULL, MMIO_FINDRIFF))
        goto aacb_Error;

    //
    //  we found a WAVE chunk--now go through and get all subchunks that
    //  we know how to deal with...
    //
    while (mmioDescend(paacd->hmmioSrc, &ck, &ckSrcRIFF, 0) == 0)
    {
        //
        //  quickly check for corrupt RIFF file--don't ascend past end!
        //
        if ((ck.dwDataOffset + ck.cksize) > (ckSrcRIFF.dwDataOffset + ckSrcRIFF.cksize))
            goto aacb_Error;

        switch (ck.ckid)
        {
            //
            //  explicitly skip these...
            //
            //
            //
            case mmioFOURCC('f', 'm', 't', ' '):
                break;

            case mmioFOURCC('d', 'a', 't', 'a'):
                break;

            case mmioFOURCC('f', 'a', 'c', 't'):
                break;

            case mmioFOURCC('J', 'U', 'N', 'K'):
                break;

            case mmioFOURCC('P', 'A', 'D', ' '):
                break;

            case mmioFOURCC('c', 'u', 'e', ' '):
                break;


            //
            //  copy chunks that are OK to copy
            //
            //
            //
            case mmioFOURCC('p', 'l', 's', 't'):
                // although without the 'cue' chunk, it doesn't make much sense
                riffCopyChunk(paacd->hmmioSrc, paacd->hmmioDst, &ck);
                break;

            case mmioFOURCC('D', 'I', 'S', 'P'):
                riffCopyChunk(paacd->hmmioSrc, paacd->hmmioDst, &ck);
                break;

                
            //
            //  don't copy unknown chunks
            //
            //
            //
            default:
                break;
        }

        //
        //  step up to prepare for next chunk..
        //
        mmioAscend(paacd->hmmioSrc, &ck, 0);
    }

#if 0
    //
    //  now write out possibly editted chunks...
    //
    if (riffWriteINFO(paacd->hmmioDst, (glwio.pInfo)))
    {
        goto aacb_Error;
    }
#endif

    //
    // go back to beginning of data portion of WAVE chunk
    //
    if (-1 == mmioSeek(paacd->hmmioSrc, ckSrcRIFF.dwDataOffset + sizeof(FOURCC), SEEK_SET))
        goto aacb_Error;

    ck.ckid = mmioFOURCC('d', 'a', 't', 'a');
    mmioDescend(paacd->hmmioSrc, &ck, &ckSrcRIFF, MMIO_FINDCHUNK);


    //
    //  now create the destination fmt, fact, and data chunks _in that order_
    //
    //
    //
    //  hmmio is now descended into the 'RIFF' chunk--create the format chunk
    //  and write the format header into it
    //
    dw = SIZEOF_WAVEFORMATEX(paacd->pwfxDst);

    paacd->ckDst.ckid   = mmioFOURCC('f', 'm', 't', ' ');
    paacd->ckDst.cksize = dw;
    if (mmioCreateChunk(paacd->hmmioDst, &paacd->ckDst, 0))
        goto aacb_Error;

    if (mmioWrite(paacd->hmmioDst, (HPSTR)paacd->pwfxDst, dw) != (LONG)dw)
        goto aacb_Error;

    if (mmioAscend(paacd->hmmioDst, &paacd->ckDst, 0) != 0)
        goto aacb_Error;

    //
    //  create the 'fact' chunk (not necessary for PCM--but is nice to have)
    //  since we are not writing any data to this file (yet), we set the
    //  samples contained in the file to 0..
    //
    paacd->ckDst.ckid   = mmioFOURCC('f', 'a', 'c', 't');
    paacd->ckDst.cksize = 0L;
    if (mmioCreateChunk(paacd->hmmioDst, &paacd->ckDst, 0))
        goto aacb_Error;

    if (mmioWrite(paacd->hmmioDst, (HPSTR)&paacd->dwSrcSamples, sizeof(DWORD)) != sizeof(DWORD))
        goto aacb_Error;

    if (mmioAscend(paacd->hmmioDst, &paacd->ckDst, 0) != 0)
        goto aacb_Error;


    //
    //  create the data chunk AND STAY DESCENDED... for reasons that will
    //  become apparent later..
    //
    paacd->ckDst.ckid   = mmioFOURCC('d', 'a', 't', 'a');
    paacd->ckDst.cksize = 0L;
    if (mmioCreateChunk(paacd->hmmioDst, &paacd->ckDst, 0))
        goto aacb_Error;

    //
    //  at this point, BOTH the src and dst files are sitting at the very
    //  beginning of their data chunks--so we can READ from the source,
    //  CONVERT the data, then WRITE it to the destination file...
    //
    return TRUE;

aacb_Error:
	return FALSE;
}

BOOL CWaveformConverter::ConvertProcess(PAACONVERTDESC paacd)
{
    MMRESULT            mmr;
    TCHAR               ach[40];
    DWORD               dw;
    WORD                w;
    DWORD               dwCurrent;
    WORD                wCurPercent;
    LPACMSTREAMHEADER   pash;
    DWORD               cbRead;
    DWORD               dwTime;


    wCurPercent = (WORD)-1;

    paacd->cTotalConverts    = 0L;
    paacd->dwTimeTotal       = 0L;
    paacd->dwTimeLongest     = 0L;
    if (0 == paacd->cbSrcData)
    {
        paacd->dwTimeShortest    = 0L;
        paacd->dwShortestConvert = 0L;
        paacd->dwLongestConvert  = 0L;
    }
    else
    {
        paacd->dwTimeShortest    = (DWORD)-1L;
        paacd->dwShortestConvert = (DWORD)-1L;
        paacd->dwLongestConvert  = (DWORD)-1L;
    }

    pash = &paacd->ash;

    for (dwCurrent = 0; dwCurrent < paacd->cbSrcData; )
    {
        w = (WORD)((dwCurrent * 100) / paacd->cbSrcData);
        if (w != wCurPercent)
        {
            wCurPercent = w;
            wsprintf(ach, TEXT("%u%%"), wCurPercent);

//            if (hdlg)
  //              SetDlgItemText(hdlg, IDD_AACONVERT_TXT_STATUS, ach);
        }

//        AppDlgYield(hdlg);

//        if (gfCancelConvert)
  //          goto aacc_Error;

        //
        //
        //
        cbRead = min(paacd->cbSrcReadSize, paacd->cbSrcData - dwCurrent);
        dw = mmioRead(paacd->hmmioSrc, (char*)paacd->pbSrc, cbRead);
        if (0L == dw)
            break;


//        AppDlgYield(hdlg);
//        if (gfCancelConvert)
  //          goto aacc_Error;

             

        //
        //
        //
        pash->cbSrcLength     = dw;
        pash->cbDstLengthUsed = 0L;



        dwTime = timeGetTime();

        mmr = acmStreamConvert(paacd->has,
                               &paacd->ash,
                               ACM_STREAMCONVERTF_BLOCKALIGN);

        if (MMSYSERR_NOERROR != mmr)
        {
            MsgBox(MB_OK | MB_ICONEXCLAMATION, "acmStreamConvert() failed. Error = %u", mmr);
            goto aacc_Error;
        }

        while (0 == (ACMSTREAMHEADER_STATUSF_DONE & ((AACONVERTDESC volatile *)paacd)->ash.fdwStatus))
            ;

        dwTime = timeGetTime() - dwTime;


        paacd->dwTimeTotal += dwTime;

        if (dwTime < paacd->dwTimeShortest)
        {
            paacd->dwTimeShortest    = dwTime;
            paacd->dwShortestConvert = paacd->cTotalConverts;
        }

        if (dwTime > paacd->dwTimeLongest)
        {
            paacd->dwTimeLongest     = dwTime;
            paacd->dwLongestConvert  = paacd->cTotalConverts;
        }

        paacd->cTotalConverts++;


//        AppDlgYield(hdlg);
//        if (gfCancelConvert)
            //goto aacc_Error;


        //
        //
        //
        dw = (cbRead - pash->cbSrcLengthUsed);
        if (0L != dw)
        {
            mmioSeek(paacd->hmmioSrc, -(LONG)dw, SEEK_CUR);
        }

        dwCurrent += pash->cbSrcLengthUsed;


        //
        //
        //
        dw = pash->cbDstLengthUsed;
        if (0L == dw)
            break;
          
        if (mmioWrite(paacd->hmmioDst, (const char*)paacd->pbDst, dw) != (LONG)dw)
            goto aacc_Error;
    }


    //
    //
    //
    //
    //
    //
    wCurPercent = (WORD)-1;

    for (;paacd->cbSrcData;)
    {
        w = (WORD)((dwCurrent * 100) / paacd->cbSrcData);
        if (w != wCurPercent)
        {
            wCurPercent = w;
            wsprintf(ach, TEXT("Cleanup Pass -- %u%%"), wCurPercent);

  //          if (hdlg)
//                SetDlgItemText(hdlg, IDD_AACONVERT_TXT_STATUS, ach);
        }

//        AppDlgYield(hdlg);
//        if (gfCancelConvert)
  //          goto aacc_Error;


        //
        //
        //
        dw = 0L;
        cbRead = min(paacd->cbSrcReadSize, paacd->cbSrcData - dwCurrent);
        if (0L != cbRead)
        {
            dw = mmioRead(paacd->hmmioSrc, (char*)paacd->pbSrc, cbRead);
            if (0L == dw)
                break;
        }


//        AppDlgYield(hdlg);
  //      if (gfCancelConvert)
    //        goto aacc_Error;

             

        //
        //
        //
        pash->cbSrcLength     = dw;
        pash->cbDstLengthUsed = 0L;



        dwTime = timeGetTime();

        mmr = acmStreamConvert(paacd->has,
                               &paacd->ash,
                               ACM_STREAMCONVERTF_BLOCKALIGN |
                               ACM_STREAMCONVERTF_END);

        if (MMSYSERR_NOERROR != mmr)
        {
            MsgBox(MB_OK | MB_ICONEXCLAMATION, "acmStreamConvert() failed. Error = %u", mmr);
            goto aacc_Error;
        }

        while (0 == (ACMSTREAMHEADER_STATUSF_DONE & ((AACONVERTDESC volatile *)paacd)->ash.fdwStatus))
            ;

        dwTime = timeGetTime() - dwTime;


        paacd->dwTimeTotal += dwTime;

        if (dwTime < paacd->dwTimeShortest)
        {
            paacd->dwTimeShortest    = dwTime;
            paacd->dwShortestConvert = paacd->cTotalConverts;
        }

        if (dwTime > paacd->dwTimeLongest)
        {
            paacd->dwTimeLongest     = dwTime;
            paacd->dwLongestConvert  = paacd->cTotalConverts;
        }

        paacd->cTotalConverts++;


//        AppDlgYield(hdlg);
  //      if (gfCancelConvert)
    //        goto aacc_Error;

        //
        //
        //
        dw = pash->cbDstLengthUsed;
        if (0L == dw)
        {
            pash->cbDstLengthUsed = 0L;

            //
            //  BUGBUG BOBSTER
            //  What if the last conversion ate up some of the source bytes?
            //  It's possible - the codec could cache some of the data
            //  without actually converting it, right?  The pbSrc pointer
            //  might have to be incremented, and cbSrcLength might have to
            //  to be decreased by cbSrcLengthUsed.  This probably wouldn't
            //  happen with most of our codecs though...?
            //
            mmr = acmStreamConvert(paacd->has,
                                   &paacd->ash,
                                   ACM_STREAMCONVERTF_END);

            if (MMSYSERR_NOERROR == mmr)
            {
                while (0 == (ACMSTREAMHEADER_STATUSF_DONE & ((AACONVERTDESC volatile *)paacd)->ash.fdwStatus))
                    ;
            }

            dw = pash->cbDstLengthUsed;
            if (0L == dw)
                break;
        }
          
        if (mmioWrite(paacd->hmmioDst, (const char*)paacd->pbDst, dw) != (LONG)dw)
            goto aacc_Error;

        //
        //
        //
        dw = (cbRead - pash->cbSrcLengthUsed);
        if (0L != dw)
        {
            mmioSeek(paacd->hmmioSrc, -(LONG)dw, SEEK_CUR);
        }

        dwCurrent += pash->cbSrcLengthUsed;

        if (0L == pash->cbDstLengthUsed)
            break;
    }
    //return (!gfCancelConvert);
	return TRUE;

aacc_Error:
    return (FALSE);
}

BOOL CWaveformConverter::ConvertEnd(PAACONVERTDESC paacd)
{
    MMRESULT            mmr;
    LPACMSTREAMHEADER   pash;

    //
    //
    //
    //
    if (NULL != paacd->hmmioSrc)
    {
        mmioClose(paacd->hmmioSrc, 0);
        paacd->hmmioSrc = NULL;
    }

    if (NULL != paacd->hmmioDst)
    {
        mmioAscend(paacd->hmmioDst, &paacd->ckDst, 0);
        mmioAscend(paacd->hmmioDst, &paacd->ckDstRIFF, 0);

        mmioClose(paacd->hmmioDst, 0);
        paacd->hmmioDst = NULL;
    }

    //
    //
    //
    if (NULL != paacd->has)
    {
        pash = &paacd->ash;

        if (ACMSTREAMHEADER_STATUSF_PREPARED & pash->fdwStatus)
        {
            pash->cbSrcLength = paacd->cbSrcReadSize;
            pash->cbDstLength = paacd->cbDstBufSize;

            mmr = acmStreamUnprepareHeader(paacd->has, &paacd->ash, 0L);
            if (MMSYSERR_NOERROR != mmr)
            {
                MsgBox(MB_OK | MB_ICONEXCLAMATION, "acmStreamUnprepareHeader() failed. Error = %u", mmr);
				return FALSE;
            }
        }

        //
        //
        //
        acmStreamClose(paacd->has, 0L);
        paacd->has = NULL;

        if (NULL != paacd->had)
        {
            acmDriverClose(paacd->had, 0L);
            paacd->had = NULL;
        }
    }
    //
    //
    //
    //
    if (NULL != paacd->pbSrc)
    {
        GlobalFreePtr(paacd->pbSrc);
        paacd->pbSrc = NULL;
    }
    
    if (NULL != paacd->pbDst)
    {
        GlobalFreePtr(paacd->pbDst);
        paacd->pbDst = NULL;
    }

    return (TRUE);
}


BOOL CWaveformConverter::riffCopyChunk(HMMIO hmmioSrc, HMMIO hmmioDst, const LPMMCKINFO lpck)
{
    MMCKINFO    ck;
    HPSTR       hpBuf;

    //
    //
    //
    hpBuf = (HPSTR)GlobalAllocPtr(lpck->cksize);
    if (!hpBuf)
        return (FALSE);

    ck.ckid   = lpck->ckid;
    ck.cksize = lpck->cksize;
    if (mmioCreateChunk(hmmioDst, &ck, 0))
        goto rscc_Error;
        
    if (mmioRead(hmmioSrc, hpBuf, lpck->cksize) != (LONG)lpck->cksize)
        goto rscc_Error;

    if (mmioWrite(hmmioDst, hpBuf, lpck->cksize) != (LONG)lpck->cksize)
        goto rscc_Error;

    if (mmioAscend(hmmioDst, &ck, 0))
        goto rscc_Error;

    if (hpBuf)
        GlobalFreePtr(hpBuf);

    return (TRUE);

rscc_Error:

    if (hpBuf)
        GlobalFreePtr(hpBuf);

    return (FALSE);
} /* RIFFSupCopyChunk() */

BOOL CWaveformConverter::GetSourceInfo(PAACONVERTDESC paacd)
{
	MMCKINFO    ckRIFF;
	MMCKINFO    ck;
	WAVEIOCB	wio;
	WIOERR		werr = WIOERR_NOERROR;
	DWORD       dw;
    DWORD               nAvgBytesPerSec;
    DWORD               nBlockAlign;
	memset(&wio, 0, sizeof(wio));

    //
    //  first try to open the file, etc.. open the given file for reading
    //  using buffered I/O
    //
    HMMIO hmmio = mmioOpen((LPTSTR)paacd->szFilePathSrc, NULL, MMIO_READ | MMIO_ALLOCBUF);
    if (NULL == hmmio)
        goto wio_Open_Error;
    //
    //  locate a 'WAVE' form type...
    //
    ckRIFF.fccType = mmioFOURCC('W', 'A', 'V', 'E');
    if (mmioDescend(hmmio, &ckRIFF, NULL, MMIO_FINDRIFF))
        goto wio_Open_Error;

    //
    //  we found a WAVE chunk--now go through and get all subchunks that
    //  we know how to deal with...
    //
    wio.dwDataSamples = (DWORD)-1L;

    //
    //
    //
    while (MMSYSERR_NOERROR == mmioDescend(hmmio, &ck, &ckRIFF, 0))
    {
        //
        //  quickly check for corrupt RIFF file--don't ascend past end!
        //
        if ((ck.dwDataOffset + ck.cksize) > (ckRIFF.dwDataOffset + ckRIFF.cksize))
        {
/*            wsprintf(ach, TEXT("This wave file might be corrupt. The RIFF chunk.ckid '%.08lX' (data offset at %lu) specifies a cksize of %lu that extends beyond what the RIFF header cksize of %lu allows. Attempt to load?"),
                     ck.ckid, ck.dwDataOffset, ck.cksize, ckRIFF.cksize);
            u = MessageBox(NULL, ach, TEXT("wioFileOpen"),
                           MB_YESNO | MB_ICONEXCLAMATION | MB_TASKMODAL);
            if (IDNO == u)*/
            {
                werr = WIOERR_BADFILE;
                goto wio_Open_Error;
            }
        }

        switch (ck.ckid)
        {
            case mmioFOURCC('L', 'I', 'S', 'T'):
                if (ck.fccType == mmioFOURCC('I', 'N', 'F', 'O'))
                {
#if 0
                    if(lrt=riffReadINFO(hmmio, &ck, wio.pInfo))
                    {
                        lr=lrt;
                        goto wio_Open_Error;
                    }
#endif
                }
                break;
                
            case mmioFOURCC('D', 'I', 'S', 'P'):
#if 0
                riffReadDISP(hmmio, &ck, &(wio.pDisp));
#endif
                break;
                
            case mmioFOURCC('f', 'm', 't', ' '):
                //
                //  !?! another format chunk !?!
                //
                if (NULL != wio.pwfx)
                    break;

                //
                //  get size of the format chunk, allocate and lock memory
                //  for it. we always alloc a complete extended format header
                //  (even for PCM headers that do not have the cbSize field
                //  defined--we just set it to zero).
                //
                dw = ck.cksize;
                if (dw < sizeof(WAVEFORMATEX))
                    dw = sizeof(WAVEFORMATEX);

                wio.pwfx = (LPWAVEFORMATEX)GlobalAllocPtr(dw);
                if (NULL == wio.pwfx)
                {
                    werr = WIOERR_NOMEM;
                    goto wio_Open_Error;
                }

                //
                //  read the format chunk
                //
                werr = WIOERR_FILEERROR;
                dw = ck.cksize;
                if (mmioRead(hmmio, (HPSTR)wio.pwfx, dw) != (LONG)dw)
                    goto wio_Open_Error;
                break;


            case mmioFOURCC('d', 'a', 't', 'a'):
                //
                //  !?! multiple data chunks !?!
                //
                if (0L != wio.dwDataBytes)
                    break;

                //
                //  just hang on to the total length in bytes of this data
                //  chunk.. and the offset to the start of the data
                //
                wio.dwDataBytes  = ck.cksize;
                wio.dwDataOffset = ck.dwDataOffset;
                break;


            case mmioFOURCC('f', 'a', 'c', 't'):
                //
                //  !?! multiple fact chunks !?!
                //
                if (-1L != wio.dwDataSamples)
                    break;

                //
                //  read the first dword in the fact chunk--it's the only
                //  info we need (and is currently the only info defined for
                //  the fact chunk...)
                //
                //  if this fails, dwDataSamples will remain -1 so we will
                //  deal with it later...
                //
                mmioRead(hmmio, (HPSTR)&wio.dwDataSamples, sizeof(DWORD));
                break;
        }

        //
        //  step up to prepare for next chunk..
        //
        mmioAscend(hmmio, &ck, 0);
    }

    //
    //  if no fmt chunk was found, then die!
    //
    if (NULL == wio.pwfx)
    {
        werr = WIOERR_ERROR;
        goto wio_Open_Error;
    }

    //
    //  all wave files other than PCM are _REQUIRED_ to have a fact chunk
    //  telling the number of samples that are contained in the file. it
    //  is optional for PCM (and if not present, we compute it here).
    //
    //  if the file is not PCM and the fact chunk is not found, then fail!
    //
    if (-1L == wio.dwDataSamples)
    {
        if (WAVE_FORMAT_PCM == wio.pwfx->wFormatTag)
        {
            wio.dwDataSamples = wio.dwDataBytes / wio.pwfx->nBlockAlign;
        }
        else
        {
            //
            //  !!! HACK HACK HACK !!!
            //
            //  although this should be considered an invalid wave file, we
            //  will bring up a message box describing the error--hopefully
            //  people will start realizing that something is missing???
            //
            /*u = MessageBox(NULL, TEXT("This wave file does not have a 'fact' chunk and requires one! This is completely invalid and MUST be fixed! Attempt to load it anyway?"),
                            TEXT("wioFileOpen"), MB_YESNO | MB_ICONEXCLAMATION | MB_TASKMODAL);
            if (IDNO == u)*/
            {
                werr = WIOERR_BADFILE;
                goto wio_Open_Error;
            }

            //
            //  !!! need to hack stuff in here !!!
            //
            wio.dwDataSamples = 0L;
        }
    }

	// Copy the information to our conversion structure
	paacd->uBufferTimePerConvert = 1000;
    paacd->dwSrcSamples = wio.dwDataSamples;
    nAvgBytesPerSec     = wio.pwfx->nAvgBytesPerSec;
    nBlockAlign         = wio.pwfx->nBlockAlign;
    paacd->cbSrcReadSize = nAvgBytesPerSec - (nAvgBytesPerSec % nBlockAlign);
	paacd->pwfxSrc		= wio.pwfx;
	paacd->cbSrcData    = wio.dwDataBytes;
    paacd->dwTimeShortest     = (DWORD)-1L;
    paacd->dwShortestConvert  = (DWORD)-1L;
	paacd->dwLongestConvert   = (DWORD)-1L;
	mmioClose(hmmio, 0);
	return TRUE;

    //
    //  return error (after minor cleanup)
    //
wio_Open_Error:

	/*
    if (wio.pInfo)
        riffFreeINFO(&(lwio.pInfo));
    
    if (wio.pDisp)
        riffFreeDISP(&(lwio.pDisp));
#endif

    if (NULL != wio.pwfx)
        GlobalFreePtr(wio.pwfx);*/
	mmioClose(hmmio, 0);
    return FALSE;
}
