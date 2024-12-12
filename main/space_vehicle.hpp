#ifndef VEHICLE_HPP_E4D1E8EC_6CDA_4800_ABDD_264F643AF5DB
#define VEHICLE_HPP_E4D1E8EC_6CDA_4800_ABDD_264F643AF5DB

#include <vector>

#include <cstdlib>

#include "simple_mesh.hpp"

#include "../vmlib/vec3.hpp"
#include "../vmlib/mat44.hpp"

SimpleMeshData make_cylinder(
    bool aCapped,
    std::size_t aSubdivs,
    Vec3f aColor,
    Mat44f aPreTransform = kIdentity44f
);

SimpleMeshData make_cone(
	bool aCapped,
	std::size_t aSubdivs,
	Vec3f aColor,
	Mat44f aPreTransform = kIdentity44f
);


SimpleMeshData make_cube(
    Vec3f aColor, 
    Mat44f aPreTransform 
);

#endif // VEHICLE_HPP_E4D1E8EC_6CDA_4800_ABDD_264F643AF5DB
