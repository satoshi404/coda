// shader.c
#include "shader.h"

#include <glad/glad.h>
#include <core/debug.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __namespace(func_name) renderer_Shader##func_name

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static u32 CompileShader(const char* source, u32 type)
{
    u32 shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    i32 success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("ERROR::SHADER::%s::COMPILATION_FAILED\n%s\n",
            (type == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT", infoLog);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static u32 CreateProgram(u32 vertex, u32 fragment)
{
    u32 program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

    i32 success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) 
    {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
        glDeleteProgram(program);
        return 0;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);
    return program;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static i32 GetUniformLocation( u32 program, const char* name )
{
    i32 loc = glGetUniformLocation(program, name);
    //if ( loc == -1 )
    //{
    //    LOG_WARN( "Warning: uniform '%s' not found in shader\n", name );
    //}
    return loc;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Shader* __namespace(Create)(const char* name, const char* vertexSrc, const char* fragmentSrc)
{
    u32 vertex = CompileShader(vertexSrc, GL_VERTEX_SHADER);
    if (!vertex) return NULL;

    u32 fragment = CompileShader(fragmentSrc, GL_FRAGMENT_SHADER);
    if (!fragment) {
        glDeleteShader(vertex);
        return NULL;
    }

    u32 program = CreateProgram(vertex, fragment);
    if (!program) return NULL;

    Shader* shader = (Shader*)malloc(sizeof(Shader));
    shader->programID = program;
    shader->name = name;

    // Cache common uniforms
    shader->uniforms.mvp          = GetUniformLocation(program, "uMVP");
    shader->uniforms.model        = GetUniformLocation(program, "uModel");
    shader->uniforms.view         = GetUniformLocation(program, "uView");
    shader->uniforms.projection   = GetUniformLocation(program, "uProjection");
    shader->uniforms.normalMatrix = GetUniformLocation(program, "uNormalMatrix");
    shader->uniforms.color        = GetUniformLocation(program, "uColor");
    shader->uniforms.texture0     = GetUniformLocation(program, "uTexture0");
    shader->uniforms.cameraPos    = GetUniformLocation(program, "uCameraPos");

    return shader;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Shader* __namespace(LoadFromFile)(const char* name, const char* vertPath, const char* fragPath)
{
    FILE* f;
    char* buffer = NULL;
    long length;

    // Load vertex
    f = fopen(vertPath, "rb");
    if (!f) { printf("Failed to open vertex shader: %s\n", vertPath); return NULL; }
    fseek(f, 0, SEEK_END);
    length = ftell(f);
    fseek(f, 0, SEEK_SET);
    buffer = (char*)malloc(length + 1);
    fread(buffer, 1, length, f);
    buffer[length] = '\0';
    fclose(f);
    const char* vertexSrc = buffer;

    // Load fragment
    f = fopen(fragPath, "rb");
    if (!f) { printf("Failed to open fragment shader: %s\n", fragPath); free(buffer); return NULL; }
    fseek(f, 0, SEEK_END);
    length = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* fragBuffer = (char*)malloc(length + 1);
    fread(fragBuffer, 1, length, f);
    fragBuffer[length] = '\0';
    fclose(f);
    const char* fragmentSrc = fragBuffer;

    Shader* shader = __namespace(Create)(name, vertexSrc, fragmentSrc);

    free(buffer);
    free(fragBuffer);
    return shader;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __namespace(Bind)(Shader* shader)
{
    if (shader && shader->programID)
        glUseProgram(shader->programID);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __namespace(Ubind)(void)
{
    glUseProgram(0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __namespace(SetInt)(Shader* shader, const char* name, i32 value)
{
    if (shader && shader->programID)
        glUniform1i(GetUniformLocation(shader->programID, name), value);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __namespace(SetFloat)(Shader* shader, const char* name, f32 value)
{
    if (shader && shader->programID)
        glUniform1f(GetUniformLocation(shader->programID, name), value);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __namespace(SetVec2)(Shader* shader, const char* name, Vec2 value)
{
    if (shader && shader->programID)
        glUniform2f(GetUniformLocation(shader->programID, name), value.x, value.y);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __namespace(SetVec3)(Shader* shader, const char* name, Vec3 value)
{
    if (shader && shader->programID)
        glUniform3f(GetUniformLocation(shader->programID, name), value.x, value.y, value.z);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __namespace(SetVec4)(Shader* shader, const char* name, Vec4 value)
{
    if (shader && shader->programID)
        glUniform4f(GetUniformLocation(shader->programID, name), value.x, value.y, value.z, value.w);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __namespace(SetMat4)(Shader* shader, const char* name, const Mat4* value)
{
    if (shader && shader->programID)
        glUniformMatrix4fv(GetUniformLocation(shader->programID, name), 1, GL_FALSE, value->m);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __namespace(SetColor)(Shader* shader, const char* name, Vec3 value)
{
    __namespace(SetVec3)(shader, name, value);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __namespace(SetMvp)(Shader* shader, const Mat4* model, const Mat4* view, const Mat4* proj)
{
    if (!shader || !shader->programID) return;

    Mat4 viewProj = core_MathMat4Multiply(*proj, *view);
    Mat4 mvp = core_MathMat4Multiply(viewProj, *model);

    if (shader->uniforms.mvp != -1)
        glUniformMatrix4fv(shader->uniforms.mvp, 1, GL_FALSE, mvp.m);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __namespace(SetMatrices)(Shader* shader, const Mat4* model, const Mat4* view, const Mat4* proj)
{
    if (!shader || !shader->programID) return;

    if (shader->uniforms.model != -1)
        glUniformMatrix4fv(shader->uniforms.model, 1, GL_FALSE, model->m);
    if (shader->uniforms.view != -1)
        glUniformMatrix4fv(shader->uniforms.view, 1, GL_FALSE, view->m);
    if (shader->uniforms.projection != -1)
        glUniformMatrix4fv(shader->uniforms.projection, 1, GL_FALSE, proj->m);

    // Normal matrix (inverse transpose of model)
    if (shader->uniforms.normalMatrix != -1) {
        // Simplified: assume model is rigid (no non-uniform scale)
        Mat4 normal = *model;
        normal.m[12] = normal.m[13] = normal.m[14] = 0;
        // In real engine, compute inverse transpose
        glUniformMatrix4fv(shader->uniforms.normalMatrix, 1, GL_FALSE, normal.m);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __namespace(Destroy)(Shader* shader)
{
    if (shader) {
        if (shader->programID)
            glDeleteProgram(shader->programID);
        free(shader);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct ShaderNode {
    Shader* shader;
    struct ShaderNode* next;
} ShaderNode;

struct ShaderLibrary {
    ShaderNode* head;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ShaderLibrary* __namespace(LibraryCreate)(void)
{
    ShaderLibrary* lib = (ShaderLibrary*)malloc(sizeof(ShaderLibrary));
    lib->head = NULL;
    return lib;
}

void __namespace(LibraryAdd)(ShaderLibrary* lib, Shader* shader)
{
    if (!lib || !shader) return;

    ShaderNode* node = (ShaderNode*)malloc(sizeof(ShaderNode));
    node->shader = shader;
    node->next = lib->head;
    lib->head = node;
}

Shader* __namespace(LibraryGet)(ShaderLibrary* lib, const char* name)
{
    if (!lib) return NULL;
    ShaderNode* current = lib->head;
    while (current) {
        if (strcmp(current->shader->name, name) == 0)
            return current->shader;
        current = current->next;
    }
    return NULL;
}

void __namespace(LibraryDestroy)(ShaderLibrary* lib)
{
    if (!lib) return;
    ShaderNode* current = lib->head;
    while (current) {
        ShaderNode* next = current->next;
        __namespace(Destroy)(current->shader);
        free(current);
        current = next;
    }
    free(lib);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char* basic_vert = 
    "#version 330 core\n"
    "layout(location = 0) in vec3 aPosition;\n"
    "layout(location = 1) in vec3 aNormal;\n"
    "layout(location = 2) in vec3 aColor;\n"
    "uniform mat4 uMVP;\n"
    "uniform mat4 uModel;\n"
    "out vec3 vColor;\n"
    "out vec3 vNormal;\n"
    "out vec3 vFragPos;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = uMVP * vec4(aPosition, 1.0);\n"
    "    vColor = aColor;\n"
    "    vNormal = mat3(uModel) * aNormal;\n"
    "    vFragPos = vec3(uModel * vec4(aPosition, 1.0));\n"
    "}\n";

static const char* basic_frag = 
    "#version 330 core\n"
    "in vec3 vColor;\n"
    "in vec3 vNormal;\n"
    "in vec3 vFragPos;\n"
    "out vec4 FragColor;\n"
    "uniform vec3 uLightDir = vec3(-1.0, -1.0, -1.0);\n"
    "void main()\n"
    "{\n"
    "    vec3 normal = normalize(vNormal);\n"
    "    vec3 lightDir = normalize(-uLightDir);\n"
    "    float diff = max(dot(normal, lightDir), 0.0);\n"
    "    float ambient = 0.3;\n"
    "    vec3 lighting = (ambient + diff * 0.7) * vColor;\n"
    "    FragColor = vec4(lighting, 1.0);\n"
    "}\n";

void __namespace(LibraryLoadDefaults)(ShaderLibrary* lib)
{
    Shader* defaultShader = __namespace(Create)("basic", basic_vert , basic_frag);
    if (defaultShader)
        __namespace(LibraryAdd)(lib, defaultShader);
}

#undef __namespace
