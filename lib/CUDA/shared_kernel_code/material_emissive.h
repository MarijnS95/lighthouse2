

class Emissive : public MaterialIntf
{
	float3 color;

  public:
	__device__ virtual void Setup(
		const float3 D,									   // IN:	incoming ray direction, used for consistent normals
		const float u, const float v,					   //		barycentric coordinates of intersection point
		const float coneWidth,							   //		ray cone width, for texture LOD
		const CoreTri4& tri,							   //		triangle data
		const int instIdx,								   //		instance index, for normal transform
		const int materialInstance,						   //		Material instance id/location
		float3& N, float3& iN, float3& fN,				   //		geometric normal, interpolated normal, final normal (normal mapped)
		float3& T,										   //		tangent vector
		const float waveLength = -1.0f,					   // IN:	wavelength (optional)
		const bool allowMultipleLobes = true,			   // IN:	Integrator samples multiple lobes (optional)
		const TransportMode mode = TransportMode::Radiance // IN:	Mode based on integrator (optional)
	)
	{
		float w;
		SetupFrame(
			// In:
			D, u, v, tri, instIdx, /* TODO: Extract smoothnormal information elsewhere */ false,
			// Out:
			N, iN, fN, T, w );

		color = LoadMaterial<float3>( materialInstance );
	}
	__device__ virtual bool IsEmissive() const
	{
		return true;
	}
	__device__ virtual bool IsAlpha() const
	{
		return false;
	}
	/**
	 * Used to retrieve color for emissive surfaces.
	 */
	__device__ virtual float3 Color() const
	{
		return color;
	}

	__device__ virtual float3 Evaluate( const float3 iN, const float3 T,
										const float3 woWorld, const float3 wiWorld,
										const BxDFType flags,
										float& pdf ) const
	{
		pdf = 0.f;
		return make_float3( 0.f );
	}
	__device__ virtual float3 Sample( float3 iN, const float3 N, const float3 T,
									  const float3 woWorld, const float r3, const float r4,
									  const BxDFType flags,
									  float3& wiWorld, float& pdf,
									  BxDFType& sampledType ) const
	{
		pdf = 0.f;
		sampledType = BxDFType( 0 );
		return make_float3( 0.f );
	}
};
