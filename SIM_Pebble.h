#pragma once

class SIM_Pebble : public SIM_Geometry
{
public:
	vector<Patch*> m_P;

	// ...EDGEMAP

	map<string, vector< UT_Vector2i> > m_edges;

	void pubHandleModification() { handleModification(); }

	//int cntPebble() const { return m_P.size(); };

	//const Pebble& get(unsigned int i) const { return *m_P[i]; };
	//Pebble& get(unsigned int i) { return *m_P[i]; };

protected:
	explicit SIM_Pebble(const SIM_DataFactory *factory) : SIM_Geometry(factory) {};
	virtual ~SIM_Pebble()
	{
		for (Patch* pb : m_P) if (pb != NULL) delete pb;
		m_P.clear();
	};

	virtual void	 initializeSubclass();
	virtual void	 makeEqualSubclass(const SIM_Data *source);

	virtual void	 saveIOSubclass(std::ostream &os, SIM_DataThreadedIO *io) const;
	virtual bool	 loadIOSubclass(UT_IStream &is, SIM_DataThreadedIO *io);

	virtual int64	 getMemorySizeSubclass() const;
	//virtual void	 optionChangedSubclass(const char *name);

	virtual void	 handleModificationSubclass(int code);

	// Override the getGeometrySubclass function to construct our geometric
	// representation in a just-in-time fashion.
	virtual GU_ConstDetailHandle getGeometrySubclass() const;

private:

	SIM_Pebble() : SIM_Geometry(NULL)
	{
		// SHOULD NOT BE!
		const char* MSG = "SHOULD NOT TO HAPPEN!";

		throw(MSG);
	};

	SIM_Pebble(const SIM_Pebble& scr) : SIM_Geometry(NULL)
	{
		// SHOULD NOT BE!
		const char* MSG = "SHOULD NOT TO HAPPEN!";

		throw(MSG);
	};

	virtual SIM_Pebble& operator=(const SIM_Pebble& scr)
	{
		// SHOULD NOT BE!
		const char* MSG = "SHOULD NOT TO HAPPEN!";

		throw(MSG);
	};

	mutable GU_DetailHandle m_detailHandle;

	static const SIM_DopDescription	*getDopDescription();

	DECLARE_STANDARD_GETCASTTOTYPE();
	DECLARE_DATAFACTORY(SIM_Pebble, SIM_Geometry, "Pebble Data", getDopDescription());
};
