#include "PlanetMesh.h"

PlanetMesh::PlanetMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, HWND hwnd, unsigned resolution) :
	resolution_(resolution), radius_(1.f), debug_building_(false), planet_tree_scale_(.075f)
{
	noise_layers_.push_back(std::make_unique<NoiseLayerSettings>());
	GenerateVertices();
	GenerateMesh(device, deviceContext, hwnd);
}

PlanetMesh::~PlanetMesh()
{
	BaseMesh::~BaseMesh();
}

bool PlanetMesh::does_file_exist(const char* filename, const int namesize)
{
	std::ifstream file;
	std::string path("planet_settings/");
	path.append(filename, namesize);
	file.open(path.c_str(), std::ofstream::binary);
	return file.good();
}

void PlanetMesh::ExportSettings(const char* filename, const int namesize)
{
	//Function to export the current settings to a binary file
	std::ofstream file;
	std::string path("planet_settings/");
	path.append(filename, namesize);
	file.open(path.c_str(), std::ofstream::binary);
	if (file.is_open())
	{
		//start by outputing the number of layers
		//will be needed for importation
		unsigned n_layers = noise_layers_.size();
		file.write((char*)&n_layers, sizeof(unsigned));

		//output all the current settings
		for (unsigned i = 0u; i < n_layers; ++i)
			file.write((char*)noise_layers_.at(i).get(), sizeof(NoiseLayerSettings));

		//remember to close the file!
		file.close();
	}
}

unsigned PlanetMesh::ImportSettings(ID3D11Device* device, const char* filename, const int namesize)
{
	//Function to import a set of settings from a binary file
	//Returns the number of layers that have been loaded
	unsigned n_layers = 0u;
	std::ifstream file;
	std::string path("planet_settings/");
	path.append(filename, namesize);
	file.open(path.c_str(), std::ofstream::binary);
	if (file.is_open())
	{
		//start by retrieving the number of players to import
		file.read((char*)&n_layers, sizeof(unsigned));

		//Make sure the vector of layers will accommodate exactly that number
		for (unsigned i = noise_layers_.size(); i < n_layers; ++i) noise_layers_.push_back(std::make_unique<NoiseLayerSettings>());
		for (unsigned i = noise_layers_.size(); i > n_layers; --i) noise_layers_.pop_back();

		//output all the current settings
		for (unsigned i = 0u; i < n_layers; ++i)
			file.read((char*)noise_layers_.at(i).get(), sizeof(NoiseLayerSettings));

		//remember to close the file!
		file.close();
	}
	return n_layers;
}

void PlanetMesh::GenerateVertices()
{
	//The cube sphere is generated with a radius of radius_ around the center (0,0,0)
	//Every vertex needs to be at the same distance from the center

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

	//front face
	for (unsigned y = 0; y < resolution_; y++)	// for each quad in the y direction
	{
		for (unsigned x = 0; x < resolution_; x++)	// for each quad in the x direction
		{
			// Load the vertex array with data.
			// 0 - Bottom left. -1. -1. 0
			farm.add_task(new GenerateMeshTask(
				&vertices[v].position,
				XMFLOAT3(xstart, ystart - yincrement, -1.0f),
				radius_,
				&noise_layers_
			));
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//1 - Top right.	1.0, 1.0 0.0
			farm.add_task(new GenerateMeshTask(
				&vertices[v].position,
				XMFLOAT3(xstart + xincrement, ystart, -1.0f),
				radius_,
				&noise_layers_
			));
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv);

			indices[i] = i;
			v++;
			i++;

			//2 - Top left.	-1.0, 1.0
			farm.add_task(new GenerateMeshTask(
				&vertices[v].position,
				XMFLOAT3(xstart, ystart, -1.0f),
				radius_,
				&noise_layers_
			));
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv);

			indices[i] = i;
			v++;
			i++;

			//0 - Bottom left. -1. -1. 0
			farm.add_task(new GenerateMeshTask(
				&vertices[v].position,
				XMFLOAT3(xstart, ystart - yincrement, -1.0f),
				radius_,
				&noise_layers_
			));
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//3 - Bottom right.	1.0, -1.0, 0.0
			farm.add_task(new GenerateMeshTask(
				&vertices[v].position,
				XMFLOAT3(xstart + xincrement, ystart - yincrement, -1.0f),
				radius_,
				&noise_layers_
			));
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//1 - Top right.	1.0, 1.0 0.0
			farm.add_task(new GenerateMeshTask(
				&vertices[v].position,
				XMFLOAT3(xstart + xincrement, ystart, -1.0f),
				radius_,
				&noise_layers_
			));
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
	if (debug_building_) goto run_farm;

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
			farm.add_task(new GenerateMeshTask(
				&vertices[v].position,
				XMFLOAT3(xstart, ystart - yincrement, 1.0f),
				radius_,
				&noise_layers_
			));
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//2
			farm.add_task(new GenerateMeshTask(
				&vertices[v].position,
				XMFLOAT3(xstart - xincrement, ystart, 1.0f),
				radius_,
				&noise_layers_
			));
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv);

			indices[i] = i;
			v++;
			i++;

			//1
			farm.add_task(new GenerateMeshTask(
				&vertices[v].position,
				XMFLOAT3(xstart, ystart, 1.0f),
				radius_,
				&noise_layers_
			));
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv);

			indices[i] = i;
			v++;
			i++;

			//0
			farm.add_task(new GenerateMeshTask(
				&vertices[v].position,
				XMFLOAT3(xstart, ystart - yincrement, 1.0f),
				radius_,
				&noise_layers_
			));
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//3
			farm.add_task(new GenerateMeshTask(
				&vertices[v].position,
				XMFLOAT3(xstart - xincrement, ystart - yincrement, 1.0f),
				radius_,
				&noise_layers_
			));
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//2
			farm.add_task(new GenerateMeshTask(
				&vertices[v].position,
				XMFLOAT3(xstart - xincrement, ystart, 1.0f),
				radius_,
				&noise_layers_
			));
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
			farm.add_task(new GenerateMeshTask(
				&vertices[v].position,
				XMFLOAT3(1.0f, ystart - yincrement, xstart),
				radius_,
				&noise_layers_
			));
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//2
			farm.add_task(new GenerateMeshTask(
				&vertices[v].position,
				XMFLOAT3(1.0f, ystart, xstart + xincrement),
				radius_,
				&noise_layers_
			));
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv);

			indices[i] = i;
			v++;
			i++;

			//1
			farm.add_task(new GenerateMeshTask(
				&vertices[v].position,
				XMFLOAT3(1.0f, ystart, xstart),
				radius_,
				&noise_layers_
			));
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv);

			indices[i] = i;
			v++;
			i++;

			//0
			farm.add_task(new GenerateMeshTask(
				&vertices[v].position,
				XMFLOAT3(1.0f, ystart - yincrement, xstart),
				radius_,
				&noise_layers_
			));
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//3
			farm.add_task(new GenerateMeshTask(
				&vertices[v].position,
				XMFLOAT3(1.0f, ystart - yincrement, xstart + xincrement),
				radius_,
				&noise_layers_
			));
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//2
			farm.add_task(new GenerateMeshTask(
				&vertices[v].position,
				XMFLOAT3(1.0f, ystart, xstart + xincrement),
				radius_,
				&noise_layers_
			));
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
			farm.add_task(new GenerateMeshTask(
				&vertices[v].position,
				XMFLOAT3(-1.0f, ystart - yincrement, xstart),
				radius_,
				&noise_layers_
			));
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//2
			farm.add_task(new GenerateMeshTask(
				&vertices[v].position,
				XMFLOAT3(-1.0f, ystart, xstart - xincrement),
				radius_,
				&noise_layers_
			));
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv);

			indices[i] = i;
			v++;
			i++;

			//1
			farm.add_task(new GenerateMeshTask(
				&vertices[v].position,
				XMFLOAT3(-1.0f, ystart, xstart),
				radius_,
				&noise_layers_
			));
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv);

			indices[i] = i;
			v++;
			i++;

			//0
			farm.add_task(new GenerateMeshTask(
				&vertices[v].position,
				XMFLOAT3(-1.0f, ystart - yincrement, xstart),
				radius_,
				&noise_layers_
			));
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//3
			farm.add_task(new GenerateMeshTask(
				&vertices[v].position,
				XMFLOAT3(-1.0f, ystart - yincrement, xstart - xincrement),
				radius_,
				&noise_layers_
			));
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//2
			farm.add_task(new GenerateMeshTask(
				&vertices[v].position,
				XMFLOAT3(-1.0f, ystart, xstart - xincrement),
				radius_,
				&noise_layers_
			));
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
			farm.add_task(new GenerateMeshTask(
				&vertices[v].position,
				XMFLOAT3(xstart, 1.0f, ystart - yincrement),
				radius_,
				&noise_layers_
			));
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//2
			farm.add_task(new GenerateMeshTask(
				&vertices[v].position,
				XMFLOAT3(xstart + xincrement, 1.0f, ystart),
				radius_,
				&noise_layers_
			));
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv);

			indices[i] = i;
			v++;
			i++;

			//1
			farm.add_task(new GenerateMeshTask(
				&vertices[v].position,
				XMFLOAT3(xstart, 1.0f, ystart),
				radius_,
				&noise_layers_
			));
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv);

			indices[i] = i;
			v++;
			i++;

			//0
			farm.add_task(new GenerateMeshTask(
				&vertices[v].position,
				XMFLOAT3(xstart, 1.0f, ystart - yincrement),
				radius_,
				&noise_layers_
			));
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//3
			farm.add_task(new GenerateMeshTask(
				&vertices[v].position,
				XMFLOAT3(xstart + xincrement, 1.0f, ystart - yincrement),
				radius_,
				&noise_layers_
			));
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//2
			farm.add_task(new GenerateMeshTask(
				&vertices[v].position,
				XMFLOAT3(xstart + xincrement, 1.0f, ystart),
				radius_,
				&noise_layers_
			));
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
			farm.add_task(new GenerateMeshTask(
				&vertices[v].position,
				XMFLOAT3(xstart, -1.0f, ystart + yincrement),
				radius_,
				&noise_layers_
			));
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//2
			farm.add_task(new GenerateMeshTask(
				&vertices[v].position,
				XMFLOAT3(xstart + xincrement, -1.0f, ystart),
				radius_,
				&noise_layers_
			));
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv);

			indices[i] = i;
			v++;
			i++;

			//1
			farm.add_task(new GenerateMeshTask(
				&vertices[v].position,
				XMFLOAT3(xstart, -1.0f, ystart),
				radius_,
				&noise_layers_
			));
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv);

			indices[i] = i;
			v++;
			i++;

			//0
			farm.add_task(new GenerateMeshTask(
				&vertices[v].position,
				XMFLOAT3(xstart, -1.0f, ystart + yincrement),
				radius_,
				&noise_layers_
			));
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//3
			farm.add_task(new GenerateMeshTask(
				&vertices[v].position,
				XMFLOAT3(xstart + xincrement, -1.0f, ystart + yincrement),
				radius_,
				&noise_layers_
			));
			//Assign the texture coordinate
			vertices[v].texture = XMFLOAT2(txu + txuinc, txv + txvinc);

			indices[i] = i;
			v++;
			i++;

			//2
			farm.add_task(new GenerateMeshTask(
				&vertices[v].position,
				XMFLOAT3(xstart + xincrement, -1.0f, ystart),
				radius_,
				&noise_layers_
			));
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

run_farm:
	//Run the thread farm to calculate the vertices
	farm.run();
}

void PlanetMesh::GenerateMesh(ID3D11Device* device, ID3D11DeviceContext* device_context, HWND hwnd)
{	
	//This function should not be called before GenerateVertices has been called and a check of the farm status has completed
	//Clean the farm since the generation is complete
	farm.clean();

	//Clean any trees that have been previously generated
	planet_trees_.clear();

	//Recalculate normals
	//Set up normals for the face
	for (int k = 0; k < vertexCount - 3; k += 3) {
		for (int l = 0; l < 3; ++l)
		{
			//Calculate the plane normals
			XMFLOAT3 a, b, c;	//Three corner vertices
			XMFLOAT3 cross;		//Cross product result
			float mag;			//Magnitude of the cross product (so we can normalize it)
			XMFLOAT3 ab;		//Edge 1
			XMFLOAT3 ac;		//Edge 2

			//Retrieve the three vertices
			a = vertices[k + warp(l, 0, 2)].position;
			b = vertices[k + warp(l + 1, 0, 2)].position;
			c = vertices[k + warp(l + 2, 0, 2)].position;

			//Two edges
			ab = XMFLOAT3(c.x - a.x, c.y - a.y, c.z - a.z);
			ac = XMFLOAT3(b.x - a.x, b.y - a.y, b.z - a.z);

			//Calculate the cross product
			cross.x = ab.y * ac.z - ab.z * ac.y;
			cross.y = ab.z * ac.x - ab.x * ac.z;
			cross.z = ab.x * ac.y - ab.y * ac.x;
			mag = (cross.x * cross.x) + (cross.y * cross.y) + (cross.z * cross.z);
			mag = sqrtf(mag);
			cross.x /= mag;
			cross.y /= mag;
			cross.z /= mag;
			vertices[k + l].normal = cross;
		}
		//1 % chance to spawn a tree on this triangle
		if (rand() % 100 < 1)
		{
			//Ensure the vertex is within grass distance
			XMVECTOR position = (XMLoadFloat3(&vertices[k].position) + XMLoadFloat3(&vertices[k + 1].position) + XMLoadFloat3(&vertices[k + 2].position)) / 3.f;
			float vertex_distance = XMVectorGetX(XMVector3Length(position)) - radius_;
			if (vertex_distance < .1f || vertex_distance > .5f) continue;

			//Scale the tree
			XMMATRIX transform = XMMatrixScaling(planet_tree_scale_, planet_tree_scale_, planet_tree_scale_);
			//Rotate the tree so that it's up direction is along the surface normal
			XMVECTOR up = XMVector3Normalize(position);
			XMVECTOR normal = (XMLoadFloat3(&vertices[k].normal) + XMLoadFloat3(&vertices[k + 1].normal) + XMLoadFloat3(&vertices[k + 2].normal)) / 3.f;
			XMVECTOR rotation_axis = XMVector3Cross(up, normal);
			float rotation_angle = XMVectorGetX(XMVector3AngleBetweenNormals(up, normal));
			if (rotation_angle < -10.f || rotation_angle > 10.f) continue;	//Ensure a tree can't be placed on a steep surface
			transform = XMMatrixMultiply(transform, XMMatrixRotationAxis(rotation_axis, rotation_angle));
			//Translate the tree to the vertex's position
			transform = XMMatrixMultiply(transform, XMMatrixTranslation(XMVectorGetX(position), XMVectorGetY(position), XMVectorGetZ(position)));
			planet_trees_.push_back(std::make_unique<Tree>(
				device,
				device_context,
				hwnd,
				transform
				));
		}
	}

	//Check if the cube sphere was already generated, in the event of regeneration
	if (vertexBuffer) vertexBuffer->Release(), vertexBuffer = NULL;
	if (indexBuffer) indexBuffer->Release(), indexBuffer = NULL;

	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
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

void PlanetMesh::initBuffers(ID3D11Device* device)
{

}
