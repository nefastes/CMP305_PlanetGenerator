#include "CubeSphereMesh.h"

CubeSphereMesh::CubeSphereMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, unsigned resolution, float radius,
	float noise_frequency, float noise_amplitude, XMFLOAT3 noise_center, float noise_min_threshold, unsigned noise_layers,
	float noise_layer_roughness, float noise_layer_persistence) :
	resolution_(resolution), radius_(radius), noise_frequency_(noise_frequency), noise_amplitude_(noise_amplitude), noise_center_(noise_center),
	noise_min_threshold_(noise_min_threshold), noise_layer_iterations_(noise_layers), noise_layer_roughness_(noise_layer_roughness),
	noise_layer_persistence_(noise_layer_persistence)
{
	initBuffers(device);
}

CubeSphereMesh::~CubeSphereMesh()
{
	BaseMesh::~BaseMesh();
}

void CubeSphereMesh::Regenrate(ID3D11Device* device, unsigned resolution, float radius, float noise_frequency, float noise_amplitude,
	XMFLOAT3 noise_center, float noise_min_threshold, unsigned noise_layers, float noise_layer_roughness, float noise_layer_persistence)
{
	resolution_ = resolution;
	radius_ = radius;
	noise_frequency_ = noise_frequency;
	noise_amplitude_ = noise_amplitude;
	noise_center_ = noise_center;
	noise_min_threshold_ = noise_min_threshold;
	noise_layer_iterations_ = noise_layers;
	noise_layer_roughness_ = noise_layer_roughness;
	noise_layer_persistence_ = noise_layer_persistence;
	initBuffers(device);
}

void CubeSphereMesh::initBuffers(ID3D11Device* device)
{
	//The cube sphere is generated with a radius of radius_ around the center (0,0,0)
	//Every vertex needs to be at the same distance from the center

	//Check if the cube sphere was already generated, in the event of regeneration
	if (vertexBuffer) vertexBuffer->Release(), vertexBuffer = NULL;
	if (indexBuffer) indexBuffer->Release(), indexBuffer = NULL;

	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	// 6 vertices per quad, res*res is face, times 6 for each face (From Paul Robertson - CubeMesh)
	// 6 vertices per quad because one quad consists of two triangles, i.e. 2x3 vertices
	vertexCount = static_cast<int>(((6u * resolution_) * resolution_) * 6u);
	//There are as many indices as vertices
	indexCount = vertexCount;
	// Create the vertex and index array.
	vertices = new VertexType[vertexCount];
	indices = new unsigned long[indexCount];

	// Vertex variables
	float yincrement = 2.0f / resolution_;
	float xincrement = 2.0f / resolution_;
	float ystart = 1.0f;
	float xstart = -1.0f;
	//UV variables
	float txu = 0.0f;
	float txv = 0.0f;
	float txuinc = 1.0f / resolution_;	// UV increment
	float txvinc = 1.0f / resolution_;
	//Counters
	int v = 0;	// vertex counter
	int i = 0;	// index counter
	//Lambda to calculate a new vertex position and normal base on it's intended position on a cube
	auto calculate_vertex_pos_and_normal = [&](VertexType& v, const XMFLOAT3& cube_vertex_pos) -> void {
		//Create a vector from the center to the position
		XMVECTOR target_position = XMLoadFloat3(&cube_vertex_pos);
		//Calculate its unit normal with the calculated vector
		target_position = XMVector3Normalize(target_position);
		XMStoreFloat3(&v.normal, target_position);
		//Assign the new vertex position
		target_position *= radius_;
		float noise_value = 0.f;
		float f = noise_frequency_;
		float a = noise_amplitude_;
		for(unsigned i = 0u; i < noise_layer_iterations_; ++i)
		{
			noise_value += .5f * a * (static_cast<float>(ImprovedNoise::noise(
				(XMVectorGetX(target_position) + noise_center_.x) * f,
				(XMVectorGetY(target_position) + noise_center_.y) * f,
				(XMVectorGetZ(target_position) + noise_center_.z) * f)) + 1.f
			);
			f *= noise_layer_roughness_;
			a *= noise_layer_persistence_;
		}
		noise_value = max(0.f, noise_value - noise_min_threshold_);
		XMVECTOR noise_displacement = target_position * noise_value;
		XMStoreFloat3(&v.position, target_position + noise_displacement);
	};

	//front face
	for (unsigned y = 0; y < resolution_; y++)	// for each quad in the y direction
	{
		for (unsigned x = 0; x < resolution_; x++)	// for each quad in the x direction
		{
			// Load the vertex array with data.
			// 0 - Bottom left. -1. -1. 0
			calculate_vertex_pos_and_normal(
				vertices[v],
				XMFLOAT3(xstart, ystart - yincrement, -1.0f)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//1 - Top right.	1.0, 1.0 0.0
			calculate_vertex_pos_and_normal(
				vertices[v],
				XMFLOAT3(xstart + xincrement, ystart, -1.0f)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv);

			indices[i] = i;
			v++;
			i++;

			//2 - Top left.	-1.0, 1.0
			calculate_vertex_pos_and_normal(
				vertices[v],
				XMFLOAT3(xstart, ystart, -1.0f)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv);

			indices[i] = i;
			v++;
			i++;

			//0 - Bottom left. -1. -1. 0
			calculate_vertex_pos_and_normal(
				vertices[v],
				XMFLOAT3(xstart, ystart - yincrement, -1.0f)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//3 - Bottom right.	1.0, -1.0, 0.0
			calculate_vertex_pos_and_normal(
				vertices[v],
				XMFLOAT3(xstart + xincrement, ystart - yincrement, -1.0f)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//1 - Top right.	1.0, 1.0 0.0
			calculate_vertex_pos_and_normal(
				vertices[v],
				XMFLOAT3(xstart + xincrement, ystart, -1.0f)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv);

			indices[i] = i;
			v++;
			i++;

			// increment x for the next vertex
			xstart += xincrement;
			// increment the tex coord in u too
			txu += txuinc;
		}

		// increment y for the next vertex line
		ystart -= yincrement;
		// reset x start pos
		xstart = -1.f;
		// reset tex coord u
		txu = 0.f;
		// increment tex coord in v for the next vertex line
		txv += txvinc;
	}

	//back face
	ystart = 1.f;
	xstart = 1.f;
	txv = 0.f;
	for (unsigned y = 0; y < resolution_; y++)	// for each quad in the y direction
	{
		for (unsigned x = 0; x < resolution_; x++)	// for each quad in the x direction
		{
			// Load the vertex array with data.
			//0
			calculate_vertex_pos_and_normal(
				vertices[v],
				XMFLOAT3(xstart, ystart - yincrement, 1.0f)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//2
			calculate_vertex_pos_and_normal(
				vertices[v],
				XMFLOAT3(xstart - xincrement, ystart, 1.0f)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv);

			indices[i] = i;
			v++;
			i++;

			//1
			calculate_vertex_pos_and_normal(
				vertices[v],
				XMFLOAT3(xstart, ystart, 1.0f)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv);

			indices[i] = i;
			v++;
			i++;

			//0
			calculate_vertex_pos_and_normal(
				vertices[v],
				XMFLOAT3(xstart, ystart - yincrement, 1.0f)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//3
			calculate_vertex_pos_and_normal(
				vertices[v],
				XMFLOAT3(xstart - xincrement, ystart - yincrement, 1.0f)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//2
			calculate_vertex_pos_and_normal(
				vertices[v],
				XMFLOAT3(xstart - xincrement, ystart, 1.0f)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv);

			indices[i] = i;
			v++;
			i++;

			// increment x for the next vertex
			xstart -= xincrement;
			// increment the tex coord in u too
			txu += txuinc;
		}

		// increment y for the next vertex line
		ystart -= yincrement;
		// reset x start pos
		xstart = 1.f;
		// reset tex coord u
		txu = 0.f;
		// increment tex coord in v for the next vertex line
		txv += txvinc;
	}

	//right face
	ystart = 1.f;
	xstart = -1.f;
	txv = 0.f;
	for (unsigned y = 0; y < resolution_; y++)	// for each quad in the y direction
	{
		for (unsigned x = 0; x < resolution_; x++)	// for each quad in the x direction
		{
			// Load the vertex array with data.
			//0
			calculate_vertex_pos_and_normal(
				vertices[v],
				XMFLOAT3(1.0f, ystart - yincrement, xstart)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//2
			calculate_vertex_pos_and_normal(
				vertices[v],
				XMFLOAT3(1.0f, ystart, xstart + xincrement)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv);

			indices[i] = i;
			v++;
			i++;

			//1
			calculate_vertex_pos_and_normal(
				vertices[v],
				XMFLOAT3(1.0f, ystart, xstart)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv);

			indices[i] = i;
			v++;
			i++;

			//0
			calculate_vertex_pos_and_normal(
				vertices[v],
				XMFLOAT3(1.0f, ystart - yincrement, xstart)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//3
			calculate_vertex_pos_and_normal(
				vertices[v],
				XMFLOAT3(1.0f, ystart - yincrement, xstart + xincrement)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//2
			calculate_vertex_pos_and_normal(
				vertices[v],
				XMFLOAT3(1.0f, ystart, xstart + xincrement)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv);

			indices[i] = i;
			v++;
			i++;

			// increment x for the next vertex
			xstart += xincrement;
			// increment the tex coord in u too
			txu += txuinc;
		}

		// increment y for the next vertex line
		ystart -= yincrement;
		// reset x start pos
		xstart = -1.f;
		// reset tex coord u
		txu = 0.f;
		// increment tex coord in v for the next vertex line
		txv += txvinc;
	}

	//left face
	ystart = 1.f;
	xstart = 1.f;
	txv = 0.f;
	for (unsigned y = 0; y < resolution_; y++)	// for each quad in the y direction
	{
		for (unsigned x = 0; x < resolution_; x++)	// for each quad in the x direction
		{
			// Load the vertex array with data.
			//0
			calculate_vertex_pos_and_normal(
				vertices[v],
				XMFLOAT3(-1.0f, ystart - yincrement, xstart)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//2
			calculate_vertex_pos_and_normal(
				vertices[v],
				XMFLOAT3(-1.0f, ystart, xstart - xincrement)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv);

			indices[i] = i;
			v++;
			i++;

			//1
			calculate_vertex_pos_and_normal(
				vertices[v],
				XMFLOAT3(-1.0f, ystart, xstart)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv);

			indices[i] = i;
			v++;
			i++;

			//0
			calculate_vertex_pos_and_normal(
				vertices[v],
				XMFLOAT3(-1.0f, ystart - yincrement, xstart)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//3
			calculate_vertex_pos_and_normal(
				vertices[v],
				XMFLOAT3(-1.0f, ystart - yincrement, xstart - xincrement)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//2
			calculate_vertex_pos_and_normal(
				vertices[v],
				XMFLOAT3(-1.0f, ystart, xstart - xincrement)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv);

			indices[i] = i;
			v++;
			i++;

			// increment x for the next vertex
			xstart -= xincrement;
			// increment the tex coord in u too
			txu += txuinc;
		}

		// increment y for the next vertex line
		ystart -= yincrement;
		// reset x start pos
		xstart = 1.f;
		// reset tex coord u
		txu = 0.f;
		// increment tex coord in v for the next vertex line
		txv += txvinc;
	}

	//top face
	ystart = 1.f;
	xstart = -1.f;
	txv = 0.f;
	for (unsigned y = 0; y < resolution_; y++)	// for each quad in the y direction
	{
		for (unsigned x = 0; x < resolution_; x++)	// for each quad in the x direction
		{
			// Load the vertex array with data.
			//0
			calculate_vertex_pos_and_normal(
				vertices[v],
				XMFLOAT3(xstart, 1.0f, ystart - yincrement)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//2
			calculate_vertex_pos_and_normal(
				vertices[v],
				XMFLOAT3(xstart + xincrement, 1.0f, ystart)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv);

			indices[i] = i;
			v++;
			i++;

			//1
			calculate_vertex_pos_and_normal(
				vertices[v],
				XMFLOAT3(xstart, 1.0f, ystart)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv);

			indices[i] = i;
			v++;
			i++;

			//0
			calculate_vertex_pos_and_normal(
				vertices[v],
				XMFLOAT3(xstart, 1.0f, ystart - yincrement)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//3
			calculate_vertex_pos_and_normal(
				vertices[v],
				XMFLOAT3(xstart + xincrement, 1.0f, ystart - yincrement)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//2
			calculate_vertex_pos_and_normal(
				vertices[v],
				XMFLOAT3(xstart + xincrement, 1.0f, ystart)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv);

			indices[i] = i;
			v++;
			i++;

			// increment x for the next vertex
			xstart += xincrement;
			// increment the tex coord in u too
			txu += txuinc;
		}

		// increment y for the next vertex line
		ystart -= yincrement;
		// reset x start pos
		xstart = -1.f;
		// reset tex coord u
		txu = 0.f;
		// increment tex coord in v for the next vertex line
		txv += txvinc;
	}


	//bottom face
	ystart = -1.f;
	xstart = -1.f;
	txv = 0.f;
	for (unsigned y = 0; y < resolution_; y++)	// for each quad in the y direction
	{
		for (unsigned x = 0; x < resolution_; x++)	// for each quad in the x direction
		{
			// Load the vertex array with data.
			//0
			calculate_vertex_pos_and_normal(
				vertices[v],
				XMFLOAT3(xstart, -1.0f, ystart + yincrement)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//2
			calculate_vertex_pos_and_normal(
				vertices[v],
				XMFLOAT3(xstart + xincrement, -1.0f, ystart)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv);

			indices[i] = i;
			v++;
			i++;

			//1
			calculate_vertex_pos_and_normal(
				vertices[v],
				XMFLOAT3(xstart, -1.0f, ystart)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv);

			indices[i] = i;
			v++;
			i++;

			//0
			calculate_vertex_pos_and_normal(
				vertices[v],
				XMFLOAT3(xstart, -1.0f, ystart + yincrement)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//3
			calculate_vertex_pos_and_normal(
				vertices[v],
				XMFLOAT3(xstart + xincrement, -1.0f, ystart + yincrement)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//2
			calculate_vertex_pos_and_normal(
				vertices[v],
				XMFLOAT3(xstart + xincrement, -1.0f, ystart)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv);

			indices[i] = i;
			v++;
			i++;

			// increment x for the next vertex
			xstart += xincrement;
			// increment the tex coord in u too
			txu += txuinc;
		}

		// increment y for the next vertex line
		ystart += yincrement;
		// reset x start pos
		xstart = -1.f;
		// reset tex coord u
		txu = 0.f;
		// increment tex coord in v for the next vertex line
		txv += txvinc;
	}

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;
	// Now create the vertex buffer.
	device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;
	// Create the index buffer.
	device->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	delete[] vertices;
	vertices = 0;
	delete[] indices;
	indices = 0;
}