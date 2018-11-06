#pragma once

class SIM_PBVisualizeAdvect : public SIM_Data, public SIM_OptionsUser
{
	const SIM_Pebble* m_pebble;

public:
	GET_GUIDE_FUNC_B(SIM_NAME_SHOWGUIDE, ShowGuide, true);

	GET_GUIDE_FUNC_F(SIM_NAME_TIMESTEP, Timestep, 0.1);

	GET_GUIDE_FUNC_F(SIM_NAME_MINMAGNITUDE, MinMagnitude, 1e-4);

	GET_GUIDE_FUNC_F(SIM_NAME_AMOUNT, Amount, 2);
	GET_GUIDE_FUNC_I(SIM_NAME_SEED, Seed, 0);
	GET_GUIDE_FUNC_I(SIM_NAME_MARK, Mark, -1);

	GET_GUIDE_FUNC_F(SIM_NAME_UPPER, Upper, 0.01);

	GET_GUIDE_FUNC_B(SIM_NAME_SHOWV, ShowVGuide, true);



	//GET_GUIDE_FUNC_V3(SIM_NAME_COLOR, Color, (1, 1, 1));

protected:
	// Overrides to properly implement this class as a SIM_Data.
	explicit		 SIM_PBVisualizeAdvect(const SIM_DataFactory *factory)
		: SIM_Data(factory)
		, SIM_OptionsUser(this)
		, m_pebble(NULL) {};
	virtual		~SIM_PBVisualizeAdvect() {};
	virtual void	 initializeSubclass() { BaseClass::initializeSubclass(); m_pebble = NULL; };
	virtual bool	 getIsAlternateRepresentationSubclass() const { return true; };
	virtual void	 initAlternateRepresentationSubclass(const SIM_Data &parent)
	{
		m_pebble = SIM_DATA_CASTCONST(&parent, SIM_Pebble);
	};
	virtual SIM_Guide	*createGuideObjectSubclass() const { return new SIM_GuideShared(this, false); };

	// MAIN
	virtual void	 buildGuideGeometrySubclass(const SIM_RootData &root,
		const SIM_Options &options,
		const GU_DetailHandle &gdh,
		UT_DMatrix4 *xform,
		const SIM_Time &t) const;

private:
	static const SIM_DopDescription	*getDopDescription();

	DECLARE_STANDARD_GETCASTTOTYPE();
	DECLARE_DATAFACTORY(SIM_PBVisualizeAdvect, SIM_Data, "PB Visualize Advect", getDopDescription());

	// THREADED
	THREADED_METHOD_CONST(SIM_PBVisualizeAdvect, true, build);

	void buildPartial(const UT_JobInfo &info) const;

	mutable uint m_S;
	mutable float m_amount;
	mutable float m_timestep;
	mutable float m_minmagnitude;
	//bool m_oneWay;
	mutable UT_Lock m_lock;

	mutable vector<Page*> m_Ps, m_Vs, m_Rels, m_dPdUs, m_dPdVs, m_Ns, m_Gs;
	mutable bool m_sg;

	mutable GU_Detail* m_gdp;
	mutable float m_upper;

	mutable GA_RWHandleV3 m_cdh;
};
