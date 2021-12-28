#pragma once
#include "BaseMesh.h"
#include "ImprovedNoise.h"

class CubeSphereMesh : public BaseMesh
{
public:
	CubeSphereMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, unsigned resolution = 20u, float radius = 1.f,
		float noise_frequency = 0.f, float noise_amplitude = 0.f, XMFLOAT3 noise_center = XMFLOAT3(0.f, 0.f, 0.f));
	~CubeSphereMesh();

	void Regenrate(ID3D11Device* device, unsigned resolution = 20u, float radius = 1.f, float noise_frequency = 0.f, float noise_amplitude = 0.f,
		XMFLOAT3 noise_center = XMFLOAT3(0.f, 0.f, 0.f));

private:
	void initBuffers(ID3D11Device* device);
	unsigned resolution_;
	float radius_;
	float noise_frequency_, noise_amplitude_;
	XMFLOAT3 noise_center_;
};

