cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

cbuffer Settings : register(b1)
{
	float4 material_thresholds;	//x snow, y rock, z grass, w beach
};

cbuffer colours
{
	static float4 sea = float4(0.f, 0.f, 1.f, 1.f);
	static float4 beach = float4(1.f, 1.f, 0.f, 1.f);
	static float4 grass = float4(0.f, 1.f, 0.f, 1.f);
	static float4 rock = float4(.5f, .5f, .5f, 1.f);
	static float4 snow = float4(1.f, 1.f, 1.f, 1.f);
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
	float noise_height = (input.position.x * input.position.x + input.position.y * input.position.y + input.position.z * input.position.z) - 1.f; //Radius of sphere is 1.f
	float4 colour = sea;
	if (noise_height > material_thresholds.w) colour = beach;
	if (noise_height > material_thresholds.z) colour = grass;
	if (noise_height > material_thresholds.y) colour = rock;
	if (noise_height > material_thresholds.x) colour = snow;
	output.color = colour;

	// Calculate the normal vector against the world matrix only and normalise.
	output.normal = mul(input.normal, (float3x3) worldMatrix);
	output.normal = normalize(output.normal);

	return output;
}