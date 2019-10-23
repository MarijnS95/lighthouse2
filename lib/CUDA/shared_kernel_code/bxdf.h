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

// Commonly used types. This saves massively on readability,
// instead of defining all
struct CommonIntersectionParams
{
	float3 wo;
	float NDotV;
	float3 wi;
	float NDotL;
	float3 N;
};

// ----------------------------------------------------------------

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

class BxDF : public HasPlacementNewOperator
{
  public:
	__device__ virtual float3 f( const float NDotV, const float NDotL ) const = 0;

	__device__ /* virtual */ float3 f( const float3& wo, const float3& wi ) const
	{
		return f( AbsCosTheta( wo ), AbsCosTheta( wi ) );
	}

	__device__ virtual float3 Sample_f( const float NDotV, float3& wi,
										/*  const Point2f& u, */ const float r0, const float r1,
										float& pdf, BxDFType* sampledType ) const
	{
		// TODO: sampledType

		// Cosine-sample the hemisphere, flipping the direction if necessary
		// *wi = CosineSampleHemisphere( u );
		wi = DiffuseReflectionCosWeighted( r0, r1 );
		if ( wo.z < 0 ) wi.z *= -1;
		pdf = Pdf( NDotV, AbsCosTheta( wi ) );
		return f( NDotV, AbsCosTheta( wi ) );
	}

	__device__ /* virtual */ float3 Sample_f( const float3& wo, float3& wi,
											  /*  const Point2f& u, */ const float r0, const float r1,
											  float& pdf, BxDFType* sampledType ) const
	{
		return Sample_f( AbsCosTheta( wo ), wi, r0, r1, pdf, sampledType );
	}

	__device__ virtual float Pdf( const float NDotV, const float NDotL ) const
	{
		return NDotV * NDotL > 0 ? NDotL * INVPI : 0;
	}

	__device__ float Pdf( const float3& wo, const float3& wi ) const
	{
		return Pdf( AbsCosTheta( wo ), AbsCosTheta( wi ) );
		// return SameHemisphere( wo, wi ) ? AbsCosTheta( wi ) * INVPI : 0;
	}
};

// Templated type that may ever be helpful
template <typename Derived, BxDFType type>
class BxDF_T : public BxDF
{
  public:
	// TODO: We do not currently store the type, but this _MUST_ happen
	// when evaluating BxDFs in different ways.
	// BxDF_T() : BxDF( type ) {}
};
