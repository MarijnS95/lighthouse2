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
//  |  Virtual interface holding a material.                                |
//  +-----------------------------------------------------------------------------+
class DynamicHostMaterial
{
  protected:
	DynamicHostMaterial( MaterialType type );

  public:
	const MaterialType type;

	virtual uint32_t Flatten( Flattener<sizeof( uint32_t )>& flattener ) const = 0;
	// Blegh.
	virtual void CollectMaps( CoreMaterialEx& gpuMatEx ) const = 0;

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

	uint32_t Flatten( Flattener<sizeof( uint32_t )>& flattener ) const override
	{
		return flattener.push_back( material ).offset();
	}

	void CollectMaps( CoreMaterialEx& gpuMatEx ) const override
	{
		// Nothing. This function should be refactored out.
	}

	// TODO:
	bool Changed()
	{
		return false;
	}
	void MarkAsDirty()
	{
	}
};

// template <typename Material>
// class SimpleHostMaterial<Material, Material::type> {
// };
