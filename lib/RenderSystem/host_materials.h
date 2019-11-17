#pragma once

namespace materials
{
namespace pbrt
{

using namespace common::materials::pbrt;

/**
 * Simple holder for a material already stored in GPU format
 */
template <typename Material>
class TexturedMaterial : public DynamicHostMaterial_T<TexturedMaterial<Material>, Material::type>
{
  public:
	bool IsEmissive() const override
	{
		// Assuming no TexturedMaterial is emissive
		return false;
	}

	float3 Color() const override
	{
		// No way to extract this properly. Function is really only used
		// for emissive materials, which are likely not implemented through
		// TexturedMaterial.
		return make_float3( 0.f );
	}

	bool isDirty = true;
	bool Changed()
	{
		bool wasDirty = isDirty;
		// Assuming no-one changes the material, for now.
		isDirty = false;
		return wasDirty;
	}
	void MarkAsDirty()
	{
		isDirty = true;
	}
};

struct DisneyHostMaterial : public TexturedMaterial<Disney>
{
	DynamicHostTexture<float3>* color;
	DynamicHostTexture<float>* flatness;
	DynamicHostTexture<float3>* scatterDistance;
	DynamicHostTexture<float>* specTrans;
	DynamicHostTexture<float>*metallic, *eta;
	DynamicHostTexture<float>*roughness, *specularTint, *anisotropic;
	DynamicHostTexture<float>*sheenTint, *sheen;
	DynamicHostTexture<float>*clearcoat, *clearcoatGloss;
	DynamicHostTexture<float>* diffTrans;
	// float bumpMap;
	bool thin;

	uint32_t Flatten( Flattener<sizeof( uint32_t )>& flattener, const CoreTexDesc* texDescs ) const override
	{
		Disney material;

		material.color = color->GetCoreTexture( texDescs );
		material.flatness = flatness->GetCoreTexture( texDescs );
		material.scatterDistance = scatterDistance->GetCoreTexture( texDescs );
		material.specTrans = specTrans->GetCoreTexture( texDescs );
		material.metallic = metallic->GetCoreTexture( texDescs );
		material.eta = eta->GetCoreTexture( texDescs );
		material.roughness = roughness->GetCoreTexture( texDescs );
		material.specularTint = specularTint->GetCoreTexture( texDescs );
		material.anisotropic = anisotropic->GetCoreTexture( texDescs );
		material.sheenTint = sheenTint->GetCoreTexture( texDescs );
		material.sheen = sheen->GetCoreTexture( texDescs );
		material.clearcoat = clearcoat->GetCoreTexture( texDescs );
		material.clearcoatGloss = clearcoatGloss->GetCoreTexture( texDescs );
		material.diffTrans = diffTrans->GetCoreTexture( texDescs );

		material.thin = thin;

		return flattener.push_back( material ).offset();
	}
};

struct GlassHostMaterial : public TexturedMaterial<Glass>
{
	DynamicHostTexture<float3>* R;
	DynamicHostTexture<float>* urough;
	DynamicHostTexture<float3>* T;
	DynamicHostTexture<float>* vrough;

	DynamicHostTexture<float>* eta;

	bool remapRoughness;

	uint32_t Flatten( Flattener<sizeof( uint32_t )>& flattener, const CoreTexDesc* texDescs ) const override
	{
		Glass material;

		material.R = R->GetCoreTexture( texDescs );
		material.urough = urough->GetCoreTexture( texDescs );
		material.T = T->GetCoreTexture( texDescs );
		material.vrough = vrough->GetCoreTexture( texDescs );
		material.eta = eta->GetCoreTexture( texDescs );

		material.remapRoughness = remapRoughness;

		return flattener.push_back( material ).offset();
	}
};

struct MatteHostMaterial : public TexturedMaterial<Matte>
{
	DynamicHostTexture<float3>* Kd;
	DynamicHostTexture<float>* sigma;

	uint32_t Flatten( Flattener<sizeof( uint32_t )>& flattener, const CoreTexDesc* texDescs ) const override
	{
		Matte material;

		material.Kd = Kd->GetCoreTexture( texDescs );
		material.sigma = sigma->GetCoreTexture( texDescs );

		return flattener.push_back( material ).offset();
	}
};

struct MetalHostMaterial : public TexturedMaterial<Metal>
{
	DynamicHostTexture<float3>* eta;
	DynamicHostTexture<float>* urough;
	DynamicHostTexture<float3>* k;
	DynamicHostTexture<float>* vrough;
	bool remapRoughness;

	// float roughness;
	// float bumpmap;

	uint32_t Flatten( Flattener<sizeof( uint32_t )>& flattener, const CoreTexDesc* texDescs ) const override
	{
		Metal material;

		material.eta = eta->GetCoreTexture( texDescs );
		material.urough = urough->GetCoreTexture( texDescs );
		material.k = k->GetCoreTexture( texDescs );
		material.vrough = vrough->GetCoreTexture( texDescs );

		material.remapRoughness = remapRoughness;

		return flattener.push_back( material ).offset();
	}
};

struct MirrorHostMaterial : public TexturedMaterial<Mirror>
{
	DynamicHostTexture<float3>* Kr;

	uint32_t Flatten( Flattener<sizeof( uint32_t )>& flattener, const CoreTexDesc* texDescs ) const override
	{
		Mirror material;

		material.Kr = Kr->GetCoreTexture( texDescs );

		return flattener.push_back( material ).offset();
	}
};

struct PlasticHostMaterial : public TexturedMaterial<Plastic>
{
	DynamicHostTexture<float3>* Kd;
	DynamicHostTexture<float>* roughness;
	DynamicHostTexture<float3>* Ks;

	bool remapRoughness;

	uint32_t Flatten( Flattener<sizeof( uint32_t )>& flattener, const CoreTexDesc* texDescs ) const override
	{
		Plastic material;

		material.Kd = Kd->GetCoreTexture( texDescs );
		material.roughness = roughness->GetCoreTexture( texDescs );
		material.Ks = Ks->GetCoreTexture( texDescs );

		material.remapRoughness = remapRoughness;

		return flattener.push_back( material ).offset();
	}
};

struct SubstrateHostMaterial : public TexturedMaterial<Substrate>
{
	DynamicHostTexture<float3>* Kd;
	DynamicHostTexture<float>* urough;
	DynamicHostTexture<float3>* Ks;
	DynamicHostTexture<float>* vrough;

	bool remapRoughness;

	uint32_t Flatten( Flattener<sizeof( uint32_t )>& flattener, const CoreTexDesc* texDescs ) const override
	{
		Substrate material;

		material.Kd = Kd->GetCoreTexture( texDescs );
		material.urough = urough->GetCoreTexture( texDescs );
		material.Ks = Ks->GetCoreTexture( texDescs );
		material.vrough = vrough->GetCoreTexture( texDescs );

		material.remapRoughness = remapRoughness;

		return flattener.push_back( material ).offset();
	}
};

}; // namespace pbrt
}; // namespace materials
