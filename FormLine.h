#pragma once

#include "FormControl.h"

class FormLine : public FormControl  
{
public:
	FormLine();
	FormLine(FormControl *control);
	FormLine (int setx1, int sety1, int setx2, int sety2, int setwidth, int setformat, COLORREF setcolor);
	int			x1, 
				y1, 
				x2, 
				y2;
	COLORREF	color;
	virtual		~FormLine	();
	virtual void Draw		(CDC *dc);
	bool		Collide		(int x, int y);
	RECT		GetRect		(void);
	virtual	void PrintOut	(CDC *pDC);
//	virtual void	DeleteThis(void){delete this;}
};