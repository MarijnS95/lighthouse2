#pragma once

#include "Storage.h"

namespace materials
{

// BSDF Declarations
enum /* class */ BxDFType : int
{
	BSDF_REFLECTION = 1 << 0,
	BSDF_TRANSMISSION = 1 << 1,
	BSDF_DIFFUSE = 1 << 2,
	BSDF_GLOSSY = 1 << 3,
	BSDF_SPECULAR = 1 << 4,
	BSDF_ALL = BSDF_DIFFUSE | BSDF_GLOSSY | BSDF_SPECULAR | BSDF_REFLECTION |
			   BSDF_TRANSMISSION,
};

class MaterialIntf : public HasPlacementNewOperator
{
  public:
	__device__ virtual void Setup(
		const float3 D,					   // IN:	incoming ray direction, used for consistent normals
		const float u, const float v,	  //		barycentric coordinates of intersection point
		const float coneWidth,			   //		ray cone width, for texture LOD
		const CoreTri4& tri,			   //		triangle data
		const int instIdx,				   //		instance index, for normal transform
		float3& N, float3& iN, float3& fN, //		geometric normal, interpolated normal, final normal (normal mapped)
		float3& T,						   //		tangent vector
		const float waveLength = -1.0f	 // IN:	wavelength (optional)
		) = 0;

	__device__ virtual void DisableTransmittance()
	{
		// Nothing at the moment
	}

	__device__ virtual bool IsEmissive() const = 0;
	__device__ virtual bool IsAlpha() const = 0;
	/**
	 * Used to retrieve color for emissive surfaces.
	 */
	__device__ virtual float3 Color() const = 0;

	__device__ virtual float3 Evaluate( const float3 iN, const float3 T,
										const float3 wo, const float3 wi, float& pdf ) const = 0;
	__device__ virtual float3 Sample( float3 iN, const float3 N, const float3 T,
									  const float3 wo, const float distance,
									  const float r3, const float r4,
									  float3& wi, float& pdf,
									  BxDFType& sampledType ) const = 0;
};

#include "material_bsdf_stack.h"

#include "material_disney.h"
#include "pbrt/materials.h"

// WARNING: When adding a new material type, it _MUST_ be listed here!
using MaterialStore = StorageRequirement<pbrt::DisneyGltf, DisneyMaterial>::type;

// NOTE: Materialstore is a pointer-type (array) by design
static_assert( std::is_array<MaterialStore>::value, "MaterialStore must be an array" );
static_assert( std::is_pointer<std::decay_t<MaterialStore>>::value, "Decayed material store must be an array" );

LH2_DEVFUNC MaterialIntf* GetMaterial( MaterialStore inplace, const CoreTri4& tri )
{
	// Copy desc (single integer) to register:
	const CoreMaterialDesc matDesc = materialDescriptions[GET_TRI_MATERIAL( tri.v4 )];

	// Call placement new operator to set up vtables.

	switch ( matDesc.type )
	{
	case MaterialType::DISNEY:
		// Implement the gltf-extracted material through PBRT BxDFs
		// (WARNING: No 1-1 mapping!)
#if 1
		return new ( inplace ) pbrt::DisneyGltf();
#else
		return new ( inplace ) DisneyMaterial();
#endif
		// case MaterialType::CUSTOM_BSDF:
		// 	return new ( inplace ) BSDFMaterial();
		// case MaterialType::PBRT_DISNEY:
		// TODO:
		// 	return new ( inplace ) PbrtDisneyMaterial();
	}

	// Unknown material:
	return nullptr;
}
}; // namespace materials
