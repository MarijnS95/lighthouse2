/* api.cpp - Copyright 2019 Utrecht University

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

	   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   In this file, the PBRT "API" is implemented. This API is used to add
   geometry, material info, cameras and a hoist of other settings to the
   scene. Conversion from "pbrt" to LH2 "Host" representation happens here.
*/

// core/api.cpp*
#include "api.h"

#include "create_material.h"

#include "paramset.h"
#include "texture.h"

namespace pbrt
{

// MaterialInstance represents both an instance of a material as well as
// the information required to create another instance of it (possibly with
// different parameters from the shape).
struct MaterialInstance
{
	MaterialInstance() = default;
	MaterialInstance( const std::string& name,
					  const std::shared_ptr<DynamicHostMaterial>& mtl,
					  ParamSet params )
		: name( name ), material( mtl ), params( std::move( params ) ) {}

	std::string name;
	std::shared_ptr<DynamicHostMaterial> material;
	ParamSet params;
};

struct GraphicsState
{
	// Graphics State Methods
	GraphicsState()
		: floatTextures( std::make_shared<FloatTextureMap>() ),
		  spectrumTextures( std::make_shared<SpectrumTextureMap>() ),
		  namedMaterials( std::make_shared<NamedMaterialMap>() )
	{
		ParamSet empty;
		TextureParams tp( empty, empty, *floatTextures, *spectrumTextures );
		std::shared_ptr<DynamicHostMaterial> mtl( CreateMatteMaterial( tp ) );
		currentMaterial = std::make_shared<MaterialInstance>( "matte", mtl, ParamSet() );
	}
	std::shared_ptr<DynamicHostMaterial> GetMaterialForShape( const ParamSet& geomParams );
	// MediumInterface CreateMediumInterface();

	// Graphics State
	// std::string currentInsideMedium, currentOutsideMedium;

	// Updated after book publication: floatTextures, spectrumTextures, and
	// namedMaterials are all implemented using a "copy on write" approach
	// for more efficient GraphicsState management.  When state is pushed
	// in pbrtAttributeBegin(), we don't immediately make a copy of these
	// maps, but instead record that each one is shared.  Only if an item
	// is added to one is a unique copy actually made.
	using FloatTextureMap = std::map<std::string, std::shared_ptr<Texture<Float>>>;
	std::shared_ptr<FloatTextureMap> floatTextures;
	bool floatTexturesShared = false;

	using SpectrumTextureMap = std::map<std::string, std::shared_ptr<Texture<Spectrum>>>;
	std::shared_ptr<SpectrumTextureMap> spectrumTextures;
	bool spectrumTexturesShared = false;

	using NamedMaterialMap = std::map<std::string, std::shared_ptr<MaterialInstance>>;
	std::shared_ptr<NamedMaterialMap> namedMaterials;
	bool namedMaterialsShared = false;

	std::shared_ptr<MaterialInstance> currentMaterial;
	ParamSet areaLightParams;
	std::string areaLight;
	bool reverseOrientation = false;
};

GraphicsState graphicsState;

Options PbrtOptions;

int catIndentCount = 0;

static std::shared_ptr<DynamicHostMaterial> MakeMaterial( const std::string& name,
														  const TextureParams& mp )
{
	std::shared_ptr<DynamicHostMaterial> material = nullptr;
	if ( name == "" || name == "none" )
		return nullptr;
	else if ( name == "matte" )
		material = CreateMatteMaterial( mp );
	else if ( name == "plastic" )
		material = CreatePlasticMaterial( mp );
	// else if ( name == "translucent" )
	// 	material = CreateTranslucentMaterial( mp );
	else if ( name == "glass" )
		material = CreateGlassMaterial( mp );
	else if ( name == "mirror" )
		material = CreateMirrorMaterial( mp );
	// else if ( name == "hair" )
	// 	material = CreateHairMaterial( mp );
	else if ( name == "disney" )
		material = CreateDisneyMaterial( mp );
#if 0 // Not implemented
	else if ( name == "mix" )
	{
		std::string m1 = mp.FindString( "namedmaterial1", "" );
		std::string m2 = mp.FindString( "namedmaterial2", "" );
		std::shared_ptr<DynamicHostMaterial> mat1, mat2;
		if ( graphicsState.namedMaterials->find( m1 ) ==
			 graphicsState.namedMaterials->end() )
		{
			Error( "Named material \"%s\" undefined.  Using \"matte\"",
				   m1.c_str() );
			mat1 = MakeMaterial( "matte", mp );
		}
		else
			mat1 = ( *graphicsState.namedMaterials )[m1]->material;

		if ( graphicsState.namedMaterials->find( m2 ) ==
			 graphicsState.namedMaterials->end() )
		{
			Error( "Named material \"%s\" undefined.  Using \"matte\"",
				   m2.c_str() );
			mat2 = MakeMaterial( "matte", mp );
		}
		else
			mat2 = ( *graphicsState.namedMaterials )[m2]->material;

		material = CreateMixMaterial( mp, mat1, mat2 );
	}
#endif
	else if ( name == "metal" )
		material = CreateMetalMaterial( mp );
	else if ( name == "substrate" )
		material = CreateSubstrateMaterial( mp );
#if 0 // Not implemented
	else if ( name == "uber" )
		material = CreateUberMaterial( mp );
	else if ( name == "subsurface" )
		material = CreateSubsurfaceMaterial( mp );
	else if ( name == "kdsubsurface" )
		material = CreateKdSubsurfaceMaterial( mp );
	else if ( name == "fourier" )
		material = CreateFourierMaterial( mp );
#endif
	else
	{
		Warning( "Material \"%s\" unknown. Using \"matte\".", name.c_str() );
		material = CreateMatteMaterial( mp );
	}

#if 0 // All existing implementations are pathtracers
	if ( ( name == "subsurface" || name == "kdsubsurface" ) &&
		 ( renderOptions->IntegratorName != "path" &&
		   ( renderOptions->IntegratorName != "volpath" ) ) )
		Warning(
			"Subsurface scattering material \"%s\" used, but \"%s\" "
			"integrator doesn't support subsurface scattering. "
			"Use \"path\" or \"volpath\".",
			name.c_str(), renderOptions->IntegratorName.c_str() );
#endif

	mp.ReportUnused();
	if ( !material )
		Error( "Unable to create material \"%s\"", name.c_str() );
	return material;
}

void pbrtInit( const Options& opt )
{
	PbrtOptions = opt;
}

void pbrtCleanup()
{
	Warning( "pbrtCleanup is not implemented!" );
}

void pbrtIdentity()
{
	Warning( "pbrtIdentity is not implemented!" );
}

void pbrtTranslate( Float dx, Float dy, Float dz )
{
	Warning( "pbrtTranslate is not implemented!" );
}

void pbrtRotate( Float angle, Float ax, Float ay, Float az )
{
	Warning( "pbrtRotate is not implemented!" );
}

void pbrtScale( Float sx, Float sy, Float sz )
{
	Warning( "pbrtScale is not implemented!" );
}

void pbrtLookAt( Float ex, Float ey, Float ez, Float lx, Float ly, Float lz,
				 Float ux, Float uy, Float uz )
{
	Warning( "pbrtLookAt is not implemented!" );
}

void pbrtConcatTransform( Float transform[16] )
{
	Warning( "pbrtConcatTransform is not implemented!" );
}

void pbrtTransform( Float transform[16] )
{
	Warning( "pbrtTransform is not implemented!" );
}

void pbrtCoordinateSystem( const std::string& )
{
	Warning( "pbrtCoordinateSystem is not implemented!" );
}

void pbrtCoordSysTransform( const std::string& )
{
	Warning( "pbrtCoordSysTransform is not implemented!" );
}

void pbrtActiveTransformAll()
{
	Warning( "pbrtActiveTransformAll is not implemented!" );
}

void pbrtActiveTransformEndTime()
{
	Warning( "pbrtActiveTransformEndTime is not implemented!" );
}

void pbrtActiveTransformStartTime()
{
	Warning( "pbrtActiveTransformStartTime is not implemented!" );
}

void pbrtTransformTimes( Float start, Float end )
{
	Warning( "pbrtTransformTimes is not implemented!" );
}

void pbrtPixelFilter( const std::string& name, const ParamSet& params )
{
	Warning( "pbrtPixelFilter is not implemented!" );
}

void pbrtFilm( const std::string& type, const ParamSet& params )
{
	Warning( "pbrtFilm is not implemented!" );
}

void pbrtSampler( const std::string& name, const ParamSet& params )
{
	Warning( "pbrtSampler is not implemented!" );
}

void pbrtAccelerator( const std::string& name, const ParamSet& params )
{
	Warning( "pbrtAccelerator is not implemented!" );
}

void pbrtIntegrator( const std::string& name, const ParamSet& params )
{
	Warning( "pbrtIntegrator is not implemented!" );
}

void pbrtCamera( const std::string&, const ParamSet& cameraParams )
{
	Warning( "pbrtCamera is not implemented!" );
}

void pbrtMakeNamedMedium( const std::string& name, const ParamSet& params )
{
	Warning( "pbrtMakeNamedMedium is not implemented!" );
}

void pbrtMediumInterface( const std::string& insideName, const std::string& outsideName )
{
	Warning( "pbrtMediumInterface is not implemented!" );
}

void pbrtWorldBegin()
{
	Warning( "pbrtWorldBegin is not implemented!" );
}

void pbrtAttributeBegin()
{
	Warning( "pbrtAttributeBegin is not implemented!" );
}

void pbrtAttributeEnd()
{
	Warning( "pbrtAttributeEnd is not implemented!" );
}

void pbrtTransformBegin()
{
	Warning( "pbrtTransformBegin is not implemented!" );
}

void pbrtTransformEnd()
{
	Warning( "pbrtTransformEnd is not implemented!" );
}

void pbrtTexture( const std::string& name, const std::string& type, const std::string& texname, const ParamSet& params )
{
	Warning( "pbrtTexture is not implemented!" );
}

void pbrtMaterial( const std::string& name, const ParamSet& params )
{
	ParamSet emptyParams;
	TextureParams mp( params, emptyParams, *graphicsState.floatTextures,
					  *graphicsState.spectrumTextures );

	auto mtl = MakeMaterial( name, mp );
	graphicsState.currentMaterial =
		std::make_shared<MaterialInstance>( name, mtl, params );

	if ( PbrtOptions.cat || PbrtOptions.toPly )
	{
		printf( "%*sMaterial \"%s\" ", catIndentCount, "", name.c_str() );
		params.Print( catIndentCount );
		printf( "\n" );
	}
}

void pbrtMakeNamedMaterial( const std::string& name, const ParamSet& params )
{
	ParamSet emptyParams;
	TextureParams mp( params, emptyParams, *graphicsState.floatTextures,
					  *graphicsState.spectrumTextures );
	std::string matName = mp.FindString( "type" );
	// WARN_IF_ANIMATED_TRANSFORM( "MakeNamedMaterial" );
	if ( matName == "" )
		Error( "No parameter string \"type\" found in MakeNamedMaterial" );

	if ( PbrtOptions.cat || PbrtOptions.toPly )
	{
		printf( "%*sMakeNamedMaterial \"%s\" ", catIndentCount, "",
				name.c_str() );
		params.Print( catIndentCount );
		printf( "\n" );
	}
	else
	{
		auto mtl = MakeMaterial( matName, mp );
		if ( graphicsState.namedMaterials->find( name ) !=
			 graphicsState.namedMaterials->end() )
			Warning( "Named material \"%s\" redefined.", name.c_str() );
		if ( graphicsState.namedMaterialsShared )
		{
			graphicsState.namedMaterials =
				std::make_shared<GraphicsState::NamedMaterialMap>( *graphicsState.namedMaterials );
			graphicsState.namedMaterialsShared = false;
		}
		( *graphicsState.namedMaterials )[name] =
			std::make_shared<MaterialInstance>( matName, mtl, params );
	}
}

void pbrtNamedMaterial( const std::string& name )
{
	Warning( "pbrtNamedMaterial is not implemented!" );
}

void pbrtLightSource( const std::string& name, const ParamSet& params )
{
	Warning( "pbrtLightSource is not implemented!" );
}

void pbrtAreaLightSource( const std::string& name, const ParamSet& params )
{
	Warning( "pbrtAreaLightSource is not implemented!" );
}

void pbrtShape( const std::string& name, const ParamSet& params )
{
	Warning( "pbrtShape is not implemented!" );
}

void pbrtReverseOrientation()
{
	Warning( "pbrtReverseOrientation is not implemented!" );
}

void pbrtObjectBegin( const std::string& name )
{
	Warning( "pbrtObjectBegin is not implemented!" );
}

void pbrtObjectEnd()
{
	Warning( "pbrtObjectEnd is not implemented!" );
}

void pbrtObjectInstance( const std::string& name )
{
	Warning( "pbrtObjectInstance is not implemented!" );
}

void pbrtWorldEnd()
{
	Warning( "pbrtWorldEnd is not implemented!" );
}

}; // namespace pbrt
