#pragma once


class SIM_PBSolverFLAdvect : public SIM_SingleSolver, public SIM_OptionsUser
{
public:
	// Access methods for our configuration data.
	//GETSET_DATA_FUNCS_I(SIM_NAME_ACTIVATE, Activate);
	//GETSET_DATA_FUNCS_F(SIM_NAME_DISSIPATE, Dissipate);
	GETSET_DATA_FUNCS_S(SIM_NAME_CHANNELS, Channels);
	GETSET_DATA_FUNCS_S(SIM_NAME_DATAGEOMETRY, DataName);

protected:
	explicit	SIM_PBSolverFLAdvect(const SIM_DataFactory *factory) : BaseClass(factory), SIM_OptionsUser(this) {};
	virtual		~SIM_PBSolverFLAdvect() {};

	virtual SIM_Result	 solveSingleObjectSubclass(SIM_Engine &engine,
		SIM_Object &object,
		SIM_ObjectArray &feedbacktoobjects,
		const SIM_Time &timestep,
		bool newobject);

private:

	static const SIM_DopDescription	*getDopDescription();

	DECLARE_STANDARD_GETCASTTOTYPE();
	DECLARE_DATAFACTORY(SIM_PBSolverFLAdvect, SIM_SingleSolver, "PBFL Solver Advect", getDopDescription());

	// THREADING PART

	THREADED_METHOD(SIM_PBSolverFLAdvect, true, solve);

	void solvePartial(const UT_JobInfo &info);

	vector<Page*> m_Ps, m_Vs, m_Rels; // m_dPdUs, m_dPdVs, m_Ns,
	UT_Lock m_lock;

	vector<Proxy> m_sources;
	vector<Proxy> m_proxies;

	float m_timestep; // , m_dissipation;

	UT_StringArray m_channels;

	SIM_Pebble* m_pebble;
};