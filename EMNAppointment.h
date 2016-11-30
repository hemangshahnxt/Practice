#pragma once

// (a.walling 2013-01-16 13:04) - PLID 54650 - Appointment linked with this EMN
class EMNAppointment {
public:
	long nID;

	EMNAppointment()
		: nID(-1)
	{}

	explicit EMNAppointment(long nID)
		: nID(nID)
	{}

	operator long() const
	{
		return nID;
	}

	friend bool operator==(const EMNAppointment& l, const EMNAppointment& r)
	{
		return l.nID == r.nID;
	}

	friend bool operator<(const EMNAppointment& l, const EMNAppointment& r)
	{
		return l.nID < r.nID;
	}

	CString GetAuditString() const; // <None> or ID
};