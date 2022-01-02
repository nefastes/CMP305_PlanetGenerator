#pragma once
// Light pixel shader
// Calculate diffuse lighting for a single directional light

cbuffer LightBuffer : register(b0)
{
	float4 diffuseColour;
	float3 lightDirection;
	float padding;
};

struct InputType
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
	float3 normal : NORMAL;
};

// Calculate lighting intensity based on direction and normal. Combine with light colour.
float4 calculateLighting(float3 lightDirection, float3 normal, float4 diffuse)
{
	float intensity = saturate(dot(normal, lightDirection));
	float4 colour = saturate(diffuse * intensity);
	return colour;
}

float4 main(InputType input) : SV_TARGET
{
	//return float4(input.normal, 1.f);

	// Sample the texture. Calculate light intensity and colour, return light*texture for final pixel colour.
	float4 lightColour = calculateLighting(-lightDirection, input.normal, diffuseColour);
	
	return lightColour * input.color;
}



