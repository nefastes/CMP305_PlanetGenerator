#pragma once
#include "DXF.h"
class PlanetShader : public BaseShader
{
private:
	struct LightBufferType
	{
		XMFLOAT4 diffuse;
		XMFLOAT3 direction;
		float padding;
	};

	struct SettingsBufferType
	{
		float noise_max;
		XMFLOAT3 padding;
		XMFLOAT4 material_thresholds;	//x snow, y rock, z grass, w beach
	};

public:
	PlanetShader(ID3D11Device* device, HWND window);
	~PlanetShader();

	void setShaderParameters(ID3D11DeviceContext* dc, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, Light* light,
		const float& noise_max, XMFLOAT4& thresholds);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);
	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* settingsBuffer;
};

