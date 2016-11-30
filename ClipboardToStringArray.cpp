#include "stdafx.h"
#include "ClipboardToStringArray.h"
#include "ClipboardUtils.h"

// (d.thompson 2012-10-16) - PLID 53184
CClipboardToStringArray::CClipboardToStringArray(long nClipboardParsingOptions)
{
	//Start with a fresh list
	m_aryItems.RemoveAll();

	//Poll the clipboard and conver it as necessary
	GatherClipboardData(nClipboardParsingOptions);
}

CClipboardToStringArray::~CClipboardToStringArray(void)
{
}

void CClipboardToStringArray::GatherClipboardData(long nClipboardParsingOptions)
{
	Clipboard clipboard;
	clipboard.Open(NULL);
	//Should return nothing if it's not text in the clipboard
	char *szClip = clipboard.GetText();
	CString strClipText(szClip);
	//The clipboard returns a newly allocated char* array, so we need to clear it.  We've copied the data to strClipText now.
	delete [] szClip;
	if(strClipText.IsEmpty()) {
		return;
	}

	//This text could be anything.  We're parsing anything based on newlines, but to be safest we're going to
	//	look simply for \n (not \r) and trim.  We'll also ignore any blank lines entirely, and we'll trim the
	//	end of any text that you enter.
	CStringArray aryItems;
	SplitString(strClipText, "\n", &aryItems);

	EvaluateDataWithOptions(&aryItems, nClipboardParsingOptions);
}

void CClipboardToStringArray::EvaluateDataWithOptions(CStringArray *paryItems, long nClipboardParsingOptions)
{
	//Simply iterate through the entire given list, applying the selected options to each element, if the option
	//	is enabled.  Copy any approved values into the member output list.
	for(int i = 0; i < paryItems->GetSize(); i++)
	{
		bool bAddToFinalArray = true;
		CString strData = paryItems->GetAt(i);

		//Apply options here.  Please assess the proper order of any future options.  For example, applying cpoTrimRightText then 
		//	cpoRemoveBlankLines will yield different results than applying them in reverse.
		if(nClipboardParsingOptions & cpoTrimRightText)
		{
			strData.TrimRight();
		}

		if(nClipboardParsingOptions & cpoSanitizePrintable)
		{
			strData = SanitizePastedData(strData);
		}

		//This should happen after anything that might modify the data.  Please make sure to move it below any future options
		//	that do the same.
		if(nClipboardParsingOptions & cpoRemoveBlankLines)
		{
			if(strData.IsEmpty()) {
				bAddToFinalArray = false;
			}
		}


		//... Implement future options here



		//All options applied, if approved, copy it to the final array
		if(bAddToFinalArray) {
			m_aryItems.Add(strData);
		}
	}
}

//This will make sure that the given data does not contain any generally unprinted
//	characters, like newlines, null characters, tabs, etc.
CString CClipboardToStringArray::SanitizePastedData(IN CString strData)
{
	CString strOutput;
	for(int i = 0; i < strData.GetLength(); i++) {
		TCHAR c = strData.GetAt(i);
		if( (unsigned long)c >= 32)
		{
			strOutput += c;
		}
	}

	return strOutput;
}
