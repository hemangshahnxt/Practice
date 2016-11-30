// ANSI823Parser.h interface for ANSI823Parser class

// (d.singleton 2014-07-15 15:08) - PLID 62896 - parse the data from an ANSI 823 file and show the data in a datalist

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

struct LockboxCheckInfo
{
	enum paymentType {
		Debit = 0,
		Credit,
	};

	COleCurrency cAmount = COleCurrency(0, 0);
	CString strCheckNumber;
	CString strAccountNumber;
	CString strRoutingNumber;
	paymentType ptType = Credit;
};

struct LockboxPaymentInfo
{
	CString strPatientID;
	COleCurrency cAmount = COleCurrency(0, 0);
	CString strCheckNumber;
	CString strAccountNumber;
	CString strRoutingNumber;
};

typedef std::unique_ptr<LockboxPaymentInfo> LockboxPaymentInfoPtr;
typedef std::unique_ptr<LockboxCheckInfo> LockboxCheckInfoPtr;

struct LockboxDepositInfo
{
	COleCurrency cAmount = COleCurrency(0, 0);
	COleDateTime dtDepositDate = g_cdtNull;
	CString strBankName;
	std::vector<LockboxPaymentInfoPtr> aryLockboxPayments;
	std::vector<LockboxCheckInfoPtr> aryLockboxChecks;
	CString strType;
};

class CANSI823Parser
{
public:
	CANSI823Parser(CWnd* pWnd, CProgressCtrl* pProgressBar, CEdit* pEdit);
	virtual ~CANSI823Parser();

	LockboxDepositInfo m_ldiDeposit;

	BOOL ParseFile();
	CString m_strAnsi823FileName;
	

private:
	CWnd* m_pParentWnd;	
	CProgressCtrl* m_pcProgressBar;
	CEdit* m_eProgressText;
	CString m_strFilePath;
	char m_chSegmentTerminator;
	char m_chElementTerminator;

	LockboxPaymentInfoPtr& GetCurrentPaymentInfo();
	LockboxCheckInfoPtr& GetCurrentCheckInfo();

	CString ParseSegment(CString& strIn);
	CString ParseElement(CString& strIn);
	CString ParseSection(CString& strIn, char chDelimiter);	

	//parsing functions for needed segments
	void ANSI_DEP(CString& strIn);
	void ANSI_AMT(CString& strIn);
	void ANSI_QTY(CString& strIn);
	void ANSI_BPR(CString& strIn);
	void ANSI_DTM(CString& strIn);
	void ANSI_REF(CString& strIn);
	void ANSI_N1(CString& strIn);
	void ANSI_RMR(CString& strIn);
};