////////////////////////////////
// PEBBLE 2 GEO CONVERTER

#include "include.h"

#include "SIM_Pebble.h"

#include "SIM_PB2Geo.h"

const SIM_DopDescription * SIM_PB2Geo::getDopDescription()
{
	static PRM_Template	 theTemplates[] = {

		PRM_Template(PRM_TOGGLE_J,		1, &theActivateName, PRMoneDefaults),
		PRM_Template(PRM_STRING,	1, &theDataNameName, &theDataNameDef),
		PRM_Template(PRM_STRING,	1, &theGeoDataNameName, &theGeoDataNameDef),

		PRM_Template() };

	static SIM_DopDescription	 theDopDescription(true, "sim_pb2geo", "Pebble2Geo", "PB2Geo", classname(), theTemplates);
	return &theDopDescription;
};

SIM_Solver::SIM_Result SIM_PB2Geo::solveSingleObjectSubclass(SIM_Engine & engine, SIM_Object & object, SIM_ObjectArray & feedbacktoobjects, const SIM_Time & timestep, bool newobject)
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

	//UT_String GDN = dn;
	//GDN += "/";
	//GDN += gdn;

	SIM_GeometryCopy* geocp = SIM_DATA_GET(object, gdn, SIM_GeometryCopy);
	if (geocp == NULL)
	{
		geocp = SIM_DATA_CREATE(object, gdn, SIM_GeometryCopy, 0);
		if (geocp == NULL) return SIM_SOLVER_FAIL;
	};

	{
		SIM_GeometryAutoWriteLock lock(geocp);
		GU_Detail &gdp = lock.getGdp();

		for (const Patch* pb : pebble->m_P)
		{
			pb2geo(&gdp, *pb);
			int cnt = gdp.getNumPoints();
		};

		edges2geo(&gdp, pebble->m_edges);
	};

	return SIM_SOLVER_SUCCESS;
};