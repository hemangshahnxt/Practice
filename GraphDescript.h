//GraphDescript.h

#ifndef NEXTECH_PRACTICE_GRAPH_DESCRIPT_H
#define NEXTECH_PRACTICE_GRAPH_DESCRIPT_H

#pragma once




class GraphDescript
{
	public:
		typedef enum
		{
			GD_ADD,
			GD_AVG,
			GD_DIV,
			//(e.lally 2009-09-16) PLID 35559 - automatically multiplies the division by 100 for a percent.
			GD_PERCENT,
		} GD_OP;

		GraphDescript(const LPCSTR sql = NULL);
		void GraphDescript::CHECK_SIZE (unsigned short const index, CString callingfunc) const;
		GraphDescript::~GraphDescript();
		void Add (	const LPCSTR label, const LPCSTR field, unsigned long color, const LPCSTR field2 = "", long operation = GD_ADD,
					const double dblTotal = 0,	const double dblTotal2 = 0);
		unsigned short Size() const;
		unsigned int Color(unsigned int index) const;
		CString Label(unsigned int index) const;
		CString Field(unsigned int index) const;
		CString Field2(unsigned int index) const;
		double dblTotal(unsigned int index);
		double dblTotal2(unsigned int index);
		void AddToTotal(unsigned int index, double dblValToAdd);
		void AddToTotal2(unsigned int index, double dblValToAdd);
		GD_OP Op(unsigned int index) const;
	
		CString		 m_sql;

	protected:
		CStringArray m_labels;
		CStringArray m_fields;
		CStringArray m_fields2;
		CArray<unsigned int, unsigned int> m_ops;
		CArray<unsigned int, unsigned int> m_colors;
		CArray<double, double> m_dblTotals;
		CArray<double, double> m_dblTotals2;
};

#endif