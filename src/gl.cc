//
// Created by Raymond on 2/13/25.
//

#include <bindings.hh>

const char* vertexShaderSource =
    "#version 300 es\n"
    "precision highp float;\n"
    "layout (location=0) in vec4 aVertex;\n"
    "layout (location=1) in vec2 aUV;\n"
    "out vec2 uv;\n"
    "void main() {\n"
    "   gl_Position = aVertex;\n"
    "   uv = aUV;\n"
    "}\n";
const char* fragmentShaderSource =
    "#version 300 es\n"
    "precision highp float;\n"
    "in vec2 uv;\n"
    "uniform sampler2D tex;\n"
    "out vec4 fragColor;"
    "void main() {\n"
    "   fragColor = texture(tex, uv);\n"
    "}\n";

GLuint compileShader(GLenum shaderType, const char* shaderSource) {
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderSource, NULL);
    glCompileShader(shader);

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("Error compiling shader: %s\n", infoLog);
    };

    return shader;
}

GLuint createProgram(GLuint vertexShader, GLuint fragmentShader) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        printf("Error linking program: %s\n", infoLog);
    }

    return program;
}

bool initializeGL() {
    auto vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    auto fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    if (!vertexShader || !fragmentShader)
        return false;

    auto program = createProgram(vertexShader, fragmentShader);
    glUseProgram(program); // We can only have one program.

    return true;
};