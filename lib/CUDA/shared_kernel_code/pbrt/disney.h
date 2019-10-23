/*
    pbrt source code is Copyright(c) 1998-2017
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

// ----------------------------------------------------------------

// https://seblagarde.wordpress.com/2013/04/29/memo-on-fresnel-equations/
//
// The Schlick Fresnel approximation is:
//
// R = R(0) + (1 - R(0)) (1 - cos theta)^5,
//
// where R(0) is the reflectance at normal indicence.
LH2_DEVFUNC float SchlickWeight( float cosTheta )
{
	float m = clamp( 1.f - cosTheta, 0.f, 1.f );
	return ( m * m ) * ( m * m ) * m;
}

// ----------------------------------------------------------------

class DisneyDiffuse : public BxDF_T<DisneyDiffuse,
									BxDFType( BxDFType::BSDF_REFLECTION | BxDFType::BSDF_DIFFUSE )>
{
	const float3 R;

  public:
	__device__ DisneyDiffuse( const float3& r ) : R( r )
	{
	}

	__device__ float3 f( const float NDotV, const float NDotL ) const override
	{
		float Fo = SchlickWeight( NDotV ),
			  Fi = SchlickWeight( NDotL );

		// Diffuse fresnel - go from 1 at normal incidence to .5 at grazing.
		// Burley 2015, eq (4).
		return R * INVPI * ( 1.f - Fo * .5f ) * ( 1.f - Fi * .5f );
	}
};

/**
 * DisneyGltf: Disney material expressed as PBRT BxDF stack.
 * Material input data does not match
 */
class DisneyGltf : public BSDFStackMaterial<DisneyDiffuse>
{
  public:
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
		// metallic: Controls how "metal" the object appears. Higher values reduce diffuse scattering and shift the highlight color towards the material's color. Range: [0,1].
		// spectrans: Controls contribution of glossy specular transmission. Range: [0,1].

		ShadingData shadingData;

		GetShadingData( D, u, v, coneWidth, tri, instIdx, shadingData, N, iN, fN, T, waveLength );

		float strans = TRANSMISSION;
		float diffuseWeight = ( 1.f - METALLIC ) * ( 1.f - strans );
		const float3 c = shadingData.color;

		if ( diffuseWeight > 0 )
		{
			if ( /* thin */ false )
			{
				// TODO: use thin, difftrans and flatness properties
				const float3 s = make_float3( sqrt( c.x ), sqrt( c.y ), sqrt( c.z ) );
				// TODO: Weight with flatness!

				// new ( Reserve() ) DisneyDiffuse( diffuseWeight * s );
				// new ( Reserve() ) DisneyFakeSS( diffuseWeight * s );
			}
			else
			{
				if ( /* scatterdistance == 0 */ true )
				{
					// Add( stack, BxDF_INSTANCE_DISNEY_DIFFUSE, diffuseWeight * c );
					new ( Reserve() ) DisneyDiffuse( diffuseWeight * c );
				}
				else
				{
					// TODO: bssrdf
				}
			}

			// Add( stack, BxDF_INSTANCE_DISNEY_RETRO, diffuseWeight * c );

			const float lum = 0.212671f * c.x + 0.715160f * c.y + 0.072169f * c.z;
			// normalize lum. to isolate hue+sat
			float3 Ctint = lum > 0 ? ( c / lum ) : make_float3( 1.f );

			float sheenWeight = SHEEN;
			if ( sheenWeight > 0 )
			{
				float stint = SHEENTINT;
				float3 Csheen = pbrt_Lerp( stint, make_float3( 1.f ), Ctint );

				// Add( stack, BxDF_INSTANCE_DISNEY_SHEEN, diffuseWeight * sheenWeight * Csheen );
			}
		}
	}
};
