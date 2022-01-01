#pragma once
#include "task.h"
#include "DXF.h"
#include "ImprovedNoise.h"
#include "ExtraAlgorithms.h"

class GenerateMeshTask : public Task
{
private:

public:
	GenerateMeshTask(XMFLOAT3* v_pos, const XMFLOAT3& cube_vertex_pos, const float& radius, const std::vector<std::unique_ptr<NoiseLayerSettings>>* noise_layers);
	~GenerateMeshTask() override;

	void run() override;
private:
	XMFLOAT3* task_vertex_position_;
	XMFLOAT3 task_cube_vertex_pos_;
	float task_radius_;
	const std::vector<std::unique_ptr<NoiseLayerSettings>>* task_noise_layers_;
};

