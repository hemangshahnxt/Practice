#pragma once

namespace TopazSigPad
{
	MSINKAUTLib::IInkDispPtr GetSignatureInk(CWnd* pParentWnd);
	BOOL IsTabletConnected(CWnd* pParentWnd, long nComPort);

	long GetPort();
}

