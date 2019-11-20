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

class Uber : public SimpleMaterial<
				 common::materials::pbrt::Uber,
				 SpecularTransmission,
				 // It is possible to have two instances. Add it to ensure the stack is large enough
				 SpecularTransmission,
				 LambertianReflection,
				 MicrofacetReflection<TrowbridgeReitzDistribution<>, FresnelDielectric>,
				 SpecularReflection<FresnelDielectric>>
{

  public:
	__device__ void ComputeScatteringFunctions( const common::materials::pbrt::Uber& params,
												const float2 uv,
												const bool allowMultipleLobes,
												const TransportMode mode ) override
	{
		// TODO: Bumpmapping

		const auto e = params.eta.Evaluate( uv );
		const auto op = params.opacity.Evaluate( uv );
		const auto t = make_float3( 1.f ) - op;

		if ( !IsBlack( t ) )
			bxdfs.emplace_back<SpecularTransmission>( t, 1.f, 1.f, mode );

		const auto Kd = op * params.Kd.Evaluate( uv );
		if ( !IsBlack( Kd ) )
			bxdfs.emplace_back<LambertianReflection>( Kd );

		const auto Ks = op * params.Ks.Evaluate( uv );
		if ( !IsBlack( Ks ) )
		{
			const FresnelDielectric fresnel( 1.f, e );
			float roughu, roughv;
			roughu = params.roughnessu.Evaluate( uv );
			roughv = params.roughnessv.Evaluate( uv );

			if ( params.remapRoughness )
			{
				roughu = RoughnessToAlpha( roughu );
				roughv = RoughnessToAlpha( roughv );
			}

			const TrowbridgeReitzDistribution<> distrib( roughu, roughv );
			bxdfs.emplace_back<MicrofacetReflection<TrowbridgeReitzDistribution<>, FresnelDielectric>>( Ks, distrib, fresnel );
		}

		const auto Kr = op * params.Kr.Evaluate( uv );
		if ( !IsBlack( Kr ) )
		{
			const FresnelDielectric fresnel( 1.f, e );
			bxdfs.emplace_back<SpecularReflection<FresnelDielectric>>( Kr, fresnel );
		}

		const auto Kt = op * params.Kt.Evaluate( uv );
		if ( !IsBlack( Kt ) )
			bxdfs.emplace_back<SpecularTransmission>( Kt, 1.f, e, mode );
	}
};
