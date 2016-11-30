#include "StdAfx.h"

#include <NxMD5.h>

#include "NxImageImpl.h"

// (a.walling 2013-06-27 13:21) - PLID 57348 - NxImageLib handles the cache

namespace NxImageLib
{
namespace Cache
{
	// (a.walling 2013-06-06 09:42) - PLID 57070 - Amortize image load CPU across network IO
	// Basically this creates a file mapping on the local machine the same size
	// as the file we are transferring, and opens the remote file with the
	// sequential scan hint. Any read operations to this object will be blocked
	// waiting for the local file's data to sequentially catch up to the file pointer.
	// Windows will be smart with the sequential scan hint and start caching ahead
	// within the system anyway. For the most part the reads are entirely sequential anyway 
	// except for a few jumps at the very beginning of png/jpeg
	class CxCachingIOFile : public CxFile
	{
	public:
		CxCachingIOFile(const CString& strRemoteFile, const CString& strLocalFile, const WIN32_FILE_ATTRIBUTE_DATA& attribs)
			: remote_(
				::CreateFile(strRemoteFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL)
			)
			, local_(
				::CreateFile(strLocalFile, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)
			)
			, fileMapping_(
				::CreateFileMapping(local_, NULL, PAGE_READWRITE, 0, attribs.nFileSizeLow, NULL)
			)
			, mappedView_(
				(BYTE*)::MapViewOfFile(fileMapping_, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, attribs.nFileSizeLow)
			)
			, pBegin_(mappedView_.Get())
			, pEnd_(pBegin_ + attribs.nFileSizeLow)
			, pPos_(pBegin_)
			, pValidEnd_(pBegin_)
			, error_(0)
			, attribs_(attribs)
		{
			ASSERT(!attribs_.nFileSizeHigh);
		}

		~CxCachingIOFile()
		{
			Close();
		}

		bool IsOpen()
		{
			return remote_ && pBegin_;
		}

		virtual bool Close()
		{
			// copy any remaining data
			if (pEnd_ > pValidEnd_) {
				CommitRemote(pValidEnd_, pEnd_ - pValidEnd_);
				ASSERT(pValidEnd_ == pEnd_);
			}

			// order of destruction is important here
			// unmap the view
			mappedView_.Close();
			// and close the file mapping
			fileMapping_.Close();

			if (local_) {
				BOOL ok = ::SetFileTime(local_, NULL, NULL, &attribs_.ftLastWriteTime);
				ASSERT(ok);
			}

			return true;
		}

		size_t CommitRemote(BYTE* commitBegin, size_t len)
		{
			BYTE* commitEnd = pPos_ + len;
			
			if (commitEnd > pEnd_) {
				commitEnd = pEnd_;
			}

			if (commitEnd > pValidEnd_) {
				// round to nearest 16kb (SMB blocks usually in 16kb anyway)
				static const size_t readPageSize = 0x4000;
				size_t readLen = commitEnd - pValidEnd_;
				readLen += (readPageSize - 1);
				readLen -= (readLen % readPageSize);

				BYTE* readEnd = pValidEnd_ + readLen;
				if (readEnd > pEnd_) {
					readEnd = pEnd_;
					readLen = readEnd - pValidEnd_;
				}
				
				DWORD bytesRead = 0;
				BOOL ok = ::ReadFile(remote_, pValidEnd_, readLen, &bytesRead, NULL);
				ASSERT(ok);
				pValidEnd_ += bytesRead;
			}

			if (commitBegin > pValidEnd_) {
				return 0;
			} else {
				return pValidEnd_ - commitBegin;
			}
		}

		virtual size_t Read(void *buffer, size_t size, size_t count)
		{
			if (!size || !count) return 0;

			size_t readLen = size * count;
			size_t commitAvail = CommitRemote(pPos_, readLen);
			if (readLen > commitAvail) {
				readLen = commitAvail;
			}

			memcpy(buffer, pPos_, readLen);
			pPos_ += readLen;

			return (size_t)(readLen / size);
		}

		// this is only for reading!
		virtual size_t Write(const void *buffer, size_t size, size_t count)
		{ throw std::invalid_argument(__FUNCTION__" not supported"); return 0; }

		virtual bool Seek(int32_t offset, int32_t origin)
		{
			switch (origin) {
				case SEEK_SET:
					pPos_ = pBegin_ + offset;
					break;
				case SEEK_CUR:
					pPos_ += offset;
					break;
				case SEEK_END:
					pPos_ = pEnd_ + offset;
					break;
			}

			if (pPos_ < pBegin_) {
				pPos_ = pBegin_;
				error_ = EINVAL;
				return false;
			} else if (pPos_ > pEnd_) {
				pPos_ = pEnd_;
				error_ = EINVAL;
				return false;
			} else {
				return true;
			}
		}

		virtual int32_t Tell()
		{ return (int32_t)(pPos_ - pBegin_); }

		virtual int32_t	Size()
		{ return (int32_t)(pEnd_ - pBegin_); }

		virtual bool	Flush()
		{ return pPos_ != NULL; }

		virtual bool	Eof()
		{ return pPos_ >= pEnd_; }

		virtual int32_t	Error()
		{ return (int32_t)error_; }

		// this is only for reading!
		virtual bool PutC(uint8_t c)
		{ throw std::invalid_argument(__FUNCTION__" not supported"); return false; }

		// used by EXIF
		virtual int32_t	GetC()
		{
			size_t commitAvail = CommitRemote(pPos_, 1);
			if (Eof()) return EOF;
			if (!commitAvail) {
				error_ = EIO;
				return EOF;
			}

			BYTE c = *pPos_;
			++pPos_;
			return c;
		}

		// only used by PSD, RAW, which we don't support
		virtual char *	GetS(char *string, int32_t n)
		{ throw std::invalid_argument(__FUNCTION__" not supported"); return NULL; }

		// only used by PSD, RAW, which we don't support
		virtual int32_t	Scanf(const char *format, void* output)
		{ throw std::invalid_argument(__FUNCTION__" not supported"); return 0; }

	protected:
		Nx::Handle remote_;
		Nx::Handle local_;
		Nx::Handle fileMapping_;
		Nx::MappedView mappedView_;

		BYTE* pBegin_;
		BYTE* pEnd_;
		BYTE* pPos_;
		BYTE* pValidEnd_;

		DWORD error_;

		WIN32_FILE_ATTRIBUTE_DATA attribs_;
	};

	///

	CString CalcLocalCachedFileName(CString strRemotePath)
	{
		strRemotePath.TrimLeft();
		strRemotePath.TrimRight();
		strRemotePath.MakeLower();
		NxMD5 md5(strRemotePath);

		// ah curses, Base64 is case-sensitive, derp!
		//CString strHashedName = CBase64::Encode((unsigned char*)md5.m_digest, sizeof(md5.m_digest));
		CString strHashedName = GetByteArrayAsHexString((BYTE*)md5.m_digest, sizeof(md5.m_digest), "");

		strHashedName += ".";
		strHashedName += FileUtils::GetFileExtension(strRemotePath);
		
		// (a.walling 2011-08-24 12:45) - PLID 45172 - Use Practice's GetLocalNxTempPath
		//strLocalPath = GetNxTempPath() ^ strHashedName;
		return GetLocalNxTempPath("ImgCache") ^ strHashedName;
	}

	// finds an existing image and verifies its timestamp; returns "" if invalid
	CString FindExistingLocalCachedImage(const CString& strRemotePath, WIN32_FILE_ATTRIBUTE_DATA& remoteAttributes)
	{
		bool bIsDiagram = false;

		// (a.walling 2013-03-07 08:41) - PLID 55496 - EMR diagram images do not need to constantly cause network access
		CString strBaseImagesPath = GetSharedPath() / "Images";
		strBaseImagesPath.MakeLower();
		if (-1 != strRemotePath.Find(strBaseImagesPath)) {
			bIsDiagram = true;
		} else {
			// we will need the date info
			if (!::GetFileAttributesEx(strRemotePath, GetFileExInfoStandard, &remoteAttributes)) {
				remoteAttributes.dwFileAttributes = INVALID_FILE_ATTRIBUTES;
			}
		}

		CString strLocalPath = CalcLocalCachedFileName(strRemotePath);

		WIN32_FILE_ATTRIBUTE_DATA localAttributes = {0};

		// make sure it exists on disk
		if (!::GetFileAttributesEx(strLocalPath, GetFileExInfoStandard, &localAttributes)) {
			localAttributes.dwFileAttributes = INVALID_FILE_ATTRIBUTES;
		} else if (localAttributes.nFileSizeHigh == 0 && localAttributes.nFileSizeLow == 0) {
			// empty file? delete it
			localAttributes.dwFileAttributes = INVALID_FILE_ATTRIBUTES;
			::DeleteFile(strLocalPath);
		}

		if (INVALID_FILE_ATTRIBUTES == localAttributes.dwFileAttributes) {
			// nope
			// (a.walling 2013-06-10 17:34) - PLID 57070 - if a diagram, need to get the remote attributes now
			if (bIsDiagram) {
				if (!::GetFileAttributesEx(strRemotePath, GetFileExInfoStandard, &remoteAttributes)) {
					remoteAttributes.dwFileAttributes = INVALID_FILE_ATTRIBUTES;
				}
			}

			return "";
		}

		// (a.walling 2013-03-07 08:41) - PLID 55496 - EMR diagram images do not need to constantly cause network access
		if (bIsDiagram) {
			// assume up to date
			return strLocalPath;
		}
		
		if ( 
			(INVALID_FILE_ATTRIBUTES == remoteAttributes.dwFileAttributes) ||
			(remoteAttributes.nFileSizeHigh	!=	localAttributes.nFileSizeHigh)	||
			(remoteAttributes.nFileSizeLow	!=	localAttributes.nFileSizeLow)	||
			(FileUtils::CompareFileModifiedTimes(remoteAttributes.ftLastWriteTime, localAttributes.ftLastWriteTime) != 0)
			)
		{
			::DeleteFile(strLocalPath);

			// As per existing behavior, no throws if invalid, just return an empty string
			LogDetail("Cached file for %s invalidated", strRemotePath);

			return "";
		}

		return strLocalPath;
	}

	// (a.walling 2010-10-25 10:04) - PLID 41043 - Copy the remote file to the local machine if necessary
	// (a.walling 2013-06-06 09:42) - PLID 57070 - Amortize image load CPU across network IO
	// returns local path to cached file, or empty if failure
	CString CacheAndLoadFileOnLocalMachine(const CString& strRemotePath, OUT CxImage& img)
	{
		CNxPerform nxp(__FUNCTION__);

		WIN32_FILE_ATTRIBUTE_DATA remoteAttributes = {0};
		CString strLocalPath = FindExistingLocalCachedImage(strRemotePath, remoteAttributes);

		if (!strLocalPath.IsEmpty())
		{
			// already on local machine, just load it
			long nFormatID = GuessCxFormatFromExtension(strRemotePath);
			if (img.Load(strLocalPath, nFormatID)) {
				return strLocalPath;
			}
			else if (img.Load(strLocalPath)) {
				return strLocalPath;
			}
			else {
				// (z.manning 2014-07-03 14:56) - PLID 62666 - We used to throw an exception here. Now we simply
				// delete the unloadable cached file and use the one from the server.
				::DeleteFile(strLocalPath);
				strLocalPath.Empty();
			}
		}

		// if the remote file was invalid, we just return an empty string
		if (INVALID_FILE_ATTRIBUTES == remoteAttributes.dwFileAttributes || remoteAttributes.nFileSizeHigh != 0 || remoteAttributes.nFileSizeLow == 0) {
			// also return an empty string if an empty file or a file > 4gb
			return "";
		}

		// otherwise, needs to be cached locally and loaded
		strLocalPath = CalcLocalCachedFileName(strRemotePath);

		nxp.Tick("Caching and loading %s", strRemotePath);

		{
			// this will allow the CPU load to be interleaved with the network IO
			// which effectively causes the time to load to become
			// max(ioTime, cpuTime) rather than ioTime + cpuTime
			CxCachingIOFile cachingFile(strRemotePath, strLocalPath, remoteAttributes);
			if (!cachingFile.IsOpen()) {
				DWORD lastError = ::GetLastError();
				ThrowNxException("Error 0x%08x creating caching IO file for `%s` and local path `%s`", lastError, strRemotePath, strLocalPath);
			}

			long nFormatID = GuessCxFormatFromExtension(strRemotePath);
			if (!img.Decode(&cachingFile, nFormatID)) {

				cachingFile.Seek(0, SEEK_SET);

				if (!img.Decode(&cachingFile, CXIMAGE_FORMAT_UNKNOWN)) {
					cachingFile.Close();
					::DeleteFile(strLocalPath);
					ThrowNxException("Failed to load image `%s` from path `%s` during cache", strRemotePath, strLocalPath);
				}
			}
		}

		if (!strLocalPath.IsEmpty()) {
			::MoveFileEx(strLocalPath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
		}

		return strLocalPath;
	}
}
}