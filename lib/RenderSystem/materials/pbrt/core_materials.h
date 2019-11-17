#pragma once

typedef uint32_t CoreMaterialChunk;

namespace common
{
namespace materials
{
namespace pbrt
{

using namespace lh2core;

struct Disney
{
	static constexpr auto type = MaterialType::PBRT_DISNEY;

	CoreTexture<float3> color;
	CoreTexture<float> flatness;
	CoreTexture<float3> scatterDistance;
	CoreTexture<float> specTrans;
	CoreTexture<float> metallic, eta;
	CoreTexture<float> roughness, specularTint, anisotropic;
	CoreTexture<float> sheenTint, sheen;
	CoreTexture<float> clearcoat, clearcoatGloss;
	CoreTexture<float> diffTrans;
	// float bumpMap;
	bool thin;
};

struct Glass
{
	static constexpr auto type = MaterialType::PBRT_GLASS;

	CoreTexture<float3> R;
	CoreTexture<float> urough;
	CoreTexture<float3> T;
	CoreTexture<float> vrough;

	CoreTexture<float> eta;

	bool remapRoughness;
};

struct Matte
{
	static constexpr auto type = MaterialType::PBRT_MATTE;

	CoreTexture<float3> Kd;
	CoreTexture<float> sigma;
};

struct Metal
{
	static constexpr auto type = MaterialType::PBRT_METAL;

	CoreTexture<float3> eta;
	CoreTexture<float> urough;
	CoreTexture<float3> k;
	CoreTexture<float> vrough;
	bool remapRoughness;

	// float roughness;
	// float bumpmap;
};

struct Mirror
{
	static constexpr auto type = MaterialType::PBRT_MIRROR;

	CoreTexture<float3> Kr;
};

struct Plastic
{
	static constexpr auto type = MaterialType::PBRT_PLASTIC;

	CoreTexture<float3> Kd;
	CoreTexture<float> roughness;
	CoreTexture<float3> Ks;

	bool remapRoughness;
};

struct Substrate
{
	static constexpr auto type = MaterialType::PBRT_SUBSTRATE;

	CoreTexture<float3> Kd;
	CoreTexture<float> urough;
	CoreTexture<float3> Ks;
	CoreTexture<float> vrough;

	bool remapRoughness;
};

}; // namespace pbrt
}; // namespace materials
}; // namespace common
