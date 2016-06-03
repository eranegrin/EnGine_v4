#pragma once

#ifndef OGL4_H
#define OGL4_H


/////////////
// LINKING //
/////////////
#pragma comment(lib, "opengl32.lib")


//////////////
// INCLUDES //
//////////////
#include <windows.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <math.h>


typedef char GLchar;
typedef ptrdiff_t GLintptr;
typedef ptrdiff_t GLsizeiptr;

/////////////
// DEFINES //
/////////////
#define WGL_DRAW_TO_WINDOW_ARB								0x2001
#define WGL_ACCELERATION_ARB								0x2003
#define WGL_SWAP_METHOD_ARB									0x2007
#define WGL_SUPPORT_OPENGL_ARB								0x2010
#define WGL_DOUBLE_BUFFER_ARB								0x2011
#define WGL_PIXEL_TYPE_ARB									0x2013
#define WGL_COLOR_BITS_ARB									0x2014
#define WGL_DEPTH_BITS_ARB									0x2022
#define WGL_STENCIL_BITS_ARB								0x2023
#define WGL_FULL_ACCELERATION_ARB							0x2027
#define WGL_SWAP_EXCHANGE_ARB								0x2028
#define WGL_TYPE_RGBA_ARB									0x202B
#define WGL_CONTEXT_MAJOR_VERSION_ARB						0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB						0x2092
#define WGL_SAMPLE_BUFFERS_ARB								0x2041
#define WGL_SAMPLES_ARB										0x2042

#define GL_ARRAY_BUFFER										0x8892
#define GL_STREAM_DRAW										0x88E0
#define GL_STREAM_READ										0x88E1
#define GL_STREAM_COPY										0x88E2
#define GL_STATIC_DRAW										0x88E4
#define GL_STATIC_READ										0x88E5
#define GL_STATIC_COPY										0x88E6
#define GL_DYNAMIC_DRAW										0x88E8
#define GL_DYNAMIC_READ										0x88E9
#define GL_DYNAMIC_COPY										0x88EA
#define GL_FRAGMENT_SHADER									0x8B30
#define GL_VERTEX_SHADER									0x8B31
#define GL_GEOMETRY_SHADER									0x8DD9
#define GL_COMPILE_STATUS									0x8B81
#define GL_LINK_STATUS										0x8B82
#define GL_INFO_LOG_LENGTH									0x8B84
#define GL_TEXTURE0											0x84C0
#define GL_TEXTURE1											0x84C1
#define GL_TEXTURE2											0x84C2
#define GL_TEXTURE3											0x84C3
#define GL_TEXTURE4											0x84C4
#define GL_TEXTURE5											0x84C5
#define GL_TEXTURE6											0x84C6
#define GL_TEXTURE7											0x84C7
#define GL_TEXTURE8											0x84C8
#define GL_TEXTURE9											0x84C9
#define GL_TEXTURE10										0x84CA
#define GL_TEXTURE11										0x84CB
#define GL_TEXTURE12										0x84CC
#define GL_TEXTURE13										0x84CD
#define GL_TEXTURE14										0x84CE
#define GL_TEXTURE15										0x84CF
#define GL_TEXTURE16										0x84D0
#define GL_TEXTURE17										0x84D1
#define GL_TEXTURE18										0x84D2
#define GL_TEXTURE19										0x84D3
#define GL_TEXTURE20										0x84D4
#define GL_TEXTURE21										0x84D5
#define GL_TEXTURE22										0x84D6
#define GL_TEXTURE23										0x84D7
#define GL_TEXTURE24										0x84D8
#define GL_TEXTURE25										0x84D9
#define GL_TEXTURE26										0x84DA
#define GL_TEXTURE27										0x84DB
#define GL_TEXTURE28										0x84DC
#define GL_TEXTURE29										0x84DD
#define GL_TEXTURE30										0x84DE
#define GL_TEXTURE31										0x84DF
#define GL_ACTIVE_TEXTURE									0x84E0
#define GL_BGR												0x80E0
#define GL_BGRA												0x80E1
#define GL_RGBA32F											0x8814
#define GL_RGB32F											0x8815
#define GL_RGBA16F											0x881A
#define GL_RGB16F											0x881B
#define GL_HALF_FLOAT										0x140B
#define GL_TEXTURE_3D										0x806F
#define GL_PROGRAM_POINT_SIZE								0x8642

#define GL_TEXTURE_WRAP_R									0x8072
#define GL_NORMAL_MAP										0x8511
#define GL_REFLECTION_MAP									0x8512
#define GL_TEXTURE_CUBE_MAP									0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP							0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X						0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X						0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y						0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y						0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z						0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z						0x851A
#define GL_PROXY_TEXTURE_CUBE_MAP							0x851B
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE						0x851C


#define GL_ELEMENT_ARRAY_BUFFER								0x8893
#define GL_BUFFER_SIZE										0x8764
#define GL_CLAMP_TO_EDGE									0x812F
#define GL_MIRRORED_REPEAT									0x8370
#define GL_TEXTURE_MAX_ANISOTROPY							0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY						0x84FF

#define GL_UNIFORM_BUFFER                                  0x8A11
#define GL_UNIFORM_BUFFER_BINDING                          0x8A28
#define GL_UNIFORM_BUFFER_START                            0x8A29
#define GL_UNIFORM_BUFFER_SIZE                             0x8A2A
#define GL_MAX_VERTEX_UNIFORM_BLOCKS                       0x8A2B
#define GL_MAX_GEOMETRY_UNIFORM_BLOCKS                     0x8A2C
#define GL_MAX_FRAGMENT_UNIFORM_BLOCKS                     0x8A2D
#define GL_MAX_COMBINED_UNIFORM_BLOCKS                     0x8A2E
#define GL_MAX_UNIFORM_BUFFER_BINDINGS                     0x8A2F
#define GL_MAX_UNIFORM_BLOCK_SIZE                          0x8A30
#define GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS          0x8A31
#define GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS        0x8A32
#define GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS        0x8A33
#define GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT                 0x8A34
#define GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH            0x8A35
#define GL_ACTIVE_UNIFORM_BLOCKS                           0x8A36

// GL 3.3 - Geometry shader
#define GL_GEOMETRY_VERTICES_OUT			 0x8916
#define GL_GEOMETRY_INPUT_TYPE		0x8917
#define GL_GEOMETRY_OUTPUT_TYPE 0x8918
#define GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS 0x8C29
#define GL_FRAMEBUFFER_ATTACHMENT_LAYERED 0x8DA7
#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS 0x8DA8
#define GL_MAX_GEOMETRY_UNIFORM_COMPONENTS 0x8DDF
#define GL_MAX_GEOMETRY_OUTPUT_VERTICES 0x8DE0
#define GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS 0x8DE1
#define GL_MAX_VERTEX_OUTPUT_COMPONENTS 0x9122
#define GL_MAX_GEOMETRY_INPUT_COMPONENTS 0x9123
#define GL_MAX_GEOMETRY_OUTPUT_COMPONENTS 0x9124
#define GL_MAX_FRAGMENT_INPUT_COMPONENTS 0x9125
// GL 4.0 - Geometry shader
#define GL_GEOMETRY_SHADER_INVOCATIONS 0x887F
#define GL_SAMPLE_SHADING 0x8C36
#define GL_MIN_SAMPLE_SHADING_VALUE 0x8C37
#define GL_MAX_GEOMETRY_SHADER_INVOCATIONS 0x8E5A

#define GL_UNIFORM_TYPE                                    0x8A37
#define GL_UNIFORM_SIZE                                    0x8A38
#define GL_UNIFORM_NAME_LENGTH                             0x8A39
#define GL_UNIFORM_BLOCK_INDEX                             0x8A3A
#define GL_UNIFORM_OFFSET                                  0x8A3B
#define GL_UNIFORM_ARRAY_STRIDE                            0x8A3C
#define GL_UNIFORM_MATRIX_STRIDE                           0x8A3D
#define GL_UNIFORM_IS_ROW_MAJOR                            0x8A3E
#define GL_UNIFORM_BLOCK_BINDING                           0x8A3F
#define GL_UNIFORM_BLOCK_DATA_SIZE                         0x8A40
#define GL_UNIFORM_BLOCK_NAME_LENGTH                       0x8A41
#define GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS                   0x8A42
#define GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES            0x8A43
#define GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER       0x8A44
#define GL_UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADER     0x8A45
#define GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER     0x8A46

// FBO

#define GL_INVALID_FRAMEBUFFER_OPERATION 0x0506
#define GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING 0x8210
#define GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE 0x8211
#define GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE 0x8212
#define GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE 0x8213
#define GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE 0x8214
#define GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE 0x8215
#define GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE 0x8216
#define GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE 0x8217
#define GL_FRAMEBUFFER_DEFAULT 0x8218
#define GL_FRAMEBUFFER_UNDEFINED 0x8219
#define GL_DEPTH_STENCIL_ATTACHMENT 0x821A
#define GL_INDEX 0x8222
#define GL_MAX_RENDERBUFFER_SIZE 0x84E8
#define GL_DEPTH_STENCIL 0x84F9
#define GL_UNSIGNED_INT_24_8 0x84FA
#define GL_DEPTH24_STENCIL8 0x88F0
#define GL_TEXTURE_STENCIL_SIZE 0x88F1
#define GL_UNSIGNED_NORMALIZED 0x8C17
#define GL_SRGB 0x8C40
#define GL_DRAW_FRAMEBUFFER_BINDING 0x8CA6
#define GL_FRAMEBUFFER_BINDING 0x8CA6
#define GL_RENDERBUFFER_BINDING 0x8CA7
#define GL_READ_FRAMEBUFFER 0x8CA8
#define GL_DRAW_FRAMEBUFFER 0x8CA9
#define GL_READ_FRAMEBUFFER_BINDING 0x8CAA
#define GL_RENDERBUFFER_SAMPLES 0x8CAB
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE 0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME 0x8CD1
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL 0x8CD2
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE 0x8CD3
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER 0x8CD4
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT 0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT 0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER 0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER 0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED 0x8CDD
#define GL_MAX_COLOR_ATTACHMENTS 0x8CDF
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_COLOR_ATTACHMENT1 0x8CE1
#define GL_COLOR_ATTACHMENT2 0x8CE2
#define GL_COLOR_ATTACHMENT3 0x8CE3
#define GL_COLOR_ATTACHMENT4 0x8CE4
#define GL_COLOR_ATTACHMENT5 0x8CE5
#define GL_COLOR_ATTACHMENT6 0x8CE6
#define GL_COLOR_ATTACHMENT7 0x8CE7
#define GL_COLOR_ATTACHMENT8 0x8CE8
#define GL_COLOR_ATTACHMENT9 0x8CE9
#define GL_COLOR_ATTACHMENT10 0x8CEA
#define GL_COLOR_ATTACHMENT11 0x8CEB
#define GL_COLOR_ATTACHMENT12 0x8CEC
#define GL_COLOR_ATTACHMENT13 0x8CED
#define GL_COLOR_ATTACHMENT14 0x8CEE
#define GL_COLOR_ATTACHMENT15 0x8CEF
#define GL_DEPTH_ATTACHMENT 0x8D00
#define GL_STENCIL_ATTACHMENT 0x8D20
#define GL_FRAMEBUFFER 0x8D40
#define GL_RENDERBUFFER 0x8D41
#define GL_RENDERBUFFER_WIDTH 0x8D42
#define GL_RENDERBUFFER_HEIGHT 0x8D43
#define GL_RENDERBUFFER_INTERNAL_FORMAT 0x8D44
#define GL_STENCIL_INDEX1 0x8D46
#define GL_STENCIL_INDEX4 0x8D47
#define GL_STENCIL_INDEX8 0x8D48
#define GL_STENCIL_INDEX16 0x8D49
#define GL_RENDERBUFFER_RED_SIZE 0x8D50
#define GL_RENDERBUFFER_GREEN_SIZE 0x8D51
#define GL_RENDERBUFFER_BLUE_SIZE 0x8D52
#define GL_RENDERBUFFER_ALPHA_SIZE 0x8D53
#define GL_RENDERBUFFER_DEPTH_SIZE 0x8D54
#define GL_RENDERBUFFER_STENCIL_SIZE 0x8D55
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE 0x8D56
#define GL_MAX_SAMPLES 0x8D57

#define GL_BLEND_DST_RGB 0x80C8
#define GL_BLEND_SRC_RGB 0x80C9
#define GL_BLEND_DST_ALPHA 0x80CA
#define GL_BLEND_SRC_ALPHA 0x80CB
#define GL_POINT_SIZE_MIN 0x8126
#define GL_POINT_SIZE_MAX 0x8127
#define GL_POINT_FADE_THRESHOLD_SIZE 0x8128
#define GL_POINT_DISTANCE_ATTENUATION 0x8129
#define GL_GENERATE_MIPMAP 0x8191
#define GL_GENERATE_MIPMAP_HINT 0x8192
#define GL_DEPTH_COMPONENT16 0x81A5
#define GL_DEPTH_COMPONENT24 0x81A6
#define GL_DEPTH_COMPONENT32 0x81A7
#define GL_MIRRORED_REPEAT 0x8370
#define GL_FOG_COORDINATE_SOURCE 0x8450
#define GL_FOG_COORDINATE 0x8451
#define GL_FRAGMENT_DEPTH 0x8452
#define GL_CURRENT_FOG_COORDINATE 0x8453
#define GL_FOG_COORDINATE_ARRAY_TYPE 0x8454
#define GL_FOG_COORDINATE_ARRAY_STRIDE 0x8455
#define GL_FOG_COORDINATE_ARRAY_POINTER 0x8456
#define GL_FOG_COORDINATE_ARRAY 0x8457
#define GL_COLOR_SUM 0x8458
#define GL_CURRENT_SECONDARY_COLOR 0x8459
#define GL_SECONDARY_COLOR_ARRAY_SIZE 0x845A
#define GL_SECONDARY_COLOR_ARRAY_TYPE 0x845B
#define GL_SECONDARY_COLOR_ARRAY_STRIDE 0x845C
#define GL_SECONDARY_COLOR_ARRAY_POINTER 0x845D
#define GL_SECONDARY_COLOR_ARRAY 0x845E
#define GL_MAX_TEXTURE_LOD_BIAS 0x84FD
#define GL_TEXTURE_FILTER_CONTROL 0x8500
#define GL_TEXTURE_LOD_BIAS 0x8501
#define GL_INCR_WRAP 0x8507
#define GL_DECR_WRAP 0x8508
#define GL_TEXTURE_DEPTH_SIZE 0x884A
#define GL_DEPTH_TEXTURE_MODE 0x884B
#define GL_TEXTURE_COMPARE_MODE 0x884C
#define GL_TEXTURE_COMPARE_FUNC 0x884D
#define GL_COMPARE_R_TO_TEXTURE 0x884E

#define GL_TEXTURE_2D_MULTISAMPLE 0x9100
#define GL_MULTISAMPLE            0x809D

#define GL_COMPARE_R_TO_TEXTURE_ARB 0x884E
#define GL_COMPARE_REF_TO_TEXTURE 0x884E

#define GL_R8 0x8229
#define GL_FUNC_ADD 0x8006
#define GL_FUNC_SUBTRACT 0x800A
#define GL_FUNC_REVERSE_SUBTRACT 0x800B

//////////////
// TYPEDEFS //
//////////////
typedef BOOL (WINAPI * PFNWGLCHOOSEPIXELFORMATARBPROC) (HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
typedef HGLRC (WINAPI * PFNWGLCREATECONTEXTATTRIBSARBPROC) (HDC hDC, HGLRC hShareContext, const int *attribList);
typedef BOOL (WINAPI * PFNWGLSWAPINTERVALEXTPROC) (int interval);
typedef void (APIENTRY * PFNGLATTACHSHADERPROC) (GLuint program, GLuint shader);
typedef void (APIENTRY * PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
typedef void (APIENTRY * PFNGLBINDVERTEXARRAYPROC) (GLuint array);
typedef void (APIENTRY * PFNGLBUFFERDATAPROC) (GLenum target, ptrdiff_t size, const GLvoid *data, GLenum usage);
typedef void (APIENTRY * PFNGLCOMPILESHADERPROC) (GLuint shader);
typedef GLuint (APIENTRY * PFNGLCREATEPROGRAMPROC) (void);
typedef GLuint (APIENTRY * PFNGLCREATESHADERPROC) (GLenum type);
typedef void (APIENTRY * PFNGLDELETEBUFFERSPROC) (GLsizei n, const GLuint *buffers);
typedef void (APIENTRY * PFNGLDELETEPROGRAMPROC) (GLuint program);
typedef void (APIENTRY * PFNGLDELETESHADERPROC) (GLuint shader);
typedef void (APIENTRY * PFNGLDELETEVERTEXARRAYSPROC) (GLsizei n, const GLuint *arrays);
typedef void (APIENTRY * PFNGLDETACHSHADERPROC) (GLuint program, GLuint shader);
typedef void (APIENTRY * PFNGLENABLEVERTEXATTRIBARRAYPROC) (GLuint index);
typedef void (APIENTRY * PFNGLGENBUFFERSPROC) (GLsizei n, GLuint *buffers);
typedef void (APIENTRY * PFNGLGENVERTEXARRAYSPROC) (GLsizei n, GLuint *arrays);
typedef GLint (APIENTRY * PFNGLGETATTRIBLOCATIONPROC) (GLuint program, const char *name);
typedef void (APIENTRY * PFNGLGETPROGRAMINFOLOGPROC) (GLuint program, GLsizei bufSize, GLsizei *length, char *infoLog);
typedef void (APIENTRY * PFNGLGETPROGRAMIVPROC) (GLuint program, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLGETSHADERINFOLOGPROC) (GLuint shader, GLsizei bufSize, GLsizei *length, char *infoLog);
typedef void (APIENTRY * PFNGLGETSHADERIVPROC) (GLuint shader, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLLINKPROGRAMPROC) (GLuint program);
typedef void (APIENTRY * PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const char* *string, const GLint *length);
typedef void (APIENTRY * PFNGLUSEPROGRAMPROC) (GLuint program);
typedef void (APIENTRY * PFNGLVERTEXATTRIBPOINTERPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
typedef void (APIENTRY * PFNGLBINDATTRIBLOCATIONPROC) (GLuint program, GLuint index, const char *name);
typedef GLint (APIENTRY * PFNGLGETUNIFORMLOCATIONPROC) (GLuint program, const char *name);
typedef void (APIENTRY * PFNGLUNIFORMMATRIX4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRY * PFNGLACTIVETEXTUREPROC) (GLenum texture);
typedef void (APIENTRY * PFNGLUNIFORM1IPROC) (GLint location, GLint v0);
typedef void (APIENTRY * PFNGLUNIFORM1FPROC) (GLint location, GLfloat v0);
typedef void (APIENTRY * PFNGLGENERATEMIPMAPPROC) (GLenum target);
typedef void (APIENTRY * PFNGLDISABLEVERTEXATTRIBARRAYPROC) (GLuint index);
typedef void (APIENTRY * PFNGLUNIFORM1FVPROC) (GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRY * PFNGLUNIFORM3FVPROC) (GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRY * PFNGLUNIFORM4FVPROC) (GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRY * PFNGLGETBUFFERPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRY * PFNGLBUFFERSUBDATAPROC) (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data);

typedef void (APIENTRY * PFNGLCOPYTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (APIENTRY * PFNGLDRAWRANGEELEMENTSPROC) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);
typedef void (APIENTRY * PFNGLTEXIMAGE3DPROC) (GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (APIENTRY * PFNGLTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);

// Uniform buffers
typedef void (APIENTRY * PFNGLBINDBUFFERBASEPROC) (GLenum target, GLuint index, GLuint buffer);
typedef void (APIENTRY * PFNGLBINDBUFFERRANGEPROC) (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
typedef void (APIENTRY * PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC) (GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName);
typedef void (APIENTRY * PFNGLGETACTIVEUNIFORMBLOCKIVPROC) (GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params);
typedef void (APIENTRY * PFNGLGETACTIVEUNIFORMNAMEPROC) (GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformName);
typedef void (APIENTRY * PFNGLGETACTIVEUNIFORMSIVPROC) (GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params);
typedef void (APIENTRY * PFNGLGETINTEGERI_VPROC) (GLenum target, GLuint index, GLint* data);
typedef GLuint (APIENTRY * PFNGLGETUNIFORMBLOCKINDEXPROC) (GLuint program, const GLchar* uniformBlockName);
typedef void (APIENTRY * PFNGLGETUNIFORMINDICESPROC) (GLuint program, GLsizei uniformCount, const GLchar** uniformNames, GLuint* uniformIndices);
typedef void (APIENTRY * PFNGLUNIFORMBLOCKBINDINGPROC) (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
// Subroutine
typedef GLuint (APIENTRY * PFNGLGETSUBROUTINEINDEXPROC) (GLuint program, GLenum shadertype, const GLchar* name);
typedef GLint (APIENTRY * PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC) (GLuint program, GLenum shadertype, const GLchar* name);
typedef void (APIENTRY * PFNGLGETUNIFORMSUBROUTINEUIVPROC) (GLenum shadertype, GLint location, GLuint* params);
typedef void (APIENTRY * PFNGLUNIFORMSUBROUTINESUIVPROC) (GLenum shadertype, GLsizei count, const GLuint* indices);
// FBO
typedef void (APIENTRY * PFNGLBINDFRAMEBUFFERPROC) (GLenum target, GLuint framebuffer);
typedef void (APIENTRY * PFNGLBINDRENDERBUFFERPROC) (GLenum target, GLuint renderbuffer);
typedef void (APIENTRY * PFNGLBLITFRAMEBUFFERPROC) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
typedef GLenum (APIENTRY * PFNGLCHECKFRAMEBUFFERSTATUSPROC) (GLenum target);
typedef void (APIENTRY * PFNGLDELETEFRAMEBUFFERSPROC) (GLsizei n, const GLuint* framebuffers);
typedef void (APIENTRY * PFNGLDELETERENDERBUFFERSPROC) (GLsizei n, const GLuint* renderbuffers);
typedef void (APIENTRY * PFNGLFRAMEBUFFERRENDERBUFFERPROC) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void (APIENTRY * PFNGLFRAMEBUFFERTEXTURE1DPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (APIENTRY * PFNGLFRAMEBUFFERTEXTURE2DPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (APIENTRY * PFNGLFRAMEBUFFERTEXTURE3DPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint layer);
typedef void (APIENTRY * PFNGLGENFRAMEBUFFERSPROC) (GLsizei n, GLuint* framebuffers);
typedef void (APIENTRY * PFNGLGENRENDERBUFFERSPROC) (GLsizei n, GLuint* renderbuffers);
typedef void (APIENTRY * PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC) (GLenum target, GLenum attachment, GLenum pname, GLint* params);
typedef void (APIENTRY * PFNGLGETRENDERBUFFERPARAMETERIVPROC) (GLenum target, GLenum pname, GLint* params);
typedef GLboolean (APIENTRY * PFNGLISFRAMEBUFFERPROC) (GLuint framebuffer);
typedef GLboolean (APIENTRY * PFNGLISRENDERBUFFERPROC) (GLuint renderbuffer);
typedef void (APIENTRY * PFNGLRENDERBUFFERSTORAGEPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (APIENTRY * PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);

typedef void (APIENTRY * PFNGLDRAWBUFFERSPROC) (GLsizei n, const GLenum* bufs);

typedef void (APIENTRY * PFNGLTEXIMAGE2DMULTISAMPLEPROC) (GLenum target, GLsizei samples, GLint internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
typedef void (APIENTRY * PFNGLBLENDEQUATIONPROC) (GLenum mode);
typedef void (APIENTRY * PFNGLBLENDFUNCSEPARATEEXTPROC) (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);

class OGL4
{
public:
	OGL4();
	~OGL4();

	bool InitializeOpenGL(HWND, int, int, bool);
	bool InitializeExtensions(HWND);
	void Shutdown(HWND);

	void BeginScene(float, float, float, float);
	void ClearBuffers(GLsizei iw, GLsizei ih);
	void SwapBuffersHDC()
	{
		// Present the back buffer to the screen since rendering is complete.
		SwapBuffers(m_deviceContext);
	}

	void GetVideoCardInfo(char*);
// 	const GLenum arrAttachments[] {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};

	
private:
	bool LoadExtensionList();	

private:
	HDC m_deviceContext;
	HGLRC m_renderingContext;
	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
	char m_videoCardDescription[128];

public:
	PFNGLATTACHSHADERPROC glAttachShader;
	PFNGLBINDBUFFERPROC glBindBuffer;
	PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
	PFNGLBUFFERDATAPROC glBufferData;
	PFNGLCOMPILESHADERPROC glCompileShader;
	PFNGLCREATEPROGRAMPROC glCreateProgram;
	PFNGLCREATESHADERPROC glCreateShader;
	PFNGLDELETEBUFFERSPROC glDeleteBuffers;
	PFNGLDELETEPROGRAMPROC glDeleteProgram;
	PFNGLDELETESHADERPROC glDeleteShader;
	PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
	PFNGLDETACHSHADERPROC glDetachShader;
	PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
	PFNGLGENBUFFERSPROC glGenBuffers;
	PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
	PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
	PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
	PFNGLGETPROGRAMIVPROC glGetProgramiv;
	PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
	PFNGLGETSHADERIVPROC glGetShaderiv;
	PFNGLLINKPROGRAMPROC glLinkProgram;
	PFNGLSHADERSOURCEPROC glShaderSource;
	PFNGLUSEPROGRAMPROC glUseProgram;
	PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
	PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation;
	PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
	PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
	PFNGLACTIVETEXTUREPROC glActiveTexture;
	PFNGLUNIFORM1IPROC glUniform1i;
	PFNGLUNIFORM1FPROC glUniform1f;
	PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
	PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
	PFNGLUNIFORM1FVPROC glUniform1fv;
	PFNGLUNIFORM3FVPROC glUniform3fv;
	PFNGLUNIFORM4FVPROC glUniform4fv;
	PFNGLGETBUFFERPARAMETERIVPROC glGetBufferParameteriv;
	PFNGLBUFFERSUBDATAPROC glBufferSubData;
	// 3D Texture
	PFNGLCOPYTEXSUBIMAGE3DPROC glCopyTexSubImage3D;
	PFNGLDRAWRANGEELEMENTSPROC glDrawRangeElements;
	PFNGLTEXIMAGE3DPROC glTexImage3D;
	PFNGLTEXSUBIMAGE3DPROC glTexSubImage3D;
	//EO: 3D Texture

	PFNGLBINDBUFFERBASEPROC glBindBufferBase;
	PFNGLBINDBUFFERRANGEPROC glBindBufferRange;
	PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC glGetActiveUniformBlockName;
	PFNGLGETACTIVEUNIFORMBLOCKIVPROC glGetActiveUniformBlockiv;
	PFNGLGETACTIVEUNIFORMNAMEPROC glGetActiveUniformName;
	PFNGLGETACTIVEUNIFORMSIVPROC glGetActiveUniformsiv;
	PFNGLGETINTEGERI_VPROC glGetIntegeri_v;
	PFNGLGETUNIFORMBLOCKINDEXPROC glGetUniformBlockIndex;
	PFNGLGETUNIFORMINDICESPROC glGetUniformIndices;
	PFNGLUNIFORMBLOCKBINDINGPROC glUniformBlockBinding;

	PFNGLGETSUBROUTINEINDEXPROC glGetSubroutineIndex;
	PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC glGetSubroutineUniformLocation;
	PFNGLGETUNIFORMSUBROUTINEUIVPROC glGetUniformSubroutineuiv;
	PFNGLUNIFORMSUBROUTINESUIVPROC glUniformSubroutinesuiv;

	// FBO
	PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
	PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
	PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer;
	PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
	PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
	PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;
	PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;
	PFNGLFRAMEBUFFERTEXTURE1DPROC glFramebufferTexture1D;
	PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
	PFNGLFRAMEBUFFERTEXTURE3DPROC glFramebufferTexture3D;
	PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
	PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
	PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC glGetFramebufferAttachmentParameteriv;
	PFNGLGETRENDERBUFFERPARAMETERIVPROC glGetRenderbufferParameteriv;
	PFNGLISFRAMEBUFFERPROC glIsFramebuffer;
	PFNGLISRENDERBUFFERPROC glIsRenderbuffer;
	PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
	PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC glRenderbufferStorageMultisample;
	PFNGLDRAWBUFFERSPROC glDrawBuffers;
	PFNGLTEXIMAGE2DMULTISAMPLEPROC glTexImage2DMultisample;
	PFNGLBLENDEQUATIONPROC glBlendEquation;
	PFNGLBLENDFUNCSEPARATEEXTPROC glBlendFuncSeparate;
};


#endif  //OGL4_H
