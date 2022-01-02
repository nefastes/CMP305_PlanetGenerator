#pragma once
cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

cbuffer Settings : register(b1)
{
	//needs to be 16 bytes total
	bool is_branch;	//1 byte
	bool3 padding;	//3 bytes
	float3 padding2; //12 bytes
};

cbuffer colours
{
	static float4 colour_branch = float4(.4f, .13f, 0.f, 1.f);
	static float4 colour_leaf = float4(0.f, .6f, 0.f, 1.f);
};

struct InputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct OutputType
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
	float3 normal : NORMAL;
};

OutputType main(InputType input)
{
	OutputType output;

	// Calculate the position of the vertex against the world, view, and projection matrices.
	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	// Calculate the color of the vertex
	output.color = is_branch * colour_branch + !is_branch * colour_leaf;

	// Calculate the normal vector against the world matrix only and normalise.
	output.normal = mul(input.normal, (float3x3) worldMatrix);
	output.normal = normalize(output.normal);

	return output;
}