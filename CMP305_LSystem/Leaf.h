#pragma once
#include "SphereMesh.h"
class Leaf : public SphereMesh
{
public:
	Leaf(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int lresolution = 20) : SphereMesh(device, deviceContext, lresolution),
		m_Transform(XMMatrixIdentity())
	{
	}
	~Leaf() { SphereMesh::~SphereMesh(); }
	XMMATRIX m_Transform;

private:

};

