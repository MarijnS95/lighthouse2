/**
 *  dynamic_host_material.h - Copyright 2019 Utrecht University
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Virtual material to be implemented by actual materials.
 */

//  +-----------------------------------------------------------------------------+
//  |  DynamicHostMaterial                                                        |
//  |  Host-side dynamic material definition.                               LH2'19|
//  |  Virtual interface holding a material.                                      |
//  +-----------------------------------------------------------------------------+
class DynamicHostMaterial
{
  protected:
	DynamicHostMaterial( MaterialType type );

  public:
	const MaterialType type;

	virtual uint32_t Flatten( Flattener<sizeof( uint32_t )>& flattener, const CoreTexDesc* texDescs ) const = 0;

	virtual bool IsEmissive() const = 0;
	// Retrieve color for material. Mainly used for emissive materials
	virtual float3 Color() const = 0;

	// TODO: API
  public:
	virtual bool Changed() = 0;
	virtual void MarkAsDirty() = 0;
};

template <typename Derived, MaterialType _type>
class DynamicHostMaterial_T : public DynamicHostMaterial
{
  protected:
	DynamicHostMaterial_T() : DynamicHostMaterial( _type ) {}

  public:
	static constexpr MaterialType type = _type;
};

/**
 * Simple holder for a material already stored in GPU format
 */
template <typename Material>
class SimpleHostMaterial : public DynamicHostMaterial_T<SimpleHostMaterial<Material>, Material::type>
{
	Material material;

  public:
	SimpleHostMaterial( const Material& material ) : material( material )
	{
	}

	uint32_t Flatten( Flattener<sizeof( uint32_t )>& flattener, const CoreTexDesc* texDescs ) const override
	{
		return flattener.push_back( material ).offset();
	}

	bool IsEmissive() const override
	{
		// Assuming no SimpleHostMaterial is emissive
		return false;
	}

	float3 Color() const override
	{
		// No way to extract this properly. Function is really only used
		// for emissive materials, which are likely not implemented through
		// SimpleHostMaterial.
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

// template <typename Material>
// class SimpleHostMaterial<Material, Material::type> {
// };

class EmissiveMaterial : public DynamicHostMaterial_T<EmissiveMaterial, MaterialType::EMISSIVE>
{
	CoreEmissiveMaterial light;

  public:
	EmissiveMaterial( const float3& color, bool twosided ) : light{color, twosided}
	{
	}

	uint32_t Flatten( Flattener<sizeof( uint32_t )>& flattener, const CoreTexDesc* texDescs ) const override
	{
		// TODO: return -1, assume host_light picks this up
		return flattener.push_back( light ).offset();
	}

	bool IsEmissive() const override
	{
		// Assuming no material is emissive
		return true;
	}

	float3 Color() const override
	{
		return light.color;
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
