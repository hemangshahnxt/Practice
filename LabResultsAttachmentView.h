// (c.haag 2011-02-21) - PLID 41618 - Initial implementation.

#pragma once

class CLabResultsTabDlg;

// (c.haag 2010-12-29 09:16) - PLID 41618 - We used to manage attachments in a map that had filenames in it. I
// replaced the filenames with this object because we need to also have access to the row and MailID for PDF view
// navigation.
class CAttachedLabFile
{
public:
	CString strFileName;
	long nMailID;
	COleDateTime dtAttached;
	LPDISPATCH lpRow;

public:
	BOOL operator ==(const CAttachedLabFile& alf) const 
	{
		return (this->dtAttached == alf.dtAttached &&
			this->nMailID == alf.nMailID &&
			this->strFileName == alf.strFileName);
	}
	BOOL operator !=(const CAttachedLabFile& alf) const 
	{
		return (*this == alf) ? FALSE : TRUE;
	}

public:
	CAttachedLabFile()
	{
		nMailID = -1;
		dtAttached = g_cdtInvalid;
		lpRow = NULL;
	}
};

/* (c.haag 2011-02-21) - PLID 41618 - This class encapsulates the functionality of the attachment view of the
lab results tab. In a more perfect world, this would have been its own child dialog; and the attachments and results
would have been stored in a standalone document class. Instead, everything is in LabResultsTabDlg. In order to keep
the code as maintainable as possible, this class was created.

When the attachments view is active, most of the controls of the discrete values view are hidden, and the ActiveX
control that enables us to see attachments is resized to take up most of the tab area. However, there are some
controls that both the discrete view and attachment view share. I've designated numbers for them and you will see
them in my later code comments:

1. "Result" label near the top left.
2. Detach button and the filename next to it at the top middle.
3. Scroll buttons on the top right (ok I lied a little bit here, these buttons are hidden in the discrete and report views 
and not shared)
4. The zoom button to the right of the scroll buttons (this one is shared)
5. The Acknowledged button on the lower left
6. The Sign button on the lower middle
7. The Mark Lab Completed button on the lower right
8. The two Result Notes buttons on the lower right

The important thing to know about this view is that the selected attachment is not 1:1 with the discrete values tree.
You can have attachment alpha selected in the attachment view, and a result with attachment bravo selected in the
discrete values view. This is by design. You may think that this will confuse the shared buttons, but it will not. Each
of the functions which format the button labels and handle button presses are now designed to be as generic as possible.
To ensure the attachment view does not fail after a user deletes or changes something in the discrete values, the
attachment view will always validate its current attachment result when it's opened or navigated. It will also update
all the button appearances accordingly.

*/
class CLabResultsAttachmentView
{
private:
	// The parent dialog. This is needed to access data and form controls.
	CLabResultsTabDlg& m_dlg;

private:
	// The current attachment we're viewing
	CAttachedLabFile m_curAttachment;

public:
	CLabResultsAttachmentView(CLabResultsTabDlg& dlg);

public:
	// This is called when the current selection is invalid to try to make it valid. This happens after the result tab is
	// initially loaded and every time a user opens the view when the current attachment is invalid.
	void SetDefaultSelection();

	// This is called every time the user switches to the attachment view. Here, all non-relevant controls are hidden
	// and the current selection is validated.
	void Setup();

	// This is called when the user switches to the attachment view and navigates it. The current attachment image
	// is updated, as well as all the button states.
	void UpdateView();

public:
	// This is called when the user clicks on any hyperlink label
	LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	// This is called when the user pressed the Detach button while in this view
	void OnDetachFile();
	// This is called when the user pressed on either scroll button while in this view
	void OnBnClickedResultScroll(int dir);
	// This is called when the user pressed on the zoom button while in this view
	void OnZoom();
	// This is called when the user pressed on the acknowledge results button while in this view
	void OnAcknowledgeResult();
	// This is called when the user pressed on the signature button while in this view
	void OnSignature();
	// This is called when the user pressed on the mark lab complete button while in this view
	void OnLabMarkCompleted();
	// This is called when the user pressed on one of the notes buttons while in this view
	void SetUpNotesDlg(bool bAutoAddNewNote);
};
