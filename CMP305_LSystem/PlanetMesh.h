#pragma once
#include "BaseMesh.h"
#include "ImprovedNoise.h"
#include "ExtraAlgorithms.h"
#include <vector>
#include <memory>
//Noise settings - A collection of settings for each layer instanciated
enum class NoiseType { FBM, RIGID };

struct NoiseLayerSettings {
	NoiseLayerSettings() {
		layer_active_ = true;
		layer_use_previous_layer_as_mask_ = false;
		noise_type_ = NoiseType::FBM;

		noise_base_frequency_ = 0.f;
		noise_base_amplitude_ = 0.f;
		noise_min_threshold_ = 0.f;
		layer_roughness_ = 0.f;
		layer_persistence_ = 0.f;
		layer_nSubLayers_ = 1;
		layer_center_ = XMFLOAT3(0.f, 0.f, 0.f);

		rigid_noise_sharpness_ = 1.f;
		rigid_noise_LOD_multiplier_ = 1.f;
	}
	NoiseLayerSettings(const float& r, const float& p, const unsigned& n, const XMFLOAT3& c, const float& basef, const float& basea, const float& t){
		layer_active_ = true;
		layer_use_previous_layer_as_mask_ = false;
		noise_type_ = NoiseType::FBM;

		layer_roughness_ = r;
		layer_persistence_ = p;
		layer_nSubLayers_ = n;
		layer_center_ = c;
		noise_base_frequency_ = basef;
		noise_base_amplitude_ = basea;
		noise_min_threshold_ = t;

		rigid_noise_sharpness_ = 1.f;
		rigid_noise_LOD_multiplier_ = 1.f;
	}
	//General settings for a layer
	bool layer_active_, layer_use_previous_layer_as_mask_;
	NoiseType noise_type_;

	//Basic Settings that each layer will have at the least no matter the noise type
	float noise_base_frequency_, noise_base_amplitude_;
	float noise_min_threshold_;
	float layer_roughness_, layer_persistence_;
	unsigned layer_nSubLayers_;
	XMFLOAT3 layer_center_;

	//Rigid noise specific settings
	float rigid_noise_sharpness_;			//Defines how sharp the curve becomes, used as a power when calculating the noise
	float rigid_noise_LOD_multiplier_;	//Defines how detailed the noise becomes, more details near peaks and low detail on plains
};

class PlanetMesh : public BaseMesh
{
public:
	PlanetMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, unsigned resolution = 20u,
		float noise_frequency = 0.f, float noise_amplitude = 0.f, XMFLOAT3 noise_center = XMFLOAT3(0.f, 0.f, 0.f),
		float noise_min_threshold = 0.f, unsigned noise_layers = 1u, float noise_layer_roughness = 2.f, float noise_layer_persistence = .5f);
	~PlanetMesh();

	void Regenrate(ID3D11Device* device);

	void setResolution(const unsigned& r) { resolution_ = r; }
	//void setRadius(const float& r) { radius_ = r; }
	void setDebug(const bool& d) { debug_building_ = d; }
	unsigned* getResolution() { return &resolution_; }
	//float* getRadius() { return &radius_; }
	bool* getDebug() { return &debug_building_; }
	std::vector<std::unique_ptr<NoiseLayerSettings>>* getNoiseLayers() { return &noise_layers_; }
	const float& getMaxNoise() { return noise_max; }

private:
	void initBuffers(ID3D11Device* device);
	unsigned resolution_;
	float radius_;

	//A simple boolean that will allow to create the vertices of one face only
	//Useful when building the planet on a high resolution
	//This can be done better by multhreading the generation on the CPU or moving it to the GPU
	bool debug_building_;

	//At least one layer will exist, it is initialised in the constructor
	std::vector<std::unique_ptr<NoiseLayerSettings>> noise_layers_;

	//A float to keep track of the highest noise values
	//Used in the shader to determine colour gradients
	float noise_max;
};

