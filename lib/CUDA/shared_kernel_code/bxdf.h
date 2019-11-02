/**
 * BXDF interfaces and reflection/transmission types.
 *
 * Based on PBRT interface, modified for the GPU.
 */

#pragma once

namespace pbrt
{
#include "pbrt/util.h"
};
using namespace pbrt;

class BxDF : public HasPlacementNewOperator
{
  public:
	__device__ virtual bool MatchesFlags( BxDFType t ) const = 0;
	__device__ virtual bool HasFlags( BxDFType t ) const = 0;
	__device__ virtual BxDFType GetType() const = 0;

	__device__ virtual float3 f( const float3& wo, const float3& wi ) const = 0;

	__device__ virtual float3 Sample_f( const float3 wo, float3& wi,
										/*  const Point2f& u, */ const float r0, const float r1,
										float& pdf, BxDFType& sampledType ) const
	{
		// Cosine-sample the hemisphere, flipping the direction if necessary
		wi = DiffuseReflectionCosWeighted( r0, r1 );
		if ( wo.z < 0 ) wi.z *= -1;

		pdf = Pdf( wo, wi );
		return f( wo, wi );
	}

	__device__ virtual float Pdf( const float3& wo, const float3& wi ) const
	{
		return SameHemisphere( wo, wi ) ? AbsCosTheta( wi ) * INVPI : 0;
	}
};

// Templated type that may ever be helpful
template <typename Derived, BxDFType _type>
class BxDF_T : public BxDF
{
  protected:
	// TODO: We do not currently store the type, but this _MUST_ happen
	// when evaluating BxDFs in different ways.
	// __device__ BxDF_T() : BxDF( _type ) {}

	__device__ bool MatchesFlags( const BxDFType t ) const override
	{
		return ( _type & t ) == _type;
	}

	__device__ bool HasFlags( const BxDFType t ) const override
	{
		return ( _type & t ) == t;
	}

	__device__ BxDFType GetType() const override
	{
		return _type;
	}
};
