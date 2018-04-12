#include "include.h"

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
	//vector<Page*>& dPdUs, vector<Page*>& dPdVs, vector<Page*>& Ns,
	vector<Page*>& Rels,
	UT_Lock& lock, traceStat& stat)
{
	if (stat.max_deep < 0)
	{
		stat.state = 1;
		return false;
	};

	UT_Matrix2FArray M;
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
		D.normalize();

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