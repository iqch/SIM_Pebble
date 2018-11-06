//////////////////////////////////////////////////////
// SOLVER PROJECT RELATIONSHIP

#include "include.h"

#include "SIM_Pebble.h"

#include "SIM_PBSolverRelationships.h"

const SIM_DopDescription * SIM_PBSolverRelationships::getDopDescription()
{
	static PRM_Template	 theTemplates[] = {

		PRM_Template(PRM_TOGGLE_J,			1, &theActivateName, PRMoneDefaults),
		PRM_Template(PRM_ALPHASTRING,	1, &theRelAttribName, &theRelAttribDef),
		PRM_Template(PRM_ALPHASTRING,	1, &theDataNameName, &theDataNameDef),

		PRM_Template() };

	static SIM_DopDescription	 theDopDescription(true, "sim_pb_relapply", "Pebble Apply Relationships", "PBSolver_RelApply", classname(), theTemplates);
	return &theDopDescription;
}

SIM_Solver::SIM_Result SIM_PBSolverRelationships::solveSingleObjectSubclass(SIM_Engine & engine, SIM_Object & object, SIM_ObjectArray & feedbacktoobjects, const SIM_Time & timestep, bool newobject)
{
	//bool activate = (getActivate() != 0);
	//if (!activate) return SIM_SOLVER_SUCCESS;

	UT_String dn = "";
	getDataName(dn);

	SIM_Data* isPebble = SIM_DATA_GET(object, "IsPebble", SIM_Data);
	if (isPebble == NULL) return SIM_SOLVER_SUCCESS;

	m_pebble = SIM_DATA_GET(object, dn, SIM_Pebble);
	if (m_pebble == NULL) return SIM_SOLVER_FAIL;
	for (Patch* _pb : m_pebble->m_P) if (_pb == NULL) return SIM_SOLVER_FAIL;

	//UT_String relAttr = "";
	m_relAttr = getRelAttrib();

	if (m_relAttr == "")
	{
		return SIM_SOLVER_FAIL;
	};

	// RELATTR
	for (Patch* pb : m_pebble->m_P) pb->declarePage(m_relAttr);

	// RELATIONSHIPS

	// COLLIDERS
	m_colliders.clear();
	m_sources.clear();

	{
		const SIM_RelationshipArray	&ref = object.getRelationships(true);

		for (int i = 0; i < ref.size(); i++)
		{
			SIM_Relationship* rel = ref(i);

			SIM_RelationshipSink* cr = dynamic_cast<SIM_RelationshipSink*>(rel->getRelationshipTypeData());
			if (cr != NULL)
			{
				int afnum = rel->getAffGroupEntries();
				for (int j = 0; j < afnum; j++)
				{
					const SIM_Object	*affector = rel->getAffGroupObject(j);
					const SIM_Geometry	*geometry = affector->getGeometry();

					if (!geometry) continue;

					const GU_ConstDetailHandle &gdh = geometry->getGeometry();

					if (gdh.isNull()) continue;

					GU_DetailHandleAutoReadLock gdl(gdh);
					const GU_Detail *gdp = gdl.getGdp();

					const GA_PrimitiveGroup *zgroup = 0;

					GU_RayIntersect *isect = new GU_RayIntersect(gdp, zgroup, 0, 0, 1, 0, true);

					m_colliders.push_back(isect);
				};
				continue;
			};

			SIM_RelationshipSource* sr = dynamic_cast<SIM_RelationshipSource*>(rel->getRelationshipTypeData());
			if (sr != NULL)
			{
				int afnum = rel->getAffGroupEntries();
				for (int j = 0; j < afnum; j++)
				{
					const SIM_Object	*affector = rel->getAffGroupObject(j);
					const SIM_Geometry	*geometry = affector->getGeometry();

					if (!geometry) continue;

					const GU_ConstDetailHandle &gdh = geometry->getGeometry();

					if (gdh.isNull()) continue;

					GU_DetailHandleAutoReadLock gdl(gdh);
					const GU_Detail *gdp = gdl.getGdp();

					const GA_PrimitiveGroup *zgroup = 0;

					GU_RayIntersect *isect = new GU_RayIntersect(gdp, zgroup, 0, 0, 1, 0, true);

					m_sources.push_back(isect);
				};
				continue;
			};

		};
	}

	if (m_sources.size() == 0 && m_colliders.size() == 0) return SIM_SOLVER_SUCCESS;

	solve();

	// CLEANUP
	for (GU_RayIntersect *isect : m_sources) delete isect;
	for (GU_RayIntersect *isect : m_colliders) delete isect;
	m_sources.clear();
	m_colliders.clear();

	m_pebble->pubHandleModification();

	return SIM_SOLVER_SUCCESS;
};

void SIM_PBSolverRelationships::solvePartial(const UT_JobInfo & info)
{
	int start, end;
	info.divideWork(m_pebble->m_P.size(), start, end);

	for (int id = start; id < end; id++)
	{
		if (UTgetInterrupt()->opInterrupt()) break;

		Patch& pb = *m_pebble->m_P[id];

		UT_StringArray channels = pb.chs;
		channels.append(m_relAttr);

		pb.declarePages(channels);

		Page& PP = pb.getPrimVar("P");
		Page& Rel = pb.getPrimVar(m_relAttr);

		// EACH SAMPLE
		for (int i = 0; i < pb.dim[0]; i++)
		{
			for (int j = 0; j < pb.dim[1]; j++)
			{
				UT_Vector3& rel = Rel.get(i, j);

				rel[0] = 0; rel[1] = 0;

				UT_Vector3 orig(PP.get(i, j));

				for (GU_RayIntersect *isect : m_sources)
				{
					if (isect->isInside(orig, 0))
					{
						rel[0] = 1;
						break;
					};
				};

				for (GU_RayIntersect *isect : m_colliders)
				{
					if (isect->isInside(orig, 0))
					{
						rel[1] = 1;
						break;
					};
				};

			};
		};
	};
};

