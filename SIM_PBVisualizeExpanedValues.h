#pragma once

class SIM_PBVisualizeExpanedValues : public SIM_Data, public SIM_OptionsUser
{
	const SIM_Pebble* m_pebble;

public:
	GET_GUIDE_FUNC_B(SIM_NAME_SHOWGUIDE, ShowGuide, true);
	GET_GUIDE_FUNC_V3(SIM_NAME_COLOR, Color, (1, 1, 1));
	GET_GUIDE_FUNC_I(SIM_NAME_PATCH, Patch, 0);
	GET_GUIDE_FUNC_S(SIM_NAME_CHANNELS, Channels, "v");
	GET_GUIDE_FUNC_F(SIM_NAME_UPPER, Upper, 0.01);


protected:
	// Overrides to properly implement this class as a SIM_Data.
	explicit		 SIM_PBVisualizeExpanedValues(const SIM_DataFactory *factory)
		: SIM_Data(factory)
		, SIM_OptionsUser(this)
		, m_pebble(NULL) {};
	virtual		~SIM_PBVisualizeExpanedValues() {};
	virtual void	 initializeSubclass() { BaseClass::initializeSubclass(); m_pebble = NULL; };
	virtual bool	 getIsAlternateRepresentationSubclass() const
	{
		return true;
	};
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
	DECLARE_DATAFACTORY(SIM_PBVisualizeExpanedValues, SIM_Data, "PB Visualize EV", getDopDescription());
};

