#include "GenerateTreeTask.h"

GenerateTreeTask::GenerateTreeTask(ID3D11Device* device, ID3D11DeviceContext* deviceContext, HWND hwnd,
	const float& grass_low_threshold, const float& grass_high_threshold, const float& tree_scale, const float& tree_max_angle,
	const XMFLOAT3& pos_v1, const XMFLOAT3& pos_v2, const XMFLOAT3& pos_v3,
	const XMFLOAT3& norm_v1, const XMFLOAT3& norm_v2, const XMFLOAT3& norm_v3,
	std::vector<Tree*>* trees)
{
	device_ = device;
	deviceContext_ = deviceContext;
	window_ = hwnd;
	grass_low_threshold_ = grass_low_threshold;
	grass_high_threshold_ = grass_high_threshold;
	tree_scale_ = tree_scale;
	tree_max_angle_ = tree_max_angle;
	pos_v1_ = pos_v1;
	pos_v2_ = pos_v2;
	pos_v3_ = pos_v3;
	norm_v1_ = norm_v1;
	norm_v2_ = norm_v2;
	norm_v3_ = norm_v3;
	trees_ = trees;
}

GenerateTreeTask::~GenerateTreeTask()
{
	trees_ = nullptr;
}

void GenerateTreeTask::run()
{
	//Spawn trees if possible
	//Ensure the vertex is within grass distance
	XMVECTOR position = (XMLoadFloat3(&pos_v1_) + XMLoadFloat3(&pos_v2_) + XMLoadFloat3(&pos_v3_)) / 3.f;
	float pos_x = XMVectorGetX(position);
	float pos_y = XMVectorGetY(position);
	float pos_z = XMVectorGetZ(position);
	float vertex_distance = (pos_x * pos_x + pos_y * pos_y + pos_z * pos_z) - 1.f;
	if (vertex_distance < grass_low_threshold_ || vertex_distance > grass_high_threshold_) return;

	//Scale the tree
	XMMATRIX transform = XMMatrixScaling(tree_scale_, tree_scale_, tree_scale_);
	//Rotate the tree so that it's up direction is along the surface normal
	XMVECTOR up = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	XMVECTOR normal = (XMLoadFloat3(&norm_v1_) + XMLoadFloat3(&norm_v2_) + XMLoadFloat3(&norm_v3_)) / 3.f;
	XMVECTOR rotation_axis = XMVector3Cross(up, normal);
	float rotation_angle = XMVectorGetX(XMVector3AngleBetweenNormals(up, normal));
	if (rotation_angle < -tree_max_angle_ || rotation_angle > tree_max_angle_) return;	//Ensure a tree can't be placed on a steep surface
	transform = XMMatrixMultiply(transform, XMMatrixRotationAxis(rotation_axis, rotation_angle));
	//Translate the tree to the vertex's position
	transform = XMMatrixMultiply(transform, XMMatrixTranslation(XMVectorGetX(position), XMVectorGetY(position), XMVectorGetZ(position)));
	const std::lock_guard<std::mutex> lock(tree_mutex_);
	trees_->push_back(new Tree(device_, deviceContext_, window_, transform));
}