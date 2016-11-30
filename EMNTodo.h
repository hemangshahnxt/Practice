#pragma once

// (c.haag 2008-06-24 13:59) - PLID 17244 - This structure represents a newly created todo alarm
// spawned from a detail that has not been saved.
class EMNTodo
{
public:
	long nTodoID; // The ID of the todo alarm
	// (z.manning 2009-02-26 14:42) - PLID 33141 - Use the new class to keep track of all the source action info
	SourceActionInfo sai;

public:
	// (c.haag 2008-07-09 11:25) - PLID 30648 - These are todo-specific
	// fields used to restore deleted todos after unspawning them and then
	// cancelling out of the EMR (which effectively reverts the unspawn
	// operation)
	_variant_t vRemind;
	_variant_t vDone;
	_variant_t vDeadline;
	_variant_t vEnteredBy;
	_variant_t vPersonID;
	_variant_t vNotes;
	_variant_t vPriority;
	_variant_t vTask;
	_variant_t vLocationID;
	_variant_t vCategoryID;
	_variant_t vRegardingID;
	_variant_t vRegardingType;
	CArray<long,long> anAssignTo;
	// (c.haag 2008-07-10 09:56) - Need to store names, but only for display purposes for when the
	// user deals with the "outstanding" todos they spawned/unspawned before cancelling out of the EMR
	_variant_t vAssignToNames;
};