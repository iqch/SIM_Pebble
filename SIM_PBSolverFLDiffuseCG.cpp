/////////////////////////////////////////////////////
// FLUID : DIFFUSE SOLVER

#include "include.h"

#include "SIM_Pebble.h"
#include "SIM_PBBoundaryCondition.h"

#include "SIM_PBSolverFLDiffuseCG.h"

const SIM_DopDescription * SIM_PBSolverFLDiffuseCG::getDopDescription()
{
	static PRM_Template	 theTemplates[] = {

		PRM_Template(PRM_TOGGLE_J,		1, &theActivateName, PRMoneDefaults),
		PRM_Template(PRM_FLT_J,			1, &theDiffuseName, PRMzeroDefaults),
		PRM_Template(PRM_INT_J,			1, &theIterationsName, PRM20Defaults),
		PRM_Template(PRM_STRING,		1, &theChannelsName, &theChannelsDef),
		PRM_Template(PRM_ALPHASTRING,	1, &theDataNameName, &theDataNameDef),

		PRM_Template() };

	static SIM_DopDescription	 theDopDescription(true, "sim_pb_fl_diffuse2", "PebbleFL Diffuse v2", "PBSolver_FLDiffuseCG", classname(), theTemplates);
	return &theDopDescription;
};

SIM_Solver::SIM_Result SIM_PBSolverFLDiffuseCG::solveSingleObjectSubclass(SIM_Engine & engine, SIM_Object & object, SIM_ObjectArray & feedbacktoobjects, const SIM_Time & timestep, bool newobject)
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
	m_timestep = timestep;

	for (int chi = 0; chi < m_channels.size(); chi++)
	{
		UT_String name;
		name = m_channels[chi];

		UT_String proxyName = "_proxy_";
		proxyName += name;

		declareEntity(m_pebble->m_P, proxyName);
	};

	// SOLVE
	solve();

	// FOR EACH PEBBLE
	//for (Patch* _pb : m_pebble->m_P)
	//{

	//};

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

void SIM_PBSolverFLDiffuseCG::solvePartial(const UT_JobInfo & info)
{
	int start, end;
	info.divideWork(m_pebble->m_P.size(), start, end);

	for (int id = start; id < end; id++)
	{
		if (UTgetInterrupt()->opInterrupt()) break;

		Patch& pb = *m_pebble->m_P[id];

		float a = m_timestep*m_diffuse; // *pb.dim[0] * pb.dim[1];

		Page* _G = getExpandedPrimVar(m_pebble->m_P, pb, "G");
		Page& G = *_G;

		Page DEST(pb.dim[0] + 2, pb.dim[0] + 2);

		for (int chi = 0; chi < m_channels.size(); chi++)
		{
			UT_String name;
			name = m_channels[chi];

			Page* _v = getExpandedPrimVar(m_pebble->m_P, pb, m_channels[chi]);
			Page& v = *_v;

			// SOLVE

			int dim = (pb.dim[0] + 2)*(pb.dim[1] + 2);

			// SETUP MATRICES
			UT_SparseMatrixF M(dim,dim);
			M.zero();

			//UT_SparseMatrixELLF M(dim, 5*dim);

			int rdim = pb.dim[0] + 2;

			int index;
			//cout << "M : " << endl << "{" << endl;

			int rz = 0;
			for (index = 0; index < dim; index++)
			{
				//cout << " : " << index << endl;

				int I = index / rdim;
				int J = index % rdim;

				int NI = max(0,I-1);
				int NJ = max(0,J-1);

				int IP = min(I + 1, pb.dim[0]+1);
				int JP = min(J + 1, pb.dim[1]+1);

				//cout << "<00> [" << index << " : " << I*rdim + J << "] " << 1 - 4 * a << endl;

				M.addToElement(index, I*rdim + J, 1 - 4 * a);
				//M.appendRowElement(index, I*rdim + J, 1 - 4 * a, rz);

				if(index != NI*rdim + J)
				{
					//cout << "<-0> [" << index << " : " << NI*rdim + J << "] " << a << endl;
					M.addToElement(index, NI*rdim + J, a);
					//M.appendRowElement(index, NI*rdim + J, a, rz);
				};

				if(index != I*rdim + NJ)
				{
					//cout << "<0-> [" << index << " : " << I*rdim + NJ << "] " << a << endl;
					M.addToElement(index, I*rdim + NJ, a);
					//M.appendRowElement(index, I*rdim + NJ, a, rz);
				};

				if(index != IP*rdim + J)
				{
					//cout << "<+0> [" << index << " : " << IP*rdim + J << "] " << a << endl;
					M.addToElement(index, IP*rdim + J, a);
					//M.appendRowElement(index, IP*rdim + J, a, rz);
				};

				if(index != I*rdim + JP)
				{
					//cout << "<0+> [" << index << " : " << I*rdim + JP << "] " << a << endl;
					M.addToElement(index, I*rdim + JP, a);
					//M.appendRowElement(index, I*rdim + JP, a, rz);
				};
			};

			//cout << "}" << endl;

			M.shrinkToFit();
			M.compile();

			//M.printFull(cout);

			// SETUP FREE PART
			UT_VectorF BX(0, dim - 1); BX.zero();
			UT_VectorF BY(0, dim - 1); BY.zero();
			UT_VectorF BZ(0, dim - 1); BZ.zero();

			index = 0;
			for (int I = 0; I < pb.dim[0] + 2; I++)
			{
				for (int J = 0; J < pb.dim[1] + 2; J++)
				{
					UT_Vector3& VAL = v.get(I, J);

					BX(index) = VAL[0];
					BY(index) = VAL[1];
					BZ(index) = VAL[2];

					index++;
				};
			};

			//cout << "BX " << endl << BX << endl;
			//cout << "BY " << endl << BY << endl;
			//cout << "BZ " << endl << BZ << endl;

			// CD SOLVE
			UT_VectorF X(0, dim-1); X.zero();
			UT_VectorF Y(0, dim-1); Y.zero();
			UT_VectorF Z(0, dim-1); Z.zero();

			M.solveConjugateGradient(X, BX, NULL, NULL, NULL, 1e-4, m_iterations);
			M.solveConjugateGradient(Y, BY, NULL, NULL, NULL, 1e-4, m_iterations);
			M.solveConjugateGradient(Z, BZ, NULL, NULL, NULL, 1e-4, m_iterations);

			// APPLY
			index = 0;
			for (int I = 0; I < pb.dim[0]+2; I++)
			{
				for (int J = 0; J < pb.dim[1]+2; J++)
				{
					UT_Vector3 V(X(index), Y(index), Z(index));

					DEST.get(I, J) = V;

					index++;
				};
			};

			for (SIM_PBBoundaryCondition* BC : m_bcs)
			{
				UT_String chan = m_channels[chi];
				BC->applyBoundaryConditions(*m_pebble, pb, chan, DEST);
			};

			UT_String proxyName = "_proxy_";
			proxyName += name;
			Page& proxy = pb.getPrimVar(proxyName);
			proxy.apply(DEST, 1, 1);
		};
		delete _G;
	};
};