// (r.gonet 12/03/2012) - PLID 54117 - Added. Most of the method documentation is in the .cpp file.

#ifndef HL7_CLIENT_PRACTICE_H
#define HL7_CLIENT_PRACTICE_H

#pragma once

#include "HL7Client.h"
#include "WindowlessTimer.h"
#include <afxmt.h>

// (r.gonet 12/11/2012) - PLID 54117 - CHL7Client_Practice is a middleman between Practice and NxServer
//  when sending HL7 messages to NxServer to send on to the link or when requesting NxServer
//  generate a message for a record and send it to the Link (exporting). NxServer will send
//  back responses to these requests which gives the status of the request and some other
//  useful information.
//  
//  CHL7Client_Practice handles the socket communication and is responsible for timeouts and getting 
//  acknowledgements. This class
//
//  This class can operate in two modes: asynchronous and synchronous. When in asynchronous
//  mode, CHL7Client_Practice does not block while waiting for a response from NxServer. Sending still 
//  blocks but there are bigger issues if we cannot connect to NxServer in Practice. Timeouts
//  of responses or delays in responses are more likely since there is processing to be done
//  by NxServer before a response is sent. There is coupling with CMainFrame in that it will 
//  get the packet and pass it along to this class. The GetNxServerSocket() socket is used here.
//  The asynchronous model looks like this:
//  
//  Caller -> CHL7Client_Practice::SendMessageToHL7() -> Message is sent, begin waiting on response -> returns to caller with Pending status
//  ... -> CMainFrame gets the response packet -> Passes packet to CHL7Client_Practice::OnHL7ExportResponsePacket() -> Routes to a callback
//  (or for a timeout)
//  ... -> Timeout -> CHL7Client_Practice calls the callback.
//
//  Synchronous mode blocks the calling thread while waiting for a response from NxServer.
//  In synchronous mode, CHL7Client_Practice creates a new socket. The model looks like this:
//
//  Caller -> CHL7Client_Practice::SendMessageToHL7() -> Sends message -> Block for response -> Get response ->
//  -> See if we need an ACK -> Block for ACK -> Get ACK response -> Return to the caller with a non-intermediate status
//
//  Errors encountered in sending or receiving will be returned in the HL7ResponsePtr object.
//
// (z.manning 2013-05-20 10:40) - PLID 56777 - Renamed to be Practice specific
class CHL7Client_Practice : public CHL7Client
{
public:
	// (r.gonet 12/03/2012) - PLID 54117
	CHL7Client_Practice(bool bAsynchronous = false, boost::function<void (CHL7Client *pClient, HL7ResponsePtr)> fnAsyncCallback = 0);
	
	// (r.gonet 12/03/2012) - PLID 54117
	virtual bool EnsureSocket();
};

#endif