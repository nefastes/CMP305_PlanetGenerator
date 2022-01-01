#pragma once
#include "task.h"
#include "DXF.h"
#include "ImprovedNoise.h"
#include "ExtraAlgorithms.h"

class GenerateMeshTask : public Task
{
private:
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
		XMFLOAT3 normal;
	};

public:
	GenerateMeshTask(VertexType* v, const XMFLOAT3& cube_vertex_pos, const float& radius, const std::vector<std::unique_ptr<NoiseLayerSettings>>* noise_layers);
	~GenerateMeshTask() override;

	void run() override;
private:
	VertexType* task_vertex_;
	XMFLOAT3 task_cube_vertex_pos_;
	float task_radius_;
	const std::vector<std::unique_ptr<NoiseLayerSettings>>* task_noise_layers_;
};

