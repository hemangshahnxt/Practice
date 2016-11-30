#pragma once

#include <NxNetworkLib\LicenseSocket.h>

// (z.manning 2015-05-19 16:48) - PLID 65971 - Created. Has Practice-specific license functionality as
// the core license code was moved to NxNetworkLib
class CPracticeLicenseSocket :
	public CLicenseSocket
{
public:
	CPracticeLicenseSocket(CPracticeLicense* pParent);

	//This function will try to reconnect the socket after it's been disconnected.  Should be called ONLY when disconnected.
	virtual bool TryToReconnect();
	virtual LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};
