// FormLine.cpp: implementation of the FormLine class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FormLine.h"
#include "FormFormat.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FormLine::FormLine()
{

}

FormLine::FormLine (FormControl *control)
	:FormControl(control)
{
	x1 = x;
	y1 = y;
	x2 = x + width;
	y2 = y + height;
	color = control->color;
}

FormLine::FormLine(int setx1, int sety1, int setx2, int sety2, int setwidth, int setformat, COLORREF setcolor)
{
	x1 = setx1;
	y1 = sety1;
	x2 = setx2;
	y2 = sety2;
	format = setformat;
	color = setcolor;
}

FormLine::~FormLine()
{

}

void FormLine::Draw(CDC *dc)
{	//CString temp;

	CPen pen(PS_SOLID, 1, color), *oldPen;
	oldPen = (CPen *)dc->SelectObject(&pen);
	dc->MoveTo(x1, y1);
	dc->LineTo(x2, y2);
	dc->SelectObject(oldPen);
	//temp.Format ("%i, %i; %i, %i",x1,y1,x2,y2);
}

bool FormLine::Collide (int x, int y)
{//does not yet support diagonals
	if (y1 == y2 && y1 == y)// && ((x >= x1 && x <= x2) || (x <= x1 && x >= x2)))
		return true;
	if (x1 == x2 && x1 == x)// && ((y >= y1 && y <= y2) || (y <= y1 && y >= y2)))
		return true;
	x++;
	if (y1 == y2 && y1 == y)// && ((x >= x1 && x <= x2) || (x <= x1 && x >= x2)))
		return true;
	if (x1 == x2 && x1 == x)// && ((y >= y1 && y <= y2) || (y <= y1 && y >= y2)))
		return true;
	x-=2;
	if (y1 == y2 && y1 == y)// && ((x >= x1 && x <= x2) || (x <= x1 && x >= x2)))
		return true;
	if (x1 == x2 && x1 == x)// && ((y >= y1 && y <= y2) || (y <= y1 && y >= y2)))
		return true;
	x++; y++;
	if (y1 == y2 && y1 == y)// && ((x >= x1 && x <= x2) || (x <= x1 && x >= x2)))
		return true;
	if (x1 == x2 && x1 == x)// && ((y >= y1 && y <= y2) || (y <= y1 && y >= y2)))
		return true;
	y-=2;
	if (y1 == y2 && y1 == y)// && ((x >= x1 && x <= x2) || (x <= x1 && x >= x2)))
		return true;
	if (x1 == x2 && x1 == x)// && ((y >= y1 && y <= y2) || (y <= y1 && y >= y2)))
		return true;
	return false;
}

RECT FormLine::GetRect(void)
{
	RECT rect;
	if (x1 > x2)
	{	rect.left = x1;
		rect.right = x2 + 1;
	}
	else
	{	rect.left = x2;
		rect.right = x1 + 1;
	}
	if (y1 > y2)
	{	rect.top = y1 - 1;
		rect.bottom = y2 + 1;
	}
	else
	{	rect.top = y2 - 1;
		rect.bottom = y1 + 1;
	}
	return rect;
}

void FormLine::PrintOut(CDC *pdc)
{
	//comment this in if you need to print lines, for debugging purposes

	/*
	CPen pen(PS_SOLID, 1, color);
	pdc->SelectObject(pen);

	int xp1 = (int)((double)x1 * PRINT_X_SCALE) + PRINT_X_OFFSET;
	int xp2 = (int)((double)x2 * PRINT_X_SCALE) + PRINT_X_OFFSET;
	int yp1 = PRINT_Y_OFFSET - (int)((double)y1 * PRINT_Y_SCALE);
	int yp2 = PRINT_Y_OFFSET - (int)((double)y2 * PRINT_Y_SCALE);

	pdc->MoveTo(xp1, yp1);
	pdc->LineTo(xp2, yp2);
	*/
}










