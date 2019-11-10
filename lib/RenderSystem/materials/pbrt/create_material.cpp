/* create_material.cpp - Copyright 2019 Utrecht University

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

	   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include "create_material.h"

#include "textures/constant.h"

namespace pbrt
{

SimpleHostMaterial<Disney>* CreateDisneyMaterial( const TextureParams& mp )
{
	Disney disney;
	disney.color = mp.GetConstantSpectrumTexture( "color", Spectrum( .5f ) ).vector();
	disney.metallic = mp.GetConstantFloatTexture( "metallic", 0.f );
	disney.eta = mp.GetConstantFloatTexture( "eta", 1.5f );
	disney.roughness = mp.GetConstantFloatTexture( "roughness", .5f );
	disney.specularTint = mp.GetConstantFloatTexture( "speculartint", 0.f );
	disney.anisotropic = mp.GetConstantFloatTexture( "anisotropic", 0.f );
	disney.sheen = mp.GetConstantFloatTexture( "sheen", 0.f );
	disney.sheenTint = mp.GetConstantFloatTexture( "sheentint", .5f );
	disney.clearcoat = mp.GetConstantFloatTexture( "clearcoat", 0.f );
	disney.clearcoatGloss = mp.GetConstantFloatTexture( "clearcoatgloss", 1.f );
	disney.specTrans = mp.GetConstantFloatTexture( "spectrans", 0.f );
	disney.scatterDistance = mp.GetConstantSpectrumTexture( "scatterdistance", Spectrum( 0.f ) ).vector();
	disney.thin = mp.FindBool( "thin", false );
	disney.flatness = mp.GetConstantFloatTexture( "flatness", 0.f );
	disney.diffTrans = mp.GetConstantFloatTexture( "difftrans", 0.f );

	if ( mp.GetFloatTextureOrNull( "bumpmap" ) )
		Error( "Bumpmaps not yet supported!" );

	return new SimpleHostMaterial<Disney>( disney );
}

SimpleHostMaterial<Glass>* CreateGlassMaterial( const TextureParams& mp )
{
	Glass glass;
	glass.R = mp.GetConstantSpectrumTexture( "Kr", Spectrum( 1.f ) ).vector();
	glass.T = mp.GetConstantSpectrumTexture( "Kt", Spectrum( 1.f ) ).vector();
	// Use "eta", otherwise fall back on "index", otherwise use 1.f:
	glass.eta = mp.GetConstantFloatTexture( "eta", mp.GetConstantFloatTexture( "index", 1.f ) );

	glass.urough = mp.GetConstantFloatTexture( "uroughness", 0.f );
	glass.vrough = mp.GetConstantFloatTexture( "vroughness", 0.f );

	if ( mp.GetFloatTextureOrNull( "bumpmap" ) )
		Error( "Bumpmaps not yet supported!" );

	glass.remapRoughness = mp.FindBool( "remaproughness", true );

	return new SimpleHostMaterial<Glass>( glass );
}

SimpleHostMaterial<Matte>* CreateMatteMaterial( const TextureParams& mp )
{
	Matte matte;
	matte.Kd = mp.GetConstantSpectrumTexture( "Kd", Spectrum( .5f ) ).vector();
	matte.sigma = mp.GetConstantFloatTexture( "sigma", 0.f );

	if ( mp.GetFloatTextureOrNull( "bumpmap" ) )
		Error( "Bumpmaps not yet supported!" );

	return new SimpleHostMaterial<Matte>( matte );
}

SimpleHostMaterial<Metal>* CreateMetalMaterial( const TextureParams& mp )
{
	// Values courtesy of PBRT
	const int CopperSamples = 56;
	const Float CopperWavelengths[CopperSamples] = {
		298.7570554, 302.4004341, 306.1337728, 309.960445, 313.8839949,
		317.9081487, 322.036826, 326.2741526, 330.6244747, 335.092373,
		339.6826795, 344.4004944, 349.2512056, 354.2405086, 359.374429,
		364.6593471, 370.1020239, 375.7096303, 381.4897785, 387.4505563,
		393.6005651, 399.9489613, 406.5055016, 413.2805933, 420.2853492,
		427.5316483, 435.0322035, 442.8006357, 450.8515564, 459.2006593,
		467.8648226, 476.8622231, 486.2124627, 495.936712, 506.0578694,
		516.6007417, 527.5922468, 539.0616435, 551.0407911, 563.5644455,
		576.6705953, 590.4008476, 604.8008683, 619.92089, 635.8162974,
		652.5483053, 670.1847459, 688.8009889, 708.4810171, 729.3186941,
		751.4192606, 774.9011125, 799.8979226, 826.5611867, 855.0632966,
		885.6012714};

	const Float CopperN[CopperSamples] = {
		1.400313, 1.38, 1.358438, 1.34, 1.329063, 1.325, 1.3325, 1.34,
		1.334375, 1.325, 1.317812, 1.31, 1.300313, 1.29, 1.281563, 1.27,
		1.249062, 1.225, 1.2, 1.18, 1.174375, 1.175, 1.1775, 1.18,
		1.178125, 1.175, 1.172812, 1.17, 1.165312, 1.16, 1.155312, 1.15,
		1.142812, 1.135, 1.131562, 1.12, 1.092437, 1.04, 0.950375, 0.826,
		0.645875, 0.468, 0.35125, 0.272, 0.230813, 0.214, 0.20925, 0.213,
		0.21625, 0.223, 0.2365, 0.25, 0.254188, 0.26, 0.28, 0.3};

	const Float CopperK[CopperSamples] = {
		1.662125, 1.687, 1.703313, 1.72, 1.744563, 1.77, 1.791625, 1.81,
		1.822125, 1.834, 1.85175, 1.872, 1.89425, 1.916, 1.931688, 1.95,
		1.972438, 2.015, 2.121562, 2.21, 2.177188, 2.13, 2.160063, 2.21,
		2.249938, 2.289, 2.326, 2.362, 2.397625, 2.433, 2.469187, 2.504,
		2.535875, 2.564, 2.589625, 2.605, 2.595562, 2.583, 2.5765, 2.599,
		2.678062, 2.809, 3.01075, 3.24, 3.458187, 3.67, 3.863125, 4.05,
		4.239563, 4.43, 4.619563, 4.817, 5.034125, 5.26, 5.485625, 5.717};

	static Spectrum copperN =
		Spectrum::FromSampled( CopperWavelengths, CopperN, CopperSamples );
	static Spectrum copperK =
		Spectrum::FromSampled( CopperWavelengths, CopperK, CopperSamples );

	Metal metal;

	metal.eta = mp.GetConstantSpectrumTexture( "eta", copperN ).vector();
	metal.k = mp.GetConstantSpectrumTexture( "k", copperN ).vector();

	auto roughness = mp.GetConstantFloatTexture( "roughness", .01f );
	// Try u/v, otherwise fall back on roughness:
	metal.urough = mp.GetConstantFloatTexture( "uroughness", roughness );
	metal.vrough = mp.GetConstantFloatTexture( "vroughness", roughness );

	metal.remapRoughness = mp.FindBool( "remaproughness", true );

	if ( mp.GetFloatTextureOrNull( "bumpmap" ) )
		Error( "Bumpmaps not yet supported!" );

	return new SimpleHostMaterial<Metal>( metal );
}
SimpleHostMaterial<Mirror>* CreateMirrorMaterial( const TextureParams& mp )
{
	Mirror mirror;
	mirror.Kr = mp.GetConstantSpectrumTexture( "Kr", .9f ).vector();

	if ( mp.GetFloatTextureOrNull( "bumpmap" ) )
		Error( "Bumpmaps not yet supported!" );

	return new SimpleHostMaterial<Mirror>( mirror );
}

SimpleHostMaterial<Plastic>* CreatePlasticMaterial( const TextureParams& mp )
{
	Plastic plastic;
	plastic.Kd = mp.GetConstantSpectrumTexture( "Kd", .25f ).vector();
	plastic.Ks = mp.GetConstantSpectrumTexture( "Ks", .25f ).vector();
	plastic.roughness = mp.GetConstantFloatTexture( "roughness", .1f );
	plastic.remapRoughness = mp.FindBool( "remaproughness", true );

	if ( mp.GetFloatTextureOrNull( "bumpmap" ) )
		Error( "Bumpmaps not yet supported!" );

	return new SimpleHostMaterial<Plastic>( plastic );
}

SimpleHostMaterial<Substrate>* CreateSubstrateMaterial( const TextureParams& mp )
{
	Substrate substrate;
	substrate.Kd = mp.GetConstantSpectrumTexture( "Kd", .5f ).vector();
	substrate.Ks = mp.GetConstantSpectrumTexture( "Ks", .5f ).vector();
	substrate.urough = mp.GetConstantFloatTexture( "uroughness", .1f );
	substrate.vrough = mp.GetConstantFloatTexture( "vroughness", .1f );
	substrate.remapRoughness = mp.FindBool( "remaproughness", true );

	if ( mp.GetFloatTextureOrNull( "bumpmap" ) )
		Error( "Bumpmaps not yet supported!" );

	return new SimpleHostMaterial<Substrate>( substrate );
}

}; // namespace pbrt
