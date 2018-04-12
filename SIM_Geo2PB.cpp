#include "include.h"

#include "SIM_Pebble.h"

#include "SIM_Geo2PB.h"

const SIM_DopDescription * SIM_Geo2PB::getDopDescription()
{
	static PRM_Template	 theTemplates[] = {

		PRM_Template(PRM_TOGGLE_J,		1, &theActivateName, PRMoneDefaults),
		PRM_Template(PRM_STRING,	1, &theGeoDataNameName, &theGeoDataNameDef),
		PRM_Template(PRM_STRING,	1, &theDataNameName, &theDataNameDef),

		PRM_Template() };

	static SIM_DopDescription	 theDopDescription(true, "sim_geo2pb", "Geo2Pebble", "Geo2PB", classname(), theTemplates);
	return &theDopDescription;
};

SIM_Solver::SIM_Result SIM_Geo2PB::solveSingleObjectSubclass(SIM_Engine & engine, SIM_Object & object, SIM_ObjectArray & feedbacktoobjects, const SIM_Time & timestep, bool newobject)
{
	UT_String dn = "";
	getDataName(dn);

	SIM_Data* isPebble = SIM_DATA_GET(object, "IsPebble", SIM_Data);
	if (isPebble == NULL) return SIM_SOLVER_SUCCESS;

	SIM_Pebble* pebble = SIM_DATA_GET(object, dn, SIM_Pebble);
	if (pebble == NULL)
	{
		return SIM_SOLVER_FAIL;
	};

	UT_String gdn = "";
	getGeoDataName(gdn);

	UT_String GDN = dn;
	GDN += "/";
	GDN += gdn;

	SIM_Geometry* geo = SIM_DATA_GET(object, GDN, SIM_Geometry);
	if (geo == NULL)
	{
		return SIM_SOLVER_FAIL;
	};

	GU_ConstDetailHandle cdh = geo->getGeometry();
	const GU_Detail* gdp = cdh.gdp();

	geo2pb(pebble->m_P, gdp);
	geo2edges(pebble->m_edges,gdp);

	return SIM_SOLVER_SUCCESS;
};