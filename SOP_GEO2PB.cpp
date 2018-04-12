///////////////////////////////////////////////////
// SOP GEO2PB Converter

#include "include.h"

#include "osd.h"

#include "SOP_GEO2PB.h"

extern const char* ch_base_names[];

extern PRM_Name	 theDetailName;
extern PRM_Range theDetailRng;

int* SOP_GEO2PB::myIndirect = NULL;

void SOP_GEO2PB::buildPartial(int nsamples, const UT_JobInfo &info)
{
	int start, end;
	info.divideWork(m_primcnt, start, end);

	float pWeights[20], dsWeights[20], dtWeights[20];

	for (int face = start; face < end; face++)
	{
		Patch pb(nsamples, nsamples, m_baseNames);
		pb.id = face;

		Page& UVF = pb.getPrimVar("uvw");
		Page& P = pb.getPrimVar("P");
		Page& DPDU = pb.getPrimVar("dPdu");
		Page& DPDV = pb.getPrimVar("dPdv");
		Page& N = pb.getPrimVar("N");
		Page& CD = pb.getPrimVar("Cd");

		Page& G = pb.getPrimVar("g");
		Page& GG = pb.getPrimVar("G");

		for (int x = 0; x < nsamples; x++)
		{
			for (int y = 0; y < nsamples; y++)
			{
				// 0 : UVF
				UT_Vector3 _UVF = m_uvf->get(x, y);
				_UVF[2] = face;
				UVF.get(x, y) = _UVF;

				// COMPUTE FRAME

				// Locate the patch corresponding to the face ptex idx and (s,t)
				Far::PatchTable::PatchHandle const * handle = m_patchmap->FindPatch(face, _UVF[0], _UVF[1]);

				// Evaluate the patch weights, identify the CVs and compute the limit frame:
				m_patchTable->EvaluateBasis(*handle, _UVF[0], _UVF[1], pWeights, dsWeights, dtWeights);

				Far::ConstIndexArray cvs = m_patchTable->GetPatchVertices(*handle);

				LimitFrame PLF, CLF;

				for (int cv = 0; cv < cvs.size(); ++cv)
				{
					PLF.AddWithWeight(m_vertsP[cvs[cv]], pWeights[cv], dsWeights[cv], dtWeights[cv]);
					CLF.AddWithWeight(m_vertsCd[cvs[cv]], pWeights[cv], dsWeights[cv], dtWeights[cv]);
				}

				// 1 : P
				float const * pos = PLF._position;
				P.get(x, y) = UT_Vector3(pos);

				// 2 : dPdu
				float const * tan1 = PLF.deriv1;
				UT_Vector3 dPdu = UT_Vector3(tan1);
				DPDU.get(x, y) = dPdu;

				// 3 : dPdv
				float const * tan2 = PLF.deriv2;
				UT_Vector3 dPdv = UT_Vector3(tan2);
				DPDV.get(x, y) = dPdv;

				// 4 : N
				UT_Vector3 _N = dPdu;
				_N.cross(dPdv);
				_N.normalize();
				N.get(x, y) = _N;

				// 5 : Cd
				float const * cd = CLF._position;
				CD.get(x, y) = UT_Vector3(cd);

				// 6 : g
				UT_Vector3 g = UT_Vector3(
					dPdu.dot(dPdu),
					dPdv.dot(dPdv),
					dPdu.dot(dPdv)
				);
				G.get(x, y) = g;

				// 7 : G
				UT_Vector3 gg = UT_Vector3(
					sqrt(g[0] * g[1] - g[2] * g[2]),
					0.5*(1.0 / (pb.dim[0] - 1) + 1.0 / (pb.dim[1] - 1)),
					0);
				gg[2] = (dPdu + dPdv).length()*gg[1] / 1.44;
				GG.get(x, y) = gg;
			};
		};

		// TOPOLOGY RESOLVE

		vector<int32>& pp = m_edgemap[face];
		//pp.reverse();

		UT_Vector2Array emaps;

		for (int i = 0; i < 4; i++)
		{
			bool found = false;
			for (int j = 0; j < m_primcnt; j++)
			{
				// SAME FACE?
				if (j == face) continue;

				const vector<int32>& _pp = m_edgemap[j];

				for (int k = 1; k < 5; k++)
				{
					if (_pp[k] == pp[i])
					{
						// DIFFERENT EDGE?
						if (_pp[k - 1] != pp[i + 1]) continue;

						// FOUND!
						pb.f[i] = j;
						found = true;
						break;
					};
				};

				if (found) break;
			};

			if (!found) pb.f[i] = -1;
		};

		// CONSTRUCT GEO
		UT_AutoJobInfoLock a(info);
		pb2geo(gdp, pb);
	};
};

OP_ERROR SOP_GEO2PB::cookMySop(OP_Context &context)
{
	gdp->clearAndDestroy();

	//return error();

	if (lockInputs(context) >= UT_ERROR_ABORT) return error();

	m_input = inputGeo(0, context);

	int detail = evalInt(0, 0, context.getTime());

	/////////////////////////////////////////////////////

	m_primcnt = 0;

	const GA_Primitive* prim = NULL;
	GA_FOR_ALL_PRIMITIVES(m_input, prim)
	{
		const GU_PrimPoly* P = reinterpret_cast<const GU_PrimPoly*>(prim);

		if (P == NULL)
		{
			return error();
		};

		if (P->getVertexCount() != 4)
		{
			return  error();
		};

		if (!P->isClosed())
		{
			return  error();
		};

		m_primcnt++;
	};

	// OSD
	Sdc::SchemeType type = OpenSubdiv::Sdc::SCHEME_CATMARK;

	Sdc::Options options;
	options.SetVtxBoundaryInterpolation(Sdc::Options::VTX_BOUNDARY_EDGE_AND_CORNER); //VTX_BOUNDARY_EDGE_ONLY

	Far::TopologyDescriptor desc;
	desc.numVertices = m_input->getNumPoints();
	desc.numFaces = m_primcnt;
	int* F = new int[m_primcnt];
	for (int i = 0; i < m_primcnt; i++) F[i] = 4;
	desc.numVertsPerFace = F;
	int* FI = new int[m_primcnt * 4];
	for (int i = 0; i < m_primcnt; i++)
	{
		GA_Offset v0, v1, v2, v3;
		prim = m_input->getPrimitiveByIndex(i);

		for (int j = 0; j < 4; j++)
		{
			GA_Offset vtx = prim->getVertexOffset(j);
			GA_Offset poff = m_input->vertexPoint(vtx);

			FI[i * 4 + 3 - j] = m_input->pointIndex(poff);
		};
	};
	desc.vertIndicesPerFace = FI;

	// Instantiate a FarTopologyRefiner from the descriptor
	Far::TopologyRefiner* refiner = Far::TopologyRefinerFactory<Far::TopologyDescriptor>::Create(desc,
		Far::TopologyRefinerFactory<Far::TopologyDescriptor>::Options(type, options));

	// Adaptively refine the topology with an isolation level
	refiner->RefineAdaptive(Far::TopologyRefiner::AdaptiveOptions(detail));

	// Generate a set of Far::PatchTable that we will use to evaluate the
	// surface limit
	Far::PatchTableFactory::Options patchOptions;
	patchOptions.endCapType = Far::PatchTableFactory::Options::ENDCAP_GREGORY_BASIS;

	m_patchTable = Far::PatchTableFactory::Create(*refiner, patchOptions);

	// Compute the total number of points we need to evaluate patchtable.
	// we use local points around extraordinary features.
	int nRefinerVertices = refiner->GetNumVerticesTotal();
	int nLocalPoints = m_patchTable->GetNumLocalPoints();

	//// Create a buffer to hold the position of the refined verts and
	//// local points, then copy the coarse positions at the beginning.
	m_vertsP.resize(nRefinerVertices + nLocalPoints);
	m_vertsCd.resize(nRefinerVertices + nLocalPoints);

	//// prepare buffers
	//int nCoarseVerts = ptcnt;

	GA_ROHandleV3 Cd(m_input, GA_ATTRIB_POINT, "Cd");

	// Initialize coarse mesh positions
	for (int i = 0; i<m_input->getNumPoints(); i++)
	{
		GA_Offset poff = m_input->pointOffset(i);

		UT_Vector3 V = m_input->getPos3(poff);

		m_vertsP[i].SetPoint(V.x(), V.y(), V.z());
		UT_Vector3 C(1.0, 1.0, 1.0);
		if (Cd.isValid()) C = Cd.get(poff);

		m_vertsCd[i].SetPoint(C.x(), C.y(), C.z());
	};

	// Adaptive refinement may result in fewer levels than maxIsolation.
	int nRefinedLevels = refiner->GetNumLevels();
	Far::PrimvarRefiner primvarRefiner(*refiner);

	// Interpolate vertex primvar data : they are the control vertices
	// of the limit patches (see far_tutorial_0 for details)
	VertexPosition * srcP = &m_vertsP[0];
	VertexColor * srcCd = &m_vertsCd[0];
	for (int level = 1; level < nRefinedLevels; ++level)
	{
		VertexPosition * dstP = srcP + refiner->GetLevel(level - 1).GetNumVertices();
		VertexColor * dstCd = srcCd + refiner->GetLevel(level - 1).GetNumVertices();
		primvarRefiner.Interpolate(level, srcP, dstP);
		primvarRefiner.InterpolateVarying(level, srcCd, dstCd);
		srcP = dstP;
		srcCd = dstCd;
	};

	// Evaluate local points from interpolated vertex primvars.
	m_patchTable->ComputeLocalPointValues(&m_vertsP[0], &m_vertsP[nRefinerVertices]);
	m_patchTable->ComputeLocalPointValues(&m_vertsCd[0], &m_vertsCd[nRefinerVertices]);

	// Create a Far::PatchMap to help locating patches in the table
	m_patchmap = new Far::PatchMap(*m_patchTable);

	// Create a Far::PtexIndices to help find indices of ptex faces.
	Far::PtexIndices ptexIndices(*refiner);

	// Generate samples on each ptex face

	int nsamples = 1;
	for (int i = 0; i < detail; i++) nsamples <<= 1;

	float d = 0.5 / nsamples;
	float dd = 1.0 / nsamples;

	m_uvf = new Page(nsamples, nsamples);

	Page& uvf = *m_uvf;

	float x = d;

	for (int i = 0; i < nsamples; i++)
	{
		float y = d;

		for (int j = 0; j < nsamples; j++)
		{
			uvf.get(i, j) = UT_Vector3(x, y, 0);
			y += dd;
		};

		x += dd;
	};

	// BASE CHANNELS
	m_baseNames.clear();
	{
		int chi = 0;
		while (ch_base_names[chi] != NULL)
		{
			m_baseNames.append(ch_base_names[chi]);
			chi++;
		};
	};


	// EDGEMAP
	m_edgemap.resize(m_primcnt);

	for (int i = 0; i<m_primcnt; i++)
	{
		m_edgemap[i].resize(5);

		for (int j = 0; j < 5; j++)
		{
			m_edgemap[i][j] = FI[4 * i + (j % 4)];
		};
	};

	//m_pebble->m_P.resize(m_primcnt);

	build(nsamples);

	// EDGES
	const GA_EdgeGroupTable& egt = m_input->edgeGroups();
	m_glist.clear();
	egt.getList(m_glist);

	map<string, vector< UT_Vector2i> > m_edges;

	for (int i = 0; i < m_glist.entries(); i++)
	{
		const GA_EdgeGroup* eg = m_glist[i];

		string name = eg->getName();

		auto it = eg->begin();

		vector<UT_Vector2i> V;

		do
		{
			const GA_Edge& e = it.getEdge();
			const GA_Offset prm = it.getPrimitive();

			int p0 = m_input->pointIndex(e.p0());
			int p1 = m_input->pointIndex(e.p1());

			bool found = false;
			for (int prm = 0; prm < m_primcnt; prm++)
			{
				vector<int>& EM = m_edgemap[prm];

				for (int j = 0; j < 4; j++)
				{
					// +
					if (EM[j] == p0 && EM[j + 1] == p1)
					{
						found = true;
						V.push_back(UT_Vector2i(prm, j));
						break;
					};

					// -
					if (EM[j] == p1 && EM[j + 1] == p0)
					{
						found = true;
						V.push_back(UT_Vector2i(prm, j));
						break;
					};
				};
			};

			if (!found)
			{
				// FAIL!
				int K = 0;
			};

			++it;
		} while (it != eg->end());

		m_edges[name] = V;
	};

	edges2geo(gdp, m_edges);

	// CLEANUP
	delete m_patchmap;
	delete m_uvf;

	delete refiner;

	delete[] F;
	delete[] FI;

	////////////////////////////////////////////////////


	unlockInput(0);

	return error();
};

PRM_Template templateList[] = {
	PRM_Template(PRM_INT_J,			1, &theDetailName, PRMtwoDefaults, NULL, &theDetailRng),
	PRM_Template()
};

OP_GEO2PBOperator::OP_GEO2PBOperator() : OP_Operator("geo2pb", "Prepare PB", creator, templateList, 1, 1) {};
