#include "FabrikMesh.h"

FabrikMesh::FabrikMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext) : LineMesh(device, deviceContext)
{
	position = XMFLOAT3(0.f, 0.f, 0.f);
	goal_position = XMFLOAT3(0.f, 1.f, 0.f);
	n_segments = 1;
	total_length = 1.f;
}

FabrikMesh::FabrikMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, XMFLOAT3 pos) : LineMesh(device, deviceContext)
{
	position = pos;
	goal_position = XMFLOAT3(0.f, 1.f, 0.f);
	n_segments = 1;
	total_length = 1.f;
}

FabrikMesh::FabrikMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, float posX, float posY, float posZ) : LineMesh(device, deviceContext)
{
	position = XMFLOAT3(posX, posY, posZ);
	goal_position = XMFLOAT3(0.f, 1.f, 0.f);
	n_segments = 1;
	total_length = 1.f;
}

FabrikMesh::~FabrikMesh()
{
}

void FabrikMesh::init()
{
	//Clear any lines we might already have
	Clear();

	//Calculate the length of each individual segment, they will all be pointing upwards first
	float segment_length = total_length / (float)n_segments;

	//Place each segment one after the other
	for (int i = 0; i < n_segments; ++i)
		AddLine(
			XMVectorSet(0.f, total_length - (float)(i + 1) * segment_length, 0.f, 1.f),
			XMVectorSet(0.f, total_length - (float)i * segment_length, 0.f, 1.f)
		);
}

void FabrikMesh::update()
{
	/////
	// Inverse Kinematics
	// FABRIK - Forward
	////

	//Move each segment towards their target position
	//Move the first segment to the goal
	getSegment(0).follow(XMVectorSet(goal_position.x, goal_position.y, goal_position.z, 1.f));
	//Move the other segments to the start of each previous segment
	for (int i = 1; i < n_segments; ++i)
		getSegment(i).follow(getSegment(i - 1).getStart());

	////
	// Inverse Kinematics
	// FABRIK - Backward
	////

	//Move the last segment to the origin
	getSegment(n_segments - 1).moveBack(XMVectorSet(0.f, 0.f, 0.f, 1.f));
	//Move the other segments to the end of the next segment
	for (int i = n_segments - 2; i >= 0; --i)
		getSegment(i).moveBack(getSegment(i + 1).getEnd());
}

void FabrikMesh::render(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	//Build the vertices
	BuildLine(deviceContext, device);
}

void FabrikMesh::setPosition(float& x, float& y, float& z)
{
	position = XMFLOAT3(x, y, z);
}

void FabrikMesh::setPosition(XMFLOAT3& pos)
{
	position = pos;
}

void FabrikMesh::setGoal(float& x, float& y, float& z)
{
	goal_position = XMFLOAT3(x, y, z);
}

void FabrikMesh::setGoal(XMFLOAT3& g)
{
	goal_position = g;
}

void FabrikMesh::setSegmentCount(int& c)
{
	n_segments = c;
}

void FabrikMesh::setLength(float& l)
{
	total_length = l;
}

const XMFLOAT3& FabrikMesh::getPosition()
{
	return position;
}

const XMFLOAT3& FabrikMesh::getGoal()
{
	return goal_position;
}

const int& FabrikMesh::getSegmentCount()
{
	return n_segments;
}

const float& FabrikMesh::getLength()
{
	return total_length;
}
