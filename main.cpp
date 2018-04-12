#include "include.h"
#include "osd.h"

// DATATYPE
#include "SIM_Pebble.h"

// SETUP SOLVERS
#include "SIM_PBSolverMake.h"
#include "SIM_PBSolverVOPProcess.h"
#include "SIM_PBSolverProjectForces.h"
#include "SIM_PBSolverRelationships.h"

// FLUIDS SIMULATION SOLVERS
#include "SIM_PBSolverFLAdvect.h"
#include "SIM_PBSolverFLDiffuse.h"
#include "SIM_PBSolverFLDivergenceProject.h"

#include "SIM_PBSolverFLDiffuseCG.h"


#include "SIM_PBBoundaryCondition.h"
#include "SIM_PBApplyBoundaryCondition.h"

// CONVERTERS
#include "SIM_PB2Geo.h"
#include "SIM_Geo2PB.h"

// VISUALIZERS
#include "SIM_PBVisualize.h"
#include "SIM_PBVisualizeAdvect.h"
#include "SIM_PBVisualizeExpand.h"
#include "SIM_PBVisualizeEdges.h"

void initializeSIM(void *)
{
	// DATA
	IMPLEMENT_DATAFACTORY(SIM_Pebble);

	//// SETUP ROUTINES	
	IMPLEMENT_DATAFACTORY(SIM_PBSolverMake);
	IMPLEMENT_DATAFACTORY(SIM_PBSolverVOPProcess);
	IMPLEMENT_DATAFACTORY(SIM_PBSolverProjectForces);
	IMPLEMENT_DATAFACTORY(SIM_PBSolverRelationships);

	IMPLEMENT_DATAFACTORY(SIM_PB2Geo);
	IMPLEMENT_DATAFACTORY(SIM_Geo2PB);

	//// VISUALIZERS
	IMPLEMENT_DATAFACTORY(SIM_PBVisualize);
	IMPLEMENT_DATAFACTORY(SIM_PBVisualizeAdvect);
	IMPLEMENT_DATAFACTORY(SIM_PBVisualizeExpand);
	IMPLEMENT_DATAFACTORY(SIM_PBVisualizeEdges);

	//// FLUID SYSTEM
	IMPLEMENT_DATAFACTORY(SIM_PBSolverFLAdvect);
	IMPLEMENT_DATAFACTORY(SIM_PBSolverFLDiffuse);
	IMPLEMENT_DATAFACTORY(SIM_PBSolverFLDivergenceProject);

	IMPLEMENT_DATAFACTORY(SIM_PBSolverFLDiffuseCG);


	// FLUID SYSTEM BOUNDARY CONDITIONS
	//IMPLEMENT_DATAFACTORY(SIM_PBBoundaryCondition);
	IMPLEMENT_DATAFACTORY(SIM_PBApplyBoundaryCondition);
};

#include "SOP_GEO2PB.h"

//void newSopOperator(OP_OperatorTable *table) { table->addOperator(new OP_GEO2PBOperator()); };

#include <UT/UT_DSOVersion.h>
