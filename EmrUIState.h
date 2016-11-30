#pragma once

// (a.walling 2012-10-01 09:15) - PLID 52119 - Maintain UI state somewhere accessible a bit easier outside of the frame wnd

#include <map>
#include <boost/optional.hpp>
#include <boost/container/flat_map.hpp>

class CEMN;

namespace Emr
{

using boost::optional;

struct UIState
{
	UIState()
	{}

	optional<CString> selectedEmnMergeTemplate;
	optional<CString> defaultEmnMergeTemplate;
};

typedef boost::container::flat_map<CEMN*, UIState> UIStateMap;

}
