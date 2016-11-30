//GraphDescript.cpp

#include "stdafx.h"
#include "Graphdescript.h"

GraphDescript::GraphDescript (const LPCSTR sql /*= NULL*/)
{
	m_sql = sql;
}


GraphDescript::~GraphDescript() {
	m_labels.RemoveAll();
	m_fields.RemoveAll();
	m_fields2.RemoveAll();
	m_colors.RemoveAll();
	m_ops.RemoveAll();
	m_dblTotals.RemoveAll();
	m_dblTotals2.RemoveAll();
}

void GraphDescript::Add (	const LPCSTR label,	const LPCSTR field, unsigned long color, const LPCSTR field2 /*= ""*/, long operation /*= GD_ADD*/,
						 const double dblTotal /*= 0*/,	const double dblTotal2 /*= 0*/) {
	m_labels.Add(label);
	m_fields.Add(field);
	m_fields2.Add(field2);
	m_colors.Add(color);
	m_ops.Add(operation);
	m_dblTotals.Add(dblTotal);
	m_dblTotals2.Add(dblTotal2);
}

void GraphDescript::CHECK_SIZE (unsigned short index, CString callingfunc) const {

	if (index >= (unsigned short)m_colors.GetSize()) {
		ASSERT(FALSE);
		CString str;
		str.Format("%s Index: %li, Size: %li", callingfunc, index, m_colors.GetSize());
	}
}

unsigned short GraphDescript::Size() const {
		
	//aribitrary, they should all be the same
	return m_colors.GetSize();
}

unsigned int GraphDescript::Color(unsigned int index) const 
{
	CHECK_SIZE(index, "Color");
	return m_colors[index];
}

CString GraphDescript::Label(unsigned int index) const	{
	CHECK_SIZE(index, "Label");
	return m_labels[index];
}


CString GraphDescript::Field(unsigned int index) const	{
			
	CHECK_SIZE(index, "Field");
	return m_fields[index];
}

CString GraphDescript::Field2(unsigned int index) const	{
	
	CHECK_SIZE(index, "Field2");
	return m_fields2[index];
}

double GraphDescript::dblTotal(unsigned int index) {
			
	CHECK_SIZE(index, "dblTotal");
	return m_dblTotals[index];
}

double GraphDescript::dblTotal2(unsigned int index) {
		
	CHECK_SIZE(index, "dblTotal2");
	return m_dblTotals2[index];
}


void GraphDescript::AddToTotal(unsigned int index, double dblValToAdd) {
	CHECK_SIZE(index, "AddToTotal");
	m_dblTotals[index] += dblValToAdd;
}


void GraphDescript::AddToTotal2(unsigned int index, double dblValToAdd) {
	
	CHECK_SIZE(index, "AddToTotal2");
	m_dblTotals2[index] += dblValToAdd;
}

GraphDescript::GD_OP GraphDescript::Op(unsigned int index) const {
	
	CHECK_SIZE(index, "Op");
	return (GD_OP)m_ops[index];
}

