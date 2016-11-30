// (c.haag 2010-02-23 15:38) - PLID 37364 - This source file contains the implementation for the
// Mirror Patient Image Manager class, as well as 

#include "stdafx.h"
#include "MirrorPatientImageMgr.h"
#include "NxCanfieldLink.h"
#include "Mirror.h"

using namespace ADODB;

#pragma region Mirror Patient Image Manager Object

// (c.haag 2010-02-23 15:38) - PLID 37364 - This is a wrapper class for optimized Mirror image functionality.
// Wherever in Practice we load multiple Mirror thumbnails, we should be using this class because it will fliter out
// all Mirror images that were already imported into the history tab...and do so in an optimal way (as opposed to
// running queries and invoking the Canfield SDK on-demand once per image).
CMirrorPatientImageMgr::CMirrorPatientImageMgr(long nPersonID)
{
	// (c.haag 2011-01-20) - PLID 42166 - Assign m_bDetectMirrorImageImportAppPhotos now based on ConfigRT
	m_bDetectMirrorImageImportAppPhotos = (BOOL)GetRemotePropertyInt("MirrorLinkHideSDKImagesImportedFromMirrorImageImportApp", 0, 0, "<None>");

	// Reset local variables that require it
	m_bImageCountSet = FALSE;
	m_nMirrorImageCount = -1;
	m_bMirrorThumbArrayValid = FALSE;

	if (Mirror::IsMirrorEnabled()) {
		Mirror::ECanfieldSDKInitResult result = Mirror::InitCanfieldSDK(TRUE);
		if (Mirror::eCSDK_Failure_Link_SDK_Disabled == result || Mirror::eCSDK_Failure_NotAvailable == result) {
			// All the worker functions will defer to pre-SDK logic in this case. No need to query data
			// or anything from Mirror because we can't import images from clients who don't have the
			// SDK (who have Mirror 6.0 or older)
		} else {
			// (c.haag 2011-01-20) - PLID 42166 - The purpose for this code was to get the thumbnail ID's of all
			// the patient's native Mirror images. We would use this information to search the patient's History tab
			// for any images that were imported as normal jpeg's by the Mirror Image Import app. If we found any,
			// we would have to reduce the Mirror image count by the number of Mirror Image Import imported images.
			//
			// However, this is only needed for clients who are actively in the middle of a Mirror Image Import and who
			// are still using the link. These clients would have m_bDetectMirrorImageImportAppPhotos set to TRUE.
			// Everyone else would have it turned off.
			//
			if (m_bDetectMirrorImageImportAppPhotos) 
			{
				_RecordsetPtr prs = CreateParamRecordset(
					"DECLARE @PersonID INT \r\n"
					"SET @PersonID = {INT} \r\n"
					"SELECT MirrorID FROM PatientsT WHERE PersonID = @PersonID\r\n"
					"SELECT PathName FROM MailSent WHERE MailSent.PersonID = @PersonID \r\n"
					, nPersonID);

				// Load the Mirror ID
				m_strMirrorID = AdoFldString(prs, "MirrorID", "");

				// Load patient MailSent path names
				prs = prs->NextRecordset(NULL);
				FieldsPtr f = prs->Fields;
				while (!prs->eof) {
					m_astrMailSentPathNames.Add(AdoFldString(f, "PathName", ""));
					prs->MoveNext();
				}

				// Load the Mirror thumbnail ID's
				m_bMirrorThumbArrayValid = CanfieldLink::GetThumbnailIDs(m_strMirrorID, m_astrThumbnailIDs);
			}
			else {
				_RecordsetPtr prs = CreateParamRecordset(
					"SELECT MirrorID FROM PatientsT WHERE PersonID = {INT}"
					, nPersonID);

				// Load the Mirror ID
				m_strMirrorID = AdoFldString(prs, "MirrorID", "");
			}
		}
	}
	else {
		// The Mirror link is disabled. Nothing to do.
	}
}

CMirrorPatientImageMgr::CMirrorPatientImageMgr(const CString& strMirrorID)
{
	// (c.haag 2011-01-20) - PLID 42166 - Assign m_bDetectMirrorImageImportAppPhotos now based on ConfigRT
	m_bDetectMirrorImageImportAppPhotos = (BOOL)GetRemotePropertyInt("MirrorLinkHideSDKImagesImportedFromMirrorImageImportApp", 0, 0, "<None>");

	// Reset local variables that require it
	m_bImageCountSet = FALSE;
	m_nMirrorImageCount = -1;
	m_bMirrorThumbArrayValid = FALSE;
	m_strMirrorID = strMirrorID;

	if (Mirror::IsMirrorEnabled()) {
		Mirror::ECanfieldSDKInitResult result = Mirror::InitCanfieldSDK(TRUE);
		if (Mirror::eCSDK_Failure_Link_SDK_Disabled == result || Mirror::eCSDK_Failure_NotAvailable == result) {
			// All the worker functions will defer to pre-SDK logic in this case. No need to query data
			// or anything from Mirror because we can't import images from clients who don't have the
			// SDK (who have Mirror 6.0 or older)
		} else {
			// (c.haag 2011-01-20) - PLID 42166 - The purpose for this code was to get the thumbnail ID's of all
			// the patient's native Mirror images. We would use this information to search the patient's History tab
			// for any images that were imported as normal jpeg's by the Mirror Image Import app. If we found any,
			// we would have to reduce the Mirror image count by the number of Mirror Image Import imported images.
			//
			// However, this is only needed for clients who are actively in the middle of a Mirror Image Import and who
			// are still using the link. These clients would have m_bDetectMirrorImageImportAppPhotos set to TRUE.
			// Everyone else would have it turned off.
			//
			if (m_bDetectMirrorImageImportAppPhotos) 
			{
				_RecordsetPtr prs = CreateParamRecordset(
					"DECLARE @MirrorID NVARCHAR(17) \r\n"
					"SET @MirrorID = {STRING} \r\n"
					"SELECT PathName FROM MailSent "
					"INNER JOIN PatientsT ON PatientsT.PersonID = MailSent.PersonID "
					"WHERE PatientsT.MirrorID IS NOT NULL AND PatientsT.MirrorID = @MirrorID \r\n"
					, strMirrorID);

				// Load patient MailSent path names
				FieldsPtr f = prs->Fields;
				while (!prs->eof) {
					m_astrMailSentPathNames.Add(AdoFldString(f, "PathName", ""));
					prs->MoveNext();
				}
				prs->Close();

				// Load the Mirror thumbnail ID's
				m_bMirrorThumbArrayValid = CanfieldLink::GetThumbnailIDs(m_strMirrorID, m_astrThumbnailIDs);
			}
		}
	}
	else {
		// The Mirror link is disabled. Nothing to do.
	}
}

CMirrorPatientImageMgr::CMirrorPatientImageMgr(CMirrorPatientImageMgr* pSrc)
{
	if (NULL != pSrc) {
		m_strMirrorID = pSrc->m_strMirrorID;
		m_astrMailSentPathNames.Copy( pSrc->m_astrMailSentPathNames );
		m_astrThumbnailIDs.Copy( pSrc->m_astrThumbnailIDs );
		m_bMirrorThumbArrayValid = pSrc->m_bMirrorThumbArrayValid;
		m_bImageCountSet = pSrc->m_bImageCountSet;
		m_nMirrorImageCount = pSrc->m_nMirrorImageCount;
		m_bDetectMirrorImageImportAppPhotos = pSrc->m_bDetectMirrorImageImportAppPhotos; // (c.haag 2011-01-20) - PLID 42166
	}
	else {
		ThrowNxException("Tried to create a CMirrorPatientImageMgr object from a NULL pointer!");
	}
}

// Returns the count of Mirror images for this patient. This result is adjusted for images imported from Mirror; if a patient
// has 10 mirror photos and 6 were imported into MailSent, this function would return a count of 4.
long CMirrorPatientImageMgr::GetImageCount()
{
	if (!m_bImageCountSet) {
		if (m_bMirrorThumbArrayValid) {
			m_nMirrorImageCount = Mirror::GetImageCount(m_strMirrorID, &m_astrThumbnailIDs, &m_astrMailSentPathNames);
		} else {
			m_nMirrorImageCount = Mirror::GetImageCount(m_strMirrorID, NULL, &m_astrMailSentPathNames);
		}
		m_bImageCountSet = TRUE;
	}
	return m_nMirrorImageCount;
}

// Loads a Mirror image given an index. The index should be between 0 and the number of Mirror images for this patient
// which were not imported by the Mirror image import app.
HBITMAP CMirrorPatientImageMgr::LoadMirrorImage(long &nIndex, long &nCount, long nQualityOverride)
{
	if (m_bMirrorThumbArrayValid) {
		return Mirror::LoadMirrorImage(m_strMirrorID, nIndex, nCount, nQualityOverride, &m_astrThumbnailIDs, &m_astrMailSentPathNames);
	} else {
		return Mirror::LoadMirrorImage(m_strMirrorID, nIndex, nCount, nQualityOverride, NULL, &m_astrMailSentPathNames);
	}
}

// Returns the first valid Mirror image index for this patient
long CMirrorPatientImageMgr::GetFirstValidImageIndex()
{
	return Mirror::GetFirstValidImageIndex(m_strMirrorID);
}

// Returns the last valid Mirror image index for this patient
long CMirrorPatientImageMgr::GetLastValidImageIndex()
{
	if (m_bMirrorThumbArrayValid) {
		return Mirror::GetLastValidImageIndex(m_strMirrorID, &m_astrThumbnailIDs, &m_astrMailSentPathNames);
	} else {
		return Mirror::GetLastValidImageIndex(m_strMirrorID,  NULL, &m_astrMailSentPathNames);
	}
}

#pragma endregion


namespace Mirror
{
	// (c.haag 2010-02-22 17:33) - PLID 37364 - Overload for not providing a list of Mirror thumbnail ID's or MailSent.PathNames
	long GetImageCount(const CString &strMirrorId)
	{
		return GetImageCount(strMirrorId, NULL, NULL);
	}

	// (c.haag 2009-04-01 16:06) - PLID 33630 - Removed bForceSynchronousOpen. We always do
	// this synchronously now.
	// (c.haag 2010-02-22 17:34) - PLID 37364 - Overload that takes in an array of patient Mirror thumbnail ID's
	// and MailSent.PathName's. Also, from now on, this function returns the count of Mirror images that were not
	// imported by the Mirror image import app.
	long GetImageCount(const CString &strMirrorId, const CStringArray* apAllPatientThumbnailIDs, const CStringArray* apAllPatientMailSentPathNames)
	{
		// (c.haag 2009-03-31 14:55) - PLID 33630 - Use new logic to test Mirror connectivity
		if (!IsMirrorEnabled()) {
			return -1;
		}
		ECanfieldSDKInitResult result = InitCanfieldSDK(TRUE);
		switch (result) {
			case eCSDK_Failure_Link_SDK_Disabled:
			case eCSDK_Failure_NotAvailable:
				// Defer to pre-SDK logic
				return GetImageCount_PreCanfieldSDK(strMirrorId);
			case eCSDK_Failure_Link_Disabled:
			case eCSDK_Failure_TimeoutExpired:
			case eCSDK_Failure_Error:
				// Failure
				return -1;
			default:
				break;
		}
		
		// (c.haag 2010-02-22 14:50) - PLID 37364 - If the patient corresponding to recnum had
		// one or more Mirror images imported into NexTech Practice, then we need to reduce the
		// count by the number of imported images. This is an attempt to completely hide them from
		// Practice so that users don't see duplicate images.
		//
		CStringArray astrThumbnailIDs;
		CStringArray astrAllPatientMailSentPathNames;

		// (c.haag 2011-01-20) - PLID 42166 - This is TRUE if we support advanced calculations to determine how many Mirror images
		// already exist in Practice as history files imported and attached by the MirrorImageImport app. If FALSE, we use legacy behavior
		// to simply return the unfiltered Mirror image count.
		BOOL bDetectMirrorImageImportAppPhotos = (BOOL)GetRemotePropertyInt("MirrorLinkHideSDKImagesImportedFromMirrorImageImportApp", 0, 0, "<None>");
		if (!bDetectMirrorImageImportAppPhotos) {
			return CanfieldLink::GetImageCount(strMirrorId);
		}

		// First, get a list of all patient thumbnail ID's
		if (NULL != apAllPatientThumbnailIDs) {
			astrThumbnailIDs.Copy(*apAllPatientThumbnailIDs);
		} else {
			if (!CanfieldLink::GetThumbnailIDs(strMirrorId, astrThumbnailIDs)) {			
				// An internal failure occurred. Defer to legacy functionality and quit.
				return CanfieldLink::GetImageCount(strMirrorId);
			}
		}

		// Now get a list of all patient MailSent.PathName values.
		if (NULL != apAllPatientMailSentPathNames) {
			astrAllPatientMailSentPathNames.Copy(*apAllPatientMailSentPathNames);
		} else {
			_RecordsetPtr prs = CreateParamRecordset("SELECT PathName FROM MailSent "
				"INNER JOIN PatientsT ON PatientsT.PersonID = MailSent.PersonID "
				"WHERE PatientsT.MirrorID IS NOT NULL AND PatientsT.MirrorID = {STRING} "
				, strMirrorId);
			FieldsPtr f = prs->Fields;

			// Do for each MailSent record
			while (!prs->eof) {
				astrAllPatientMailSentPathNames.Add( AdoFldString(f, "PathName", "") );
				prs->MoveNext();
			}
		}

		// Now destroy all the astrThumbnailIDs elements that exist in the MailSent.PathName array.
		// This will consequently leave astrThumbnailIDs with only available Mirror thumbnail ID's.
		for (int i=0; i < astrAllPatientMailSentPathNames.GetSize(); i++) {
			CString strPathName = astrAllPatientMailSentPathNames[i];

			// Every image was imported as a JPEG. If this record is a JPEG, then check the thumbnail
			// ID list for it. If it exists, remove it from the list.
			if (strPathName.GetLength() > 4 && !strPathName.Right(4).CompareNoCase(".jpg")) {
				strPathName = strPathName.Left( strPathName.GetLength() - 4);

				for (int j=0; j < astrThumbnailIDs.GetSize(); j++) {
					if (astrThumbnailIDs[j] == strPathName) {
						// If we get here, this Mirror thumbnail exists in the patient's MailSent record set.
						// Rremove the ID from the thumbnail ID array.
						astrThumbnailIDs.RemoveAt(j);
						break;
					}
				}
			}
		}

		// This will give us our true count
		int nImageCount = astrThumbnailIDs.GetSize();

		// (c.haag 2006-10-23 11:00) - PLID 23181 - This functionality is now depreciated.
		// If we are viewing primary images only as dictated by the patients module view
		// options, this function should not even be called.

		/*// If we only want to see the primary image of a Mirror patient,
		// we either return a 1 or a 0 because that's all we get.
		if (GetRemotePropertyInt("MirrorPrimaryImageOnly", FALSE))
		{
			if (nImageCount > 0)
				nImageCount = 1;
		}*/
		return nImageCount;
	}

	// (c.haag 2010-02-22 17:33) - PLID 37364 - Overload for not providing a list of Mirror thumbnail ID's or MailSent.PathNames
	HBITMAP LoadMirrorImage(const CString &recnum, IN OUT long &nIndex, OUT long &nCount, long nQualityOverride)
	{
		return LoadMirrorImage(recnum, nIndex, nCount, nQualityOverride, NULL, NULL);
	}

	// (c.haag 2008-06-19 09:33) - PLID 28886 - Added nQualityOverride
	// (c.haag 2009-04-01 16:06) - PLID 33630 - Removed bForceSynchronousOpen and default parameters
	// (c.haag 2010-02-22 17:34) - PLID 37364 - Overload that takes in an array of patient Mirror thumbnail ID's
	// and MailSent.PathName's. Also, from now on, nIndex exists in the range that only includes Mirror thumbnails
	// that were not imported by the Mirror image import app.
	HBITMAP LoadMirrorImage(const CString &recnum, IN OUT long &nIndex, OUT long &nCount, long nQualityOverride, 
		const CStringArray* apAllPatientThumbnailIDs, const CStringArray* apAllPatientMailSentPathNames)
	{
		// (c.haag 2009-04-01 16:07) - PLID 33630 - Use new logic to handle Mirror connectivity
		if (!IsMirrorEnabled()) {
			return NULL;
		}
		ECanfieldSDKInitResult result = InitCanfieldSDK(TRUE);
		switch (result) {
			case eCSDK_Failure_Link_SDK_Disabled:
			case eCSDK_Failure_NotAvailable:
				// Defer to pre-SDK logic
				return LoadMirrorImage_PreCanfieldSDK(recnum, nIndex, nCount, nQualityOverride);
			case eCSDK_Failure_Link_Disabled:
			case eCSDK_Failure_TimeoutExpired:
			case eCSDK_Failure_Error:
				// Failure
				return NULL;
			default:
				break;
		}

		//index::in is the requested index of the image (images are ordered by the create time)
		//index::out is the actual index used, in case we could not find the requested image
		//index is 0 based - 0 being the first image
		//index of -1 explicitly means invalid, but all negative indexes are also invalid
		//count is the total number of images - 1 image means index = 0 and count = 1

		// (c.haag 2008-06-19 09:33) - PLID 28886 - Calculate the quality
		long nQuality;
		if (-1 != nQualityOverride) {
			// Use the caller's quality
			nQuality = nQualityOverride;
		} else {
			// Use the Practice-defined quality
			nQuality = GetPropertyInt("MirrorImageDisplay", GetPropertyInt("MirrorShowImages", 1));
		}

		if (MIRROR_INDEX_PRIMARY_THUMBNAIL == nIndex) {
			return CanfieldLink::LoadPrimaryImage(recnum, nQuality);
		} else {
			// (c.haag 2010-02-22 14:50) - PLID 37364 - If the patient corresponding to recnum had
			// one or more Mirror images imported into NexTech Practice, then we need to offset the index
			// by the number of imported images between thumbnail 0 and thumbnail index.
			//
			// For example, say a patient has 8 photos in Mirror, and 6 were imported. Lets say if you
			// gathered an array of all Mirror thumbnail ID's, the spread is:
			//
			//		0 1 2 3 4 5 6 7
			//		x x x _ x x _ x
			//
			// Where an x designates an imported photo, and a _ designates a photo that was not imported
			// into Practice. We want it so an input index of 0 corresponds to the first _; which is 3...and an
			// input index of 1 corresponds to the second _, which is 6.
			//
			// If we cannot get this adjusted index due to some unforeseen error, then we defer to the legacy
			// behavior.
			//
			CStringArray astrThumbnailIDs;
			CStringArray astrAllPatientThumbnailIDs;
			CStringArray astrAllPatientMailSentPathNames;

			// (c.haag 2011-01-20) - PLID 42166 - This is TRUE if we support advanced calculations to determine how many Mirror images
			// already exist in Practice as history files imported and attached by the MirrorImageImport app. If FALSE, we use legacy behavior
			// to simply return the unfiltered Mirror image count.
			BOOL bDetectMirrorImageImportAppPhotos = (BOOL)GetRemotePropertyInt("MirrorLinkHideSDKImagesImportedFromMirrorImageImportApp", 0, 0, "<None>");
			if (!bDetectMirrorImageImportAppPhotos) {
				return CanfieldLink::LoadImage(recnum, nIndex, nCount, nQuality);
			}

			// First, get a list of all patient thumbnail ID's
			if (NULL != apAllPatientThumbnailIDs) {
				astrThumbnailIDs.Copy(*apAllPatientThumbnailIDs);
			} else {
				if (!CanfieldLink::GetThumbnailIDs(recnum, astrThumbnailIDs)) {			
					// An internal failure occurred. Defer to legacy functionality and quit.
					return CanfieldLink::LoadImage(recnum, nIndex, nCount, nQuality);
				}
			}
			astrAllPatientThumbnailIDs.Copy( astrThumbnailIDs );

			// Now get a list of all patient MailSent.PathName values.
			if (NULL != apAllPatientMailSentPathNames) {
				astrAllPatientMailSentPathNames.Copy(*apAllPatientMailSentPathNames);
			} else {
				_RecordsetPtr prs = CreateParamRecordset("SELECT PathName FROM MailSent "
					"INNER JOIN PatientsT ON PatientsT.PersonID = MailSent.PersonID "
					"WHERE PatientsT.MirrorID IS NOT NULL AND PatientsT.MirrorID = {STRING} "
					, recnum);
				FieldsPtr f = prs->Fields;

				// Do for each MailSent record
				while (!prs->eof) {
					astrAllPatientMailSentPathNames.Add( AdoFldString(f, "PathName", "") );
					prs->MoveNext();
				}
			}

			// Now destroy all the astrThumbnailIDs elements that exist in the MailSent.PathName array.
			// This will consequently leave astrThumbnailIDs with only available Mirror thumbnail ID's.
			for (int i=0; i < astrAllPatientMailSentPathNames.GetSize(); i++) {
				CString strPathName = astrAllPatientMailSentPathNames[i];

				// Every image was imported as a JPEG. If this record is a JPEG, then check the thumbnail
				// ID list for it. If it exists, remove it from the list.
				if (strPathName.GetLength() > 4 && !strPathName.Right(4).CompareNoCase(".jpg")) {
					strPathName = strPathName.Left( strPathName.GetLength() - 4);

					for (int j=0; j < astrThumbnailIDs.GetSize(); j++) {
						if (astrThumbnailIDs[j] == strPathName) {
							// If we get here, this Mirror thumbnail exists in the patient's MailSent record set.
							// Rremove the ID from the thumbnail ID array.
							astrThumbnailIDs.RemoveAt(j);
							break;
						}
					}
				}
			}

			// Update the count "out" parameter
			nCount = astrThumbnailIDs.GetSize();

			// Now clamp the index to the bounds of astrThumbnailIDs.
			if (nIndex < 0) { nIndex = 0; }
			if (nIndex >= astrThumbnailIDs.GetSize()) { nIndex = astrThumbnailIDs.GetSize() - 1; }
			if (nIndex < 0) {
				return NULL; // The index is invalid
			}

			// Now get the thumbnail ID corresponding to the index (again, the index exists in the range that only includes 
			// Mirror thumbnails that were not imported by the Mirror image import app)
			const CString strDesiredThumbID = astrThumbnailIDs[nIndex];

			// Now find its location in the "All thumbnail" array and change nIndex accordingly
			for (nIndex=0; astrAllPatientThumbnailIDs[nIndex] != strDesiredThumbID; nIndex++);
			if (nIndex == astrAllPatientThumbnailIDs.GetSize()) {
				// Not sure how this could happen; but if we get here, the thumbnail was not found.
				return NULL;
			}
			else {
				// nIndex is now the index to the thumbnail as it is in the "All thumbnail" list. Now we
				// can FINALLY pass it into the CanfieldLink LoadImage function and call it a day.
				HBITMAP hBmp = CanfieldLink::LoadImage(recnum, nIndex, nCount, nQuality);
				return hBmp;
			}
		}
	}

	long GetFirstValidImageIndex(const CString &strMirrorId)
	{
		// (c.haag 2009-03-31 12:38) - PLID 33630 - New logic for ensuring we can query Mirror data directly
		if (!IsMirrorEnabled()) {
			return -1;
		} else switch (InitCanfieldSDK(TRUE)) {
			case eCSDK_Failure_Link_SDK_Disabled:
			case eCSDK_Failure_NotAvailable:
				// Defer to pre-SDK logic (this is what we want)
				// (c.haag 2010-02-24 10:32) - PLID 37364 - Moved Mirror 6.0 (pre-SDK) logic into its own function
				return GetFirstValidImageIndex_PreCanfieldSDK(strMirrorId);
			default:
				// Client is running (or trying to run) the Canfield SDK. Default to 0
				return 0;
		}
	}

	// (c.haag 2010-02-24 10:30) - PLID 37364 - We now take in apAllPatientThumbnailIDs and apAllPatientMailSentPathNames
	long GetLastValidImageIndex(const CString &strMirrorId, const CStringArray* apAllPatientThumbnailIDs, const CStringArray* apAllPatientMailSentPathNames)
	{
		if (strMirrorId.IsEmpty())
			return -1;

		// (c.haag 2009-03-31 12:38) - PLID 33630 - New logic for ensuring we can query Mirror data directly
		if (!IsMirrorEnabled()) {
			return -1;
		} else switch (InitCanfieldSDK(TRUE)) {
			case eCSDK_Failure_Link_SDK_Disabled:
			case eCSDK_Failure_NotAvailable:
				// Defer to pre-SDK logic
				return GetLastValidImageIndex_PreCanfieldSDK(strMirrorId);
				break;
			case eCSDK_Success:
				// Defer to SDK logic
				// (c.haag 2010-02-23 13:21) - PLID 37364 - Do this at the Mirror namespace level, not the SDK level
				return GetImageCount(strMirrorId, apAllPatientThumbnailIDs, apAllPatientMailSentPathNames) - 1;
			default:
				// Client is trying to run the Canfield SDK and failing
				return -1;
		}
	}

}
