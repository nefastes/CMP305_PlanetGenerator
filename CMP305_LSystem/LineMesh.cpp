#include "LineMesh.h"

LineMesh::LineMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	initBuffers(device);
}

//Cleanup the heightMap
LineMesh::~LineMesh() {
	// Run parent deconstructor
	BaseMesh::~BaseMesh();
}

void LineMesh::BuildLine(ID3D11DeviceContext* deviceContext, ID3D11Device* device) {
	//Clear the vertex buffers
	if (vertexBuffer != NULL) {
		vertexBuffer->Release();
	}
	vertexBuffer = NULL;

	vertices.clear();
	indices.clear();

	indexCount = m_LineList.size() * 2;

	float uvStep = 1.0f / indexCount;
	VertexType v;
	for (int i = 0; i < m_LineList.size(); i++) {
		XMStoreFloat3(&(v.position), m_LineList[i].start);
		v.texture = XMFLOAT2(uvStep * (float)i, uvStep * (float)i);
		vertices.push_back(v);

		XMStoreFloat3(&(v.position), m_LineList[i].end);
		v.texture = XMFLOAT2(uvStep*(float)i, uvStep * (float)i);
		vertices.push_back(v);
	}
	for (int i = 0; i < m_LineList.size(); i++) {
		indices.push_back(i * 2);
		indices.push_back(i * 2+1);
	}

	initBuffers(device);
}

void LineMesh::sendData(ID3D11DeviceContext* deviceContext, D3D_PRIMITIVE_TOPOLOGY top)
{
	unsigned int stride;
	unsigned int offset;

	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType);
	offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetPrimitiveTopology(top);
}

// Generate buffers
void LineMesh::initBuffers(ID3D11Device* device)
{
	if (vertices.size() < 1)
		return;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	// Set up the description of the dyanmic vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * vertices.size();
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = &vertices[0];
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;
	// Now create the vertex buffer.
	device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(UINT) * indices.size();
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = &indices[0];
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	indexCount = indices.size();

	// Create the index buffer.
	device->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);
}

void LineSegment::update(XMVECTOR goal)
{
	//Compute the two vectors
	XMVECTOR dir = goal - start;
	XMVECTOR segment = end - start;

	//Compute the required data for the rotation of the segment
	XMVECTOR normalized_dir = XMVector3Normalize(dir);
	XMVECTOR normalized_segment = XMVector3Normalize(segment);
	XMVECTOR rotation_axis = XMVector3Normalize(XMVector3Cross(normalized_dir, normalized_segment));
	//rotation_axis = XMVectorSetW(rotation_axis, 1.f);	//gets zero'd out by the cross
	float rotation_angle = -XMVectorGetX(XMVector3AngleBetweenNormals(normalized_dir, normalized_segment));
	//Rotate the segment
	if (!XMVector3Equal(rotation_axis, XMVectorZero()))
	{
		XMMATRIX transform = XMMatrixRotationAxis(rotation_axis, rotation_angle);
		segment = XMVector3Transform(segment, transform);
	}

	//Update the line components
	start = goal - segment;
	end = goal;
}
