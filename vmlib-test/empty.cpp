#include "../vmlib/mat44.hpp"
#include <cmath>
#include <catch2/catch_amalgamated.hpp>


TEST_CASE("Matrix multiplication", "[matrix][multiplication]") {
   SECTION("Basic matrix multiplication") {
       Mat44f mat1 = {
           1.f, 2.f, 3.f, 4.f,
           5.f, 6.f, 7.f, 8.f,
           9.f, 10.f, 11.f, 12.f,
           13.f, 14.f, 15.f, 16.f
       };
       Mat44f mat2 = {
           16.f, 15.f, 14.f, 13.f,
           12.f, 11.f, 10.f, 9.f,
           8.f, 7.f, 6.f, 5.f,
           4.f, 3.f, 2.f, 1.f
       };


       Mat44f result = mat1 * mat2;


       REQUIRE(result(0, 0) == 80.f);
       REQUIRE(result(3, 3) == 386.f);
   }
}


TEST_CASE("Matrix Vector multiplication", "[matrix][vector]") {
   SECTION("Basic matrix-vector multiplication") {
       Mat44f mat = {
           1.f, 0.f, 0.f, 0.f,
           0.f, 2.f, 0.f, 0.f,
           0.f, 0.f, 3.f, 0.f,
           0.f, 0.f, 0.f, 4.f
       };
       Vec4f vec = {1.f, 2.f, 3.f, 4.f};


       Vec4f result = mat * vec;


       REQUIRE(result.x == 1.f);
       REQUIRE(result.y == 4.f);
       REQUIRE(result.z == 9.f);
       }
}

TEST_CASE("Matrix Rotation", "[matrix][rotation]") {
    Mat44f mat = {
        1.f, 0.f, 0.f, 0.f,
        0.f, 2.f, 0.f, 0.f,
        0.f, 0.f, 3.f, 0.f,
        0.f, 0.f, 0.f, 4.f
    };
    SECTION("rotation x") {
        Mat44f result = make_rotation_x(std::numbers::pi_v<float> / 2.0f) * mat


        REQUIRE(result(0,0) == Catch::Approx(1.f).margin(0.001));
        REQUIRE(result(1,1) == Catch::Approx(0.f).margin(0.001));
        REQUIRE(result(2,2) == Catch::Approx(0.f).margin(0.001));
        REQUIRE(result(3,3) == Catch::Approx(4.f).margin(0.001));
    }
    SECTION("rotation y") {
        Mat44f result = make_rotation_y(std::numbers::pi_v<float> / 2.0f) * mat;


        REQUIRE(result(0,0) == Catch::Approx(0.f).margin(0.001));
        REQUIRE(result(1,1) == Catch::Approx(2.f).margin(0.001));
        REQUIRE(result(2,2) == Catch::Approx(0.f).margin(0.001));
        REQUIRE(result(3,3) == Catch::Approx(4.f).margin(0.001));
    }
    SECTION("rotation z") {
        Mat44f result = make_rotation_z(std::numbers::pi_v<float> / 3.0f) * mat;

        REQUIRE(result(0,0) == Catch::Approx(0.5f).margin(0.001));
        REQUIRE(result(1,1) == Catch::Approx(1.f).margin(0.001));
        REQUIRE(result(2,2) == Catch::Approx(3.f).margin(0.001));
        REQUIRE(result(3,3) == Catch::Approx(4.f).margin(0.001));
   }
}


TEST_CASE("Matrix Tranformations", "[matrix][transformation]") {
    Mat44f mat = {
           1.f, 0.f, 0.f, 0.f,
           0.f, 2.f, 0.f, 0.f,
           0.f, 0.f, 3.f, 0.f,
           1.f, 1.f, 1.f, 1.f
       };

    SECTION("translation matrix") {

        Mat44f result = make_translation({1.f , 2.f , 3.f}) * mat;

        REQUIRE(result(0,1) == 1.f);
        REQUIRE(result(1,2) == 2.f);
        REQUIRE(result(2,3) == 3.f);
        REQUIRE(result(3,3) == 1.f);
    }
    SECTION("perspective projection matrix") {
        float fov = std::numbers::pi_v<float> / 4.0f; 
        float aspect = 16.0f / 9.0f; 
        float near = 0.1f;
        float far = 100.0f;

        Mat44f perspectiveMatrix = make_perspective_projection(fov, aspect, near, far);

        float tanHalfFov = tanf(fov / 2.0f);
        REQUIRE(perspectiveMatrix(0, 0) == Catch::Approx(1.0f / (aspect * tanHalfFov)));
        REQUIRE(perspectiveMatrix(1, 1) == Catch::Approx(1.0f / tanHalfFov));
        REQUIRE(perspectiveMatrix(2, 2) == Catch::Approx(-(far + near) / (far - near)));
        REQUIRE(perspectiveMatrix(2, 3) == Catch::Approx(-(2.0f * far * near) / (far - near)));
        REQUIRE(perspectiveMatrix(3, 2) == Catch::Approx(-1.0f));
        REQUIRE(perspectiveMatrix(3, 3) == Catch::Approx(0.0f));
    }
}
