__constant__ CoreMaterialDesc* materialDescriptions;
__constant__ CoreMaterial* disneyMaterials;
__constant__ uint32_t* pbrtMaterials;

__host__ void SetMaterialDescList( CoreMaterialDesc* p ) { cudaMemcpyToSymbol( materialDescriptions, &p, sizeof( p ) ); }
__host__ void SetDisneyMaterialList( CoreMaterial* p ) { cudaMemcpyToSymbol( disneyMaterials, &p, sizeof( p ) ); }
__host__ void SetPbrtMaterialList( uint32_t* p ) { cudaMemcpyToSymbol( pbrtMaterials, &p, sizeof( p ) ); }

template <typename T>
__device__ const std::decay_t<T>& LoadMaterial( int materialInstance )
{
	// Materials are stored at 4-byte offsets, exactly the size of an integer (address increment):
	return *reinterpret_cast<const std::decay_t<T>*>( pbrtMaterials + materialInstance );
}
