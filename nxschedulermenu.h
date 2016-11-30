

#pragma once


class CNxSchedulerMenu : public CMenu
{
public:
	typedef enum EPopupStyle {
		eReservation,
		eTemplateBlock,
	} EPopupStyle;

public:
// Operations
	void AppendColorMenuItem(UINT nID, COLORREF color);

// Implementation
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMIS);
	virtual BOOL CreatePopupMenu(EPopupStyle style);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
	CNxSchedulerMenu();
	virtual ~CNxSchedulerMenu();
};
