#pragma once

// (a.walling 2011-11-11 11:11) - PLID 46621 - Unify font handing for emr items

// (a.walling 2011-11-11 11:11) - PLID 46621 - This thin wrapper class just makes it easier to automatically cast
// from the HFONT to a long for automation
struct EmrFont
{
	EmrFont()
		: hFont(NULL)
	{
	}

	EmrFont(const EmrFont& r)
		: hFont(r.hFont)
	{
	}

	EmrFont& operator=(const EmrFont& r)
	{
		hFont = r.hFont;
		return *this;
	}

	EmrFont(HFONT hFont)
		: hFont(hFont)
	{
	}

	EmrFont(long nFont)
		: hFont(reinterpret_cast<HFONT>(nFont))
	{
	}

	operator HFONT() const
	{
		return hFont;
	}

	operator long() const
	{
		return reinterpret_cast<long>(hFont);
	}

	operator CFont*() const
	{
		if (!hFont) {
			ASSERT(FALSE);
			return NULL;
		}

		return CFont::FromHandle(hFont);
	}

	// safe bool
	typedef void (EmrFont::*unspecified_bool_type)() const;
	operator unspecified_bool_type() const
	{
		if (!this || !hFont) return NULL;
		return &EmrFont::bool_helper;
	}

	bool operator!() const
	{
		return NULL == hFont;
	}

protected:
	void bool_helper() const {}

protected:
	HFONT hFont;
};

namespace EmrFonts
{
	EmrFont GetTopicHeaderFont();
	EmrFont GetTopicHeaderWebdingsFont();
	EmrFont GetTopicHeaderSmallerFont();

	EmrFont GetTitleFont();
	
	EmrFont GetRegularFont();
	EmrFont GetBoldFont();
	EmrFont GetUnderlineFont();
	
	EmrFont GetSmallFont(); // (z.manning 2012-07-20 15:01) - PLID 51676	
	EmrFont GetTinyUnderlineFont();

	// (a.walling 2012-10-03 12:09) - PLID 53002 - List fonts may use the custom font in properties
	EmrFont GetRegularListFont();
	EmrFont GetBoldListFont();
	EmrFont GetUnderlineListFont();
};
