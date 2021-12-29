#pragma once
#include "BaseMesh.h"
#include "ImprovedNoise.h"
#include <vector>
#include <memory>
//Noise settings - A collection of settings for each layer instanciated
struct NoiseLayerSettings {
	NoiseLayerSettings() {
		layer_active_ = true;
		layer_use_previous_layer_as_mask_ = false;
		noise_base_frequency_ = 0.f;
		noise_base_amplitude_ = 0.f;
		noise_min_threshold_ = 0.f;
		layer_roughness_ = 0.f;
		layer_persistence_ = 0.f;
		layer_nSubLayers_ = 1;
		layer_center_ = XMFLOAT3(0.f, 0.f, 0.f);
	}
	NoiseLayerSettings(const float& r, const float& p, const unsigned& n, const XMFLOAT3& c, const float& basef, const float& basea, const float& t){
		layer_active_ = true;
		layer_use_previous_layer_as_mask_ = false;
		layer_roughness_ = r;
		layer_persistence_ = p;
		layer_nSubLayers_ = n;
		layer_center_ = c;
		noise_base_frequency_ = basef;
		noise_base_amplitude_ = basea;
		noise_min_threshold_ = t;
	}
	bool layer_active_, layer_use_previous_layer_as_mask_;
	float noise_base_frequency_, noise_base_amplitude_;
	float noise_min_threshold_;
	float layer_roughness_, layer_persistence_;
	unsigned layer_nSubLayers_;
	XMFLOAT3 layer_center_;
};

class CubeSphereMesh : public BaseMesh
{
public:
	CubeSphereMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, unsigned resolution = 20u, float radius = 1.f,
		float noise_frequency = 0.f, float noise_amplitude = 0.f, XMFLOAT3 noise_center = XMFLOAT3(0.f, 0.f, 0.f),
		float noise_min_threshold = 0.f, unsigned noise_layers = 1u, float noise_layer_roughness = 2.f, float noise_layer_persistence = .5f);
	~CubeSphereMesh();

	void Regenrate(ID3D11Device* device);

	void setResolution(const unsigned& r) { resolution_ = r; }
	void setRadius(const float& r) { radius_ = r; }
	unsigned* getResolution() { return &resolution_; }
	float* getRadius() { return &radius_; }
	std::vector<std::unique_ptr<NoiseLayerSettings>>* getNoiseLayers() { return &noise_layers_; }

private:
	void initBuffers(ID3D11Device* device);
	unsigned resolution_;
	float radius_;

	//At least one layer will exist, it is initialised in the constructor
	std::vector<std::unique_ptr<NoiseLayerSettings>> noise_layers_;
};

