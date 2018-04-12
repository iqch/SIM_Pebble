////////////////////////////////////////////////////
// VISUALIZER - STRUCTURE

#include "include.h"

#include "SIM_Pebble.h"

#include "SIM_PBVisualizeExpanedValues.h"

const SIM_DopDescription * SIM_PBVisualizeExpanedValues::getDopDescription()
{
	static PRM_Template	 theTemplates[] = {
		PRM_Template()
	};

	static PRM_Template		 theGuideTemplates[] = {
		PRM_Template(PRM_TOGGLE,	1, &SIMshowguideName, PRMzeroDefaults),
		PRM_Template(PRM_RGB,		3, &SIMcolorName, PRMoneDefaults, 0, &PRMunitRange),
		PRM_Template(PRM_INT_J,		1, &thePatchName, PRMzeroDefaults, NULL, &thePatchRng),
		PRM_Template(PRM_STRING,	1, &theChannelsName, &theChannelsDef),
		PRM_Template(PRM_FLT_J,		1, &theUpperName, PRMzeroDefaults, 0, &theUpperRng),

		PRM_Template()
	};

	static SIM_DopDescription	 theDopDescription(true, "sim_pb_visualize_ev", "Pebble Visualize Expanded Data", "VisualizeEV", classname(), theTemplates);
	theDopDescription.setGuideTemplates(theGuideTemplates);

	return &theDopDescription;
};

void SIM_PBVisualizeExpanedValues::buildGuideGeometrySubclass(const SIM_RootData & root, const SIM_Options & options, const GU_DetailHandle & gdh, UT_DMatrix4 * xform, const SIM_Time & t) const
{
	// Build our template geometry, if we are so asked.
	initAlternateRepresentation();

	if (!m_pebble) return;
	for (const Patch* _pb : m_pebble->m_P) if (_pb == NULL) return;

	if (gdh.isNull()) return;

	GU_DetailHandleAutoWriteLock gdl(gdh);
	GU_Detail *gdp = gdl.getGdp();

	UT_Vector3 color = getColor(options);

	GA_RWHandleV3 cdh(gdp->findPointAttribute("Cd"));
	if (!cdh.isValid())
	{
		cdh = GA_RWHandleV3(gdp->addFloatTuple(GA_ATTRIB_POINT, "Cd", 3, GA_Defaults(1.0)));
		cdh->setTypeInfo(GA_TYPE_COLOR);
	};

	//const vector<Patch*>& PEBBLE = m_pebble->m_P;

	float upper = getUpper(options);

	int patch = getPatch(options);

	UT_String chan;
	getChannels(chan, options);

	if (patch < 0) return;
	if (patch >= m_pebble->m_P.size()) return;

	const Patch& pb = *m_pebble->m_P[patch];

	if (pb.chs.find(chan) == -1) return;

	const Page& P = pb.getPrimVar("P");
	const Page& N = pb.getPrimVar("N");
	const Page& DPDU = pb.getPrimVar("dPdu");
	const Page& DPDV = pb.getPrimVar("dPdv");
	const Page& CD = pb.getPrimVar("Cd");

	const Page& V = pb.getPrimVar(chan);

	// FRAME
	for (int i = 0; i < pb.dim[0]; i++)
		for (int j = 0; j < pb.dim[1]; j++)
	{
		UT_Vector3 n = N.get(i, j);
		UT_Vector3 dpdu = DPDU.get(i, j);
		UT_Vector3 dpdv = DPDV.get(i, j);

		UT_Vector3 p = P.get(i, j);

		UT_Vector3 v = V.get(i, j);

		GEO_PrimPoly* PPN = GEO_PrimPoly::build(gdp, 2, true, false);
		GA_Offset pt = gdp->appendPointBlock(2);

		p += upper*n;
		gdp->setPos3(pt, p);
		cdh.set(pt, UT_Vector3(0, 0, 1));
		PPN->setPointOffset(0, pt);
		pt++;

		p += v[0]*dpdu+v[1]*dpdv;
		gdp->setPos3(pt, p);
		cdh.set(pt, UT_Vector3(0, 0.5, 1));
		PPN->setPointOffset(1, pt);


		//// FRAME
		//int vtcnt = pb.dim[0] * 2 + pb.dim[1] * 2;
		//GA_Offset pt = gdp->appendPointBlock(vtcnt);

		//// V0
		//GEO_PrimPoly* PPV0 = GEO_PrimPoly::build(gdp, pb.dim[1], true, false);
		//for (int i = 0; i < pb.dim[1]; i++)
		//{
		//	gdp->setPos3(pt, P.get(i, 0) + N.get(i, 0)*l);
		//	PPV0->setPointOffset(i, pt);
		//	cdh.set(pt, UT_Vector3(1, 0, 0));
		//	pt++;
		//};

	};

};


