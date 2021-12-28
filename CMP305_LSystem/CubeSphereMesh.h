#pragma once
#include "BaseMesh.h"

class CubeSphereMesh : public BaseMesh
{
public:
	CubeSphereMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, unsigned resolution = 20u, float radius = 1.f);
	~CubeSphereMesh();

private:
	void initBuffers(ID3D11Device* device);
	unsigned resolution_;
	float radius_;
};

