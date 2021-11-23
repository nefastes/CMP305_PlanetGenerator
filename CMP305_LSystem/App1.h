// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"	// include dxframework
#include "LightShader.h"
#include "LineMesh.h"
#include "CylinderMesh.h"
#include "Leaf.h"
#include "LSystem.h"
#include "FabrikMesh.h"
#include <memory>
#include <vector>
#include <array>

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

	unique_ptr<LightShader>					shader;
	unique_ptr<LineMesh>					m_Line;
	std::vector<unique_ptr<CylinderMesh>>	m_3dtree_branches;
	std::vector<unique_ptr<Leaf>>			m_3dtree_leaves;

	unique_ptr<Light>		light;
	LSystem					lSystem;
	int						lSystem_nIterations;
	int						lSystem_BuildType;
	bool					lSystem_UseCylinders;


	std::unique_ptr<SphereMesh> fabrik_goal_mesh;
	XMFLOAT3 fabrik_goal_position;
	int fabrik_n_segments;
	float fabrik_total_length;
	std::unique_ptr<FabrikMesh> fabrik_mesh;

	std::array<std::unique_ptr<FabrikMesh>, 10> grass_sprouts;
};

#endif