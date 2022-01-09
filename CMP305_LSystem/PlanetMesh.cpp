#include "PlanetMesh.h"

PlanetMesh::PlanetMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, HWND hwnd, unsigned resolution) :
	resolution_(resolution), radius_(1.f), debug_building_(false), planet_tree_scale_(.075f), n_trees_per_face_(20u),
	tree_max_angle_to_normal_(10.f), current_task(0u), tree_system_n_iterations_(5u)
{
	noise_layers_.push_back(std::make_unique<NoiseLayerSettings>());
	GenerateVertices();
	GenerateNormals();
	GenerateTrees(device, deviceContext, hwnd, .1f, .5f);
	GenerateMesh(device);
}

PlanetMesh::~PlanetMesh()
{
	BaseMesh::~BaseMesh();
	//Release the vertex array if existent
	if (vertices != 0)
	{
		delete[] vertices;
		vertices = 0;
	}
	//Release the index array
	if (indices != 0)
	{
		delete[] indices;
		indices = 0;
	}
	//Clean planet trees
	for (size_t i = 0; i < planet_trees_.size(); ++i) delete planet_trees_[i], planet_trees_[i] = nullptr;
	planet_trees_.clear();
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
	//Release the vertex array if existent
	if (vertices != 0)
	{
		delete[] vertices;
		vertices = 0;
	}
	//Release the index array
	if (indices != 0)
	{
		delete[] indices;
		indices = 0;
	}
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
	current_task = 1u;
	farm.run();
}

void PlanetMesh::GenerateNormals()
{
	//This function should not be called before GenerateVertices has been called and a check of the farm status has completed
	//Clean the farm since the generation is complete
	farm.clean();

	//Recalculate normals
	//Set up normals for the face
	for (int k = 0; k < vertexCount; k += 3) {
		for (int l = 0; l < 3; ++l)
		{
			farm.add_task(new GenerateNormalsTask(
				&vertices[k + l].normal,
				vertices[k + warp(l, 0, 2)].position,
				vertices[k + warp(l + 1, 0, 2)].position,
				vertices[k + warp(l + 2, 0, 2)].position
			));
		}
	}
	current_task = 2u;
	farm.run();
}

void PlanetMesh::GenerateTrees(ID3D11Device* device, ID3D11DeviceContext* device_context, HWND hwnd, float grass_low_threshold, float grass_high_threshold)
{
	//This function should not be called before GeneratenNormals has been called and a check of the farm status has completed
	//Clean the farm since the generation is complete
	farm.clean();

	//Clean any trees that have been previously generated
	for (size_t i = 0; i < planet_trees_.size(); ++i) delete planet_trees_[i], planet_trees_[i] = nullptr;
	planet_trees_.clear();

	//Retrieve the number of vertices per face
	int n_triangles_per_face = vertexCount / 3 / 6;
	for (unsigned face_number = 0u; face_number < 6u; ++face_number)
	{
		for (unsigned short n = 0u; n < n_trees_per_face_; ++n)
		{
			//Generate random tree positions
			// rand() % n_triangles_per_face to randomly determine which triangle we spawn the tree on a face
			// add n_triangles_per_face * face_number to determine which face we operate on
			// multiply the end result by 3 so that the index represents the location of the first vertex of the triangle
			int index = 3 * (rand() % n_triangles_per_face + n_triangles_per_face * face_number);

			farm.add_task(new GenerateTreeTask(
				device, device_context, hwnd, tree_system_n_iterations_,
				grass_low_threshold, grass_high_threshold, planet_tree_scale_, tree_max_angle_to_normal_,
				vertices[index].position, vertices[index + 1].position, vertices[index + 2].position,
				vertices[index].normal, vertices[index + 1].normal, vertices[index + 2].normal,
				&planet_trees_
			));
		}
	}
	current_task = 3u;
	farm.run();
}

void PlanetMesh::GenerateMesh(ID3D11Device* device)
{	
	//This function should not be called before GenerateTrees has been called and a check of the farm status has completed
	//Clean the farm since the generation is complete
	farm.clean();
	current_task = 0u;

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
}

void PlanetMesh::RegenerateSystemTrees(ID3D11Device* device, ID3D11DeviceContext* device_context)
{
	//Check that no farming is occuring
	farm.clean();

	//Retrieve the number of vertices per face
	int n_triangles_per_face = vertexCount / 3 / 6;
	for(size_t i = 0; i < planet_trees_.size(); ++i)
		if(planet_trees_[i]) farm.add_task(new RegenerateTreeSystemTask(device, device_context, planet_trees_[i]));
	current_task = 4u;
	farm.run();
}

void PlanetMesh::initBuffers(ID3D11Device* device)
{
}
