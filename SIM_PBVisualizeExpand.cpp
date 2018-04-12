/////////////////////////////////////////////////////
// VISUALIZER - PATCH EXPANDED PAGE BOUNDS

#include "include.h"

#include "SIM_Pebble.h"

#include "SIM_PBVisualizeExpand.h"

const SIM_DopDescription * SIM_PBVisualizeExpand::getDopDescription()
{
	static PRM_Template	 theTemplates[] = { PRM_Template() };

	static PRM_Template		 theGuideTemplates[] = {
		PRM_Template(PRM_TOGGLE,	1, &SIMshowguideName, PRMzeroDefaults),
		PRM_Template(PRM_RGB,		3, &SIMcolorName, PRMoneDefaults, 0, &PRMunitRange),
		PRM_Template(PRM_INT_J, 1, &thePatchName, PRMzeroDefaults, NULL, &thePatchRng),
		PRM_Template(PRM_TOGGLE,	1, &theAllName, PRMoneDefaults),
		PRM_Template()
	};

	static SIM_DopDescription	 theDopDescription(true, "sim_pb_visualize_exp", "Pebble Visualize Expanded Page", "VisualizationExp", classname(), theTemplates);
	theDopDescription.setGuideTemplates(theGuideTemplates);

	return &theDopDescription;
};

void SIM_PBVisualizeExpand::buildGuideGeometrySubclass(const SIM_RootData & root, const SIM_Options & options, const GU_DetailHandle & gdh, UT_DMatrix4 * xform, const SIM_Time & t) const
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

	int _patch = getPatch(options);

	if (_patch < 0) _patch = 0;
	_patch %= m_pebble->m_P.size();

	bool all = getAll(options);

	

	for (const Patch* _pb : m_pebble->m_P)
	{
		const Patch& pb = *_pb;

		Page* _P = getExpandedPrimVar(m_pebble->m_P, pb, "P");
		const Page& P = *_P;

		UT_Vector3 CLR(0, 0, 0);

		if (pb.id != _patch && !all) continue;

		if (pb.id == _patch) CLR = getColor(options);

		// FRAME
		{
			// FRAME
			int vtcnt = P.dim[0] * 2 + P.dim[1] * 2;
			GA_Offset pt = gdp->appendPointBlock(vtcnt);

			// V0
			GEO_PrimPoly* PPV0 = GEO_PrimPoly::build(gdp, P.dim[1], true, false);
			for (int i = 0; i < P.dim[1]; i++)
			{
				gdp->setPos3(pt, P.get(i, 0));
				PPV0->setPointOffset(i, pt);
				cdh.set(pt, CLR);
				pt++;
			};

			// V1
			GEO_PrimPoly* PPV1 = GEO_PrimPoly::build(gdp, P.dim[1], true, false);
			for (int i = 0; i < P.dim[1]; i++)
			{
				gdp->setPos3(pt, P.get(i, -1));
				PPV1->setPointOffset(i, pt);
				cdh.set(pt, CLR);
				pt++;
			};

			// U0
			GEO_PrimPoly* PPU0 = GEO_PrimPoly::build(gdp, P.dim[0], true, false);
			for (int i = 0; i < P.dim[0]; i++)
			{
				gdp->setPos3(pt, P.get(0, i));
				PPU0->setPointOffset(i, pt);
				cdh.set(pt, CLR);
				pt++;
			};

			// U1
			GEO_PrimPoly* PPU1 = GEO_PrimPoly::build(gdp, P.dim[0], true, false);
			for (int i = 0; i < P.dim[0]; i++)
			{
				gdp->setPos3(pt, P.get(-1, i));
				PPU1->setPointOffset(i, pt);
				cdh.set(pt, CLR);
				pt++;
			};
		};

		delete _P;
	};
};