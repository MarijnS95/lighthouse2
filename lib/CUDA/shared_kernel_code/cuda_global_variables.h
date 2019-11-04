__constant__ CoreMaterialDesc* materialDescriptions;
__constant__ CoreMaterialChunk* coreMaterials;

__host__ void SetMaterialDescList( CoreMaterialDesc* p ) { cudaMemcpyToSymbol( materialDescriptions, &p, sizeof( p ) ); }
__host__ void SetMaterialList( CoreMaterialChunk* p ) { cudaMemcpyToSymbol( coreMaterials, &p, sizeof( p ) ); }

template <typename T>
__device__ const std::decay_t<T>& LoadMaterial( int materialInstance )
{
	// Materials are stored at 4-byte offsets, exactly the size of an integer (address increment):
	return *reinterpret_cast<const std::decay_t<T>*>( coreMaterials + materialInstance );
}
