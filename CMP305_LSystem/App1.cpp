// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"
#include <stack>
App1::App1() :
	lSystem("A")
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

	//Build the line
	BuildLine2D();
}

App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();
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
	std::string systemString = "ABABABBABABA";

	//Initialise some variables
	XMVECTOR pos = XMVectorReplicate(0.0f);	//Current position (0,0,0)
	XMVECTOR dir = XMVectorSet(0, 1, 0, 1);	//Current direction is "Up"
	XMVECTOR fwd = XMVectorSet(0, 0, 1, 1);	//Rotation axis. Our rotations happen around the "forward" vector
	XMMATRIX currentRotation = XMMatrixRotationRollPitchYaw(0, 0, 0);

	//Go through the L-System string
	for (int i = 0; i < systemString.length(); i++) {
		switch (systemString[i]) {			
			case 'A':	//If it's A, make a line
				m_Line->AddLine(pos, pos + XMVector3Transform(dir, currentRotation));	//Add the line segment to the line mesh
				pos += XMVector3Transform(dir, currentRotation);						//Move the position marker
				break;			
			case 'B':	//If it's B, rotate
				currentRotation *= XMMatrixRotationAxis(fwd, AI_DEG_TO_RAD(45.0f));
				break;
		}
	}
	//Build the vertices
	m_Line->BuildLine(renderer->getDeviceContext(), renderer->getDevice());
}

void App1::CleanSystem()
{
	m_Line->Clear();
}