#pragma once

// (a.walling 2012-05-31 14:49) - PLID 50719 - EmrColors


namespace EmrColors
{

class Color
{
public:
	Color()
		: m_color(0)
		, m_brush(NULL)
	{}

	Color(COLORREF color, HBRUSH brush)
		: m_color(color)
		, m_brush(brush)
	{}

	operator COLORREF() const
	{
		return m_color;
	}

	operator HBRUSH() const
	{
		return m_brush;
	}
protected:
	COLORREF m_color;
	HBRUSH m_brush;
};

namespace Topic
{
	/// Main background color
	const Color& PatientBackground();
	const Color& TemplateBackground();
	inline const Color& Background(bool bIsTemplate)
	{
		if (bIsTemplate) {
			return TemplateBackground();
		} else {
			return PatientBackground();
		}
	}

	/// Tree stateful colors
	const Color& Partial();
	const Color& Required();
	const Color& Complete();
	const Color& Locked();
} // namespace Topic

namespace Item
{
	const Color& Background();
} // namespace Item

} // namespace EmrColors
