#include "loadOBJ.hpp"
#include "rapidobj/rapidobj.hpp"
#include <iostream>
#include <glad/glad.h>
#include <vector>

namespace obj_loader {

    bool load_and_display_obj(const std::string& path) {
        auto result = rapidobj::ParseFile(path);
        if (!result) {
            std::cerr << "Failed to parse '" << path << "' : " << result.error().message << '\n';
            return false;
        }

        std::cout << "Parsed '" << path << "' successfully!\n";

        // Assuming you have a shader program already set up and bound
        unsigned int vbo, vao;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        // Upload vertex data to the GPU
        glBufferData(GL_ARRAY_BUFFER, result.attrib.vertices.size() * sizeof(float), result.attrib.vertices.data(), GL_STATIC_DRAW);

        // Assuming vertices are layout(location = 0) in the vertex shader
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Unbind for now
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        // In your render loop, you'll need to bind the VAO like so:
        // glBindVertexArray(vao);
        // glDrawArrays(GL_TRIANGLES, 0, result.attrib.vertices.size() / 3);

        return true;
    }
}
