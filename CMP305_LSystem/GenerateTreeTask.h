#pragma once
#include "task.h"
#include "DXF.h"
#include "Tree.h"

class GenerateTreeTask : public Task
{
public:
	GenerateTreeTask(const ID3D11Device* device, const ID3D11DeviceContext* deviceContext, HWND hwnd,
		const float& grass_low_threshold, const float& grass_high_threshold, const float& tree_scale, const float& tree_max_angle,
		const XMFLOAT3& pos_v1, const XMFLOAT3& pos_v2, const XMFLOAT3& pos_v3,
		const XMFLOAT3& norm_v1, const XMFLOAT3& norm_v2, const XMFLOAT3& norm_v3,
		std::unique_ptr<Tree>* tree_to_modify);
	~GenerateTreeTask() override;

	void run() override;

private:
	const ID3D11Device* device_;
	const ID3D11DeviceContext* deviceContext_;
	HWND window_;
	float grass_low_threshold_, grass_high_threshold_, tree_scale_, tree_max_angle_;
	XMFLOAT3 pos_v1_, pos_v2_, pos_v3_, norm_v1_, norm_v2_, norm_v3_;
	std::unique_ptr<Tree>* tree_to_modify_;
};

