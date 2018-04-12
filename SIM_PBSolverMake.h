#pragma once

class SIM_PBSolverMake : public SIM_SingleSolver, public SIM_OptionsUser
{
public:
	// Access methods for our configuration data.
	//GETSET_DATA_FUNCS_I(SIM_NAME_ACTIVATE, Activate);
	GETSET_DATA_FUNCS_S(SIM_NAME_DATAGEOMETRY, DataName);
	GETSET_DATA_FUNCS_S(SIM_NAME_GEODATA, GeoDataName);
	GETSET_DATA_FUNCS_I(SIM_NAME_DETAIL, Detail);

protected:
	explicit	SIM_PBSolverMake(const SIM_DataFactory *factory) : BaseClass(factory), SIM_OptionsUser(this) {};
	virtual		~SIM_PBSolverMake() {};

	virtual SIM_Result	 solveSingleObjectSubclass(SIM_Engine &engine,
		SIM_Object &object,
		SIM_ObjectArray &feedbacktoobjects,
		const SIM_Time &timestep,
		bool newobject);

private:

	static const SIM_DopDescription	*getDopDescription();

	DECLARE_STANDARD_GETCASTTOTYPE();
	DECLARE_DATAFACTORY(SIM_PBSolverMake, SIM_SingleSolver, "PB Solver Make", getDopDescription());

	int m_primcnt;

	THREADED_METHOD1(SIM_PBSolverMake, m_primcnt > 15, solve, int, nsamples);

	void solvePartial(int nsamples, const UT_JobInfo &info);

	UT_StringArray m_baseNames;

	Far::PatchMap* m_patchmap;
	Far::PatchTable const *m_patchTable;

	//vector<UT_IntArray> m_edgemap;
	vector<vector<int32> > m_edgemap;

	Page* m_uvf;

	vector<VertexPosition> m_vertsP;
	vector<VertexColor> m_vertsCd;

	// EDGES
	UT_Array<const GA_EdgeGroup *> m_glist;
	//vector<UT_Vector2IArray> m_edges;
	//UT_StringArray m_edgeNames;


	//const GA_Primitive* m_prim;
	//const GU_PrimPolySoup* m_ps;

	vector<GA_Offset>  m_ptoff;
	//GA_OffsetList  m_ptoff;

	//const GU_PrimPoly* m_p;

	const GU_Detail* m_input;

	SIM_Pebble* m_pebble;
};
