/*++

Copyright (c) 1999 Microsoft Corporation.  All Rights Reserved.

Module Name:
    callnot.cpp

Abstract:

    Implementation of the ITTAPIEventNotification interface.

    This is an outgoing interface that is defined by TAPI 3.0.  This
    is basically a callback function that TAPI 3.0 calls to inform
    the application of events related to calls (on a specific address)

    Please refer to COM documentation for information on outgoing
    interfaces.

    An application must implement and register this interface in order
    to receive calls and events related to calls

--*/


#include "stdafx.h"
#include <tapi3.h>
#include <control.h>
#include <strmif.h>
#include "globalutils.h"

bool m_bIncomingCall;
CString m_strIncomingCallNum;
#include "callerid.h"
#include "resource.h"
#include "nxmessagedef.h"

//if we're in show-mode, this is defined elsewhere, if not, it needs to be here so it will compile
//note:  none of this code is ever called in release mode, it is only needed for compiling purposes
extern ITBasicCallControl * gpCall;


void
SetStatusMessage(
                 LPWSTR pszMessage
                );

void
EnableButton(
             int ID
            );
void
DisableButton(
              int ID
             );

enum {
    VWP_LEFT = 20,
    VWP_TOP = 100
};

class CallerIDInfo {
public:
	ITCallInfo* pCall;
	HWND hwnd;
	CallerIDInfo(ITCallInfo* pNewCall, HWND NewHWnd){ pNewCall->AddRef();  pCall = pNewCall;  hwnd = NewHWnd;  }
	~CallerIDInfo() {  pCall->Release();  }

};


//thread function to return the call string
UINT GetCallingNumber(LPVOID pParam)
{
	CallerIDInfo* pInfo = (CallerIDInfo*)pParam;
	ITCallInfo* pCall = pInfo->pCall;
	HWND hWnd = pInfo->hwnd;

	delete pInfo;

	BSTR strCallNumber;
	
	//try to get the Number first
	if(pCall->get_CallInfoString(CIS_CALLERIDNUMBER, &strCallNumber) == TAPI_E_INVALCALLSTATE)
		MessageBox(hWnd, "Invalid Call State - Cannot get the caller ID information", "Error", MB_OK);

	//if we don't have a number, try for 2 seconds to get it, then quit
	for(int i = 50; i < 2000 && (CString)strCallNumber == ""; i += 50){
		Sleep(50);
		if(pCall->get_CallInfoString(CIS_CALLERIDNUMBER, &strCallNumber) == TAPI_E_INVALCALLSTATE)
			MessageBox(hWnd, "Invalid Call State - Cannot get the caller ID information", "Error", MB_OK);
	}

	//the BSTR is destroyed in the message handler because it is not used after this point in this thread
	::PostMessage(hWnd, NXM_CALLERIDNUMBER, (WPARAM)IDC_NUMBER, (LPARAM)strCallNumber);	

	return 0;

}

///////////////////////////////////////////////////////////////////
// HostWindow
//
// This method takes a pVideoWindow interface pointer and puts
//  the window in our dialog.
//
// The location depends on the iWhere parameter.
//
///////////////////////////////////////////////////////////////////

void
HostWindow(
           IVideoWindow *pVideoWindow,
           int iWhere
          )
{
    
    pVideoWindow->put_WindowStyle( WS_CHILDWINDOW | WS_BORDER );

    long lWidth, lHeight;
    pVideoWindow->get_Width( &lWidth );
    pVideoWindow->get_Height( &lHeight );

    pVideoWindow->SetWindowPosition( VWP_LEFT + (iWhere * (VWP_LEFT + lWidth)),
                                     VWP_TOP,
                                     lWidth,
                                     lHeight );

    pVideoWindow->put_Visible( VARIANT_TRUE );
}

///////////////////////////////////////////////////////////////////
// GetVideoRenderTerminalFromStreamEvent
//
// Find out if this stream has a video render terminal. If not,
// we don't need to do anything with this stream. Also note
// if this is the video capture stream or the video render
// stream.
//
///////////////////////////////////////////////////////////////////

HRESULT
GetVideoRenderTerminalFromStreamEvent(
                                      ITCallMediaEvent  * pCallMediaEvent,
                                      ITTerminal       ** ppTerminal,
                                      BOOL              * pfRenderStream
                                     )
{
    HRESULT hr;

    //
    // Get the stream for this event.
    //

    ITStream * pStream;

    hr = pCallMediaEvent->get_Stream( &pStream );
    
    if ( FAILED(hr) || (pStream == NULL) )
    {
        return E_FAIL;
    }

    //
    // Find out if this is a video stream. If not, fail --
    // we don't care about this stream.
    //

    long lMediaType;

    hr = pStream->get_MediaType(&lMediaType);
    
    if ( FAILED(hr) )
    {
        return hr;
    }

    if ( lMediaType != TAPIMEDIATYPE_VIDEO )
    {
        return E_FAIL;
    }

    //
    // Find out the direction of this stream. We will return this info to the
    // caller.
    //

    TERMINAL_DIRECTION td;

    hr = pStream->get_Direction(&td);
    
    if ( FAILED(hr) )
    {
        return hr;
    }

    *pfRenderStream = (td == TD_RENDER);

    //
    // Enumerate the terminals on this stream.
    //

    IEnumTerminal * pEnumTerminal;

    hr = pStream->EnumerateTerminals( &pEnumTerminal );
    
    pStream->Release();
    
    if ( FAILED(hr) )
    {
        return hr;
    }

    //
    // Search for the first video render terminal and return it. If there are
    // none, fail -- we don't care about this stream.
    //

    while ( S_OK == pEnumTerminal->Next(1, ppTerminal, NULL) )
    {
        if ( SUCCEEDED( (*ppTerminal)->get_Direction(&td) ) )
        {
            if ( td == TD_RENDER )
            {
                // found it; done with enum, but keep ref on terminal
                pEnumTerminal->Release();

                return S_OK;
            }
        }

        // this is not the right one
        (*ppTerminal)->Release();
    }

    pEnumTerminal->Release();

    return E_FAIL; // no terminal found
}
                
///////////////////////////////////////////////////////////////////
// CallEventNotification
//
// The only method in the ITCallEventNotification interface.  This gets
// called by TAPI 3.0 when there is a call event to report. This just
// posts the message to our UI thread, so that we do as little as
// possible on TAPI's callback thread.
//
///////////////////////////////////////////////////////////////////

HRESULT
STDMETHODCALLTYPE
CTAPIEventNotification::Event(
                              TAPI_EVENT TapiEvent,
                              IDispatch * pEvent
                             )
{
    //
    // Addref the event so it doesn't go away.
    //

    pEvent->AddRef();

    //
    // Post a message to our own UI thread.
    //

	

    PostMessage(
                GetMainFrame()->m_hWnd, 
                WM_PRIVATETAPIEVENT,
                (WPARAM) TapiEvent,
                (LPARAM) pEvent
               );

    return S_OK;
}

ITCallInfo *GetCallObjectFromEvent(TAPI_EVENT TapiEvent, IDispatch * pEvent)
{
	switch (TapiEvent) {
	case TE_CALLNOTIFICATION:
		{
			ITCallNotificationEvent *pCNE;
			HRESULT hr = pEvent->QueryInterface(IID_ITCallStateEvent, (void **)&pCNE);
			if (hr == S_OK) {
				ITCallInfo *pAns;
				hr = pCNE->get_Call(&pAns);
				if (hr == S_OK) {
					// Got it, release our objects and return the result
					pCNE->Release();
					return pAns;
				} else {
					pCNE->Release();
					_com_raise_error(hr);
					throw;
				}
			} else {
				_com_raise_error(hr);
				throw;
			}
		}
		break;
	case TE_CALLINFOCHANGE:
		{
			ITCallInfoChangeEvent *pCICE;
			HRESULT hr = pEvent->QueryInterface(IID_ITCallInfoChangeEvent, (void **)&pCICE);
			if (hr == S_OK) {
				ITCallInfo *pAns;
				hr = pCICE->get_Call(&pAns);
				if (hr == S_OK) {
					// Got it, release our objects and return the result
					pCICE->Release();
					return pAns;
				} else {
					pCICE->Release();
					_com_raise_error(hr);
					throw;
				}
			} else {
				_com_raise_error(hr);
				throw;
			}
		}
	case TE_CALLSTATE:
		{
			ITCallStateEvent *pCSE;
			HRESULT hr = pEvent->QueryInterface(IID_ITCallStateEvent, (void **)&pCSE);
			if (hr == S_OK) {
				ITCallInfo *pAns;
				hr = pCSE->get_Call(&pAns);
				if (hr == S_OK) {
					// Got it, release our objects and return the result
					pCSE->Release();
					return pAns;
				} else {
					pCSE->Release();
					_com_raise_error(hr);
					throw;
				}
			} else {
				_com_raise_error(hr);
				throw;
			}
		}
		break;
	default:
		ASSERT(FALSE);
		return NULL;
		break;
	}
}
///////////////////////////////////////////////////////////////////
// OnTapiEvent
//
// This is the real event handler, called on our UI thread when
// the WM_PRIVATETAPIEVENT message is received
//
///////////////////////////////////////////////////////////////////

HRESULT
OnTapiEvent(
            TAPI_EVENT TapiEvent,
            IDispatch * pEvent
           )
{
    HRESULT hr;

    switch ( TapiEvent )
    {
		/*
        case TE_CALLNOTIFICATION:
        {
            //
            // TE_CALLNOTIFICATION means that the application is being notified
            // of a new call.
            //
            // Note that we don't answer to call at this point.  The application
            // should wait for a CS_OFFERING CallState message before answering
            // the call.
            //

            //enable the button by setting this global variable
			m_bIncomingCall = TRUE;
			
			ITCallNotificationEvent         * pNotify;
            
            hr = pEvent->QueryInterface( IID_ITCallNotificationEvent, (void **)&pNotify );

            if (S_OK != hr)
            {
                AfxMessageBox("Incoming call, but failed to get the interface");
            }
            else
            {
                CALL_PRIVILEGE          cp;
                ITCallInfo *            pCall;

                //
                // get the call
                //

                hr = pNotify->get_Call( &pCall );

                pNotify->Release();

                if ( SUCCEEDED(hr) )
                {
                    //
                    // check to see if we own the call
                    //

                    hr = pCall->get_Privilege( &cp );

                    if ( FAILED(hr) || (CP_OWNER != cp) )
                    {
                        //
                        // just ignore it if we don't own it
                        //

                        pCall->Release();

                        pEvent->Release(); // we addrefed it CTAPIEventNotification::Event()

                        return S_OK;
                    }

                    //
                    // Get the ITBasicCallControl interface and save it in our
                    // global variable.
                    //

                    hr = pCall->QueryInterface( IID_ITBasicCallControl,
                                                (void**)&gpCall );

					//the thread posts a message telling the main frame to update the global caller number string with the number received
					//the thread will take care of deleting the CallerIDInfo object as well (since it is called here and only here)
					CWinThread* pThread = AfxBeginThread(GetCallingNumber, new CallerIDInfo(pCall, GetMainFrame()->m_hWnd));

					pCall->Release();

                }
            }

            break;
        }

        case TE_CALLSTATE:
        {

//			AfxMessageBox("Callstate");

            // TE_CALLSTATE is a call state event.  pEvent is
            // an ITCallStateEvent

            CALL_STATE         cs;
            ITCallStateEvent * pCallStateEvent;

            // Get the interface
            hr = pEvent->QueryInterface( IID_ITCallStateEvent, (void **)&pCallStateEvent );

            if ( FAILED(hr) )
            {
                break;
            }

            // get the CallState that we are being notified of.
            hr = pCallStateEvent->get_State( &cs );

            // Release the interface
            pCallStateEvent->Release();

            if ( FAILED(hr) )
            {
                break;
            }

            // if it's offering, update our UI
            if (CS_OFFERING == cs)
            {


            }
            else if (CS_DISCONNECTED == cs)
            {
				//disable the call button
				m_bIncomingCall = false;
				
				//release the call so we can receive a new one
		        gpCall->Release();
		        gpCall = NULL;

//that doesn't seem to always work more than once or twice, so let's try this:
/*
				GetMainFrame()->ShutdownTapi();

				if(GetMainFrame()->InitializeTapi() != S_OK)
				{
					AfxMessageBox("Problems resetting CallerID functionality.  Please attempt to restart it manually");
					SetRemotePropertyInt("UseCallerID", 0);
				}
* /

            }
            else if (CS_CONNECTED == cs)
            {
                // nothing to do -- we handle connection synchronously
            }
                
            break;
        }

        case TE_CALLMEDIA:
        {

//			AfxMessageBox("CallMedia");
            // TE_CALLMEDIA is a media event.  pEvent is
            // an ITCallMediaEvent

            CALL_MEDIA_EVENT    cme;
            ITCallMediaEvent  * pCallMediaEvent;

            // Get the interface
            hr = pEvent->QueryInterface( IID_ITCallMediaEvent, (void **)&pCallMediaEvent );

            if ( FAILED(hr) )
            {
                break;
            }

            // get the CALL_MEDIA_EVENT that we are being notified of.
            hr = pCallMediaEvent->get_Event( &cme );

            if ( SUCCEEDED(hr) )
            {
                switch ( cme )
                {
                case CME_STREAM_NOT_USED:
                case CME_STREAM_INACTIVE:
                case CME_NEW_STREAM:
                    break;

                case CME_STREAM_FAIL:
                    AfxMessageBox("Call media event: stream failed");
                    break;
    
                case CME_TERMINAL_FAIL:
                    AfxMessageBox("Call media event: terminal failed");
                    break;

                case CME_STREAM_ACTIVE:
                {
                    //
                    // Find out if this stream has a video render terminal. If not,
                    // we don't need to do anything with this stream. Also note
                    // if this is the video capture stream or the video render
                    // stream.
                    //

                    ITTerminal * pTerminal;
                    BOOL         fRenderStream;

                    hr = GetVideoRenderTerminalFromStreamEvent(
                                                               pCallMediaEvent,
                                                               &pTerminal,
                                                               &fRenderStream
                                                              );
                
                    if ( SUCCEEDED(hr) )
                    {
                        // Get the video window interface for the terminal
                        IVideoWindow *pVideoWindow = NULL;

                        hr = pTerminal->QueryInterface( IID_IVideoWindow, (void**)&pVideoWindow );
                
                        pTerminal->Release();
            
                        if ( SUCCEEDED(hr) )
                        {
                            // Put this window in our dialog
                            HostWindow(pVideoWindow, fRenderStream);
            
                            pVideoWindow->Release();
                        }
                    }

                    break;
                }
            
                default:
                    break;
                }
            }

            // We no longer need this interface.
            pCallMediaEvent->Release();

            break;
        }
		*/

		// For "audio" devices we get caller id info here
        case TE_CALLNOTIFICATION:
		case TE_CALLINFOCHANGE:
        case TE_CALLSTATE:
			try {
				// Get the callinfo object
				ITCallInfo *pCall = GetCallObjectFromEvent(TapiEvent, pEvent);
				if (pCall) {
					// Get the string
					BSTR bstr;
					hr = pCall->get_CallInfoString(CIS_CALLERIDNUMBER, &bstr);
					if (hr == S_OK) {
						// We're done with the COM objects
						pCall->Release();
						// Convert the bstr to a CString
						CString strCallerId = (LPCTSTR)_bstr_t(bstr, false);
						CMainFrame *pMainFrame = GetMainFrame();
						if (pMainFrame->GetSafeHwnd()) {
							pMainFrame->OnReceivedCallerIdNumber(strCallerId);
						} else {
							ThrowNxException("Mainframe does not exist!");
						}
					} else {
						pCall->Release();
						_com_raise_error(hr);
					}
				}
			} catch (_com_error e) {
				Log("OnTapiEvent:TE_CALLINFOCHANGE COM ERROR %li: %s (%s)", e.Error(), e.ErrorMessage(), (LPCTSTR)e.Description());
			} catch (CException *e) {
				CString strErrMsg;
				e->GetErrorMessage(strErrMsg.GetBuffer(4096), 4095);
				strErrMsg.ReleaseBuffer();
				Log("OnTapiEvent:TE_CALLINFOCHANGE CException: %s", strErrMsg);
			} catch (...) {
				Log("OnTapiEvent:TE_CALLINFOCHANGE unexpected low level exception");
			}
			break;

        default:
            break;
    }

    pEvent->Release(); // we addrefed it CTAPIEventNotification::Event()
    
    return S_OK;
}

