// Color.cpp: implementation of the CColor class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Color.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

static BYTE max1(BYTE a, BYTE b, BYTE c)
{
	if (a > b && a > c)
		return a;
	else if (b > c)
		return b;
	else return c;
}

static BYTE min1(BYTE a, BYTE b, BYTE c)
{
	if (a < b && a < c)
		return a;
	else if (b < c)
		return b;
	else return c;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CColor::CColor(DWORD dwColor /*=0*/)
{	
	m_dwColor = dwColor;
}

CColor::CColor(BYTE R, BYTE G, BYTE B)
{
	m_dwColor = R + (G << 8) + (B << 16);
}

//////////////////////////////////////////////////////////////////////
// Public Functions
//////////////////////////////////////////////////////////////////////

BYTE CColor::R()
{
	return (BYTE)(m_dwColor & 0x0000FF);
}

BYTE CColor::G()
{
	return (BYTE)((m_dwColor & 0x00FF00) >> 8);
}

BYTE CColor::B()
{
	return (BYTE)((m_dwColor & 0xFF0000) >> 16);
}

//taken from 3D Computer Graphics 2nd ed - Watt, p418
BYTE CColor::H()
{
	BYTE	r = R(),
			g = G(),
			b = B(),
			v = V(),
			diff = v - min1(r, g, b);

	double 	r_dist,
			g_dist,
			b_dist,
			s;

	if (!diff)
		return 0;//treat pure grey as red hue

	r_dist = v - r / diff,
	g_dist = v - g / diff,
	b_dist = v - b / diff;

	if (r == v)
		s = b_dist - g_dist;
	else if (g == v)
		s = 2 + r_dist - b_dist;
	else
		s = 4 + g_dist - r_dist;

	s *= 60;

	return BYTE(s / 360 * 255);
}

BYTE CColor::S()
{
	BYTE min = min1(R(), G(), B());
	BYTE v = V();

	if (!v)
		return 0;
	else return 256 * (v - min) / v;
}

BYTE CColor::V()
{
	return max1(R(), G(), B());
}

void CColor::SetR(BYTE R)
{
	m_dwColor &= 0xFFFF00;
	m_dwColor += R;
}

void CColor::SetG(BYTE G)
{
	m_dwColor &= 0xFF00FF;
	m_dwColor += G<<8;
}

void CColor::SetB(BYTE B)
{
	m_dwColor &= 0x00FFFF;
	m_dwColor += B<<16;
}

void CColor::SetH(BYTE H)
{
	//too hard for me, maybe later
}

void CColor::SetS(BYTE S)
{
//	s == 256 * (v - min) / v; todo later
}

void CColor::SetV(BYTE V)
{
	BYTE oldV = this->V();

	if (!oldV)
	{	
		SetR(V);
		SetG(V);
		SetB(V);
	}
	else
	{
		double mul = (double)V / (double)oldV;
		SetR((BYTE)((double)R() * mul));
		SetG((BYTE)((double)G() * mul));
		SetB((BYTE)((double)B() * mul));
	}
}