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

#ifdef __CUDA_ARCH__

LH2_DEVFUNC auto CreateGlass()
{
	constexpr TransportMode mode = TransportMode::Radiance;

	using Params = common::materials::pbrt::Glass;

	const auto isSpecular = [] __device__( const Params& params ) {
		float urough = params.urough;
		float vrough = params.vrough;
		return urough == 0 && vrough == 0;
	};

	return GuardNode(
		[] __device__( const Params& params ) {
			return !IsBlack( params.R ) || !IsBlack( params.T );
		},
		EitherNode(
			[isSpecular] __device__( const Params& params ) {
				// TODO: Pass transportmode and allowMultipleLobes
				constexpr bool allowMultipleLobes = true;

				return isSpecular( params ) && allowMultipleLobes;
			},
			BxDFNode<Params>( [] __device__( const Params& params ) {
				return FresnelSpecular( params.R, params.T, 1.f, params.eta, mode );
			} ),
			StackNode(
				EitherNode( isSpecular,
							BxDFNode<Params>( [] __device__( const Params& params ) {
								const FresnelDielectric fresnel( 1.f, params.eta );
								return SpecularReflection<FresnelDielectric>( params.R, fresnel );
							} ),
							BxDFNode<Params>( [] __device__( const Params& params ) {
								float urough = params.urough;
								float vrough = params.vrough;
								const TrowbridgeReitzDistribution<> distrib( urough, vrough );
								const FresnelDielectric fresnel( 1.f, params.eta );

								return MicrofacetReflection<TrowbridgeReitzDistribution<>, FresnelDielectric>( params.R, distrib, fresnel );
							} ) ),
				EitherNode( isSpecular,
							BxDFNode<Params>( [] __device__( const Params& params ) {
								return SpecularTransmission( params.T, 1.f, params.eta, mode );
							} ),
							BxDFNode<Params>( [] __device__( const Params& params ) {
								float urough = params.urough;
								float vrough = params.vrough;
								const TrowbridgeReitzDistribution<> distrib( urough, vrough );

								return MicrofacetTransmission<TrowbridgeReitzDistribution<>>( params.T, distrib, 1.f, params.eta, mode );
							} ) ) )
			//
			) );
};

using GlassStack = decltype( CreateGlass() );

class Glass : public StacklessMaterial<common::materials::pbrt::Glass, GlassStack>
{
  protected:
	__device__ GlassStack CreateBxDFStack() const override
	{
		return CreateGlass();
	}
};

#else

class Glass : public SimpleMaterial<
				  common::materials::pbrt::Glass,
				  FresnelSpecular,
				  SpecularReflection<FresnelDielectric>,
				  MicrofacetReflection<TrowbridgeReitzDistribution<>, FresnelDielectric>,
				  SpecularTransmission,
				  MicrofacetTransmission<TrowbridgeReitzDistribution<>>>
{

  public:
	__device__ void ComputeScatteringFunctions( const common::materials::pbrt::Glass& params,
												const bool allowMultipleLobes,
												const TransportMode mode ) override
	{
		// TODO: Bumpmapping

		if ( IsBlack( params.R ) && IsBlack( params.T ) )
			return;

		float urough = params.urough;
		float vrough = params.vrough;

		const bool isSpecular = urough == 0 && vrough == 0;

		if ( isSpecular && allowMultipleLobes )
		{
			bxdfs.emplace_back<FresnelSpecular>( params.R, params.T, 1.f, params.eta, mode );
		}
		else
		{
			// TODO: Do on CPU!!
			if ( params.remapRoughness )
			{
				urough = RoughnessToAlpha( urough );
				vrough = RoughnessToAlpha( vrough );
			}

			const TrowbridgeReitzDistribution<> distrib( urough, vrough );

			if ( !IsBlack( params.R ) )
			{
				const FresnelDielectric fresnel( 1.f, params.eta );
				if ( isSpecular )
					bxdfs.emplace_back<SpecularReflection<FresnelDielectric>>( params.R, fresnel );
				else
					bxdfs.emplace_back<MicrofacetReflection<TrowbridgeReitzDistribution<>, FresnelDielectric>>( params.R, distrib, fresnel );
			}

			if ( !IsBlack( params.T ) )
			{
				if ( isSpecular )
					bxdfs.emplace_back<SpecularTransmission>( params.T, 1.f, params.eta, mode );
				else
					bxdfs.emplace_back<MicrofacetTransmission<TrowbridgeReitzDistribution<>>>( params.T, distrib, 1.f, params.eta, mode );
			}
		}
	}
};

#endif
