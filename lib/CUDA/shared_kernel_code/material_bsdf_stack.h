#pragma once

// #define NDEBUG

#include <assert.h>

#include "bxdf.h"

template <typename... BxDFs>
class BSDFStackMaterial : public MaterialIntf
{
	using BSDFStorageType = typename StorageRequirement<BxDFs...>::type;

	static constexpr auto max_elements = 8;
	BSDFStorageType stack[max_elements];
	int items = 0;

  public:
	__device__ BxDF& operator[]( size_t idx )
	{
		assert( idx < items );
		return *(BxDF*)( stack + idx );
	}

	__device__ const BxDF& operator[]( size_t idx ) const
	{
		assert( idx < items );
		return *(const BxDF*)( stack + idx );
	}

	__device__ void* Reserve()
	{
		assert( items < max_elements );
		return stack + items++;
	}

	// Pass type to copy proper size:
	template <typename T>
	__device__ void Add( const T& bxdf )
	{
#if 0
		assert( items < max_elements );
		(T&)stack[items++] = bxdf;
#else
		*(T*)Reserve() = bxdf;
#endif
	}

	// ----------------------------------------------------------------

	__device__ float Pdf( const float NDotV, const float NDotL ) const
	{
		int matches = items;
		float pdf = 0.f;
		for ( int i = 0; i < items; ++i )
		{
			if ( true ) // TODO: Implement type matching here, if necessary
				pdf += ( *this )[i].Pdf( NDotV, NDotL );
			else
				matches -= 1;
		}

		return matches > 0 ? pdf / matches : 0.f;
	}

	// ----------------------------------------------------------------
	// Overrides:

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
		for ( int i = 0; i < items; ++i )
			// TODO: Match based on reflect/transmit!
			r += ( *this )[i].f( NDotV, NDotL );
		return r;
	}

	__device__ float3 Sample( float3 iN, const float3 N, const float3 T,
							  const float3 wo,
							  float r3, float r4,
							  float3& wi, float& pdf,
							  BxDFType& sampledType ) const override
	{
		pdf = 0;
		sampledType = BxDFType( 0 );

		const float NDotV = dot( iN, wo );

		// TODO: Select bsdf based on comp !!AND!! match type

		const int matchingComps = /* NumBxDFs( type ) */ items;

		if ( !matchingComps )
			return make_float3( 0.f );

		// Select a random BxDF (that matches the flags) to sample:
		const int comp = min( (int)floor( r3 * matchingComps ), matchingComps - 1 );

		// Rescale r3:
		r3 = min( r3 * matchingComps - comp, 1.f - EPSILON );

		const BxDF* bxdf = nullptr;
		int count = comp;
		for ( int i = 0; i < items; ++i )
			if ( /* (*this)[i].MatchesFlags( type ) && */ count-- == 0 )
			{
				bxdf = &( *this )[i];
				break;
			}
		assert( bxdf );

		sampledType = bxdf->type;
		auto f = bxdf->Sample_f( NDotV, wi, r3, r4, pdf, sampledType );

		// wi = wiForType( stack.types[comp], r3, r4 );

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
