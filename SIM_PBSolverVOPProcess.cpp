////////////////////////////////////////////////////
// VOP PROCESS SOLVER

#include "include.h"

#include "SIM_Pebble.h"

#include "SIM_PBSolverVOPProcess.h"

const SIM_DopDescription * SIM_PBSolverVOPProcess::getDopDescription()
{
	static PRM_Template	 theTemplates[] = {

		PRM_Template(PRM_TOGGLE_J,			1, &theActivateName, PRMoneDefaults),

		PRM_Template(PRM_STRING, PRM_TYPE_DYNAMIC_PATH,PRM_Template::PRM_EXPORT_MAX, 1,
		&theVOPPathName, 0, 0, 0, 0,&PRM_SpareData::shopCVEX),

		PRM_Template(PRM_ALPHASTRING,	1, &theDataNameName, &theDataNameDef),

		PRM_Template() };

	static SIM_DopDescription	 theDopDescription(true, "sim_pb_vop", "Pebble VOP Process", "PBSolver_VOPProcess", classname(), theTemplates);
	return &theDopDescription;
};

SIM_Solver::SIM_Result SIM_PBSolverVOPProcess::solveSingleObjectSubclass(SIM_Engine & engine, SIM_Object & object, SIM_ObjectArray & feedbacktoobjects, const SIM_Time & timestep, bool newobject)
{
	//bool activate = (getActivate() != 0);
	//if (!activate) return SIM_SOLVER_SUCCESS;

	UT_String dn = "";
	getDataName(dn);

	SIM_Data* isPebble = SIM_DATA_GET(object, "IsPebble", SIM_Data);
	if (isPebble == NULL) return SIM_SOLVER_SUCCESS;

	m_pebble = SIM_DATA_GET(object, dn, SIM_Pebble);
	if (m_pebble == NULL) return SIM_SOLVER_FAIL;
	for (Patch* _pb : m_pebble->m_P) if (_pb == NULL) return SIM_SOLVER_FAIL;

	m_script;
	m_script = getVopPath();

	if (m_script == "") return SIM_SOLVER_SUCCESS;

	OP_Director* D = OPgetDirector();
	float time = D->getTime();

	OP_Node* CND = getCreatorNode();

	UT_String nm = CND->getName();

	SHOP_Node *shop = CND->findSHOPNode(m_script);
	if (shop == NULL) return SIM_SOLVER_FAIL;

	addOPInterest(shop);

	UT_String vexsrc;

	shop->buildVexCommand(vexsrc, shop->getSpareParmTemplates(), time); // timestep);
	shop->buildShaderString(m_script, time, 0); // timestep, 0);

	m_script += vexsrc;

	char *argv[4096];
	UT_String src = m_script;
	int argc = src.parse(argv, 4096);

	CVEX_Context  ctx;

	if (!ctx.load(argc, argv))
	{
		return SIM_SOLVER_SUCCESS;
	};

	// CH OUTS
	CVEX_ValueList& cvl = ctx.getOutputList();
	m_chouts.clear();

	for (int ol = 0; ol < cvl.entries(); ol++)
	{
		CVEX_Value* V = cvl[ol];

		UT_String name;
		name = V->getName();

		m_chouts.append(name);
	};

	// SOLVE
	m_primcnt = m_pebble->m_P.size();

	solve();

	m_pebble->pubHandleModification();

	return SIM_SOLVER_SUCCESS;
};

void SIM_PBSolverVOPProcess::solvePartial(const UT_JobInfo &info)
{
	int start, end;
	info.divideWork(m_primcnt, start, end);

	float pWeights[20], dsWeights[20], dtWeights[20];

	char *argv[4096];
	UT_String src = m_script;
	int argc = src.parse(argv, 4096);

	for (int id = start; id < end; id++)
	{
		if (UTgetInterrupt()->opInterrupt()) break;

		// FOR EACH PEBBLE
		Patch& pb = *m_pebble->m_P[id];

		int data_size = pb.dim[0] * pb.dim[1];

		CVEX_Context  ctx;

		// CHANNELS - IN
		for (int chi = 0; chi < pb.chs.size(); chi++)
		{
			UT_String name;
			name = pb.chs[chi];

			Page& p = pb.getPrimVar(name);

			// IN
			void* data = (UT_Vector3*)p;
			bool res = ctx.addInput(name, CVEX_TYPE_VECTOR3, data, data_size);
		};

		bool res = ctx.load(argc, argv);

		// CHANNELS - IN
		for (int i = 0; i < pb.chs.size(); i++)
		{
			CVEX_Value* inv = ctx.findInput(pb.chs[i], CVEX_TYPE_VECTOR3);
			if (inv == NULL) continue;

			Page& p = pb.getPrimVar(pb.chs[i].c_str());

			void* data = (UT_Vector3*)p.data();

			inv->setData(data, data_size);
		};


		pb.declarePages(m_chouts);

		// CHANNELS - OUT
		for (int i = 0; i < m_chouts.size(); i++)
		{
			CVEX_Value* outv = ctx.findOutput(m_chouts[i], CVEX_TYPE_VECTOR3);
			if (outv == NULL) continue;

			Page& p = pb.getPrimVar(m_chouts[i].c_str());

			void* data = (UT_Vector3*)p.data();

			outv->setData(data, data_size);
		};

		res = ctx.run(data_size, false);
	};
};
