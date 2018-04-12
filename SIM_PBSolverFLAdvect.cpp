/////////////////////////////////////////////////////
// FLUID : ADVECT SOLVER

#include "include.h"

#include "SIM_Pebble.h"

#include "SIM_PBSolverFLAdvect.h"

const SIM_DopDescription * SIM_PBSolverFLAdvect::getDopDescription()
{

	static PRM_Template	 theTemplates[] = {

		PRM_Template(PRM_TOGGLE_J,			1, &theActivateName, PRMoneDefaults),
		//PRM_Template(PRM_FLT_J,			1, &theDissipateName, PRMoneDefaults), // , 0, &PRMunitRange),
		PRM_Template(PRM_STRING,		1, &theChannelsName, &theChannelsDef),
		PRM_Template(PRM_ALPHASTRING,	1, &theDataNameName, &theDataNameDef),

		PRM_Template() };

	static SIM_DopDescription	 theDopDescription(true, "sim_pb_fl_advect", "PebbleFL Advect", "PBSolver_FLAdvect", classname(), theTemplates);
	return &theDopDescription;
}

// +++-
SIM_Solver::SIM_Result SIM_PBSolverFLAdvect::solveSingleObjectSubclass(SIM_Engine & engine, SIM_Object & object, SIM_ObjectArray & feedbacktoobjects, const SIM_Time & timestep, bool newobject)
{
	//bool activate = (getActivate() != 0);
	//if (!activate) return SIM_SOLVER_SUCCESS;

	UT_String dn = "";
	getDataName(dn);

	SIM_Data* isPebble = SIM_DATA_GET(object, "IsPebble", SIM_Data);
	if (isPebble == NULL) return SIM_SOLVER_SUCCESS;


	m_timestep = timestep;
	//m_dissipation = getDissipate();

	UT_String chanStr;
	chanStr = getChannels();

	m_channels.clear();
	chanStr.tokenize(m_channels, " ");

	m_pebble = SIM_DATA_GET(object, dn, SIM_Pebble);
	if (m_pebble == NULL) return SIM_SOLVER_FAIL;
	for (Patch* _pb : m_pebble->m_P) if (_pb == NULL) return SIM_SOLVER_FAIL;
	
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
		//m_dPdUs[i] = NULL; m_dPdVs[i] = NULL; m_Ns[i] = NULL;
		m_Rels[i] = NULL; 
	};

	// PROXY PAGES
	m_sources.resize(psz);
	m_proxies.resize(psz);

	UT_StringArray _channels;

	for (Patch* _pb : m_pebble->m_P)
	{
		Patch& pb = *_pb;
		//UT_StringArray _channels = pb.chs;
		for (int chi = 0; chi < m_channels.size(); chi++)
		{
			UT_String name;
			name = m_channels[chi];
			if (pb.chs.find(name) == -1) continue;
			_channels.append(name);
		};
		break;
	};

	UT_StringArray reqCh;
	reqCh.append("v");
	reqCh.append("rel");

	for (Patch* _pb : m_pebble->m_P)
	{
		Patch& pb = *_pb;
		pb.declarePages(reqCh);
		pb.declarePages(_channels);
	};

	for (int chi = 0; chi < psz; chi++)
	{
		m_sources[chi].resize(m_channels.size());
		m_proxies[chi].resize(m_channels.size());
	};

	for (Patch* _pb : m_pebble->m_P)
	{
		Patch& pb = *_pb;
		for (int chi = 0; chi < m_channels.size(); chi++)
		{
			UT_String name;
			name = m_channels[chi];
			const Page& var = pb.getPrimVar(name);
			if (var.m_projected)
			{
				m_sources[pb.id][chi] = getExpandedPrimVarProjected(m_pebble->m_P, pb, name);
				//m_proxies[pb.id][chi] = getExpandedPrimVarProjected(m_pebble->m_P, pb, name);
			}
			else
			{
				m_sources[pb.id][chi] = getExpandedPrimVar(m_pebble->m_P, pb, name);
			};
			m_proxies[pb.id][chi] = getExpandedPrimVar(m_pebble->m_P, pb, name);
		};
	};

	// FOR EACH PEBBLE
	solve();

	// APPLY & CLEAN
	for (int i = 0; i < psz; i++)
	{
		if (m_Ps[i] != NULL) delete m_Ps[i];
		if (m_Vs[i] != NULL) delete m_Vs[i];
		//if (m_dPdUs[i] != NULL) delete m_dPdUs[i];
		//if (m_dPdVs[i] != NULL) delete m_dPdVs[i];
		//if (m_Ns[i] != NULL) delete m_Ns[i];
		if (m_Rels[i] != NULL) delete m_Rels[i];

		Patch& pb = *m_pebble->m_P[i];

		for (int chi = 0; chi < m_channels.size(); chi++)
		{
			UT_String name;
			name = m_channels[chi];

			if (m_proxies[i][chi] == NULL) continue;

			pb.getPrimVar(name).apply(*m_proxies[i][chi], 1, 1);

			delete m_sources[i][chi];
			delete m_proxies[i][chi];
		};
	};

	m_pebble->pubHandleModification();

	return SIM_SOLVER_SUCCESS;
};

void SIM_PBSolverFLAdvect::solvePartial(const UT_JobInfo & info)
{
	int start, end;
	info.divideWork(m_pebble->m_P.size(), start, end);

	for (int id = start; id < end; id++)
	{
		if (UTgetInterrupt()->opInterrupt()) break;

		Patch& pb = *m_pebble->m_P[id];

		// MAIN PAGES
		const Page& P = pb.getPrimVar("P");
		const Page& V = pb.getPrimVar("v");
		const Page& REL = pb.getPrimVar("rel");

		const Page& DPDU = pb.getPrimVar("dPdu");
		const Page& DPDV = pb.getPrimVar("dPdv");
		const Page& N = pb.getPrimVar("N");
		
		const Page& UV = pb.getPrimVar("uvw");

		// EACH SAMPLE
		for (int I = 0; I < pb.dim[0]; I++)
		{
			for (int J = 0; J < pb.dim[1]; J++)
			{
				// TRACE
				UT_Vector3 rel = REL.get(I, J);

				if (rel[1] > 0) continue;

				float length = V.get(I, J).length() * m_timestep; // *m_dissipation;

				if (length == 0) continue;

				float L = length;

				if (isnan(L)) continue;
				if (!isfinite(L)) continue;

				// CALCULATE
				UT_Vector3 uv = UV.get(I, J);

				UT_Vector3Array path;
				UT_Vector3Array colors;

				path.append(uv);
				colors.append(UT_Vector3(1, 1, 0));

				traceStat stat;

				trace(m_pebble->m_P, uv, length, path, colors, m_Ps, m_Vs, /*m_dPdUs, m_dPdVs, m_Ns, */m_Rels, m_lock, stat);

				UT_Vector3 C = path.last();

				//Patch& PB = *m_pebble->m_P[C[2]];

				// EACH PAGE
				for (int chi = 0; chi < m_channels.size(); chi++)
				{
					Page& PP = *m_sources[C[2]][chi];
					UT_Vector3 v = PP.get(C);
					if (PP.m_projected)
					{
						Patch& PB = *m_pebble->m_P[C[2]];

						const Page& DPDU = PB.getPrimVar("dPdu");
						const Page& DPDV = PB.getPrimVar("dPdv");
						const Page& N = PB.getPrimVar("N");

						v = v[0] * DPDU.get(C) + v[1] * DPDV.get(C) + v[2] * N.get(C);
					};

					m_proxies[pb.id][chi]->get(I + 1, J + 1) = v;
				};
			};
		};
	};
};

