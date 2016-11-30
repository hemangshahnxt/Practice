#pragma once
#pragma pack(push, 8)

#include <comdef.h>

// (a.walling 2012-09-05 11:35) - PLID 52053 - For lack of a better PLID - getting rid of cdosys.dll import, just use what we know is good
// unfortunately cdosys.dll will automatically find the (bad) ado typelibs due to auto_search, so this just saves everyone a big headache

namespace CDOSYS {

//
// Forward references and typedefs
//

struct __declspec(uuid("cd000000-8b95-11d1-82db-00c04fb1625d"))
/* LIBID */ __CDO;
enum CdoConfigSource;
enum CdoDSNOptions;
enum CdoEventStatus;
enum cdoImportanceValues;
enum CdoMessageStat;
enum CdoMHTMLFlags;
enum CdoNNTPProcessingField;
enum CdoPostUsing;
enum cdoPriorityValues;
enum CdoProtocolsAuthentication;
enum CdoReferenceType;
enum CdoSendUsing;
enum cdoSensitivityValues;
enum CdoTimeZoneId;
struct __declspec(uuid("cd000023-8b95-11d1-82db-00c04fb1625d"))
/* dual interface */ IBodyParts;
struct __declspec(uuid("cd000021-8b95-11d1-82db-00c04fb1625d"))
/* dual interface */ IBodyPart;
struct __declspec(uuid("cd000029-8b95-11d1-82db-00c04fb1625d"))
/* dual interface */ IDataSource;
struct __declspec(uuid("cd000025-8b95-11d1-82db-00c04fb1625d"))
/* dual interface */ IMessages;
struct __declspec(uuid("cd000020-8b95-11d1-82db-00c04fb1625d"))
/* dual interface */ IMessage;
struct __declspec(uuid("cd000022-8b95-11d1-82db-00c04fb1625d"))
/* dual interface */ IConfiguration;
struct /* coclass */ Message;
struct /* coclass */ Configuration;
struct /* coclass */ DropDirectory;
struct __declspec(uuid("cd000024-8b95-11d1-82db-00c04fb1625d"))
/* dual interface */ IDropDirectory;
struct /* coclass */ SMTPConnector;
struct __declspec(uuid("cd000030-8b95-11d1-82db-00c04fb1625d"))
/* dual interface */ ISMTPScriptConnector;
struct __declspec(uuid("cd000026-8b95-11d1-82db-00c04fb1625d"))
/* dual interface */ ISMTPOnArrival;
struct /* coclass */ NNTPEarlyConnector;
struct __declspec(uuid("cd000034-8b95-11d1-82db-00c04fb1625d"))
/* dual interface */ INNTPEarlyScriptConnector;
struct __declspec(uuid("cd000033-8b95-11d1-82db-00c04fb1625d"))
/* dual interface */ INNTPOnPostEarly;
struct /* coclass */ NNTPPostConnector;
struct __declspec(uuid("cd000031-8b95-11d1-82db-00c04fb1625d"))
/* dual interface */ INNTPPostScriptConnector;
struct __declspec(uuid("cd000027-8b95-11d1-82db-00c04fb1625d"))
/* dual interface */ INNTPOnPost;
struct /* coclass */ NNTPFinalConnector;
struct __declspec(uuid("cd000032-8b95-11d1-82db-00c04fb1625d"))
/* dual interface */ INNTPFinalScriptConnector;
struct __declspec(uuid("cd000028-8b95-11d1-82db-00c04fb1625d"))
/* dual interface */ INNTPOnPostFinal;

//
// Smart pointer typedef declarations
//

_COM_SMARTPTR_TYPEDEF(IDataSource, __uuidof(IDataSource));
_COM_SMARTPTR_TYPEDEF(IConfiguration, __uuidof(IConfiguration));
_COM_SMARTPTR_TYPEDEF(ISMTPScriptConnector, __uuidof(ISMTPScriptConnector));
_COM_SMARTPTR_TYPEDEF(INNTPEarlyScriptConnector, __uuidof(INNTPEarlyScriptConnector));
_COM_SMARTPTR_TYPEDEF(INNTPPostScriptConnector, __uuidof(INNTPPostScriptConnector));
_COM_SMARTPTR_TYPEDEF(INNTPFinalScriptConnector, __uuidof(INNTPFinalScriptConnector));
_COM_SMARTPTR_TYPEDEF(IBodyParts, __uuidof(IBodyParts));
_COM_SMARTPTR_TYPEDEF(IBodyPart, __uuidof(IBodyPart));
_COM_SMARTPTR_TYPEDEF(IMessage, __uuidof(IMessage));
_COM_SMARTPTR_TYPEDEF(IMessages, __uuidof(IMessages));
_COM_SMARTPTR_TYPEDEF(IDropDirectory, __uuidof(IDropDirectory));
_COM_SMARTPTR_TYPEDEF(ISMTPOnArrival, __uuidof(ISMTPOnArrival));
_COM_SMARTPTR_TYPEDEF(INNTPOnPostEarly, __uuidof(INNTPOnPostEarly));
_COM_SMARTPTR_TYPEDEF(INNTPOnPost, __uuidof(INNTPOnPost));
_COM_SMARTPTR_TYPEDEF(INNTPOnPostFinal, __uuidof(INNTPOnPostFinal));

//
// Type library items
//

enum CdoConfigSource
{
    cdoDefaults = -1,
    cdoIIS = 1,
    cdoOutlookExpress = 2
};

enum CdoDSNOptions
{
    cdoDSNDefault = 0,
    cdoDSNNever = 1,
    cdoDSNFailure = 2,
    cdoDSNSuccess = 4,
    cdoDSNDelay = 8,
    cdoDSNSuccessFailOrDelay = 14
};

enum CdoEventStatus
{
    cdoRunNextSink = 0,
    cdoSkipRemainingSinks = 1
};

enum cdoImportanceValues
{
    cdoLow = 0,
    cdoNormal = 1,
    cdoHigh = 2
};

enum CdoMessageStat
{
    cdoStatSuccess = 0,
    cdoStatAbortDelivery = 2,
    cdoStatBadMail = 3
};

enum CdoMHTMLFlags
{
    cdoSuppressNone = 0,
    cdoSuppressImages = 1,
    cdoSuppressBGSounds = 2,
    cdoSuppressFrames = 4,
    cdoSuppressObjects = 8,
    cdoSuppressStyleSheets = 16,
    cdoSuppressAll = 31
};

enum CdoNNTPProcessingField
{
    cdoPostMessage = 1,
    cdoProcessControl = 2,
    cdoProcessModerator = 4
};

enum CdoPostUsing
{
    cdoPostUsingPickup = 1,
    cdoPostUsingPort = 2
};

enum cdoPriorityValues
{
    cdoPriorityNonUrgent = -1,
    cdoPriorityNormal = 0,
    cdoPriorityUrgent = 1
};

enum CdoProtocolsAuthentication
{
    cdoAnonymous = 0,
    cdoBasic = 1,
    cdoNTLM = 2
};

enum CdoReferenceType
{
    cdoRefTypeId = 0,
    cdoRefTypeLocation = 1
};

enum CdoSendUsing
{
    cdoSendUsingPickup = 1,
    cdoSendUsingPort = 2
};

enum cdoSensitivityValues
{
    cdoSensitivityNone = 0,
    cdoPersonal = 1,
    cdoPrivate = 2,
    cdoCompanyConfidential = 3
};

enum CdoTimeZoneId
{
    cdoUTC = 0,
    cdoGMT = 1,
    cdoSarajevo = 2,
    cdoParis = 3,
    cdoBerlin = 4,
    cdoEasternEurope = 5,
    cdoPrague = 6,
    cdoAthens = 7,
    cdoBrasilia = 8,
    cdoAtlanticCanada = 9,
    cdoEastern = 10,
    cdoCentral = 11,
    cdoMountain = 12,
    cdoPacific = 13,
    cdoAlaska = 14,
    cdoHawaii = 15,
    cdoMidwayIsland = 16,
    cdoWellington = 17,
    cdoBrisbane = 18,
    cdoAdelaide = 19,
    cdoTokyo = 20,
    cdoSingapore = 21,
    cdoBangkok = 22,
    cdoBombay = 23,
    cdoAbuDhabi = 24,
    cdoTehran = 25,
    cdoBaghdad = 26,
    cdoIsrael = 27,
    cdoNewfoundland = 28,
    cdoAzores = 29,
    cdoMidAtlantic = 30,
    cdoMonrovia = 31,
    cdoBuenosAires = 32,
    cdoCaracas = 33,
    cdoIndiana = 34,
    cdoBogota = 35,
    cdoSaskatchewan = 36,
    cdoMexicoCity = 37,
    cdoArizona = 38,
    cdoEniwetok = 39,
    cdoFiji = 40,
    cdoMagadan = 41,
    cdoHobart = 42,
    cdoGuam = 43,
    cdoDarwin = 44,
    cdoBeijing = 45,
    cdoAlmaty = 46,
    cdoIslamabad = 47,
    cdoKabul = 48,
    cdoCairo = 49,
    cdoHarare = 50,
    cdoMoscow = 51,
    cdoFloating = 52,
    cdoCapeVerde = 53,
    cdoCaucasus = 54,
    cdoCentralAmerica = 55,
    cdoEastAfrica = 56,
    cdoMelbourne = 57,
    cdoEkaterinburg = 58,
    cdoHelsinki = 59,
    cdoGreenland = 60,
    cdoRangoon = 61,
    cdoNepal = 62,
    cdoIrkutsk = 63,
    cdoKrasnoyarsk = 64,
    cdoSantiago = 65,
    cdoSriLanka = 66,
    cdoTonga = 67,
    cdoVladivostok = 68,
    cdoWestCentralAfrica = 69,
    cdoYakutsk = 70,
    cdoDhaka = 71,
    cdoSeoul = 72,
    cdoPerth = 73,
    cdoArab = 74,
    cdoTaipei = 75,
    cdoSydney2000 = 76,
    cdoChihuahua = 77,
    cdoCanberraCommonwealthGames2006 = 78,
    cdoAdelaideCommonwealthGames2006 = 79,
    cdoHobartCommonwealthGames2006 = 80,
    cdoTijuana = 81,
    cdoInvalidTimeZone = 82
};
    const BSTR cdoTimeZoneIDURN = (wchar_t*) L"urn:schemas:calendar:timezoneid";
    const BSTR cdoBIG5 = (wchar_t*) L"big5";
    const BSTR cdoEUC_JP = (wchar_t*) L"euc-jp";
    const BSTR cdoEUC_KR = (wchar_t*) L"euc-kr";
    const BSTR cdoGB2312 = (wchar_t*) L"gb2312";
    const BSTR cdoISO_2022_JP = (wchar_t*) L"iso-2022-jp";
    const BSTR cdoISO_2022_KR = (wchar_t*) L"iso-2022-kr";
    const BSTR cdoISO_8859_1 = (wchar_t*) L"iso-8859-1";
    const BSTR cdoISO_8859_2 = (wchar_t*) L"iso-8859-2";
    const BSTR cdoISO_8859_3 = (wchar_t*) L"iso-8859-3";
    const BSTR cdoISO_8859_4 = (wchar_t*) L"iso-8859-4";
    const BSTR cdoISO_8859_5 = (wchar_t*) L"iso-8859-5";
    const BSTR cdoISO_8859_6 = (wchar_t*) L"iso-8859-6";
    const BSTR cdoISO_8859_7 = (wchar_t*) L"iso-8859-7";
    const BSTR cdoISO_8859_8 = (wchar_t*) L"iso-8859-8";
    const BSTR cdoISO_8859_9 = (wchar_t*) L"iso-8859-9";
    const BSTR cdoKOI8_R = (wchar_t*) L"koi8-r";
    const BSTR cdoShift_JIS = (wchar_t*) L"shift-jis";
    const BSTR cdoUS_ASCII = (wchar_t*) L"us-ascii";
    const BSTR cdoUTF_7 = (wchar_t*) L"utf-7";
    const BSTR cdoUTF_8 = (wchar_t*) L"utf-8";
    const BSTR cdoISO_8859_15 = (wchar_t*) L"iso-8859-15";
    const BSTR cdoAutoPromoteBodyParts = (wchar_t*) L"http://schemas.microsoft.com/cdo/configuration/autopromotebodyparts";
    const BSTR cdoFlushBuffersOnWrite = (wchar_t*) L"http://schemas.microsoft.com/cdo/configuration/flushbuffersonwrite";
    const BSTR cdoHTTPCookies = (wchar_t*) L"http://schemas.microsoft.com/cdo/configuration/httpcookies";
    const BSTR cdoLanguageCode = (wchar_t*) L"http://schemas.microsoft.com/cdo/configuration/languagecode";
    const BSTR cdoNNTPAccountName = (wchar_t*) L"http://schemas.microsoft.com/cdo/configuration/nntpaccountname";
    const BSTR cdoNNTPAuthenticate = (wchar_t*) L"http://schemas.microsoft.com/cdo/configuration/nntpauthenticate";
    const BSTR cdoNNTPConnectionTimeout = (wchar_t*) L"http://schemas.microsoft.com/cdo/configuration/nntpconnectiontimeout";
    const BSTR cdoNNTPServer = (wchar_t*) L"http://schemas.microsoft.com/cdo/configuration/nntpserver";
    const BSTR cdoNNTPServerPickupDirectory = (wchar_t*) L"http://schemas.microsoft.com/cdo/configuration/nntpserverpickupdirectory";
    const BSTR cdoNNTPServerPort = (wchar_t*) L"http://schemas.microsoft.com/cdo/configuration/nntpserverport";
    const BSTR cdoNNTPUseSSL = (wchar_t*) L"http://schemas.microsoft.com/cdo/configuration/nntpusessl";
    const BSTR cdoPostEmailAddress = (wchar_t*) L"http://schemas.microsoft.com/cdo/configuration/postemailaddress";
    const BSTR cdoPostPassword = (wchar_t*) L"http://schemas.microsoft.com/cdo/configuration/postpassword";
    const BSTR cdoPostUserName = (wchar_t*) L"http://schemas.microsoft.com/cdo/configuration/postusername";
    const BSTR cdoPostUserReplyEmailAddress = (wchar_t*) L"http://schemas.microsoft.com/cdo/configuration/postuserreplyemailaddress";
    const BSTR cdoPostUsingMethod = (wchar_t*) L"http://schemas.microsoft.com/cdo/configuration/postusing";
    const BSTR cdoSaveSentItems = (wchar_t*) L"http://schemas.microsoft.com/cdo/configuration/savesentitems";
    const BSTR cdoSendEmailAddress = (wchar_t*) L"http://schemas.microsoft.com/cdo/configuration/sendemailaddress";
    const BSTR cdoSendPassword = (wchar_t*) L"http://schemas.microsoft.com/cdo/configuration/sendpassword";
    const BSTR cdoSendUserName = (wchar_t*) L"http://schemas.microsoft.com/cdo/configuration/sendusername";
    const BSTR cdoSendUserReplyEmailAddress = (wchar_t*) L"http://schemas.microsoft.com/cdo/configuration/senduserreplyemailaddress";
    const BSTR cdoSendUsingMethod = (wchar_t*) L"http://schemas.microsoft.com/cdo/configuration/sendusing";
    const BSTR cdoSMTPAccountName = (wchar_t*) L"http://schemas.microsoft.com/cdo/configuration/smtpaccountname";
    const BSTR cdoSMTPAuthenticate = (wchar_t*) L"http://schemas.microsoft.com/cdo/configuration/smtpauthenticate";
    const BSTR cdoSMTPConnectionTimeout = (wchar_t*) L"http://schemas.microsoft.com/cdo/configuration/smtpconnectiontimeout";
    const BSTR cdoSMTPServer = (wchar_t*) L"http://schemas.microsoft.com/cdo/configuration/smtpserver";
    const BSTR cdoSMTPServerPickupDirectory = (wchar_t*) L"http://schemas.microsoft.com/cdo/configuration/smtpserverpickupdirectory";
    const BSTR cdoSMTPServerPort = (wchar_t*) L"http://schemas.microsoft.com/cdo/configuration/smtpserverport";
    const BSTR cdoSMTPUseSSL = (wchar_t*) L"http://schemas.microsoft.com/cdo/configuration/smtpusessl";
    const BSTR cdoURLGetLatestVersion = (wchar_t*) L"http://schemas.microsoft.com/cdo/configuration/urlgetlatestversion";
    const BSTR cdoURLProxyBypass = (wchar_t*) L"http://schemas.microsoft.com/cdo/configuration/urlproxybypass";
    const BSTR cdoURLProxyServer = (wchar_t*) L"http://schemas.microsoft.com/cdo/configuration/urlproxyserver";
    const BSTR cdoUseMessageResponseText = (wchar_t*) L"http://schemas.microsoft.com/cdo/configuration/usemessageresponsetext";
    const BSTR cdoGif = (wchar_t*) L"image/gif";
    const BSTR cdoJpeg = (wchar_t*) L"image/jpeg";
    const BSTR cdoMessageExternalBody = (wchar_t*) L"message/external-body";
    const BSTR cdoMessagePartial = (wchar_t*) L"message/partial";
    const BSTR cdoMessageRFC822 = (wchar_t*) L"message/rfc822";
    const BSTR cdoMultipartAlternative = (wchar_t*) L"multipart/alternative";
    const BSTR cdoMultipartDigest = (wchar_t*) L"multipart/digest";
    const BSTR cdoMultipartMixed = (wchar_t*) L"multipart/mixed";
    const BSTR cdoMultipartRelated = (wchar_t*) L"multipart/related";
    const BSTR cdoTextHTML = (wchar_t*) L"text/html";
    const BSTR cdoTextPlain = (wchar_t*) L"text/plain";
    const BSTR cdoContentClass = (wchar_t*) L"DAV:contentclass";
    const BSTR cdoGetContentType = (wchar_t*) L"DAV:getcontenttype";
    const BSTR cdo7bit = (wchar_t*) L"7bit";
    const BSTR cdo8bit = (wchar_t*) L"8bit";
    const BSTR cdoBase64 = (wchar_t*) L"base64";
    const BSTR cdoBinary = (wchar_t*) L"binary";
    const BSTR cdoMacBinHex40 = (wchar_t*) L"mac-binhex40";
    const BSTR cdoQuotedPrintable = (wchar_t*) L"quoted-printable";
    const BSTR cdoUuencode = (wchar_t*) L"uuencode";
    const BSTR cdoSensitivity = (wchar_t*) L"http://schemas.microsoft.com/exchange/sensitivity";
    const BSTR cdoAttachmentFilename = (wchar_t*) L"urn:schemas:httpmail:attachmentfilename";
    const BSTR cdoBcc = (wchar_t*) L"urn:schemas:httpmail:bcc";
    const BSTR cdoCc = (wchar_t*) L"urn:schemas:httpmail:cc";
    const BSTR cdoContentDispositionType = (wchar_t*) L"urn:schemas:httpmail:content-disposition-type";
    const BSTR cdoContentMediaType = (wchar_t*) L"urn:schemas:httpmail:content-media-type";
    const BSTR cdoDate = (wchar_t*) L"urn:schemas:httpmail:date";
    const BSTR cdoDateReceived = (wchar_t*) L"urn:schemas:httpmail:datereceived";
    const BSTR cdoFrom = (wchar_t*) L"urn:schemas:httpmail:from";
    const BSTR cdoHasAttachment = (wchar_t*) L"urn:schemas:httpmail:hasattachment";
    const BSTR cdoHTMLDescription = (wchar_t*) L"urn:schemas:httpmail:htmldescription";
    const BSTR cdoImportance = (wchar_t*) L"urn:schemas:httpmail:importance";
    const BSTR cdoNormalizedSubject = (wchar_t*) L"urn:schemas:httpmail:normalizedsubject";
    const BSTR cdoPriority = (wchar_t*) L"urn:schemas:httpmail:priority";
    const BSTR cdoReplyTo = (wchar_t*) L"urn:schemas:httpmail:reply-to";
    const BSTR cdoSender = (wchar_t*) L"urn:schemas:httpmail:sender";
    const BSTR cdoSubject = (wchar_t*) L"urn:schemas:httpmail:subject";
    const BSTR cdoTextDescription = (wchar_t*) L"urn:schemas:httpmail:textdescription";
    const BSTR cdoThreadTopic = (wchar_t*) L"urn:schemas:httpmail:thread-topic";
    const BSTR cdoTo = (wchar_t*) L"urn:schemas:httpmail:to";
    const BSTR cdoAdoStream = (wchar_t*) L"_Stream";
    const BSTR cdoIBodyPart = (wchar_t*) L"IBodyPart";
    const BSTR cdoIConfiguration = (wchar_t*) L"IConfiguration";
    const BSTR cdoIDataSource = (wchar_t*) L"IDataSource";
    const BSTR cdoIMessage = (wchar_t*) L"IMessage";
    const BSTR cdoIStream = (wchar_t*) L"IStream";
    const BSTR cdoApproved = (wchar_t*) L"urn:schemas:mailheader:approved";
    const BSTR cdoComment = (wchar_t*) L"urn:schemas:mailheader:comment";
    const BSTR cdoContentBase = (wchar_t*) L"urn:schemas:mailheader:content-base";
    const BSTR cdoContentDescription = (wchar_t*) L"urn:schemas:mailheader:content-description";
    const BSTR cdoContentDisposition = (wchar_t*) L"urn:schemas:mailheader:content-disposition";
    const BSTR cdoContentId = (wchar_t*) L"urn:schemas:mailheader:content-id";
    const BSTR cdoContentLanguage = (wchar_t*) L"urn:schemas:mailheader:content-language";
    const BSTR cdoContentLocation = (wchar_t*) L"urn:schemas:mailheader:content-location";
    const BSTR cdoContentTransferEncoding = (wchar_t*) L"urn:schemas:mailheader:content-transfer-encoding";
    const BSTR cdoContentType = (wchar_t*) L"urn:schemas:mailheader:content-type";
    const BSTR cdoControl = (wchar_t*) L"urn:schemas:mailheader:control";
    const BSTR cdoDisposition = (wchar_t*) L"urn:schemas:mailheader:disposition";
    const BSTR cdoDispositionNotificationTo = (wchar_t*) L"urn:schemas:mailheader:disposition-notification-to";
    const BSTR cdoDistribution = (wchar_t*) L"urn:schemas:mailheader:distribution";
    const BSTR cdoExpires = (wchar_t*) L"urn:schemas:mailheader:expires";
    const BSTR cdoFollowupTo = (wchar_t*) L"urn:schemas:mailheader:followup-to";
    const BSTR cdoInReplyTo = (wchar_t*) L"urn:schemas:mailheader:in-reply-to";
    const BSTR cdoLines = (wchar_t*) L"urn:schemas:mailheader:lines";
    const BSTR cdoMessageId = (wchar_t*) L"urn:schemas:mailheader:message-id";
    const BSTR cdoMIMEVersion = (wchar_t*) L"urn:schemas:mailheader:mime-version";
    const BSTR cdoNewsgroups = (wchar_t*) L"urn:schemas:mailheader:newsgroups";
    const BSTR cdoOrganization = (wchar_t*) L"urn:schemas:mailheader:organization";
    const BSTR cdoOriginalRecipient = (wchar_t*) L"urn:schemas:mailheader:original-recipient";
    const BSTR cdoPath = (wchar_t*) L"urn:schemas:mailheader:path";
    const BSTR cdoPostingVersion = (wchar_t*) L"urn:schemas:mailheader:posting-version";
    const BSTR cdoReceived = (wchar_t*) L"urn:schemas:mailheader:received";
    const BSTR cdoReferences = (wchar_t*) L"urn:schemas:mailheader:references";
    const BSTR cdoRelayVersion = (wchar_t*) L"urn:schemas:mailheader:relay-version";
    const BSTR cdoReturnPath = (wchar_t*) L"urn:schemas:mailheader:return-path";
    const BSTR cdoReturnReceiptTo = (wchar_t*) L"urn:schemas:mailheader:return-receipt-to";
    const BSTR cdoSummary = (wchar_t*) L"urn:schemas:mailheader:summary";
    const BSTR cdoThreadIndex = (wchar_t*) L"urn:schemas:mailheader:thread-index";
    const BSTR cdoXMailer = (wchar_t*) L"urn:schemas:mailheader:x-mailer";
    const BSTR cdoXref = (wchar_t*) L"urn:schemas:mailheader:xref";
    const BSTR cdoXUnsent = (wchar_t*) L"urn:schemas:mailheader:x-unsent";
    const BSTR cdoXFidelity = (wchar_t*) L"urn:schemas:mailheader:x-cdostreamhighfidelity";
    const BSTR cdoNSConfiguration = (wchar_t*) L"http://schemas.microsoft.com/cdo/configuration/";
    const BSTR cdoNSContacts = (wchar_t*) L"urn:schemas:contacts:";
    const BSTR cdoNSHTTPMail = (wchar_t*) L"urn:schemas:httpmail:";
    const BSTR cdoNSMailHeader = (wchar_t*) L"urn:schemas:mailheader:";
    const BSTR cdoNSNNTPEnvelope = (wchar_t*) L"http://schemas.microsoft.com/cdo/nntpenvelope/";
    const BSTR cdoNSSMTPEnvelope = (wchar_t*) L"http://schemas.microsoft.com/cdo/smtpenvelope/";
    const BSTR cdoNewsgroupList = (wchar_t*) L"http://schemas.microsoft.com/cdo/nntpenvelope/newsgrouplist";
    const BSTR cdoNNTPProcessing = (wchar_t*) L"http://schemas.microsoft.com/cdo/nntpenvelope/nntpprocessing";
    const BSTR cdoKeywords = (wchar_t*) L"urn:schemas-microsoft-com:office:office#Keywords";
    const BSTR cdoArrivalTime = (wchar_t*) L"http://schemas.microsoft.com/cdo/smtpenvelope/arrivaltime";
    const BSTR cdoClientIPAddress = (wchar_t*) L"http://schemas.microsoft.com/cdo/smtpenvelope/clientipaddress";
    const BSTR cdoMessageStatus = (wchar_t*) L"http://schemas.microsoft.com/cdo/smtpenvelope/messagestatus";
    const BSTR cdoPickupFileName = (wchar_t*) L"http://schemas.microsoft.com/cdo/smtpenvelope/pickupfilename";
    const BSTR cdoRecipientList = (wchar_t*) L"http://schemas.microsoft.com/cdo/smtpenvelope/recipientlist";
    const BSTR cdoSenderEmailAddress = (wchar_t*) L"http://schemas.microsoft.com/cdo/smtpenvelope/senderemailaddress";
    const long CDO_E_UNCAUGHT_EXCEPTION = -2147220991;
    const long CDO_E_NOT_OPENED = -2147220990;
    const long CDO_E_UNSUPPORTED_DATASOURCE = -2147220989;
    const long CDO_E_INVALID_PROPERTYNAME = -2147220988;
    const long CDO_E_PROP_UNSUPPORTED = -2147220987;
    const long CDO_E_INACTIVE = -2147220986;
    const long CDO_E_NO_SUPPORT_FOR_OBJECTS = -2147220985;
    const long CDO_E_NOT_AVAILABLE = -2147220984;
    const long CDO_E_NO_DEFAULT_DROP_DIR = -2147220983;
    const long CDO_E_SMTP_SERVER_REQUIRED = -2147220982;
    const long CDO_E_NNTP_SERVER_REQUIRED = -2147220981;
    const long CDO_E_RECIPIENT_MISSING = -2147220980;
    const long CDO_E_FROM_MISSING = -2147220979;
    const long CDO_E_SENDER_REJECTED = -2147220978;
    const long CDO_E_RECIPIENTS_REJECTED = -2147220977;
    const long CDO_E_NNTP_POST_FAILED = -2147220976;
    const long CDO_E_SMTP_SEND_FAILED = -2147220975;
    const long CDO_E_CONNECTION_DROPPED = -2147220974;
    const long CDO_E_FAILED_TO_CONNECT = -2147220973;
    const long CDO_E_INVALID_POST = -2147220972;
    const long CDO_E_AUTHENTICATION_FAILURE = -2147220971;
    const long CDO_E_INVALID_CONTENT_TYPE = -2147220970;
    const long CDO_E_LOGON_FAILURE = -2147220969;
    const long CDO_E_HTTP_NOT_FOUND = -2147220968;
    const long CDO_E_HTTP_FORBIDDEN = -2147220967;
    const long CDO_E_HTTP_FAILED = -2147220966;
    const long CDO_E_MULTIPART_NO_DATA = -2147220965;
    const long CDO_E_INVALID_ENCODING_FOR_MULTIPART = -2147220964;
    const long CDO_E_UNSAFE_OPERATION = -2147220963;
    const long CDO_E_PROP_NOT_FOUND = -2147220962;
    const long CDO_E_INVALID_SEND_OPTION = -2147220960;
    const long CDO_E_INVALID_POST_OPTION = -2147220959;
    const long CDO_E_NO_PICKUP_DIR = -2147220958;
    const long CDO_E_NOT_ALL_DELETED = -2147220957;
    const long CDO_E_NO_METHOD = -2147220956;
    const long CDO_E_PROP_READONLY = -2147220953;
    const long CDO_E_PROP_CANNOT_DELETE = -2147220952;
    const long CDO_E_BAD_DATA = -2147220951;
    const long CDO_E_PROP_NONHEADER = -2147220950;
    const long CDO_E_INVALID_CHARSET = -2147220949;
    const long CDO_E_ADOSTREAM_NOT_BOUND = -2147220948;
    const long CDO_E_CONTENTPROPXML_NOT_FOUND = -2147220947;
    const long CDO_E_CONTENTPROPXML_WRONG_CHARSET = -2147220946;
    const long CDO_E_CONTENTPROPXML_PARSE_FAILED = -2147220945;
    const long CDO_E_CONTENTPROPXML_CONVERT_FAILED = -2147220944;
    const long CDO_E_NO_DIRECTORIES_SPECIFIED = -2147220943;
    const long CDO_E_DIRECTORIES_UNREACHABLE = -2147220942;
    const long CDO_E_BAD_SENDER = -2147220941;
    const long CDO_E_SELF_BINDING = -2147220940;
    const long CDO_E_BAD_ATTENDEE_DATA = -2147220939;
    const long CDO_E_ROLE_NOMORE_AVAILABLE = -2147220938;
    const long CDO_E_BAD_TASKTYPE_ONASSIGN = -2147220937;
    const long CDO_E_NOT_ASSIGNEDTO_USER = -2147220936;
    const long CDO_E_OUTOFDATE = -2147220935;
    const long CDO_E_ARGUMENT1 = -2147205120;
    const long CDO_E_ARGUMENT2 = -2147205119;
    const long CDO_E_ARGUMENT3 = -2147205118;
    const long CDO_E_ARGUMENT4 = -2147205117;
    const long CDO_E_ARGUMENT5 = -2147205116;
    const long CDO_E_NOT_FOUND = -2146644475;
    const long CDO_E_INVALID_ENCODING_TYPE = -2146644451;

struct __declspec(uuid("cd000029-8b95-11d1-82db-00c04fb1625d"))
IDataSource : IDispatch
{
    //
    // Property data
    //

    __declspec(property(get=GetSourceClass))
    _bstr_t SourceClass;
    __declspec(property(get=GetSource))
    IUnknownPtr Source;
    __declspec(property(get=GetIsDirty,put=PutIsDirty))
    VARIANT_BOOL IsDirty;
    __declspec(property(get=GetSourceURL))
    _bstr_t SourceURL;
    __declspec(property(get=GetActiveConnection))
    _ConnectionPtr ActiveConnection;

    //
    // Wrapper methods for error-handling
    //

    _bstr_t GetSourceClass ( );
    IUnknownPtr GetSource ( );
    VARIANT_BOOL GetIsDirty ( );
    void PutIsDirty (
        VARIANT_BOOL pIsDirty );
    _bstr_t GetSourceURL ( );
    _ConnectionPtr GetActiveConnection ( );
    HRESULT SaveToObject (
        IUnknown * Source,
        _bstr_t InterfaceName );
    HRESULT OpenObject (
        IUnknown * Source,
        _bstr_t InterfaceName );
    HRESULT SaveTo (
        _bstr_t SourceURL,
        IDispatch * ActiveConnection,
        enum ConnectModeEnum Mode,
        enum RecordCreateOptionsEnum CreateOptions,
        enum RecordOpenOptionsEnum Options,
        _bstr_t UserName,
        _bstr_t Password );
    HRESULT Open (
        _bstr_t SourceURL,
        IDispatch * ActiveConnection,
        enum ConnectModeEnum Mode,
        enum RecordCreateOptionsEnum CreateOptions,
        enum RecordOpenOptionsEnum Options,
        _bstr_t UserName,
        _bstr_t Password );
    HRESULT Save ( );
    HRESULT SaveToContainer (
        _bstr_t ContainerURL,
        IDispatch * ActiveConnection,
        enum ConnectModeEnum Mode,
        enum RecordCreateOptionsEnum CreateOptions,
        enum RecordOpenOptionsEnum Options,
        _bstr_t UserName,
        _bstr_t Password );

    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_SourceClass (
        /*[out,retval]*/ BSTR * varSourceClass ) = 0;
      virtual HRESULT __stdcall get_Source (
        /*[out,retval]*/ IUnknown * * varSource ) = 0;
      virtual HRESULT __stdcall get_IsDirty (
        /*[out,retval]*/ VARIANT_BOOL * pIsDirty ) = 0;
      virtual HRESULT __stdcall put_IsDirty (
        /*[in]*/ VARIANT_BOOL pIsDirty ) = 0;
      virtual HRESULT __stdcall get_SourceURL (
        /*[out,retval]*/ BSTR * varSourceURL ) = 0;
      virtual HRESULT __stdcall get_ActiveConnection (
        /*[out,retval]*/ struct _Connection * * varActiveConnection ) = 0;
      virtual HRESULT __stdcall raw_SaveToObject (
        /*[in]*/ IUnknown * Source,
        /*[in]*/ BSTR InterfaceName ) = 0;
      virtual HRESULT __stdcall raw_OpenObject (
        /*[in]*/ IUnknown * Source,
        /*[in]*/ BSTR InterfaceName ) = 0;
      virtual HRESULT __stdcall raw_SaveTo (
        /*[in]*/ BSTR SourceURL,
        /*[in]*/ IDispatch * ActiveConnection,
        /*[in]*/ enum ConnectModeEnum Mode,
        /*[in]*/ enum RecordCreateOptionsEnum CreateOptions,
        /*[in]*/ enum RecordOpenOptionsEnum Options,
        /*[in]*/ BSTR UserName,
        /*[in]*/ BSTR Password ) = 0;
      virtual HRESULT __stdcall raw_Open (
        /*[in]*/ BSTR SourceURL,
        /*[in]*/ IDispatch * ActiveConnection,
        /*[in]*/ enum ConnectModeEnum Mode,
        /*[in]*/ enum RecordCreateOptionsEnum CreateOptions,
        /*[in]*/ enum RecordOpenOptionsEnum Options,
        /*[in]*/ BSTR UserName,
        /*[in]*/ BSTR Password ) = 0;
      virtual HRESULT __stdcall raw_Save ( ) = 0;
      virtual HRESULT __stdcall raw_SaveToContainer (
        /*[in]*/ BSTR ContainerURL,
        /*[in]*/ IDispatch * ActiveConnection,
        /*[in]*/ enum ConnectModeEnum Mode,
        /*[in]*/ enum RecordCreateOptionsEnum CreateOptions,
        /*[in]*/ enum RecordOpenOptionsEnum Options,
        /*[in]*/ BSTR UserName,
        /*[in]*/ BSTR Password ) = 0;
};

struct __declspec(uuid("cd000022-8b95-11d1-82db-00c04fb1625d"))
IConfiguration : IDispatch
{
    //
    // Property data
    //

    __declspec(property(get=GetFields))
    FieldsPtr Fields;

    //
    // Wrapper methods for error-handling
    //

    FieldsPtr GetFields ( );
    HRESULT Load (
        enum CdoConfigSource LoadFrom,
        _bstr_t URL );
    IDispatchPtr GetInterface (
        _bstr_t Interface );

    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Fields (
        /*[out,retval]*/ struct Fields * * varFields ) = 0;
      virtual HRESULT __stdcall raw_Load (
        /*[in]*/ enum CdoConfigSource LoadFrom,
        /*[in]*/ BSTR URL ) = 0;
      virtual HRESULT __stdcall raw_GetInterface (
        /*[in]*/ BSTR Interface,
        /*[out,retval]*/ IDispatch * * ppUnknown ) = 0;
};

struct __declspec(uuid("cd000001-8b95-11d1-82db-00c04fb1625d"))
Message;
    // [ default ] interface IMessage
    // interface IDataSource
    // interface IBodyPart

struct __declspec(uuid("cd000002-8b95-11d1-82db-00c04fb1625d"))
Configuration;
    // [ default ] interface IConfiguration

struct __declspec(uuid("cd000004-8b95-11d1-82db-00c04fb1625d"))
DropDirectory;
    // [ default ] interface IDropDirectory

struct __declspec(uuid("cd000008-8b95-11d1-82db-00c04fb1625d"))
SMTPConnector;
    // [ default ] interface ISMTPScriptConnector
    // [ default, source ] interface ISMTPOnArrival

struct __declspec(uuid("cd000030-8b95-11d1-82db-00c04fb1625d"))
ISMTPScriptConnector : IDispatch
{};

struct __declspec(uuid("cd000011-8b95-11d1-82db-00c04fb1625d"))
NNTPEarlyConnector;
    // [ default ] interface INNTPEarlyScriptConnector
    // [ default, source ] interface INNTPOnPostEarly

struct __declspec(uuid("cd000034-8b95-11d1-82db-00c04fb1625d"))
INNTPEarlyScriptConnector : IDispatch
{};

struct __declspec(uuid("cd000009-8b95-11d1-82db-00c04fb1625d"))
NNTPPostConnector;
    // [ default ] interface INNTPPostScriptConnector
    // [ default, source ] interface INNTPOnPost

struct __declspec(uuid("cd000031-8b95-11d1-82db-00c04fb1625d"))
INNTPPostScriptConnector : IDispatch
{};

struct __declspec(uuid("cd000010-8b95-11d1-82db-00c04fb1625d"))
NNTPFinalConnector;
    // [ default ] interface INNTPFinalScriptConnector
    // [ default, source ] interface INNTPOnPostFinal

struct __declspec(uuid("cd000032-8b95-11d1-82db-00c04fb1625d"))
INNTPFinalScriptConnector : IDispatch
{};

struct __declspec(uuid("cd000023-8b95-11d1-82db-00c04fb1625d"))
IBodyParts : IDispatch
{
    //
    // Property data
    //

    __declspec(property(get=GetItem))
    IBodyPartPtr Item[];
    __declspec(property(get=GetCount))
    long Count;
    __declspec(property(get=Get_NewEnum))
    IUnknownPtr _NewEnum;

    //
    // Wrapper methods for error-handling
    //

    long GetCount ( );
    IBodyPartPtr GetItem (
        long Index );
    IUnknownPtr Get_NewEnum ( );
    HRESULT Delete (
        const _variant_t & varBP );
    HRESULT DeleteAll ( );
    IBodyPartPtr Add (
        long Index );

    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Count (
        /*[out,retval]*/ long * varCount ) = 0;
      virtual HRESULT __stdcall get_Item (
        /*[in]*/ long Index,
        /*[out,retval]*/ struct IBodyPart * * ppBody ) = 0;
      virtual HRESULT __stdcall get__NewEnum (
        /*[out,retval]*/ IUnknown * * retval ) = 0;
      virtual HRESULT __stdcall raw_Delete (
        /*[in]*/ VARIANT varBP ) = 0;
      virtual HRESULT __stdcall raw_DeleteAll ( ) = 0;
      virtual HRESULT __stdcall raw_Add (
        /*[in]*/ long Index,
        /*[out,retval]*/ struct IBodyPart * * ppPart ) = 0;
};

struct __declspec(uuid("cd000021-8b95-11d1-82db-00c04fb1625d"))
IBodyPart : IDispatch
{
    //
    // Property data
    //

    __declspec(property(get=GetBodyParts))
    IBodyPartsPtr BodyParts;
    __declspec(property(get=GetContentTransferEncoding,put=PutContentTransferEncoding))
    _bstr_t ContentTransferEncoding;
    __declspec(property(get=GetContentMediaType,put=PutContentMediaType))
    _bstr_t ContentMediaType;
    __declspec(property(get=GetFields))
    FieldsPtr Fields;
    __declspec(property(get=GetCharset,put=PutCharset))
    _bstr_t Charset;
    __declspec(property(get=GetFileName))
    _bstr_t FileName;
    __declspec(property(get=GetDataSource))
    IDataSourcePtr DataSource;
    __declspec(property(get=GetContentClass,put=PutContentClass))
    _bstr_t ContentClass;
    __declspec(property(get=GetContentClassName,put=PutContentClassName))
    _bstr_t ContentClassName;
    __declspec(property(get=GetParent))
    IBodyPartPtr Parent;

    //
    // Wrapper methods for error-handling
    //

    IBodyPartsPtr GetBodyParts ( );
    _bstr_t GetContentTransferEncoding ( );
    void PutContentTransferEncoding (
        _bstr_t pContentTransferEncoding );
    _bstr_t GetContentMediaType ( );
    void PutContentMediaType (
        _bstr_t pContentMediaType );
    FieldsPtr GetFields ( );
    _bstr_t GetCharset ( );
    void PutCharset (
        _bstr_t pCharset );
    _bstr_t GetFileName ( );
    IDataSourcePtr GetDataSource ( );
    _bstr_t GetContentClass ( );
    void PutContentClass (
        _bstr_t pContentClass );
    _bstr_t GetContentClassName ( );
    void PutContentClassName (
        _bstr_t pContentClassName );
    IBodyPartPtr GetParent ( );
    IBodyPartPtr AddBodyPart (
        long Index );
    HRESULT SaveToFile (
        _bstr_t FileName );
    _StreamPtr GetEncodedContentStream ( );
    _StreamPtr GetDecodedContentStream ( );
    _StreamPtr GetStream ( );
    _bstr_t GetFieldParameter (
        _bstr_t FieldName,
        _bstr_t Parameter );
    IDispatchPtr GetInterface (
        _bstr_t Interface );

    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_BodyParts (
        /*[out,retval]*/ struct IBodyParts * * varBodyParts ) = 0;
      virtual HRESULT __stdcall get_ContentTransferEncoding (
        /*[out,retval]*/ BSTR * pContentTransferEncoding ) = 0;
      virtual HRESULT __stdcall put_ContentTransferEncoding (
        /*[in]*/ BSTR pContentTransferEncoding ) = 0;
      virtual HRESULT __stdcall get_ContentMediaType (
        /*[out,retval]*/ BSTR * pContentMediaType ) = 0;
      virtual HRESULT __stdcall put_ContentMediaType (
        /*[in]*/ BSTR pContentMediaType ) = 0;
      virtual HRESULT __stdcall get_Fields (
        /*[out,retval]*/ struct Fields * * varFields ) = 0;
      virtual HRESULT __stdcall get_Charset (
        /*[out,retval]*/ BSTR * pCharset ) = 0;
      virtual HRESULT __stdcall put_Charset (
        /*[in]*/ BSTR pCharset ) = 0;
      virtual HRESULT __stdcall get_FileName (
        /*[out,retval]*/ BSTR * varFileName ) = 0;
      virtual HRESULT __stdcall get_DataSource (
        /*[out,retval]*/ struct IDataSource * * varDataSource ) = 0;
      virtual HRESULT __stdcall get_ContentClass (
        /*[out,retval]*/ BSTR * pContentClass ) = 0;
      virtual HRESULT __stdcall put_ContentClass (
        /*[in]*/ BSTR pContentClass ) = 0;
      virtual HRESULT __stdcall get_ContentClassName (
        /*[out,retval]*/ BSTR * pContentClassName ) = 0;
      virtual HRESULT __stdcall put_ContentClassName (
        /*[in]*/ BSTR pContentClassName ) = 0;
      virtual HRESULT __stdcall get_Parent (
        /*[out,retval]*/ struct IBodyPart * * varParent ) = 0;
      virtual HRESULT __stdcall raw_AddBodyPart (
        /*[in]*/ long Index,
        /*[out,retval]*/ struct IBodyPart * * ppPart ) = 0;
      virtual HRESULT __stdcall raw_SaveToFile (
        /*[in]*/ BSTR FileName ) = 0;
      virtual HRESULT __stdcall raw_GetEncodedContentStream (
        /*[out,retval]*/ struct _Stream * * ppStream ) = 0;
      virtual HRESULT __stdcall raw_GetDecodedContentStream (
        /*[out,retval]*/ struct _Stream * * ppStream ) = 0;
      virtual HRESULT __stdcall raw_GetStream (
        /*[out,retval]*/ struct _Stream * * ppStream ) = 0;
      virtual HRESULT __stdcall raw_GetFieldParameter (
        /*[in]*/ BSTR FieldName,
        /*[in]*/ BSTR Parameter,
        /*[out,retval]*/ BSTR * pbstrValue ) = 0;
      virtual HRESULT __stdcall raw_GetInterface (
        /*[in]*/ BSTR Interface,
        /*[out,retval]*/ IDispatch * * ppUnknown ) = 0;
};

struct __declspec(uuid("cd000020-8b95-11d1-82db-00c04fb1625d"))
IMessage : IDispatch
{
    //
    // Property data
    //

    __declspec(property(get=GetBodyPart))
    IBodyPartPtr BodyPart;
    __declspec(property(get=GetDataSource))
    IDataSourcePtr DataSource;
    __declspec(property(get=GetFields))
    FieldsPtr Fields;
    __declspec(property(get=GetMDNRequested,put=PutMDNRequested))
    VARIANT_BOOL MDNRequested;
    __declspec(property(get=GetBCC,put=PutBCC))
    _bstr_t BCC;
    __declspec(property(get=GetCC,put=PutCC))
    _bstr_t CC;
    __declspec(property(get=GetFollowUpTo,put=PutFollowUpTo))
    _bstr_t FollowUpTo;
    __declspec(property(get=GetFrom,put=PutFrom))
    _bstr_t From;
    __declspec(property(get=GetKeywords,put=PutKeywords))
    _bstr_t Keywords;
    __declspec(property(get=GetMimeFormatted,put=PutMimeFormatted))
    VARIANT_BOOL MimeFormatted;
    __declspec(property(get=GetNewsgroups,put=PutNewsgroups))
    _bstr_t Newsgroups;
    __declspec(property(get=GetOrganization,put=PutOrganization))
    _bstr_t Organization;
    __declspec(property(get=GetReceivedTime))
    DATE ReceivedTime;
    __declspec(property(get=GetReplyTo,put=PutReplyTo))
    _bstr_t ReplyTo;
    __declspec(property(get=GetDSNOptions,put=PutDSNOptions))
    enum CdoDSNOptions DSNOptions;
    __declspec(property(get=GetSentOn))
    DATE SentOn;
    __declspec(property(get=GetSubject,put=PutSubject))
    _bstr_t Subject;
    __declspec(property(get=GetTo,put=PutTo))
    _bstr_t To;
    __declspec(property(get=GetTextBody,put=PutTextBody))
    _bstr_t TextBody;
    __declspec(property(get=GetHTMLBody,put=PutHTMLBody))
    _bstr_t HTMLBody;
    __declspec(property(get=GetAttachments))
    IBodyPartsPtr Attachments;
    __declspec(property(get=GetSender,put=PutSender))
    _bstr_t Sender;
    __declspec(property(get=GetConfiguration,put=PutRefConfiguration))
    IConfigurationPtr Configuration;
    __declspec(property(get=GetAutoGenerateTextBody,put=PutAutoGenerateTextBody))
    VARIANT_BOOL AutoGenerateTextBody;
    __declspec(property(get=GetEnvelopeFields))
    FieldsPtr EnvelopeFields;
    __declspec(property(get=GetTextBodyPart))
    IBodyPartPtr TextBodyPart;
    __declspec(property(get=GetHTMLBodyPart))
    IBodyPartPtr HTMLBodyPart;

    //
    // Wrapper methods for error-handling
    //

    _bstr_t GetBCC ( );
    void PutBCC (
        _bstr_t pBCC );
    _bstr_t GetCC ( );
    void PutCC (
        _bstr_t pCC );
    _bstr_t GetFollowUpTo ( );
    void PutFollowUpTo (
        _bstr_t pFollowUpTo );
    _bstr_t GetFrom ( );
    void PutFrom (
        _bstr_t pFrom );
    _bstr_t GetKeywords ( );
    void PutKeywords (
        _bstr_t pKeywords );
    VARIANT_BOOL GetMimeFormatted ( );
    void PutMimeFormatted (
        VARIANT_BOOL pMimeFormatted );
    _bstr_t GetNewsgroups ( );
    void PutNewsgroups (
        _bstr_t pNewsgroups );
    _bstr_t GetOrganization ( );
    void PutOrganization (
        _bstr_t pOrganization );
    DATE GetReceivedTime ( );
    _bstr_t GetReplyTo ( );
    void PutReplyTo (
        _bstr_t pReplyTo );
    enum CdoDSNOptions GetDSNOptions ( );
    void PutDSNOptions (
        enum CdoDSNOptions pDSNOptions );
    DATE GetSentOn ( );
    _bstr_t GetSubject ( );
    void PutSubject (
        _bstr_t pSubject );
    _bstr_t GetTo ( );
    void PutTo (
        _bstr_t pTo );
    _bstr_t GetTextBody ( );
    void PutTextBody (
        _bstr_t pTextBody );
    _bstr_t GetHTMLBody ( );
    void PutHTMLBody (
        _bstr_t pHTMLBody );
    IBodyPartsPtr GetAttachments ( );
    _bstr_t GetSender ( );
    void PutSender (
        _bstr_t pSender );
    IConfigurationPtr GetConfiguration ( );
    void PutConfiguration (
        struct IConfiguration * pConfiguration );
    void PutRefConfiguration (
        struct IConfiguration * pConfiguration );
    VARIANT_BOOL GetAutoGenerateTextBody ( );
    void PutAutoGenerateTextBody (
        VARIANT_BOOL pAutoGenerateTextBody );
    FieldsPtr GetEnvelopeFields ( );
    IBodyPartPtr GetTextBodyPart ( );
    IBodyPartPtr GetHTMLBodyPart ( );
    IBodyPartPtr GetBodyPart ( );
    IDataSourcePtr GetDataSource ( );
    FieldsPtr GetFields ( );
    VARIANT_BOOL GetMDNRequested ( );
    void PutMDNRequested (
        VARIANT_BOOL pMDNRequested );
    IBodyPartPtr AddRelatedBodyPart (
        _bstr_t URL,
        _bstr_t Reference,
        enum CdoReferenceType ReferenceType,
        _bstr_t UserName,
        _bstr_t Password );
    IBodyPartPtr AddAttachment (
        _bstr_t URL,
        _bstr_t UserName,
        _bstr_t Password );
    HRESULT CreateMHTMLBody (
        _bstr_t URL,
        enum CdoMHTMLFlags Flags,
        _bstr_t UserName,
        _bstr_t Password );
    IMessagePtr Forward ( );
    HRESULT Post ( );
    IMessagePtr PostReply ( );
    IMessagePtr Reply ( );
    IMessagePtr ReplyAll ( );
    HRESULT Send ( );
    _StreamPtr GetStream ( );
    IDispatchPtr GetInterface (
        _bstr_t Interface );

    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_BCC (
        /*[out,retval]*/ BSTR * pBCC ) = 0;
      virtual HRESULT __stdcall put_BCC (
        /*[in]*/ BSTR pBCC ) = 0;
      virtual HRESULT __stdcall get_CC (
        /*[out,retval]*/ BSTR * pCC ) = 0;
      virtual HRESULT __stdcall put_CC (
        /*[in]*/ BSTR pCC ) = 0;
      virtual HRESULT __stdcall get_FollowUpTo (
        /*[out,retval]*/ BSTR * pFollowUpTo ) = 0;
      virtual HRESULT __stdcall put_FollowUpTo (
        /*[in]*/ BSTR pFollowUpTo ) = 0;
      virtual HRESULT __stdcall get_From (
        /*[out,retval]*/ BSTR * pFrom ) = 0;
      virtual HRESULT __stdcall put_From (
        /*[in]*/ BSTR pFrom ) = 0;
      virtual HRESULT __stdcall get_Keywords (
        /*[out,retval]*/ BSTR * pKeywords ) = 0;
      virtual HRESULT __stdcall put_Keywords (
        /*[in]*/ BSTR pKeywords ) = 0;
      virtual HRESULT __stdcall get_MimeFormatted (
        /*[out,retval]*/ VARIANT_BOOL * pMimeFormatted ) = 0;
      virtual HRESULT __stdcall put_MimeFormatted (
        /*[in]*/ VARIANT_BOOL pMimeFormatted ) = 0;
      virtual HRESULT __stdcall get_Newsgroups (
        /*[out,retval]*/ BSTR * pNewsgroups ) = 0;
      virtual HRESULT __stdcall put_Newsgroups (
        /*[in]*/ BSTR pNewsgroups ) = 0;
      virtual HRESULT __stdcall get_Organization (
        /*[out,retval]*/ BSTR * pOrganization ) = 0;
      virtual HRESULT __stdcall put_Organization (
        /*[in]*/ BSTR pOrganization ) = 0;
      virtual HRESULT __stdcall get_ReceivedTime (
        /*[out,retval]*/ DATE * varReceivedTime ) = 0;
      virtual HRESULT __stdcall get_ReplyTo (
        /*[out,retval]*/ BSTR * pReplyTo ) = 0;
      virtual HRESULT __stdcall put_ReplyTo (
        /*[in]*/ BSTR pReplyTo ) = 0;
      virtual HRESULT __stdcall get_DSNOptions (
        /*[out,retval]*/ enum CdoDSNOptions * pDSNOptions ) = 0;
      virtual HRESULT __stdcall put_DSNOptions (
        /*[in]*/ enum CdoDSNOptions pDSNOptions ) = 0;
      virtual HRESULT __stdcall get_SentOn (
        /*[out,retval]*/ DATE * varSentOn ) = 0;
      virtual HRESULT __stdcall get_Subject (
        /*[out,retval]*/ BSTR * pSubject ) = 0;
      virtual HRESULT __stdcall put_Subject (
        /*[in]*/ BSTR pSubject ) = 0;
      virtual HRESULT __stdcall get_To (
        /*[out,retval]*/ BSTR * pTo ) = 0;
      virtual HRESULT __stdcall put_To (
        /*[in]*/ BSTR pTo ) = 0;
      virtual HRESULT __stdcall get_TextBody (
        /*[out,retval]*/ BSTR * pTextBody ) = 0;
      virtual HRESULT __stdcall put_TextBody (
        /*[in]*/ BSTR pTextBody ) = 0;
      virtual HRESULT __stdcall get_HTMLBody (
        /*[out,retval]*/ BSTR * pHTMLBody ) = 0;
      virtual HRESULT __stdcall put_HTMLBody (
        /*[in]*/ BSTR pHTMLBody ) = 0;
      virtual HRESULT __stdcall get_Attachments (
        /*[out,retval]*/ struct IBodyParts * * varAttachments ) = 0;
      virtual HRESULT __stdcall get_Sender (
        /*[out,retval]*/ BSTR * pSender ) = 0;
      virtual HRESULT __stdcall put_Sender (
        /*[in]*/ BSTR pSender ) = 0;
      virtual HRESULT __stdcall get_Configuration (
        /*[out,retval]*/ struct IConfiguration * * pConfiguration ) = 0;
      virtual HRESULT __stdcall put_Configuration (
        /*[in]*/ struct IConfiguration * pConfiguration ) = 0;
      virtual HRESULT __stdcall putref_Configuration (
        /*[in]*/ struct IConfiguration * pConfiguration ) = 0;
      virtual HRESULT __stdcall get_AutoGenerateTextBody (
        /*[out,retval]*/ VARIANT_BOOL * pAutoGenerateTextBody ) = 0;
      virtual HRESULT __stdcall put_AutoGenerateTextBody (
        /*[in]*/ VARIANT_BOOL pAutoGenerateTextBody ) = 0;
      virtual HRESULT __stdcall get_EnvelopeFields (
        /*[out,retval]*/ struct Fields * * varEnvelopeFields ) = 0;
      virtual HRESULT __stdcall get_TextBodyPart (
        /*[out,retval]*/ struct IBodyPart * * varTextBodyPart ) = 0;
      virtual HRESULT __stdcall get_HTMLBodyPart (
        /*[out,retval]*/ struct IBodyPart * * varHTMLBodyPart ) = 0;
      virtual HRESULT __stdcall get_BodyPart (
        /*[out,retval]*/ struct IBodyPart * * varBodyPart ) = 0;
      virtual HRESULT __stdcall get_DataSource (
        /*[out,retval]*/ struct IDataSource * * varDataSource ) = 0;
      virtual HRESULT __stdcall get_Fields (
        /*[out,retval]*/ struct Fields * * varFields ) = 0;
      virtual HRESULT __stdcall get_MDNRequested (
        /*[out,retval]*/ VARIANT_BOOL * pMDNRequested ) = 0;
      virtual HRESULT __stdcall put_MDNRequested (
        /*[in]*/ VARIANT_BOOL pMDNRequested ) = 0;
      virtual HRESULT __stdcall raw_AddRelatedBodyPart (
        /*[in]*/ BSTR URL,
        /*[in]*/ BSTR Reference,
        /*[in]*/ enum CdoReferenceType ReferenceType,
        /*[in]*/ BSTR UserName,
        /*[in]*/ BSTR Password,
        /*[out,retval]*/ struct IBodyPart * * ppBody ) = 0;
      virtual HRESULT __stdcall raw_AddAttachment (
        /*[in]*/ BSTR URL,
        /*[in]*/ BSTR UserName,
        /*[in]*/ BSTR Password,
        /*[out,retval]*/ struct IBodyPart * * ppBody ) = 0;
      virtual HRESULT __stdcall raw_CreateMHTMLBody (
        /*[in]*/ BSTR URL,
        /*[in]*/ enum CdoMHTMLFlags Flags,
        /*[in]*/ BSTR UserName,
        /*[in]*/ BSTR Password ) = 0;
      virtual HRESULT __stdcall raw_Forward (
        /*[out,retval]*/ struct IMessage * * ppMsg ) = 0;
      virtual HRESULT __stdcall raw_Post ( ) = 0;
      virtual HRESULT __stdcall raw_PostReply (
        /*[out,retval]*/ struct IMessage * * ppMsg ) = 0;
      virtual HRESULT __stdcall raw_Reply (
        /*[out,retval]*/ struct IMessage * * ppMsg ) = 0;
      virtual HRESULT __stdcall raw_ReplyAll (
        /*[out,retval]*/ struct IMessage * * ppMsg ) = 0;
      virtual HRESULT __stdcall raw_Send ( ) = 0;
      virtual HRESULT __stdcall raw_GetStream (
        /*[out,retval]*/ struct _Stream * * ppStream ) = 0;
      virtual HRESULT __stdcall raw_GetInterface (
        /*[in]*/ BSTR Interface,
        /*[out,retval]*/ IDispatch * * ppUnknown ) = 0;
};

struct __declspec(uuid("cd000025-8b95-11d1-82db-00c04fb1625d"))
IMessages : IDispatch
{
    //
    // Property data
    //

    __declspec(property(get=GetCount))
    long Count;
    __declspec(property(get=Get_NewEnum))
    IUnknownPtr _NewEnum;

    //
    // Wrapper methods for error-handling
    //

    IMessagePtr GetItem (
        long Index );
    long GetCount ( );
    HRESULT Delete (
        long Index );
    HRESULT DeleteAll ( );
    IUnknownPtr Get_NewEnum ( );
    _bstr_t GetFileName (
        const _variant_t & var );

    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall get_Item (
        long Index,
        /*[out,retval]*/ struct IMessage * * ppMessage ) = 0;
      virtual HRESULT __stdcall get_Count (
        /*[out,retval]*/ long * varCount ) = 0;
      virtual HRESULT __stdcall raw_Delete (
        /*[in]*/ long Index ) = 0;
      virtual HRESULT __stdcall raw_DeleteAll ( ) = 0;
      virtual HRESULT __stdcall get__NewEnum (
        /*[out,retval]*/ IUnknown * * retval ) = 0;
      virtual HRESULT __stdcall get_FileName (
        VARIANT var,
        /*[out,retval]*/ BSTR * FileName ) = 0;
};

struct __declspec(uuid("cd000024-8b95-11d1-82db-00c04fb1625d"))
IDropDirectory : IDispatch
{
    //
    // Wrapper methods for error-handling
    //

    IMessagesPtr GetMessages (
        _bstr_t DirName );

    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall raw_GetMessages (
        /*[in]*/ BSTR DirName,
        /*[out,retval]*/ struct IMessages * * Msgs ) = 0;
};

struct __declspec(uuid("cd000026-8b95-11d1-82db-00c04fb1625d"))
ISMTPOnArrival : IDispatch
{
    //
    // Wrapper methods for error-handling
    //

    HRESULT OnArrival (
        struct IMessage * Msg,
        enum CdoEventStatus * EventStatus );

    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall raw_OnArrival (
        /*[in]*/ struct IMessage * Msg,
        /*[in,out]*/ enum CdoEventStatus * EventStatus ) = 0;
};

struct __declspec(uuid("cd000033-8b95-11d1-82db-00c04fb1625d"))
INNTPOnPostEarly : IDispatch
{
    //
    // Wrapper methods for error-handling
    //

    HRESULT OnPostEarly (
        struct IMessage * Msg,
        enum CdoEventStatus * EventStatus );

    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall raw_OnPostEarly (
        /*[in]*/ struct IMessage * Msg,
        /*[in,out]*/ enum CdoEventStatus * EventStatus ) = 0;
};

struct __declspec(uuid("cd000027-8b95-11d1-82db-00c04fb1625d"))
INNTPOnPost : IDispatch
{
    //
    // Wrapper methods for error-handling
    //

    HRESULT OnPost (
        struct IMessage * Msg,
        enum CdoEventStatus * EventStatus );

    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall raw_OnPost (
        /*[in]*/ struct IMessage * Msg,
        /*[in,out]*/ enum CdoEventStatus * EventStatus ) = 0;
};

struct __declspec(uuid("cd000028-8b95-11d1-82db-00c04fb1625d"))
INNTPOnPostFinal : IDispatch
{
    //
    // Wrapper methods for error-handling
    //

    HRESULT OnPostFinal (
        struct IMessage * Msg,
        enum CdoEventStatus * EventStatus );

    //
    // Raw methods provided by interface
    //

      virtual HRESULT __stdcall raw_OnPostFinal (
        /*[in]*/ struct IMessage * Msg,
        /*[in,out]*/ enum CdoEventStatus * EventStatus ) = 0;
};

///
//
// Wrapper method implementations
//
///

inline _bstr_t IDataSource::GetSourceClass ( ) {
    BSTR _result = 0;
    HRESULT _hr = get_SourceClass(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _bstr_t(_result, false);
}

inline IUnknownPtr IDataSource::GetSource ( ) {
    IUnknown * _result = 0;
    HRESULT _hr = get_Source(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return IUnknownPtr(_result, false);
}

inline VARIANT_BOOL IDataSource::GetIsDirty ( ) {
    VARIANT_BOOL _result = 0;
    HRESULT _hr = get_IsDirty(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _result;
}

inline void IDataSource::PutIsDirty ( VARIANT_BOOL pIsDirty ) {
    HRESULT _hr = put_IsDirty(pIsDirty);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
}

inline _bstr_t IDataSource::GetSourceURL ( ) {
    BSTR _result = 0;
    HRESULT _hr = get_SourceURL(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _bstr_t(_result, false);
}

inline _ConnectionPtr IDataSource::GetActiveConnection ( ) {
    struct _Connection * _result = 0;
    HRESULT _hr = get_ActiveConnection(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _ConnectionPtr(_result, false);
}

inline HRESULT IDataSource::SaveToObject ( IUnknown * Source, _bstr_t InterfaceName ) {
    HRESULT _hr = raw_SaveToObject(Source, InterfaceName);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _hr;
}

inline HRESULT IDataSource::OpenObject ( IUnknown * Source, _bstr_t InterfaceName ) {
    HRESULT _hr = raw_OpenObject(Source, InterfaceName);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _hr;
}

inline HRESULT IDataSource::SaveTo ( _bstr_t SourceURL, IDispatch * ActiveConnection, enum ConnectModeEnum Mode, enum RecordCreateOptionsEnum CreateOptions, enum RecordOpenOptionsEnum Options, _bstr_t UserName, _bstr_t Password ) {
    HRESULT _hr = raw_SaveTo(SourceURL, ActiveConnection, Mode, CreateOptions, Options, UserName, Password);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _hr;
}

inline HRESULT IDataSource::Open ( _bstr_t SourceURL, IDispatch * ActiveConnection, enum ConnectModeEnum Mode, enum RecordCreateOptionsEnum CreateOptions, enum RecordOpenOptionsEnum Options, _bstr_t UserName, _bstr_t Password ) {
    HRESULT _hr = raw_Open(SourceURL, ActiveConnection, Mode, CreateOptions, Options, UserName, Password);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _hr;
}

inline HRESULT IDataSource::Save ( ) {
    HRESULT _hr = raw_Save();
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _hr;
}

inline HRESULT IDataSource::SaveToContainer ( _bstr_t ContainerURL, IDispatch * ActiveConnection, enum ConnectModeEnum Mode, enum RecordCreateOptionsEnum CreateOptions, enum RecordOpenOptionsEnum Options, _bstr_t UserName, _bstr_t Password ) {
    HRESULT _hr = raw_SaveToContainer(ContainerURL, ActiveConnection, Mode, CreateOptions, Options, UserName, Password);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _hr;
}

//
// interface IConfiguration wrapper method implementations
//

inline FieldsPtr IConfiguration::GetFields ( ) {
    struct Fields * _result = 0;
    HRESULT _hr = get_Fields(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return FieldsPtr(_result, false);
}

inline HRESULT IConfiguration::Load ( enum CdoConfigSource LoadFrom, _bstr_t URL ) {
    HRESULT _hr = raw_Load(LoadFrom, URL);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _hr;
}

inline IDispatchPtr IConfiguration::GetInterface ( _bstr_t Interface ) {
    IDispatch * _result = 0;
    HRESULT _hr = raw_GetInterface(Interface, &_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return IDispatchPtr(_result, false);
}

//
// interface IBodyParts wrapper method implementations
//

inline long IBodyParts::GetCount ( ) {
    long _result = 0;
    HRESULT _hr = get_Count(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _result;
}

inline IBodyPartPtr IBodyParts::GetItem ( long Index ) {
    struct IBodyPart * _result = 0;
    HRESULT _hr = get_Item(Index, &_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return IBodyPartPtr(_result, false);
}

inline IUnknownPtr IBodyParts::Get_NewEnum ( ) {
    IUnknown * _result = 0;
    HRESULT _hr = get__NewEnum(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return IUnknownPtr(_result, false);
}

inline HRESULT IBodyParts::Delete ( const _variant_t & varBP ) {
    HRESULT _hr = raw_Delete(varBP);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _hr;
}

inline HRESULT IBodyParts::DeleteAll ( ) {
    HRESULT _hr = raw_DeleteAll();
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _hr;
}

inline IBodyPartPtr IBodyParts::Add ( long Index ) {
    struct IBodyPart * _result = 0;
    HRESULT _hr = raw_Add(Index, &_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return IBodyPartPtr(_result, false);
}

//
// interface IBodyPart wrapper method implementations
//

inline IBodyPartsPtr IBodyPart::GetBodyParts ( ) {
    struct IBodyParts * _result = 0;
    HRESULT _hr = get_BodyParts(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return IBodyPartsPtr(_result, false);
}

inline _bstr_t IBodyPart::GetContentTransferEncoding ( ) {
    BSTR _result = 0;
    HRESULT _hr = get_ContentTransferEncoding(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _bstr_t(_result, false);
}

inline void IBodyPart::PutContentTransferEncoding ( _bstr_t pContentTransferEncoding ) {
    HRESULT _hr = put_ContentTransferEncoding(pContentTransferEncoding);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
}

inline _bstr_t IBodyPart::GetContentMediaType ( ) {
    BSTR _result = 0;
    HRESULT _hr = get_ContentMediaType(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _bstr_t(_result, false);
}

inline void IBodyPart::PutContentMediaType ( _bstr_t pContentMediaType ) {
    HRESULT _hr = put_ContentMediaType(pContentMediaType);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
}

inline FieldsPtr IBodyPart::GetFields ( ) {
    struct Fields * _result = 0;
    HRESULT _hr = get_Fields(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return FieldsPtr(_result, false);
}

inline _bstr_t IBodyPart::GetCharset ( ) {
    BSTR _result = 0;
    HRESULT _hr = get_Charset(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _bstr_t(_result, false);
}

inline void IBodyPart::PutCharset ( _bstr_t pCharset ) {
    HRESULT _hr = put_Charset(pCharset);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
}

inline _bstr_t IBodyPart::GetFileName ( ) {
    BSTR _result = 0;
    HRESULT _hr = get_FileName(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _bstr_t(_result, false);
}

inline IDataSourcePtr IBodyPart::GetDataSource ( ) {
    struct IDataSource * _result = 0;
    HRESULT _hr = get_DataSource(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return IDataSourcePtr(_result, false);
}

inline _bstr_t IBodyPart::GetContentClass ( ) {
    BSTR _result = 0;
    HRESULT _hr = get_ContentClass(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _bstr_t(_result, false);
}

inline void IBodyPart::PutContentClass ( _bstr_t pContentClass ) {
    HRESULT _hr = put_ContentClass(pContentClass);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
}

inline _bstr_t IBodyPart::GetContentClassName ( ) {
    BSTR _result = 0;
    HRESULT _hr = get_ContentClassName(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _bstr_t(_result, false);
}

inline void IBodyPart::PutContentClassName ( _bstr_t pContentClassName ) {
    HRESULT _hr = put_ContentClassName(pContentClassName);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
}

inline IBodyPartPtr IBodyPart::GetParent ( ) {
    struct IBodyPart * _result = 0;
    HRESULT _hr = get_Parent(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return IBodyPartPtr(_result, false);
}

inline IBodyPartPtr IBodyPart::AddBodyPart ( long Index ) {
    struct IBodyPart * _result = 0;
    HRESULT _hr = raw_AddBodyPart(Index, &_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return IBodyPartPtr(_result, false);
}

inline HRESULT IBodyPart::SaveToFile ( _bstr_t FileName ) {
    HRESULT _hr = raw_SaveToFile(FileName);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _hr;
}

inline _StreamPtr IBodyPart::GetEncodedContentStream ( ) {
    struct _Stream * _result = 0;
    HRESULT _hr = raw_GetEncodedContentStream(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _StreamPtr(_result, false);
}

inline _StreamPtr IBodyPart::GetDecodedContentStream ( ) {
    struct _Stream * _result = 0;
    HRESULT _hr = raw_GetDecodedContentStream(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _StreamPtr(_result, false);
}

inline _StreamPtr IBodyPart::GetStream ( ) {
    struct _Stream * _result = 0;
    HRESULT _hr = raw_GetStream(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _StreamPtr(_result, false);
}

inline _bstr_t IBodyPart::GetFieldParameter ( _bstr_t FieldName, _bstr_t Parameter ) {
    BSTR _result = 0;
    HRESULT _hr = raw_GetFieldParameter(FieldName, Parameter, &_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _bstr_t(_result, false);
}

inline IDispatchPtr IBodyPart::GetInterface ( _bstr_t Interface ) {
    IDispatch * _result = 0;
    HRESULT _hr = raw_GetInterface(Interface, &_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return IDispatchPtr(_result, false);
}

//
// interface IMessage wrapper method implementations
//

inline _bstr_t IMessage::GetBCC ( ) {
    BSTR _result = 0;
    HRESULT _hr = get_BCC(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _bstr_t(_result, false);
}

inline void IMessage::PutBCC ( _bstr_t pBCC ) {
    HRESULT _hr = put_BCC(pBCC);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
}

inline _bstr_t IMessage::GetCC ( ) {
    BSTR _result = 0;
    HRESULT _hr = get_CC(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _bstr_t(_result, false);
}

inline void IMessage::PutCC ( _bstr_t pCC ) {
    HRESULT _hr = put_CC(pCC);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
}

inline _bstr_t IMessage::GetFollowUpTo ( ) {
    BSTR _result = 0;
    HRESULT _hr = get_FollowUpTo(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _bstr_t(_result, false);
}

inline void IMessage::PutFollowUpTo ( _bstr_t pFollowUpTo ) {
    HRESULT _hr = put_FollowUpTo(pFollowUpTo);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
}

inline _bstr_t IMessage::GetFrom ( ) {
    BSTR _result = 0;
    HRESULT _hr = get_From(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _bstr_t(_result, false);
}

inline void IMessage::PutFrom ( _bstr_t pFrom ) {
    HRESULT _hr = put_From(pFrom);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
}

inline _bstr_t IMessage::GetKeywords ( ) {
    BSTR _result = 0;
    HRESULT _hr = get_Keywords(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _bstr_t(_result, false);
}

inline void IMessage::PutKeywords ( _bstr_t pKeywords ) {
    HRESULT _hr = put_Keywords(pKeywords);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
}

inline VARIANT_BOOL IMessage::GetMimeFormatted ( ) {
    VARIANT_BOOL _result = 0;
    HRESULT _hr = get_MimeFormatted(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _result;
}

inline void IMessage::PutMimeFormatted ( VARIANT_BOOL pMimeFormatted ) {
    HRESULT _hr = put_MimeFormatted(pMimeFormatted);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
}

inline _bstr_t IMessage::GetNewsgroups ( ) {
    BSTR _result = 0;
    HRESULT _hr = get_Newsgroups(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _bstr_t(_result, false);
}

inline void IMessage::PutNewsgroups ( _bstr_t pNewsgroups ) {
    HRESULT _hr = put_Newsgroups(pNewsgroups);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
}

inline _bstr_t IMessage::GetOrganization ( ) {
    BSTR _result = 0;
    HRESULT _hr = get_Organization(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _bstr_t(_result, false);
}

inline void IMessage::PutOrganization ( _bstr_t pOrganization ) {
    HRESULT _hr = put_Organization(pOrganization);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
}

inline DATE IMessage::GetReceivedTime ( ) {
    DATE _result = 0;
    HRESULT _hr = get_ReceivedTime(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _result;
}

inline _bstr_t IMessage::GetReplyTo ( ) {
    BSTR _result = 0;
    HRESULT _hr = get_ReplyTo(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _bstr_t(_result, false);
}

inline void IMessage::PutReplyTo ( _bstr_t pReplyTo ) {
    HRESULT _hr = put_ReplyTo(pReplyTo);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
}

inline enum CdoDSNOptions IMessage::GetDSNOptions ( ) {
    enum CdoDSNOptions _result;
    HRESULT _hr = get_DSNOptions(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _result;
}

inline void IMessage::PutDSNOptions ( enum CdoDSNOptions pDSNOptions ) {
    HRESULT _hr = put_DSNOptions(pDSNOptions);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
}

inline DATE IMessage::GetSentOn ( ) {
    DATE _result = 0;
    HRESULT _hr = get_SentOn(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _result;
}

inline _bstr_t IMessage::GetSubject ( ) {
    BSTR _result = 0;
    HRESULT _hr = get_Subject(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _bstr_t(_result, false);
}

inline void IMessage::PutSubject ( _bstr_t pSubject ) {
    HRESULT _hr = put_Subject(pSubject);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
}

inline _bstr_t IMessage::GetTo ( ) {
    BSTR _result = 0;
    HRESULT _hr = get_To(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _bstr_t(_result, false);
}

inline void IMessage::PutTo ( _bstr_t pTo ) {
    HRESULT _hr = put_To(pTo);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
}

inline _bstr_t IMessage::GetTextBody ( ) {
    BSTR _result = 0;
    HRESULT _hr = get_TextBody(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _bstr_t(_result, false);
}

inline void IMessage::PutTextBody ( _bstr_t pTextBody ) {
    HRESULT _hr = put_TextBody(pTextBody);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
}

inline _bstr_t IMessage::GetHTMLBody ( ) {
    BSTR _result = 0;
    HRESULT _hr = get_HTMLBody(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _bstr_t(_result, false);
}

inline void IMessage::PutHTMLBody ( _bstr_t pHTMLBody ) {
    HRESULT _hr = put_HTMLBody(pHTMLBody);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
}

inline IBodyPartsPtr IMessage::GetAttachments ( ) {
    struct IBodyParts * _result = 0;
    HRESULT _hr = get_Attachments(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return IBodyPartsPtr(_result, false);
}

inline _bstr_t IMessage::GetSender ( ) {
    BSTR _result = 0;
    HRESULT _hr = get_Sender(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _bstr_t(_result, false);
}

inline void IMessage::PutSender ( _bstr_t pSender ) {
    HRESULT _hr = put_Sender(pSender);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
}

inline IConfigurationPtr IMessage::GetConfiguration ( ) {
    struct IConfiguration * _result = 0;
    HRESULT _hr = get_Configuration(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return IConfigurationPtr(_result, false);
}

inline void IMessage::PutConfiguration ( struct IConfiguration * pConfiguration ) {
    HRESULT _hr = put_Configuration(pConfiguration);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
}

inline void IMessage::PutRefConfiguration ( struct IConfiguration * pConfiguration ) {
    HRESULT _hr = putref_Configuration(pConfiguration);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
}

inline VARIANT_BOOL IMessage::GetAutoGenerateTextBody ( ) {
    VARIANT_BOOL _result = 0;
    HRESULT _hr = get_AutoGenerateTextBody(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _result;
}

inline void IMessage::PutAutoGenerateTextBody ( VARIANT_BOOL pAutoGenerateTextBody ) {
    HRESULT _hr = put_AutoGenerateTextBody(pAutoGenerateTextBody);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
}

inline FieldsPtr IMessage::GetEnvelopeFields ( ) {
    struct Fields * _result = 0;
    HRESULT _hr = get_EnvelopeFields(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return FieldsPtr(_result, false);
}

inline IBodyPartPtr IMessage::GetTextBodyPart ( ) {
    struct IBodyPart * _result = 0;
    HRESULT _hr = get_TextBodyPart(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return IBodyPartPtr(_result, false);
}

inline IBodyPartPtr IMessage::GetHTMLBodyPart ( ) {
    struct IBodyPart * _result = 0;
    HRESULT _hr = get_HTMLBodyPart(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return IBodyPartPtr(_result, false);
}

inline IBodyPartPtr IMessage::GetBodyPart ( ) {
    struct IBodyPart * _result = 0;
    HRESULT _hr = get_BodyPart(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return IBodyPartPtr(_result, false);
}

inline IDataSourcePtr IMessage::GetDataSource ( ) {
    struct IDataSource * _result = 0;
    HRESULT _hr = get_DataSource(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return IDataSourcePtr(_result, false);
}

inline FieldsPtr IMessage::GetFields ( ) {
    struct Fields * _result = 0;
    HRESULT _hr = get_Fields(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return FieldsPtr(_result, false);
}

inline VARIANT_BOOL IMessage::GetMDNRequested ( ) {
    VARIANT_BOOL _result = 0;
    HRESULT _hr = get_MDNRequested(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _result;
}

inline void IMessage::PutMDNRequested ( VARIANT_BOOL pMDNRequested ) {
    HRESULT _hr = put_MDNRequested(pMDNRequested);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
}

inline IBodyPartPtr IMessage::AddRelatedBodyPart ( _bstr_t URL, _bstr_t Reference, enum CdoReferenceType ReferenceType, _bstr_t UserName, _bstr_t Password ) {
    struct IBodyPart * _result = 0;
    HRESULT _hr = raw_AddRelatedBodyPart(URL, Reference, ReferenceType, UserName, Password, &_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return IBodyPartPtr(_result, false);
}

inline IBodyPartPtr IMessage::AddAttachment ( _bstr_t URL, _bstr_t UserName, _bstr_t Password ) {
    struct IBodyPart * _result = 0;
    HRESULT _hr = raw_AddAttachment(URL, UserName, Password, &_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return IBodyPartPtr(_result, false);
}

inline HRESULT IMessage::CreateMHTMLBody ( _bstr_t URL, enum CdoMHTMLFlags Flags, _bstr_t UserName, _bstr_t Password ) {
    HRESULT _hr = raw_CreateMHTMLBody(URL, Flags, UserName, Password);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _hr;
}

inline IMessagePtr IMessage::Forward ( ) {
    struct IMessage * _result = 0;
    HRESULT _hr = raw_Forward(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return IMessagePtr(_result, false);
}

inline HRESULT IMessage::Post ( ) {
    HRESULT _hr = raw_Post();
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _hr;
}

inline IMessagePtr IMessage::PostReply ( ) {
    struct IMessage * _result = 0;
    HRESULT _hr = raw_PostReply(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return IMessagePtr(_result, false);
}

inline IMessagePtr IMessage::Reply ( ) {
    struct IMessage * _result = 0;
    HRESULT _hr = raw_Reply(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return IMessagePtr(_result, false);
}

inline IMessagePtr IMessage::ReplyAll ( ) {
    struct IMessage * _result = 0;
    HRESULT _hr = raw_ReplyAll(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return IMessagePtr(_result, false);
}

inline HRESULT IMessage::Send ( ) {
    HRESULT _hr = raw_Send();
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _hr;
}

inline _StreamPtr IMessage::GetStream ( ) {
    struct _Stream * _result = 0;
    HRESULT _hr = raw_GetStream(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _StreamPtr(_result, false);
}

inline IDispatchPtr IMessage::GetInterface ( _bstr_t Interface ) {
    IDispatch * _result = 0;
    HRESULT _hr = raw_GetInterface(Interface, &_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return IDispatchPtr(_result, false);
}

//
// interface IMessages wrapper method implementations
//

inline IMessagePtr IMessages::GetItem ( long Index ) {
    struct IMessage * _result = 0;
    HRESULT _hr = get_Item(Index, &_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return IMessagePtr(_result, false);
}

inline long IMessages::GetCount ( ) {
    long _result = 0;
    HRESULT _hr = get_Count(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _result;
}

inline HRESULT IMessages::Delete ( long Index ) {
    HRESULT _hr = raw_Delete(Index);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _hr;
}

inline HRESULT IMessages::DeleteAll ( ) {
    HRESULT _hr = raw_DeleteAll();
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _hr;
}

inline IUnknownPtr IMessages::Get_NewEnum ( ) {
    IUnknown * _result = 0;
    HRESULT _hr = get__NewEnum(&_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return IUnknownPtr(_result, false);
}

inline _bstr_t IMessages::GetFileName ( const _variant_t & var ) {
    BSTR _result = 0;
    HRESULT _hr = get_FileName(var, &_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _bstr_t(_result, false);
}

//
// interface IDropDirectory wrapper method implementations
//

inline IMessagesPtr IDropDirectory::GetMessages ( _bstr_t DirName ) {
    struct IMessages * _result = 0;
    HRESULT _hr = raw_GetMessages(DirName, &_result);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return IMessagesPtr(_result, false);
}

//
// interface ISMTPOnArrival wrapper method implementations
//

inline HRESULT ISMTPOnArrival::OnArrival ( struct IMessage * Msg, enum CdoEventStatus * EventStatus ) {
    HRESULT _hr = raw_OnArrival(Msg, EventStatus);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _hr;
}

//
// interface INNTPOnPostEarly wrapper method implementations
//

inline HRESULT INNTPOnPostEarly::OnPostEarly ( struct IMessage * Msg, enum CdoEventStatus * EventStatus ) {
    HRESULT _hr = raw_OnPostEarly(Msg, EventStatus);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _hr;
}

//
// interface INNTPOnPost wrapper method implementations
//

inline HRESULT INNTPOnPost::OnPost ( struct IMessage * Msg, enum CdoEventStatus * EventStatus ) {
    HRESULT _hr = raw_OnPost(Msg, EventStatus);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _hr;
}

//
// interface INNTPOnPostFinal wrapper method implementations
//

inline HRESULT INNTPOnPostFinal::OnPostFinal ( struct IMessage * Msg, enum CdoEventStatus * EventStatus ) {
    HRESULT _hr = raw_OnPostFinal(Msg, EventStatus);
    if (FAILED(_hr)) _com_issue_errorex(_hr, this, __uuidof(this));
    return _hr;
}

} // namespace CDOSYS

#pragma pack(pop)
