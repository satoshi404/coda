#ifndef __shader_h__
#define __shader_h__

#include <core/types.h>
#include <core/math.h>

#define __namespace( func_name ) renderer##_##Shader##func_name

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
    u32         programID;
    const char* name;
    
    struct 
    {
        i32 mvp;
        i32 model;
        i32 view;
        i32 projection;
        i32 normalMatrix;
        i32 color;
        i32 texture0;
        i32 cameraPos;
    } uniforms;
    
} Shader;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Shader* __namespace( Create )        ( const char* name, const char* vertexSrc, const char* fragmentSrc );
Shader* __namespace( LoadFromFile )  ( const char* name, const char* vertPath, const char* fragPath );

void __namespace( Bind  )            ( Shader* shader );
void __namespace( Ubind )            ( void );

void __namespace( SetInt   )         ( Shader* shader, const char* name, i32 value );
void __namespace( SetFloat )         ( Shader* shader, const char* name, f32 value );
void __namespace( SetVec2  )         ( Shader* shader, const char* name, Vec2 value );
void __namespace( SetVec3  )         ( Shader* shader, const char* name, Vec3 value );
void __namespace( SetVec4  )         ( Shader* shader, const char* name, Vec4 value );
void __namespace( SetMat4  )         ( Shader* shader, const char* name, const Mat4* value );
void __namespace( SetColor )         ( Shader* shader, const char* name, Vec3 value );

void __namespace( SetMvp      )      ( Shader* shader, const Mat4* model, const Mat4* view, const Mat4* proj );
void __namespace( SetMatrices )      ( Shader* shader, const Mat4* model, const Mat4* view, const Mat4* proj );

void __namespace( Destroy )          ( Shader* shader );


typedef struct ShaderLibrary ShaderLibrary;

ShaderLibrary* __namespace( LibraryCreate )         ( void );
void           __namespace( LibraryAdd )            ( ShaderLibrary* lib, Shader* shader );
Shader*        __namespace( LibraryGet )            ( ShaderLibrary* lib, const char* name );
void           __namespace( LibraryDestroy )        ( ShaderLibrary* lib );
void           __namespace( LibraryLoadDefaults )   ( ShaderLibrary* lib );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef __namespace

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif /* __shader_h__ */