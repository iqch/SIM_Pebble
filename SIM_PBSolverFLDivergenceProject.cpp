/////////////////////////////////////////////////////
// ++-FLUID : DIVERGENCE PROJECT SOLVER

#include "include.h"

#include "SIM_Pebble.h"
#include "SIM_PBBoundaryCondition.h"

#include "SIM_PBSolverFLDivergenceProject.h"

const SIM_DopDescription * SIM_PBSolverFLDivergenceProject::getDopDescription()
{
	static PRM_Template	 theTemplates[] = {

		PRM_Template(PRM_TOGGLE_J,			1, &theActivateName, PRMoneDefaults),
		PRM_Template(PRM_INT_J,			1, &theIterationsName, PRM20Defaults),
		//PRM_Template(PRM_STRING,		1, &theChannelsName, &theChannelsDef),
		PRM_Template(PRM_ALPHASTRING,	1, &theDataNameName, &theDataNameDef),

		PRM_Template() };

	static SIM_DopDescription	 theDopDescription(true, "sim_pb_fl_divproj", "PebbleFL Divergence Projection", "PBSolver_FLDivproj", classname(), theTemplates);
	return &theDopDescription;
};

// ++-
SIM_Solver::SIM_Result SIM_PBSolverFLDivergenceProject::solveSingleObjectSubclass(SIM_Engine & engine, SIM_Object & object, SIM_ObjectArray & feedbacktoobjects, const SIM_Time & timestep, bool newobject)
{
	//bool activate = (getActivate() != 0);
	//if (!activate) return SIM_SOLVER_SUCCESS;

	UT_String dn = "";
	getDataName(dn);

	SIM_Data* isPebble = SIM_DATA_GET(object, "IsPebble", SIM_Data);
	if (isPebble == NULL) return SIM_SOLVER_SUCCESS;

	//UT_String chanStr;
	//chanStr = getChannels();

	//m_channels.clear();
	//chanStr.tokenize(m_channels, " ");

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

	declareEntity(m_pebble->m_P, "_proxy_v");

	solve();

	//UT_StringArray pxs;
	//pxs.append("_proxy_v");

	//vector<Page*> PVS(m_pebble->m_P.size());

	//// FOR EACH PEBBLE
	//for (Patch* _pb : m_pebble->m_P)
	//{
	//	if (UTgetInterrupt()->opInterrupt()) break;

	//	Patch& pb = *_pb;

	//	//delete _G;
	//};

	// APPLY & CLEANUP
	applyProxy(m_pebble->m_P, "v");

	m_pebble->pubHandleModification();

	return SIM_SOLVER_SUCCESS;
};

void SIM_PBSolverFLDivergenceProject::solvePartial(const UT_JobInfo & info)
{
	int start, end;
	info.divideWork(m_pebble->m_P.size(), start, end);

	for (int id = start; id < end; id++)
	{
		if (UTgetInterrupt()->opInterrupt()) break;

		Patch& pb = *m_pebble->m_P[id];

		//pb.declarePage("_proxy_v");

		float dx = 1.0 / (pb.dim[0] - 1);
		float dy = 1.0 / (pb.dim[1] - 1);

		Page* _du = getExpandedPrimVar(m_pebble->m_P, pb, "dPdu");
		Page& dPdu = *_du;

		Page* _dv = getExpandedPrimVar(m_pebble->m_P, pb, "dPdv");
		Page& dPdv = *_dv;

		//Page* _n = getExpandedPrimVar(PEBBLE, pb, "N");
		//Page& N = *_n;

		Page* _v = getExpandedPrimVar(m_pebble->m_P, pb, "v");
		Page& V = *_v;

		//Page* _G = getExpandedPrimVar(PEBBLE, pb, "G");
		//Page& G = *_G;

		// PREPARE DIV PAGE
		Page DIV(pb.dim[0] + 2, pb.dim[0] + 2);

		for (int I = 0; I < pb.dim[0] + 2; I++)
		{
			for (int J = 0; J < pb.dim[1] + 2; J++)
			{
				float div = 0;

				// X
				int I0 = max(I - 1, 0);
				int I1 = min(I + 1, pb.dim[0] + 1);
				{
					UT_Vector3 dPdu0 = dPdu.get(I0, J); dPdu0.normalize();
					UT_Vector3 dPdu1 = dPdu.get(I1, J); dPdu1.normalize();

					UT_Vector3 dPdv0 = dPdv.get(I0, J); dPdv0.makeOrthonormal(dPdu0); dPdv0.normalize();
					UT_Vector3 dPdv1 = dPdv.get(I1, J); dPdv1.makeOrthonormal(dPdu1); dPdv1.normalize();

					UT_Vector3 N0 = dPdu0; N0.cross(dPdv0); N0.normalize();
					UT_Vector3 N1 = dPdu1; N1.cross(dPdv1); N1.normalize();

					UT_Vector3 V0 = V.get(I0, J); V0 -= N0*V0.dot(N0);
					UT_Vector3 V1 = V.get(I1, J); V1 -= N1*V1.dot(N0);

					div += dx*(V1.dot(dPdu1) - V0.dot(dPdu0));
				};

				// Y
				int J0 = max(J - 1, 0);
				int J1 = min(J + 1, pb.dim[1] + 1);
				{
					UT_Vector3 dPdu0 = dPdu.get(I, J0); dPdu0.normalize();
					UT_Vector3 dPdu1 = dPdu.get(I, J1); dPdu1.normalize();

					UT_Vector3 dPdv0 = dPdv.get(I, J0); dPdv0.makeOrthonormal(dPdu0); dPdv0.normalize();
					UT_Vector3 dPdv1 = dPdv.get(I, J1); dPdv1.makeOrthonormal(dPdu1); dPdv1.normalize();

					UT_Vector3 N0 = dPdu0; N0.cross(dPdv0); N0.normalize();
					UT_Vector3 N1 = dPdu1; N1.cross(dPdv1); N1.normalize();

					UT_Vector3 V0 = V.get(I, J0); V0 -= N0*V0.dot(N0);
					UT_Vector3 V1 = V.get(I, J1); V1 -= N1*V1.dot(N0);

					div += dy*(V1.dot(dPdv1) - V0.dot(dPdv0));
				}

				div *= -0.5;

				DIV.get(I, J) = UT_Vector3(div, 0, 0);
			};
		};

		// RELAXING ITERATIONS
		for (int k = 0; k < m_iterations; k++)
		{
			// EACH SAMPLE - COMPUTE
			for (int I = 0; I < pb.dim[0] + 2; I++)
			{
				for (int J = 0; J < pb.dim[1] + 2; J++)
				{
					// COMPUTE RELAXATION
					UT_Vector3 D = DIV.get(I, J);

					int I0 = max(I - 1, 0);
					int I1 = min(I + 1, pb.dim[0] + 1);
					int J0 = max(J - 1, 0);
					int J1 = min(J + 1, pb.dim[1] + 1);

					float RES = D[0];
					RES += DIV.get(I0, J)[1] + DIV.get(I1, J)[1] + DIV.get(I, J0)[1] + DIV.get(I, J1)[1];
					RES /= 4;
					DIV.get(I, J)[2] = RES;
				};
			};

			// ...APPLY RELAXING BC!?

			// EACH SAMPLE - SWAP
			for (int I = 0; I < pb.dim[0] + 2; I++)
			{
				for (int J = 0; J < pb.dim[1] + 2; J++)
				{
					UT_Vector3 V = DIV.get(I, J);
					DIV.get(I, J) = UT_Vector3(V[0], V[2], V[1]);
				};
			};
		};

		// PROJECTION
		for (int I = 1; I < pb.dim[0] + 1; I++)
		{
			for (int J = 1; J < pb.dim[1] + 1; J++)
			{
				UT_Vector3 _dPdu = dPdu.get(I, J); _dPdu.normalize();
				UT_Vector3 _dPdv = dPdv.get(I, J); _dPdv.makeOrthonormal(_dPdu); _dPdv.normalize();

				UT_Vector3 N = _dPdu; N.cross(_dPdv); N.normalize();

				UT_Vector3 v = V.get(I, J); v -= N*v.dot(N);

				float vu = v.dot(_dPdu) - 0.5*(DIV.get(I + 1, J)[1] - DIV.get(I - 1, J)[1]) / dx;
				float vv = v.dot(_dPdv) - 0.5*(DIV.get(I, J + 1)[1] - DIV.get(I, J - 1)[1]) / dy;

				V.get(I, J) = vu*_dPdu + vv*_dPdv;
			};
		};

		// APPLY
		for (SIM_PBBoundaryCondition* BC : m_bcs) BC->applyBoundaryConditions(*m_pebble, pb, "v", V);

		Page& VV = pb.getPrimVar("_proxy_v");
		VV.apply(V, 1, 1);

		delete _du; delete _dv;
		delete _v;

	};
};