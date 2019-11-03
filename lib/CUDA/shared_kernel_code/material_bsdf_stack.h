#pragma once

#define NDEBUG

#include "VariantStore.h"
#include "bxdf.h"

template <typename... BxDFs>
class BSDFStackMaterial : public MaterialIntf
{
  protected:
	VariantStore<BxDF, BxDFs...> bxdfs;

	// ----------------------------------------------------------------

	__device__ float Pdf( const float NDotV, const float NDotL ) const
	{
		int matches = (int)bxdfs.size();
		float pdf = 0.f;
		for ( const auto& bxdf : bxdfs )
		{
			if ( true ) // TODO: Implement type matching here, if necessary
				pdf += bxdf.Pdf( NDotV, NDotL );
			else
				matches -= 1;
		}

		return matches > 0 ? pdf / matches : 0.f;
	}

	// ----------------------------------------------------------------
	// Overrides:
  public:
	/**
	 * Create BxDF stack
	 */
	__device__ void Setup(
		const float3 D,					   // IN:	incoming ray direction, used for consistent normals
		const float u, const float v,	  //		barycentric coordinates of intersection point
		const float coneWidth,			   //		ray cone width, for texture LOD
		const CoreTri4& tri,			   //		triangle data
		const int instIdx,				   //		instance index, for normal transform
		float3& N, float3& iN, float3& fN, //		geometric normal, interpolated normal, final normal (normal mapped)
		float3& T,						   //		tangent vector
		const float waveLength = -1.0f	 // IN:	wavelength (optional)
		) override
	{
		// Empty BxDF stack by default
	}

	__device__ void InvertETAAndDisableTransmittance() override
	{
		// Nothing at the moment
	}

	__device__ bool IsEmissive() const override
	{
		return false;
	}

	__device__ bool IsAlpha() const override
	{
		return false;
	}

	/**
	 * Used to retrieve color for emissive surfaces.
	 */
	__device__ float3 Color() const override
	{
		// TODO:
		return make_float3( 1, 0, 1 );
	}

	__device__ float3 Evaluate( const float3 iN, const float3 T,
								const float3 wo, const float3 wi,
								float& pdf ) const override
	{
		const float NDotV = dot( iN, wo );
		const float NDotL = dot( iN, wi );

		pdf = Pdf( NDotV, NDotL );

		// TODO: Reuse this inlined datastructure on more functions?
		// CommonIntersectionParams ss = {
		// 	wo,
		// 	NDotV,
		// 	wi,
		// 	NDotL,
		// 	iN,
		// };

		float3 r = make_float3( 0.f );
		for ( const auto& bxdf : bxdfs )
			// for (int i=0;i<bxdfs.size();++i)
			// TODO: Match based on reflect/transmit!
			r += bxdf.f( NDotV, NDotL );
		return r;
	}

	__device__ float3 Sample( float3 iN, const float3 N, const float3 T,
							  const float3 wo, const float distance,
							  float r3, const float r4,
							  float3& wi, float& pdf,
							  BxDFType& sampledType ) const override
	{
		pdf = 0;
		sampledType = BxDFType( 0 );

		const float NDotV = dot( iN, wo );

		// TODO: Select bsdf based on comp !!AND!! match type

		const int matchingComps = /* NumBxDFs( type ) */ (int)bxdfs.size();

		if ( !matchingComps )
			return make_float3( 0.f );

		// Select a random BxDF (that matches the flags) to sample:
		const int comp = min( (int)floor( r3 * matchingComps ), matchingComps - 1 );

		// Rescale r3:
		r3 = min( r3 * matchingComps - comp, 1.f - EPSILON );

		const BxDF* bxdf = nullptr;
		int count = comp;
		for ( const auto& bxdf_i : bxdfs )
			if ( /* bxdf_i.MatchesFlags( type ) && */ count-- == 0 )
			{
				bxdf = &bxdf_i;
				break;
			}
		assert( bxdf );

		sampledType = bxdf->type;
		auto f = bxdf->Sample_f( NDotV, wi, r3, r4, pdf, sampledType );

		// const float NDotL = dot( iN, wi );

		if ( pdf == 0 )
		{
			sampledType = BxDFType( 0 );
			return make_float3( 0.f );
		}

		// Convert wi to world-space. (Not using Tangent2World because we already have T, besides N)

		const float3 B = normalize( cross( T, iN ) );
		const float3 Tfinal = cross( B, iN );

		wi = Tfinal * wi.x + B * wi.y + iN * wi.z;

		// ShaderState ss = {
		// 	wo,
		// 	NDotV,
		// 	wi,
		// 	NDotL,
		// 	iN,
		// };

		// printf( "f.x: %f\n", f.x );

		return f;
		// TODO: Calculate pdf and f over all MATCHING brdfs if stack.types[comp] is _not_ specular
	}
};
