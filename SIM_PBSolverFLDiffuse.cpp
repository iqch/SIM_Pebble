/////////////////////////////////////////////////////
// FLUID : DIFFUSE SOLVER

#include "include.h"

#include "SIM_Pebble.h"
#include "SIM_PBBoundaryCondition.h"

#include "SIM_PBSolverFLDiffuse.h"

const SIM_DopDescription * SIM_PBSolverFLDiffuse::getDopDescription()
{
	static PRM_Template	 theTemplates[] = {

		PRM_Template(PRM_TOGGLE_J,		1, &theActivateName, PRMoneDefaults),
		PRM_Template(PRM_FLT_J,			1, &theDiffuseName, PRMzeroDefaults),
		PRM_Template(PRM_INT_J,			1, &theIterationsName, PRM20Defaults),
		PRM_Template(PRM_STRING,		1, &theChannelsName, &theChannelsDef),
		PRM_Template(PRM_ALPHASTRING,	1, &theDataNameName, &theDataNameDef),

		PRM_Template() };

	static SIM_DopDescription	 theDopDescription(true, "sim_pb_fl_diffuse", "PebbleFL Diffuse", "PBSolver_FLDiffuse", classname(), theTemplates);
	return &theDopDescription;
};

SIM_Solver::SIM_Result SIM_PBSolverFLDiffuse::solveSingleObjectSubclass(SIM_Engine & engine, SIM_Object & object, SIM_ObjectArray & feedbacktoobjects, const SIM_Time & timestep, bool newobject)
{
	//bool activate = (getActivate() != 0);
	//if (!activate) return SIM_SOLVER_SUCCESS;

	UT_String dn = "";
	getDataName(dn);

	SIM_Data* isPebble = SIM_DATA_GET(object, "IsPebble", SIM_Data);
	if (isPebble == NULL) return SIM_SOLVER_SUCCESS;

	m_diffuse = getDiffuse();
	if (m_diffuse == 0) return SIM_SOLVER_SUCCESS;

	UT_String chanStr;
	chanStr = getChannels();

	m_channels.clear();
	chanStr.tokenize(m_channels, " ");

	m_pebble = SIM_DATA_GET(object, dn, SIM_Pebble);
	if (m_pebble == NULL) return SIM_SOLVER_FAIL;
	for (Patch* _pb : m_pebble->m_P) if (_pb == NULL) return SIM_SOLVER_FAIL;

	// BOUNDARIES
	m_bcs.clear();
	SIM_DataArray subd;
	this->filterSubData(subd, NULL, SIM_DataFilterByType("SIM_PBBoundaryCondition"), NULL, SIM_DataFilterNone());

	for (int i = 0; i < subd.entries(); i++)
	{
		SIM_Data* dt = subd[i];
		SIM_PBBoundaryCondition* bcd = dynamic_cast<SIM_PBBoundaryCondition*>(dt);
		if (bcd != NULL) m_bcs.push_back(bcd);
	};

	// PREPARE
	m_iterations = getIterations();

	// FOR EACH PEBBLE
	for (Patch* _pb : m_pebble->m_P)
	{
		if (UTgetInterrupt()->opInterrupt()) break;

		Patch& pb = *_pb;

		Page V0(pb.dim[0] + 2, pb.dim[0] + 2);
		Page V1(pb.dim[0] + 2, pb.dim[0] + 2);
		Page* VV[2] = { &V0, &V1 };

		float a = timestep*m_diffuse *pb.dim[0] * pb.dim[1];

		UT_StringArray _channels = pb.chs;

		for (int chi = 0; chi < m_channels.size(); chi++)
		{
			UT_String name;
			name = m_channels[chi];

			if (pb.chs.find(name) == -1) continue;

			UT_String proxyName = "_proxy_";
			proxyName += name;

			_channels.append(proxyName);
		};

		pb.declarePages(_channels);

		Page* _G = getExpandedPrimVar(m_pebble->m_P, pb, "G");
		Page& G = *_G;

		for (int chi = 0; chi < m_channels.size(); chi++)
		{
			UT_String name;
			name = m_channels[chi];

			if (pb.chs.find(name) == -1) continue;

			UT_String proxyName = "_proxy_";
			proxyName += name;

			Page* _v = getExpandedPrimVar(m_pebble->m_P, pb, m_channels[chi]);
			Page& v = *_v;

			// ITERATIONS
			for (int k = 0; k < m_iterations; k++)
			{
				int k_src = k % 2;
				int k_dst = k_src == 0 ? 1 : 0;

				Page* src = VV[k_src];
				Page* dst = VV[k_dst];

				// EACH SAMPLE
				for (int I = 0; I < pb.dim[0] + 2; I++)
				{
					for (int J = 0; J < pb.dim[1] + 2; J++)
					{
						// COMPUTE RELAXATION
						UT_Vector3 V(0, 0, 0);

						V += src->get(max(I - 1, 0), J)*G.get(max(I - 1, 0), J)[0];
						V += src->get(min(I + 1, pb.dim[0] + 1), J)*G.get(min(I + 1, pb.dim[0] + 1), J)[0];
						V += src->get(I, max(J - 1, 0))*G.get(I, max(J - 1, 0))[0];
						V += src->get(I, min(J + 1, pb.dim[1] + 1))*G.get(I, min(J + 1, pb.dim[1] + 1))[0];
						V *= a;

						V += UT_Vector3(v.get(I, J))*G.get(I, J)[0];

						V /= 1 + 4 * a;
						V /= G.get(I, J)[0];

						dst->get(I, J) = V;
					};
				};

				for (SIM_PBBoundaryCondition* BC : m_bcs)
				{  
					UT_String chan = m_channels[chi];
					BC->applyBoundaryConditions(*m_pebble, pb, chan, *dst);
				}
			};

			Page& proxy = pb.getPrimVar(proxyName);
			proxy.apply(*VV[(m_iterations + 1) % 2], 1, 1);

			delete _v;

			V0.zero();
			V1.zero();
		};

		delete _G;
	};

	// APPLY & CLEANUP
	for (int chi = 0; chi < m_channels.size(); chi++)
	{
		UT_String name;
		name = m_channels[chi];

		if ((*m_pebble->m_P[0]).chs.find(name) == -1) continue;
		applyProxy(m_pebble->m_P, name);
	};

	m_pebble->pubHandleModification();

	return SIM_SOLVER_SUCCESS;
};
