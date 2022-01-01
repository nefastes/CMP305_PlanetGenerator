#include "GenerateMeshTask.h"

GenerateMeshTask::GenerateMeshTask(XMFLOAT3* v_pos, const XMFLOAT3& cube_vertex_pos, const float& radius, const std::vector<std::unique_ptr<NoiseLayerSettings>>* noise_layers)
{
	task_vertex_position_ = v_pos;
	task_cube_vertex_pos_ = cube_vertex_pos;
	task_radius_ = radius;
	task_noise_layers_ = noise_layers;
}

GenerateMeshTask::~GenerateMeshTask()
{
}

void GenerateMeshTask::run()
{
	//Create a vector from the center to the position
	XMVECTOR target_position = XMLoadFloat3(&task_cube_vertex_pos_);
	//Calculate its unit vector
	XMVECTOR unit_target_position = XMVector3Normalize(target_position);
	//Assign the new vertex position
	target_position = unit_target_position * task_radius_;
	float total_noise = 0.f;
	float layer_mask = 1.f; //Start with 1.f in case the user ticks the box for layer 0
	for (unsigned i = 0u; i < task_noise_layers_->size(); ++i)
	{
		//Get the current layer settings
		NoiseLayerSettings* layer = task_noise_layers_->at(i).get();
		//If the layer is deactivated, continue to deal with the next layer
		if (!layer->layer_active_)continue;
		//Setup base noise values
		float noise_value = 0.f;
		float f = layer->noise_base_frequency_;
		float a = layer->noise_base_amplitude_;
		float rigid_noise_LOD = 1.f;	//only used by rigid noise
		//For every sub layers, calculate the FBM noise
		for (unsigned j = 0u; j < layer->layer_nSubLayers_; ++j)
		{
			float noise = static_cast<float>(ImprovedNoise::noise(
				(XMVectorGetX(target_position) + layer->layer_center_.x) * f,
				(XMVectorGetY(target_position) + layer->layer_center_.y) * f,
				(XMVectorGetZ(target_position) + layer->layer_center_.z) * f));
			switch (layer->noise_type_)
			{
			case NoiseType::FBM:
				//Noise returned is range [-1,1]
				//Simply convert it in the range [0,1] and multiply by amplitude
				noise_value += a * .5f * (noise + 1.f);
				break;
			case NoiseType::RIGID:
			{
				//Noise is in range [-1,1]
				//Mirror the negative part to positive, i.e. do the absolute value, this will result in values smoothly
				//transitioning from 0 to 1 to 0
				//Substract the absolute value from 1 so that we get the inverse effect: the noise will peak from 0 to 1 to 0
				//Repeat the operation for sharper results (i.e. pow())
				//Multiply the result of the above by  a scalar to gain details from layer to layer (LOD)
				float rigid_noise = powf(1.f - std::abs(noise), layer->rigid_noise_sharpness_) * rigid_noise_LOD;
				//The LOD for the next layer is the LOD of the previous one multiplied by a scalar
				//Need to make sure the LOD stays between 0 and 1, otherwise it will act as an amplitude instead of tiny details
				rigid_noise_LOD = clamp(rigid_noise * layer->rigid_noise_LOD_multiplier_, 0.f, 1.f);
				//The final noise value is the calculated rigid noise multiplied by an amplitude
				noise_value += a * rigid_noise;
			}
			break;
			default:
				break;
			}
			f *= layer->layer_roughness_;
			a *= layer->layer_persistence_;
		}
		//This operation reduces the displacement dictated by the threshold value
		//It ensures it cannot go negative, so the minimum distance from the center will be radius_
		//This allows to create round water and noisy continents
		//The final ampltiude therefore influences only what is above the radius, usefull to create large flat continents
		noise_value = max(0.f, noise_value - layer->noise_min_threshold_) * layer->noise_final_amplitude_;
		//Add the calculated noise value to the total noise
		total_noise += noise_value * (layer->layer_use_previous_layer_as_mask_ ? layer_mask : 1.f);
		//Update the mask with this layer's noise value for the next layer
		layer_mask = noise_value;
	}
	//The noise displacement is a simple multiplication of the unit vector of the vertex position from the center with the total noise
	XMVECTOR noise_displacement = unit_target_position * total_noise;
	//The final position is then the traget position (unit vector * radius_) plus noisy displacement
	XMStoreFloat3(task_vertex_position_, target_position + noise_displacement);
}
