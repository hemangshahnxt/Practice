#include "stdafx.h"
#include "EmrItemEntryDlgSpellCheckHandler.h"
#include "SpellExUtils.h"
#include "EmrItemEntryDlg.h"

// (b.cardillo 2007-01-11 11:24) - PLID 17929 - Implementation of the handler for spell checking an EMR Item Entry dialog

CEmrItemEntryDlgSpellCheckHandler::CEmrItemEntryDlgSpellCheckHandler(CEmrItemEntryDlg *pDlgToCheck)
{
	// Remember the control we're checking
	m_pDlgToCheck = pDlgToCheck;
	// Start at the begining, the name control
	m_ectcCurrentlyCheckingControl = ctcName;
	// Assume when we hit a datalist we want to start with the first (0th) row
	//TES 3/14/2011 - PLID 42784 - Converted to datalist2
	m_pCurrentlyCheckingRowInList = m_pDlgToCheck->m_pdlDataElementList->GetFirstRow();
	m_bCurrentlyCheckingDataElementLongForm = FALSE;
}

void CEmrItemEntryDlgSpellCheckHandler::OnPreStart(OUT BOOL &bAllowStart, OUT CString &strTextToSpellCheck)
{
	// Our approach in this class is to attempt to spell check EVERYTHING on the item entry screen.  But 
	// obviously that's no good, because if we're a narrative, then the controls for table entry, multi-
	// select list entry, etc., won't be on screen, so we mustn't spell-check those.  This code checks to 
	// be sure that what we're attempting to spell-check we are actually able to.  If not, then it short 
	// circuits the remainder of the function after setting bAllowStart to FALSE.
	{
		switch (m_ectcCurrentlyCheckingControl) {
		case ctcType_Narrative_DefaultText: 
			if (!m_pDlgToCheck->IsDlgButtonChecked(IDC_RADIO_NARRATIVE)) {
				// It's not narrative type so this control is not on screen so not currently editable
				bAllowStart = FALSE;
				return;
			}
			break;
		case ctcType_Text_DefaultText:
			if (!m_pDlgToCheck->IsDlgButtonChecked(IDC_RADIO_TEXT_SELECT)) {
				// It's not text type so this control is not on screen so not currently editable
				bAllowStart = FALSE;
				return;
			}
			break;
		case ctcType_TableOrSelectList_DataList:
			if (!m_pDlgToCheck->IsDlgButtonChecked(IDC_RADIO_LIST_SELECT) && !m_pDlgToCheck->IsDlgButtonChecked(IDC_RADIO_LIST_MULTISELECT) && !m_pDlgToCheck->IsDlgButtonChecked(IDC_RADIO_TABLE)) {
				// It's not single-select type, multi-select type, or table type, so this control is not on screen so not currently editable
				bAllowStart = FALSE;
				return;
			}
			break;
		case ctcType_Table_ColumnList:
			if (!m_pDlgToCheck->IsDlgButtonChecked(IDC_RADIO_TABLE)) {
				// It's not table type so this control is not on screen so not currently editable
				bAllowStart = FALSE;
				return;
			}
			break;
		}
	}



	// Depending on what control we're checking, we either get the text of the whole 
	// control, or find the next sub-entity within the control and get the text of that.
	switch (m_ectcCurrentlyCheckingControl) {
	case ctcName:
		// Simple, just get the text out of the edit box
		m_pDlgToCheck->GetDlgItemText(IDC_ITEM_NAME, strTextToSpellCheck);
		break;
	case ctcSentence:
		// Simple, just get the text out of the edit box
		m_pDlgToCheck->GetDlgItemText(IDC_SENTENCE, strTextToSpellCheck);
		break;
	case ctcType_Narrative_DefaultText:
		{
			// This is tricky and relies on internal implementation details of the NxRichTextEditor; I've 
			// added comments there to reference this code so that in case it needs to be changed someday 
			// this code can be repaired as well.
			// NOTE: Our external spell-checking of the narrative rests heavily on the assumption that the 
			// when editing the narrative EMR Item you CANNOT add special fields.  It can be rich text, it 
			// just can't have anything like "he/she" or "patient age" or whatever in it.  Right now the 
			// EMR Item Entry dialog is indeed designed such that you can't enter such fields, so it works.
			CRichEditCtrl *pre = (CRichEditCtrl *)m_pDlgToCheck->GetDlgItem(IDC_NARRATIVE_DEFAULT_TEXT)->GetDlgItem(23509);
			pre->GetWindowText(strTextToSpellCheck);
		}
		break;
	case ctcType_Text_DefaultText:
		// Simple, just get the text out of the edit box
		m_pDlgToCheck->GetDlgItemText(IDC_DEFAULT_TEXT, strTextToSpellCheck);
		break;
	case ctcType_TableOrSelectList_DataList:
		if (m_pCurrentlyCheckingRowInList != NULL) {
			if (m_bCurrentlyCheckingDataElementLongForm) {
				// Get the long form for the current data element
				long nArrayIndex = m_pDlgToCheck->GetCurDataElementArrayIndex(m_pCurrentlyCheckingRowInList->GetValue(CEmrItemEntryDlg::delcSortOrder), FALSE);
				CEmrInfoDataElement *peide = m_pDlgToCheck->m_aryCurDataElements.GetAt(nArrayIndex);
				strTextToSpellCheck = peide->m_strLongForm;
			} else {
				// Get the current string value out of the list
				strTextToSpellCheck = AsString(m_pCurrentlyCheckingRowInList->GetValue(CEmrItemEntryDlg::delcData));
			}
		} else {
			// We ran out of rows, so can't start (our OnFinished() will discover that we're past the end, 
			// and it will move to the next control)
			bAllowStart = FALSE;
		}
		break;
	case ctcType_Table_ColumnList:
		if (m_pCurrentlyCheckingRowInList != NULL) {
			// Get the current string value out of the list
			strTextToSpellCheck = AsString(m_pCurrentlyCheckingRowInList->GetValue(CEmrItemEntryDlg::cdelcData));
		} else {
			// We ran out of rows, so can't start (our OnFinished() will discover that we're past the end, 
			// and it will move to the next control)
			bAllowStart = FALSE;
		}
		break;
	default:
		ASSERT(FALSE);
		break;
	}
}

void CEmrItemEntryDlgSpellCheckHandler::OnMistake(IN const ESpellingMistakeType esmtMistakeType, IN const long nStart, IN const long nLen, IN const CSize &szDlgSize, IN OUT CPoint &ptDlgPos, IN OUT EMistakeResponse &emrResponse, OPTIONAL OUT CString &strChangeToWord)
{
	// Highlight the word if possible within the current sub-entity of our current control
	switch (m_ectcCurrentlyCheckingControl) {
	case ctcName:
		{
			// Just set the selection in the edit window
			m_pDlgToCheck->GetDlgItem(IDC_ITEM_NAME)->SendMessage(EM_SETSEL, nStart, nStart + nLen);
			m_pDlgToCheck->GetDlgItem(IDC_ITEM_NAME)->SendMessage(EM_SCROLLCARET, 0, 0);
		}
		break;
	case ctcSentence:
		{
			// Just set the selection in the edit window
			m_pDlgToCheck->GetDlgItem(IDC_SENTENCE)->SendMessage(EM_SETSEL, nStart, nStart + nLen);
			m_pDlgToCheck->GetDlgItem(IDC_SENTENCE)->SendMessage(EM_SCROLLCARET, 0, 0);
		}
		break;
	case ctcType_Narrative_DefaultText:
		{
			// This is tricky and relies on internal implementation details of the NxRichTextEditor; I've 
			// added comments there to reference this code so that in case it needs to be changed someday 
			// this code can be repaired as well.
			// NOTE: Our external spell-checking of the narrative rests heavily on the assumption that the 
			// when editing the narrative EMR Item you CANNOT add special fields.  It can be rich text, it 
			// just can't have anything like "he/she" or "patient age" or whatever in it.  Right now the 
			// EMR Item Entry dialog is indeed designed such that you can't enter such fields, so it works.
			CRichEditCtrl *pre = (CRichEditCtrl *)m_pDlgToCheck->GetDlgItem(IDC_NARRATIVE_DEFAULT_TEXT)->GetDlgItem(23509);
			pre->SetSel(nStart, nStart + nLen);
		}
		break;
	case ctcType_Text_DefaultText:
		{
			// Just set the selection in the edit window
			m_pDlgToCheck->GetDlgItem(IDC_DEFAULT_TEXT)->SendMessage(EM_SETSEL, nStart, nStart + nLen);
			m_pDlgToCheck->GetDlgItem(IDC_DEFAULT_TEXT)->SendMessage(EM_SCROLLCARET, 0, 0);
		}
		break;
	case ctcType_TableOrSelectList_DataList:
		{
			if (m_bCurrentlyCheckingDataElementLongForm) {
				// Select the row in the data elements list and reflect the fact that it's selected
				m_pDlgToCheck->m_pdlDataElementList->PutCurSel(m_pCurrentlyCheckingRowInList);
				m_pDlgToCheck->ReflectDataElementSelection();
				// Now our long form text is in the edit box, so select the misspelled word in there
				m_pDlgToCheck->GetDlgItem(IDC_DATA_LONGFORM)->SendMessage(EM_SETSEL, nStart, nStart + nLen);
				m_pDlgToCheck->GetDlgItem(IDC_DATA_LONGFORM)->SendMessage(EM_SCROLLCARET, 0, 0);
			} else {
				// Start editing the data cell of the current row of the data elements list
				m_pDlgToCheck->m_pdlDataElementList->StartEditing(m_pCurrentlyCheckingRowInList, CEmrItemEntryDlg::delcData);
				// Set the text selection
				m_pDlgToCheck->m_pdlDataElementList->SetEditingHighlight(nStart, nStart + nLen, VARIANT_FALSE);
				// Disengage the editor and release its capture so our spell check dialog doesn't cause it to dismiss itself
				m_pDlgToCheck->m_pdlDataElementList->EngageEditor(VARIANT_FALSE);
				::ReleaseCapture();
			}
		}
		break;
	case ctcType_Table_ColumnList:
		{
			// Start editing the data cell of the current row of the column data elements list
			m_pDlgToCheck->m_pdlColumnDataElementList->StartEditing(m_pCurrentlyCheckingRowInList, CEmrItemEntryDlg::cdelcData);
			// Set the text selection
			m_pDlgToCheck->m_pdlColumnDataElementList->SetEditingHighlight(nStart, nStart + nLen, VARIANT_FALSE);
			// Disengage the editor and release its capture so our spell check dialog doesn't cause it to dismiss itself
			m_pDlgToCheck->m_pdlColumnDataElementList->EngageEditor(VARIANT_FALSE);
			::ReleaseCapture();
		}
		break;
	default:
		ASSERT(FALSE);
		break;
	}
}

void CEmrItemEntryDlgSpellCheckHandler::OnReplaceText(IN const long nStart, IN const long nLen, IN const CString &strReplaceWithWord)
{
	// Replace the word within the current sub-entity of our current control
	switch (m_ectcCurrentlyCheckingControl) {
	case ctcName:
		{
			// Set the selection in the edit window
			m_pDlgToCheck->GetDlgItem(IDC_ITEM_NAME)->SendMessage(EM_SETSEL, nStart, nStart + nLen);
			m_pDlgToCheck->GetDlgItem(IDC_ITEM_NAME)->SendMessage(EM_SCROLLCARET, 0, 0);
			// And replace that text with the replacement word
			m_pDlgToCheck->GetDlgItem(IDC_ITEM_NAME)->SendMessage(EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)(LPCTSTR)strReplaceWithWord);
		}
		break;
	case ctcSentence:
		{
			// Set the selection in the edit window
			m_pDlgToCheck->GetDlgItem(IDC_SENTENCE)->SendMessage(EM_SETSEL, nStart, nStart + nLen);
			m_pDlgToCheck->GetDlgItem(IDC_SENTENCE)->SendMessage(EM_SCROLLCARET, 0, 0);
			// And replace that text with the replacement word
			m_pDlgToCheck->GetDlgItem(IDC_SENTENCE)->SendMessage(EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)(LPCTSTR)strReplaceWithWord);
		}
		break;
	case ctcType_Narrative_DefaultText:
		{
			// This is tricky and relies on internal implementation details of the NxRichTextEditor; I've 
			// added comments there to reference this code so that in case it needs to be changed someday 
			// this code can be repaired as well.
			// NOTE: Our external spell-checking of the narrative rests heavily on the assumption that the 
			// when editing the narrative EMR Item you CANNOT add special fields.  It can be rich text, it 
			// just can't have anything like "he/she" or "patient age" or whatever in it.  Right now the 
			// EMR Item Entry dialog is indeed designed such that you can't enter such fields, so it works.
			CRichEditCtrl *pre = (CRichEditCtrl *)m_pDlgToCheck->GetDlgItem(IDC_NARRATIVE_DEFAULT_TEXT)->GetDlgItem(23509);
			pre->SetSel(nStart, nStart + nLen);
			pre->ReplaceSel((LPCTSTR)strReplaceWithWord, TRUE);
		}
		break;
	case ctcType_Text_DefaultText:
		{
			// Set the selection in the edit window
			m_pDlgToCheck->GetDlgItem(IDC_DEFAULT_TEXT)->SendMessage(EM_SETSEL, nStart, nStart + nLen);
			m_pDlgToCheck->GetDlgItem(IDC_DEFAULT_TEXT)->SendMessage(EM_SCROLLCARET, 0, 0);
			// And replace that text with the replacement word
			m_pDlgToCheck->GetDlgItem(IDC_DEFAULT_TEXT)->SendMessage(EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)(LPCTSTR)strReplaceWithWord);
		}
		break;
	case ctcType_TableOrSelectList_DataList:
		{
			if (m_bCurrentlyCheckingDataElementLongForm) {
				// Set the selection in the edit window
				m_pDlgToCheck->GetDlgItem(IDC_DATA_LONGFORM)->SendMessage(EM_SETSEL, nStart, nStart + nLen);
				m_pDlgToCheck->GetDlgItem(IDC_DATA_LONGFORM)->SendMessage(EM_SCROLLCARET, 0, 0);
				// And replace that text with the replacement word
				m_pDlgToCheck->GetDlgItem(IDC_DATA_LONGFORM)->SendMessage(EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)(LPCTSTR)strReplaceWithWord);
			} else {
				// Make sure we're editing already (which we pretty much have to be since OnReplaceText() is 
				// only ever called after OnMistake() and our case of the OnMistake() handler starts the 
				// editing as well)
				m_pDlgToCheck->m_pdlDataElementList->StartEditing(m_pCurrentlyCheckingRowInList, CEmrItemEntryDlg::delcData);
				// Make sure the right text is selected
				m_pDlgToCheck->m_pdlDataElementList->SetEditingHighlight(nStart, nStart + nLen, VARIANT_FALSE);
				// Replace it with the replace string
				m_pDlgToCheck->m_pdlDataElementList->ReplaceEditingHighlight(_bstr_t(strReplaceWithWord.AllocSysString(), FALSE));
			}
		}
		break;
	case ctcType_Table_ColumnList:
		{
			// Make sure we're editing already (which we pretty much have to be since OnReplaceText() is 
			// only ever called after OnMistake() and our case of the OnMistake() handler starts the 
			// editing as well)
			m_pDlgToCheck->m_pdlColumnDataElementList->StartEditing(m_pCurrentlyCheckingRowInList, CEmrItemEntryDlg::cdelcData);
			// Make sure the right text is selected
			m_pDlgToCheck->m_pdlColumnDataElementList->SetEditingHighlight(nStart, nStart + nLen, VARIANT_FALSE);
			// Replace it with the replace string
			m_pDlgToCheck->m_pdlColumnDataElementList->ReplaceEditingHighlight(_bstr_t(strReplaceWithWord.AllocSysString(), FALSE));
		}
		break;
	default:
		ASSERT(FALSE);
		break;
	}
}

void CEmrItemEntryDlgSpellCheckHandler::OnFinished(IN const EFinishedReason efrReason, OUT BOOL &bStartAgain)
{
	if (efrReason != frUserCanceled) {
		// See if we were on one of the datalist controls, and if so try to move to the next row of that 
		// control instead of moving off onto the next control altogether.
		if (m_ectcCurrentlyCheckingControl == ctcType_TableOrSelectList_DataList) {
			// See if we were actually editing the row, or the long form of the row
			if (m_bCurrentlyCheckingDataElementLongForm) {
				// We've finished checking the long form for this row, so move to the next row
				if (m_pCurrentlyCheckingRowInList && m_pCurrentlyCheckingRowInList->GetNextRow() != NULL) {
					m_bCurrentlyCheckingDataElementLongForm = FALSE;
					m_pCurrentlyCheckingRowInList = m_pCurrentlyCheckingRowInList->GetNextRow();
				} else {
					// No more rows in this list so move to the next control
					m_bCurrentlyCheckingDataElementLongForm = FALSE;
					m_pCurrentlyCheckingRowInList = m_pDlgToCheck->m_pdlColumnDataElementList->GetFirstRow();
					m_ectcCurrentlyCheckingControl = (EControlsToCheck)((long)m_ectcCurrentlyCheckingControl + 1);
				}
			} else {
				// We're moving out of the row we were on.  In case we'd been editing that row (due to the 
				// discovery of a spelling mistake), we dismiss the editing.  This is how the spell correction 
				// gets written back properly (and the emr item entry screen is made aware of it).
				m_pDlgToCheck->m_pdlDataElementList->EngageEditor(VARIANT_TRUE);
				m_pDlgToCheck->m_pdlDataElementList->StopEditing(VARIANT_TRUE);
				// Then move to the long form if we're on a single- or multi-select list
				if (m_pDlgToCheck->IsDlgButtonChecked(IDC_RADIO_LIST_SELECT) || m_pDlgToCheck->IsDlgButtonChecked(IDC_RADIO_LIST_MULTISELECT)) {
					// Stay on the current row but switch to the long form control
					m_bCurrentlyCheckingDataElementLongForm = TRUE;
				} else {
					// The "long form" option doesn't even exist, so just move to the next row
					if (m_pCurrentlyCheckingRowInList && m_pCurrentlyCheckingRowInList->GetNextRow() != NULL) {
						m_pCurrentlyCheckingRowInList = m_pCurrentlyCheckingRowInList->GetNextRow();
					} else {
						// No more rows in this list so move to the next control
						//TES 3/14/2011 - PLID 42784 - Converted to datalist2
						m_pCurrentlyCheckingRowInList = m_pDlgToCheck->m_pdlColumnDataElementList->GetFirstRow();
						m_ectcCurrentlyCheckingControl = (EControlsToCheck)((long)m_ectcCurrentlyCheckingControl + 1);
					}
				}
			}
		} else if (m_ectcCurrentlyCheckingControl == ctcType_Table_ColumnList) {
			// We're moving out of the row we were on.  In case we'd been editing that row (due to the 
			// discovery of a spelling mistake), we dismiss the editing.  This is how the spell correction 
			// gets written back properly (and the emr item entry screen is made aware of it).
			m_pDlgToCheck->m_pdlColumnDataElementList->EngageEditor(VARIANT_TRUE);
			m_pDlgToCheck->m_pdlColumnDataElementList->StopEditing(VARIANT_TRUE);
			// Then move to the next row
			if (m_pCurrentlyCheckingRowInList && m_pCurrentlyCheckingRowInList->GetNextRow() != NULL) {
				m_pCurrentlyCheckingRowInList = m_pCurrentlyCheckingRowInList->GetNextRow();
			} else {
				// No more rows in this list so move to the next control
				m_pCurrentlyCheckingRowInList = NULL;
				m_ectcCurrentlyCheckingControl = (EControlsToCheck)((long)m_ectcCurrentlyCheckingControl + 1);
			}
		} else {
			// We simply move forward according to the order in our enum that lists the spell-checkable controls
			m_ectcCurrentlyCheckingControl = (EControlsToCheck)((long)m_ectcCurrentlyCheckingControl + 1);
		}
		
		// And return that we should start the spell-check again unless we've run out of controls to check
		if (m_ectcCurrentlyCheckingControl < ctc_NO_MORE_CONTROLS) {
			bStartAgain = TRUE;
		} else {
			// We hit the end, so we're not starting again.
		}
	} else {
		// The user canceled so just be done.  But if we'd been editing a datalist cell we need to cancel it
		if (m_ectcCurrentlyCheckingControl == ctcType_TableOrSelectList_DataList) {
			m_pDlgToCheck->m_pdlDataElementList->EngageEditor(VARIANT_TRUE);
			m_pDlgToCheck->m_pdlDataElementList->StopEditing(VARIANT_FALSE);
		} else if (m_ectcCurrentlyCheckingControl == ctcType_Table_ColumnList) {
			m_pDlgToCheck->m_pdlColumnDataElementList->EngageEditor(VARIANT_TRUE);
			m_pDlgToCheck->m_pdlColumnDataElementList->StopEditing(VARIANT_FALSE);
		}
	}
}

