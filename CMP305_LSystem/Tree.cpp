#include "Tree.h"

Tree::Tree(ID3D11Device* device, ID3D11DeviceContext* device_context, HWND hwnd, XMMATRIX& transform) : lSystem_(std::string("FA")),
tree_shader_(device, hwnd), tree_transform_(transform)
{
	add_system_rules();
	lSystem_.Run(5u);	//5 iterations
	build(device, device_context);
}

Tree::~Tree()
{
	//Clean memory
	tree_branches_.clear();
	tree_leaves_.clear();
}

void Tree::render(ID3D11DeviceContext* device_context, XMMATRIX& world, XMMATRIX& view, XMMATRIX& proj, Light* light)
{
	//Draw the branches
	for (unsigned i = 0u; i < tree_branches_.size(); ++i)
	{
		tree_shader_.setShaderParameters(device_context, tree_branches_[i]->m_Transform * tree_transform_ * world, view, proj, light, true);
		tree_branches_[i]->sendData(device_context);
		tree_shader_.render(device_context, tree_branches_[i]->getIndexCount());
	}
	//Draw the leaves
	for (unsigned i = 0u; i < tree_leaves_.size(); ++i)
	{
		tree_shader_.setShaderParameters(device_context, tree_leaves_[i]->m_Transform * tree_transform_ * world, view, proj, light, false);
		tree_leaves_[i]->sendData(device_context);
		tree_shader_.render(device_context, tree_leaves_[i]->getIndexCount());
	}
}

void Tree::add_system_rules()
{
	//3D tree rules from lab sheet
	lSystem_.ClearRules();
	lSystem_.AddRule('A', "A");
	lSystem_.AddRule('A', "[&FA]");
	lSystem_.AddRule('A', "[&FA][<&FA]");
	lSystem_.AddRule('A', "[&FA][>&FA]");
	lSystem_.AddRule('A', "[&FA][<&FA][>&FA]");
	lSystem_.AddRule('A', "[<&FA]");
	lSystem_.AddRule('A', "[<&FA][>&FA]");
	lSystem_.AddRule('A', "[>&FA]");
	lSystem_.AddRule('F', "F");
	lSystem_.AddRule('[', "[");
	lSystem_.AddRule(']', "]");
	lSystem_.AddRule('&', "&");
	lSystem_.AddRule('>', ">");
	lSystem_.AddRule('<', "<");
}

void Tree::build(ID3D11Device* device, ID3D11DeviceContext* device_context)
{
	tree_branches_.clear();
	tree_leaves_.clear();

	//Get the current L-System string, right now we have a place holder
	std::string systemString = lSystem_.GetCurrentSystem();

	//Initialise some variables
	XMVECTOR pos = XMVectorReplicate(0.0f);	//Current position (0,0,0)
	XMVECTOR dir = XMVectorSet(0, 1, 0, 1);	//Current direction is "Up"
	XMVECTOR fwd = XMVectorSet(0, 0, 1, 1);	//Rotation axis.
	XMMATRIX currentRotation = XMMatrixRotationRollPitchYaw(0, 0, 0);
	std::vector<XMMATRIX> rotation_stack;
	std::vector<XMVECTOR> pos_stack;
	float rng = 0.f, cylinder_radius_scale = 1.f;

	//Build 3D tree
	for (int i = 0; i < systemString.length(); i++) {
		switch (systemString[i]) {
		case 'F':
			{
			//Draw a line segment and move forward
			XMVECTOR step = XMVector3Transform(dir, currentRotation);
			tree_branches_.push_back(std::make_unique<CylinderMesh>(
				device,
				device_context,
				1,
				4,
				XMVectorGetX(XMVector3Length(step)),
				.1f * cylinder_radius_scale,
				.1f * cylinder_radius_scale * .6f,
				currentRotation * XMMatrixTranslationFromVector(pos)
				));
			//Move the position marker
			pos += step;
			}
			break;
		case 'A':
			//Add a leaf
			tree_leaves_.push_back(std::make_unique<Leaf>(device, device_context, 2));
			tree_leaves_.back()->m_Transform = XMMatrixScaling(.25f, .25f, .25f) * XMMatrixTranslationFromVector(pos);
			break;
		case '[':
			pos_stack.push_back(pos);
			rotation_stack.push_back(currentRotation);
			dir *= XMVectorSet(.8f, .8f, .8f, 1.f);
			cylinder_radius_scale *= .6f;
			break;
		case ']':
			pos = pos_stack.back();
			pos_stack.pop_back();
			currentRotation = rotation_stack.back();
			rotation_stack.pop_back();
			dir /= XMVectorSet(.8f, .8f, .8f, 1.f);
			cylinder_radius_scale /= .6f;
			break;
		case '&':
			rng = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / (3.1415f / 4.f));
			currentRotation *= XMMatrixRotationAxis(XMVector3Transform(fwd, currentRotation), rng);
			break;
		case '<':
			rng = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / 3.1415f);
			currentRotation *= XMMatrixRotationAxis(XMVector3Transform(dir, currentRotation), rng);
			break;
		case '>':
			rng = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / -3.1415f);
			currentRotation *= XMMatrixRotationAxis(XMVector3Transform(dir, currentRotation), rng);
			break;
		default:
			break;
		}
	}
}
