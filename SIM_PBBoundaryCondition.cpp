/////////////////////////////////////////////////////
// BOUNDARY CONDITION DATA : BASE

#include "include.h"

#include "SIM_PBBoundaryCondition.h"

const SIM_DopDescription *SIM_PBBoundaryCondition::getDopDescription()
{
	static PRM_Template	 theTemplates[] = { PRM_Template() };

	static SIM_DopDescription	 theDopDescription(true, "sim_pebble_bc", "Pebble Boundary Condition", "PebbleBC", classname(), theTemplates);
	return &theDopDescription;
};