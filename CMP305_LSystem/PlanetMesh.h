#pragma once
#include "BaseMesh.h"
#include "farm.h"
#include "GenerateMeshTask.h"
#include "GenerateNormalsTask.h"
#include "GenerateTreeTask.h"
#include "RegenerateTreeSystemTask.h"
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
	void GenerateNormals();
	void GenerateTrees(ID3D11Device* device, ID3D11DeviceContext* device_context, HWND hwnd, float grass_low_threshold, float grass_high_threshold);
	void GenerateMesh(ID3D11Device* device);

	void waitAndCleanFarm() { farm.clean(); }

	void RegenerateSystemTrees(ID3D11Device* device, ID3D11DeviceContext* device_context);

	bool isGenerating() { return farm.isRunning(); }
	const uint8_t& getCurrentTask() { return current_task; }
	const float& getGenerationProgress() { return farm.getProgressPercentage(); }

	void setResolution(const unsigned& r) { resolution_ = r; }
	//void setRadius(const float& r) { radius_ = r; }
	void setDebug(const bool& d) { debug_building_ = d; }
	unsigned* getResolution() { return &resolution_; }
	//float* getRadius() { return &radius_; }
	bool* getDebug() { return &debug_building_; }
	std::vector<std::unique_ptr<NoiseLayerSettings>>* getNoiseLayers() { return &noise_layers_; }
	std::vector<Tree*>* getTrees() { return &planet_trees_; }
	float* getTreeScale() { return &planet_tree_scale_; }
	unsigned* getNumberTreesPerFace() { return &n_trees_per_face_; }
	float* getTreeNormalMaxAngle() { return &tree_max_angle_to_normal_; }
	unsigned* getTreeSystemIterations() { return &tree_system_n_iterations_; }
	XMFLOAT4* getShaderSettings() { return &shader_material_thresholds; }

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
	uint8_t current_task;	//this variable is a simple byte that flags which task is being generated: 0 - OFF, 1 - Vertices, 2 - Normals, 3 - Trees generation, 4 - Tree Regeneration
	VertexType* vertices;
	unsigned long* indices;

	//Planet trees
	std::vector<Tree*> planet_trees_;
	float planet_tree_scale_;
	//Define a maximum amount of trees per face
	unsigned n_trees_per_face_;
	float tree_max_angle_to_normal_;
	unsigned tree_system_n_iterations_;
	//Shader settings
	XMFLOAT4 shader_material_thresholds;

	struct Export_Settings {
		Export_Settings() {
			resolution = 0;
			tree_scale = 0.f;
			n_tree_per_face = 0u;
			tree_max_angle = 0.f;
			tree_system_n_iterations = 0u;
			shader = XMFLOAT4(0.f, 0.f, 0.f, 0.f);
		}
		Export_Settings(unsigned r, float sc, unsigned t, float m, unsigned i, XMFLOAT4 s) {
			resolution = r;
			tree_scale = sc;
			n_tree_per_face = t;
			tree_max_angle = m;
			tree_system_n_iterations = i;
			shader = s;
		}
		unsigned resolution;
		float tree_scale;
		unsigned n_tree_per_face;
		float tree_max_angle;
		unsigned tree_system_n_iterations;
		XMFLOAT4 shader;
	};
};

