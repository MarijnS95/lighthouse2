/* create_material.h - Copyright 2019 Utrecht University

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

	   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   Glue between PBRT API and HostMaterial.
*/

#pragma once

#include <system.h>

#include "paramset.h"
#include "pbrt_wrap.h"

#include <memory>

namespace pbrt
{
using namespace materials::pbrt;
using namespace common::materials::pbrt;

DisneyHostMaterial* CreateDisneyMaterial( const TextureParams& mp );
GlassHostMaterial* CreateGlassMaterial( const TextureParams& mp );
MatteHostMaterial* CreateMatteMaterial( const TextureParams& mp );
MetalHostMaterial* CreateMetalMaterial( const TextureParams& mp );
MirrorHostMaterial* CreateMirrorMaterial( const TextureParams& mp );
PlasticHostMaterial* CreatePlasticMaterial( const TextureParams& mp );
SubstrateHostMaterial* CreateSubstrateMaterial( const TextureParams& mp );
UberHostMaterial* CreateUberMaterial( const TextureParams& mp );

}; // namespace pbrt
