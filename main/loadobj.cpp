#include "loadobj.hpp"

#include <rapidobj/rapidobj.hpp>

#include "../support/error.hpp"

SimpleMeshData load_wavefront_obj( char const* aPath )
{
	
	// ask for loading the requested file 
	auto result = rapidobj::ParseFile( aPath );
	if( result.error ) 
	{
		throw Error( "Unable to load OBJ file ’%s’: %s", aPath, result.error.code.message().c_str());
	}

	// triangulate all faces since openGL renders only triangles
	rapidobj::Triangulate( result );

	SimpleMeshData ret;

	for( auto const& shape : result.shapes ) // For every shape i Result object
	{
		for( std::size_t i = 0; i < shape.mesh.indices.size(); ++i ) // For every Index object in indices array
		{
			auto const& idx = shape.mesh.indices[i];

			// Pushing the vector coordinates from the correct index position
			ret.positions.emplace_back( Vec3f{	result.attributes.positions[idx.position_index*3 + 0],
				result.attributes.positions[idx.position_index*3 + 1],
				result.attributes.positions[idx.position_index*3 + 2]
			} );

			auto const& mat = result.materials[shape.mesh.material_ids[i / 3]]; // Extracting the material from the current face index
			
			// Pushing the vector colour
			ret.colors.emplace_back( Vec3f{
				mat.ambient[0],
				mat.ambient[1],
				mat.ambient[2]
			} );

			// Pushing the vector normals from the correct index position
			ret.normals.emplace_back( Vec3f{   result.attributes.normals[idx.normal_index*3 + 0],
				result.attributes.normals[idx.normal_index*3 + 1],
				result.attributes.normals[idx.normal_index*3 + 2]
			} );

			// Pushing the vector texture
			ret.texcoords.emplace_back( Vec2f {
				result.attributes.texcoords[idx.texcoord_index*2 + 0],
				result.attributes.texcoords[idx.texcoord_index*2 + 1]
			});
		}	
	}

	return ret;
}

