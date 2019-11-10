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
	FLOAT3( R, urough );
	FLOAT3( T, vrough );

	float eta;

	bool remapRoughness;
};

struct Matte
{
	FLOAT3( Kd, sigma );
};

struct Metal
{
	FLOAT3( eta, urough );
	FLOAT3( k, vrough );
	bool remapRoughness;

	// float roughness;
	// float bumpmap;
};

struct Mirror
{
	float3 Kr;
};

struct Substrate
{
	FLOAT3( Kd, urough );
	FLOAT3( Ks, vrough );

	bool remapRoughness;
};
