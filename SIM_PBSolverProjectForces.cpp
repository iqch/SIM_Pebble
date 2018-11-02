/////////////////////////////////////////////////////
// PROJECT FORCES SOLVER

#include "include.h"

#include "SIM_Pebble.h"

#include "SIM_PBSolverProjectForces.h"

const SIM_DopDescription * SIM_PBSolverProjectForces::getDopDescription()
{
	static PRM_Template	 theTemplates[] = {

		PRM_Template(PRM_TOGGLE_J,			1, &theActivateName, PRMoneDefaults),
		PRM_Template(PRM_ALPHASTRING,	1, &theDataNameName, &theDataNameDef),

		PRM_Template() };

	static SIM_DopDescription	 theDopDescription(true, "sim_pb_forces", "Pebble Project Forces", "PBSolver_ProjectForces", classname(), theTemplates);
	return &theDopDescription;
}

SIM_Solver::SIM_Result SIM_PBSolverProjectForces::solveSingleObjectSubclass(SIM_Engine & engine, SIM_Object & object, SIM_ObjectArray & feedbacktoobjects, const SIM_Time & timestep, bool newobject)
{
	// PREPARE
	//bool activate = (getActivate() != 0);
	//if (!activate) return SIM_SOLVER_SUCCESS;

	UT_String dn = "";
	getDataName(dn);

	SIM_Data* isPebble = SIM_DATA_GET(object, "IsPebble", SIM_Data);
	if (isPebble == NULL) return SIM_SOLVER_SUCCESS;

	m_pebble = SIM_DATA_GET(object, dn, SIM_Pebble);
	if (m_pebble == NULL) return SIM_SOLVER_FAIL;
	for (Patch* _pb : m_pebble->m_P) if (_pb == NULL) return SIM_SOLVER_FAIL;

	m_object = &object;

	m_time = timestep;

	// COLLECT FORCES
	SIM_ConstDataArray F;

	object.filterConstSubData(F, NULL, SIM_DataFilterByType("SIM_Force"), SIM_FORCES_DATANAME, SIM_DataFilterNone());

	m_forces.clear();

	for (int i = 0; i < F.size(); i++)
	{
		const SIM_Force	*f = SIM_DATA_CASTCONST(F[i], SIM_Force);
		if (f == NULL) continue;

		m_forces.push_back(f);
	};

	//solve();

	solve(0, m_pebble->m_P.size());

	m_pebble->pubHandleModification();

	return SIM_SOLVER_SUCCESS;
};

void SIM_PBSolverProjectForces::solvePartial(const UT_JobInfo & info)
{
	int start, end;
	info.divideWork(m_pebble->m_P.size(), start, end);

	solve(start, end);
};

void SIM_PBSolverProjectForces::solve(int start, int end)
{
	UT_StringArray channels;
	channels.append("v");
	channels.append("force");
	channels.append("mass");

	for (int id = start; id < end; id++)
	{
		if (UTgetInterrupt()->opInterrupt()) break;

		Patch& pb = *m_pebble->m_P[id];

		pb.declarePages(channels);

		Page& P = pb.getPrimVar("P");
		//Page& N = pb.getPrimVar("N");

		//Page& DPDU = pb.getPrimVar("dPdu");
		//Page& DPDV = pb.getPrimVar("dPdv");

		Page& MASS = pb.getPrimVar("mass");

		Page& V = pb.getPrimVar("v");
		V.m_projected = true;

		Page& force = pb.getPrimVar("force");
		force.m_projected = true;

		// EACH SAMPLE
		for (int i = 0; i < pb.dim[0]; i++)
		{
			for (int j = 0; j < pb.dim[1]; j++)
			{
				UT_Vector3 FF(0, 0, 0);

				UT_Vector3 nul(0, 0, 0), _F, _W;

				UT_Vector3 v = V.get(i, j);

				for (int fi = 0; fi < m_forces.size(); fi++)
				{
					const SIM_Force* f = m_forces[fi];

					f->getForce(*m_object, P.get(i, j), v, nul, 1.0, _F, _W);

					FF += _F;
				};

				//UT_Vector3 n = N.get(i, j);
				//UT_Vector3 dPdu = DPDU.get(i, j);
				//UT_Vector3 dPdv = DPDV.get(i, j);

				//float nproj = n.dot(FF);

				//UT_Vector3 F = FF - n*nproj;

				//UT_Vector3 E(0, 0, nproj);

				//float fval = F.length();
				//F.normalize();

				// * ITER 1
				//E[0] = FF.dot(dPdu)*fval;
				//E[1] = FF.dot(dPdv)*fval;

				// * ITER 2
				//UT_Vector3 diff = E[0] * dPdu + E[1] * dPdv - fval*F;

				//int dl = diff.length();

				//UT_Vector3 diff2 = E[0] * dPdu + E[1] * dPdv + E[2] * n - fval*F;

				//int dl2 = diff2.length();

				// * ITER 3
				//float detxy = dPdu[0] * dPdv[1] - dPdu[1] * dPdv[0];

				//float dU = F[0] * dPdv[1] - F[1] * dPdv[0];
				//float dV = dPdu[0] * F[1] - dPdu[1] * F[0];
				//E[0] += fval * dU / detxy; E[1] += fval * dV / detxy;

				// * ITER 4
				//E[0] = F.dot(dPdu);

				//dPdv -= dPdv.dot(dPdu)*dPdu;
				//dPdv.normalize();
				//
				//E[1] = F.dot(dPdv);

				UT_Vector3 F = pb.project(FF,i, j);

				F[2] = 0;

				// OK
				force.get(i, j) = F;

				float mass = MASS.get(i, j)[0];

				if (mass == 0) continue;

				v += F*m_time / mass;

				V.get(i, j) = v;
			};
		};
	};
};


