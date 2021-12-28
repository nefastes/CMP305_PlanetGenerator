// Application.h
#ifndef _APP1_H
#define _APP1_H

#define DISTANCE_BETWEEN_GRASS_SPROUTS .4f

// Includes
#pragma once
#include "DXF.h"	// include dxframework
#include "LightShader.h"
#include "LineMesh.h"
#include "CylinderMesh.h"
#include "Leaf.h"
#include "LSystem.h"
#include "FabrikMesh.h"
#include "ImprovedNoise.h"
#include "CubeSphereMesh.h"
#include <memory>
#include <vector>
#include <array>

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

	std::unique_ptr<LightShader>			shader;
	std::unique_ptr<LineMesh>				m_Line;
	std::unique_ptr<PlaneMesh>				m_ground;
	std::vector<unique_ptr<CylinderMesh>>	m_3dtree_branches;
	std::vector<unique_ptr<Leaf>>			m_3dtree_leaves;

	std::unique_ptr<Light>		light;
	LSystem						lSystem;
	int							lSystem_nIterations;
	int							lSystem_BuildType;
	bool						lSystem_UseCylinders;


	std::unique_ptr<SphereMesh> fabrik_goal_mesh;
	XMFLOAT3 fabrik_goal_position;
	bool fabrik_animate_with_noise;
	XMFLOAT2 fabrik_animate_noise_offset;
	float gui_debug_noise;
	int fabrik_n_segments;
	float fabrik_total_length;
	std::unique_ptr<FabrikMesh> fabrik_mesh;

	XMFLOAT2 gui_wind_direction;
	float gui_wind_strength;

	bool fabrik_render_cylinders;

	std::array<std::unique_ptr<FabrikMesh>, 400> grass_sprouts;

	//Planet
	std::unique_ptr<CubeSphereMesh> planet_mesh;
	int gui_planet_resolution;
	float gui_planet_radius;
	float gui_noise_frequency;
	float gui_noise_amplitude;
	XMFLOAT3 gui_noise_center;
};

#endif