#pragma once

#include "NxProtocolRoot.h"

// (a.walling 2012-09-04 12:10) - PLID 52439 - Open up stamp image as an IStream

namespace Nx
{
namespace Protocol
{
namespace Emr
{

IStreamPtr OpenStream(const CString& path, const CString& extra, IInternetProtocolSink* pSink);



} // namespace Emr
} // namespace Protocol
} // namespace Nx
