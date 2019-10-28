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
		const auto F = fresnel.Evaluate( dot( wi, wh ) );
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
