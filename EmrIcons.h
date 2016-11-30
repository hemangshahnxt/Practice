#pragma once

// (a.walling 2011-12-09 10:16) - PLID 46642 - Artwork
// (a.walling 2013-04-08 08:57) - PLID 56131 - Added more icons, more!
namespace EmrIcons
{
	enum Common
	{
		None = -1,
	};

	namespace Small
	{
		enum {
			None = -1,
			Save = 0,
			SaveAll,
			SaveAndClose,
			Close,
			AddSubtopic,
			AddTopic,
			ImportSubtopic,
			ImportTopic,
			Item,
			ImageItem,
			SignatureItem,
			TextItem,
			EmnFolder,
			Task,
			Recording,
			Problem,
			Command,
			Import,
			Add,
			Link,
			Topic,
			Emr,
			eRx,
			DrugInteractions,
			Settings,
			Print,
			PrintPreview,
			Summary,
			Checklist,
			Placeholder1,
			Placeholder2,
			EmnHistory,
			EmrHistory,
			Refresh,
			Merge,
			WoundCare,
			Check,
			UpdateDemographics,
			MoreInfo,
			WriteAccess,
			LinkedAppointment,
			ConfidentialInfo,
		};
	}

	namespace Large
	{
		enum {
			None = -1,
			Save = 0,
			SaveAll,
			SaveAndClose,
			Close,
			AddSubtopic,
			AddTopic,
			ImportSubtopic,
			ImportTopic,
			AddItem,
			NewEmn,
			Task,
			Recording,
			Problem,
			Command,
			eRx,
			DrugInteractions,
			Settings,
			Print,
			PrintPreview,
			Summary,
			Checklist,
			EmnHistory,
			EmrHistory,
			Refresh,
			Merge,
			WoundCare,
			Check,
			UpdateDemographics,
			MoreInfo,
			WriteAccess,
			LinkedAppointment,
			ConfidentialInfo,
		};
	}

	namespace Tools
	{
		// (b.savon 2012-02-22 11:02) - PLID 48307 - Added Recall
		enum {
			None = -1,
			Caduceus = 0,
			Graphs,
			Camera,
			EyeMaginations,
			Recall,
			DeviceImport, // (a.walling 2012-04-02 16:51) - PLID 49360 - Device Import handling
			eRx,
			DrugInteraction,	// (j.jones 2012-09-26 15:12) - PLID 52879
		};
	}

	namespace StatusBar
	{
		enum {
			None = -1,
			First = 0,
			Previous,
			Topics,
			Next,
			Last,
		};
	}
}