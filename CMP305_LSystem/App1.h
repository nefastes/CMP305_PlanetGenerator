// Application.h
#ifndef _APP1_H
#define _APP1_H

#define DISTANCE_BETWEEN_GRASS_SPROUTS .4f

// Includes
#pragma once
#include "DXF.h"	// include dxframework
#include "LightShader.h"
#include "PlanetShader.h"
#include "LineMesh.h"
#include "CylinderMesh.h"
#include "Leaf.h"
#include "LSystem.h"
#include "FabrikMesh.h"
#include "ImprovedNoise.h"
#include "PlanetMesh.h"
#include "Tree.h"
#include <memory>
#include <vector>
#include <array>
#include <time.h>

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
	std::unique_ptr<LightShader>			light_shader;
	std::unique_ptr<PlanetShader>			planet_shader;
	std::unique_ptr<PlaneMesh>				m_ground;
	std::unique_ptr<Light>					light;


	//Fabrik
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
	std::unique_ptr<PlanetMesh> planet_mesh;
	int gui_planet_noise_n_layers;
	XMFLOAT3 gui_planet_rotation;
	char settings_filename[64];
	XMFLOAT4 gui_planet_shader_material_thresholds;
	bool gui_planet_generate_on_input;

	//Planet trees
	std::vector<std::unique_ptr<Tree>> planet_trees;
};

#endif