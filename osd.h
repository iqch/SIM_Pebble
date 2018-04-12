#pragma once

// OSD

#include <opensubdiv/far/topologyDescriptor.h>
#include <opensubdiv/far/primvarRefiner.h>
#include <opensubdiv/far/patchTableFactory.h>
#include <opensubdiv/far/patchMap.h>
#include <opensubdiv/far/ptexIndices.h>

using namespace OpenSubdiv;

/// OSD VERTEX

struct osdVertex
{

	// Minimal required interface ----------------------
	osdVertex() {};

	osdVertex(osdVertex const & src)
	{
		_position[0] = src._position[0];
		_position[1] = src._position[1];
		_position[2] = src._position[2];
	};

	void Clear(void * = 0) { _position[0] = _position[1] = _position[2] = 0.0f; };

	void AddWithWeight(osdVertex const & src, float weight)
	{
		_position[0] += weight*src._position[0];
		_position[1] += weight*src._position[1];
		_position[2] += weight*src._position[2];
	};

	// Public interface ------------------------------------
	void SetPoint(float x, float y, float z) { _position[0] = x; _position[1] = y; _position[2] = z; };

	const float * GetPoint() const { return _position; };

	float _position[3];
};

typedef osdVertex VertexPosition;
typedef osdVertex VertexColor;

//------------------------------------------------------------------------------
// Limit frame container implementation -- this interface is not strictly
// required but follows a similar pattern to Vertex.
//
struct LimitFrame {

	void Clear(void * = 0)
	{
		_position[0] = _position[1] = _position[2] = 0.0f;
		deriv1[0] = deriv1[1] = deriv1[2] = 0.0f;
		deriv2[0] = deriv2[1] = deriv2[2] = 0.0f;
	}

	void AddWithWeight(osdVertex const & src,
		float weight, float d1Weight, float d2Weight)
	{

		_position[0] += weight * src._position[0];
		_position[1] += weight * src._position[1];
		_position[2] += weight * src._position[2];

		deriv1[0] += d1Weight * src._position[0];
		deriv1[1] += d1Weight * src._position[1];
		deriv1[2] += d1Weight * src._position[2];

		deriv2[0] += d2Weight * src._position[0];
		deriv2[1] += d2Weight * src._position[1];
		deriv2[2] += d2Weight * src._position[2];
	}

	float _position[3],
		deriv1[3],
		deriv2[3];

	LimitFrame() { Clear(); };
};