#include "include.h"

// PAGE

const UT_Vector3& Page::get(int i, int j) const
{
	int I = i;
	int J = j;

	while (I < 0) I += dim[0];
	while (J < 0) J += dim[1];

	I %= dim[0];
	J %= dim[1];

	return (*this)[I*dim[0] + J];
};

UT_Vector3& Page::get(int i, int j)
{
	int I = i;
	int J = j;

	while (I < 0) I += dim[0];
	while (J < 0) J += dim[1];

	I %= dim[0];
	J %= dim[1];

	return (*this)[I*dim[0] + J];
};

UT_Vector3 Page::get(UT_Vector3 C) const
{
	float x = C[0];
	float y = C[1];

	int DX = dim[0];
	int DY = dim[1];

	if (m_extended)
	{
		DX -= 2;
		DY -= 2;
	};

	float dx = 1.0 / DX;
	float dy = 1.0 / DY;

	if (m_extended)
	{
		if (x > 1 + 0.5 * dx) x = 1 + 0.5 * dx;
		if (y > 1 + 0.5 * dy) y = 1 + 0.5 * dy;

		if (x < -0.5 * dx) x = -0.5 * dx;
		if (y < -0.5 * dy) y = -0.5 * dy;
	}
	else
	{
		if (x > 1 - 0.5 * dx) x = 1 - 0.5 * dx;
		if (y > 1 - 0.5 * dy) y = 1 - 0.5 * dy;

		if (x < 0.5 * dx) x = 0.5 * dx;
		if (y < 0.5 * dy) y = 0.5 * dy;
	};

	int DDX = floor(x / dx + 0.5);
	int DDY = floor(y / dy + 0.5);

	float QX = DDX*dx - 0.5*dx;
	float QY = DDY*dy - 0.5*dy;

	float ddx = x - QX; ddx /= dx;
	float ddy = y - QY; ddy /= dy;

	UT_Vector3 U0 = get(DDX, DDY)*(1 - ddx);
	if (ddx != 0) U0 += get(DDX + 1, DDY)*ddx;
	UT_Vector3 U1 = get(DDX, DDY + 1)*(1 - ddx);
	if (ddx != 0) U1 += get(DDX + 1, DDY + 1)*ddx;

	UT_Vector3 res = U0*(1 - ddy) + U1*ddy;

	return res;
};

UT_Vector3 Page::d() const
{
	return UT_Vector3(
		1.0 / (dim[0] - (m_extended ? 2 : 0)),
		1.0 / (dim[1] - (m_extended ? 2 : 0)),
		0.0
	);
};

void Page::cross(UT_Vector3& C, const UT_Vector2& D, UT_Vector3& CLR) const
{
	float x = C[0];
	float y = C[1];

	int DX = dim[0];
	int DY = dim[1];

	if (m_extended)
	{
		DX -= 2;
		DY -= 2;
	};

	float dx = 1.0 / DX;
	float dy = 1.0 / DY;

	int DDX = floor(x / dx + 0.5);
	int DDY = floor(y / dy + 0.5);

	float QX = DDX*dx - 0.5*dx;
	float QY = DDY*dy - 0.5*dy;

	float ddx = x - QX; ddx /= dx;
	float ddy = y - QY; ddy /= dy;

	float tgx = D[0] < 0 ? 0.0 : 1.0;
	float tgy = D[1] < 0 ? 0.0 : 1.0;

	float X = ddx;
	float Y = ddy;

	//QQ[0] = QY; QQ[1] = QY;
	//dd[0] = ddx; dd[1] = ddy;
	//DD[0] = DDX; DD[1] = DDY;

	if (ddx*ddy == 0) // SIDE/CORNER CASE
	{
		if (ddx == 0 && D[0] < 0) { DDX--; QX -= dx; X = 1; };

		if (ddy == 0 && D[1] < 0) { DDY--; QY -= dy; Y = 1; };

		CLR = UT_Vector3(0, 0, 1);
	}
	else
	{
		CLR = UT_Vector3(1, 0, 1);
	};

	if (DDX == 0 && D[0] < 0) tgx = 0.5;
	if (DDX == DX && D[0] > 0) tgx = 0.5;
	if (DDY == 0 && D[1] < 0) tgy = 0.5;
	if (DDY == DY && D[1] > 0) tgy = 0.5;

	if (D[0] == 0)
	{
		C[0] = QX;
		C[1] = QY + tgy*dy;
		return;
	};

	if (D[1] == 0)
	{
		C[0] = QX + tgx*dx;
		C[1] = QY;
		return;
	};

	float tx = (tgx - X) / D[0];
	float ty = (tgy - Y) / D[1];

	float t = min(tx, ty);

	C[0] = QX + (X + D[0] * t)*dx;
	C[1] = QY + (Y + D[1] * t)*dy;

	return;
};

Page::Page(int i, int j, bool extended) : m_extended(extended)
{
	dim[0] = i; dim[1] = j;
	resize(capacity());
	zero();
};

void Page::zero()
{
	for (int k = 0; k < capacity(); k++) (*this)[k] = UT_Vector3(0, 0, 0);
};

int Page::capacity() const { return dim[0] * dim[1]; };

Page::operator UT_Vector3*() { return data(); };
const UT_Vector3* Page::getConstData() const { return data(); };

void Page::apply(const Page& src, int x0, int y0)
{
	for (int i = 0; i < dim[0]; i++)
		for (int j = 0; j < dim[1]; j++)
		{
			get(i, j) = src.get(i + x0, j + y0);
		};
};

Page::Page(const Page& src) : vector<UT_Vector3>(src)
{
	dim[0] = src.dim[0]; dim[1] = src.dim[1];
	m_name = src.m_name;
	m_extended = src.m_extended;
	m_projected = src.m_projected;
};

Page& Page::operator=(const Page& src)
{
	dim[0] = src.dim[0]; dim[1] = src.dim[1];
	resize(capacity());
	apply(src);

	m_name = src.m_name;

	m_extended = src.m_extended;
	m_projected = src.m_projected;

	return *this;
};

Page::Page()
{
	// SHOULD NOT BE!
	const char* MSG = "SHOULD NOT TO HAPPEN!";

	throw(MSG);

};

// PATCH
Patch::Patch(int x, int y, UT_StringArray names) :
	m_N(NULL), m_DPDU(NULL), m_DPDV(NULL)
{
	dim[0] = x; dim[1] = y;
	declarePages(names);
};

Patch::~Patch() { for (Page* p : *this) delete p; };

Patch::Patch(const Patch& src) : vector<Page*>(src),
	m_N(NULL), m_DPDU(NULL), m_DPDV(NULL)
{
	id = src.id;
	dim[0] = src.dim[0]; dim[1] = src.dim[1];
	memcpy(&f, &src.f, 4 * sizeof(int32));

	clear();

	for (Page* p : src)
	{
		Page* _P = new Page(*p);
		push_back(_P);
	};

	chs += src.chs;

	// SYNC PROJECTIONS
	int iN = src.chs.find("N");
	m_N = iN == -1 ? NULL : (*this)[iN];

	int iDPDU = src.chs.find("dPdu");
	m_DPDU = iDPDU == -1 ? NULL : (*this)[iDPDU];

	int iDPDV = src.chs.find("dPdv");
	m_DPDV = iDPDV == -1 ? NULL : (*this)[iDPDV];
};

void Patch::declarePages(const UT_StringArray& names)
{
	for (int i = 0; i < names.size(); i++)
	{
		UT_String name;
		name = names[i];
		declarePage(name);
	};
};

void Patch::declarePage(const UT_String& name)
{
	if (chs.find(name) != -1) return;

	// NEW
	Page* _p = new Page(dim[0], dim[1]);
	Page& p = *_p;
	p.m_name = name;
	p.zero();

	push_back(_p);

	chs.append(name);

	if (name == "N") m_N = _p;
	if (name == "dpdu") m_DPDU = _p;
	if (name == "dpdv") m_DPDV = _p;
};

Page& Patch::getPrimVar(const UT_String& name)
{
	// ALREADY DEFINED
	int index = chs.find(name);
	if (index != -1)
	{
		Page& found = *(*this)[index];
		return found;
	};

	const char* MSG = "SHOULD NOT HAPPEN!";
	throw(MSG);
};

const Page& Patch::getPrimVar(const UT_String& name) const
{
	// ALREADY DEFINED
	int index = chs.find(name);
	if (index != -1)
	{
		const Page& found = *(*this)[index];
		return found;
	};

	const char* MSG = "SHOULD NOT HAPPEN!";
	throw(MSG);
};

const UT_Vector3 Patch::centroid() const
{
	const Page& P = getPrimVar(UT_String("P"));

	int x0 = 0, x1 = 0;
	if (dim[0] % 2 == 1)
	{
		x0 = (dim[0] - 1) / 2;
		x1 = x0;
	}
	else
	{
		x0 = dim[0] / 2 - 1;
		x1 = x0 + 1;
	};

	int y0 = 0, y1 = 0;
	if (dim[1] % 2 == 1)
	{
		y0 = (dim[1] - 1) / 2;
		y1 = y0;
	}
	else
	{
		y0 = dim[1] / 2 - 1;
		y1 = y0 + 1;
	};

	UT_Vector3 C0 = P.get(x0, y0);
	UT_Vector3 C1 = P.get(x1, y1);

	return (C0 + C1) / 2;
};

int64 Patch::memsize() const
{
	int64 m = sizeof(Patch);
	for (const Page* p : *this) m += p->capacity() * 3 * sizeof(float);
	return m;
};

void Patch::kill(int n)
{
	Page* l = (*this)[n];
	delete l;

	(*this)[n] = (*this).back();
	resize(chs.size() - 1);

	chs[n] = chs.last();
	chs.setSize(chs.size() - 1);
};

void Patch::killLast()
{
	Page* l = (*this).back();
	delete l;

	int n = chs.size() - 1;
	chs.setSize(n);
	resize(n);
};

int Patch::reflect(int id) const
{
	for (int i = 0; i < 4; i++)
	{
		if (id != f[i]) continue;
		return i;
	};

	return -1;
};

UT_Vector3 Patch::project(UT_Vector3 V, int i, int j)
{
	UT_Vector3 RES(0, 0, 0);

	if (m_N == NULL) return RES;
	if (m_DPDU == NULL) return RES;
	if (m_DPDV == NULL) return RES;

	UT_Vector3 n = m_N->get(i, j);
	UT_Vector3 dPdu = m_DPDU->get(i, j);
	UT_Vector3 dPdv = m_DPDV->get(i, j);

	RES[2] = n.dot(V);

	UT_Vector3 _V = V - n*RES[2];

	RES[0] = _V.dot(dPdu);

	dPdv -= dPdv.dot(dPdu)*dPdu;
	dPdv.normalize();

	RES[1] = _V.dot(dPdv);

	return RES;
};

UT_Vector3 Patch::project(UT_Vector3 V, UT_Vector3 UV)
{
	UT_Vector3 RES(0, 0, 0);

	if (m_N == NULL) return RES;
	if (m_DPDU == NULL) return RES;
	if (m_DPDV == NULL) return RES;

	UT_Vector3 n = m_N->get(UV);
	UT_Vector3 dPdu = m_DPDU->get(UV);
	UT_Vector3 dPdv = m_DPDV->get(UV);

	RES[2] = n.dot(V);

	UT_Vector3 _V = V - n*RES[2];

	RES[0] = _V.dot(dPdu);

	dPdv -= dPdv.dot(dPdu)*dPdu;
	dPdv.normalize();

	RES[1] = _V.dot(dPdv);

	return RES;
};

UT_Vector3 Patch::composite(UT_Vector3 V, UT_Vector3 UV)
{
	UT_Vector3 RES(0, 0, 0);

	if (m_N == NULL) return RES;
	if (m_DPDU == NULL) return RES;
	if (m_DPDV == NULL) return RES;

	UT_Vector3 n = m_N->get(UV);
	UT_Vector3 dPdu = m_DPDU->get(UV);

	UT_Vector3 dPdv = m_DPDV->get(UV);
	dPdv -= dPdv.dot(dPdu)*dPdu;
	dPdv.normalize();

	RES = dPdu*V[0] + dPdv*V[1] + n*V[2];
	return RES;
};

Patch::Patch()
{
	// SHOULD NOT HAPPEN!
	const char* MSG = "SHOULD NOT HAPPEN!";

	throw(MSG);
};

	///////////////////////////////////////////////////////////////
// UTILS

void pb2geo(GU_Detail* gdp, const Patch& pb)
{
	// BUILD POINTS
	GEO_Hull* mesh = GU_PrimMesh::build(gdp, pb.dim[0], pb.dim[1], GEO_PATCH_QUADS, 0, 0, 0);

	GA_Attribute* idatt = gdp->findPrimitiveAttribute("_pbid_");
	if (idatt == NULL)
	{
		idatt = gdp->addIntTuple(
			GA_AttributeOwner::GA_ATTRIB_PRIMITIVE,
			GA_AttributeScope::GA_SCOPE_PUBLIC,
			"_pbid_", 1);
	};

	GA_RWHandleI pbida(idatt);
	pbida.set(mesh->getMapOffset(), pb.id);

	int nelems = pb.dim[0] * pb.dim[1];

	GA_Offset ptoff = gdp->appendPointBlock(nelems);

	for (int i = 0; i<pb.chs.size(); i++)
	{
		const Page& pp = *pb[i];
		const UT_Vector3* data = pp.getConstData();

		GA_RWHandleV3 ha(gdp->findPointAttribute(pb.chs[i]));
		if (ha.isValid())
		{
			ha.setBlock(ptoff, nelems, data);
			continue;
		};

		GA_Attribute* att = gdp->addFloatTuple(
			GA_AttributeOwner::GA_ATTRIB_POINT,
			GA_AttributeScope::GA_SCOPE_PUBLIC,
			pb.chs[i], 3);

		GA_RWHandleV3 _ha(att);
		_ha.setBlock(ptoff, nelems, data);
	};

	for (int i = 0; i < pb.dim[0]; i++)
	{
		for (int j = 0; j < pb.dim[1]; j++)
		{
			GEO_Vertex& vtx = (*mesh)(i, j);
			vtx.setPointOffset(ptoff);
			ptoff++;
		};
	};

	GA_Attribute* att = gdp->findPrimitiveAttribute("__connectivity__");
	if (att == NULL)
	{
		att = gdp->addIntTuple(
			GA_AttributeOwner::GA_ATTRIB_PRIMITIVE,
			GA_AttributeScope::GA_SCOPE_PUBLIC,
			"__connectivity__", 4);
	};
	GA_RWHandleT<UT_Vector4I> ca(att);

	UT_Vector4I val(pb.f);
	ca.set(mesh->getMapOffset(), val);

	return;
};

void geo2pb(vector<Patch*>& PB, const GU_Detail* gdp)
{
	// CHECK PRIMITIVES
	vector<const GEO_Hull*> patches;

	const GEO_Primitive* prim = NULL;

	GA_FOR_ALL_PRIMITIVES(gdp, prim)
	{
		const GEO_Hull* ps = reinterpret_cast<const GEO_Hull*>(prim);

		if (ps == NULL)
		{
			continue;
		};

		patches.push_back(ps);
	};

	if (patches.size() == 0)
	{
		return;
	}

	// CHECK REQUIRED CHANNELS & PREPARE HANDLES
	const GA_Attribute* att = gdp->findPrimitiveAttribute("__connectivity__");
	if (att == NULL)
	{
		return;
	};

	GA_ROHandleT<UT_Vector4I> ca(att);

	const GA_Attribute* idatt = gdp->findPrimitiveAttribute("_pbid_");
	if (idatt == NULL)
	{
		return;
	};

	GA_ROHandleI pbida(idatt);

	///////////////

	UT_StringArray baseNames;
	int chi = 0;
	while (ch_base_names[chi] != NULL)
	{
		baseNames.append(ch_base_names[chi]);
		chi++;
	};

	vector< const GA_Attribute* > ba(chi);

	for (int i = 0; i < chi; i++)
	{
		const GA_Attribute* att = gdp->findPointAttribute(ch_base_names[i]);
		if (att == NULL)
		{
			return;
		};

		ba[i] = att;
	};

	// ANALYSE SPARE CHANNELS
	vector<const GA_Attribute*> sa;
	UT_StringArray sa_names;

	const GA_AttributeDict& ad = gdp->pointAttribs();

	GA_AttributeDict::iterator it = ad.begin();

	while (it != ad.end())
	{
		const char* nm = it.name();

		UT_String name = nm;
		if (baseNames.find(name) != -1) { ++it; continue; };


		GA_Attribute* a = it.attrib();
		if (a->getTupleSize() != 3) { ++it; continue; };

		const GA_AttributeType& type = a->getType();

		if (type.getTypeId() != 2) { ++it; continue; };

		sa.push_back(a);
		sa_names.append(name);

		++it;
	}

	// COLLECT
	PB.resize(patches.size());

	UT_StringArray all_names = baseNames;
	all_names += sa_names;

	for (int i = 0; i < sa.size(); i++) ba.push_back(sa[i]);

	vector<GA_ROHandleT<UT_Vector3F> > H(ba.size());
	for (int i = 0; i < ba.size(); i++) H[i].bind(ba[i]);

	//int index = 0;
	for (const GEO_Hull* p : patches)
	{
		Patch* _pb = new Patch(p->getNumCols(), p->getNumRows(), all_names);
		Patch& pb = *_pb;

		// CONNECTIVITY
		UT_Vector4I F = ca.get(p->getMapOffset());

		pb.f[0] = F[0]; pb.f[1] = F[1]; pb.f[2] = F[2]; pb.f[3] = F[3];

		//GA_Range prng = p->getPointRange();

		vector<GA_Offset> off(pb.dim[0] * pb.dim[1]);

		int idx = 0;
		for (int i = 0; i < pb.dim[0]; i++)
		{
			for (int j = 0; j < pb.dim[1]; j++)
			{
				const GEO_Vertex& vtx = (*p)(i, j);
				off[idx] = vtx.getPointOffset();
				idx++;
			};
		};

		//idx = 0;
		for (int i = 0; i < all_names.size(); i++)
		{
			Page& pg = pb.getPrimVar(all_names[i].c_str());

			for (int j = 0; j < pb.dim[0] * pb.dim[1]; j++)
			{
				UT_Vector3 V = H[i].get(off[j]);
				pg[j] = V;
			}
		};


		// ID
		int ID = pbida.get(p->getMapOffset());
		pb.id = ID;

		PB[ID] = _pb;
	};

	return;
};

// +++
void edges2geo(GU_Detail* gdp, const map<string, vector< UT_Vector2i> >& E)
{
	for (const pair<string, vector< UT_Vector2i> >&e : E)
	{
		UT_String grp = EDGE_CLAUSE;
		grp += e.first;

		GA_Attribute* att = gdp->findGlobalAttribute(grp);

		att = gdp->findIntArray(GA_AttributeOwner::GA_ATTRIB_GLOBAL,grp,-1, -1);

		if (att == NULL)
		{
			att = gdp->addIntArray(GA_AttributeOwner::GA_ATTRIB_GLOBAL, grp, 2);
		};

		const GA_AIFNumericArray *aif = att->getAIFNumericArray();

		UT_IntArray data;

		for (UT_Vector2i _e : e.second)
		{
			data.append(_e[0]);
			data.append(_e[1]);
		};

		aif->set(att, GA_Offset(0), data);

		att->bumpDataId();
	}

};

// +++
void geo2edges(map<string, vector< UT_Vector2i> >& E, const GU_Detail* gdp)
{
	const GA_AttributeDict& ad = gdp->attribs();

	GA_AttributeDict::iterator it = ad.begin();

	while (it != ad.end())
	{
		const char* nm = it.name();

		UT_String name = nm;

		if (!name.startsWith(EDGE_CLAUSE)) { ++it; continue; };

		GA_Attribute* a = it.attrib();
		if (a->getTupleSize() != 2) { ++it; continue; };

		const GA_AttributeType& type = a->getType();
		if (type.getTypeId() != 1) { ++it; continue; };

		const GA_AIFNumericArray* aif = a->getAIFNumericArray();
		if (aif == NULL) { ++it; continue; };

		UT_IntArray data;
		aif->get(a, GA_Offset(0), data);

		vector< UT_Vector2i> _val;
		E[nm] = _val;

		vector< UT_Vector2i>& val = E[nm];
		for (int i = 0; i < data.size(); i += 2) val.push_back(UT_Vector2i(data[i], data[i + 1]));

		++it;
	};
};

// +--
void addClosure(GU_Detail* gdp, const vector<Patch*>& PB)
{
	GA_PrimitiveGroup* cg = gdp->findPrimitiveGroup(CLOSURE_GROUP);
	if (cg == NULL) cg = gdp->newPrimitiveGroup(CLOSURE_GROUP);

	GA_PrimitiveGroup* ccg = gdp->findPrimitiveGroup(CLOSURE_CORNERS_GROUP);
	if (ccg == NULL) ccg = gdp->newPrimitiveGroup(CLOSURE_CORNERS_GROUP);

	for (const Patch* pb : PB)
	{
		int id = pb->id;

		//GA_Offset pp = gdp->primitiveOffset(id);

		GA_Primitive* prim = gdp->getPrimitiveByIndex(id);

		const GEO_Hull* ps = reinterpret_cast<const GEO_Hull*>(prim);

		// HERE MAP
		int basex[4] = { 0, pb->dim[0] - 1, pb->dim[0] - 1, 0 };
		int dx[4] = { +1, 0, -1, 0 };

		int basey[4] = { 0, 0, pb->dim[1]-1, pb->dim[1]-1 };
		int dy[4] = { 0, +1, 0, -1 };

		for (int i = 0; i < 4; i++)
		{
			int _id = pb->f[i];

			// SIDES
			if (_id != -1 && id < _id)
			{
				// SHOULD BUILD

				// THERE MAP
				int _basex[4] = { pb->dim[0] - 1, pb->dim[0] - 1, 0, 0 };
				int _dx[4] = { -1, 0, 1, 0 };

				int _basey[4] = { 0, pb->dim[1] - 1, pb->dim[1] - 1, 0 };
				int _dy[4] = { 0, -1, 0, +1 };

				const GEO_Hull* po = reinterpret_cast<const GEO_Hull*>(gdp->getPrimitiveByIndex(_id));

				const Patch* pp = PB[_id];

				int _i = pp->reflect(id);

				int dim = pb->dim[i % 2];

				int I, J;

				GEO_Hull* P = GU_PrimMesh::build(gdp, dim, 2, GEO_PATCH_QUADS, 0, 0, 0);
				cg->add(P);

				for (int j = 0; j < dim; j++)
				{
					//GEO_PrimPoly* P = GU_PrimPoly::build(gdp, 4, 0, 0);

					I = basex[i] + dx[i] * j;
					J = basey[i] + dy[i] * j;
					GA_Offset v0 = ps->getVertexOffset(I, J);
					P->setVertexPoint(j, 1, gdp->vertexPoint(v0));

					I = _basex[_i] + _dx[_i] * j;
					J = _basey[_i] + _dy[_i] * j;
					GA_Offset v2 = po->getVertexOffset(I, J);
					P->setVertexPoint(j, 0, gdp->vertexPoint(v2));
				};
			};

			// CORNERS
			//bool build = true;

			UT_IntArray naf;
			UT_IntArray nrf;

			naf.append(pb->id);
			nrf.append(i);

			// NEG WIND
			int last = pb->id;
			int cf = pb->f[i];

			while (cf != -1 && cf > pb->id)
			{
				const Patch& pp = *PB[cf];
				naf.append(cf);

				int ref = pp.reflect(last);

				ref += 1; ref %= 4;
				nrf.append(ref);

				last = cf;
				cf = pp.f[ref];
			};

			if (cf != -1 && cf < pb->id) continue; // ...ALREADY BUILT


			UT_IntArray paf;
			UT_IntArray prf;

			if (cf == -1)
			{
				// POS WIND

				last = pb->id;
				cf = pb->f[(i+3)%4];

				while (cf != -1 && cf > pb->id)
				{
					const Patch& pp = *PB[cf];
					paf.append(cf);

					int ref = pp.reflect(last);

					prf.append(ref);
					ref += 3; ref %= 4;

					last = cf;
					cf = pp.f[ref];
				};

				if (cf != -1 && cf < pb->id) continue; // ...ALREADY BUILT
			};

			paf.reverse();
			prf.reverse();

			paf.concat(naf);
			prf.concat(nrf);

			if (paf.size() < 3)  continue; // ...SKIP SIMPLE BORDER

			GEO_PrimPoly* PP = GEO_PrimPoly::build(gdp, paf.size(), false, false);
			ccg->add(PP);
			for (int j = 0; j < paf.size(); j++)
			{
				const Patch& p = *PB[paf[j]];

				int corner = prf[j];

				int I = (corner + 1) % 4 < 2 ? 0 : p.dim[0] - 1;
				int J = corner < 2 ? 0 : p.dim[1] - 1;

				const GEO_Hull* pp = reinterpret_cast<const GEO_Hull*>(gdp->getPrimitiveByIndex(p.id));

				GA_Offset v0 = pp->getVertexOffset(I, J);
				PP->setVertexPoint(j, gdp->vertexPoint(v0));
			};
		};
	};
};

int CASE[16] = {
	1, 2, 3, 0, // 0
	0, 1, 2, 3, // 1
	3, 0, 1, 2, // 2
	2, 3, 0, 1	// 3
};

// +++
bool trace(const vector<Patch*>& PEBBLE, UT_Vector3 coords, /*UT_Vector2 d,*/ float & length,
	UT_Vector3Array & path, UT_Vector3Array & colors, //UT_Vector3Array & normals,
	vector<Page*>& Ps, vector<Page*>& Vs,
	vector<Page*>& dPdUs, vector<Page*>& dPdVs, vector<Page*>& Ns,
	vector<Page*>& Rels,
	UT_Lock& lock, traceStat& stat)
{
	if (stat.max_deep < 0)
	{
		stat.state = 1;
		return false;
	};

	UT_Matrix2FArray M(4);
	M[0] = UT_Matrix2F(0, -1, 1, 0);
	M[1] = UT_Matrix2F(-1, 0, 0, -1);
	M[2] = UT_Matrix2F(0, 1, -1, 0);
	M[3] = UT_Matrix2F(1);


	const Patch& pb = *PEBBLE[coords[2]];

	Page* _P = NULL;
	Page* _V = NULL;
	Page* _DPDU = NULL;
	Page* _DPDV = NULL;
	Page* _N = NULL;
	Page* _RELS = NULL;

	lock.lock();
	{
		if (Ps[pb.id] == NULL) Ps[pb.id] = getExpandedPrimVar(PEBBLE, pb, "P");
		_P = Ps[pb.id];

		if (Vs[pb.id] == NULL) Vs[pb.id] = getExpandedPrimVarProjected(PEBBLE, pb, "v");
		_V = Vs[pb.id];

		if (dPdUs[pb.id] == NULL) dPdUs[pb.id] = getExpandedPrimVar(PEBBLE, pb, "dPdu");
		_DPDU = dPdUs[pb.id];

		if (dPdVs[pb.id] == NULL) dPdVs[pb.id] = getExpandedPrimVar(PEBBLE, pb, "dPdv");
		_DPDV = dPdVs[pb.id];

		if (Ns[pb.id] == NULL) Ns[pb.id] = getExpandedPrimVar(PEBBLE, pb, "N");
		_N = Ns[pb.id];

		if (Rels[pb.id] == NULL) Rels[pb.id] = getExpandedPrimVar(PEBBLE, pb, "rel");
		_RELS = Rels[pb.id];
	}
	lock.unlock();

	const Page& PP = *_P;
	const Page& PV = *_V;
	const Page& PDU = *_DPDU;
	const Page& PDV = *_DPDV;
	const Page& N = *_N;
	const Page& REL = *_RELS;


	// TRACE

	bool go = true;

	UT_Vector3 UV = coords;

	UT_Vector3 O = PP.get(UV);

	//UT_Vector2 D; // = d; D.normalize();

	UT_Vector3 C;

	//vector<float> Q(2);
	//vector<int>	  DD(2);
	//vector<float> dd(2);

	//UT_Vector3 UV_Orig = UV;

	//UT_Vector3 v;

	//UT_Vector3 dpdu;
	//UT_Vector3 dpdv;

	//UT_Vector3 n = dpdu;

	do
	{
		// MOVE AHEAD
		UT_Vector3 UV_Orig = UV;

		// DIRECTION
		UT_Vector3 v = PV.get(UV);

		//UT_Vector3 dpdu = PDU.get(UV); dpdu.normalize();
		//UT_Vector3 dpdv = PDV.get(UV); dpdv.normalize();

		//UT_Vector3 n = N.get(UV); n.normalize();
		//n.cross(dpdv);

		//v -= n*n.dot(v);

		UT_Vector2 D(v[0], v[1]);

		//D[0] = v.dot(dpdu);
		//D[1] = v.dot(dpdv);
		D.normalize();

		// ...SHOULD ALTER NORMALIZATION PROCEDURE IN ACCOUNT WITH SKEW

		//int smm = 0;

		//D[0] = 0; D[1] = 0;

		//float detxy = dpdu[0] * dpdv[1] - dpdu[1] * dpdv[0];
		//if (detxy != 0)
		//{
		//	smm++;

		//	float dU = v[0] * dpdv[1] - v[1] * dpdv[0];
		//	float dV = dpdu[0] * v[1] - dpdu[1] * v[0];
		//	D[0] += dU / detxy; D[1] += dV / detxy;
		//};

		//// ...OTHER SIDES
		////float detxz = dpdu[0] * dpdv[2] - dpdu[2] * dpdv[0];
		////float detyz = dpdu[1] * dpdv[2] - dpdu[2] * dpdv[1];

		//if (smm == 0)
		//{
		//	stat.state = 5;
		//	return false;
		//};

		//D[0] /= smm; D[1] /= smm;
		//D.normalize();

		if (stat.back) D *= -1;

		PP.cross(UV, D, C);
		UV[2] = pb.id;

		// STOP CONDITIONS
		if (UV[0] <= 0) { UV[0] = 0; go = false; };
		if (UV[1] <= 0) { UV[1] = 0; go = false; };
		if (UV[0] >= 1) { UV[0] = 1; go = false; };
		if (UV[1] >= 1) { UV[1] = 1; go = false; };

		// RECALC LENGTH
		UT_Vector3 _O = PP.get(UV);
		float dl = (_O - O).length();

		if (dl == 0)
		{
			stat.state = 2;
			return false;
		}

		if (length < dl)
		{
			dl = length / dl;

			UV = UV*dl + UV_Orig*(1 - dl);

			UV[2] = pb.id;


			path.append(UV);
			colors.append(UT_Vector3(1, 0, 1)); //C);

			return true;
		};

		length -= dl;

		// BACK UV

		path.append(UV);
		colors.append(UT_Vector3(0, 0, 1)); //C);

		if (path.size() > stat.max_path)
		{
			stat.state = 3;
			return false;
		};

		if (REL.get(UV)[1] != 0)
		{
			return true;
		}

		// RECALC CONDITIONS
		O = _O;


	} while (go);

	//return true;

	float U = UV[0];
	float V = UV[1];

	int ref = -1;

	if (V == 0) ref = 0;
	if (U == 1) ref = 1;
	if (V == 1) ref = 2;
	if (U == 0) ref = 3;

	int rref = pb.f[ref];

	// BOUNDARY?
	if (rref == -1)
	{
		return true;
	}

	UV[2] = rref;

	const Patch& pbb = *PEBBLE[rref];

	//float DU = du, DV = dv;

	int pref = pbb.reflect(pb.id);

	//UT_Vector2 _D = D;

	// 0
	if (ref == 0)
	{
		if (pref == 0)
		{
			UV[0] = 1 - U;
			UV[1] = 0;
			//D[0] = -_D[0];
			//D[1] = -_D[1];
		};
		if (pref == 1)
		{
			UV[0] = 1;
			UV[1] = 1 - U;
			//D[0] = _D[1];
			//D[1] = -_D[0];
		};
		if (pref == 2)
		{
			UV[1] = 1;
		};
		if (pref == 3)
		{
			UV[0] = 0;
			UV[1] = U;
			//D[0] = -_D[1];
			//D[1] = _D[0];
		};
	};

	// 1
	if (ref == 1)
	{
		if (pref == 0)
		{
			UV[0] = 1 - V;
			UV[1] = 0;
			//D[0] = -_D[1];
			//D[1] = _D[0];
		};
		if (pref == 1)
		{
			UV[0] = 1;
			UV[1] = 1 - V;
			//D[0] = -_D[0];
			//D[1] = -_D[1];
		};
		if (pref == 2)
		{
			UV[0] = V;
			UV[1] = 1;
			//D[0] = _D[1];
			//D[1] = -_D[0];
		};
		if (pref == 3)
		{
			UV[0] = 0;
		};
	};

	// 2
	if (ref == 2)
	{
		if (pref == 0)
		{
			UV[1] = 0;
		};
		if (pref == 1)
		{
			UV[0] = 1;
			UV[1] = U;
			//D[0] = -_D[1];
			//D[1] = _D[0];
		};
		if (pref == 2)
		{
			UV[0] = 1 - U;
			UV[1] = 1;
			//D[0] = -_D[1];
			//D[1] = -_D[0];
		};
		if (pref == 3)
		{
			UV[0] = 0;
			UV[1] = 1 - U;
			//D[0] = -_D[1];
			//D[1] = _D[0];
		};
	};

	// 3
	if (ref == 3)
	{
		if (pref == 0)
		{
			UV[0] = V;
			UV[1] = 0;
			//D[0] = _D[1];
			//D[1] = -_D[0];
		};
		if (pref == 1)
		{
			UV[0] = 1;
		};
		if (pref == 2)
		{
			UV[0] = 1 - V;
			UV[1] = 1;
			//D[0] = _D[1];
			//D[1] = -_D[0];
		};
		if (pref == 3)
		{
			UV[0] = 0;
			UV[1] = 1 - V;
			//D[0] = -_D[0];
			//D[1] = -_D[1];
		};
	};

	stat.max_deep--;

	UT_Matrix2F& MM = M[CASE[pref * 4 + ref]];

	stat.w *= MM;

	bool result = trace(PEBBLE, UV, length,
		path, colors,
		Ps, Vs,
		dPdUs, dPdVs, Ns,
		Rels,
		lock, stat);

	if (!result)
	{
		return false;
	};

	return true;
};

bool trace_orig(const vector<Patch*>& PEBBLE, UT_Vector3 coords, /*UT_Vector2 d,*/ float & length,
	UT_Vector3Array & path, UT_Vector3Array & colors, //UT_Vector3Array & normals,
	vector<Page*>& Ps, vector<Page*>& Vs,
	//vector<Page*>& dPdUs, vector<Page*>& dPdVs, vector<Page*>& Ns,
	vector<Page*>& Rels,
	UT_Lock& lock, traceStat& stat)
{
	if (stat.max_deep < 0)
	{
		stat.state = 1;
		return false;
	};

	UT_Matrix2FArray M(4);
	M[0] = UT_Matrix2F(0, -1, 1, 0);
	M[1] = UT_Matrix2F(-1, 0, 0, -1);
	M[2] = UT_Matrix2F(0, 1, -1, 0);
	M[3] = UT_Matrix2F(1);


	const Patch& pb = *PEBBLE[coords[2]];

	Page* _P = NULL;
	Page* _V = NULL;
	//Page* _DPDU = NULL;
	//Page* _DPDV = NULL;
	//Page* _N = NULL;
	Page* _RELS = NULL;

	lock.lock();
	{
		if (Ps[pb.id] == NULL) Ps[pb.id] = getExpandedPrimVar(PEBBLE, pb, "P");
		_P = Ps[pb.id];

		if (Vs[pb.id] == NULL) Vs[pb.id] = getExpandedPrimVarProjected(PEBBLE, pb, "v");
		_V = Vs[pb.id];

		//if (dPdUs[pb.id] == NULL) dPdUs[pb.id] = getExpandedPrimVar(PEBBLE, pb, "dPdu", true);
		//_DPDU = dPdUs[pb.id];

		//if (dPdVs[pb.id] == NULL) dPdVs[pb.id] = getExpandedPrimVar(PEBBLE, pb, "dPdv", true);
		//_DPDV = dPdVs[pb.id];

		//if (Ns[pb.id] == NULL) Ns[pb.id] = getExpandedPrimVar(PEBBLE, pb, "N");
		//_N = Ns[pb.id];

		if (Rels[pb.id] == NULL) Rels[pb.id] = getExpandedPrimVar(PEBBLE, pb, "rel");
		_RELS = Rels[pb.id];
	}
	lock.unlock();

	const Page& PP = *_P;
	const Page& PV = *_V;
	//const Page& PDU = *_DPDU;
	//const Page& PDV = *_DPDV;
	//const Page& N = *_N;
	const Page& REL = *_RELS;


	// TRACE

	bool go = true;

	UT_Vector3 UV = coords;

	UT_Vector3 O = PP.get(UV);

	//UT_Vector2 D; // = d; D.normalize();

	UT_Vector3 C;

	//vector<float> Q(2);
	//vector<int>	  DD(2);
	//vector<float> dd(2);

	//UT_Vector3 UV_Orig = UV;

	//UT_Vector3 v;

	//UT_Vector3 dpdu;
	//UT_Vector3 dpdv;

	//UT_Vector3 n = dpdu;

	do
	{
		// MOVE AHEAD
		UT_Vector3 UV_Orig = UV;

		// DIRECTION
		UT_Vector3 v = PV.get(UV);

		//UT_Vector3 dpdu = PDU.get(UV); dpdu.normalize();
		//UT_Vector3 dpdv = PDV.get(UV); dpdv.normalize();

		//UT_Vector3 n = N.get(UV); n.normalize();
		//n.cross(dpdv);

		//v -= n*n.dot(v);

		UT_Vector2 D(v[0],v[1]);

		//D[0] = v.dot(dpdu);
		//D[1] = v.dot(dpdv);
		D.normalize();

		// ...SHOULD ALTER NORMALIZATION PROCEDURE IN ACCOUNT WITH SKEW

		//int smm = 0;

		//D[0] = 0; D[1] = 0;

		//float detxy = dpdu[0] * dpdv[1] - dpdu[1] * dpdv[0];
		//if (detxy != 0)
		//{
		//	smm++;

		//	float dU = v[0] * dpdv[1] - v[1] * dpdv[0];
		//	float dV = dpdu[0] * v[1] - dpdu[1] * v[0];
		//	D[0] += dU / detxy; D[1] += dV / detxy;
		//};

		//// ...OTHER SIDES
		////float detxz = dpdu[0] * dpdv[2] - dpdu[2] * dpdv[0];
		////float detyz = dpdu[1] * dpdv[2] - dpdu[2] * dpdv[1];

		//if (smm == 0)
		//{
		//	stat.state = 5;
		//	return false;
		//};

		//D[0] /= smm; D[1] /= smm;
		//D.normalize();

		if (stat.back) D *= -1;

		PP.cross(UV, D, C);
		UV[2] = pb.id;

		// STOP CONDITIONS
		if (UV[0] <= 0) { UV[0] = 0; go = false; };
		if (UV[1] <= 0) { UV[1] = 0; go = false; };
		if (UV[0] >= 1) { UV[0] = 1; go = false; };
		if (UV[1] >= 1) { UV[1] = 1; go = false; };

		// RECALC LENGTH
		UT_Vector3 _O = PP.get(UV);
		float dl = (_O - O).length();

		if (dl == 0)
		{
			stat.state = 2;
			return false;
		}

		if (length < dl)
		{
			dl = length / dl;

			UV = UV*dl + UV_Orig*(1 - dl);

			UV[2] = pb.id;


			path.append(UV);
			colors.append(UT_Vector3(1, 0, 1)); //C);

			return true;
		};

		length -= dl;

		// BACK UV

		path.append(UV);
		colors.append(UT_Vector3(0, 0, 1)); //C);

		if (path.size() > stat.max_path)
		{
			stat.state = 3;
			return false;
		};

		if (REL.get(UV)[1] != 0)
		{
			return true;
		}

		// RECALC CONDITIONS
		O = _O;


	} while (go);

	//return true;

	float U = UV[0];
	float V = UV[1];

	int ref = -1;

	if (V == 0) ref = 0;
	if (U == 1) ref = 1;
	if (V == 1) ref = 2;
	if (U == 0) ref = 3;

	int rref = pb.f[ref];

	// BOUNDARY?
	if (rref == -1)
	{
		return true;
	}

	UV[2] = rref;

	const Patch& pbb = *PEBBLE[rref];

	//float DU = du, DV = dv;

	int pref = pbb.reflect(pb.id);

	//UT_Vector2 _D = D;

	// 0
	if (ref == 0)
	{
		if (pref == 0)
		{
			UV[0] = 1 - U;
			UV[1] = 0;
			//D[0] = -_D[0];
			//D[1] = -_D[1];
		};
		if (pref == 1)
		{
			UV[0] = 1;
			UV[1] = 1 - U;
			//D[0] = _D[1];
			//D[1] = -_D[0];
		};
		if (pref == 2)
		{
			UV[1] = 1;
		};
		if (pref == 3)
		{
			UV[0] = 0;
			UV[1] = U;
			//D[0] = -_D[1];
			//D[1] = _D[0];
		};
	};

	// 1
	if (ref == 1)
	{
		if (pref == 0)
		{
			UV[0] = 1 - V;
			UV[1] = 0;
			//D[0] = -_D[1];
			//D[1] = _D[0];
		};
		if (pref == 1)
		{
			UV[0] = 1;
			UV[1] = 1 - V;
			//D[0] = -_D[0];
			//D[1] = -_D[1];
		};
		if (pref == 2)
		{
			UV[0] = V;
			UV[1] = 1;
			//D[0] = _D[1];
			//D[1] = -_D[0];
		};
		if (pref == 3)
		{
			UV[0] = 0;
		};
	};

	// 2
	if (ref == 2)
	{
		if (pref == 0)
		{
			UV[1] = 0;
		};
		if (pref == 1)
		{
			UV[0] = 1;
			UV[1] = U;
			//D[0] = -_D[1];
			//D[1] = _D[0];
		};
		if (pref == 2)
		{
			UV[0] = 1 - U;
			UV[1] = 1;
			//D[0] = -_D[1];
			//D[1] = -_D[0];
		};
		if (pref == 3)
		{
			UV[0] = 0;
			UV[1] = 1 - U;
			//D[0] = -_D[1];
			//D[1] = _D[0];
		};
	};

	// 3
	if (ref == 3)
	{
		if (pref == 0)
		{
			UV[0] = V;
			UV[1] = 0;
			//D[0] = _D[1];
			//D[1] = -_D[0];
		};
		if (pref == 1)
		{
			UV[0] = 1;
		};
		if (pref == 2)
		{
			UV[0] = 1 - V;
			UV[1] = 1;
			//D[0] = _D[1];
			//D[1] = -_D[0];
		};
		if (pref == 3)
		{
			UV[0] = 0;
			UV[1] = 1 - V;
			//D[0] = -_D[0];
			//D[1] = -_D[1];
		};
	};

	stat.max_deep--;

	UT_Matrix2F& MM = M[CASE[pref * 4 + ref]];

	stat.w *= MM;

	bool result = trace_orig(PEBBLE, UV, length,
		path, colors,
		Ps, Vs,
		//dPdUs, dPdVs, Ns,
		Rels,
		lock, stat);

	if (!result)
	{
		return false;
	};

	return true;
};

//Page* getExpandedPrimVar(const vector<Patch*>& PEBBLE, const Patch & pb, UT_String name)
//{
//	// MAKE
//	Page* _P = new Page(pb.dim[0] + 2, pb.dim[1] + 2, true);
//
//	Page& P = *_P;
//
//	// ASSIGN
//	const Page& E = pb.getPrimVar(name);
//
//	// CORE
//	for (int i = 0; i < pb.dim[0]; i++)
//		for (int j = 0; j < pb.dim[1]; j++)
//			P.get(i + 1, j + 1) = E.get(i, j);
//
//	// SIDES
//
//	int xs[4] = { 1,	pb.dim[0] + 1,	pb.dim[0],	0 };
//	int ys[4] = { 0,	1,			pb.dim[1] + 1,	pb.dim[1] };
//
//	int dxs[4] = { 1,0,-1,0 };
//	int dys[4] = { 0,1,0,-1 };
//
//	int sl[4] = { pb.dim[0],pb.dim[1],pb.dim[0],pb.dim[1] };
//
//	int ddx[4] = { 0,-1,0,1 };
//	int ddy[4] = { 1,0,-1,0 };
//
//	int id = pb.id;
//
//	UT_Matrix2FArray M(4);
//
//	M[0] = UT_Matrix2F(0, -1, 1, 0);
//	M[1] = UT_Matrix2F(-1, 0, 0, -1);
//	M[2] = UT_Matrix2F(0, 1, -1, 0);
//	M[3] = UT_Matrix2F(1);
//
//	int CASE[16] = { 
//		1, 2, 3, 0, // 0
//		0, 1, 2, 3, // 1
//		3, 0, 1, 2, // 2
//		2, 3, 0, 1	// 3
//	};
//
//	for (int S = 0; S < 4; S++)
//	{
//
//		int BX = 0;
//		int BY = -1;
//
//		int mark = /*extrapolate ? -1 :*/ pb.f[S];
//		if (mark != -1)
//		{
//			const Patch& PP = *PEBBLE[pb.f[S]];
//
//			int x0 = xs[S];
//			int y0 = ys[S];
//			int dx = dxs[S];
//			int dy = dys[S];
//
//			int X0, Y0, DX, DY;
//
//			if (PP.f[0] == id)
//			{
//				X0 = PP.dim[0] - 1;
//				Y0 = 0;
//				DX = -1;
//				DY = 0;
//			};
//
//			if (PP.f[1] == id)
//			{
//				X0 = PP.dim[0] - 1;
//				Y0 = PP.dim[1] - 1;
//				DX = 0;
//				DY = -1;
//			};
//
//			if (PP.f[2] == id)
//			{
//				X0 = 0;
//				Y0 = PP.dim[1] - 1;
//				DX = 1;
//				DY = 0;
//			};
//
//			if (PP.f[3] == id)
//			{
//				X0 = 0;
//				Y0 = 0;
//				DX = 0;
//				DY = 1;
//			};
//
//			int X = X0;
//			int Y = Y0;
//
//			int x = x0;
//			int y = y0;
//
//			const Page& e = PP.getPrimVar(name);
//
//			int CS = S * 4 + PP.f[3];
//
//			UT_Matrix2F& MM = M[CASE[CS]];
//
//			for (int i = 0; i < sl[S]; i++)
//			{
//				const UT_Vector3& src = e.get(X, Y);
//				UT_Vector3& dest = P.get(x, y);
//				dest = src;
//				X += DX; Y += DY;
//				x += dx;
//				y += dy;
//			};
//		}
//		else // MARK == -1
//		{
//			int x0 = xs[S];
//			int y0 = ys[S];
//			int dx = dxs[S];
//			int dy = dys[S];
//
//			int x = x0;
//			int y = y0;
//
//			// JUST COPY ROW
//			for (int i = 0; i < sl[S]; i++)
//			{
//				P.get(x, y) = P.get(x + ddx[S], y + ddy[S]);
//				x += dx; y += dy;
//			};
//		};
//	};
//
//	// CORNERS
//	int MNX[4] = { 0,-1,-1,0 };
//	int MNY[4] = { 0,0,-1,-1 };
//
//	int MPX[4] = { -1,-1,0,0 };
//	int MPY[4] = { 0,-1,-1,0 };
//
//	for (int i = 0; i < 4; i++)
//	{
//		UT_Vector3 O = E.get(MNX[i], MNY[i]);
//
//		// POS WINDING
//		UT_Vector3 O1 = O;
//
//		int lastid = pb.id;
//		int idx = pb.f[i];
//
//		int far = 0;
//		while (idx != -1 && idx != id && far < 2)
//		{
//			const Patch& PP = *PEBBLE[idx];
//			int ref = PP.reflect(lastid);
//
//			if (ref == -1)
//			{
//				// INCONSISTENCE!
//				break;
//			};
//
//			const Page& e = PP.getPrimVar(name);
//
//			O1 = e.get(MPX[ref], MPY[ref]);
//
//			//C.append(e.get(MPX[ref], MPY[ref]));
//
//			ref++;
//			ref %= 4;
//
//			lastid = idx;
//			idx = PP.f[ref];
//
//			far++;
//		};
//
//		// NEG WINDING
//		UT_Vector3 O2 = O;
//
//		lastid = pb.id;
//		idx = pb.f[(i + 3) % 4]; // i-1
//
//		far = 0;
//
//		while (idx != -1 && idx != id && far < 2)
//		{
//			const Patch& PP = *PEBBLE[idx];
//			int ref = PP.reflect(lastid);
//
//			if (ref == -1)
//			{
//				// INCONSISTENCE!
//				break;
//			};
//
//			const Page& e = PP.getPrimVar(name);
//
//			O2 = e.get(MNX[ref], MNY[ref]);
//
//			//C.append(e.get(MNX[ref], MNY[ref]));
//
//			ref += 3; // -1
//			ref %= 4;
//
//			lastid = idx;
//			idx = PP.f[ref];
//
//			far++;
//		};
//
//		UT_Vector3 res = (O1 + O2)*0.5; // C.size();
//
//		P.get(MNX[i], MNY[i]) = res;
//	};
//
//
//	// RETURN
//	return _P;
//};

Page* getExpandedPrimVar(const vector<Patch*>& PEBBLE, const Patch & pb, UT_String name)
{
	// MAKE
	Page* _P = new Page(pb.dim[0] + 2, pb.dim[1] + 2, true);

	Page& P = *_P;

	// ASSIGN
	const Page& E = pb.getPrimVar(name);

	// CORE
	for (int i = 0; i < pb.dim[0]; i++)
		for (int j = 0; j < pb.dim[1]; j++)
			P.get(i + 1, j + 1) = E.get(i, j);

	// SIDES

	int xs[4] = { 1,	pb.dim[0] + 1,	pb.dim[0],	0 };
	int ys[4] = { 0,	1,			pb.dim[1] + 1,	pb.dim[1] };

	int dxs[4] = { 1,0,-1,0 };
	int dys[4] = { 0,1,0,-1 };

	int sl[4] = { pb.dim[0],pb.dim[1],pb.dim[0],pb.dim[1] };

	int ddx[4] = { 0,-1,0,1 };
	int ddy[4] = { 1,0,-1,0 };

	int id = pb.id;

	for (int S = 0; S < 4; S++)
	{
		int mark = pb.f[S];
		if (mark != -1)
		{
			const Patch& PP = *PEBBLE[pb.f[S]];

			int x0 = xs[S];
			int y0 = ys[S];
			int dx = dxs[S];
			int dy = dys[S];

			int X0, Y0, DX, DY;

			if (PP.f[0] == id)
			{
				X0 = PP.dim[0] - 1;
				Y0 = 0;
				DX = -1;
				DY = 0;
			};

			if (PP.f[1] == id)
			{
				X0 = PP.dim[0] - 1;
				Y0 = PP.dim[1] - 1;
				DX = 0;
				DY = -1;
			};

			if (PP.f[2] == id)
			{
				X0 = 0;
				Y0 = PP.dim[1] - 1;
				DX = 1;
				DY = 0;
			};

			if (PP.f[3] == id)
			{
				X0 = 0;
				Y0 = 0;
				DX = 0;
				DY = 1;
			};

			int X = X0;
			int Y = Y0;

			int x = x0;
			int y = y0;

			const Page& e = PP.getPrimVar(name);

			for (int i = 0; i < sl[S]; i++)
			{
				const UT_Vector3& src = e.get(X, Y);
				UT_Vector3& dest = P.get(x, y);
				dest = src;
				X += DX; Y += DY;
				x += dx;
				y += dy;
			};
		}
		else // MARK == -1
		{
			int x0 = xs[S];
			int y0 = ys[S];
			int dx = dxs[S];
			int dy = dys[S];

			int x = x0;
			int y = y0;

			// JUST COPY ROW
			for (int i = 0; i < sl[S]; i++)
			{
				P.get(x, y) = P.get(x + ddx[S], y + ddy[S]);
				x += dx; y += dy;
			};
		};
	};

	// CORNERS
	int MNX[4] = { 0,-1,-1,0 };
	int MNY[4] = { 0,0,-1,-1 };

	int MPX[4] = { -1,-1,0,0 };
	int MPY[4] = { 0,-1,-1,0 };

	for (int i = 0; i < 4; i++)
	{
		UT_Vector3 O = E.get(MNX[i], MNY[i]);

		// POS WINDING
		UT_Vector3 O1 = O;

		int lastid = pb.id;
		int idx = pb.f[i];

		int far = 0;
		while (idx != -1 && idx != id && far < 2)
		{
			const Patch& PP = *PEBBLE[idx];
			int ref = PP.reflect(lastid);

			if (ref == -1)
			{
				// INCONSISTENCE!
				break;
			};

			const Page& e = PP.getPrimVar(name);

			O1 = e.get(MPX[ref], MPY[ref]);

			ref++;
			ref %= 4;

			lastid = idx;
			idx = PP.f[ref];

			far++;
		};

		// NEG WINDING
		UT_Vector3 O2 = O;

		lastid = pb.id;
		idx = pb.f[(i + 3) % 4]; // i-1

		far = 0;

		while (idx != -1 && idx != id && far < 2)
		{
			const Patch& PP = *PEBBLE[idx];
			int ref = PP.reflect(lastid);

			if (ref == -1)
			{
				// INCONSISTENCE!
				break;
			};

			const Page& e = PP.getPrimVar(name);

			O2 = e.get(MNX[ref], MNY[ref]);

			ref += 3; // -1
			ref %= 4;

			lastid = idx;
			idx = PP.f[ref];

			far++;
		};

		UT_Vector3 res = (O1 + O2)*0.5;

		P.get(MNX[i], MNY[i]) = res;
	};


	// RETURN
	return _P;
};

bool getExpandedPrimVarDiv(const vector<Patch*>& PEBBLE, const Patch & pb, Page* DPDU, Page* DPDV)
{
	float A[4] = { M_PI_2, M_PI, M_PI+M_PI_2, 0};

	const Page& dPdu = pb.getPrimVar("dPdu");
	const Page& dPdv = pb.getPrimVar("dPdv");
	//const Page& N = pb.getPrimVar("N");

	// CORE
	for (int i = 0; i < pb.dim[0]; i++)
		for (int j = 0; j < pb.dim[1]; j++)
		{
			DPDU->get(i + 1, j + 1) = dPdu.get(i, j);
			DPDV->get(i + 1, j + 1) = dPdv.get(i, j);
		};

	// SIDES
	int xs[4] = { 1,	pb.dim[0] + 1,	pb.dim[0],	0 };
	int ys[4] = { 0,	1,			pb.dim[1] + 1,	pb.dim[1] };

	int dxs[4] = { 1,0,-1,0 };
	int dys[4] = { 0,1,0,-1 };

	int sl[4] = { pb.dim[0],pb.dim[1],pb.dim[0],pb.dim[1] };

	int ddx[4] = { 0,-1,0,1 };
	int ddy[4] = { 1,0,-1,0 };

	int id = pb.id;

	for (int S = 0; S < 4; S++)
	{
		int mark = pb.f[S];
		if (mark != -1)
		{
			const Patch& PP = *PEBBLE[pb.f[S]];

			int x0 = xs[S];
			int y0 = ys[S];
			int dx = dxs[S];
			int dy = dys[S];

			int X0, Y0, DX, DY;

			if (PP.f[0] == id)
			{
				X0 = PP.dim[0] - 1;
				Y0 = 0;
				DX = -1;
				DY = 0;
			};

			if (PP.f[1] == id)
			{
				X0 = PP.dim[0] - 1;
				Y0 = PP.dim[1] - 1;
				DX = 0;
				DY = -1;
			};

			if (PP.f[2] == id)
			{
				X0 = 0;
				Y0 = PP.dim[1] - 1;
				DX = 1;
				DY = 0;
			};

			if (PP.f[3] == id)
			{
				X0 = 0;
				Y0 = 0;
				DX = 0;
				DY = 1;
			};

			int X = X0;
			int Y = Y0;

			int x = x0;
			int y = y0;

			const Page& dPdu = PP.getPrimVar("dPdu");
			const Page& dPdv = PP.getPrimVar("dPdv");
			const Page& N = PP.getPrimVar("N");

			int CS = S * 4 + PP.reflect(id);
			float a = A[CASE[CS]];

			UT_QuaternionF Q;

			for (int i = 0; i < sl[S]; i++)
			{
				UT_Vector3 dpdu = dPdu.get(X, Y);
				UT_Vector3 dpdv = dPdv.get(X, Y);

				UT_Vector3 n = N.get(X, Y);

				Q.updateFromAngleAxis(a, n);

				DPDU->get(x, y) = Q.rotate(dpdu);;
				DPDV->get(x, y) = Q.rotate(dpdv);

				X += DX; Y += DY;
				x += dx;
				y += dy;
			};
		}
		else // MARK == -1
		{
			int x0 = xs[S];
			int y0 = ys[S];
			int dx = dxs[S];
			int dy = dys[S];

			int x = x0;
			int y = y0;

			// JUST COPY ROW
			for (int i = 0; i < sl[S]; i++)
			{
				DPDU->get(x, y) = DPDU->get(x + ddx[S], y + ddy[S]);
				DPDV->get(x, y) = DPDV->get(x + ddx[S], y + ddy[S]);
				x += dx; y += dy;
			};
		};
	};

	// CORNERS
	int MNX[4] = { 0,-1,-1,0 };
	int MNY[4] = { 0,0,-1,-1 };

	int MPX[4] = { -1,-1,0,0 };
	int MPY[4] = { 0,-1,-1,0 };

	UT_QuaternionF Q;

	for (int i = 0; i < 4; i++)
	{
		UT_Vector3 dpdu = dPdu.get(MNX[i], MNY[i]);
		UT_Vector3 dpdv = dPdv.get(MNX[i], MNY[i]);

		UT_Vector3 OU = dpdu;
		UT_Vector3 OV = dpdv;

		// POS WINDING
		UT_Vector3 O1U = OU;
		UT_Vector3 O1V = OV;

		int lastid = pb.id;
		int idx = pb.f[i];

		int lastref = i;

		int far = 0;
		while (idx != -1 && idx != id && far < 2)
		{
			const Patch& PP = *PEBBLE[idx];
			int ref = PP.reflect(lastid);

			if (ref == -1)
			{
				// INCONSISTENCE!
				break;
			};

			int CS = lastref * 4 + ref;

			float a  = A[CASE[CS]];

			//const Page& e = PP.getPrimVar(name);

			UT_Vector3 dpdu = PP.getPrimVar("dPdu").get(MPX[ref], MPY[ref]);
			UT_Vector3 dpdv = PP.getPrimVar("dPdv").get(MPX[ref], MPY[ref]);
			UT_Vector3 n = PP.getPrimVar("N").get(MPX[ref], MPY[ref]);

			Q.updateFromAngleAxis(a, n);

			O1U = Q.rotate(dpdu);
			O1V = Q.rotate(dpdv);

			ref++;
			ref %= 4;

			lastref = ref;

			lastid = idx;
			idx = PP.f[ref];

			far++;
		};

		// NEG WINDING
		UT_Vector3 O2U = OU;
		UT_Vector3 O2V = OV;

		lastid = pb.id;
		idx = pb.f[(i + 3) % 4]; // i-1

		far = 0;

		lastref = i;

		while (idx != -1 && idx != id && far < 2)
		{
			const Patch& PP = *PEBBLE[idx];
			int ref = PP.reflect(lastid);

			if (ref == -1)
			{
				// INCONSISTENCE!
				break;
			};

			int CS = lastref * 4 + ref;

			float a = A[CASE[CS]];

			//const Page& e = PP.getPrimVar(name);

			UT_Vector3 dpdu = PP.getPrimVar("dPdu").get(MNX[ref], MNY[ref]);
			UT_Vector3 dpdv = PP.getPrimVar("dPdv").get(MNX[ref], MNY[ref]);
			UT_Vector3 n = PP.getPrimVar("N").get(MNX[ref], MNY[ref]);

			Q.updateFromAngleAxis(a, n);

			O2U = Q.rotate(dpdu);
			O2V = Q.rotate(dpdv);

			ref += 3; // -1
			ref %= 4;

			lastref = ref;

			lastid = idx;
			idx = PP.f[ref];

			far++;
		};

		UT_Vector3 resU = (O1U + O2U)*0.5; resU.normalize();
		UT_Vector3 resV = (O1V + O2V)*0.5; resV.normalize();

		DPDU->get(MNX[i], MNY[i]) = resU; 
		DPDV->get(MNX[i], MNY[i]) = resV;
	};

	return true;
}

Page* getExpandedPrimVarProjected(const vector<Patch*>& PEBBLE, const Patch & pb, UT_String name)
{
	// MAKE
	Page* _P = new Page(pb.dim[0] + 2, pb.dim[1] + 2, true);

	Page& P = *_P;

	P.m_projected = true;

	// ASSIGN
	UT_Matrix2FArray M(4);
	M[0] = UT_Matrix2F(0, -1, 1, 0);
	M[1] = UT_Matrix2F(-1, 0, 0, -1);
	M[2] = UT_Matrix2F(0, 1, -1, 0);
	M[3] = UT_Matrix2F(1);

	const Page& E = pb.getPrimVar(name);

	const Page& dPdu = pb.getPrimVar("dPdu");
	const Page& dPdv = pb.getPrimVar("dPdv");
	const Page& N = pb.getPrimVar("N");

	// CORE
	for (int i = 0; i < pb.dim[0]; i++)
		for (int j = 0; j < pb.dim[1]; j++)
		{
			UT_Vector3 dpdu = dPdu.get(i, j);
			UT_Vector3 dpdv = dPdv.get(i, j);
			UT_Vector3 n = N.get(i, j);

			UT_Vector3 v = E.get(i, j);

			float nproj = n.dot(v);

			v -= n*nproj;
			float lv = v.length();
			v.normalize();

			UT_Vector3 e(0,0,nproj);

			float detxy = dpdu[0] * dpdv[1] - dpdu[1] * dpdv[0];
			if (detxy != 0)
			{
				float dU = v[0] * dpdv[1] - v[1] * dpdv[0];
				float dV = dpdu[0] * v[1] - dpdu[1] * v[0];
				e[0] = lv*dU / detxy; e[1] = lv*dV / detxy;
			};

			P.get(i + 1, j + 1) = e;
		};

	// SIDES

	int xs[4] = { 1,	pb.dim[0] + 1,	pb.dim[0],	0 };
	int ys[4] = { 0,	1,			pb.dim[1] + 1,	pb.dim[1] };

	int dxs[4] = { 1,0,-1,0 };
	int dys[4] = { 0,1,0,-1 };

	int sl[4] = { pb.dim[0],pb.dim[1],pb.dim[0],pb.dim[1] };

	int ddx[4] = { 0,-1,0,1 };
	int ddy[4] = { 1,0,-1,0 };

	int id = pb.id;

	for (int S = 0; S < 4; S++)
	{
		int mark = pb.f[S];
		if (mark != -1)
		{
			const Patch& PP = *PEBBLE[pb.f[S]];

			int x0 = xs[S];
			int y0 = ys[S];
			int dx = dxs[S];
			int dy = dys[S];

			int X0, Y0, DX, DY;

			if (PP.f[0] == id)
			{
				X0 = PP.dim[0] - 1;
				Y0 = 0;
				DX = -1;
				DY = 0;
			};

			if (PP.f[1] == id)
			{
				X0 = PP.dim[0] - 1;
				Y0 = PP.dim[1] - 1;
				DX = 0;
				DY = -1;
			};

			if (PP.f[2] == id)
			{
				X0 = 0;
				Y0 = PP.dim[1] - 1;
				DX = 1;
				DY = 0;
			};

			if (PP.f[3] == id)
			{
				X0 = 0;
				Y0 = 0;
				DX = 0;
				DY = 1;
			};

			int X = X0;
			int Y = Y0;

			int x = x0;
			int y = y0;


			const Page& E = PP.getPrimVar(name);

			const Page& dPdu = PP.getPrimVar("dPdu");
			const Page& dPdv = PP.getPrimVar("dPdv");
			const Page& N = PP.getPrimVar("N");

			//const Page& e = PP.getPrimVar(name);

			int CS = S * 4 + PP.reflect(id);

			UT_Matrix2F& MM = M[CASE[CS]];

			for (int i = 0; i < sl[S]; i++)
			{
				//const UT_Vector3& src = E.get(X, Y);

				UT_Vector3 dpdu = dPdu.get(X, Y);
				UT_Vector3 dpdv = dPdv.get(X, Y);
				UT_Vector3 n = N.get(X, Y);

				UT_Vector3 v = E.get(X, Y);

				float nproj = n.dot(v);

				v -= n*nproj;
				float lv = v.length();
				v.normalize();

				UT_Vector2 d(0, 0); 

				float detxy = dpdu[0] * dpdv[1] - dpdu[1] * dpdv[0];
				if (detxy != 0)
				{
					float dU = v[0] * dpdv[1] - v[1] * dpdv[0];
					float dV = dpdu[0] * v[1] - dpdu[1] * v[0];
					d[0] = dU / detxy; d[1] = dV / detxy;
				};

				d = d*MM;

				UT_Vector3 e(d[0], d[1], nproj);
				P.get(x, y) = e;

				X += DX; Y += DY;
				x += dx;
				y += dy;
			};
		}
		else // MARK == -1
		{
			int x0 = xs[S];
			int y0 = ys[S];
			int dx = dxs[S];
			int dy = dys[S];

			int x = x0;
			int y = y0;

			// JUST COPY ROW
			for (int i = 0; i < sl[S]; i++)
			{
				P.get(x, y) = P.get(x + ddx[S], y + ddy[S]);
				x += dx; y += dy;
			};
		};
	};

	// CORNERS
	int MNX[4] = { 0,-1,-1,0 };
	int MNY[4] = { 0,0,-1,-1 };

	int MPX[4] = { -1,-1,0,0 };
	int MPY[4] = { 0,-1,-1,0 };

	for (int i = 0; i < 4; i++)
	{
		UT_Vector3 dpdu = dPdu.get(MNX[i], MNY[i]);
		UT_Vector3 dpdv = dPdv.get(MNX[i], MNY[i]);
		UT_Vector3 n = N.get(MNX[i], MNY[i]);

		UT_Vector3 v = E.get(MNX[i], MNY[i]);

		float nproj = n.dot(v);

		v -= n*nproj;
		float lv = v.length();
		v.normalize();

		UT_Vector3 e(0, 0, nproj);

		float detxy = dpdu[0] * dpdv[1] - dpdu[1] * dpdv[0];
		if (detxy != 0)
		{
			float dU = v[0] * dpdv[1] - v[1] * dpdv[0];
			float dV = dpdu[0] * v[1] - dpdu[1] * v[0];
			e[0] = lv*dU / detxy; e[1] = lv*dV / detxy;
		};

		UT_Vector3 O = e; // E.get(MNX[i], MNY[i]);

		// POS WINDING
		UT_Vector3 O1 = O;

		int lastid = pb.id;
		int idx = pb.f[i];

		int lastref = i;

		int far = 0;
		while (idx != -1 && idx != id && far < 2)
		{
			const Patch& PP = *PEBBLE[idx];
			int ref = PP.reflect(lastid);

			if (ref == -1)
			{
				// INCONSISTENCE!
				break;
			};

			int CS = lastref * 4 + ref;

			UT_Matrix2F& MM = M[CASE[CS]];

			//const Page& e = PP.getPrimVar(name);

			UT_Vector3 dpdu = dPdu.get(MPX[ref], MPY[ref]);
			UT_Vector3 dpdv = dPdv.get(MPX[ref], MPY[ref]);
			UT_Vector3 n = N.get(MPX[ref], MPY[ref]);

			UT_Vector3 v = E.get(MPX[ref], MPY[ref]);

			float nproj = n.dot(v);

			v -= n*nproj;
			float lv = v.length();
			v.normalize();

			UT_Vector2 d(0, 0);

			float detxy = dpdu[0] * dpdv[1] - dpdu[1] * dpdv[0];
			if (detxy != 0)
			{
				float dU = v[0] * dpdv[1] - v[1] * dpdv[0];
				float dV = dpdu[0] * v[1] - dpdu[1] * v[0];
				d[0] = dU / detxy; d[1] = dV / detxy;
			};

			d = d*MM;

			UT_Vector3 e(d[0], d[1], nproj);

			O1 = e; // .get(MPX[ref], MPY[ref]);

			ref++;
			ref %= 4;

			lastref = ref;

			lastid = idx;
			idx = PP.f[ref];

			far++;
		};

		// NEG WINDING
		UT_Vector3 O2 = O;

		lastid = pb.id;
		idx = pb.f[(i + 3) % 4]; // i-1

		far = 0;

		lastref = i;

		while (idx != -1 && idx != id && far < 2)
		{
			const Patch& PP = *PEBBLE[idx];
			int ref = PP.reflect(lastid);

			if (ref == -1)
			{
				// INCONSISTENCE!
				break;
			};

			int CS = lastref * 4 + ref;

			UT_Matrix2F& MM = M[CASE[CS]];

			//const Page& e = PP.getPrimVar(name);

			UT_Vector3 dpdu = dPdu.get(MNX[ref], MNY[ref]);
			UT_Vector3 dpdv = dPdv.get(MNX[ref], MNY[ref]);
			UT_Vector3 n = N.get(MNX[ref], MNY[ref]);

			UT_Vector3 v = E.get(MNX[ref], MNY[ref]);

			float nproj = n.dot(v);

			v -= n*nproj;
			float lv = v.length();
			v.normalize();

			UT_Vector2 d(0, 0);

			float detxy = dpdu[0] * dpdv[1] - dpdu[1] * dpdv[0];
			if (detxy != 0)
			{
				float dU = v[0] * dpdv[1] - v[1] * dpdv[0];
				float dV = dpdu[0] * v[1] - dpdu[1] * v[0];
				d[0] = dU / detxy; d[1] = dV / detxy;
			};

			d = d*MM;

			UT_Vector3 e(d[0], d[1], nproj);


			//const Page& e = PP.getPrimVar(name);

			O2 = e; // .get(MNX[ref], MNY[ref]);

			ref += 3; // -1
			ref %= 4;

			lastref = ref;

			lastid = idx;
			idx = PP.f[ref];

			far++;
		};

		UT_Vector3 res = (O1 + O2)*0.5;

		P.get(MNX[i], MNY[i]) = res;
	};


	// RETURN
	return _P;
};

bool declareEntity(vector<Patch*>& PEBBLE, UT_String name)
{
	for (Patch* _pb : PEBBLE) _pb->declarePage(name);
	return true;
};

bool applyProxy(vector<Patch*>& PEBBLE, UT_String name)
{
	UT_String nn = "_proxy_";
	nn += name;

	for (Patch* _pb : PEBBLE)
	{
		Patch& pb = *_pb;

		int n = pb.chs.find(nn);
		if (n == -1) continue;

		// APPLY 
		Page& DEST = pb.getPrimVar(name);
		Page& SRC = pb.getPrimVar(nn);

		DEST.apply(SRC);

		// KILL
		if (n == pb.chs.size() - 1) // LAST?
		{
			pb.killLast();
		}
		else // NO
		{
			pb.kill(n);
		};
	};

	return true;
};