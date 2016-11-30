#include "stdafx.h"
#include "TopazSigPad.h"
#include <NxTaskDialog.h>
#include <NxSystemUtilitiesLib/NxHandle.h>
#include <sstream>

#define BOOST_ASIO_DISABLE_BOOST_REGEX
#include <boost/asio/streambuf.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/asio/windows/stream_handle.hpp>

#include <atlsecurity.h>

// (a.walling 2014-07-28 10:29) - PLID 63067 - Nx::Redirected 
// spawns child processes and redirects stdin, stdout, stderr 
// asynchronously with overlapped IO via named pipes.
namespace Nx
{
	namespace Redirected
	{
		// holds both the read and write end of the pipe
		struct Pipe
		{
			Handle r;
			Handle w;
		};

		long nextSerialNum = 0;

		// returns a unique object name
		CString MakeObjectName(const char* objectName)
		{
			long serialNum = ::InterlockedIncrement(&nextSerialNum);
			return FormatString(
				"Nextech$%s_%lu_%lu"
				, objectName
				, GetCurrentProcessId()
				, serialNum
			);
		}

		// creates a named pipe and returns handles to the read end and write end; inherited flag is set as appropriate.
		// inherited ends of the pipe will NOT be opened in overlapped mode, since console functions expect synchronicity.
		int CreateInheritablePipePair(Pipe& pipe, const char* objectName, bool inheritRead, bool inheritWrite, const SECURITY_DESCRIPTOR* pSecurityDesc)
		{
			// (a.walling 2014-09-02 12:34) - PLID 63067 - Previously this used a NULL security descriptor
			// However a NULL security descriptor here means Full Control for Everyone!
			// This is different than a NULL security attributes, which will just use
			// the process default DACL.
			SECURITY_ATTRIBUTES saRead = { sizeof(SECURITY_ATTRIBUTES), (void*)pSecurityDesc, inheritRead };
			SECURITY_ATTRIBUTES saWrite = { sizeof(SECURITY_ATTRIBUTES), (void*)pSecurityDesc, inheritWrite };

			// console will be expecting a handle not operating in overlapped mode!
			// ensure handles that will be inherited will not be overlapped
			DWORD readOverlapped = inheritRead ? 0 : FILE_FLAG_OVERLAPPED;
			DWORD writeOverlapped = inheritWrite ? 0 : FILE_FLAG_OVERLAPPED;

			// anonymous pipes are not created with OVERLAPPED support :(
			//if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
			//	return false;
			//}

			CString pipeName = MakeObjectName(objectName);
			pipeName.Insert(0, "\\\\.\\pipe\\");

			// (a.walling 2014-09-02 12:34) - PLID 63067 - Ensure this is the first instance of the pipe
			pipe.r.Attach(
				::CreateNamedPipe(
				pipeName
				, PIPE_ACCESS_INBOUND | FILE_FLAG_FIRST_PIPE_INSTANCE | readOverlapped
				, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | (IsVistaOrGreater() ? PIPE_REJECT_REMOTE_CLIENTS : 0)
				, 1
				, 0
				, 0
				, 0
				, &saRead
				)
				);
			if (!pipe.r) {
				return ::GetLastError();
			}

			pipe.w.Attach(
				::CreateFile(
				pipeName
				, GENERIC_WRITE | SYNCHRONIZE
				, FILE_SHARE_READ | FILE_SHARE_WRITE
				, &saWrite
				, OPEN_EXISTING
				, writeOverlapped
				, NULL
				)
				);
			if (!pipe.w) {
				int err = ::GetLastError();
				pipe.r.Close();
				return err;
			}

			return 0;
		}

		struct Pipes
		{
			Pipe in;
			Pipe out;
			Pipe err;
		};

		int CreateRedirectedHandles(Pipes& pipes)
		{
			CAccessToken processToken;
			CDacl dacl;
			CSecurityDesc securityDesc;

			// (a.walling 2014-09-02 12:34) - PLID 63067 - Create pipes with process default DACL + deny network ACE
			processToken.GetProcessToken(GENERIC_ALL);
			processToken.GetDefaultDacl(&dacl);
			dacl.AddDeniedAce(Sids::Network(), GENERIC_ALL);
			securityDesc.SetDacl(dacl);

			int rc = 0;
			if (rc = CreateInheritablePipePair(pipes.in, "stdin", true, false, securityDesc)) {
				return rc;
			}
			if (rc = CreateInheritablePipePair(pipes.out, "stdout", false, true, securityDesc)) {
				return rc;
			}
			if (rc = CreateInheritablePipePair(pipes.err, "stderr", false, true, securityDesc)) {
				return rc;
			}

			return 0;
		}

		void DisableInheritanceForAll(Pipes& pipes)
		{
			SetHandleInformation(pipes.in.r, HANDLE_FLAG_INHERIT, 0);
			SetHandleInformation(pipes.in.w, HANDLE_FLAG_INHERIT, 0);
			SetHandleInformation(pipes.out.r, HANDLE_FLAG_INHERIT, 0);
			SetHandleInformation(pipes.out.w, HANDLE_FLAG_INHERIT, 0);
			SetHandleInformation(pipes.err.r, HANDLE_FLAG_INHERIT, 0);
			SetHandleInformation(pipes.err.w, HANDLE_FLAG_INHERIT, 0);
		}

		struct ProcessInfo
		{
			Handle process;
			Handle thread;
			DWORD processID;
			DWORD threadID;

			ProcessInfo()
				: processID(0)
				, threadID(0)
			{}

			void Attach(PROCESS_INFORMATION pi)
			{
				process.Attach(pi.hProcess);
				thread.Attach(pi.hThread);
				processID = pi.dwProcessId;
				threadID = pi.dwThreadId;
			}
		};

		int Spawn(ProcessInfo& processInfo, Pipes& pipes, CString cmdLine)
		{
			PROCESS_INFORMATION pi = { 0 };
			STARTUPINFO si = { sizeof(si) };

			si.hStdInput = pipes.in.r;
			si.hStdOutput = pipes.out.w;
			si.hStdError = pipes.err.w;
			si.dwFlags |= STARTF_USESTDHANDLES;

			CString::CStrBuf bufCmdLine(cmdLine);

			DWORD createFlags = 0;

			createFlags |= CREATE_NO_WINDOW; // don't show the console window

			if (!CreateProcess(NULL
				, bufCmdLine
				, NULL
				, NULL
				, TRUE // inherit handles!
				, createFlags
				, NULL
				, NULL
				, &si
				, &pi))
			{
				return ::GetLastError();
			}

			processInfo.Attach(pi);

			// disable inheritance of any existing pipes now
			DisableInheritanceForAll(pipes);

			return 0;
		}

		struct ProcessIO
		{
			// asio and stream_handle for asynchronous IO to the redirected pipes
			boost::asio::io_service service;
			//boost::asio::windows::stream_handle childIn;
			boost::asio::windows::stream_handle childOut;
			boost::asio::windows::stream_handle childErr;

			// all data is gathered into these streambufs
			//boost::asio::streambuf in;
			boost::asio::streambuf out;
			boost::asio::streambuf err;

			ProcessIO()
				: service()
				//, childIn(service)
				, childOut(service)
				, childErr(service)
			{}

			void ReadOutBegin()
			{
				boost::asio::streambuf::mutable_buffers_type buf = out.prepare(0x1000);
				childOut.async_read_some(buf, boost::bind(&ProcessIO::ReadOutComplete, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
			}

			void ReadOutComplete(const boost::system::error_code& ec, size_t len)
			{
				if (len) {
					// supposedly windows won't return an error if bytes were read, but just in case...

					//TRACE(__FUNCTION__" Read %lu bytes from child stdout...\n", len);
					out.commit(len);
				}

				if (ec) {
					// error
					if (ec != boost::system::errc::broken_pipe) { // ERROR_BROKEN_PIPE
						LogDetail(__FUNCTION__" - (code %li) %s", ec.value(), ec.message().c_str());
					}
					boost::system::error_code ecClosing;
					childOut.close(ecClosing);
					if (ecClosing) {
						LogDetail(__FUNCTION__" - Closing - (code %li) %s", ecClosing.value(), ecClosing.message().c_str());
					}
					return;
				}
				ReadOutBegin();
			}

			void ReadErrBegin()
			{
				boost::asio::streambuf::mutable_buffers_type buf = err.prepare(0x1000);
				childErr.async_read_some(buf, boost::bind(&ProcessIO::ReadErrComplete, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
			}

			void ReadErrComplete(const boost::system::error_code& ec, size_t len)
			{
				if (len) {
					// supposedly windows won't return an error if bytes were read, but just in case...

					//TRACE(__FUNCTION__" Read %lu bytes from child stderr...\n", len);
					err.commit(len);
				}

				if (ec) {
					// error
					if (ec != boost::system::errc::broken_pipe) { // ERROR_BROKEN_PIPE
						LogDetail(__FUNCTION__" - (code %li) %s", ec.value(), ec.message().c_str());
					}
					boost::system::error_code ecClosing;
					childErr.close(ecClosing);
					if (ecClosing) {
						LogDetail(__FUNCTION__" - Closing - (code %li) %s", ecClosing.value(), ecClosing.message().c_str());
					}
					return;
				}
				ReadErrBegin();
			}
		};
	}
}

// (a.walling 2014-07-28 10:36) - PLID 62825 - Launches NxCaptureSig in a separate process and gathers data from there.
namespace TopazSigPad
{
	struct AsyncCaptureSigProcess
	{
		int Start(const char* params)
		{
			int rc = 0;
			if (rc = CreateRedirectedHandles(pipes)) {
				return rc;
			}

			interruptEventName = Nx::Redirected::MakeObjectName("interruptEvent");
			interruptEvent.Attach(::CreateEvent(NULL, TRUE, FALSE, interruptEventName));


			long port = GetPort();
			CString exePath = "NxCaptureSig.exe";
			CString cmdLine;
			cmdLine.Format("\"%s\" --port %li --timeout 300 --idle 30 --interruptEvent %s %s", exePath, port, interruptEventName, params);

			LogDetail(__FUNCTION__" spawning %s", cmdLine);

			if (rc = Spawn(info, pipes, cmdLine)) {
				return rc;
			}

			pipes.in.r.Close();
			pipes.out.w.Close();
			pipes.err.w.Close();

			pipes.in.w.Close(); // not sending anything

			io.childOut.assign(pipes.out.r.Detach());
			io.childErr.assign(pipes.err.r.Detach());

			io.ReadOutBegin();
			io.ReadErrBegin();

			return 0;
		}

		void Poll()
		{
			for (;;) {
				size_t ran = io.service.poll();
				if (!ran) {
					return;
				}
				//TRACE(__FUNCTION__" Poll ran %lu handlers.\n", ran);
			}
		}

		int Stop()
		{
			if (interruptEvent) {
				::SetEvent(interruptEvent);
			}

			info.thread.Close();

			// stop after 30 seconds just in case
			DWORD ticksBegin = ::GetTickCount();
			do {
				// process all queued handlers
				do {} while (io.service.run_one());

				DWORD ret = ::WaitForSingleObject(info.process, 60);
				if (ret == WAIT_OBJECT_0) {
					// ensure all async io operations are completed
					io.service.run();
					break;
				}
			} while ((::GetTickCount() - ticksBegin) < 30000);

			if (io.childOut.is_open()) {
				boost::system::error_code ecClosing;
				io.childOut.close(ecClosing);
			}
			if (io.childErr.is_open()) {
				boost::system::error_code ecClosing;
				io.childErr.close(ecClosing);
			}

			// ensure all async io operations are completed
			io.service.run();
			io.service.stop();

			interruptEvent.Close();

			DWORD exitCode = -1;
			::GetExitCodeProcess(info.process, &exitCode);
			TRACE(__FUNCTION__" Exit code %u\n", exitCode);

			info.process.Close();

			return (int)exitCode;
		}

		Nx::Redirected::ProcessInfo info;
		Nx::Redirected::Pipes pipes;
		Nx::Redirected::ProcessIO io;
		Nx::Handle interruptEvent;
		CString interruptEventName;
	};

	long GetPort()
	{
		return GetPropertyInt("TopazSigPadPortNumber", -1, 0);
	}

	bool ShouldMock(CWnd* pParentWnd)
	{
		if ((GetAsyncKeyState(VK_SHIFT) < 0) && (GetAsyncKeyState(VK_CONTROL) < 0)) {
			if (IDOK == pParentWnd->MessageBox("Communication with NxCaptureSig will be mocked.", "Debug", MB_OKCANCEL)) {
				return true;
			}
		}
		return false;
	}

	//(d.singleton 2013-01-05 15:58) - PLID 56520 - check the passed in com port, or read from registry to check if we are connected.
	BOOL IsTabletConnected(CWnd* pParentWnd, long nComPort)
	{
		int rc = 0;
		AsyncCaptureSigProcess cap;

		CString cmdArgs = "--test";

		if (ShouldMock(pParentWnd)) {
			cmdArgs += " --mock";
		}

		DWORD ticks = GetTickCount();
		if (rc = cap.Start(cmdArgs)) {
			RaiseComError(HRESULT_FROM_WIN32(rc), "Failed to start signature connection test");
			return false;
		}

		do {
			::Sleep(200);
			cap.Poll();
			if (!cap.info.process || (WAIT_OBJECT_0 == ::WaitForSingleObject(cap.info.process, 0))) {
				break;
			}
		} while ((GetTickCount() - ticks) < 2000);

		int exitCode = cap.Stop();

		if (!exitCode) {
			LogDetail(__FUNCTION__" ExitCode: %li", exitCode);
			std::istream sErr(&cap.io.err);
			std::string line;
			while (getline(sErr, line)) {
				LogDetail("%s", line.c_str());
			}
		}

		return 0 == exitCode;
	}

	struct SignatureData
	{
		std::vector<std::vector<POINT>> strokes;
		CRect box;

		SignatureData()
		{
			box.SetRectEmpty();
		}
	};

	// (b.spivey, April 26, 2013) - PLID 30035 - Get ink from the topaz signature pad. 
	MSINKAUTLib::IInkDispPtr GetSignatureInk(CWnd* pParentWnd)
	{
		class SignatureTaskDialog
			: public NxTaskDialog
		{
		public:
			SignatureTaskDialog(CWnd* pParentDlg, AsyncCaptureSigProcess& cap)
				: NxTaskDialog(pParentDlg)
				, cap(cap)
			{}

			virtual BOOL OnTimer(DWORD dwTickCount) override
			{
				try {
					cap.Poll();

					if (!cap.info.process || (WAIT_OBJECT_0 == ::WaitForSingleObject(cap.info.process, 0))) {
						// process has completed!
						ClickButton(IDOK);
					}
				} NxCatchAllIgnore();

				return FALSE;
			}

			virtual void OnDialogConstructed() override
			{
				__super::OnDialogConstructed();
			}

			AsyncCaptureSigProcess& cap;
		};

		int rc = 0;
		AsyncCaptureSigProcess cap;

		SignatureTaskDialog dlg(pParentWnd, cap);
		dlg.Config()
			.OKCancel()
			.InformationIcon()
			.MainInstructionText("Waiting for user signature...")
			.ContentText("Please click OK when the signature is complete.")
			.CallbackTimer()
		; 

		CString cmdArgs = "--raw";

		if (ShouldMock(pParentWnd)) {
			cmdArgs += " --mock";
		}

		if (rc = cap.Start(cmdArgs)) {
			RaiseComError(HRESULT_FROM_WIN32(rc), "Failed to start signature capture");
			return NULL;
		}
		
		INT_PTR ret = dlg.DoModal();

		int exitCode = cap.Stop();

		if (exitCode) {
			LogDetail(__FUNCTION__" ExitCode: %li", exitCode);
			std::istream sErr(&cap.io.err);
			std::string line;
			while (getline(sErr, line)) {
				LogDetail("%s", line.c_str());
			}
		}

		// parse any data returned into here
		SignatureData data;
		{
			std::istream sOut(&cap.io.out);

			std::string line;
			getline(sOut, line); // viewBox

			{
				std::istringstream is(line);
				is >> data.box.left >> data.box.top >> data.box.right >> data.box.bottom;
			}

			data.strokes.reserve(32);

			while (getline(sOut, line)) {
				if (line.empty()) {
					continue;
				}
				if (data.strokes.empty()) {
					data.strokes.resize(1);
				}
				if (!data.strokes.back().empty()) {
					data.strokes.resize(data.strokes.size() + 1);
				}
				POINT pt;
				std::istringstream is(line);
				while (is >> pt.x >> pt.y) {
					data.strokes.back().push_back(pt);
				}
			}
			while (!data.strokes.empty() && data.strokes.back().empty()) {
				data.strokes.resize(data.strokes.size() - 1);
			}
		}
		
		if (ret == IDCANCEL) {
			return NULL;
		}

		if (!data.strokes.empty()) {
			MSINKAUTLib::IInkDispPtr pInk;
			using namespace MSINKAUTLib;
			pInk.CreateInstance(__uuidof(InkDisp));

			{
				// minimum bound

				_variant_t varPoints;
				varPoints.vt = VT_ARRAY | VT_I4; //This is a variant array of longs
				Nx::SafeArray<long> sa(2, &data.box.left);
				varPoints.parray = sa.Detach();

				pInk->CreateStroke(varPoints, g_cvarEmpty);
			}
			for each (const std::vector<POINT>& stroke in data.strokes) {
				_variant_t varPoints;
				varPoints.vt = VT_ARRAY | VT_I4; //This is a variant array of longs
				Nx::SafeArray<long> sa;
				sa.Add(stroke.size() * 2, (long*)&stroke[0], true);
				varPoints.parray = sa.Detach();

				pInk->CreateStroke(varPoints, g_cvarEmpty); //Create a new stroke from the ink IDisp object
			}
			{
				// maximum bound

				_variant_t varPoints;
				varPoints.vt = VT_ARRAY | VT_I4; //This is a variant array of longs
				Nx::SafeArray<long> sa(2, &data.box.right);
				varPoints.parray = sa.Detach();

				pInk->CreateStroke(varPoints, g_cvarEmpty);
			}

			return pInk;
		}
		else if (exitCode) {
			RaiseComError( (rc == -1) ? E_FAIL : HRESULT_FROM_WIN32(rc), "Failed to capture signature");
		}
		else {
			LogDetail("No signature captured!");
		}
		
		return NULL;
	}
}

