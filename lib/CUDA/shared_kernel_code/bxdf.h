#pragma once

// ----------------------------------------------------------------
// TODO: Move to frame class or something

LH2_DEVFUNC float AbsCosTheta( const float3& w ) { return std::abs( w.z ); }

LH2_DEVFUNC bool SameHemisphere( const float3& w, const float3& wp )
{
	return w.z * wp.z > 0;
}

// PBRT Uses t-first. Provide a function for clarity.
template <typename F, typename T>
LH2_DEVFUNC T pbrt_Lerp( F t, T a, T b )
{
	return lerp( a, b, t );
}

// ----------------------------------------------------------------

class BxDF : public HasPlacementNewOperator
{
  protected:
	__device__ BxDF( BxDFType type ) : type( type ) {}

  public:
	const BxDFType type;

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

	__device__ float Pdf( const float3& wo, const float3& wi ) const
	{
		return SameHemisphere( wo, wi ) ? AbsCosTheta( wi ) * INVPI : 0;
	}
};

// Templated type that may ever be helpful
template <typename Derived, BxDFType _type>
class BxDF_T : public BxDF
{
  protected:
	__device__ BxDF_T() : BxDF( _type ) {}
};
