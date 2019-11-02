class DisneyMaterial : public MaterialIntf
{
	ShadingData shadingData;

  public:
	__device__ void Setup(
		const float3 D,									   // IN:	incoming ray direction, used for consistent normals
		const float u, const float v,					   //		barycentric coordinates of intersection point
		const float coneWidth,							   //		ray cone width, for texture LOD
		const CoreTri4& tri,							   //		triangle data
		const int instIdx,								   //		instance index, for normal transform
		float3& N, float3& iN, float3& fN,				   //		geometric normal, interpolated normal, final normal (normal mapped)
		float3& T,										   //		tangent vector
		const float waveLength = -1.0f,					   // IN:	wavelength (optional)
		const TransportMode mode = TransportMode::Radiance // IN:	Mode based on integrator (optional)
		) override
	{
		GetShadingData( D, u, v, coneWidth, tri, instIdx, shadingData, N, iN, fN, T, waveLength );
	}

	__device__ bool IsEmissive() const override
	{
		return shadingData.IsEmissive();
	}

	__device__ bool IsAlpha() const override
	{
		return shadingData.flags & 1;
	}

	__device__ float3 Color() const override
	{
		return shadingData.color;
	}

	__device__ float3 Evaluate( const float3 iN, const float3 T,
								const float3 wo, const float3 wi,
								float& pdf ) const override
	{
		return EvaluateBSDF( shadingData, iN, T, wo, wi, pdf );
		// pdf = fabs( dot( wi, iN ) ) * INVPI;
		// return shadingData.color * INVPI;
	}

	__device__ float3 Sample( float3 iN, const float3 N, const float3 T,
							  const float3 wo,
							  const float r3, const float r4,
							  float3& wi, float& pdf,
							  BxDFType& sampledType ) const override
	{
		sampledType = ROUGHNESS < 0.01f
						  ? BxDFType::BSDF_SPECULAR
						  : /* TODO: Unknown, only specular is necesary: */
						  BxDFType( 0 );
		return SampleBSDF( shadingData, iN, N, T, wo, r3, r4, wi, pdf );
	}
};
