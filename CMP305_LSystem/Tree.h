#pragma once
#include "DXF.h"
#include "LSystem.h"
#include "Leaf.h"
#include "CylinderMesh.h"
#include "TreeShader.h"

class Tree
{
public:
	Tree(ID3D11Device* device, ID3D11DeviceContext* device_context, HWND hwnd, XMMATRIX& transform = XMMatrixIdentity());
	~Tree();

	void render(ID3D11DeviceContext* device_context, XMMATRIX& world, XMMATRIX& view, XMMATRIX& proj, Light* light);

	void runSystem() { lSystem_.Run(5u); }
	void build(ID3D11Device* device, ID3D11DeviceContext* device_context);

	void setTransform(XMMATRIX& t) { tree_transform_ = t; }
	const std::string& getCurrentSystem() { return lSystem_.GetCurrentSystem(); }
	const XMMATRIX& getTransform() { return tree_transform_; }

private:
	void add_system_rules();
	std::vector<std::unique_ptr<CylinderMesh>> tree_branches_;
	std::vector<std::unique_ptr<Leaf>> tree_leaves_;
	XMMATRIX tree_transform_;
	LSystem lSystem_;
	TreeShader tree_shader_;
};

