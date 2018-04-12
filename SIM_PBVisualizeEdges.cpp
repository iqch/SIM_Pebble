////////////////////////////////////////////////////
// VISUALIZER - EDGES

#include "include.h"

#include "SIM_Pebble.h"

#include "SIM_PBVisualizeEdges.h"


const SIM_DopDescription * SIM_PBVisualizeEdges::getDopDescription()
{
	static PRM_Template	 theTemplates[] = {
		PRM_Template()
	};

	static PRM_Template		 theGuideTemplates[] = {
		PRM_Template(PRM_TOGGLE,	1, &SIMshowguideName, PRMzeroDefaults),
		//PRM_Template(PRM_RGB,		3, &SIMcolorName, PRMoneDefaults, 0, &PRMunitRange),
		PRM_Template(PRM_FLT_J, 1, &theUpperName, PRMzeroDefaults), // , 0, &PRMunitRange),
		PRM_Template(PRM_STRING, 1, &theEdgeGroupName, &theEdgeGroupDef),

		PRM_Template()
	};

	static SIM_DopDescription	 theDopDescription(true, "sim_pb_visualizeedges", "Pebble Visualize Edges", "VisualizationGroups", classname(), theTemplates);
	theDopDescription.setGuideTemplates(theGuideTemplates);

	return &theDopDescription;
};

void SIM_PBVisualizeEdges::buildGuideGeometrySubclass(const SIM_RootData & root, const SIM_Options & options, const GU_DetailHandle & gdh, UT_DMatrix4 * xform, const SIM_Time & t) const
{
	// Build our template geometry, if we are so asked.
	initAlternateRepresentation();

	if (!m_pebble) return;
	for (const Patch* _pb : m_pebble->m_P) if (_pb == NULL) return;

	if (gdh.isNull()) return;

	GU_DetailHandleAutoWriteLock gdl(gdh);
	GU_Detail *gdp = gdl.getGdp();

	//UT_Vector3 color = getColor(options);

	GA_RWHandleV3 cdh(gdp->findPointAttribute("Cd"));
	if (!cdh.isValid())
	{
		cdh = GA_RWHandleV3(gdp->addFloatTuple(GA_ATTRIB_POINT, "Cd", 3, GA_Defaults(1.0)));
		cdh->setTypeInfo(GA_TYPE_COLOR);
	};

	//const vector<Patch*>& PEBBLE = m_pebble->m_P;

	float upper = getUpper(options);

	UT_String grpStr;
	getGroup(grpStr, options);

	UT_StringArray groups;
	grpStr.tokenize(groups, " ");

	bool all = (groups.find("*") != -1);

	//grpStr.matchPattern

	UT_Vector3 CLR[] = { UT_Vector3(1,0,0), UT_Vector3(0,1,1), UT_Vector3(1,0,1), UT_Vector3(0,1,0)};
	//	getColor(options);

	for(const pair<string, vector<UT_Vector2i>> val : m_pebble->m_edges)
	//for (Patch* _pb : PEBBLE)
	{
		if (!all)
		{
			if (groups.find(val.first.c_str()) == -1) continue;
		};

		const vector<UT_Vector2i>& E = val.second;

		for (const UT_Vector2i& e : E)
		{
			const Patch& pb = *m_pebble->m_P[e[0]];
			const Page& P = pb.getPrimVar("P");
			const Page& N = pb.getPrimVar("N");

			int _e = e[1];

			int vtcnt = pb.dim[_e % 2];

			int dx[] = {1,0,1,0};

			int x[] = { 0,pb.dim[0]-1,0,0 };
			int y[] = { 0, 0, pb.dim[1]-1, 0};

			int X = x[_e];
			int Y = y[_e];

			GA_Offset pt = gdp->appendPointBlock(vtcnt);

			GEO_PrimPoly* PPV0 = GEO_PrimPoly::build(gdp, vtcnt, true, false);
			for (int i = 0; i < vtcnt; i++)
			{
				gdp->setPos3(pt, P.get(X,Y) + N.get(X,Y)*upper);
				PPV0->setPointOffset(i, pt);
				cdh.set(pt, CLR[_e]);
				pt++;

				X += dx[_e];
				Y += 1 - dx[_e];
			};
		};
	};
};