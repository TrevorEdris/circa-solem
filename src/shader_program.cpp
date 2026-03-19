#include "circa-solem/shader_program.hpp"

#include <fstream>
#include <sstream>
#include <cstdio>
#include <utility>
#include <vector>

namespace cs {

ShaderProgram::~ShaderProgram() {
    if (program_id_) {
        glDeleteProgram(program_id_);
    }
}

ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept
    : program_id_(std::exchange(other.program_id_, 0))
    , vert_path_(std::move(other.vert_path_))
    , frag_path_(std::move(other.frag_path_))
{}

ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) noexcept {
    if (this != &other) {
        if (program_id_) glDeleteProgram(program_id_);
        program_id_ = std::exchange(other.program_id_, 0);
        vert_path_  = std::move(other.vert_path_);
        frag_path_  = std::move(other.frag_path_);
    }
    return *this;
}

bool ShaderProgram::load(std::string_view vert_path, std::string_view frag_path) {
    vert_path_ = std::string(vert_path);
    frag_path_ = std::string(frag_path);
    return reload();
}

bool ShaderProgram::reload() {
    auto vert_src = read_file(vert_path_);
    auto frag_src = read_file(frag_path_);
    if (!vert_src || !frag_src) return false;

    GLuint vert = compile_shader(GL_VERTEX_SHADER,   *vert_src, vert_path_);
    GLuint frag = compile_shader(GL_FRAGMENT_SHADER, *frag_src, frag_path_);
    if (!vert || !frag) {
        if (vert) glDeleteShader(vert);
        if (frag) glDeleteShader(frag);
        return false;
    }

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);
    glLinkProgram(prog);
    glDeleteShader(vert);
    glDeleteShader(frag);

    GLint ok = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint log_len = 0;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &log_len);
        std::vector<char> log(static_cast<size_t>(log_len));
        glGetProgramInfoLog(prog, log_len, nullptr, log.data());
        fprintf(stderr, "[shader] link error: %s\n", log.data());
        glDeleteProgram(prog);
        return false;
    }

    if (program_id_) glDeleteProgram(program_id_);
    program_id_ = prog;
    return true;
}

void ShaderProgram::use() const {
    glUseProgram(program_id_);
}

// static
GLuint ShaderProgram::compile_shader(GLenum type, const std::string& source,
                                      std::string_view filename) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint log_len = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);
        std::vector<char> log(static_cast<size_t>(log_len));
        glGetShaderInfoLog(shader, log_len, nullptr, log.data());
        fprintf(stderr, "[shader] %.*s: %s\n",
                static_cast<int>(filename.size()), filename.data(), log.data());
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

// static
std::optional<std::string> ShaderProgram::read_file(std::string_view path) {
    std::ifstream file{std::string(path)};
    if (!file.is_open()) {
        fprintf(stderr, "[shader] cannot open file: %.*s\n",
                static_cast<int>(path.size()), path.data());
        return std::nullopt;
    }
    std::ostringstream buf;
    buf << file.rdbuf();
    return buf.str();
}

} // namespace cs
