#pragma once

template <typename CoreMaterialProperties, typename Stack>
class StacklessMaterial : public MaterialIntf
{
	CoreMaterialProperties props;

  public:
	__device__ void Setup(
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
		/* Don't allow overriding this function any further */ final override
	{
		float w;
		SetupFrame(
			// In:
			D, u, v, tri, instIdx, /* TODO: Extract smoothnormal information elsewhere */ true,
			// Out:
			N, iN, fN, T, w );

		props = LoadMaterial<CoreMaterialProperties>( materialInstance );
		// ComputeScatteringFunctions( props, allowMultipleLobes, mode );
	}
	__device__ bool IsEmissive() const override
	{
		return false;
	}
	__device__ bool IsAlpha() const override
	{
		return false;
	}
	/**
	 * Used to retrieve color for emissive surfaces.
	 */
	__device__ float3 Color() const override
	{
		// TODO:
		return make_float3( 1, 0, 1 );
	}

	__device__ float3 Evaluate( const float3 iN, const float3 Tinit,
								const float3 woWorld, const float3 wiWorld,
								const BxDFType flags,
								float& pdf ) const override
	{
		const TBN tbn( Tinit, iN );
		const float3 wo = tbn.WorldToLocal( woWorld ), wi = tbn.WorldToLocal( wiWorld );

		const bool reflect = dot( wiWorld, iN ) * dot( woWorld, iN ) > 0;
		const BxDFType reflectFlag = reflect
										 ? BxDFType::BSDF_REFLECTION
										 : BxDFType::BSDF_TRANSMISSION;

		// pdf = Pdf( wo, wi, flags );
		// NOTE: Instead of calling a separate function, we are already iterating
		// over and matching bxdfs, so might as well do the sum here.
		pdf = 0.f;

		const auto bxdfs = CreateBxDFStack();

		int matches = 0;

		float3 r = make_float3( 0.f );
		bxdfs.Foreach( props, flags, [&]( const auto& bxdf ) {
			if ( bxdf.HasFlags( reflectFlag ) )
				r += bxdf.f( wo, wi );

			matches++;
			pdf += bxdf.Pdf( wo, wi );
		} );

		if ( matches > 0 )
			pdf /= (float)matches;

		return r;
	}
	__device__ float3 Sample( float3 iN, const float3 N, const float3 Tinit,
							  const float3 woWorld, float r3, const float r4,
							  const BxDFType flags,
							  float3& wiWorld, float& pdf,
							  BxDFType& sampledType ) const override
	{
		const auto bxdfs = CreateBxDFStack();

		sampledType = BxDFType( 0 );
		pdf = 0.f;

		const TBN tbn( Tinit, iN );
		const float3 wo = tbn.WorldToLocal( woWorld );

		const int matchingComps = bxdfs.CountMatching( props, flags );

		if ( !matchingComps )
			return make_float3( 0.f );

		// Select a random BxDF (that matches the flags) to sample:
		const int comp = min( (int)floor( r3 * matchingComps ), matchingComps - 1 );

		// Rescale r3:
		r3 = min( r3 * matchingComps - comp, OneMinusEpsilon );

		float3 wi, f;
		bool isSpecular = false;
		bxdfs.RunOn(
			// Run a lambda on element `comp' of the ""stack"".
			props, flags, comp,
			[&]( const auto& bxdf ) {
				sampledType = bxdf.type;
				f = bxdf.Sample_f( wo, wi, r3, r4, pdf, sampledType );

				wiWorld = tbn.LocalToWorld( wi );

				if ( pdf == 0 )
					return;

				// If the selected bxdf is specular (and thus with
				// a specifically chosen direction, wi)
				// this is the only bxdf that is supposed to be sampled.
				isSpecular = bxdf.HasFlags( BxDFType::BSDF_SPECULAR );
			} );

		// TODO: PBRT does this, but isn't it a waste?
		// Might as well set f to zero (if it isn't that already)
		// and continue the evaluation of the other matching bxdfs.
		if ( pdf == 0 )
		{
			sampledType = BxDFType( 0 );
			return make_float3( 0.f );
		}

		if ( isSpecular )
			return f;

		if ( matchingComps > 1 )
		{
			// TODO: Interpolated normal or geometric normal?
			const bool reflect = dot( wiWorld, iN ) * dot( woWorld, iN ) > 0;
			const BxDFType reflectFlag = reflect
											 ? BxDFType::BSDF_REFLECTION
											 : BxDFType::BSDF_TRANSMISSION;

			int i = 0;

			bxdfs.Foreach( props, flags, [&]( const auto& bxdf2 ) {
				// Ignore the current bxdf, it has already been sampled
				// and the pdf already taken:
				if ( i++ == comp )
					return;

				// Compute overall PDF with all matching _BxDF_s
				pdf += bxdf2.Pdf( wo, wi );

				// Compute value of BSDF for sampled direction

				// PBRT Resets f to zero and evaluates all bxdfs again.
				// We however keep the evaluation of `bxdf`, just like
				// PBRT does in the pdf sum calculation.
				if ( bxdf2.HasFlags( reflectFlag ) )
					f += bxdf2.f( wo, wi );
			} );

			pdf /= matchingComps;
		}

		return f;
	}

  protected:
	__device__ virtual Stack CreateBxDFStack() const = 0;
};

template <typename Derived>
class GraphNode
{
	__device__ Derived& derived()
	{
		return *static_cast<Derived*>( this );
	}

	__device__ const Derived& derived() const
	{
		return *static_cast<const Derived*>( this );
	}

  public:
	template <typename Params, typename Fn>
	__device__ void RunOn( const Params& params, const BxDFType type, const int idx, Fn&& func ) const
	{
		int cnt = idx;
		derived().RunOnImpl( params, type, cnt, func );
	}
};

template <typename Func, typename Nested>
struct GuardNodeImpl : public GraphNode<GuardNodeImpl<Func, Nested>>
{
	Func enabled;
	Nested nested;

	__device__ GuardNodeImpl( Func enabled, Nested nested ) : enabled( enabled ), nested( nested ) {}

	template <typename Params>
	__device__ constexpr int CountMatching( const Params& params, const BxDFType type ) const
	{
		if ( enabled( params ) )
			return nested.CountMatching( params, type );
		return 0;
	}

	template <typename Params, typename Fn>
	__device__ void Foreach( const Params& params, const BxDFType type, Fn&& func ) const
	{
		if ( enabled( params ) )
			return nested.Foreach( params, type, func );
	}

	template <typename Params, typename Fn>
	__device__ void RunOnImpl( const Params& params, const BxDFType type, int& idx, Fn&& func ) const
	{
		if ( enabled( params ) )
			return nested.RunOnImpl( params, type, idx, func );
		idx -= CountMatching( params, type );
		assert( idx >= 0 );
	}
};

template <typename Func, typename Nested>
__device__ GuardNodeImpl<Func, Nested> GuardNode( Func enabled, Nested nested )
{
	return {enabled, nested};
}

template <typename Params, typename Func>
struct BxDFNodeImpl : public GraphNode<BxDFNodeImpl<Params, Func>>
{
	Func create;

	using BxDF = decltype( create( std::declval<Params>() ) );

	__device__ BxDFNodeImpl( Func create ) : create( create ) {}

	__device__ constexpr bool Matches( const Params& params, const BxDFType type ) const
	{
		// BxDF must be a BxDF_T (or otherwise expose the type flag)
		return ( BxDF::type & type ) == BxDF::type;
	}

	__device__ constexpr int CountMatching( const Params& params, const BxDFType type ) const
	{
		return Matches( params, type ) & 1;
	}

	template <typename Fn>
	__device__ void Foreach( const Params& params, const BxDFType type, Fn&& func ) const
	{
		if ( Matches( params, type ) )
			func( create( params ) );
	}

	template <typename Fn>
	__device__ void RunOnImpl( const Params& params, const BxDFType type, int& idx, Fn&& func ) const
	{
		if ( Matches( params, type ) && idx-- == 0 )
			func( create( params ) );
	}
};

template <typename Params, typename Func>
__device__ BxDFNodeImpl<Params, Func> BxDFNode( Func create )
{
	return {create};
}
