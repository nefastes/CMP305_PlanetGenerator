#pragma once
#include "BaseMesh.h"
#include <vector>

class LineSegment {
public:
	LineSegment() {
		start = XMVectorSet(0, 0, 0, 1); end = XMVectorSet(0, 1, 0, 1);
	}
	LineSegment(XMVECTOR s, XMVECTOR e) {
		start = s; end = e;
	}

	XMVECTOR& getPosition() { return start; }

	void update(XMVECTOR goal)
	{
		//Compute the two vectors
		XMVECTOR v1 = goal - start;
		XMVECTOR v2 = end - start;

		//Compute the required data for the rotation of the segment
		XMVECTOR rotation_axis = XMVector3Cross(v1, v2);
		rotation_axis = XMVectorSetW(rotation_axis, 1.f);	//gets zero'd out by the cross
		float rotation_angle = XMVectorGetX(XMVector3Dot(v1, v2));

		//Rotate the segment
		if (!XMVector3Equal(rotation_axis, XMVectorZero()))
		{
			XMMATRIX transform = XMMatrixRotationAxis(rotation_axis, rotation_angle);
			end = XMVector3Transform(end, transform);
			v2 = end - start;
		}

		//
		end = goal;
		start = goal - v2;
	}
	
	XMVECTOR start;
	XMVECTOR end;
};

class LineMesh :
	public BaseMesh
{
public:
	
	LineMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	~LineMesh();

	void	sendData(ID3D11DeviceContext* deviceContext, D3D_PRIMITIVE_TOPOLOGY top = D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	void	BuildLine(ID3D11DeviceContext* deviceContext, ID3D11Device* device);

	inline const int		GetLineCount() { return m_LineList.size(); }

	//Line manipulation
	void	AddLine(XMVECTOR start, XMVECTOR end) { m_LineList.push_back(LineSegment(start, end)); }
	void	AddSegment() { m_LineList.push_back(LineSegment()); }
	void	Clear() {
		m_LineList.clear();
		vertices.clear();
		indices.clear();
		if(vertexBuffer != NULL) vertexBuffer->Release();
		if(indexBuffer != NULL) indexBuffer->Release();
		indexCount = 0;
	}

	LineSegment& getSegment(int index) { return m_LineList[index]; }

private:
	std::vector<VertexType>		vertices;
	std::vector<UINT>			indices;
	std::vector<LineSegment>	m_LineList;

	void	initBuffers(ID3D11Device* device);
};

