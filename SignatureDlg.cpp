// SignatureDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EmrRc.h"
#include "SignatureDlg.h"
#include "FileUtils.h"
#include "EMNDetail.h"
#include "TopazSigPad.h"
#include <NxDataUtilitiesLib/NxSafeArray.h>
#include "NxImageCache.h"

using namespace NXINKPICTURELib;

#define INK_PICTURE_IDC	1000

// CSignatureDlg dialog
// (z.manning 2008-10-15 14:55) - PLID 21082 - Created

IMPLEMENT_DYNAMIC(CSignatureDlg, CNxDialog)

CSignatureDlg::CSignatureDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSignatureDlg::IDD, pParent)
{
	m_varSignatureInkData.vt = VT_NULL;
	// (j.jones 2010-04-12 12:30) - PLID 16594 - signatures now have a stamp
	m_varSignatureTextData.vt = VT_NULL;
	m_bReadOnly = FALSE;
	m_bRequireSignature = FALSE;
	m_bAutoCommitIfSignature = FALSE;
	m_bSetupOnly = FALSE;
	m_varOriginalSignatureInkData.vt = VT_NULL;
	m_varOriginalSignatureTextData.vt = VT_NULL;
	m_bCheckPasswordOnLoad = TRUE; // (z.manning 2008-12-09 08:59) - PLID 32260

	m_nSignatureUserID = GetCurrentUserID();
	m_strSignatureUserName = GetCurrentUserName();
}

CSignatureDlg::~CSignatureDlg()
{
}

void CSignatureDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_SELECT_SIGNATURE_IMAGE, m_btnSelectImage);
	DDX_Control(pDX, IDC_USE_DEFAULT_SIGNATURE, m_btnLoadDefaultSignature);
	DDX_Control(pDX, IDC_SELECT_SIGNATURE_IMAGE, m_btnSelectImage);
	DDX_Control(pDX, IDC_USE_DEFAULT_SIGNATURE, m_btnLoadDefaultSignature);
	DDX_Control(pDX, IDC_SAVE_DEFAULT_SIGNATURE, m_btnSaveDefaultSignature);
	DDX_Control(pDX, IDC_ERASE_SIGNATURE_INK, m_btnEraseInk);
	DDX_Control(pDX, IDC_AUTO_PROMPT_FOR_SIGNATURE, m_nxbtnAutoPromptDefaultSignature);
	DDX_Control(pDX, IDC_CLOSE_SIGNATURE_DIALOG, m_btnClose);
	DDX_Control(pDX, IDC_SIGNATURE_TEXT, m_nxstaticText);
	DDX_Control(pDX, IDC_CHECK_SIGNATURE_DATE_NO_TIME, m_checkSignatureDateNoTime);
}


BEGIN_MESSAGE_MAP(CSignatureDlg, CNxDialog)
	ON_BN_CLICKED(IDC_SELECT_SIGNATURE_IMAGE, &CSignatureDlg::OnBnClickedSelectSignatureImage)
	ON_BN_CLICKED(IDC_USE_DEFAULT_SIGNATURE, &CSignatureDlg::OnBnClickedUseDefaultSignature)
	ON_BN_CLICKED(IDC_SAVE_DEFAULT_SIGNATURE, &CSignatureDlg::OnBnClickedSaveDefaultSignature)
	ON_BN_CLICKED(IDC_ERASE_SIGNATURE_INK, &CSignatureDlg::OnBnClickedEraseSignatureInk)
	ON_BN_CLICKED(IDC_AUTO_PROMPT_FOR_SIGNATURE, &CSignatureDlg::OnBnClickedAutoPromptForSignature)
	ON_BN_CLICKED(IDC_CLOSE_SIGNATURE_DIALOG, &CSignatureDlg::OnClose)
	ON_BN_CLICKED(IDC_CHECK_SIGNATURE_DATE_NO_TIME, &CSignatureDlg::OnCheckSignatureDateNoTime)
	ON_EN_KILLFOCUS(IDC_SIGNATURE_PERCENT_SIZE, &CSignatureDlg::OnEnKillfocusSignaturePercentSize)	
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CSignatureDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CSignatureDlg)
	ON_EVENT(CSignatureDlg, INK_PICTURE_IDC, 3 /* Browse */, OnBrowseInkPicture, VTS_PBOOL)
	ON_EVENT(CSignatureDlg, INK_PICTURE_IDC, 13 /*Topaz Signature*/, OnBtnClickedTopaz, VTS_NONE)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

// CSignatureDlg message handlers

// (j.jones 2013-08-08 13:35) - PLID 42958 - added optional user ID and username, for cases when
// the signature is being created by a user who is not the logged-in user
int CSignatureDlg::DoModal(OPTIONAL long nSignatureUserID /*= GetCurrentUserID()*/,
						   OPTIONAL CString strSignatureUserName /*= GetCurrentUserName()*/)
{
	m_nSignatureUserID = nSignatureUserID;
	m_strSignatureUserName = strSignatureUserName;

	//handle bad data
	if(m_nSignatureUserID == -1 || strSignatureUserName.IsEmpty()) {
		//this should be impossible
		ThrowNxException("CSignatureDlg called with invalid user ID %li and username %s!", m_nSignatureUserID, m_strSignatureUserName);
	}

	return CNxDialog::DoModal();
}

BOOL CSignatureDlg::OnInitDialog()
{
	try
	{
		CNxDialog::OnInitDialog();

		// (j.jones 2013-08-08 13:47) - PLID 42958 - Always bulk cache preferences for the
		// provided username, which might not necessarily be the logged-in user.
		// For signing EMNs, all of this should have already been cached, but this dialog
		// is accessible outside of EMR, so we have to cache every time.
		CString strBulkCacheName = "CSignatureDlg_" + m_strSignatureUserName;
		g_propManager.CachePropertiesInBulk(strBulkCacheName, propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'AutoPromptForDefaultSignature' "
			"OR Name = 'DefaultEMRImagePenColor' "				
			")",
			_Q(m_strSignatureUserName));

		// (j.jones 2013-08-08 15:01) - PLID 42958 - it signing for a different user, change
		// the window text to state that it's a different user
		if(m_nSignatureUserID != GetCurrentUserID()) {
			CString strHeader;
			strHeader.Format("Signature - Signing as %s", m_strSignatureUserName);
			SetWindowText(strHeader);

			//if we have no special label, add this info. to the label as well
			if(m_strText.IsEmpty()) {
				m_strText.Format("Signing as %s", m_strSignatureUserName);
			}
		}

		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnSelectImage.AutoSet(NXB_MODIFY);
		m_btnLoadDefaultSignature.AutoSet(NXB_MODIFY);
		m_btnSaveDefaultSignature.AutoSet(NXB_MODIFY);
		m_btnEraseInk.AutoSet(NXB_MODIFY);
		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnTopaz.AutoSet(NXB_MODIFY); 

		m_nxstaticText.SetWindowText(m_strText);
		((CEdit*)GetDlgItem(IDC_SIGNATURE_PERCENT_SIZE))->SetLimitText(3);

		// (z.manning 2008-10-22 17:33) - If we're in setup mode, hide OK/cancel buttons and show the close button
		if(m_bSetupOnly) {
			m_btnOk.ShowWindow(SW_HIDE);
			m_btnCancel.ShowWindow(SW_HIDE);
			m_btnClose.ShowWindow(SW_SHOW);
			// (z.manning 2011-09-23 12:45) - PLID 42648 - Show the EMR scaling factor option in seutp mode.
			GetDlgItem(IDC_SIGNATURE_PERCENT_SIZE)->ShowWindow(SW_SHOWNA);
			GetDlgItem(IDC_SIGNATURE_PERCENT_SIZE_LABEL)->ShowWindow(SW_SHOWNA);
		}

		CRect rc;
		GetDlgItem(IDC_INK_CTRL_PLACEHOLDER)->GetWindowRect(rc);
		ScreenToClient(rc);
		m_wndInkPic.CreateControl(__uuidof(NxInkPicture), NULL, WS_CHILD|WS_VISIBLE|WS_TABSTOP, rc, this, INK_PICTURE_IDC);
		m_pSignatureImageCtrl = m_wndInkPic.GetControlUnknown();

		// (j.jones 2013-08-08 14:04) - PLID 42958 - use the stored signature username, they might not be the logged-in user
		m_nxbtnAutoPromptDefaultSignature.SetCheck(GetRemotePropertyInt("AutoPromptForDefaultSignature", BST_UNCHECKED, 0, m_strSignatureUserName));

		// (j.jones 2010-04-12 16:20) - PLID 16594 - unlike the load default signature button,
		// we want this option to load immediately, it is like ConfigRT, but saved in UsersT,
		// and cached with the default signature
		EnsureDefaultSignatureCache(m_nSignatureUserID);
		m_checkSignatureDateNoTime.SetCheck(g_bSignatureDateOnly);

		// (z.manning 2011-09-23 12:30) - PLID 42648 - Added EMR scaling factor
		SetDlgItemInt(IDC_SIGNATURE_PERCENT_SIZE, g_nSignatureEmrScaleFactor);

		if(m_strSignatureFileName.IsEmpty()) {
			// (z.manning 2008-10-17 09:37) - If we weren't given an image file, load the default if they
			// have one.
			
			// (a.walling 2009-12-23 12:39) - PLID 36686 - Load the file name. It should be just the filename, but may be an absolute path due to
			// the way we used to save it. If so, fix it, assuming the file exists and it is in the shared path.
			// (a.walling 2009-12-24 14:37) - PLID 36377 - Get from UsersT
			CString strFileName = GetDefaultSignatureFileName(m_nSignatureUserID);
			CString strBasePath = GetSharedPath() ^ "Images";

			if (strFileName.FindOneOf("\\/") != -1) { // contains a path separator
				if ( (_strnicmp(strBasePath, strFileName, strBasePath.GetLength()) == 0) // begins with the shared path
					&& (INVALID_FILE_ATTRIBUTES != ::GetFileAttributes(strFileName)) // and it exists
					) {
					// let's fix it
					CString strActualFileName = FileUtils::GetFileName(strFileName);
					// (a.walling 2009-12-24 14:46) - PLID 36377 - Set in UsersT
					SetDefaultSignatureFileName(m_nSignatureUserID, strActualFileName);
				
					m_strSignatureFileName = strBasePath ^ strActualFileName;
				} else {
					// this does not begin with our shared path, or it does not exist, so we'll have to use it as is.
					m_strSignatureFileName = strFileName;
				}
			} else {
				// (j.jones 2013-08-09 09:04) - PLID 57944 - don't fill the full path & file if we have no filename
				if(!strFileName.IsEmpty()) {
					m_strSignatureFileName = strBasePath ^ strFileName;
				}
			}
		} else {
			// (a.walling 2010-01-05 09:15) - PLID 33887 - We were passed in an image file name
			// so, let's ensure this has the Images shared path prepended if it is not an absolute path.

			if (m_strSignatureFileName.FindOneOf("\\/") == -1) { // does not contain a path separator
				// so include our images path
				CString strBasePath = GetSharedPath() ^ "Images";
				m_strSignatureFileName = strBasePath ^ m_strSignatureFileName;
			}
		}

		// (a.walling 2010-01-05 09:14) - PLID 33887 - We now have a signature image file name
		// (j.jones 2013-08-09 09:04) - PLID 57944 - fixed exceptions for when we don't actualy have a file name
		CString strFileNameOnly = FileUtils::GetFileName(m_strSignatureFileName);
		if(strFileNameOnly.IsEmpty()) {
			MessageBox("No signature image file is configured for this user.", "Practice", MB_ICONINFORMATION|MB_OK);
			//reset the file path so we don't try to load it again
			m_strSignatureFileName = "";
		}
		else {
			// (j.jones 2013-08-09 09:07) - PLID 57944 - give a clean warning if the file does not exist
			if(!DoesExist(m_strSignatureFileName)) {
				MessageBox(FormatString("The signature image file %s could not be found.", m_strSignatureFileName), "Practice", MB_ICONINFORMATION|MB_OK);
				//reset the file path so we don't try to load it again
				m_strSignatureFileName = "";
			}
			else {
				// (a.walling 2010-10-25 10:04) - PLID 41043 - Get a cached image reference and use that instead
				/*g_EmrImageCache.GetCachedImage(m_strSignatureFileName, m_pCachedImage);
				HBITMAP hbmp = m_pCachedImage->GetImage();*/
				
				// (a.walling 2013-06-27 13:21) - PLID 57348 - Support NxImageLib::ISourceImage
				NxImageLib::ISourceImagePtr pSource = NxImageLib::Cache::OpenSourceImage(m_strSignatureFileName);

				if(pSource && SUCCEEDED(pSource->raw_Load())) { //(FileUtils::DoesFileOrDirExist(m_strSignatureFileName)) {
					m_pSignatureImageCtrl->Source = (IUnknown*)(pSource);
					m_pSignatureImageCtrl->PutPictureFileName(_bstr_t(m_strSignatureFileName));
					if((m_varSignatureInkData.vt != VT_EMPTY && m_varSignatureInkData.vt != VT_NULL)
						|| (m_varSignatureTextData.vt != VT_EMPTY && m_varSignatureTextData.vt != VT_NULL)) {
						// (z.manning 2008-10-17 09:37) - We were given ink data so apply it.
							// (j.jones 2010-04-13 08:55) - PLID 38166 - or text data
						m_pSignatureImageCtrl->PutInkData(m_varSignatureInkData);
						m_pSignatureImageCtrl->PutTextData(m_varSignatureTextData);
					}
					else if(m_nxbtnAutoPromptDefaultSignature.GetCheck() == BST_CHECKED || m_bSetupOnly) {
						// (z.manning 2008-10-17 09:38) - We weren't given any ink so if they have the option
						// enabled, load the default ink data.
						// (j.jones 2013-08-08 16:56) - PLID 42958 - if we are signing as another user,
						// they had to just enter their password, so disable any password prompts
						// this load will do
						LoadDefaultSignature(TRUE, m_nSignatureUserID != GetCurrentUserID());
						// (z.manning 2008-10-22 10:42) - PLID 23110 - See if they want to auto commit
						_variant_t varInk = m_pSignatureImageCtrl->GetInkData();
						if(m_bAutoCommitIfSignature && varInk.vt != VT_EMPTY && varInk.vt != VT_NULL && !m_bSetupOnly) {
							OnOK();
						}
					}
				}
				else {
					MessageBox(FormatString("Could not find image file %s", m_strSignatureFileName));
				}
			}
		}

		m_pSignatureImageCtrl->PutBackColor(GetSolidBackgroundColor());
		m_pSignatureImageCtrl->PutUseCustomStamps(VARIANT_FALSE);
		// (z.manning 2008-10-15 17:53) - Hide the full screen button
		m_pSignatureImageCtrl->PutIsFullScreen(VARIANT_TRUE);
		// (z.manning 2008-10-17 09:11) - Let's use the same default pen color as EMR
		// (j.jones 2013-08-08 14:04) - PLID 42958 - use the stored signature username, they might not be the logged-in user
		m_pSignatureImageCtrl->DefaultPenColor = GetRemotePropertyInt("DefaultEMRImagePenColor", RGB(255,0,0), 0, m_strSignatureUserName);
		// (z.manning 2008-10-17 12:18) - PLID 31725 - Do not allow text stamping since we don't
		// save it anyway.
		m_pSignatureImageCtrl->PutEnableTextStamping(VARIANT_FALSE);

		// (j.jones 2010-04-12 09:30) - PLID 16594 - tell the ocx that we are using a signature date
		m_pSignatureImageCtrl->PutEnableSignatureDateStamp(VARIANT_TRUE);

		// (j.jones 2010-02-10 14:26) - PLID 37312 - tell the ocx that we control stamp setup
		// (even though stamps aren't allowed on this instance of the control)
		m_pSignatureImageCtrl->PutApplicationHandlesStampSetup(VARIANT_FALSE);

		// (j.jones 2010-02-18 10:01) - PLID 37423 - tell the ocx that it is not a SmartStamp image,
		// even though that value always defaults to FALSE anyways
		m_pSignatureImageCtrl->PutIsSmartStampImage(VARIANT_FALSE);

		// (r.gonet 05/06/2011) - PLID 43542 - Do not scale text on the signature dialog.
		m_pSignatureImageCtrl->EnableTextScaling = VARIANT_FALSE;

		// (b.spivey, May 02, 2013) - PLID 56542
		m_pSignatureImageCtrl->SetTopazSigPadConnected(GetMainFrame()->GetIsTopazConnected()); 

		if(m_bReadOnly) {
			m_pSignatureImageCtrl->PutReadOnly(VARIANT_TRUE);
			m_btnLoadDefaultSignature.EnableWindow(FALSE);
			m_btnSelectImage.EnableWindow(FALSE);
			m_btnSaveDefaultSignature.EnableWindow(FALSE);
			m_btnEraseInk.EnableWindow(FALSE);
		}

		m_varOriginalSignatureInkData = m_pSignatureImageCtrl->GetInkData();
		// (j.jones 2010-04-12 12:30) - PLID 16594 - signatures now have a stamp
		m_varOriginalSignatureTextData = m_pSignatureImageCtrl->GetTextData();

		CenterWindow();

	}NxCatchAll("CSignatureDlg::OnInitDialog");

	return TRUE;
}


// (a.walling 2009-12-24 14:29) - PLID 36377 - static cache variables
// (j.jones 2013-08-08 16:07) - PLID 42958 - added a user ID for who is cached
long CSignatureDlg::g_nSignatureUserID = -1;
_variant_t CSignatureDlg::g_varSignatureData;
CString CSignatureDlg::g_strSignatureFileName;
bool CSignatureDlg::g_bSignatureCached = false;
// (j.jones 2010-04-12 14:08) - PLID 16594 - handle the signature date stamp
_variant_t CSignatureDlg::g_varSignatureText;
BOOL CSignatureDlg::g_bSignatureDateOnly = FALSE;
// (z.manning 2011-09-23 12:31) - PLID 42648 - Added EMR scaling factor
short CSignatureDlg::g_nSignatureEmrScaleFactor = 100;

// (a.walling 2009-12-24 14:29) - PLID 36377 - Ensure the cache and return the default signature filename
// (j.jones 2013-08-08 14:04) - PLID 42958 - now takes in a user ID
CString CSignatureDlg::GetDefaultSignatureFileName(long nSignatureUserID)
{
	EnsureDefaultSignatureCache(nSignatureUserID);

	return g_strSignatureFileName;
}

// (a.walling 2009-12-24 14:29) - PLID 36377 - Ensure the cache and return the default signature data
// (j.jones 2010-04-12 14:08) - PLID 16594 - handle the signature date stamp
// (j.jones 2013-08-08 14:04) - PLID 42958 - now takes in a user ID
void CSignatureDlg::GetDefaultSignatureData(long nSignatureUserID, OUT _variant_t &varData, OUT _variant_t &varText, OUT BOOL &bSignatureDateOnly)
{
	EnsureDefaultSignatureCache(nSignatureUserID);

	varData = g_varSignatureData;
	varText = g_varSignatureText;
	bSignatureDateOnly = g_bSignatureDateOnly;
}

// (a.walling 2009-12-24 14:29) - PLID 36377 - Set the default signature data
// (j.jones 2010-04-12 14:08) - PLID 16594 - handle the signature date stamp
// (j.jones 2013-08-08 14:04) - PLID 42958 - now takes in a user ID
void CSignatureDlg::SetDefaultSignatureData(long nSignatureUserID, _variant_t& varNewData, _variant_t &varNewText, BOOL &bSignatureDateOnly)
{
	ClearSignatureCache();

	ADODB::_CommandPtr pCommand = OpenParamQuery("SELECT SignatureData, SignatureTextData, SignatureDateOnly FROM UsersT WHERE PersonID = ?");
	AddParameterLong(pCommand, "PersonID", nSignatureUserID);

	ADODB::_RecordsetPtr rsSig;
	HR(rsSig.CreateInstance(__uuidof(ADODB::Recordset)));
	rsSig->CursorLocation = ADODB::adUseClient;
	pCommand->ActiveConnection = GetRemoteData();

	tagVARIANT varOptional = COleVariant((long)DISP_E_PARAMNOTFOUND, VT_ERROR);
	HR(rsSig->Open(_variant_t(pCommand.GetInterfacePtr()), varOptional, ADODB::adOpenDynamic, ADODB::adLockOptimistic, ADODB::adCmdUnspecified));

	if(!rsSig->eof) {
		rsSig->Fields->Item["SignatureData"]->Value = varNewData;
		rsSig->Fields->Item["SignatureTextData"]->Value = varNewText;
		rsSig->Fields->Item["SignatureDateOnly"]->Value = bSignatureDateOnly ? g_cvarTrue : g_cvarFalse;
		rsSig->Update();
	}
}

// (a.walling 2009-12-24 14:29) - PLID 36377 - Set the default signature file name
// (j.jones 2013-08-08 14:04) - PLID 42958 - now takes in a user ID
void CSignatureDlg::SetDefaultSignatureFileName(long nSignatureUserID, const CString& strNewFileName)
{
	ClearSignatureCache();

	// (j.jones 2013-08-08 14:04) - PLID 42958 - use the stored signature user ID, it might not be the logged in user
	ExecuteParamSql("UPDATE UsersT SET SignatureFile = {STRING} WHERE PersonID = {INT}", strNewFileName, nSignatureUserID);
}

// (a.walling 2009-12-24 14:29) - PLID 36377 - Cache the filename and data for the current user
// (j.jones 2013-08-08 14:04) - PLID 42958 - now takes in a user ID
void CSignatureDlg::EnsureDefaultSignatureCache(long nSignatureUserID)
{
	// (j.jones 2013-08-08 16:07) - PLID 42958 - clear the cache if the user is different
	if(g_nSignatureUserID != nSignatureUserID) {
		ClearSignatureCache();
	}
	else if (g_bSignatureCached) {
		return;
	}

	ClearSignatureCache();

	g_nSignatureUserID = nSignatureUserID;

	// (j.jones 2010-04-12 14:08) - PLID 16594 - handle the signature date stamp
	// (z.manning 2011-09-23 12:29) - PLID 42648 - Added SignatureEmrScaleFactor
	// (j.jones 2013-08-08 14:04) - PLID 42958 - use the stored signature user ID, it might not be the logged in user
	ADODB::_RecordsetPtr prs = CreateParamRecordset(
		"SELECT SignatureFile, SignatureData, SignatureTextData, SignatureDateOnly, SignatureEmrScaleFactor \r\n"
		"FROM UsersT \r\n"
		"WHERE PersonID = {INT} \r\n"
		, g_nSignatureUserID);

	if (!prs->eof) {
		g_strSignatureFileName = AdoFldString(prs, "SignatureFile");
		g_varSignatureData = prs->Fields->Item["SignatureData"]->Value;
		g_varSignatureText = prs->Fields->Item["SignatureTextData"]->Value;
		g_bSignatureDateOnly = AdoFldBool(prs, "SignatureDateOnly", FALSE);
		g_nSignatureEmrScaleFactor = AdoFldShort(prs, "SignatureEmrScaleFactor");
	}

	g_bSignatureCached = true;
}

// (a.walling 2009-12-24 14:29) - PLID 36377 - Clear the cache
void CSignatureDlg::ClearSignatureCache()
{
	// (j.jones 2013-08-08 16:07) - PLID 42958 - added a user ID for who is cached
	g_nSignatureUserID = -1;
	g_strSignatureFileName.Empty();
	g_varSignatureData.Clear();
	g_varSignatureText.Clear();
	g_bSignatureDateOnly = false;
	g_nSignatureEmrScaleFactor = 100; // (z.manning 2011-09-23 12:39) - PLID 42648

	g_bSignatureCached = false;
}

void CSignatureDlg::OnCancel()
{
	try
	{
		if(!SaveScalingFactor()) {
			return;
		}

		CNxDialog::OnCancel();
	}
	NxCatchAll(__FUNCTION__);
}

void CSignatureDlg::OnOK()
{
	try
	{
		if(!SaveScalingFactor()) {
			return;
		}

		// (z.manning 2008-10-17 09:39) - Save our data to the member variables so they can be
		// accessed elsewhere.
		m_varSignatureInkData = m_pSignatureImageCtrl->GetInkData();
		m_strSignatureFileName = VarString(m_pSignatureImageCtrl->GetPictureFileName(), "");
		// (j.jones 2010-04-12 12:30) - PLID 16594 - signatures now have a stamp
		m_varSignatureTextData = m_pSignatureImageCtrl->GetTextData();

		if(m_strSignatureFileName.IsEmpty()) {
			MessageBox("You must select an image before continuing.");
			return;
		}

		// (z.manning 2008-10-17 10:36) - PLID 23110 - Signature is not required for EMR, however.
		if(m_bRequireSignature) {
			// (z.manning 2008-10-17 09:39) - Per c.majeed a signature is required when marking labs completed.
			if(m_varSignatureInkData.vt == VT_NULL || m_varSignatureInkData.vt == VT_EMPTY) {
				MessageBox("You must sign before continuing.");
				return;
			}
		}

		CDialog::OnOK();

	}NxCatchAll("CSignatureDlg::OnOK");
}
void CSignatureDlg::OnBnClickedSelectSignatureImage()
{
	try
	{
		BrowseForImageFile();

	}NxCatchAll("CSignatureDlg::OnBnClickedSelectSignatureImage");
}

void CSignatureDlg::OnBrowseInkPicture(BOOL *pbProcessed)
{
	try
	{
		*pbProcessed = TRUE;
		BrowseForImageFile();

	}NxCatchAll("CSignatureDlg::OnBrowseInkPicture");
}

void CSignatureDlg::BrowseForImageFile()
{
	// (z.manning 2008-10-16 13:36) - Warn them as long as there's a previous image
	if(m_pSignatureImageCtrl->GetImage() != VT_NULL) {
		int nResult = MessageBox("Selecting a new image will erase all ink and clear out your default signature "
			"if you have one. Are you sure you want to continue?", NULL, MB_YESNO|MB_ICONWARNING);
		if(nResult != IDYES) {
			return;
		}
	}

	CSelectImageDlg dlg(this);
	CString strBasePath = GetSharedPath() ^ "Images";
	dlg.m_strImagePath = strBasePath;
	dlg.m_strMainTabName = "Images";
	dlg.m_nPatientID = -1;
	if(dlg.DoModal() != IDOK) {
		return;
	}

	CString strSignatureFileName = strBasePath ^ dlg.m_strFileName;
	NxImageLib::ISourceImagePtr pSource;

	// (a.walling 2013-08-06 13:55) - PLID 57891 - Attempt to load the image, and do so in a way such that if we fail we don't modify anything.
	pSource = NxImageLib::Cache::OpenSourceImage(strSignatureFileName);
	pSource->Load(); // throw if error

	m_strSignatureFileName = strSignatureFileName;

	// (z.manning 2008-10-15 17:44) - Remember this selection
	// (a.walling 2009-12-23 12:39) - PLID 36686 - Save only the file name.
	// (a.walling 2009-12-24 14:46) - PLID 36377 - Set in UsersT
	SetDefaultSignatureFileName(m_nSignatureUserID, FileUtils::GetFileName(m_strSignatureFileName));

	// (z.manning 2008-10-17 09:41) - Clear any existing ink and assign the new image to the control.
	m_pSignatureImageCtrl->PutInkData(g_cvarEmpty);
	// (j.jones 2010-04-12 12:23) - PLID 16594 - we now have a date stamp that needs erased
	m_pSignatureImageCtrl->PutTextData(g_cvarEmpty);

	//g_EmrImageCache.GetCachedImage(m_strSignatureFileName, m_pCachedImage);
	//m_pSignatureImageCtrl->Image = (OLE_HANDLE)m_pCachedImage->GetImage();
	
	// (a.walling 2013-06-27 13:21) - PLID 57348 - Support NxImageLib::ISourceImage
	m_pSignatureImageCtrl->Source = (IUnknown*)(pSource);
	m_pSignatureImageCtrl->PutPictureFileName(_bstr_t(m_strSignatureFileName));
	m_wndInkPic.Invalidate();
	SaveDefaultSignature(FALSE);
}

CString CSignatureDlg::GetSignatureFileName()
{
	// (a.walling 2010-01-05 09:02) - PLID 33887 - Only return the file name; the path is assumed to be the
	// root images path, as always, ie GetSharedPath() ^ "Images"
	return FileUtils::GetFileName(m_strSignatureFileName);
}

_variant_t CSignatureDlg::GetSignatureInkData()
{
	return m_varSignatureInkData;
}

// (j.jones 2010-04-12 12:30) - PLID 16594 - signatures now have a stamp
_variant_t CSignatureDlg::GetSignatureTextData()
{
	return m_varSignatureTextData;
}

BOOL CSignatureDlg::GetSignatureDateOnly()
{
	EnsureDefaultSignatureCache(m_nSignatureUserID);
	return g_bSignatureDateOnly;
}

// (j.jones 2010-04-12 12:30) - PLID 16594 - signatures now have a stamp
void CSignatureDlg::SetSignature(const CString strFile, const _variant_t varInkData, const _variant_t varTextData)
{
	m_strSignatureFileName = strFile;
	m_varSignatureInkData = varInkData;
	m_varSignatureTextData = varTextData;
}

// (z.manning 2011-09-23 14:50) - PLID 42648
short CSignatureDlg::GetSignatureEmrScaleFactor()
{
	EnsureDefaultSignatureCache(m_nSignatureUserID);
	return g_nSignatureEmrScaleFactor;
}

void CSignatureDlg::OnBnClickedUseDefaultSignature()
{
	try
	{
		LoadDefaultSignature(FALSE, FALSE);

	}NxCatchAll("CSignatureDlg::OnBnClickedUseDefaultSignature");
}

// (j.jones 2013-08-08 16:53) - PLID 42958 - added 'skip password'
void CSignatureDlg::LoadDefaultSignature(BOOL bSilent, BOOL bSkipPassword)
{
	CString strFile = VarString(m_pSignatureImageCtrl->GetPictureFileName());
	if(strFile.IsEmpty() || !FileUtils::DoesFileOrDirExist(strFile)) {
		if(!bSilent) {
			MessageBox("You must select an image first.");
		}
		return;
	}

	// (a.walling 2009-12-24 14:37) - PLID 36377 - Get from UsersT
	// (j.jones 2010-04-12 14:08) - PLID 16594 - handle the signature date stamp
	_variant_t varInkData = g_cvarEmpty;
	_variant_t varTextData = g_cvarEmpty;
	BOOL bSignatureDateOnly = FALSE;
	GetDefaultSignatureData(m_nSignatureUserID, varInkData, varTextData, bSignatureDateOnly);
	if((varInkData.vt == VT_NULL || varInkData.vt == VT_EMPTY)
		&& (varTextData.vt == VT_NULL || varTextData.vt == VT_EMPTY)) {
		if(!bSilent) {
			MessageBox("You do not have default signature information saved.");
		}
		return;
	}

	// (z.manning 2008-12-09 09:00) - PLID 32260 - Check and see if we should prompt for the password
	if(m_bCheckPasswordOnLoad && !bSkipPassword) {
		// (j.jones 2013-08-08 15:17) - PLID 42958 - check the signing user's password,
		// not the current user's password
		CString strPrompt = "Please Enter Password";	//normal generic prompt
		if(m_nSignatureUserID != GetCurrentUserID()) {
			//give a specific prompt
			strPrompt.Format("Enter the password for %s", m_strSignatureUserName);
		}
		if(!CheckSpecificUserPassword(m_nSignatureUserID, strPrompt)) {
			return;
		}
	}

	if(varInkData.vt == VT_NULL) {
		varInkData = g_cvarEmpty;
	}

	if(varTextData.vt == VT_NULL) {
		varTextData = g_cvarEmpty;
	}

	m_pSignatureImageCtrl->PutInkData(varInkData);
	// (j.jones 2010-04-12 14:08) - PLID 16594 - handle the signature date stamp
	m_pSignatureImageCtrl->PutTextData(varTextData);
	m_checkSignatureDateNoTime.SetCheck(bSignatureDateOnly);
}

void CSignatureDlg::OnBnClickedSaveDefaultSignature()
{
	try
	{
		SaveDefaultSignature(TRUE);

	}NxCatchAll("CSignatureDlg::OnBnClickedSaveDefaultSignature");
}

void CSignatureDlg::SaveDefaultSignature(BOOL bCheckPassword)
{
	if(bCheckPassword) {
		// (j.jones 2013-08-08 15:17) - PLID 42958 - check the signing user's password,
		// not the current user's password
		CString strPrompt = "Please Enter Password";	//normal generic prompt
		if(m_nSignatureUserID != GetCurrentUserID()) {
			//give a specific prompt
			strPrompt.Format("Enter the password for %s", m_strSignatureUserName);
		}
		if(!CheckSpecificUserPassword(m_nSignatureUserID, strPrompt)) {
			MessageBox("Incorrect password; the signature was not saved.", "Save Failed", MB_OK|MB_ICONWARNING);
			return;
		}
	}

	_variant_t varInkData = m_pSignatureImageCtrl->GetInkData();
	if(varInkData.vt == VT_EMPTY) {
		varInkData.vt = VT_NULL;
	}

	_variant_t varTextData = m_pSignatureImageCtrl->GetTextData();
	if(varTextData.vt == VT_EMPTY) {
		varTextData.vt = VT_NULL;
	}

	BOOL bSignatureDateOnly = m_checkSignatureDateNoTime.GetCheck();

	// (a.walling 2009-12-24 14:46) - PLID 36377 - Set in UsersT
	// (j.jones 2010-04-12 14:08) - PLID 16594 - handle the signature date stamp
	SetDefaultSignatureData(m_nSignatureUserID, varInkData, varTextData, bSignatureDateOnly);

	m_varOriginalSignatureInkData = varInkData;
	m_varOriginalSignatureTextData = varTextData;
}

void CSignatureDlg::OnBnClickedEraseSignatureInk()
{
	try
	{
		m_pSignatureImageCtrl->PutInkData(g_cvarEmpty);
		// (j.jones 2010-04-12 12:23) - PLID 16594 - we now have a date stamp that needs erased
		m_pSignatureImageCtrl->PutTextData(g_cvarEmpty);

	}NxCatchAll("CSignatureDlg::OnBnClickedEraseSignatureInk");
}

void CSignatureDlg::OnBnClickedAutoPromptForSignature()
{
	try
	{
		// (j.jones 2013-08-08 14:04) - PLID 42958 - use the stored signature username, they might not be the logged-in user
		SetRemotePropertyInt("AutoPromptForDefaultSignature", m_nxbtnAutoPromptDefaultSignature.GetCheck(), 0, m_strSignatureUserName);

	}NxCatchAll("CSignatureDlg::OnBnClickedAutoPromptForSignature");
}

void CSignatureDlg::OnClose()
{
	try
	{
		if(!SaveScalingFactor()) {
			return;
		}

		_variant_t varCurrentInk = m_pSignatureImageCtrl->GetInkData();
		_variant_t varCurrentText = m_pSignatureImageCtrl->GetTextData();
		// (z.manning 2008-10-22 17:47) - Prompt to save if the signature changed.
		if(((varCurrentInk.vt != VT_EMPTY && varCurrentInk.vt != VT_NULL)
			|| (varCurrentText.vt != VT_EMPTY && varCurrentText.vt != VT_NULL))
			&& HasSignatureChanged()) {
			CString strMessage = "You have made changes to your signature. Would you like to save it?";
			if(MessageBox(strMessage, "Save Signature", MB_YESNO|MB_ICONQUESTION) == IDYES) {
				SaveDefaultSignature(TRUE);
			}
		}

		EndDialog(IDCANCEL);

	}NxCatchAll("CSignatureDlg::OnClose");
}

BOOL CSignatureDlg::HasSignatureChanged()
{
	CString strCurrentInk = CreateByteStringFromSafeArrayVariant(m_pSignatureImageCtrl->GetInkData());
	CString strOriginalInk = CreateByteStringFromSafeArrayVariant(m_varOriginalSignatureInkData);
	// (j.jones 2010-04-12 12:30) - PLID 16594 - signatures now have a stamp
	CString strCurrentText = CreateByteStringFromSafeArrayVariant(m_pSignatureImageCtrl->GetTextData());
	CString strOriginalText = CreateByteStringFromSafeArrayVariant(m_varOriginalSignatureTextData);
	return strCurrentInk != strOriginalInk || strCurrentText != strOriginalText;
}

void CSignatureDlg::SetText(const CString strText)
{
	m_strText = strText;
}

// (j.jones 2010-04-12 16:15) - PLID 16594 - added option for the date stamp to not include time
void CSignatureDlg::OnCheckSignatureDateNoTime()
{
	try {

		//unlike the save as default signature button, we want this option
		//to save immediately
		g_bSignatureDateOnly = m_checkSignatureDateNoTime.GetCheck();
		// (j.jones 2013-08-08 14:04) - PLID 42958 - use the stored signature user ID, it might not be the logged in user
		ExecuteParamSql("UPDATE UsersT SET SignatureDateOnly = {INT} WHERE PersonID = {INT}",
			g_bSignatureDateOnly ? 1 : 0, m_nSignatureUserID);

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2011-09-23 12:40) - PLID 42648
void CSignatureDlg::OnEnKillfocusSignaturePercentSize()
{
	try
	{
		SaveScalingFactor();
	}
	NxCatchAll(__FUNCTION__);
}

BOOL CSignatureDlg::SaveScalingFactor()
{
	if(!GetDlgItem(IDC_SIGNATURE_PERCENT_SIZE)->IsWindowVisible()) {
		return TRUE;
	}

	// (z.manning 2011-09-23 12:44) - PLID 42648 - Ensure a reasonable value for the scaling factor then save it.
	UINT nScalingFactor = GetDlgItemInt(IDC_SIGNATURE_PERCENT_SIZE);
	if(nScalingFactor < 10 || nScalingFactor > 300) {
		SetDlgItemInt(IDC_SIGNATURE_PERCENT_SIZE, g_nSignatureEmrScaleFactor);
		MessageBox("Please enter a value between 10 and 300", NULL, MB_OK|MB_ICONERROR);
		return FALSE;
	}

	// (j.jones 2013-08-08 14:04) - PLID 42958 - use the stored signature user ID, it might not be the logged in user
	ExecuteParamSql("UPDATE UsersT SET SignatureEmrScaleFactor = {INT} WHERE PersonID = {INT};\r\n"
		, nScalingFactor, m_nSignatureUserID);
	g_nSignatureEmrScaleFactor = nScalingFactor;

	return TRUE;
}

// (d.singleton 2013-4-22 14:32) - PLID 56421 get the signature form the topaz sig pad and display on ink control
void CSignatureDlg::OnBtnClickedTopaz()
{
	try {
		// (a.walling 2014-07-28 10:36) - PLID 62825 - Use TopazSigPad::GetSignatureInk
		MSINKAUTLib::IInkDispPtr pInk = TopazSigPad::GetSignatureInk(this);
		if (!pInk) {
			//no ink or failure,  return
			return;
		}

		// (b.spivey, March 27, 2013) - PLID 30035 - Finally, we have created an ink object to add to the control. 
		m_pSignatureImageCtrl->PutInkData(g_cvarEmpty); 
		m_pSignatureImageCtrl->AddStrokesFromInkWithOffset(_variant_t((LPDISPATCH)pInk), (float)(-0.20), (float)(0.0)); 

	}NxCatchAll(__FUNCTION__);
}