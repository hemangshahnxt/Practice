#ifndef _BARCODE_API_H_
#define _BARCODE_API_H_

#pragma once



// Barcode defaults:
//
// 9600 baud
// Space parity
// 2 stop bits
// 7 data bits
//

#define WM_BARCODE_OPEN_FAILED		WM_USER + 20101
// wParam = 0
// lParam = 0 if CreateFile failed
//			1 if SetCommState failed

#define WM_BARCODE_SCAN			WM_USER + 20102
// wParam = size of string
// lParam = pointer to BSTR of characters

#define WM_BARCODE_OVERFLOW		WM_USER + 20103
// wParam = size of string
// lParam = pointer to BSTR of characters

// (j.jones 2008-12-23 13:24) - PLID 32545 - the Barcode_Thread now queues barcodes
// for MainFrm to handle, using this message
#define WM_ENQUEUE_BARCODE		WM_USER + 20104
// wParam = size of string
// lParam = pointer to BSTR of characters

BOOL Barcode_Open(HWND hWnd, const char* szComPort = "COM1");
void Barcode_Close();

// (a.walling 2007-05-10 09:32) - PLID 25171 - I'm on the fence about whether this belongs
// here or not, but it seems global utils is getting crowded, and anywhere that uses this
// function will have already included this file.
BOOL Barcode_CheckCollisions(CWnd* pNotifyWnd, CString &strBarcode, BOOL bForceUnique = FALSE); // check if the barcode string collides with any other barcodes

#endif