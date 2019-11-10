/* pbrt_wrap.h - Copyright 2019 Utrecht University

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

	   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   LH2 variant of pbrt.h. Forward declarations, configuration and
   typedefs from common LH2 types to PBRT types.
*/

#pragma once

#include "stringprint.h"
#include <numeric>
#include <system.h>

namespace pbrt
{

struct Options
{
	bool cat = false, toPly = false;
};

extern Options PbrtOptions;

using Float = float;
using Vector3f = float3;
using Vector2f = float2;
using Normal3f = float3;
using Point3f = float3;
using Point2f = float2;

#define PBRT_CONSTEXPR constexpr

static PBRT_CONSTEXPR Float Infinity = std::numeric_limits<Float>::infinity();

#define Warning( format, ... ) printf( "Warning: " format "\n", ##__VA_ARGS__ )

template <typename... Args>
void Error( const char* format, Args&&... args )
{
	FatalError( format, std::forward<Args>( args )... );
}

template <int nSpectrumSamples>
class CoefficientSpectrum;

class RGBSpectrum;
class SampledSpectrum;
#ifdef PBRT_SAMPLED_SPECTRUM
// All sampled spectrums must be converted to RGB for GPU processing.
#error "LH2 Does not support SampledSpectrum!"
typedef SampledSpectrum Spectrum;
#else
typedef RGBSpectrum Spectrum;
#endif

class ParamSet;
template <typename T>
struct ParamSetItem;

#define CHECK_EQ( val1, val2 ) // CHECK_OP(_EQ, ==, val1, val2)
#define CHECK_NE( val1, val2 ) // CHECK_OP(_NE, !=, val1, val2)
#define CHECK_LE( val1, val2 ) // CHECK_OP(_LE, <=, val1, val2)
#define CHECK_LT( val1, val2 ) // CHECK_OP(_LT, < , val1, val2)
#define CHECK_GE( val1, val2 ) // CHECK_OP(_GE, >=, val1, val2)
#define CHECK_GT( val1, val2 ) // CHECK_OP(_GT, > , val1, val2)
#define DCHECK( val ) assert( val )
#define CHECK( val ) assert( val )

template <typename T, typename U, typename V>
inline T Clamp( T val, U low, V high )
{
	if ( val < low )
		return low;
	else if ( val > high )
		return high;
	else
		return val;
}

inline Float Lerp( Float t, Float v1, Float v2 ) { return ( 1 - t ) * v1 + t * v2; }

template <typename Predicate>
int FindInterval( int size, const Predicate& pred )
{
	int first = 0, len = size;
	while ( len > 0 )
	{
		int half = len >> 1, middle = first + half;
		// Bisect range based on value of _pred_ at _middle_
		if ( pred( middle ) )
		{
			first = middle + 1;
			len -= half + 1;
		}
		else
			len = half;
	}
	return Clamp( first - 1, 0, size - 2 );
}

}; // namespace pbrt
