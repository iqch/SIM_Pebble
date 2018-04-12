////////////////////////////////////////////////////
// VISUALIZER - ADVECTION PATHS

#include "include.h"

#include "SIM_Pebble.h"

#include "SIM_PBVisualizeAdvect.h"

const SIM_DopDescription * SIM_PBVisualizeAdvect::getDopDescription()
{
	static PRM_Template	 theTemplates[] = {
		PRM_Template()
	};

	static PRM_Template		 theGuideTemplates[] = {
		PRM_Template(PRM_TOGGLE,	1, &SIMshowguideName, PRMzeroDefaults),
		PRM_Template(PRM_FLT_J,	1, &theTimestepName, &theTimestepDef),
		PRM_Template(PRM_FLT_J,	1, &theAmountName, PRMoneDefaults, 0, &PRMunitRange),
		PRM_Template(PRM_INT,	1, &theSeedName, PRMzeroDefaults),
		PRM_Template(PRM_INT,	1, &theMarkName, &theMarkDef, 0, &theMarkRng),

		PRM_Template(PRM_FLT_J,	1, &theUpperName, PRMzeroDefaults),

		PRM_Template(PRM_TOGGLE,	1, &theShowVGuideName, PRMoneDefaults),

		//PRM_Template(PRM_RGB,		3, &SIMcolorName, PRMoneDefaults, 0, &PRMunitRange),

		PRM_Template()
	};

	static SIM_DopDescription	 theDopDescription(true, "sim_pb_visualize_advect", "Pebble Visualize Advection", "VisualizationAdv", classname(), theTemplates);
	theDopDescription.setGuideTemplates(theGuideTemplates);

	return &theDopDescription;
}

void SIM_PBVisualizeAdvect::buildGuideGeometrySubclass(const SIM_RootData & root, const SIM_Options & options, const GU_DetailHandle & gdh, UT_DMatrix4 * xform, const SIM_Time & t) const
{
	// Build our template geometry, if we are so asked.
	initAlternateRepresentation();

	if (!m_pebble) return;
	for (const Patch* _pb : m_pebble->m_P) if (_pb == NULL) return;

	if (gdh.isNull()) return;

	GU_DetailHandleAutoWriteLock gdl(gdh);
	m_gdp = gdl.getGdp();

	//UT_Vector3 color = getColor(options);

	m_cdh.bind(m_gdp->findPointAttribute("Cd"));
	if (!m_cdh.isValid())
	{
		m_cdh = GA_RWHandleV3(m_gdp->addFloatTuple(GA_ATTRIB_POINT, "Cd", 3, GA_Defaults(1.0)));
		m_cdh->setTypeInfo(GA_TYPE_COLOR);
	};

	uint seed = getSeed(options);
	m_S = seed;
	
	m_amount = getAmount(options);

	m_timestep = getTimestep(options);
	m_upper = getUpper(options);

	m_sg = getShowVGuide(options);

	int psz = m_pebble->m_P.size();

	m_Ps.resize(psz);
	m_Vs.resize(psz);
	//m_dPdUs.resize(psz);
	//m_dPdVs.resize(psz);
	//m_Ns.resize(psz);
	m_Rels.resize(psz);

	for (int i = 0; i < psz; i++)
	{
		m_Ps[i] = NULL; m_Vs[i] = NULL;
		//m_dPdUs[i] = NULL; m_dPdVs[i] = NULL;	m_Ns[i] = NULL; 
		m_Rels[i] = NULL;
	};

	int mark = getMark(options);
	bool oneWay = (mark != -1);

	int path_index = 0;

	//UT_StringArray pages;
	//pages.append("rel");

	for (const Patch* _pb : m_pebble->m_P)
	{
		if (_pb->chs.find("v") == -1) return;
		if (_pb->chs.find("rel") == -1) return;
		//_pb->declarePages(pages);
	};

	build();

	for (int i = 0; i < psz; i++)
	{
		if (m_Ps[i] != NULL) delete m_Ps[i];
		if (m_Vs[i] != NULL) delete m_Vs[i];
		//if (m_dPdUs[i] != NULL) delete m_dPdUs[i];
		//if (m_dPdVs[i] != NULL) delete m_dPdVs[i];
		//if (m_Ns[i] != NULL) delete m_Ns[i];
		if (m_Rels[i] != NULL) delete m_Rels[i];
	};
};

void SIM_PBVisualizeAdvect::buildPartial(const UT_JobInfo & info) const
{
	int start, end;
	info.divideWork(m_pebble->m_P.size(), start, end);

	for (int id = start; id < end; id++)
	{
		UT_FastRandom RA;
		RA.setSeed(m_S+id);

		const Patch& pb = *m_pebble->m_P[id];

		const Page& P = pb.getPrimVar("P");
		const Page& V = pb.getPrimVar("v");
		//const Page& DPDU = pb.getPrimVar("dPdu");
		//const Page& DPDV = pb.getPrimVar("dPdv");
		const Page& CD = pb.getPrimVar("Cd");
		const Page& N = pb.getPrimVar("N");

		const Page& UV = pb.getPrimVar("uvw");

		const Page& REL = pb.getPrimVar("rel");

		for (int ii = 0; ii < pb.dim[0]; ii++)
		{
			for (int jj = 0; jj < pb.dim[1]; jj++)
			{
				if (RA.frandom() >= m_amount) continue;

				UT_Vector3 rel = REL.get(ii, jj);

				if (rel[1] > 0) continue;

				float length = V.get(ii, jj).length() * m_timestep;

				if (length == 0) continue;

				float L = length;

				if (isnan(L)) continue;
				if (!isfinite(L)) continue;

				//if (m_oneWay)
				//{
				//	if (mark != path_index) continue;
				//	path_index++;
				//};

				// CALCULATE

				UT_Vector3 uv = UV.get(ii, jj);

				UT_Vector3Array path;
				UT_Vector3Array colors;

				path.append(uv);
				colors.append(UT_Vector3(1, 1, 0));

				traceStat stat;

				bool res = trace(m_pebble->m_P, uv, length, path, colors, m_Ps, m_Vs, /*m_dPdUs, m_dPdVs, m_Ns,*/ m_Rels, m_lock, stat);

				if (path.size() < 2)
				{
					continue;
				};

				UT_AutoJobInfoLock a(info);

				// BUILD V GUIDE
				if (m_sg)
				{
					GEO_PrimPoly* PV = GEO_PrimPoly::build(m_gdp, 2, true, false);

					UT_Vector3 vn = V.get(ii, jj);
					UT_Vector3 n = N.get(ii, jj); n.normalize();

					vn -= n*vn.dot(n);

					vn.normalize();

					UT_Vector3 pp = P.get(ii, jj) + m_upper*n;

					GA_Offset poff0 = m_gdp->appendPoint();
					m_gdp->setPos3(poff0, pp);
					m_cdh.set(poff0, UT_Vector3(0, 1, 0));
					PV->setPointOffset(0, poff0);

					GA_Offset poff1 = m_gdp->appendPoint();
					m_gdp->setPos3(poff1, pp - vn*L);
					m_cdh.set(poff1, UT_Vector3(0, 1, 0));
					PV->setPointOffset(1, poff1);
				}

				// BUILD PATH
				GEO_PrimPoly* PP = GEO_PrimPoly::build(m_gdp, path.size(), true, false);

				for (int k = 0; k < path.size(); k++)
				{
					GA_Offset poff0 = m_gdp->appendPoint();

					UT_Vector3 C = path[k];

					const Page& pp = *m_Ps[C[2]];
					// .getPrimVar("P");
					//const Page& pp = m_pb->m_P[C[2]].getPrimVar("P");

					const Page& np = (*m_pebble->m_P[C[2]]).getPrimVar("N");

					UT_Vector3 nn = np.get(C);
					nn.normalize();

					m_gdp->setPos3(poff0, pp.get(C) + nn*m_upper);

					UT_Vector3 CLR = colors[k] * (1 - (float(k)) / float(path.size() + 1));
					// CD.get(ii, jj) * float(k) / float(k - 1); // UT_Vector3(1, 0.85, 0.15)* float(k) / float(k - 1);  
					//UV.get(ii, jj); // UT_Vector3(1, 0, 1); // CD.get(i,j) * float(k)/float(k-1)
					//CLR[2] = (ii + jj) % 2;

					if (!res && k == path.size() - 1) CLR = UT_Vector3(1, 0, 0);

					m_cdh.set(poff0, CLR);
					PP->setPointOffset(k, poff0);
				};
			};
		};
	};
};


