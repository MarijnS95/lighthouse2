/* core_texture.h - Copyright 2019 Utrecht University

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

#pragma once

namespace lh2core
{

enum TextureType : char
{
	Invalid = 0,
	Constant = 1,
	Imagemap = 2,
};

template <typename T>
struct CoreTexture
{
	union {
		T constant;

		struct
		{
			uint32_t textureOffset;
			uint32_t width, height;
			bool trilinear;
		} imagemap;
	};

	TextureType type;

  public:
	CoreTexture() : type( Invalid ) {}
	CoreTexture( const T& value ) : constant( value ), type( Constant ) {}
	CoreTexture( uint32_t offset, uint32_t w, uint32_t h ) : imagemap{offset, w, h}, type( Imagemap ) {}

#ifdef __CUDACC__
	__device__ T Evaluate( float2 uv ) const;
#endif
};

} // namespace lh2core
