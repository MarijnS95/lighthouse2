#pragma once

#define FLOAT3( name, extra_name ) \
	union {                        \
		float4 name##_float4;      \
		struct                     \
		{                          \
			float3 name;           \
			float extra_name;      \
		};                         \
	};

struct Disney
{
	static constexpr auto type = MaterialType::PBRT_DISNEY;

	FLOAT3( color, flatness );
	FLOAT3( scatterDistance, specTrans );
	float metallic, eta;
	float roughness, specularTint, anisotropic;
	float sheenTint, sheen;
	float clearcoat, clearcoatGloss;
	float diffTrans;
	// float bumpMap;
	bool thin;
};

struct Glass
{
	static constexpr auto type = MaterialType::PBRT_GLASS;

	FLOAT3( R, urough );
	FLOAT3( T, vrough );

	float eta;

	bool remapRoughness;
};

struct Matte
{
	static constexpr auto type = MaterialType::PBRT_MATTE;

	FLOAT3( Kd, sigma );
};

struct Metal
{
	static constexpr auto type = MaterialType::PBRT_METAL;

	FLOAT3( eta, urough );
	FLOAT3( k, vrough );
	bool remapRoughness;

	// float roughness;
	// float bumpmap;
};

struct Mirror
{
	static constexpr auto type = MaterialType::PBRT_MIRROR;

	float3 Kr;
};

struct Substrate
{
	static constexpr auto type = MaterialType::PBRT_SUBSTRATE;

	FLOAT3( Kd, urough );
	FLOAT3( Ks, vrough );

	bool remapRoughness;
};
