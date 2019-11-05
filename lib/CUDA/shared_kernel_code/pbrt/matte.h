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

LH2_DEVFUNC auto CreateMatte()
{
	using Params = common::materials::pbrt::Matte;
	return GuardNode(
		[] __device__( const Params& params ) {
			return !IsBlack( params.Kd );
		},
		EitherNode(
			[] __device__( const Params& params ) {
				const float sig = clamp( params.sigma, 0.f, 90.f );
				return sig == 0.f;
			},
			BxDFNode<Params>( [] __device__( const Params& params ) {
				return LambertianReflection( params.Kd );
			} ),
			BxDFNode<Params>( [] __device__( const Params& params ) {
				const float sig = clamp( params.sigma, 0.f, 90.f );
				return OrenNayar( params.Kd, sig );
			} ) )

	);
};

using MatteStack = decltype( CreateMatte() );

class Matte : public StacklessMaterial<common::materials::pbrt::Matte, MatteStack>
{
  protected:
	__device__ MatteStack CreateBxDFStack() const override
	{
		return CreateMatte();
	}
};

#else

class Matte : public SimpleMaterial<
				  common::materials::pbrt::Matte,
				  LambertianReflection,
				  OrenNayar>
{

  public:
	__device__ void ComputeScatteringFunctions( const common::materials::pbrt::Matte& params,
												const bool allowMultipleLobes,
												const TransportMode mode ) override
	{
		// TODO: Bumpmapping

		if ( IsBlack( params.Kd ) )
			return;

		const float sig = clamp( params.sigma, 0.f, 90.f );

		if ( sig == 0.f )
			bxdfs.emplace_back<LambertianReflection>( params.Kd );
		else
			bxdfs.emplace_back<OrenNayar>( params.Kd, sig );
	}
};

#endif
