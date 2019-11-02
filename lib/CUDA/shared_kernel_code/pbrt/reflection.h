/*
    pbrt source code is Copyright(c) 1998-2016
                        Matt Pharr, Greg Humphreys, and Wenzel Jakob.

    This file is part of pbrt.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

#pragma once

class LambertianTransmission : public BxDF_T<LambertianTransmission, BxDFType( BxDFType::BSDF_TRANSMISSION | BxDFType::BSDF_DIFFUSE )>
{
	const float3 T;

  public:
	__device__ LambertianTransmission( const float3& T ) : T( T )
	{
	}

	__device__ float3 f( const float3& wo, const float3& wi ) const override
	{
		return T * INVPI;
	}

	__device__ float3 Sample_f( const float3 wo, float3& wi,
								const float r0, const float r1,
								float& pdf, BxDFType& sampledType ) const override
	{
		wi = CosineSampleHemisphere( r0, r1 );
		if ( wo.z > 0 ) wi.z *= -1;
		pdf = Pdf( wo, wi );
		return f( wo, wi );
	}

	__device__ float Pdf( const float3& wo, const float3& wi ) const override
	{
		return !SameHemisphere( wo, wi ) ? AbsCosTheta( wi ) * INVPI : 0;
	}
};

template <typename MicrofacetDistribution_T, typename Fresnel_T>
class MicrofacetReflection : public BxDF_T<MicrofacetReflection<MicrofacetDistribution_T, Fresnel_T>,
										   BxDFType( BxDFType::BSDF_REFLECTION | BxDFType::BSDF_GLOSSY )>
{
	// Not strictly necessary. Any type exposing the used functions would fit (or otherwise throw a compiler error there)
	// static_assert( std::is_convertible<MicrofacetDistribution_T, MicrofacetDistribution>::value );
	// static_assert( std::is_convertible<Fresnel_T, Fresnel>::value );

	const float3 R;
	const MicrofacetDistribution_T distribution;
	const Fresnel_T fresnel;

	__device__ float distribution_Pdf( const float3& wo, const float3& wh ) const
	{
		return distribution.Pdf( wo, wh ) / ( 4.f * dot( wo, wh ) );
	}

  public:
	__device__ MicrofacetReflection( const float3& R,
									 const MicrofacetDistribution_T& distribution,
									 const Fresnel_T& fresnel )
		: R( R ),
		  distribution( distribution ),
		  fresnel( fresnel )
	{
	}

	__device__ float3 f( const float3& wo, const float3& wi ) const override
	{
		float cosThetaO = AbsCosTheta( wo ), cosThetaI = AbsCosTheta( wi );
		auto wh = wi + wo;
		// Handle degenerate cases for microfacet reflection
		if ( cosThetaI == 0 || cosThetaO == 0 ) return make_float3( 0.f );
		if ( wh.x == 0 && wh.y == 0 && wh.z == 0 ) return make_float3( 0.f );
		wh = normalize( wh );
		// For the Fresnel call, make sure that wh is in the same hemisphere
		// as the surface normal, so that TIR is handled correctly.
		// https://github.com/mmp/pbrt-v3/issues/229
		const auto F = fresnel.Evaluate(
			dot( wi, Faceforward( wh, make_float3( 0, 0, 1 ) ) ) );
		return R * distribution.D( wh ) * distribution.G( wo, wi ) * F /
			   ( 4 * cosThetaI * cosThetaO );
	}

	__device__ float3 Sample_f( const float3 wo, float3& wi,
								const float r0, const float r1,
								float& pdf, BxDFType& sampledType ) const override
	{
		// Sample microfacet orientation $\wh$ and reflected direction $\wi$
		if ( wo.z == 0.f ) return make_float3( 0.f );
		const auto wh = distribution.Sample_wh( wo, r0, r1 );
		if ( dot( wo, wh ) < 0 ) return make_float3( 0.f ); // Should be rare
		wi = pbrt_Reflect( wo, wh );
		if ( !SameHemisphere( wo, wi ) ) return make_float3( 0.f );

		// Compute PDF of _wi_ for microfacet reflection
		pdf = distribution_Pdf( wo, wh );
		return f( wo, wi );
	}

	__device__ float Pdf( const float3& wo, const float3& wi ) const override
	{
		if ( !SameHemisphere( wo, wi ) ) return 0.f;
		const auto wh = normalize( wo + wi );
		return distribution_Pdf( wo, wh );
	}
};

template <typename MicrofacetDistribution_T>
class MicrofacetTransmission : public BxDF_T<MicrofacetTransmission<MicrofacetDistribution_T>,
											 BxDFType( BxDFType::BSDF_TRANSMISSION | BxDFType::BSDF_GLOSSY )>
{
	const float3 T;
	const MicrofacetDistribution_T distribution;
	const float etaA, etaB;
	const TransportMode mode;

  public:
	__device__ MicrofacetTransmission( const float3& T,
									   const MicrofacetDistribution_T& distribution,
									   const float etaA,
									   const float etaB,
									   const TransportMode mode )
		: T( T ),
		  distribution( distribution ),
		  etaA( etaA ),
		  etaB( etaB ),
		  //   fresnel( etaA, etaB ),
		  mode( mode ){}
	{
	}

	__device__ float3 f( const float3& wo, const float3& wi ) const override
	{
		if ( SameHemisphere( wo, wi ) ) return make_float3( 0.f ); // transmission only

		const float cosThetaO = CosTheta( wo );
		const float cosThetaI = CosTheta( wi );
		if ( cosThetaI == 0 || cosThetaO == 0 ) return make_float3( 0.f );

		// Compute $\wh$ from $\wo$ and $\wi$ for microfacet transmission
		const float eta = CosTheta( wo ) > 0.f ? ( etaB / etaA ) : ( etaA / etaB );
		float3 wh = normalize( wo + wi * eta );
		if ( wh.z < 0 ) wh = -wh;

		// float3 F = fresnel.Evaluate( dot( wo, wh ) );
		float F = FrDielectric( dot( wo, wh ), etaA, etaB );

		const float sqrtDenom = dot( wo, wh ) + eta * dot( wi, wh );
		const float factor = ( mode == TransportMode::Radiance ) ? ( 1 / eta ) : 1;

		return ( 1.f - F ) *
			   std::abs( distribution.D( wh ) * distribution.G( wo, wi ) * eta * eta *
						 AbsDot( wi, wh ) * AbsDot( wo, wh ) * factor * factor /
						 ( cosThetaI * cosThetaO * sqrtDenom * sqrtDenom ) ) *
			   T;
	}

	__device__ float3 Sample_f( const float3 wo, float3& wi,
								const float r0, const float r1,
								float& pdf, BxDFType& sampledType ) const override
	{
		// Sample microfacet orientation $\wh$ and reflected direction $\wi$
		if ( wo.z == 0.f ) return make_float3( 0.f );
		const auto wh = distribution.Sample_wh( wo, r0, r1 );
		if ( dot( wo, wh ) < 0 ) return make_float3( 0.f ); // Should be rare

		const float eta = CosTheta( wo ) > 0 ? ( etaA / etaB ) : ( etaB / etaA );
		if ( !pbrt_Refract( wo, wh, eta, wi ) )
			return make_float3( 0.f );
		pdf = Pdf( wo, wi );
		return f( wo, wi );
	}

	__device__ float Pdf( const float3& wo, const float3& wi ) const override
	{
		if ( SameHemisphere( wo, wi ) ) return 0.f;

		// Compute $\wh$ from $\wo$ and $\wi$ for microfacet transmission
		const float eta = CosTheta( wo ) > 0 ? ( etaB / etaA ) : ( etaA / etaB );
		const float3 wh = normalize( wo + wi * eta );

		// Compute change of variables _dwh\_dwi_ for microfacet transmission
		const float sqrtDenom = dot( wo, wh ) + eta * dot( wi, wh );
		const float dwh_dwi =
			std::abs( ( eta * eta * dot( wi, wh ) ) / ( sqrtDenom * sqrtDenom ) );
		return distribution.Pdf( wo, wh ) * dwh_dwi;
	}
};
