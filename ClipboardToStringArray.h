#pragma once

/**************************************************
	(d.thompson 2012-10-16) - PLID 53184
	This class reads the existing clipboard when the
class is instantiated, parses the contents by \n
newline (not \r\n), and returns a CStringArray of all the 
values found in the clipboard.

	If the clipboard is not text or has no valid text, 
an empty array will be returned.

	Options exist (see below) to control how the values are
parsed from the clipboard.

	Final output is delivered via the m_aryItems member

**************************************************/

enum CClipboardParsingOptions {
	//If this option is enabled, each row data entry will be trimmed
	//	from the right;
	cpoTrimRightText		= 0x0001,

	//If this option is enabled, the output will be sanitized to only 
	//	printable ASCII characters.  Currently that means 32 and higher.
	//	This strips things like tabs, etc.
	cpoSanitizePrintable	= 0x0002,

	//If this option is enabled, all blank rows will be stripped from
	//	the output set.  This is checked after trimming the text option
	//	above.
	cpoRemoveBlankLines		= 0x0004,


	//... add more options as needed here



	//And for ease of use, this will just turn on everything (increase this if we add more than 0xFFFF options)
	cpoAll					= 0xFFFF,
};

class CClipboardToStringArray
{
public:
	CClipboardToStringArray(long nClipboardParsingOptions);
	~CClipboardToStringArray(void);

	CStringArray m_aryItems;

protected:
	void GatherClipboardData(long nClipboardParsingOptions);
	void EvaluateDataWithOptions(CStringArray *paryItems, long nClipboardParsingOptions);
	CString SanitizePastedData(IN CString strData);
};
