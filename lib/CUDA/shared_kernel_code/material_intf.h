class MaterialIntf
{
  public:
	__device__ virtual void Setup(
		const float3 D,					   // IN:	incoming ray direction, used for consistent normals
		const float u, const float v,	  //		barycentric coordinates of intersection point
		const float coneWidth,			   //		ray cone width, for texture LOD
		const CoreTri4& tri,			   //		triangle data
		const int instIdx,				   //		instance index, for normal transform
		float3& N, float3& iN, float3& fN, //		geometric normal, interpolated normal, final normal (normal mapped)
		float3& T,						   //		tangent vector
		const float waveLength = -1.0f	 // IN:	wavelength (optional)
		) = 0;

	__device__ virtual void DisableTransmittance() = 0;

	__device__ virtual bool IsEmissive() const = 0;
	__device__ virtual bool IsAlpha() const = 0;
	__device__ virtual float3 Color() const = 0;
	__device__ virtual float Roughness() const = 0;

	__device__ virtual float3 Evaluate( const float3 iN, const float3 T,
										const float3 wo, const float3 wi, float& pdf ) const = 0;
	__device__ virtual float3 Sample( float3 iN, const float3 N, const float3 T,
									  const float3 wo, const float distance,
									  const float r3, const float r4,
									  float3& wi, float& pdf, bool& specular ) const = 0;

	// When not compiling to PTX, nvcc fails to call this operator entirely (at least on Linux)
	// Defining an override (with the same void*) fixes this.
	__device__ static void* operator new( size_t, void* ptr )
	{
		return ptr;
	}

	// TODO: This does not silence the (useless) warning
	// __device__ static void operator delete(  void* ptr )
	// {
	// }
};

#include "material_disney.h"

LH2_DEVFUNC MaterialIntf* GetMaterial( void* inplace, const CoreTri4& tri )
{
	// Copy desc (single integer) to register:
	const CoreMaterialDesc matDesc = materialDescriptions[GET_TRI_MATERIAL( tri.v4 )];

	// Call placement new operator to set up vtables.

	switch ( matDesc.type )
	{
	case MaterialType::DISNEY:
		return new ( inplace ) DisneyMaterial();
		// case MaterialType::CUSTOM_BSDF:
		// 	return new ( inplace ) BSDFMaterial();
	}

	// Unknown material:
	return nullptr;
}

// Helper to store a variable-typed virtual MaterialIntf on stack/in registers:
// TODO: Expand this to a compile-time max over all material implementations
#ifdef _MSC_VER
#define ALIGN( x ) __declspec( align( x ) )
#else
#define ALIGN( x ) __attribute__( ( aligned( x ) ) )
#endif
#define LOCAL_MATERIAL_STORAGE( varname ) \
	char ALIGN( alignof( DisneyMaterial ) ) varname[sizeof( DisneyMaterial )]
