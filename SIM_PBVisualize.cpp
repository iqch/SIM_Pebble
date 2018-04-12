////////////////////////////////////////////////////
// VISUALIZER - STRUCTURE

#include "include.h"

#include "SIM_Pebble.h"

#include "SIM_PBVisualize.h"

const SIM_DopDescription * SIM_PBVisualize::getDopDescription()
{
	static PRM_Template	 theTemplates[] = {
		PRM_Template()
	};

	static PRM_Template		 theGuideTemplates[] = {
		PRM_Template(PRM_TOGGLE,	1, &SIMshowguideName, PRMzeroDefaults),
		PRM_Template(PRM_RGB,		3, &SIMcolorName, PRMoneDefaults, 0, &PRMunitRange),
		PRM_Template(PRM_FLT_J, 1, &theUpperName, PRMzeroDefaults, 0, &theUpperRng),

		PRM_Template()
	};

	static SIM_DopDescription	 theDopDescription(true, "sim_pb_visualize", "Pebble Visualize", "VisualizationStruct", classname(), theTemplates);
	theDopDescription.setGuideTemplates(theGuideTemplates);

	return &theDopDescription;
};

void SIM_PBVisualize::buildGuideGeometrySubclass(const SIM_RootData & root, const SIM_Options & options, const GU_DetailHandle & gdh, UT_DMatrix4 * xform, const SIM_Time & t) const
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

	const vector<Patch*>& PEBBLE = m_pebble->m_P;

	float upper = getUpper(options);

	for (Patch* _pb : PEBBLE)
	{
		const Patch& pb = *_pb;
		const Page& P = pb.getPrimVar("P");
		const Page& N = pb.getPrimVar("N");
		//const Page& DPDU = pb.getPrimVar("dPdu");
		//const Page& DPDV = pb.getPrimVar("dPdv");
		const Page& CD = pb.getPrimVar("Cd");

		// FRAME
		{

			UT_Vector3 n = N.get(0, 0);
			//UT_Vector3 dpdu = DPDU.get(0, 0);
			//dpdu.normalize();
			//UT_Vector3 dpdv = DPDV.get(0, 0);
			//dpdv.normalize();

			UT_Vector3 p = P.get(0, 0);
			//UT_Vector3 pu = P.get(1, 0);
			//UT_Vector3 pv = P.get(0, 1);

			//float du = (pu - p).length();
			//float dv = (pu - p).length();

			float l = upper; // *min(du, dv);

			// DN
			{
				GEO_PrimPoly* PPN = GEO_PrimPoly::build(gdp, 2, true, false);

				GA_Offset pt = gdp->appendPointBlock(2);

				p += l*n;
				gdp->setPos3(pt, p);
				cdh.set(pt, UT_Vector3(0, 0, 1));
				PPN->setPointOffset(0, pt);
				pt++;

				p += l*n;
				gdp->setPos3(pt, p);
				cdh.set(pt, UT_Vector3(0, 0, 1));
				PPN->setPointOffset(1, pt);
			}

			// FRAME
			int vtcnt = pb.dim[0] * 2 + pb.dim[1] * 2;
			GA_Offset pt = gdp->appendPointBlock(vtcnt);

			// V0
			GEO_PrimPoly* PPV0 = GEO_PrimPoly::build(gdp, pb.dim[1], true, false);
			for (int i = 0; i < pb.dim[1]; i++)
			{
				gdp->setPos3(pt, P.get(i, 0) + N.get(i, 0)*l);
				PPV0->setPointOffset(i, pt);
				cdh.set(pt, UT_Vector3(1, 0, 0));
				pt++;
			};

			// V1
			GEO_PrimPoly* PPV1 = GEO_PrimPoly::build(gdp, pb.dim[1], true, false);
			for (int i = 0; i < pb.dim[1]; i++)
			{
				gdp->setPos3(pt, P.get(i, pb.dim[0] - 1) + N.get(i, pb.dim[0] - 1)*l);
				PPV1->setPointOffset(i, pt);
				cdh.set(pt, UT_Vector3(1, 0, 1));
				pt++;
			};

			// U0
			GEO_PrimPoly* PPU0 = GEO_PrimPoly::build(gdp, pb.dim[0], true, false);
			for (int i = 0; i < pb.dim[0]; i++)
			{
				gdp->setPos3(pt, P.get(0, i) + N.get(0, i)*l);
				PPU0->setPointOffset(i, pt);
				cdh.set(pt, UT_Vector3(0, 1, 0));
				pt++;
			};

			// U1
			GEO_PrimPoly* PPU1 = GEO_PrimPoly::build(gdp, pb.dim[0], true, false);
			for (int i = 0; i < pb.dim[0]; i++)
			{
				gdp->setPos3(pt, P.get(pb.dim[1] - 1, i) + N.get(pb.dim[1] - 1, i)*l);
				PPU1->setPointOffset(i, pt);
				cdh.set(pt, UT_Vector3(0, 1, 1));
				pt++;
			};
		};

		UT_Vector3 CLR[4] = {
			UT_Vector3(1,0,0),
			UT_Vector3(0,1,1),
			UT_Vector3(1,0,1),
			UT_Vector3(0,1,0)
		};

		// CENTRAL
		{
			GA_Offset C0 = gdp->appendPoint();
			gdp->setPos3(C0, pb.centroid());
			cdh.set(C0, UT_Vector3(0, 1, 0));

			for (int i = 0; i < 4; i++)
			{
				int idx = pb.f[i];

				if (idx == -1) continue;
				//if (pb.f[i] > pb.id) continue;

				const Patch& _pb = *PEBBLE[idx];

				int e = -1;
				for (int j = 0; j < 4; j++)
				{
					if (_pb.f[j] != pb.id) continue;
					e = j;
					break;
				};

				if (e == -1)
				{
					// ...INCONSISTENT!
					//throw("INCONSISTENT!");
					break;
				};

				GEO_PrimPoly* PP = GEO_PrimPoly::build(gdp, 2, true, false);

				GA_Offset C0 = gdp->appendPoint();
				gdp->setPos3(C0, pb.centroid());
				cdh.set(C0, CLR[e]);
				PP->setPointOffset(0, C0);

				GA_Offset C1 = gdp->appendPoint();
				gdp->setPos3(C1, (pb.centroid() + _pb.centroid()) / 2);
				cdh.set(C1, CLR[e]);
				PP->setPointOffset(1, C1);
			};
		};
	};
};


