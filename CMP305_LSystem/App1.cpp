// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"
#include <stack>
App1::App1() :
	lSystem("A"),
	lSystem_nIterations(1),
	lSystem_BuildType(0),
	lSystem_UseCylinders(false),
	fabrik_goal_position(XMFLOAT3(0.f, 1.f, 0.f)),
	fabrik_n_segments(1),
	fabrik_total_length(1.f),
	fabrik_animate_with_noise(false),
	fabrik_animate_noise_offset(0.f, 0.f),
	gui_debug_noise(0.f),
	gui_wind_direction(XMFLOAT2(1.f, 0.f)),
	gui_wind_strength(1.f),
	fabrik_render_cylinders(false),
	gui_planet_resolution(20),
	gui_planet_radius(1.f),
	gui_noise_frequency(0.f),
	gui_noise_amplitude(0.f),
	gui_noise_center(XMFLOAT3(0.f, 0.f, 0.f))
{
}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	textureMgr->loadTexture(L"grass", L"res/grass.png");
	textureMgr->loadTexture(L"wood", L"res/wood.png");

	// Create Mesh object and shader object
	m_ground = std::make_unique<PlaneMesh>(renderer->getDevice(), renderer->getDeviceContext());
	m_Line.reset( new LineMesh( renderer->getDevice(), renderer->getDeviceContext() ) );
	shader.reset( new LightShader( renderer->getDevice(), hwnd ) );

	light.reset( new Light );
	light->setAmbientColour(0.25f, 0.25f, 0.25f, 1.0f);
	light->setDiffuseColour(0.75f, 0.75f, 0.75f, 1.0f);
	light->setDirection(1.0f, -.7f, 0.0f);

	camera->setPosition(0.0f, 1.0f, -3.0f);
	camera->setRotation(0.0f, 0.0f, 00.0f);

	////Add rules to the Lsystem
	//Below to test lSystem
	//lSystem.AddRule('A', "AB");
	//lSystem.AddRule('B', "A");

	//The application starts with the 2D tree rules
	lSystem.AddRule('B', "BB");
	lSystem.AddRule('A', "B[A]A");
	lSystem.AddRule('[', "[");
	lSystem.AddRule(']', "]");

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
	planet_mesh = std::make_unique<CubeSphereMesh>(renderer->getDevice(), renderer->getDeviceContext(), gui_planet_resolution, gui_planet_radius,
		gui_noise_frequency, gui_noise_amplitude, gui_noise_center);
}

App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	//Delete 3d objects
	m_3dtree_branches.clear();
	m_3dtree_leaves.clear();
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
	/*if (m_Line->getIndexCount() > 0) {
		shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"grass"), light.get());
		m_Line->sendData(renderer->getDeviceContext());
		shader->render(renderer->getDeviceContext(), m_Line->getIndexCount());
	}
	for (unsigned i = 0u; i < m_3dtree_branches.size(); ++i)
	{
		shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * m_3dtree_branches[i]->m_Transform, viewMatrix, projectionMatrix, textureMgr->getTexture(L"wood"), light.get());
		m_3dtree_branches[i]->sendData(renderer->getDeviceContext());
		shader->render(renderer->getDeviceContext(), m_3dtree_branches[i]->getIndexCount());
	}
	for (unsigned i = 0u; i < m_3dtree_leaves.size(); ++i)
	{
		shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * m_3dtree_leaves[i]->m_Transform, viewMatrix, projectionMatrix, textureMgr->getTexture(L"grass"), light.get());
		m_3dtree_leaves[i]->sendData(renderer->getDeviceContext());
		shader->render(renderer->getDeviceContext(), m_3dtree_leaves[i]->getIndexCount());
	}

	if (fabrik_mesh->getIndexCount())
	{
		if(fabrik_render_cylinders) fabrik_mesh->sendData(renderer->getDeviceContext(), D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		else fabrik_mesh->sendData(renderer->getDeviceContext());
		shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"grass"), light.get());
		shader->render(renderer->getDeviceContext(), fabrik_mesh->getIndexCount());
	}

	for (unsigned i = 0u; i < grass_sprouts.size(); ++i)
	{
		if (fabrik_render_cylinders) grass_sprouts[i]->sendData(renderer->getDeviceContext(), D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		else grass_sprouts[i]->sendData(renderer->getDeviceContext());
		shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"grass"), light.get());
		shader->render(renderer->getDeviceContext(), grass_sprouts[i]->getIndexCount());
	}

	worldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixScaling(.125f, .125f, .125f));
	worldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixTranslation(-5.f, 0.f, -5.f));
	m_ground->sendData(renderer->getDeviceContext());
	shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"grass"), light.get());
	shader->render(renderer->getDeviceContext(), m_ground->getIndexCount());
	worldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixTranslation(5.f, 0.f, 5.f));

	worldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixTranslation(fabrik_goal_position.x, fabrik_goal_position.y, fabrik_goal_position.z));
	fabrik_goal_mesh->sendData(renderer->getDeviceContext());
	shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"wood"), light.get());
	shader->render(renderer->getDeviceContext(), fabrik_goal_mesh->getIndexCount());*/


	worldMatrix = renderer->getWorldMatrix();
	planet_mesh->sendData(renderer->getDeviceContext());
	shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"grass"), light.get());
	shader->render(renderer->getDeviceContext(), planet_mesh->getIndexCount());

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

	if (ImGui::CollapsingHeader("LSystems"))
	{
		ImGui::SliderInt("nIterations", &lSystem_nIterations, 1, 15);
		if (ImGui::RadioButton("2D Tree", &lSystem_BuildType, 0))
		{
			lSystem.SetAxiom("A");
			//Branching binary tree rules from lab sheet
			lSystem.ClearRules();
			lSystem.AddRule('B', "BB");
			lSystem.AddRule('A', "B[A]A");
			lSystem.AddRule('[', "[");
			lSystem.AddRule(']', "]");
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("3D Tree", &lSystem_BuildType, 1))
		{
			lSystem.SetAxiom("FA");
			//3D tree rules from lab sheet
			lSystem.ClearRules();
			lSystem.AddRule('A', "[&FA][<&FA][>&FA]");
			lSystem.AddRule('F', "F");
			lSystem.AddRule('[', "[");
			lSystem.AddRule(']', "]");
			lSystem.AddRule('&', "&");
			lSystem.AddRule('>', ">");
			lSystem.AddRule('<', "<");
		}
		if (ImGui::Checkbox("Use Cylinders", &lSystem_UseCylinders))
		{
			if (lSystem_UseCylinders) m_Line->Clear();
			else {
				m_3dtree_branches.clear();
				m_3dtree_leaves.clear();
			}
		}
		if (ImGui::Button("Iterate"))
		{
			lSystem.Iterate();
			if (lSystem_BuildType == 0) BuildLine2D();
			else BuildTree3D();
		}
		if (ImGui::Button("Run System"))
		{
			lSystem.Run((unsigned)lSystem_nIterations);
			if (lSystem_BuildType == 0) BuildLine2D();
			else BuildTree3D();
		}

		ImGui::LabelText(lSystem.GetAxiom().c_str(), "Axiom:");

		ImGui::Text("System:");
		ImGui::TextWrapped(lSystem.GetCurrentSystem().c_str());
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
	if (ImGui::CollapsingHeader("Planet Settings"))
	{
		bool need_generation = false;
		need_generation |= ImGui::SliderInt("Resolution", &gui_planet_resolution, 1, 100);
		need_generation |= ImGui::SliderFloat("Radius", &gui_planet_radius, .1f, 100.f);
		need_generation |= ImGui::SliderFloat("Noise frequency", &gui_noise_frequency, 0.f, 1.f);
		need_generation |= ImGui::SliderFloat("Noise amplitude", &gui_noise_amplitude, 0.f, 10.f);
		need_generation |= ImGui::SliderFloat3("Noise center", &gui_noise_center.x, -10.f, 10.f);
		if (need_generation)
			planet_mesh->Regenrate(renderer->getDevice(), static_cast<unsigned>(gui_planet_resolution), gui_planet_radius,
				gui_noise_frequency, gui_noise_amplitude, gui_noise_center);
	}

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void App1::BuildLine2D()
{
	//Clear any lines we might already have
	m_Line->Clear();

	//Get the current L-System string, right now we have a place holder
	std::string systemString = lSystem.GetCurrentSystem();

	//Initialise some variables
	XMVECTOR pos = XMVectorReplicate(0.0f);	//Current position (0,0,0)
	XMVECTOR dir = XMVectorSet(0, 1, 0, 1);	//Current direction is "Up"
	XMVECTOR fwd = XMVectorSet(0, 0, 1, 1);	//Rotation axis. Our rotations happen around the "forward" vector
	XMMATRIX currentRotation = XMMatrixRotationRollPitchYaw(0, 0, 0);
	std::vector<XMMATRIX> rotation_stack;
	std::vector<XMVECTOR> pos_stack;

	//Go through the L-System string
	for (int i = 0; i < systemString.length(); i++) {
		switch (systemString[i]) {			
			case 'A':	//Draw a line segment
				m_Line->AddLine(pos, pos + XMVector3Transform(dir, currentRotation));	//Add the line segment to the line mesh
				//TODO: draw leaf
				break;			
			case 'B':	//Draw a line segment and move forward
				m_Line->AddLine(pos, pos + XMVector3Transform(dir, currentRotation));	//Add the line segment to the line mesh
				pos += XMVector3Transform(dir, currentRotation);						//Move the position marker
				break;
			case '[':
				pos_stack.push_back(pos);
				rotation_stack.push_back(currentRotation);
				currentRotation *= XMMatrixRotationAxis(fwd, AI_DEG_TO_RAD(45.0f));
				break;
			case ']':
				pos = pos_stack.back();
				pos_stack.pop_back();
				currentRotation = rotation_stack.back();
				rotation_stack.pop_back();
				currentRotation *= XMMatrixRotationAxis(fwd, AI_DEG_TO_RAD(-45.0f));
				break;
		}
	}
	
	//Build the vertices
	m_Line->BuildLine(renderer->getDeviceContext(), renderer->getDevice());
}

void App1::BuildTree3D()
{
	//Clear any lines we might already have
	m_Line->Clear();
	m_3dtree_branches.clear();
	m_3dtree_leaves.clear();

	//Get the current L-System string, right now we have a place holder
	std::string systemString = lSystem.GetCurrentSystem();

	//Initialise some variables
	XMVECTOR pos = XMVectorReplicate(0.0f);	//Current position (0,0,0)
	XMVECTOR dir = XMVectorSet(0, 1, 0, 1);	//Current direction is "Up"
	XMVECTOR fwd = XMVectorSet(0, 0, 1, 1);	//Rotation axis.
	XMMATRIX currentRotation = XMMatrixRotationRollPitchYaw(0, 0, 0);
	std::vector<XMMATRIX> rotation_stack;
	std::vector<XMVECTOR> pos_stack;
	float rng = 0.f, cylinder_radius_scale = 1.f;

	//Build 3D tree
	for (int i = 0; i < systemString.length(); i++) {
		switch (systemString[i]) {
		case 'F':	//Draw a line segment and move forward
			XMVECTOR step = XMVector3Transform(dir, currentRotation);
			if(!lSystem_UseCylinders) m_Line->AddLine(pos, pos + step);	//Add the line segment to the line mesh
			else
			{
				m_3dtree_branches.push_back(std::unique_ptr<CylinderMesh>(new CylinderMesh(
					renderer->getDevice(),
					renderer->getDeviceContext(),
					1,
					4,
					XMVectorGetX(XMVector3Length(step)),
					.1f * cylinder_radius_scale,
					.1f * cylinder_radius_scale * .6f
				)));
				m_3dtree_branches.back()->m_Transform =  currentRotation * XMMatrixTranslationFromVector(pos);
			}
			pos += step;						//Move the position marker
			break;
		case 'A':
			//Add a leave
			if (!lSystem_UseCylinders) break;
			m_3dtree_leaves.push_back(std::unique_ptr<Leaf>(new Leaf(renderer->getDevice(), renderer->getDeviceContext(), 2)));
			m_3dtree_leaves.back()->m_Transform = XMMatrixScaling(.25f, .25f, .25f) * XMMatrixTranslationFromVector(pos);
			break;
		case '[':
			pos_stack.push_back(pos);
			rotation_stack.push_back(currentRotation);
			dir *= XMVectorSet(.8f, .8f, .8f, 1.f);
			cylinder_radius_scale *= .6f;
			break;
		case ']':
			pos = pos_stack.back();
			pos_stack.pop_back();
			currentRotation = rotation_stack.back();
			rotation_stack.pop_back();
			dir /= XMVectorSet(.8f, .8f, .8f, 1.f);
			cylinder_radius_scale /= .6f;
			break;
		case '&':
			rng = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / (3.1415f / 4.f));
			//rng = AI_DEG_TO_RAD(20.f);
			currentRotation *= XMMatrixRotationAxis(XMVector3Transform(fwd, currentRotation), rng);
			break;
		case '<':
			rng = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / 3.1415f);
			//rng = AI_DEG_TO_RAD(120.f);
			currentRotation *= XMMatrixRotationAxis(XMVector3Transform(dir, currentRotation), rng);
			break;
		case '>':
			rng = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / -3.1415f);
			//rng = AI_DEG_TO_RAD(-120.f);
			currentRotation *= XMMatrixRotationAxis(XMVector3Transform(dir, currentRotation), rng);
			break;
		default:
			break;
		}
	}
	//Build the vertices if we are rendering lines
	if (!lSystem_UseCylinders) m_Line->BuildLine(renderer->getDeviceContext(), renderer->getDevice());
}