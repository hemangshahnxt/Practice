// NxRAPIBrowseDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "NxRAPIBrowseDlg.h"
#include "NxRAPI.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNxRAPIBrowseDlg dialog

#define ID_OPEN		WM_USER + 1000


CNxRAPIBrowseDlg::CNxRAPIBrowseDlg(CNxRAPI* pRAPI, BOOL bOpenFileDialog, LPCTSTR lpszDefExt /*= NULL*/, LPCTSTR lpszFileName /*= NULL*/, DWORD dwFlags /*= OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT*/, LPCTSTR lpszFilter /*= NULL*/, CWnd* pParentWnd /*= NULL*/)
	: CNxDialog(CNxRAPIBrowseDlg::IDD, pParentWnd)
{
	//{{AFX_DATA_INIT(CNxRAPIBrowseDlg)
	m_strSelectedFileName = _T("");
	m_strCurrentPath = _T("");
	//}}AFX_DATA_INIT

	m_pRAPI = pRAPI;

	if (lpszFileName)
		m_strCurrentPath = lpszFileName;
	m_strCurrentPath.Replace("/", "\\");

	// Find the last \\ in the string
	if (-1 == m_strCurrentPath.ReverseFind('\\'))
		m_strCurrentPath.Empty();
	else
	{
		m_strCurrentPath = m_strCurrentPath.Left( m_strCurrentPath.ReverseFind('\\') );
	}
}


void CNxRAPIBrowseDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNxRAPIBrowseDlg)
	DDX_Control(pDX, IDC_RAPI_BROWSE_COMBO_TYPE, m_cmbFileMask);
	DDX_Control(pDX, IDC_RAPI_BROWSE_LIST, m_List);
	DDX_Text(pDX, IDC_RAPI_BROWSE_EDIT_FILENAME, m_strSelectedFileName);
	DDX_Text(pDX, IDC_EDIT_PATH, m_strCurrentPath);
	DDX_Control(pDX, IDC_RAPI_BROWSE_EDIT_FILENAME, m_nxeditRapiBrowseEditFilename);
	DDX_Control(pDX, IDC_EDIT_PATH, m_nxeditEditPath);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNxRAPIBrowseDlg, CNxDialog)
	//{{AFX_MSG_MAP(CNxRAPIBrowseDlg)
	ON_BN_CLICKED(IDC_BTN_OPEN, OnBtnOpen)
	ON_BN_CLICKED(IDC_BTN_UP, OnBtnUp)
	ON_WM_DESTROY()
	ON_COMMAND(ID_OPEN, OnRClickOpen)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_RAPI_BROWSE_LIST, OnItemchangedRapiBrowseList)
	ON_NOTIFY(NM_DBLCLK, IDC_RAPI_BROWSE_LIST, OnDblclkRapiBrowseList)
	ON_NOTIFY(NM_RCLICK, IDC_RAPI_BROWSE_LIST, OnRclickRapiBrowseList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNxRAPIBrowseDlg message handlers

void CNxRAPIBrowseDlg::OnBtnOpen() 
{
	UpdateData(TRUE);

	CString strName = GetPathName();

	//DRT 7/2/03 - don't let them choose nothing!
	if(strName.IsEmpty()) {
		MsgBox("Please choose a file first.");
		return;
	}

	OnOK();	
}

void CNxRAPIBrowseDlg::OnBtnUp() 
{
	long iIndex = m_strCurrentPath.ReverseFind('\\');
	if (iIndex == -1) return;
	m_strCurrentPath = m_strCurrentPath.Left(iIndex);
	PopulateFileList();
}

BOOL CNxRAPIBrowseDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	// Retrieve the system image list
	SHFILEINFO sfi;
	memset(&sfi, 0, sizeof(sfi));
	HIMAGELIST hil = reinterpret_cast<HIMAGELIST> (
		SHGetFileInfo (
			"C:\\", 
			0, 
			&sfi, 
			sizeof(sfi), 
			SHGFI_SYSICONINDEX | SHGFI_SMALLICON)
	);
	if (hil)
	{
		m_SmallImgList.Attach (hil);
		m_List.SetImageList(&m_SmallImgList, LVSIL_SMALL);
	}

	// Assign the up folder icon
	((CButton*)GetDlgItem(IDC_BTN_UP))->SetIcon(::LoadIcon(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_UPFOLDER)));

	// Select the default *.* file mask
	m_cmbFileMask.SetCurSel(0);

	// Fill the list with files
	PopulateFileList();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNxRAPIBrowseDlg::OnDestroy() 
{
	m_List.SetImageList(NULL, LVSIL_SMALL);
	m_SmallImgList.Detach();
	CDialog::OnDestroy();
}

void CNxRAPIBrowseDlg::PopulateFileList()
{
	try {
		CString strCurPath = m_strCurrentPath ^ "*.*";
		CWaitCursor wc;
		LPCE_FIND_DATA pfd;
		WCHAR wsz[512];
		DWORD dwItemsFound = 0;

		////////////////////////////////////////////////////
		// Update the path in the display
		UpdateData(FALSE);

		////////////////////////////////////////////////////
		// Clear the file list
		m_List.DeleteAllItems();

		////////////////////////////////////////////////////
		// Initialize the connection with the CE device
		if (S_OK != m_pRAPI->RapiInit())
		{
			AfxThrowNxException("CopyCEToPC: Failed to initialize connection with Windows CE-based device.\n"
				"The device is not present or not connected properly.");
		}

		// Convert the current path to Unicode and then
		// iterate through the files
		MultiByteToWideChar( CP_ACP, 0, strCurPath,
				strCurPath.GetLength()+1, wsz,   
			 sizeof(wsz)/sizeof(wsz[0]) );

		if (!m_pRAPI->CeFindAllFiles(wsz, FAF_FOLDERS_ONLY | FAF_ATTRIBUTES | FAF_NAME, &dwItemsFound, &pfd))
		{
			CString strErr;
			strErr.Format("Error populating file list in CE Device folder %s", m_strCurrentPath);
			AfxThrowNxException(strErr);
		}
		for (DWORD i=0; i < dwItemsFound; i++)
		{
			if (!(pfd[i].dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
			{
				CString strFilename = pfd[i].cFileName;
				int iItem = m_List.InsertItem(m_List.GetItemCount(), strFilename, GetIconIndex(strFilename, pfd[i].dwFileAttributes));
				m_List.SetItemData(iItem, pfd[i].dwFileAttributes);
			}
		}
		m_pRAPI->CeRapiFreeBuffer(pfd);


		if (!m_pRAPI->CeFindAllFiles(wsz, FAF_ATTRIBUTES | FAF_NAME, &dwItemsFound, &pfd))
		{
			CString strErr;
			strErr.Format("Error populating file list in CE Device folder %s", m_strCurrentPath);
			AfxThrowNxException(strErr);
		}
		for (i=0; i < dwItemsFound; i++)
		{
			CString strFilename = pfd[i].cFileName;
			if (!(pfd[i].dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
				!(pfd[i].dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
			{
				int iItem = m_List.InsertItem(m_List.GetItemCount(), strFilename, GetIconIndex(strFilename, pfd[i].dwFileAttributes));
				m_List.SetItemData(iItem, pfd[i].dwFileAttributes);
			}
		}
		m_pRAPI->CeRapiFreeBuffer(pfd);

		////////////////////////////////////////////////////
		// Disconnect from the CE device
		m_pRAPI->CeRapiUninit();
	}
	catch (CNxException* e)
	{
		char sz[512];
		e->GetErrorMessage(sz, 512);
		PostMessage(WM_COMMAND, IDCANCEL);
		AfxThrowNxException(sz);
	}
}

int CNxRAPIBrowseDlg::GetIconIndex(CString strFPath, DWORD dwFileAttr)
{
	static int _nDirIconIdx = -1;
	SHFILEINFO sfi;
	memset(&sfi, 0, sizeof(sfi));

	if (dwFileAttr & FILE_ATTRIBUTE_DIRECTORY)
	{
		if (_nDirIconIdx == -1)
		{
			SHGetFileInfo (
				strFPath, 
				FILE_ATTRIBUTE_DIRECTORY, 
				&sfi, 
				sizeof(sfi), 
				SHGFI_SMALLICON | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES
			);
			// Cache the index for the directory icon
			_nDirIconIdx = sfi.iIcon;
		}

		return _nDirIconIdx;
	}
	else
	{
		SHGetFileInfo (
			strFPath, 
			FILE_ATTRIBUTE_NORMAL, 
			&sfi, 
			sizeof(sfi), 
			SHGFI_SMALLICON | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES
		);

		return sfi.iIcon;
	}

	return -1;
}

void CNxRAPIBrowseDlg::OnItemchangedRapiBrowseList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
	if (!(pNMListView->uOldState & LVIS_SELECTED) && (pNMListView->uNewState & LVIS_SELECTED))
	{
		if (!(m_List.GetItemData(pNMListView->iItem) & FILE_ATTRIBUTE_DIRECTORY))
		{
			m_strSelectedFileName = m_strCurrentPath ^ m_List.GetItemText(pNMListView->iItem, 0);
			UpdateData(FALSE);
		}
	}
	*pResult = 0;
}

void CNxRAPIBrowseDlg::OnDblclkRapiBrowseList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnRClickOpen();
	*pResult = 0;	
}

CString CNxRAPIBrowseDlg::GetPathName()
{
	return m_strSelectedFileName;
}

int CNxRAPIBrowseDlg::DoModal() 
{
	if (!m_pRAPI)
		return -1;
	
	return CDialog::DoModal();
}

void CNxRAPIBrowseDlg::OnOK()
{
	/////////////////////////////////////////////////
	// Verify the file is valid by trying to open it
	try {
		BOOL bOpenSucceeded = FALSE;
		HANDLE hCEFile;
		BSTR bstr = GetPathName().AllocSysString(); 

		////////////////////////////////////////////////////
		// Initialize the connection with the CE device
		if (S_OK != m_pRAPI->RapiInit())
		{
			SysFreeString(bstr);
			AfxThrowNxException("Failed to initialize connection with Windows CE-based device.\n"
				"The device is not present or not connected properly.");
		}

		// Open the file on the CE device		
		if (INVALID_HANDLE_VALUE == (hCEFile = m_pRAPI->CeCreateFile(bstr, GENERIC_READ, 0, NULL,
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)))
		{
			MsgBox(GetPathName() + "\r\nFile not found.\r\nPlease verify that the path and file name are correct.");
		}
		else
		{
			// Close the file
			m_pRAPI->CeCloseHandle(hCEFile);
			bOpenSucceeded = TRUE;
		}

		////////////////////////////////////////////////////
		// Disconnect from the CE device
		m_pRAPI->CeRapiUninit();

		// Free the bstr
		SysFreeString(bstr);

		if (bOpenSucceeded)
			CDialog::OnOK();
	}
	NxCatchAll("Error opening file on the PDA");
}

void CNxRAPIBrowseDlg::OnRclickRapiBrowseList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CMenu menu;
	menu.CreatePopupMenu();
	menu.InsertMenu(-1, MF_BYPOSITION, ID_OPEN, "Open");
	CPoint pt;
	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this, NULL);
	*pResult = 0;
}

void CNxRAPIBrowseDlg::OnRClickOpen()
{
    int nItem = m_List.GetNextItem(-1, LVNI_SELECTED);

	if (m_List.GetItemData(nItem) & FILE_ATTRIBUTE_DIRECTORY)
	{
		m_strCurrentPath = m_strCurrentPath ^ m_List.GetItemText(nItem, 0);
		PopulateFileList();
	}
	else
	{
		m_strSelectedFileName = m_strCurrentPath ^ m_List.GetItemText(nItem, 0);
		OnOK();
	}
}
