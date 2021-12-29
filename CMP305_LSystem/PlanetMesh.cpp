#include "PlanetMesh.h"

PlanetMesh::PlanetMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, unsigned resolution, float radius,
	float noise_frequency, float noise_amplitude, XMFLOAT3 noise_center, float noise_min_threshold, unsigned noise_layers,
	float noise_layer_roughness, float noise_layer_persistence) :
	resolution_(resolution), radius_(radius), debug_building_(true)
{
	noise_layers_.push_back(
		std::make_unique<NoiseLayerSettings>(
			noise_layer_roughness,
			noise_layer_persistence,
			noise_layers,
			noise_center,
			noise_frequency,
			noise_amplitude,
			noise_min_threshold
		)
	);
	initBuffers(device);
}

PlanetMesh::~PlanetMesh()
{
	BaseMesh::~BaseMesh();
}

void PlanetMesh::Regenrate(ID3D11Device* device)
{
	initBuffers(device);
}

void PlanetMesh::initBuffers(ID3D11Device* device)
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
	auto calculate_vertex_pos = [&](VertexType& v, const XMFLOAT3& cube_vertex_pos) -> void {
		//Create a vector from the center to the position
		XMVECTOR target_position = XMLoadFloat3(&cube_vertex_pos);
		//Calculate its unit normal with the calculated vector
		XMVECTOR unit_target_position = XMVector3Normalize(target_position);
		XMStoreFloat3(&v.normal, target_position);
		//Assign the new vertex position
		target_position = unit_target_position * radius_;
		float total_noise = 0.f;
		float layer_mask = 1.f; //Start with 1.f in case the user ticks the box for layer 0
		for (unsigned i = 0u; i < noise_layers_.size(); ++i)
		{
			//Get the current layer settings
			NoiseLayerSettings* layer = noise_layers_[i].get();
			//If the layer is deactivated, continue to deal with the next layer
			if (!layer->layer_active_)continue;
			//Setup base noise values
			float noise_value = 0.f;
			float f = layer->noise_base_frequency_;
			float a = layer->noise_base_amplitude_;
			float rigid_noise_LOD = 1.f;	//only used by rigid noise
			//For every sub layers, calculate the FBM noise
			for (unsigned j = 0u; j < layer->layer_nSubLayers_; ++j)
			{
				/*noise_value += .5f * a * (static_cast<float>(ImprovedNoise::noise(
					(XMVectorGetX(target_position) + layer->layer_center_.x) * f,
					(XMVectorGetY(target_position) + layer->layer_center_.y) * f,
					(XMVectorGetZ(target_position) + layer->layer_center_.z) * f)) + 1.f);*/
				/*noise_value += a * (1.f - std::abs(static_cast<float>(ImprovedNoise::noise(
					(XMVectorGetX(target_position) + layer->layer_center_.x) * f,
					(XMVectorGetY(target_position) + layer->layer_center_.y) * f,
					(XMVectorGetZ(target_position) + layer->layer_center_.z) * f))));*/
				float noise = static_cast<float>(ImprovedNoise::noise(
					(XMVectorGetX(target_position) + layer->layer_center_.x) * f,
					(XMVectorGetY(target_position) + layer->layer_center_.y) * f,
					(XMVectorGetZ(target_position) + layer->layer_center_.z) * f));
				switch (layer->noise_type_)
				{
				case NoiseType::FBM:
					//Noise returned is range [-1,1]
					//Simply convert it in the range [0,1] and multiply by amplitude
					noise_value += a * .5f * (noise + 1.f);
					break;
				case NoiseType::RIGID:
					{
						//Noise is in range [-1,1]
						//Mirror the negative part to positive, i.e. do the absolute value, this will result in values smoothly
						//transitioning from 0 to 1 to 0
						//Substract the absolute value from 1 so that we get the inverse effect: the noise will peak from 0 to 1 to 0
						//Repeat the operation for sharper results (i.e. pow())
						//Multiply the result of the above by  a scalar to gain details from layer to layer (LOD)
						float rigid_noise = powf(1.f - std::abs(noise), layer->rigid_noise_sharpness_) * rigid_noise_LOD;
						//The LOD for the next layer is the LOD of the previous one multiplied by a scalar
						//Need to make sure the LOD stays between 0 and 1, otherwise it will act as an amplitude instead of tiny details
						rigid_noise_LOD = clamp(rigid_noise * layer->rigid_noise_LOD_multiplier_, 0.f, 1.f);
						//The final noise value is the calculated rigid noise multiplied by an amplitude
						noise_value += a * rigid_noise;
					}
					break;
				default:
					break;
				}
				f *= layer->layer_roughness_;
				a *= layer->layer_persistence_;
			}
			//This operation reduces the displacement dictated by the threshold value
			//It ensures it cannot go negative, so the minimum distance from the center will be radius_
			//This allows to create round water and noisy continents
			noise_value = max(0.f, noise_value - layer->noise_min_threshold_);
			//Add the calculated noise value to the total noise
			total_noise += noise_value * (layer->layer_use_previous_layer_as_mask_ ? layer_mask : 1.f);
			//Update the mask with this layer's noise value for the next layer
			layer_mask = noise_value;
		}
		//The noise displacement is a simple multiplication of the unit vector of the vertex position from the center with the total noise
		XMVECTOR noise_displacement = unit_target_position * total_noise;
		//The final position is then the traget position (unit vector * radius_) plus noisy displacement
		XMStoreFloat3(&v.position, target_position + noise_displacement);
	};

	//front face
	for (unsigned y = 0; y < resolution_; y++)	// for each quad in the y direction
	{
		for (unsigned x = 0; x < resolution_; x++)	// for each quad in the x direction
		{
			// Load the vertex array with data.
			// 0 - Bottom left. -1. -1. 0
			calculate_vertex_pos(
				vertices[v],
				XMFLOAT3(xstart, ystart - yincrement, -1.0f)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//1 - Top right.	1.0, 1.0 0.0
			calculate_vertex_pos(
				vertices[v],
				XMFLOAT3(xstart + xincrement, ystart, -1.0f)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv);

			indices[i] = i;
			v++;
			i++;

			//2 - Top left.	-1.0, 1.0
			calculate_vertex_pos(
				vertices[v],
				XMFLOAT3(xstart, ystart, -1.0f)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv);

			indices[i] = i;
			v++;
			i++;

			//0 - Bottom left. -1. -1. 0
			calculate_vertex_pos(
				vertices[v],
				XMFLOAT3(xstart, ystart - yincrement, -1.0f)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//3 - Bottom right.	1.0, -1.0, 0.0
			calculate_vertex_pos(
				vertices[v],
				XMFLOAT3(xstart + xincrement, ystart - yincrement, -1.0f)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//1 - Top right.	1.0, 1.0 0.0
			calculate_vertex_pos(
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

	//If we are debugging, do not render anymore faces
	if (debug_building_) goto init_buffers;

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
			calculate_vertex_pos(
				vertices[v],
				XMFLOAT3(xstart, ystart - yincrement, 1.0f)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//2
			calculate_vertex_pos(
				vertices[v],
				XMFLOAT3(xstart - xincrement, ystart, 1.0f)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv);

			indices[i] = i;
			v++;
			i++;

			//1
			calculate_vertex_pos(
				vertices[v],
				XMFLOAT3(xstart, ystart, 1.0f)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv);

			indices[i] = i;
			v++;
			i++;

			//0
			calculate_vertex_pos(
				vertices[v],
				XMFLOAT3(xstart, ystart - yincrement, 1.0f)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//3
			calculate_vertex_pos(
				vertices[v],
				XMFLOAT3(xstart - xincrement, ystart - yincrement, 1.0f)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//2
			calculate_vertex_pos(
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
			calculate_vertex_pos(
				vertices[v],
				XMFLOAT3(1.0f, ystart - yincrement, xstart)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//2
			calculate_vertex_pos(
				vertices[v],
				XMFLOAT3(1.0f, ystart, xstart + xincrement)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv);

			indices[i] = i;
			v++;
			i++;

			//1
			calculate_vertex_pos(
				vertices[v],
				XMFLOAT3(1.0f, ystart, xstart)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv);

			indices[i] = i;
			v++;
			i++;

			//0
			calculate_vertex_pos(
				vertices[v],
				XMFLOAT3(1.0f, ystart - yincrement, xstart)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//3
			calculate_vertex_pos(
				vertices[v],
				XMFLOAT3(1.0f, ystart - yincrement, xstart + xincrement)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//2
			calculate_vertex_pos(
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
			calculate_vertex_pos(
				vertices[v],
				XMFLOAT3(-1.0f, ystart - yincrement, xstart)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//2
			calculate_vertex_pos(
				vertices[v],
				XMFLOAT3(-1.0f, ystart, xstart - xincrement)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv);

			indices[i] = i;
			v++;
			i++;

			//1
			calculate_vertex_pos(
				vertices[v],
				XMFLOAT3(-1.0f, ystart, xstart)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv);

			indices[i] = i;
			v++;
			i++;

			//0
			calculate_vertex_pos(
				vertices[v],
				XMFLOAT3(-1.0f, ystart - yincrement, xstart)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//3
			calculate_vertex_pos(
				vertices[v],
				XMFLOAT3(-1.0f, ystart - yincrement, xstart - xincrement)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//2
			calculate_vertex_pos(
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
			calculate_vertex_pos(
				vertices[v],
				XMFLOAT3(xstart, 1.0f, ystart - yincrement)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//2
			calculate_vertex_pos(
				vertices[v],
				XMFLOAT3(xstart + xincrement, 1.0f, ystart)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv);

			indices[i] = i;
			v++;
			i++;

			//1
			calculate_vertex_pos(
				vertices[v],
				XMFLOAT3(xstart, 1.0f, ystart)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv);

			indices[i] = i;
			v++;
			i++;

			//0
			calculate_vertex_pos(
				vertices[v],
				XMFLOAT3(xstart, 1.0f, ystart - yincrement)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//3
			calculate_vertex_pos(
				vertices[v],
				XMFLOAT3(xstart + xincrement, 1.0f, ystart - yincrement)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//2
			calculate_vertex_pos(
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
			calculate_vertex_pos(
				vertices[v],
				XMFLOAT3(xstart, -1.0f, ystart + yincrement)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//2
			calculate_vertex_pos(
				vertices[v],
				XMFLOAT3(xstart + xincrement, -1.0f, ystart)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv);

			indices[i] = i;
			v++;
			i++;

			//1
			calculate_vertex_pos(
				vertices[v],
				XMFLOAT3(xstart, -1.0f, ystart)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv);

			indices[i] = i;
			v++;
			i++;

			//0
			calculate_vertex_pos(
				vertices[v],
				XMFLOAT3(xstart, -1.0f, ystart + yincrement)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//3
			calculate_vertex_pos(
				vertices[v],
				XMFLOAT3(xstart + xincrement, -1.0f, ystart + yincrement)
			);
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//2
			calculate_vertex_pos(
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

init_buffers:
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