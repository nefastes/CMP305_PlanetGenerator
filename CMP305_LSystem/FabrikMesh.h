#pragma once
#include "LineMesh.h"
class FabrikMesh : public LineMesh
{
public:
	FabrikMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	FabrikMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, XMFLOAT3 pos, XMFLOAT3 goal, int nSegment, float length);
	FabrikMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, float posX, float posY, float posZ, float goalX, float goalY, float goalZ, int nSegment, float length);
	~FabrikMesh();

private:
	void init();
public:
	void update(ID3D11Device* device, ID3D11DeviceContext* deviceContext);

	void setPosition(float& x, float& y, float& z);
	void setPosition(XMFLOAT3& pos);

	void setGoal(float& x, float& y, float& z);
	void setGoal(XMFLOAT3& g);

	void setSegmentCount(int& c);
	void setLength(float& l);

	const XMFLOAT3& getPosition();
	const XMFLOAT3& getGoal();
	const int& getSegmentCount();
	const float& getLength();

private:
	XMFLOAT3 position;
	XMFLOAT3 goal_position;
	int n_segments;
	float total_length;
};

