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
#include <rendersystem.h>

#include "create_material.h"

#include "paramset.h"
#include "texture.h"

#include "shapes/plymesh.h"


namespace pbrt
{

// API Local Classes
PBRT_CONSTEXPR int MaxTransforms = 2;
PBRT_CONSTEXPR int StartTransformBits = 1 << 0;
PBRT_CONSTEXPR int EndTransformBits = 1 << 1;
PBRT_CONSTEXPR int AllTransformsBits = ( 1 << MaxTransforms ) - 1;
struct TransformSet
{
	// TransformSet Public Methods
	Transform& operator[]( int i )
	{
		CHECK_GE( i, 0 );
		CHECK_LT( i, MaxTransforms );
		return t[i];
	}
	const Transform& operator[]( int i ) const
	{
		CHECK_GE( i, 0 );
		CHECK_LT( i, MaxTransforms );
		return t[i];
	}
	friend TransformSet Inverse( const TransformSet& ts )
	{
		TransformSet tInv;
		for ( int i = 0; i < MaxTransforms; ++i ) tInv.t[i] = ts.t[i].Inverted();
		return tInv;
	}
	bool IsAnimated() const
	{
		for ( int i = 0; i < MaxTransforms - 1; ++i )
			if ( t[i] != t[i + 1] ) return true;
		return false;
	}

  private:
	Transform t[MaxTransforms];
};

// MaterialInstance represents both an instance of a material as well as
// the information required to create another instance of it (possibly with
// different parameters from the shape).
struct MaterialInstance
{
	MaterialInstance() = default;
	MaterialInstance( const std::string& name,
					  DynamicHostMaterial* mtl,
					  ParamSet params )
		: name( name ), material( mtl ), params( std::move( params ) ) {}

	std::string name;
	DynamicHostMaterial* material;
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
		DynamicHostMaterial* mtl = CreateMatteMaterial( tp );
		currentMaterial = std::make_shared<MaterialInstance>( "matte", mtl, ParamSet() );
	}
	DynamicHostMaterial* GetMaterialForShape( const ParamSet& geomParams );
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

static TransformSet curTransform;
static uint32_t activeTransformBits = AllTransformsBits;
static std::map<std::string, TransformSet> namedCoordinateSystems;
GraphicsState graphicsState;
static std::vector<GraphicsState> pushedGraphicsStates;
static std::vector<TransformSet> pushedTransforms;
static std::vector<uint32_t> pushedActiveTransformBits;

HostScene* hostScene;

Options PbrtOptions;

enum class APIState
{
	Uninitialized,
	OptionsBlock,
	WorldBlock
};
static APIState currentApiState = APIState::Uninitialized;
int catIndentCount = 0;

// API Macros
#define VERIFY_INITIALIZED( func )                         \
	if ( !( PbrtOptions.cat || PbrtOptions.toPly ) &&      \
		 currentApiState == APIState::Uninitialized )      \
	{                                                      \
		Error(                                             \
			"pbrtInit() must be before calling \"%s()\". " \
			"Ignoring.",                                   \
			func );                                        \
		return;                                            \
	}                                                      \
	else /* swallow trailing semicolon */
#define VERIFY_OPTIONS( func )                           \
	VERIFY_INITIALIZED( func );                          \
	if ( !( PbrtOptions.cat || PbrtOptions.toPly ) &&    \
		 currentApiState == APIState::WorldBlock )       \
	{                                                    \
		Error(                                           \
			"Options cannot be set inside world block; " \
			"\"%s\" not allowed.  Ignoring.",            \
			func );                                      \
		return;                                          \
	}                                                    \
	else /* swallow trailing semicolon */
#define VERIFY_WORLD( func )                                 \
	VERIFY_INITIALIZED( func );                              \
	if ( !( PbrtOptions.cat || PbrtOptions.toPly ) &&        \
		 currentApiState == APIState::OptionsBlock )         \
	{                                                        \
		Error(                                               \
			"Scene description must be inside world block; " \
			"\"%s\" not allowed. Ignoring.",                 \
			func );                                          \
		return;                                              \
	}                                                        \
	else /* swallow trailing semicolon */
#define FOR_ACTIVE_TRANSFORMS( expr )           \
	for ( int i = 0; i < MaxTransforms; ++i )   \
		if ( activeTransformBits & ( 1 << i ) ) \
		{                                       \
			expr                                \
		}
#if 1
#define WARN_IF_ANIMATED_TRANSFORM( func )
#else
// TODO
#define WARN_IF_ANIMATED_TRANSFORM( func )                           \
	do                                                               \
	{                                                                \
		if ( curTransform.IsAnimated() )                             \
			Warning(                                                 \
				"Animated transformations set; ignoring for \"%s\" " \
				"and using the start transform only",                \
				func );                                              \
	} while ( false ) /* swallow trailing semicolon */
#endif

// Object Creation Function Definitions
HostMesh* MakeShapes( const std::string& name,
					  const Transform* object2world,
					  const Transform* world2object,
					  bool reverseOrientation,
					  const ParamSet& paramSet,
					  const int materialIdx )
{
	if ( name == "plymesh" )
		return CreatePLYMesh( object2world, world2object, reverseOrientation,
							  paramSet, materialIdx, &*graphicsState.floatTextures );
	else
		Warning( "Shape \"%s\" unknown.", name.c_str() );
	return nullptr;
}

DynamicHostMaterial* MakeMaterial( const std::string& name,
								   const TextureParams& mp )
{
	DynamicHostMaterial* material = nullptr;
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
		DynamicHostMaterial *mat1, *mat2;
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

void pbrtInit( const Options& opt, HostScene* hs )
{
	PbrtOptions = opt;
	hostScene = hs;
	// API Initialization
	if ( currentApiState != APIState::Uninitialized )
		Error( "pbrtInit() has already been called." );
	currentApiState = APIState::OptionsBlock;
	// renderOptions.reset( new RenderOptions );
	graphicsState = GraphicsState();
	catIndentCount = 0;

	// General \pbrt Initialization
	SampledSpectrum::Init();
}

void pbrtCleanup()
{
	// API Cleanup
	if ( currentApiState == APIState::Uninitialized )
		Error( "pbrtCleanup() called without pbrtInit()." );
	else if ( currentApiState == APIState::WorldBlock )
		Error( "pbrtCleanup() called while inside world block." );
	currentApiState = APIState::Uninitialized;
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

void pbrtConcatTransform( Float tr[16] )
{
	VERIFY_INITIALIZED( "ConcatTransform" );
	FOR_ACTIVE_TRANSFORMS(
		curTransform[i] =
			curTransform[i] *
			Transform( mat4{tr[0], tr[4], tr[8], tr[12], tr[1], tr[5],
							tr[9], tr[13], tr[2], tr[6], tr[10], tr[14],
							tr[3], tr[7], tr[11], tr[15]} ); )
	if ( PbrtOptions.cat || PbrtOptions.toPly )
	{
		printf( "%*sConcatTransform [ ", catIndentCount, "" );
		for ( int i = 0; i < 16; ++i ) printf( "%.9g ", tr[i] );
		printf( " ]\n" );
	}
}

void pbrtTransform( Float tr[16] )
{
	VERIFY_INITIALIZED( "Transform" );
	FOR_ACTIVE_TRANSFORMS(
		curTransform[i] = Transform( mat4{
			tr[0], tr[4], tr[8], tr[12], tr[1], tr[5], tr[9], tr[13], tr[2],
			tr[6], tr[10], tr[14], tr[3], tr[7], tr[11], tr[15]} ); )
	if ( PbrtOptions.cat || PbrtOptions.toPly )
	{
		printf( "%*sTransform [ ", catIndentCount, "" );
		for ( int i = 0; i < 16; ++i ) printf( "%.9g ", tr[i] );
		printf( " ]\n" );
	}
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
	VERIFY_OPTIONS( "WorldBegin" );
	currentApiState = APIState::WorldBlock;
	for ( int i = 0; i < MaxTransforms; ++i ) curTransform[i] = Transform();
	activeTransformBits = AllTransformsBits;
	namedCoordinateSystems["world"] = curTransform;
	if ( PbrtOptions.cat || PbrtOptions.toPly )
		printf( "\n\nWorldBegin\n\n" );
}

void pbrtAttributeBegin()
{
	VERIFY_WORLD( "AttributeBegin" );
	pushedGraphicsStates.push_back( graphicsState );
	graphicsState.floatTexturesShared = graphicsState.spectrumTexturesShared =
		graphicsState.namedMaterialsShared = true;
	pushedTransforms.push_back( curTransform );
	pushedActiveTransformBits.push_back( activeTransformBits );
	if ( PbrtOptions.cat || PbrtOptions.toPly )
	{
		printf( "\n%*sAttributeBegin\n", catIndentCount, "" );
		catIndentCount += 4;
	}
}

void pbrtAttributeEnd()
{
	VERIFY_WORLD( "AttributeEnd" );
	if ( !pushedGraphicsStates.size() )
	{
		Error(
			"Unmatched pbrtAttributeEnd() encountered. "
			"Ignoring it." );
		return;
	}
	graphicsState = std::move( pushedGraphicsStates.back() );
	pushedGraphicsStates.pop_back();
	curTransform = pushedTransforms.back();
	pushedTransforms.pop_back();
	activeTransformBits = pushedActiveTransformBits.back();
	pushedActiveTransformBits.pop_back();
	if ( PbrtOptions.cat || PbrtOptions.toPly )
	{
		catIndentCount -= 4;
		printf( "%*sAttributeEnd\n", catIndentCount, "" );
	}
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
	WARN_IF_ANIMATED_TRANSFORM( "MakeNamedMaterial" );
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
	VERIFY_WORLD( "NamedMaterial" );
	if ( PbrtOptions.cat || PbrtOptions.toPly )
	{
		printf( "%*sNamedMaterial \"%s\"\n", catIndentCount, "", name.c_str() );
		return;
	}

	auto iter = graphicsState.namedMaterials->find( name );
	if ( iter == graphicsState.namedMaterials->end() )
	{
		Error( "NamedMaterial \"%s\" unknown.", name.c_str() );
		return;
	}
	graphicsState.currentMaterial = iter->second;
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
	VERIFY_WORLD( "Shape" );
	// std::vector<std::shared_ptr<Primitive>> prims;
	// std::vector<std::shared_ptr<AreaLight>> areaLights;
	if ( PbrtOptions.cat || ( PbrtOptions.toPly && name != "trianglemesh" ) )
	{
		printf( "%*sShape \"%s\" ", catIndentCount, "", name.c_str() );
		params.Print( catIndentCount );
		printf( "\n" );
	}

	if ( curTransform.IsAnimated() )
		Error( "No animated transform loader yet!" );

	auto mtl = graphicsState.GetMaterialForShape( params );
	auto materialIdx = hostScene->AddMaterial( mtl );

	// Initialize _prims_ and _areaLights_ for static shape

	// Create shapes for shape _name_
	// Transform* ObjToWorld = transformCache.Lookup( curTransform[0] );
	// Transform* WorldToObj = transformCache.Lookup( Inverse( curTransform[0] ) );
	Transform ObjToWorld = curTransform[0];
	Transform WorldToObj = curTransform[0].Inverted();
	auto hostMesh = MakeShapes( name, &ObjToWorld, &WorldToObj,
								graphicsState.reverseOrientation, params, materialIdx );
	// if ( shapes.empty() ) return;
	params.ReportUnused();

	if ( !hostMesh )
	{
		Warning( "No mesh created for %s", name.c_str() );
		return;
	}

	// TODO: Medium and area lights
#if 0
	MediumInterface mi = graphicsState.CreateMediumInterface();
	prims.reserve( shapes.size() );
	for ( auto s : shapes )
	{
		// Possibly create area light for shape
		std::shared_ptr<AreaLight> area;
		if ( graphicsState.areaLight != "" )
		{
			area = MakeAreaLight( graphicsState.areaLight, curTransform[0],
								  mi, graphicsState.areaLightParams, s );
			if ( area ) areaLights.push_back( area );
		}
		prims.push_back(
			std::make_shared<GeometricPrimitive>( s, mtl, area, mi ) );
	}
#endif

	CHECK( hostScene );

	auto meshIdx = hostScene->AddMesh( hostMesh );
	hostScene->AddInstance( meshIdx, ObjToWorld );

	// Add _prims_ and _areaLights_ to scene or current instance
	// if ( renderOptions->currentInstance )
	// {
	// 	if ( areaLights.size() )
	// 		Warning( "Area lights not supported with object instancing" );
	// 	renderOptions->currentInstance->insert(
	// 		renderOptions->currentInstance->end(), prims.begin(), prims.end() );
	// }
	// else
	// {
	// 	renderOptions->primitives.insert( renderOptions->primitives.end(),
	// 									  prims.begin(), prims.end() );
	// 	if ( areaLights.size() )
	// 		renderOptions->lights.insert( renderOptions->lights.end(),
	// 									  areaLights.begin(), areaLights.end() );
	// }
}

// Attempt to determine if the ParamSet for a shape may provide a value for
// its material's parameters. Unfortunately, materials don't provide an
// explicit representation of their parameters that we can query and
// cross-reference with the parameter values available from the shape.
//
// Therefore, we'll apply some "heuristics".
bool shapeMaySetMaterialParameters( const ParamSet& ps )
{
	for ( const auto& param : ps.textures )
		// Any texture other than one for an alpha mask is almost certainly
		// for a Material (or is unused!).
		if ( param->name != "alpha" && param->name != "shadowalpha" )
			return true;

	// Special case spheres, which are the most common non-mesh primitive.
	for ( const auto& param : ps.floats )
		if ( param->nValues == 1 && param->name != "radius" )
			return true;

	// Extra special case strings, since plymesh uses "filename", curve "type",
	// and loopsubdiv "scheme".
	for ( const auto& param : ps.strings )
		if ( param->nValues == 1 && param->name != "filename" &&
			 param->name != "type" && param->name != "scheme" )
			return true;

	// For all other parameter types, if there is a single value of the
	// parameter, assume it may be for the material. This should be valid
	// (if conservative), since no materials currently take array
	// parameters.
	for ( const auto& param : ps.bools )
		if ( param->nValues == 1 )
			return true;
	for ( const auto& param : ps.ints )
		if ( param->nValues == 1 )
			return true;
	for ( const auto& param : ps.point2fs )
		if ( param->nValues == 1 )
			return true;
	for ( const auto& param : ps.vector2fs )
		if ( param->nValues == 1 )
			return true;
	for ( const auto& param : ps.point3fs )
		if ( param->nValues == 1 )
			return true;
	for ( const auto& param : ps.vector3fs )
		if ( param->nValues == 1 )
			return true;
	for ( const auto& param : ps.normals )
		if ( param->nValues == 1 )
			return true;
	for ( const auto& param : ps.spectra )
		if ( param->nValues == 1 )
			return true;

	return false;
}

DynamicHostMaterial* GraphicsState::GetMaterialForShape(
	const ParamSet& shapeParams )
{
	CHECK( currentMaterial );
	if ( shapeMaySetMaterialParameters( shapeParams ) )
	{
		// Only create a unique material for the shape if the shape's
		// parameters are (apparently) going to provide values for some of
		// the material parameters.
		TextureParams mp( shapeParams, currentMaterial->params, *floatTextures,
						  *spectrumTextures );
		return MakeMaterial( currentMaterial->name, mp );
	}
	else
		return currentMaterial->material;
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
