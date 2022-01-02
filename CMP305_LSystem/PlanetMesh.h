#pragma once
#include "BaseMesh.h"
#include "farm.h"
#include "GenerateMeshTask.h"
#include "Tree.h"
#include <vector>
#include <memory>
#include <fstream>

class PlanetMesh : public BaseMesh
{
public:
	PlanetMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, HWND hwnd, unsigned resolution = 20u);
	~PlanetMesh();

	bool does_file_exist(const char* filename, const int namesize);
	void ExportSettings(const char* filename, const int namesize);
	unsigned ImportSettings(ID3D11Device* device, const char* filename, const int namesize);
	void GenerateVertices();
	const bool& isGeneratingVertices() { return farm.isRunning(); }
	const float& getGenerationProgress() { return farm.getProgressPercentage(); }
	void GenerateMesh(ID3D11Device* device, ID3D11DeviceContext* device_context, HWND hwnd);

	void setResolution(const unsigned& r) { resolution_ = r; }
	//void setRadius(const float& r) { radius_ = r; }
	void setDebug(const bool& d) { debug_building_ = d; }
	unsigned* getResolution() { return &resolution_; }
	//float* getRadius() { return &radius_; }
	bool* getDebug() { return &debug_building_; }
	std::vector<std::unique_ptr<NoiseLayerSettings>>* getNoiseLayers() { return &noise_layers_; }
	const std::vector<std::unique_ptr<Tree>>* getTrees() { return &planet_trees_; }
	float* getTreeScale() { return &planet_tree_scale_; }

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

	//A farm for mulithreading
	Farm farm;
	VertexType* vertices;
	unsigned long* indices;

	//Planet trees
	std::vector<std::unique_ptr<Tree>> planet_trees_;
	float planet_tree_scale_;
};

