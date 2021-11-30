#include "FabrikMesh.h"

FabrikMesh::FabrikMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext) : LineMesh(device, deviceContext)
{
	position = XMFLOAT3(0.f, 0.f, 0.f);
	goal_position = XMFLOAT3(0.f, 1.f, 0.f);
	n_segments = 1;
	total_length = 1.f;
	init();
}

FabrikMesh::FabrikMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, XMFLOAT3 pos, XMFLOAT3 goal, int nSegment, float length) : LineMesh(device, deviceContext)
{
	position = pos;
	goal_position = goal;
	n_segments = nSegment;
	total_length = length;
	init();
}

FabrikMesh::FabrikMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, float posX, float posY, float posZ,
	float goalX, float goalY, float goalZ, int nSegment, float length) : LineMesh(device, deviceContext)
{
	position = XMFLOAT3(posX, posY, posZ);
	goal_position = XMFLOAT3(goalX, goalY, goalZ);
	n_segments = nSegment;
	total_length = length;
	init();
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

void FabrikMesh::update(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	/////
	// Inverse Kinematics
	// FABRIK - Forward
	////

	//Move each segment towards their target position
	//Move the first segment to the goal
	m_LineList[0].follow(XMVectorSet(goal_position.x, goal_position.y, goal_position.z, 1.f));
	//Move the other segments to the start of each previous segment
	for (int i = 1; i < n_segments; ++i)
		m_LineList[i].follow(m_LineList[i - 1].getStart());

	////
	// Inverse Kinematics
	// FABRIK - Backward
	////

	//Move the last segment to the origin
	m_LineList[n_segments - 1].moveBack(XMVectorSet(position.x, position.y, position.z, 1.f));
	//Move the other segments to the end of the next segment
	for (int i = n_segments - 2; i >= 0; --i)
		m_LineList[i].moveBack(m_LineList[i + 1].getEnd());
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
	init();
}

void FabrikMesh::setLength(float& l)
{
	total_length = l;
	init();
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

void FabrikMesh::BuildCylinders(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	//Build a cylinder mesh that follows the segments (one segment, one stack)

	//Clear the vertex buffers
	if (vertexBuffer != NULL) {
		vertexBuffer->Release();
	}
	vertexBuffer = NULL;

	vertices.clear();
	indices.clear();

	float height = total_length, bottomRadius = .05f, topRadius = 0.f;
	int stacks = n_segments, slices = 4;

	float stackHeight = height / stacks;
	UINT ringCount = stacks + 1;
	int sliceCount = slices;

	//Vertices
	for (UINT i = 0; i < ringCount; i++) {
		float y = i * stackHeight;
		float r = topRadius + ((float)i / stacks) * bottomRadius;

		float dTheta = 2.0f * XM_PI / sliceCount;

		XMVECTOR pos = i == 0 ? m_LineList[0].getEnd() : m_LineList[i - 1].getStart();

		for (UINT j = 0; j <= sliceCount; j++) {
			VertexType vertex;

			float c = cosf(j * dTheta);
			float s = sinf(j * dTheta);

			vertex.position = XMFLOAT3(XMVectorGetX(pos) + r * c, XMVectorGetY(pos), XMVectorGetZ(pos) + r * s);
			vertex.texture.x = (float)j / sliceCount;
			vertex.texture.y = 1.0f - (float)i / stacks;
			vertex.normal = XMFLOAT3(r * c, 0.0f, r * s);

			vertices.push_back(vertex);
		}
	}

	//Indices
	UINT ringVertexCount = sliceCount + 1;
	for (UINT i = 0; i < ringCount - 1; i++) {
		for (UINT j = 0; j < sliceCount; j++) {
			indices.push_back((i + 1) * ringVertexCount + j + 1);
			indices.push_back(i * ringVertexCount + j + 1);
			indices.push_back(i * ringVertexCount + j);

			indices.push_back((i + 1) * ringVertexCount + j);
			indices.push_back((i + 1) * ringVertexCount + j + 1);
			indices.push_back(i * ringVertexCount + j);
		}
	}

	//Data created, initialise it as buffers in DirectX
	initBuffers(device);
}
