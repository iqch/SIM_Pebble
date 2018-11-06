#include "include.h"

const char* ch_base_names[] = { "uvw", "P", "dPdu", "dPdv", "N", "Cd", "g", "G", NULL };

PRM_Name	 theActivateName(SIM_NAME_ACTIVATE, "Activate");
PRM_Name	 theDataNameName(SIM_NAME_DATAGEOMETRY, "DataName");
PRM_Default theDataNameDef(0, "Pebble");

PRM_Name	 theGeoDataNameName(SIM_NAME_GEODATA, "GeoDataName");
PRM_Default	theGeoDataNameDef(0, "PBGeometry");

PRM_Name	 theDetailName(SIM_NAME_DETAIL, "Detail");
PRM_Range theDetailRng(PRM_RANGE_RESTRICTED, 1, PRM_RANGE_FREE, 10);

PRM_Name	 theChannelsName(SIM_NAME_CHANNELS, "Channels");
PRM_Default theChannelsDef(0, "v");

PRM_Name	 theUpperName(SIM_NAME_UPPER, "Upper");
PRM_Range theUpperRng(PRM_RANGE_FREE, -0.5, PRM_RANGE_FREE, 0.5);



PRM_Name	 theTimestepName(SIM_NAME_TIMESTEP, "Timestep");
PRM_Default theTimestepDef(0, "1.0/$FPS");

PRM_Name	 theMinMagnitudeName(SIM_NAME_MINMAGNITUDE, "MinMagnitude");
PRM_Default theMinMagnitudeDef(1e-4, "");

PRM_Name	 theAmountName(SIM_NAME_AMOUNT, "Amount");
PRM_Name	 theSeedName(SIM_NAME_SEED, "Seed");

PRM_Name	 theMarkName(SIM_NAME_MARK, "Path Mark");
PRM_Default theMarkDef(-1, "");
PRM_Range theMarkRng(PRM_RANGE_RESTRICTED, -1, PRM_RANGE_FREE, 25);

PRM_Name	 theShowVGuideName(SIM_NAME_SHOWV, "V-Giuide");

PRM_Name	 thePatchName(SIM_NAME_PATCH, "Patch");
PRM_Range thePatchRng(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_FREE, 10);
PRM_Name	 theAllName(SIM_NAME_ALL, "All");

PRM_Name	 theVOPPathName(SIM_NAME_VOP, "VOP Path");

PRM_Name	 theRelAttribName(SIM_RELATTRIB_NAME, "Relationship attribute");
PRM_Default  theRelAttribDef(0, "rel");

PRM_Name	 theModeName(SIM_NAME_BOUNDARYMODE, "Mode");
PRM_Name	 theValuesName(SIM_NAME_BOUNDARYVAL, "Values");
PRM_Name	 theMaskName(SIM_NAME_BOUNDARYMASK, "Mask");
//PRM_Name	 theGroupName(SIM_BOUNDARYGROUP_NAME, "Group");
//PRM_Default theGroupDef(0, "*");

PRM_Name         modeNames[] =
{
	PRM_Name("const",      "GEO:Constant"),
	PRM_Name("comp",      "GEO:Components"),
	//PRM_Name("srcg",      "Gradient:Source Constant"),
	//PRM_Name("srcg",      "Gradient:Source Components"),
	//PRM_Name("srcc",      "Gradient:Collide Constant"),
	//PRM_Name("srcc",      "Gradient:Collide Component"),
	PRM_Name(0)
};

PRM_ChoiceList modeMenu(PRM_CHOICELIST_SINGLE, modeNames);

PRM_Name	 theDissipateName(SIM_NAME_DISSIPATE, "Dissipate");
PRM_Name	 theDiffuseName(SIM_NAME_DIFFUSE, "Diffuse");
PRM_Name	 theIterationsName(SIM_NAME_ITERATIONS, "Iterations");


PRM_Name	 theEdgeGroupName(SIM_NAME_EDGEGROUP, "Edge Group");
PRM_Default  theEdgeGroupDef(0, "*");

