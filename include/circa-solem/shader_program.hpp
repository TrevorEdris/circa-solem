#pragma once

#include <glad/gl.h>
#include <string>
#include <string_view>

namespace cs {

class ShaderProgram {
public:
    ShaderProgram() = default;
    ~ShaderProgram();

    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram& operator=(const ShaderProgram&) = delete;
    ShaderProgram(ShaderProgram&& other) noexcept;
    ShaderProgram& operator=(ShaderProgram&& other) noexcept;

    // Load and compile from file paths. Prints errors to stderr. Returns false on failure.
    bool load(std::string_view vert_path, std::string_view frag_path);

    // Recompile from the same paths used in the last load() call.
    bool reload();

    void use() const;

    GLuint id() const { return program_id_; }
    bool valid() const { return program_id_ != 0; }

private:
    GLuint program_id_ = 0;
    std::string vert_path_;
    std::string frag_path_;

    static GLuint compile_shader(GLenum type, const std::string& source,
                                  std::string_view filename);
    static std::string read_file(std::string_view path);
};

} // namespace cs
