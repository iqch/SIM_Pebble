///////////////////////////////////////////////
// PEBBLE AS DATATYPE

#include "include.h"

#include "SIM_Pebble.h"

const SIM_DopDescription *SIM_Pebble::getDopDescription()
{
	static PRM_Template	 theTemplates[] = {  PRM_Template() };
	static SIM_DopDescription	 theDopDescription(true, "sim_pebble", "Pebble Struct", "Pebble", classname(), theTemplates);
	return &theDopDescription;
};

void SIM_Pebble::initializeSubclass()
{
	SIM_Geometry::initializeSubclass();
	m_detailHandle.clear();
	m_P.clear();
};

void SIM_Pebble::makeEqualSubclass(const SIM_Data * source)
{
	const SIM_Pebble	*src;

	SIM_Geometry::makeEqualSubclass(source);
	src = SIM_DATA_CASTCONST(source, SIM_Pebble);
	if(src)
	{
		// PATCHES
		for (Patch* pb : m_P) delete pb;

		m_P.resize(src->m_P.size());
		for (const Patch* _pb : src->m_P) m_P[_pb->id] = new Patch(*_pb);

		// EDGES
		m_edges.clear();
		for (const pair<string, vector< UT_Vector2i> >& k : src->m_edges)
		{
			m_edges[k.first] = k.second;
		};
	};
};

// ++-
void SIM_Pebble::saveIOSubclass(std::ostream & os, SIM_DataThreadedIO * io) const
{
	GU_Detail gdp;
	for (const Patch* p : m_P) pb2geo(&gdp, *p);

	edges2geo(&gdp, m_edges);

	BaseClass::saveIOSubclass(os, io);
	os << gdp;
};

// ++-
bool SIM_Pebble::loadIOSubclass(UT_IStream & is, SIM_DataThreadedIO * io)
{
	if (!BaseClass::loadIOSubclass(is, io)) return false;

	GU_Detail gdp;
	gdp.load(is);
	
	geo2pb(m_P, &gdp);

	geo2edges(m_edges, &gdp);

	return true;
};

void SIM_Pebble::handleModificationSubclass(int code)
{
	SIM_Geometry::handleModificationSubclass(code);
	m_detailHandle.clear();
};

// ++
GU_ConstDetailHandle SIM_Pebble::getGeometrySubclass() const
{
	if (m_detailHandle.isNull())
	{
		GU_Detail	*gdp = new GU_Detail();
		
		m_detailHandle.allocateAndSet(gdp);

		for (const Patch* _pb : m_P) if (_pb == NULL) return m_detailHandle;

		for (Patch* pb : m_P) pb2geo(gdp, *pb);

		edges2geo(gdp, m_edges);

		addClosure(gdp, m_P);
	};

	return m_detailHandle;
};

// ++
int64 SIM_Pebble::getMemorySizeSubclass() const
{
	// THIS
	int64 mem = sizeof(*this);
	
	// INTERNAL
	for (const Patch* p : m_P) mem += p->memsize();
	
	// GDP
	if (!m_detailHandle.isNull())
	{
		GU_DetailHandleAutoReadLock gdl(m_detailHandle);
		const GU_Detail *gdp = gdl.getGdp();

		mem += gdp->getMemoryUsage(true);
	}

	return mem;
};



