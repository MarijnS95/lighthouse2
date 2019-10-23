__constant__ CoreMaterialDesc* materialDescriptions;
__constant__ CoreMaterial* disneyMaterials;

__host__ void SetMaterialDescList( CoreMaterialDesc* p ) { cudaMemcpyToSymbol( materialDescriptions, &p, sizeof( p ) ); }
__host__ void SetDisneyMaterialList( CoreMaterial* p ) { cudaMemcpyToSymbol( disneyMaterials, &p, sizeof( p ) ); }
