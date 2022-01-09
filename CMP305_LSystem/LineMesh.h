#pragma once
#include "BaseMesh.h"
#include <vector>

class LineSegment {
private:
	void rotateSegment(XMVECTOR& segment, const XMVECTOR& direction);

public:
	LineSegment() {
		start = XMVectorSet(0, 0, 0, 1); end = XMVectorSet(0, 1, 0, 1);
	}
	LineSegment(const XMVECTOR& s, const XMVECTOR& e) {
		start = s; end = e;
	}

	const XMVECTOR& getStart() { return start; }
	const XMVECTOR& getEnd() { return end; }

	void follow(const XMVECTOR& goal);
	void moveBack(const XMVECTOR& start_point);
	
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

protected:
	std::vector<VertexType>		vertices;
	std::vector<UINT>			indices;
	std::vector<LineSegment>	m_LineList;

	void	initBuffers(ID3D11Device* device);
};

