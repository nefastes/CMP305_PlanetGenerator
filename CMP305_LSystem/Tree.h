#pragma once
#include "DXF.h"
#include "LSystem.h"
#include "Leaf.h"
#include "CylinderMesh.h"
#include "TreeShader.h"
#include <mutex>

class Tree
{
public:
	Tree(ID3D11Device* device, ID3D11DeviceContext* device_context, HWND hwnd, XMMATRIX& transform = XMMatrixIdentity(), const unsigned& n_iterations = 5u);
	~Tree();

	void render(ID3D11DeviceContext* device_context, XMMATRIX& world, XMMATRIX& view, XMMATRIX& proj, Light* light);

	void runSystem() { lSystem_.Run(5u); }
	void build(ID3D11Device* device, ID3D11DeviceContext* device_context);

	void setTransform(XMMATRIX& t) { tree_transform_ = t; }
	const std::string& getCurrentSystem() { return lSystem_.GetCurrentSystem(); }
	const XMMATRIX& getTransform() { return tree_transform_; }

private:
	void add_system_rules();
	std::vector<CylinderMesh*> tree_branches_;
	std::vector<Leaf*> tree_leaves_;
	XMMATRIX tree_transform_;
	LSystem lSystem_;
	TreeShader tree_shader_;

	std::mutex tree_mutex_;
};

