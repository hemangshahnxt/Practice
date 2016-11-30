#include "StdAfx.h"
#include "EmrColors.h"

// (a.walling 2012-05-31 14:49) - PLID 50719 - EmrColors

namespace EmrColors
{

namespace
{
class ColorInstance : public Color
{
public:
	~ColorInstance()
	{
		DestroyBrush();
	}

	void DestroyBrush()
	{
		if (m_brush) {
			::DeleteObject(m_brush);
			m_brush = NULL;
		}
	}

	void Reset(COLORREF color)
	{
		DestroyBrush();

		m_color = color & HEXRGB(0xFFFFFF);
		m_brush = ::CreateSolidBrush(m_color);
	}

	// safe bool
	typedef void (ColorInstance::*unspecified_bool_type)() const;
	operator unspecified_bool_type() const
	{
		if (!this || !m_brush) return NULL;
		return &ColorInstance::bool_helper;
	}

	bool operator!() const
	{
		return NULL == m_brush;
	}

protected:
	void bool_helper() const {}
};
}

//TES 1/9/2009 - PLID 32212 - Copied these #defines for the default EMR colors out of the old preferences.
//#define OLD_DEFAULT_COLOR_TOP			0x00CFE3BB
#define OLD_DEFAULT_COLOR_BOTTOM		0x00CFE3BB
#define OLD_DEFAULT_COLOR_PARTIAL		RGB(255, 253, 170)
#define OLD_DEFAULT_COLOR_FULL			RGB(192, 255, 192)
#define OLD_DEFAULT_COLOR_PEN			RGB(255, 0, 0)
// (j.jones 2007-06-15 09:26) - PLID 26297 - added locked color preference
#define OLD_DEFAULT_COLOR_LOCKED		RGB(192,192,192)
// (d.thompson 2011-05-10) - PLID 43123 - Added ability to set default for item's backgrounds
#define OLD_DEFAULT_COLOR_ITEMS			RGB(233, 231, 254)
// (b.cardillo 2012-03-09 16:49) - PLID 48790 - Preference for topics with unfilled required details
#define OLD_DEFAULT_COLOR_REQUIRED		RGB(0xFF, 0x66, 0x66)

#define NEW_DEFAULT_COLOR_ITEMS			HEXRGB(0xFFFFFF)
#define NEW_DEFAULT_COLOR_BOTTOM		HEXRGB(0xedf5ff)

namespace Topic
{
	/// Main background color
	const Color& PatientBackground()
	{
		static ColorInstance color;
		if (!color) {
			COLORREF cr = (COLORREF)GetRemotePropertyInt("EMRColorBottom", OLD_DEFAULT_COLOR_BOTTOM, 0, GetCurrentUserName(), true);
			if (cr == OLD_DEFAULT_COLOR_BOTTOM) {
				cr = NEW_DEFAULT_COLOR_BOTTOM;
			}
			color.Reset(cr);
		}
		return color;
	}

	const Color& TemplateBackground()
	{
		static ColorInstance color;
		if (!color) {
			color.Reset((COLORREF)GetNxColor(GNC_ADMIN, 0));
		}
		return color;
	}

	/// Tree stateful colors
	const Color& Partial()
	{
		static ColorInstance color;
		if (!color) {
			color.Reset((COLORREF)GetRemotePropertyInt("EMRColorPartial", OLD_DEFAULT_COLOR_PARTIAL, 0, GetCurrentUserName(), true));
		}
		return color;
	}

	const Color& Required()
	{
		static ColorInstance color;
		if (!color) {
			color.Reset((COLORREF)GetRemotePropertyInt("EMRColorRequired", OLD_DEFAULT_COLOR_REQUIRED, 0, GetCurrentUserName(), true));
		}
		return color;
	}

	const Color& Complete()
	{
		static ColorInstance color;
		if (!color) {
			color.Reset((COLORREF)GetRemotePropertyInt("EMRColorFull", OLD_DEFAULT_COLOR_FULL, 0, GetCurrentUserName(), true));
		}
		return color;
	}

	const Color& Locked()
	{
		static ColorInstance color;
		if (!color) {
			color.Reset((COLORREF)GetRemotePropertyInt("EMRColorLocked", OLD_DEFAULT_COLOR_LOCKED, 0, GetCurrentUserName(), true));
		}
		return color;
	}

} // namespace Topic

namespace Item
{
	const Color& Background()
	{
		static ColorInstance color;
		if (!color) {
			COLORREF cr = (COLORREF)GetRemotePropertyInt("EMRItemColor", OLD_DEFAULT_COLOR_ITEMS, 0, GetCurrentUserName(), true);
			if (cr == OLD_DEFAULT_COLOR_ITEMS) {
				cr = NEW_DEFAULT_COLOR_ITEMS;
			}
			color.Reset(cr);
		}
		return color;
	}
} // namespace Item

} // namespace EmrColors
