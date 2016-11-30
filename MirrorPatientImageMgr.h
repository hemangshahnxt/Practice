#pragma once

// (c.haag 2010-02-23 15:38) - PLID 37364 - This is a wrapper class for optimized Mirror image functionality.
// Wherever in Practice we load multiple Mirror thumbnails, we should be using this class because it will fliter out
// all Mirror images that were already imported into the history tab...and do so in an optimal way (as opposed to
// running queries and invoking the Canfield SDK on-demand once per image).
class CMirrorPatientImageMgr
{
private:
	// (c.haag 2011-01-20) - PLID 42166 - Set to TRUE if we want to suppress any images stored in Mirror that 
	// were already imported from the MirrorImageImport app. This feature makes the link run significantly slower,
	// though. The more photos the patient has in Mirror, the worse the slowness.
	BOOL m_bDetectMirrorImageImportAppPhotos;

private:
	CString m_strMirrorID; // The patient's Mirror ID
	CStringArray m_astrMailSentPathNames; // The list of MailSent.PathName records for this patient
	CStringArray m_astrThumbnailIDs; // The array of patient thumbnail ID's
	BOOL m_bMirrorThumbArrayValid; // TRUE if m_astrThumbnailIDs is valid; FALSE if an error occurred trying to populate it 

private:
	long m_nMirrorImageCount; // The number of Mirror images for this patient adjusted for images imported from Mirror; if a patient
		// has 10 mirror photos and 6 were imported, this value would be 4.
	BOOL m_bImageCountSet; // Set to TRUE when m_nMirrorImageCount is set

public:
	CMirrorPatientImageMgr(long nPersonID);
	CMirrorPatientImageMgr(const CString& strMirrorID);
	CMirrorPatientImageMgr(CMirrorPatientImageMgr* pSrc);

public:
	// Returns the count of Mirror images for this patient. This result is adjusted for images imported from Mirror; if a patient
	// has 10 mirror photos and 6 were imported into MailSent, this function would return a count of 4.
	long GetImageCount();

	// Loads a Mirror image given an index. The index should be between 0 and the number of Mirror images for this patient
	// which were not imported by the Mirror image import app.
	HBITMAP LoadMirrorImage(long &nIndex, long &nCount, long nQualityOverride);

	// Returns the first valid Mirror image index for this patient
	long GetFirstValidImageIndex();

	// Returns the last valid Mirror image index for this patient
	long GetLastValidImageIndex();
};