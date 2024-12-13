#include "space_vehicle.hpp"
#include "cube.hpp"

#include <numbers>
#include <iostream>

SimpleMeshData make_cylinder( bool aCapped, std::size_t aSubdivs, Vec3f aColor, Mat44f aPreTransform )
{
    SimpleMeshData meshData;
	std::vector<Vec3f> pos, normals;

    float prevY = std::cos(0.f);
    float prevZ = std::sin(0.f);
    
    // Generate the cylindrical shell
    for (std::size_t i = 0; i < aSubdivs; ++i) 
    {
        // std::fprintf( stderr, "shell %d\n", i );
        float angle = (i + 1) / static_cast<float>(aSubdivs) * 2.f * std::numbers::pi_v<float>;
        float y = std::cos( angle );
        float z = std::sin( angle );

        pos.emplace_back(Vec3f{0.f, prevY, prevZ});
        pos.emplace_back(Vec3f{0.f, y, z});
        pos.emplace_back(Vec3f{1.f, prevY, prevZ});
        
        normals.emplace_back(Vec3f{0.f, prevY, prevZ});
        normals.emplace_back(Vec3f{0.f, y, z});
        normals.emplace_back(Vec3f{1.f, prevY, prevZ});

        pos.emplace_back(Vec3f{0.f, y, z});
        pos.emplace_back(Vec3f{1.f, y, z});
        pos.emplace_back(Vec3f{1.f, prevY, prevZ});

        normals.emplace_back(Vec3f{0.f, y, z});
        normals.emplace_back(Vec3f{1.f, y, z});
        normals.emplace_back(Vec3f{1.f, prevY, prevZ});

        // Vec3f normal = normalize(Vec3f{0.f, y, z});
        // normals.insert(normals.end(), 6, normal);

        prevY = y;
        prevZ = z;
    }

    // cap the cylinder
    if (aCapped) {
        // Cap at x = 0
        for (std::size_t i = 0; i < aSubdivs; ++i) 
        {
            float angle = (i + 1) / static_cast<float>(aSubdivs) * 2.f * std::numbers::pi_v<float>;
            float y = std::cos( angle );
            float z = std::sin( angle );

            pos.emplace_back(Vec3f{0.f, 0.f, 0.f});
            pos.emplace_back(Vec3f{0.f, y, z});
            pos.emplace_back(Vec3f{0.f, prevY, prevZ});

            Vec3f normal = normalize(Vec3f{0.f, 0.f, -1.f});
            normals.insert(normals.end(), 3, normal);

            prevY = y;
            prevZ = z;
        }

        prevY = std::cos(0.f);
        prevZ =  std::sin(0.f);

        // Cap at x = 1
        for (std::size_t i = 0; i < aSubdivs; ++i) 
        {
            float angle = (i + 1) / static_cast<float>(aSubdivs) * 2.f * std::numbers::pi_v<float>;
            float y = std::cos( angle );
            float z = std::sin( angle );

            pos.emplace_back(Vec3f{1.f, 0.f, 0.f});
            pos.emplace_back(Vec3f{1.f, prevY, prevZ});
            pos.emplace_back(Vec3f{1.f, y, z});

            Vec3f normal = normalize(Vec3f{0.f, 0.f, 1.f});
            normals.insert(normals.end(), 3, normal);

            prevY = y;
            prevZ = z;
        }
    }

    for (size_t i = 0; i < pos.size(); ++i)
    {
        Vec4f tp = aPreTransform * Vec4f{ pos[i].x, pos[i].y, pos[i].z, 1.f };
        Vec4f tn = aPreTransform * Vec4f{ normals[i].x, normals[i].y, normals[i].z, 1.f };
        tp /= tp.w;
        tn /= tn.w;
        pos[i] = { tp.x, tp.y, tp.z };
        normals[i] = normalize(Vec3f{ tn.x, tn.y, tn.z });
    }

    std::vector col( pos.size(), aColor ); // Apply a color to each vertex
    meshData.positions = std::move(pos);
    meshData.colors = std::move(col);
    meshData.normals = std::move(normals);

    return meshData;
}

SimpleMeshData make_cone( bool aCapped, std::size_t aSubdivs, Vec3f aColor, Mat44f aPreTransform )
{
    SimpleMeshData meshData;
	std::vector<Vec3f> pos, normals;

    float prevY = 1.0f; 
    float prevZ = 0.0f; 

    // cone shell vertices
    for (std::size_t i = 0; i < aSubdivs; ++i) 
    {
        float angle = (i + 1) * 2 * std::numbers::pi_v<float> / aSubdivs;
        float y = std::cos( angle );
        float z = std::sin( angle );

        pos.emplace_back(Vec3f{0.f, prevY, prevZ});
        pos.emplace_back(Vec3f{0.f, y, z});
        pos.emplace_back(Vec3f{1.f, 0.f, 0.f});

        normals.emplace_back(Vec3f{0.f, y - prevY, z - prevZ});
        normals.emplace_back(Vec3f{1.f, - prevY, - prevZ});
        normals.emplace_back( normalize(cross(Vec3f{0.f, y - prevY, z - prevZ}, Vec3f{1.f, - prevY, - prevZ})));

        prevY = y;
        prevZ = z;
    }

    for (size_t i = 0; i < pos.size(); ++i)
    {
        Vec4f tp = aPreTransform * Vec4f{ pos[i].x, pos[i].y, pos[i].z, 1.f };
        Vec4f tn = aPreTransform * Vec4f{ normals[i].x, normals[i].y, normals[i].z, 1.f };
        tp /= tp.w;
        tn /= tn.w;
        pos[i] = { tp.x, tp.y, tp.z };
        normals[i] = normalize(Vec3f{ tn.x, tn.y, tn.z });
    }
    std::vector col( pos.size(), aColor ); // Apply a color to each vertex

    meshData.positions = std::move(pos);
    meshData.colors = std::move(col);
    meshData.normals = std::move(normals);


    return meshData;
}

SimpleMeshData make_cube( Vec3f aColor, Mat44f aPreTransform )
{
    SimpleMeshData meshData;

    for (int i = 0; i < sizeof(kCubePositions) / sizeof(float) / 3; i++) 
    {
        Vec4f tp = aPreTransform * Vec4f{ kCubePositions[3*i], kCubePositions[3*i+1], kCubePositions[3*i+2], 1.f };
        Vec4f tn = aPreTransform * Vec4f{ kCubeNormals[3*i], kCubeNormals[3*i+1], kCubeNormals[3*i+2], 1.f };
        tp /= tp.w;
        tn /= tn.w;
        Vec3f p = Vec3f{ tp.x, tp.y, tp.z };
        Vec3f n = normalize(Vec3f{ tn.x, tn.y, tn.z });

        meshData.positions.emplace_back(p);
        meshData.colors.emplace_back(aColor);
        meshData.normals.emplace_back(n);
    }

    return meshData;
}