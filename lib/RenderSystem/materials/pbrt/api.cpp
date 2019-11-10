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

namespace pbrt
{

Options PbrtOptions;

int catIndentCount = 0;

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
	Warning( "pbrtMaterial is not implemented!" );
}

void pbrtMakeNamedMaterial( const std::string& name, const ParamSet& params )
{
	Warning( "pbrtMakeNamedMaterial is not implemented!" );
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
