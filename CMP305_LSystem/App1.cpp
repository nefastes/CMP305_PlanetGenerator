// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"
App1::App1() :
	fabrik_goal_position(XMFLOAT3(0.f, 1.f, 0.f)),
	fabrik_n_segments(1),
	fabrik_total_length(1.f),
	fabrik_animate_with_noise(false),
	fabrik_animate_noise_offset(0.f, 0.f),
	gui_debug_noise(0.f),
	gui_wind_direction(XMFLOAT2(1.f, 0.f)),
	gui_wind_strength(1.f),
	fabrik_render_cylinders(false),
	gui_planet_noise_n_layers(1),
	gui_planet_rotation(XMFLOAT3(0.f, 0.f, 0.f)),
	settings_filename("planet_example_1"),
	gui_planet_shader_material_thresholds(XMFLOAT4(.75f, .5f, .1f, .01f)),
	gui_planet_generate_on_input(true)
{
}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);
	
	//Seed the rng
	//srand(1);
	srand(time(0));

	//Load textures
	textureMgr->loadTexture(L"grass", L"res/grass.png");
	textureMgr->loadTexture(L"wood", L"res/wood.png");

	// Create Mesh object and shader object
	m_ground = std::make_unique<PlaneMesh>(renderer->getDevice(), renderer->getDeviceContext());
	light_shader = std::make_unique<LightShader>(renderer->getDevice(), hwnd);
	planet_shader = std::make_unique<PlanetShader>(renderer->getDevice(), hwnd);

	light = std::make_unique<Light>();
	light->setAmbientColour(0.25f, 0.25f, 0.25f, 1.0f);
	light->setDiffuseColour(0.75f, 0.75f, 0.75f, 1.0f);
	light->setDirection(1.0f, -.7f, 0.0f);

	camera->setPosition(0.0f, 1.0f, -3.0f);
	camera->setRotation(0.0f, 0.0f, 00.0f);

	//Fabrik
	fabrik_goal_mesh = std::make_unique<SphereMesh>(renderer->getDevice(), renderer->getDeviceContext());
	fabrik_mesh = std::make_unique<FabrikMesh>(renderer->getDevice(), renderer->getDeviceContext(), XMFLOAT3(0.f, 0.f, 0.f), XMFLOAT3(0.f, 1.f, 0.f), 1, 1.f);

	//
	unsigned sideSize = (unsigned)sqrt(grass_sprouts.size());
	for (unsigned i = 0u; i < sideSize; ++i)
	{
		for (unsigned j = 0u; j < sideSize; ++j)
		{
			grass_sprouts[i * sideSize + j] = std::make_unique<FabrikMesh>(
				renderer->getDevice(),
				renderer->getDeviceContext(),
				XMFLOAT3((float)j * DISTANCE_BETWEEN_GRASS_SPROUTS - (sideSize / 5u), 0.f, (float)i * DISTANCE_BETWEEN_GRASS_SPROUTS - (sideSize / 5u)),
				XMFLOAT3(0.f, .5f, .5f),
				4,
				.5
			);
			grass_sprouts[i * sideSize + j]->update(renderer->getDevice(), renderer->getDeviceContext());
			grass_sprouts[i * sideSize + j]->BuildLine(renderer->getDeviceContext(), renderer->getDevice());
		}
	}


	//Planet
	planet_mesh = std::make_unique<PlanetMesh>(renderer->getDevice(), renderer->getDeviceContext(), hwnd);
}

App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();
}

bool App1::frame()
{
	bool result;
	float frame_time = timer->getTime();

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}
	
	if (fabrik_animate_with_noise)
	{
		fabrik_animate_noise_offset.x += frame_time;
		fabrik_animate_noise_offset.y += frame_time;
		float noise = (float)ImprovedNoise::noise((double)fabrik_animate_noise_offset.x, (double)fabrik_animate_noise_offset.y);
		noise += .5f;	//The above already returns a values from -0.5 to 0.5
		gui_debug_noise = noise;
		fabrik_goal_position = XMFLOAT3(0.f + gui_wind_strength * gui_wind_direction.x * noise, 1.f, 0.f + gui_wind_strength * gui_wind_direction.y * noise);
		fabrik_mesh->setGoal(fabrik_goal_position);
		//fabrik_mesh->setGoal(XMFLOAT3(fabrik_goal_position.x + noise, fabrik_goal_position.y, fabrik_goal_position.z + noise));
		for (unsigned i = 0u; i < grass_sprouts.size(); ++i)
		{
			noise = (float)ImprovedNoise::noise((double)fabrik_animate_noise_offset.x + grass_sprouts[i]->getPosition().x, (double)fabrik_animate_noise_offset.y + grass_sprouts[i]->getPosition().z);
			noise += .5f;	//The above already returns a values from -0.5 to 0.5
			fabrik_goal_position = XMFLOAT3(0.f + gui_wind_strength * gui_wind_direction.x * noise, .5f, 0.f + gui_wind_strength * gui_wind_direction.y * noise);

			XMFLOAT3 new_pos;
			XMStoreFloat3(&new_pos, XMVectorAdd(XMLoadFloat3(&grass_sprouts[i]->getPosition()), XMLoadFloat3(&fabrik_goal_position)));
			grass_sprouts[i]->setGoal(new_pos);
			grass_sprouts[i]->update(renderer->getDevice(), renderer->getDeviceContext());
			if (fabrik_render_cylinders) grass_sprouts[i]->BuildCylinders(renderer->getDevice(), renderer->getDeviceContext());
			else grass_sprouts[i]->BuildLine(renderer->getDeviceContext(), renderer->getDevice());
		}
		fabrik_goal_position.y = 1.f;
	}
	//Upadte the FABRIK
	fabrik_mesh->update(renderer->getDevice(), renderer->getDeviceContext());
	//Build the mesh again (TODO: could this be avoided?)
	if(fabrik_render_cylinders) fabrik_mesh->BuildCylinders(renderer->getDevice(), renderer->getDeviceContext());
	else fabrik_mesh->BuildLine(renderer->getDeviceContext(), renderer->getDevice());

	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	return true;
}

bool App1::render()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;

	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);

	// Generate the view matrix based on the camera's position.
	camera->update();

	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	worldMatrix = renderer->getWorldMatrix();
	viewMatrix = camera->getViewMatrix();
	projectionMatrix = renderer->getProjectionMatrix();

	/*if (fabrik_mesh->getIndexCount())
	{
		if(fabrik_render_cylinders) fabrik_mesh->sendData(renderer->getDeviceContext(), D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		else fabrik_mesh->sendData(renderer->getDeviceContext());
		light_shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"grass"), light.get());
		light_shader->render(renderer->getDeviceContext(), fabrik_mesh->getIndexCount());
	}

	for (unsigned i = 0u; i < grass_sprouts.size(); ++i)
	{
		if (fabrik_render_cylinders) grass_sprouts[i]->sendData(renderer->getDeviceContext(), D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		else grass_sprouts[i]->sendData(renderer->getDeviceContext());
		light_shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"grass"), light.get());
		light_shader->render(renderer->getDeviceContext(), grass_sprouts[i]->getIndexCount());
	}

	worldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixScaling(.125f, .125f, .125f));
	worldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixTranslation(-5.f, 0.f, -5.f));
	m_ground->sendData(renderer->getDeviceContext());
	light_shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"grass"), light.get());
	light_shader->render(renderer->getDeviceContext(), m_ground->getIndexCount());
	worldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixTranslation(5.f, 0.f, 5.f));

	worldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixTranslation(fabrik_goal_position.x, fabrik_goal_position.y, fabrik_goal_position.z));
	fabrik_goal_mesh->sendData(renderer->getDeviceContext());
	light_shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"wood"), light.get());
	light_shader->render(renderer->getDeviceContext(), fabrik_goal_mesh->getIndexCount());*/


	//Render the planet
	worldMatrix = XMMatrixRotationRollPitchYaw(gui_planet_rotation.y, gui_planet_rotation.z, gui_planet_rotation.x);
	planet_mesh->sendData(renderer->getDeviceContext());
	planet_shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, light.get(), gui_planet_shader_material_thresholds);
	planet_shader->render(renderer->getDeviceContext(), planet_mesh->getIndexCount());

	//Render the trees of the planet
	std::vector<Tree*>* trees = planet_mesh->getTrees();
	for (unsigned i = 0u; i < trees->size() && planet_mesh->isGenerating() != 3u; ++i)
	{
		Tree* current = trees->at(i);
		if (current == NULL || current == (Tree*)0xcdcdcdcdcdcdcdcd || current == (Tree*)0xdddddddddddddddd)
		{
			//Very aweful fix for the memory corruption!! DO NOT REUSE!!
			//This may prevent the program from crashing, but it doesn't deallocate the corrupted objects from memory!
			//Need to research more into this, however this is sadly not the focus of this work.
			trees->at(i) = NULL;
			continue;
		}
		current->render(
			renderer->getDeviceContext(),
			worldMatrix,
			viewMatrix,
			projectionMatrix,
			light.get()
		);
	}

	// Render GUI
	gui();

	// Swap the buffers
	renderer->endScene();

	return true;
}

void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Text("Camera Pos: (%.2f, %.2f, %.2f)", camera->getPosition().x, camera->getPosition().y, camera->getPosition().z);
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);

	if (ImGui::CollapsingHeader("Planet Settings"))
	{
		bool need_vertices_and_normals_generation = false;
		bool need_trees_generation = false;

		ImGui::Text("Current Save File:");
		ImGui::InputText("File Name", settings_filename, 64);
		if (ImGui::Button("Export", ImVec2(120, 20)))
		{
			if (planet_mesh->does_file_exist(settings_filename, 64))ImGui::OpenPopup("OverWrite");
			else
			{
				planet_mesh->ExportSettings(settings_filename, 64);
				ImGui::OpenPopup("Exported");
			}
		}
		if (ImGui::BeginPopupModal("OverWrite"))
		{
			ImGui::Text("File already exists. Overwrite?");
			if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
			ImGui::SameLine();
			if (ImGui::Button("OK"))
			{
				planet_mesh->ExportSettings(settings_filename, 64);
				ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
				ImGui::OpenPopup("Exported");
			}
			else ImGui::EndPopup();
		}
		if (ImGui::BeginPopupModal("Exported"))
		{
			ImGui::Text("Exportation Completed!");
			if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Import", ImVec2(120, 20)))
		{
			gui_planet_noise_n_layers = planet_mesh->ImportSettings(renderer->getDevice(), settings_filename, 64);
			planet_mesh->GenerateVertices();
			ImGui::OpenPopup("Generation Vertices");
		}

		ImGui::Separator();
		ImGui::Text("Mesh Settings:");
		need_vertices_and_normals_generation |= ImGui::Checkbox("Debug Generation", planet_mesh->getDebug());
		ImGui::Checkbox("Generate On Modification", &gui_planet_generate_on_input);
		if (!gui_planet_generate_on_input)
		{
			if (ImGui::Button("Generate Mesh"))
			{
				planet_mesh->GenerateVertices();
				ImGui::OpenPopup("Generation Vertices");
			}
			if (ImGui::Button("Generate Trees Only"))
			{
				planet_mesh->GenerateTrees(renderer->getDevice(), renderer->getDeviceContext(), wnd, gui_planet_shader_material_thresholds.z, gui_planet_shader_material_thresholds.y);
				ImGui::OpenPopup("Generation Trees");
			}
		}
		need_vertices_and_normals_generation |= ImGui::SliderInt("Resolution", (int*)planet_mesh->getResolution(), 1, 500);
		need_trees_generation |= ImGui::DragFloat("Tree Scale", planet_mesh->getTreeScale(), 0.001f);
		need_trees_generation |= ImGui::DragInt("N Trees Per Face", (int*)planet_mesh->getNumberTreesPerFace(), 1);
		need_trees_generation |= ImGui::DragFloat("Tree Max Angle Surface Normal", planet_mesh->getTreeNormalMaxAngle(), .01f, 0.f, 90.f);
		ImGui::DragFloat3("Roll Pitch Yaw", &gui_planet_rotation.x, .01f);

		ImGui::Separator();
		ImGui::Text("Shader Settings:");
		if (ImGui::Button("Reset Shader")) gui_planet_shader_material_thresholds = XMFLOAT4(.75f, .5f, .1f, .01f);
		need_trees_generation |= ImGui::DragFloat("Beach", &gui_planet_shader_material_thresholds.w, .001f);
		need_trees_generation |= ImGui::DragFloat("Grass", &gui_planet_shader_material_thresholds.z, .001f);
		need_trees_generation |= ImGui::DragFloat("Rock", &gui_planet_shader_material_thresholds.y, .001f);
		need_trees_generation |= ImGui::DragFloat("Snow", &gui_planet_shader_material_thresholds.x, .001f);
		
		ImGui::Separator();
		ImGui::Text("Layer Settings:");
		std::vector<std::unique_ptr<NoiseLayerSettings>>* noise_layers = planet_mesh->getNoiseLayers();
		if (ImGui::SliderInt("Number of Layers", &gui_planet_noise_n_layers, 1, 10))
		{
			int n_elements = noise_layers->size();
			for (int i = n_elements; i < gui_planet_noise_n_layers; ++i) noise_layers->push_back(std::make_unique<NoiseLayerSettings>());
			for (int i = n_elements; i > gui_planet_noise_n_layers; --i) noise_layers->pop_back();
			need_vertices_and_normals_generation |= noise_layers->size() != n_elements;
		}
		for (unsigned i = 0u; i < noise_layers->size(); ++i)
		{
			if (ImGui::TreeNode(std::to_string(i + 1u).insert(0, "Layer ").c_str()))
			{
				ImGui::PushID(i);
				NoiseLayerSettings* current_layer = noise_layers->at(i).get();

				need_vertices_and_normals_generation |= ImGui::Checkbox("Layer Active", &current_layer->layer_active_);
				need_vertices_and_normals_generation |= ImGui::Checkbox("Use Previous Layer As Mask", &current_layer->layer_use_previous_layer_as_mask_);
				ImGui::Text("Noise Settings:");
				NoiseType* current_type = &current_layer->noise_type_;
				need_vertices_and_normals_generation |= ImGui::Combo("Noise Type", (int*)current_type, "FBM\0Rigid\0\0");
				need_vertices_and_normals_generation |= ImGui::DragFloat("Noise frequency", &current_layer->noise_base_frequency_, 0.001f);
				need_vertices_and_normals_generation |= ImGui::DragFloat("Noise Base amplitude", &current_layer->noise_base_amplitude_, 0.001f);
				need_vertices_and_normals_generation |= ImGui::DragFloat("Noise Final amplitude", &current_layer->noise_final_amplitude_, 0.001f);
				need_vertices_and_normals_generation |= ImGui::DragFloat3("Noise center", &current_layer->layer_center_.x, .001f);
				need_vertices_and_normals_generation |= ImGui::SliderFloat("Min threshold", &current_layer->noise_min_threshold_, 0.f, 10.f);
				if (*current_type == NoiseType::RIGID)
				{
					need_vertices_and_normals_generation |= ImGui::DragFloat("Sharpness", &current_layer->rigid_noise_sharpness_, 0.01f);
					need_vertices_and_normals_generation |= ImGui::DragFloat("LOD Multiplier", &current_layer->rigid_noise_LOD_multiplier_, 0.01f);
				}
				ImGui::Text("Sub Layers Settings:");
				need_vertices_and_normals_generation |= ImGui::SliderInt("N layers", (int*)&current_layer->layer_nSubLayers_, 1, 20);
				need_vertices_and_normals_generation |= ImGui::SliderFloat("Layer roughness", &current_layer->layer_roughness_, 0.f, 10.f);
				need_vertices_and_normals_generation |= ImGui::SliderFloat("Layer persistence", &current_layer->layer_persistence_, 0.f, 1.f);

				ImGui::Separator();
				ImGui::PopID();
				ImGui::TreePop();
			}
		}
		if (need_vertices_and_normals_generation && gui_planet_generate_on_input)
		{
			planet_mesh->GenerateVertices();
			ImGui::OpenPopup("Generation Vertices");
		}
		if (need_trees_generation && gui_planet_generate_on_input)
		{
			planet_mesh->GenerateTrees(renderer->getDevice(), renderer->getDeviceContext(), wnd, gui_planet_shader_material_thresholds.z, gui_planet_shader_material_thresholds.y);
			ImGui::OpenPopup("Generation Trees");
		}
		if (ImGui::BeginPopupModal("Generation Vertices"))
		{
			if (planet_mesh->isGenerating() == 1u)
			{
				ImGui::Text("Generating Vertices...");
				ImGui::ProgressBar(planet_mesh->getGenerationProgress(), ImVec2(400, 20));
				ImGui::EndPopup();
			}
			else
			{
				ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
				planet_mesh->GenerateNormals();
				ImGui::OpenPopup("Generation Normals");
			}
		}
		if (ImGui::BeginPopupModal("Generation Normals"))
		{
			if (planet_mesh->isGenerating() == 2u)
			{
				ImGui::Text("Generating Normals...");
				ImGui::ProgressBar(planet_mesh->getGenerationProgress(), ImVec2(400, 20));
				ImGui::EndPopup();
			}
			else
			{
				ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
				planet_mesh->GenerateTrees(renderer->getDevice(), renderer->getDeviceContext(), wnd, gui_planet_shader_material_thresholds.z, gui_planet_shader_material_thresholds.y);
				ImGui::OpenPopup("Generation Trees");
			}
		}
		if (ImGui::BeginPopupModal("Generation Trees"))
		{
			if (planet_mesh->isGenerating() == 3u)
			{
				ImGui::Text("Generating Trees...");
				ImGui::ProgressBar(planet_mesh->getGenerationProgress(), ImVec2(400, 20));
				ImGui::EndPopup();
			}
			else
			{
				planet_mesh->GenerateMesh(renderer->getDevice());
				ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}
		}
	}
	if (ImGui::CollapsingHeader("LSystems"))
	{
		std::vector<Tree*>* trees = planet_mesh->getTrees();

		if (trees->empty()) ImGui::Text("Currently no trees have been generated");
		else
		{
			if (ImGui::Button("Run System"))
				for (unsigned i = 0u; i < trees->size(); ++i)
					trees->at(i)->runSystem(), trees->at(i)->build(renderer->getDevice(), renderer->getDeviceContext());

			ImGui::Text("System of the first generated tree:");
			ImGui::TextWrapped(trees->at(0)->getCurrentSystem().c_str());	//Only get the first tree as a debug output
		}
	}
	if (ImGui::CollapsingHeader("FABRIK"))
	{
		ImGui::Text("Goal:");
		if (ImGui::Checkbox("Render Cylinders", &fabrik_render_cylinders))
		{
			if (fabrik_render_cylinders)
			{
				fabrik_mesh->BuildCylinders(renderer->getDevice(), renderer->getDeviceContext());
				for (unsigned i = 0u; i < grass_sprouts.size(); ++i)
					grass_sprouts[i]->BuildCylinders(renderer->getDevice(), renderer->getDeviceContext());
			}
			else
			{
				fabrik_mesh->BuildLine(renderer->getDeviceContext(), renderer->getDevice());
				for (unsigned i = 0u; i < grass_sprouts.size(); ++i)
					grass_sprouts[i]->BuildLine(renderer->getDeviceContext(), renderer->getDevice());
			}
		}
		ImGui::Checkbox("Animate with Noise", &fabrik_animate_with_noise);
		ImGui::Text("Debug Noise Value: %.2f", gui_debug_noise);
		if (!fabrik_animate_with_noise)
			if (ImGui::SliderFloat3("Position", &fabrik_goal_position.x, -10.f, 10.f))
				fabrik_mesh->setGoal(fabrik_goal_position);
		ImGui::SliderFloat2("Wind Direction", &gui_wind_direction.x, 1.f, -1.f);
		ImGui::SliderFloat("Wind Strength", &gui_wind_strength, 0.f, 1.f);

		ImGui::Separator();

		ImGui::Text("Segments Control:");
		if (ImGui::SliderInt("N Segments", &fabrik_n_segments, 1, 10))
			fabrik_mesh->setSegmentCount(fabrik_n_segments);
		if (ImGui::SliderFloat("Total Length", &fabrik_total_length, .1f, 5.f))
			fabrik_mesh->setLength(fabrik_total_length);
	}

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}