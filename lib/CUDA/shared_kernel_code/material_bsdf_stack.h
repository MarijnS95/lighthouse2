/**
 * BSDF implementation to sample multiple BxDFs.
 *
 * Based on the `BSDF' structure from PBRT, altered to work on the GPU
 * with a type "invariant" list on the stack / in registers.
 */

#pragma once

#define NDEBUG

#include "VariantStore.h"
#include "bxdf.h"

template <typename... BxDFs>
class BSDFStackMaterial : public MaterialIntf
{
	float3 T, B, N;

  protected:
	VariantStore<BxDF, BxDFs...> bxdfs;

	__device__ float3 WorldToLocal( const float3& v ) const
	{
		return make_float3( dot( v, T ), dot( v, B ), dot( v, N ) );
	}

	__device__ float3 LocalToWorld( const float3& v ) const
	{
		return T * v.x + B * v.y + N * v.z;
	}

	__device__ void SetupTBN( const float3& T, const float3& N )
	{
		// Setup TBN (Not using Tangent2World/World2Tangent because we already have T, besides N)
		this->N = N;
		this->B = normalize( cross( T, N ) );
		this->T = cross( B, N );
	}

	// ----------------------------------------------------------------

  private:
	__device__ float Pdf( const float3 wo, const float3 wi ) const
	{
		int matches = (int)bxdfs.size();
		float pdf = 0.f;
		for ( const auto& bxdf : bxdfs )
		{
			if ( true ) // TODO: Implement type matching here, if necessary
				pdf += bxdf.Pdf( wo, wi );
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

		// Extract _common_ normal/frame info from triangle.
		// TODO: This should _not_ be the responsibility of the material. REFACTOR!
		float w;
		SetupFrame(
			// In:
			D, u, v, tri, instIdx, /* TODO: Extract smoothnormal information elsewhere */ true,
			// Out:
			N, iN, fN, T, w );

		SetupTBN( T, iN );
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

	__device__ float3 Evaluate( const float3 iN, const float3 /* Tinit */,
								const float3 woWorld, const float3 wiWorld,
								float& pdf ) const override
	{
		const float3 wo = WorldToLocal( woWorld ), wi = WorldToLocal( wiWorld );

		pdf = Pdf( wo, wi );

		const bool reflect = dot( wiWorld, iN ) * dot( woWorld, iN ) > 0;
		const BxDFType reflectFlag = reflect ? BxDFType::BSDF_REFLECTION : BxDFType::BSDF_TRANSMISSION;

		float3 r = make_float3( 0.f );
		for ( const auto& bxdf : bxdfs )
			if ( bxdf.HasFlags( reflectFlag ) )
				r += bxdf.f( wo, wi );

		return r;
	}

	__device__ float3 Sample( float3 iN, const float3 /* N */, const float3 /* Tinit */,
							  const float3 woWorld, const float distance,
							  float r3, float r4,
							  float3& wiWorld, float& pdf,
							  BxDFType& sampledType ) const override
	{
		pdf = 0.f;
		sampledType = BxDFType( 0 );

		const float3 wo = WorldToLocal( woWorld );

#if IMPLEMENTED_SELECT_BXDF
		// TODO: pass requested BxDFType and filter matching bxdfs.
#endif

		const int matchingComps = /* NumBxDFs( type ) */ (int)bxdfs.size();

		if ( !matchingComps )
			return make_float3( 0.f );

		// Select a random BxDF (that matches the flags) to sample:
		const int comp = min( (int)floor( r3 * matchingComps ), matchingComps - 1 );

		// Rescale r3:
		r3 = min( r3 * matchingComps - comp, 1.f - EPSILON );

		const BxDF* bxdf = nullptr;
#if IMPLEMENTED_SELECT_BXDF
		int count = comp;
		for ( const auto& bxdf_i : bxdfs )
			if ( bxdf_i.MatchesFlags( type ) && count-- == 0 )
			{
				bxdf = &bxdf_i;
				break;
			}

		assert( bxdf );
#else
		bxdf = &bxdfs[comp];
#endif

		sampledType = bxdf->type;
		float3 wi;
		auto f = bxdf->Sample_f( wo, wi, r3, r4, pdf, sampledType );
		wiWorld = LocalToWorld( wi );

		if ( pdf == 0 )
		{
			sampledType = BxDFType( 0 );
			return make_float3( 0.f );
		}

		// If the selected bxdf is specular (and thus with
		// a specifically chosen direction, wi)
		// this is the only bxdf that is supposed to be sampled.
		if ( bxdf->HasFlags( BxDFType::BSDF_SPECULAR ) )
			return f;

		if ( matchingComps > 1 )
		{
			// TODO: Interpolated normal or geometric normal?
			const bool reflect = dot( wiWorld, iN ) * dot( woWorld, iN ) > 0;
			const BxDFType reflectFlag = reflect
											 ? BxDFType::BSDF_REFLECTION
											 : BxDFType::BSDF_TRANSMISSION;

			for ( const auto& bxdf_i : bxdfs )
				if ( bxdf != &bxdf_i
#if IMPLEMENTED_SELECT_BXDF
					 && bxdf_i.MatchesFlags( type )
#endif
				)
				{
					// Compute overall PDF with all matching _BxDF_s
					pdf += bxdf_i.Pdf( wo, wi );

					// Compute value of BSDF for sampled direction

					// PBRT Resets f to zero and evaluates all bxdfs again.
					// We however keep the evaluation of `bxdf`, just like
					// PBRT does in the pdf sum calculation.
					if ( bxdf_i.HasFlags( reflectFlag ) )
						f += bxdf_i.f( wo, wi );
				}

			pdf /= matchingComps;
		}

		return f;
	}
};
