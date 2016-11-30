// (c.haag 2011-02-21) - PLID 41618 - Initial implementation. See the header file for details.

#include "stdafx.h"
#include "LabResultsAttachmentView.h"
#include "LabResultsTabDlg.h"
//TES 2/24/2012 - PLID 44841 - Moved the LabResultField enum to GlobalLabUtils.h
#include <GlobalLabUtils.h>

using namespace NXDATALIST2Lib;

CLabResultsAttachmentView::CLabResultsAttachmentView(CLabResultsTabDlg& dlg) :
m_dlg(dlg)
{
}

// This is called when the current selection is invalid to try to make it valid. This happens after the result tab is
// initially loaded and every time a user opens the view when the current attachment is invalid.
void CLabResultsAttachmentView::SetDefaultSelection()
{
	CArray<CAttachedLabFile,CAttachedLabFile&> ayAttachedFiles;
	m_dlg.GetAllAttachedFilesSorted(ayAttachedFiles);

	// If there's at least one attached file, attempt to set m_curAttachment to the oldest attachment
	// that corresponds to an incomplete result.
	if (ayAttachedFiles.GetSize() > 0) 
	{
		for (int i=0; i < ayAttachedFiles.GetSize(); i++)
		{
			CAttachedLabFile alf = ayAttachedFiles[i];
			IRowSettingsPtr pResultRow = alf.lpRow;
			long nCompletedByUserID = VarLong(pResultRow->GetValue(lrtcCompletedBy),-1);
			if (-1 == nCompletedByUserID)
			{
				// This is the oldest (by attachment) incomplete result. This will be our default.
				m_curAttachment = alf;
				return;
			}
		}

		// If we get here, all of the results are complete, so just default to the oldest attachment.
		m_curAttachment = ayAttachedFiles[0];
	}
	else 
	{
		// No results have attachments. Reset the selection.
		CAttachedLabFile empty;
		m_curAttachment = empty;
	}
}

// This is called every time the user switches to the attachment view. Here, all non-relevant controls are hidden
// and the current selection is validated. All the code was stolen from LabResultsTabDlg so you will see a lot of
// differing PL item comments.
void CLabResultsAttachmentView::Setup()
{
	// (j.gruber 2010-11-30 09:14) - PLID 41606 - unload any previous reports		
	m_dlg.UnloadHTMLReports();

	//check the radio button if it isn't checked off
	if(!((CButton *)m_dlg.GetDlgItem(IDC_PDFVIEW_RADIO))->GetCheck())
		((CButton *)m_dlg.GetDlgItem(IDC_PDFVIEW_RADIO))->SetCheck(TRUE);

	// (j.dinatale 2010-12-14) - PLID 41438 - ensure that the other radio buttons arent in their checked states
	if(((CButton *)m_dlg.GetDlgItem(IDC_DISCRETEVALUES_RADIO))->GetCheck())
		((CButton *)m_dlg.GetDlgItem(IDC_DISCRETEVALUES_RADIO))->SetCheck(FALSE);

	if(((CButton *)m_dlg.GetDlgItem(IDC_REPORTVIEW_RADIO))->GetCheck())
		((CButton *)m_dlg.GetDlgItem(IDC_REPORTVIEW_RADIO))->SetCheck(FALSE);

	//hide everything
	// (c.haag 2011-02-22) - PLID 42589 - Pass in the current tree selection. Don't pass in our current
	// row because the row is supposed to correspond what is selected or being selected in the results tree.
	m_dlg.UpdateControlStates(true, m_dlg.m_pResultsTree->CurSel);

	// (j.gruber 2010-11-24 11:42) - PLID 41607 
	m_dlg.RefreshHTMLReportOnlyControls(FALSE);

	//enlarge the webviewer
	// (c.haag 2010-11-23 11:19) - PLID 37372 - Make it a little shorter
	// (a.walling 2011-04-29 08:26) - PLID 43501 - Don't mess with z-order or copying client bits
	m_dlg.SetSingleControlPos(IDC_PDF_PREVIEW, NULL, 5, 91, 945, 350, SWP_SHOWWINDOW|SWP_NOCOPYBITS|SWP_NOZORDER|SWP_NOOWNERZORDER);		

	// (j.dinatale 2010-12-14) - PLID 41438 - until the z-order issue is fixed, we need to maintain the current view
	m_dlg.m_nCurrentView = rvPDF;

	// (c.haag 2011-02-22) - PLID 41618 - Check whether our result exists with an attachment in tree if we have one. 
	// If it's not in the tree or it's somehow different, we need to reset our selection
	if (NULL != m_curAttachment.lpRow) 
	{
		CAttachedLabFile alf = m_dlg.GetAttachedLabFile(m_curAttachment.lpRow);
		if (-1 == alf.nMailID || m_curAttachment != alf)
		{
			SetDefaultSelection();
		}
	}
	else 
	{
		// If we get here, it means we have no selection. Maybe results were added since we were here last; try setting
		// a new default selection.
		SetDefaultSelection();
	}

	// Now update the screen content
	UpdateView();

	// (j.dinatale 2010-12-13) - PLID 41438 - No need to remember, the user will be able to setup their default view
	//set the ConfigRT to store the user's last view
	//SetRemotePropertyInt("LabResultUserView", (long)rvPDF, 0, GetCurrentUserName());
}

// This is called when the user switches to the attachment view and navigates it. The current attachment image
// is updated, as well as all the button states.
void CLabResultsAttachmentView::UpdateView()
{
	const CString& strAttachedFile = m_curAttachment.strFileName;
	if(!strAttachedFile.IsEmpty()) 
	{
		// Get the full path
		CString strFileName = strAttachedFile;
		if(strFileName.Find("\\") == -1) {
			strFileName = GetPatientDocumentPath(m_dlg.m_nPatientID) ^ strFileName;
		}

		// Remember which file we're previewing and enable controls
		m_dlg.GetDlgItem(IDC_PDF_PREVIEW)->EnableWindow(TRUE); // Enable the activex control
		m_dlg.m_btnZoom.EnableWindow(TRUE); // Enable the Zoom button

		// If this is a .pdf file, we want to append #toolbar=0, to hide the .pdf controls.
		CString strBrowserFilename = strFileName;
		if(strBrowserFilename.Right(4).CompareNoCase(".pdf") == 0) {
			strBrowserFilename += "#toolbar=0";
		}

		// Now do the navigation
		COleVariant varUrl(strBrowserFilename);
		// (a.walling 2011-04-29 08:26) - PLID 43501 - Bypass navigation cache
		m_dlg.m_pBrowser->Navigate2(varUrl, COleVariant((long)(navNoHistory|navNoReadFromCache|navNoWriteToCache)), NULL, NULL, NULL);	
		m_dlg.GetDlgItem(IDC_PDF_PREVIEW)->EnableWindow(TRUE);

		// 1. Update the result label
		CString strResultName = VarString(m_dlg.GetTreeValue(m_curAttachment.lpRow,lrfName,lrtcValue), "");
		m_dlg.m_nxstaticLabResultLabel.SetWindowText(FormatString("Result: %s", strResultName));

		// 2. Update the detach/attach button
		m_dlg.GetDlgItem(IDC_ATTACH_FILE)->ShowWindow(SW_HIDE); // We can't attach from here because we only show attached documents
		m_dlg.GetDlgItem(IDC_DETACH_FILE)->ShowWindow(SW_SHOW);
		m_dlg.GetDlgItem(IDC_DOC_PATH_LINK)->ShowWindow(SW_SHOW);
		m_dlg.GetDlgItem(IDC_DETACH_FILE)->EnableWindow(TRUE);
		m_dlg.m_nxlDocPath.SetText(GetFileName(strFileName));
		m_dlg.m_nxlDocPath.SetType(dtsHyperlink);
		m_dlg.InvalidateDlgItem(IDC_DOC_PATH_LINK);
		// (z.manning 2010-05-12 12:26) - PLID 37400 - We allow them to attach files to read-only results
		// but not detach files as it may have came from HL7.
		BOOL bEnableResults = TRUE;
		IRowSettingsPtr pResultRow(m_curAttachment.lpRow);
		long nHL7MessageID = VarLong(pResultRow->GetValue(lrtcHL7MessageID), -1);
		if(nHL7MessageID != -1 && !CheckCurrentUserPermissions(bioPatientLabs, sptDynamic1, FALSE, 0, TRUE)) {
			bEnableResults = FALSE;
		}
		m_dlg.EnableResultWnd(bEnableResults, m_dlg.GetDlgItem(IDC_DETACH_FILE));

		// 3. Update the scroll buttons
		CArray<CAttachedLabFile,CAttachedLabFile&> ayAttachedFiles;
		m_dlg.GetAllAttachedFilesSorted(ayAttachedFiles);
		int i;
		for (i=0; i < ayAttachedFiles.GetSize(); i++) 
		{
			if (ayAttachedFiles[i].lpRow == m_curAttachment.lpRow) 
			{
				break;
			}
		}
		if (i < ayAttachedFiles.GetSize()) 
		{
			m_dlg.GetDlgItem(IDC_RESULT_SCROLL_LEFT)->EnableWindow( (0 == i) ? FALSE : TRUE);
			m_dlg.GetDlgItem(IDC_RESULT_SCROLL_RIGHT)->EnableWindow( (ayAttachedFiles.GetSize()-1 == i) ? FALSE : TRUE);
		} 
		else {
			// This should never happen because our current row must have come from ayAttachedFiles too!
			ThrowNxException("Attempted to update lab attachment view with foreign result!");
		}
		
		// 4. Update the zoom button
		m_dlg.m_btnZoom.EnableWindow(TRUE);

		// 5. Update the notes buttons
		// (j.dinatale 2011-01-03) - PLID 41966 - determine if the notes we are interested in are the result level or specimen level
		if(GetRemotePropertyInt("LabNotesLevelPDFView", 0, 0, "<None>", true) == 0)
		{
			// if we are interested in result level notes then format the button based on the result notes
			m_dlg.FormatNotesButtonsForResult(m_curAttachment.lpRow);
		}
		else
		{
			// otherwise, grab the lab row of the result
			IRowSettingsPtr pLabRow = m_dlg.GetLabRow(m_curAttachment.lpRow);
			if (pLabRow)
			{
				// if we got one, format the button based on the lab notes
				m_dlg.FormatNotesButtonsForLab(pLabRow);
			}
			else
			{
				// otherwise, we didnt get a lab row, play it safe and disable the buttons
				m_dlg.DisableNotesButtons();
			}
		}
	}
	else 
	{
		// Disable the navigation window
		m_dlg.KillTimer(IDT_DISABLE_BROWSER);
		// (a.walling 2011-04-29 08:26) - PLID 43501 - Bypass navigation cache
		m_dlg.m_pBrowser->Navigate2(COleVariant("about:blank"), COleVariant((long)(navNoHistory|navNoReadFromCache|navNoWriteToCache)), NULL, NULL, NULL);
		m_dlg.GetDlgItem(IDC_PDF_PREVIEW)->EnableWindow(FALSE);

		// 1. Update the result label
		m_dlg.m_nxstaticLabResultLabel.SetWindowText("(No result selected)");
		// 2. Update the detach/attach button
		m_dlg.GetDlgItem(IDC_ATTACH_FILE)->ShowWindow(SW_HIDE);
		m_dlg.GetDlgItem(IDC_DETACH_FILE)->ShowWindow(SW_HIDE);
		m_dlg.GetDlgItem(IDC_DOC_PATH_LINK)->ShowWindow(SW_HIDE);
		// 3. Update the scroll buttons
		m_dlg.GetDlgItem(IDC_RESULT_SCROLL_LEFT)->EnableWindow(FALSE);
		m_dlg.GetDlgItem(IDC_RESULT_SCROLL_RIGHT)->EnableWindow(FALSE);
		// 4. Update the zoom button		
		m_dlg.m_btnZoom.EnableWindow(FALSE);
		// 5. Update the notes buttons
		m_dlg.DisableNotesButtons();
	}

	//
	// Now update other form controls that do not require special handling in the attachment view
	//
	// 6. Update the acknowledged button text
	m_dlg.FormatAcknowledgedButtonText(m_curAttachment.lpRow);
	// 7. Update the signature button text
	m_dlg.FormatSignButtonText(m_curAttachment.lpRow);			
	// 8. Update the mark completed button text
	m_dlg.FormatMarkCompletedButtonText(m_curAttachment.lpRow);
}

// This is called when the user clicks on any hyperlink label
LRESULT CLabResultsAttachmentView::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	switch ((UINT)wParam) 
	{
	case IDC_DOC_PATH_LINK:
		// Open the file
		m_dlg.ZoomAttachment(m_curAttachment.strFileName);
		break;
	default:
		// We don't recognize any other labels
		break;
	}
	return 0;
}

// This is called when the user pressed the Detach button while in this view
void CLabResultsAttachmentView::OnDetachFile()
{
	// Detach the file
	m_dlg.DetachFile(m_curAttachment.lpRow);
	// Now reset our selection as if we were pulling up the view the first time
	SetDefaultSelection();
	// And update the view
	UpdateView();
}

// This is called when the user pressed on either scroll button while in this view
void CLabResultsAttachmentView::OnBnClickedResultScroll(int dir)
{
	// Input check
	if (dir != 1 && dir != -1) {
		ThrowNxException("CLabResultsAttachmentView::OnBnClickedResultScroll called with invalid value!");
	}

	// Calculate the current result index
	CArray<CAttachedLabFile,CAttachedLabFile&> ayAttachedFiles;
	m_dlg.GetAllAttachedFilesSorted(ayAttachedFiles);
	int i;
	for (i=0; i < ayAttachedFiles.GetSize(); i++) 
	{
		if (ayAttachedFiles[i].lpRow == m_curAttachment.lpRow) 
		{
			break;
		}
	}	

	// Now move the index to its new value and ensure it's still valid
	i += dir;
	if (i < 0 || i >= ayAttachedFiles.GetSize()) 
	{
		ThrowNxException("CLabResultsAttachmentView::OnBnClickedResultScroll moved in an invalid direction!");
	}
	m_curAttachment = ayAttachedFiles[i];

	// Now reflect the selection change
	UpdateView();
}

// This is called when the user pressed on the zoom button while in this view
void CLabResultsAttachmentView::OnZoom()
{
	m_dlg.ZoomAttachment(m_curAttachment.strFileName);
}

// This is called when the user pressed on the acknowledge results button while in this view
void CLabResultsAttachmentView::OnAcknowledgeResult()
{
	m_dlg.AcknowledgeResults(m_curAttachment.lpRow);
}

// This is called when the user pressed on the signature button while in this view
void CLabResultsAttachmentView::OnSignature()
{
	m_dlg.SignResults(m_curAttachment.lpRow);
}

// This is called when the user pressed on the mark lab complete button while in this view
void CLabResultsAttachmentView::OnLabMarkCompleted()
{
	m_dlg.MarkLabCompleted(m_curAttachment.lpRow);
}

// This is called when the user pressed on one of the notes buttons while in this view
void CLabResultsAttachmentView::SetUpNotesDlg(bool bAutoAddNewNote)
{
	if (NULL != m_curAttachment.lpRow)
	{
		// (j.dinatale 2011-01-03) - PLID 41966 - determine if we care about result level or specimen level notes
		if(GetRemotePropertyInt("LabNotesLevelPDFView", 0, 0, "<None>", true) == 0){
			m_dlg.ShowNotesDlgForResult(m_curAttachment.lpRow, bAutoAddNewNote);
		}else{
			IRowSettingsPtr pLabRow = m_dlg.GetLabRow(m_curAttachment.lpRow);
			if(pLabRow){
				m_dlg.ShowNotesDlgForLab(pLabRow, bAutoAddNewNote);
			}
		}
	}
	else {
		// This should never happen; the button should be disabled
	}
}
