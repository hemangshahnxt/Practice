#ifndef NEXTECH_PRACTICE_EMRITEMENTRYDLGSPELLCHECKHANDLER_H
#define NEXTECH_PRACTICE_EMRITEMENTRYDLGSPELLCHECKHANDLER_H

#pragma once

#include "SpellExUtils.h"

// (b.cardillo 2007-01-11 11:24) - PLID 17929 - Handler for spell checking an EMR Item Entry dialog
class CEmrItemEntryDlgSpellCheckHandler : public CSpellCheckHandler
{
public:
	CEmrItemEntryDlgSpellCheckHandler(class CEmrItemEntryDlg *peiedlgToCheck);
public:
	// Determine the text to be spell checked for the current control
	virtual void OnPreStart(OUT BOOL &bAllowStart, OUT CString &strTextToSpellCheck);
	// Handle any mistakes as appropriate
	virtual void OnMistake(IN const ESpellingMistakeType esmtMistakeType, IN const long nStart, IN const long nLen, IN const CSize &szDlgSize, IN OUT CPoint &ptDlgPos, IN OUT EMistakeResponse &emrResponse, OPTIONAL OUT CString &strChangeToWord);
	// Reflect the correction of any mistake on screen
	virtual void OnReplaceText(IN const long nStart, IN const long nLen, IN const CString &strReplaceWithWord);
	// Move to the next control if there is one
	virtual void OnFinished(IN const EFinishedReason efrReason, OUT BOOL &bStartAgain);

protected:
	CEmrItemEntryDlg *m_pDlgToCheck;
	
	// This enum uses the ++ operator, so it's critical that the values are always contiguous and always end with ctc_NO_MORE_CONTROLS
	enum EControlsToCheck {
		ctcName,
		ctcSentence,
		//TES 2/21/2010 - PLID 37464 - Added the "Smart Stamps" sentence format
		// (z.manning 2010-07-26 15:53) - PLID 39848 - Now deprecated
		//ctcSmartStampSentence,
		
		ctcType_Narrative_DefaultText,
		ctcType_Text_DefaultText,
		ctcType_TableOrSelectList_DataList, // This also includes the "long form" field when it's a single- or multi-select list
		ctcType_Table_ColumnList,

		ctc_NO_MORE_CONTROLS,
	};

	// These variables keep track of what we're currently spell-checking
	EControlsToCheck m_ectcCurrentlyCheckingControl;
	//TES 3/14/2011 - PLID 42784 - Converted to datalist2
	NXDATALIST2Lib::IRowSettingsPtr m_pCurrentlyCheckingRowInList;
	BOOL m_bCurrentlyCheckingDataElementLongForm;
};



#endif // NEXTECH_PRACTICE_EMRITEMENTRYDLGSPELLCHECKHANDLER_H