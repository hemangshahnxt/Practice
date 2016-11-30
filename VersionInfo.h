#ifndef VERSION_INFO_CONTAINER_H
#define VERSION_INFO_CONTAINER_H

#pragma once

// The ...VERSION_BASE macros must take integer literals (don't use defines or variables), 
// and always use the correct number of digits for each field (n#.##.#nnn.#nnn)

// The version of Practice.  To be updated as often as desired.  MUST be updated when the 
// .exe is being put into a release.  And it can't hurt to update every time it's put into 
// latest, though that's not strictly necessary.
#define PRODUCT_VERSION_BASE	(125, 16, 1110, 1130)

//data version, to be updated when new mods are added
//also needs to be changed in ConnectionInfo.h of NxWeb project
#define DATA_VERSION_BASE		(125, 16, 1110, 1130)

//Version for the license server, used for communicating with the activation server for 
//backward compatibility requesting update codes
// (r.gonet 2016-05-10) - This is also defined in NxLicenseServer.h and NxActivationUtils.h in NxLicenseConfig..
// (d.thompson 2015-04-17 10:26) - I rearranged these comments for readability when I'm building releases.  Please put 
//	future updates in descending order BELOW the version.
#define LICENSE_VERSION   48
// Version 48 - EnterpriseReporting
// (c.haag 2016-10-26) - NX-102370 - Version 47 - Active Directory Login
// (r.gonet 2016-05-10) - NX-100477 - Version 46 - AzureRemoteApp
// (b.cardillo 2016-03-08 14:02) - PLID 68409 - Version 45 - Iagnosis provider count
// (b.eyers - 2016-03-03) - PLID 68480 - Version 44 - Subscription Local Hosted
// (r.farnworth 2015-11-11 11:30) - PLID 66407 - Version 43 - MPV - Enhanced Messaging
// (z.manning 2015-07-21 13:20) - PLID 66599 - Version 42 - ICCP
// (z.manning 2015-06-18 09:55) - PLID 66287 - Version 41 - Portal Providers
// (z.manning 2015-06-17 09:09) - PLID 66407 - Version 40 - MPVPatients
// (a.walling 2015-05-06 15:49) - PLID 65920 - Version 39 - Analytics
// (z.manning 2015-05-12 15:04) - PLID 65961 - Version 38 - StateASCReports
// (b.spivey - August 8th, 2014) - PLID 62926
// (a.wilson 2014-07-11 14:42) - PLID 62516 - Version 36 - VisionPayments
// (j.armen 2014-02-04 14:07) - PLID 60634 - Version 35 - Nuance License Guid
// (b.spivey - November 25th, 2013) - PLID 59590 - Version 34 - Direct Message
// (z.manning 2013-08-26 17:43) - PLID 55816 - Version 33 - Anchor info
// (z.manning 2013-06-26 15:57) - PLID 57306 - Version 32 - Clearing used iPads
// (z.manning 2013-01-31 13:43) - PLID 54954 - Version 31 - Nuance dictation licensing
// (j.luckoski 2013-01-23 13:50) - PLID 49892 - Version 30 - ERX pRescribers
// (z.manning 2012-06-11 15:03) - PLID 50877 - Version 29 - iPad
// (j.armen 2012-05-30 13:56) - PLID 50688 - NexEMRCosmetic - Version 28
// (j.armen 2012-03-27 16:46) - PLID 49244 - Recall - Version 27
// (d.singleton 2012-03-21 11:49) - PLID 49086 license for code correct version 27
// (z.manning 2011-12-05 10:01) - PLID 44162 - Version 26 - NexWeb EMR license object
// (a.wilson 2011-11-8) PLID 46335 - Eyemaginations license 25
// (d.thompson 2011-10-14) - PLID 45792 - Version 24 for Concurrent TS licensing
//(a.wilson 2011-8-30) PLID 44686 - Added PQRS Licensing 23
//TES 6/20/2011 - PLID 43864 - Changed workstation count from BYTE to WORD, version 22
// (z.manning 2011-01-11 10:44) - PLID 42057 - Patient check-in version 21
//TES 12/9/2010 - PLID 41701 - Glasses Orders, version 20
// (d.lange 2010-09-08 09:50) - PLID 40370 - Chase CC Processing, which upped ver to 19
// (c.haag 2010-06-29 17:08) - PLID 39402 - Frames, which upped ver to 18
// (j.gruber 2010-06-03 11:31) - PLID 38935 - added BOLD licensing 17
// (c.haag 2010-06-01 15:40) - PLID 38965 - Added CellTrust licensing, which upped to ver 16
// (j.jones 2009-11-04 08:56) - PLID 36135 - added TOPS license, which upped to ver. 15
// (c.haag 2009-10-28 10:32) - PLID 36067 - NexPhoto licensing is also included in version 14
// (z.manning 2009-10-16 15:09) - PLID 35749 - Added NexSync licensing, up to 14
//TES 10/7/2009 - PLID 35802 - Added FirstDataBank licensing, stays at 13
//TES 10/7/2009 - PLID 35171 - Added new NexWeb license objects, up to 13
// (z.manning 2009-04-01 15:42) - PLID 32737 - Added Practice login expiration date, up to 12
// (d.thompson 2009-03-24) - PLID 33583 - Added licensing for NewCrop, up to 11
//DRT 11/20/2008 - PLID 32085 - Added licensing for ePrescribe, up to 10
//DRT 10/22/2008 - PLID 31788 - Added licensing for Cycle of Care, version remains 9
//DRT 10/9/2008 - PLID 31491 - Added licensing for Standard modules, bump to 9
//DRT 7/31/2008 - PLID 30890 - Added faxing & barcode reading, version remains 8
// (a.walling 2008-07-16 15:50) - PLID 30734 - upped to 8
// (a.walling 2008-02-14 13:05) - PLID 28388 - upped to 7
// (a.walling 2007-07-30 12:54) - PLID 26758 - upped to 6
// (j.jones 2007-06-28 16:39) - PLID 23951 & PLID 23952 - upped to version 5
// (a.walling 2006-12-18 17:09) - PLID 23932 - Version 3 adds support for EMR Provider licensing
// (a.walling 2006-12-18 17:09) - Version 2 added support for labs	



///////////////////////////////////////////////////////////////////////////////////
// No need to edit below this line
///////////////////////////////////////////////////////////////////////////////////
// The macros below are to make the product and data versions accessible in various 
// formats as needed throughout the program.
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

// this produces: "n#.##.#nnn.#nnn\0"
#define MAKEVERTEXT(major, minor, build, patch)	#major "." #minor "." #build "." #patch "\0"
// this produces: n#, ##, #nnn, #
#define MAKEVERVAL(major, minor, build, patch)		major, minor, build, patch
// this produces an integer: n###nnn#nnn
#define MAKEVERLONG(major, minor, build, patch)	((major)*1000000) + ((minor)*10000) + (build)

#define PRODUCT_VERSION_TEXT		MAKEVERTEXT		PRODUCT_VERSION_BASE
#define PRODUCT_VERSION_VAL			MAKEVERVAL		PRODUCT_VERSION_BASE
#define PRODUCT_VERSION_LONG		MAKEVERLONG		PRODUCT_VERSION_BASE

#define DATA_VERSION_TEXT			MAKEVERTEXT		DATA_VERSION_BASE
#define DATA_VERSION_VAL			MAKEVERVAL		DATA_VERSION_BASE
#define DATA_VERSION_LONG			MAKEVERLONG		DATA_VERSION_BASE

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

#endif
