// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"	// include dxframework
#include "LightShader.h"
#include "LineMesh.h"
#include "LSystem.h"
#include <memory>

using std::unique_ptr;

class App1 : public BaseApplication
{
public:
	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:
	bool render();
	void gui();

private:
	void BuildLine2D();
	void BuildTree3D();
	void CleanSystem();
	float Rand() { return (float)rand() / RAND_MAX; }

	unique_ptr<LightShader>	shader;
	unique_ptr<LineMesh>	m_Line;

	unique_ptr<Light>		light;
	LSystem					lSystem;
	int						lSystem_nIterations;
	int						lSystem_BuildType;
};

#endif