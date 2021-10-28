// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"
#include <stack>
App1::App1() :
	lSystem("A"),
	lSystem_nIterations(1),
	lSystem_BuildType(0),
	lSystem_UseCylinders(false)
{
}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	textureMgr->loadTexture(L"grass", L"res/grass.png");

	// Create Mesh object and shader object
	m_Line.reset( new LineMesh( renderer->getDevice(), renderer->getDeviceContext() ) );
	shader.reset( new LightShader( renderer->getDevice(), hwnd ) );

	light.reset( new Light );
	light->setAmbientColour(0.25f, 0.25f, 0.25f, 1.0f);
	light->setDiffuseColour(0.75f, 0.75f, 0.75f, 1.0f);
	light->setDirection(1.0f, -0.0f, 0.0f);

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

	//Build the line
	//BuildLine2D();
}

App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	//Delete cylinders
	m_Cylinders.clear();
}

bool App1::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}
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
	if (m_Line->getIndexCount() > 0) {
		shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"grass"), light.get());
		m_Line->sendData(renderer->getDeviceContext());
		shader->render(renderer->getDeviceContext(), m_Line->getIndexCount());
	}
	for (unsigned i = 0u; i < m_Cylinders.size(); ++i)
	{
		shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * m_Cylinders[i]->m_Transform, viewMatrix, projectionMatrix, textureMgr->getTexture(L"grass"), light.get());
		m_Cylinders[i]->sendData(renderer->getDeviceContext());
		shader->render(renderer->getDeviceContext(), m_Cylinders[i]->getIndexCount());
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

	ImGui::LabelText("L-System", "");

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
		else m_Cylinders.clear();
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
	m_Cylinders.clear();

	//Get the current L-System string, right now we have a place holder
	std::string systemString = lSystem.GetCurrentSystem();

	//Initialise some variables
	XMVECTOR pos = XMVectorReplicate(0.0f);	//Current position (0,0,0)
	XMVECTOR dir = XMVectorSet(0, 1, 0, 1);	//Current direction is "Up"
	XMVECTOR fwd = XMVectorSet(0, 0, 1, 1);	//Rotation axis.
	XMMATRIX currentRotation = XMMatrixRotationRollPitchYaw(0, 0, 0);
	std::vector<XMMATRIX> rotation_stack;
	std::vector<XMVECTOR> pos_stack;
	float rng;

	//Build 3D tree
	for (int i = 0; i < systemString.length(); i++) {
		switch (systemString[i]) {
		case 'F':	//Draw a line segment and move forward
			XMVECTOR step = XMVector3Transform(dir, currentRotation);
			if(!lSystem_UseCylinders) m_Line->AddLine(pos, pos + step);	//Add the line segment to the line mesh
			else
			{
				m_Cylinders.push_back(std::unique_ptr<CylinderMesh>(new CylinderMesh(
					renderer->getDevice(),
					renderer->getDeviceContext(),
					1,
					6,
					XMVectorGetX(XMVector3Length(step)),
					.1f * XMVectorGetX(XMVector3Length(dir)),
					.1f * XMVectorGetX(XMVector3Length(dir * XMVectorSet(.8f, .8f, .8f, 1.f)))
				)));
				m_Cylinders.back()->m_Transform =  currentRotation * XMMatrixTranslationFromVector(pos);
			}
			pos += step;						//Move the position marker
			break;
		case '[':
			pos_stack.push_back(pos);
			rotation_stack.push_back(currentRotation);
			dir *= XMVectorSet(.8f, .8f, .8f, 1.f);
			break;
		case ']':
			pos = pos_stack.back();
			pos_stack.pop_back();
			currentRotation = rotation_stack.back();
			rotation_stack.pop_back();
			dir /= XMVectorSet(.8f, .8f, .8f, 1.f);
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
		case 'A':
			break;
		default:
			break;
		}
	}
	//Build the vertices if we are rendering lines
	if (!lSystem_UseCylinders) m_Line->BuildLine(renderer->getDeviceContext(), renderer->getDevice());
	//If we are rendering cylinders..
	else
	{
		/*for (unsigned i = 0u; i < m_Cylinders.size(); ++i)
		{
			m_Cylinders[i]->
		}*/
	}
}

void App1::CleanSystem()
{
	m_Line->Clear();
}