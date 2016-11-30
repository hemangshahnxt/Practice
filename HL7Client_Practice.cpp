// (r.gonet 12/03/2012) - PLID 54117 - Added.

#include "stdafx.h"
#include "HL7Client_Practice.h" // (z.manning 2013-05-20 11:07) - PLID 56777 - Renamed
#include "HL7Utils.h"
#include <afxmt.h>

// (r.gonet 12/03/2012) - PLID 54117 - Initializes a new instance of the CHL7Client class.
// - bAsynchronous: If true, then when sending messages from this client, execution of the calling thread will not block when waiting for a response. Sending blocks normally.
//                  If false, then when sending messages from this client, execution of the calling thread will block until a response is received or a timeout occurs.
// - fnAsyncCallback: A callback function that is called when a response is received asynchronously. 
//                    If bAsynchronous is true, then this parameter should be non-zero (otherwise responses
//                    will never get handled). If bAsynchronous is false, then this parameter is ignored.
CHL7Client_Practice::CHL7Client_Practice(bool bAsynchronous/*= false*/, boost::function<void (CHL7Client *pClient, HL7ResponsePtr)> fnAsyncCallback/*= 0*/)
	: CHL7Client(GetSubRegistryKey(), GetAPILoginToken(), GetCurrentUserID(), GetCurrentUserName(), GetCurrentLocationID(), GetCurrentLocationName(), bAsynchronous, fnAsyncCallback)
{
}

// (r.gonet 12/11/2012) - PLID 54117 - Ensures that the socket handle we have in m_hClient is valid and 
//  initializes it if not. Returns true upon success and false upon failure. 
bool CHL7Client_Practice::EnsureSocket()
{
	if(m_bAsynchronous)
	{
		m_hClient = GetMainFrame()->GetNxServerSocket();
		// (r.gonet 12/11/2012) - PLID 54117 - If we aren't currently connected to NxServer, then connect.
		if (NULL == m_hClient) {
			// (r.gonet 12/11/2012) - PLID 54117. This could still fail, in which case the caller should
			//  error out.
			GetMainFrame()->InitNxServer();
			m_hClient = GetMainFrame()->GetNxServerSocket();
		}
		if(NULL == m_hClient) {
			return false;
		} else {
			return true;
		}
	}
	else {
		return CHL7Client::EnsureSynchronousSocket();
	}
}