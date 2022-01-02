#pragma once
#include "DXF.h"
class TreeShader : public BaseShader
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
		//needs tp be 16 bytes
		bool is_branch;		//1 byte
		bool padding[3];	//3 bytes
		XMFLOAT3 padding2;	//12 bytes
	};

public:
	TreeShader(ID3D11Device* device, HWND window);
	~TreeShader();

	void setShaderParameters(ID3D11DeviceContext* dc, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection,
		Light* light, const bool is_branch);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);
	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* settingsBuffer;
};

