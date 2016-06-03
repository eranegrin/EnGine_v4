#include "OGL4.h"
#include "../EnGBasic/EnGBasic.h"

using namespace EnG;

OGL4::OGL4()
{
}


OGL4::~OGL4()
{

}

bool OGL4::InitializeOpenGL(HWND hwnd, int screenWidth, int screenHeight, bool vsync)
{
	int attributeListInt[23];
	int pixelFormat[1];
	unsigned int formatCount;
	int result;
	PIXELFORMATDESCRIPTOR pixelFormatDescriptor;
	int attributeList[5];
	char *vendorString, *rendererString;


	// Get the device context for this window.
	m_deviceContext = GetDC(hwnd);
	if(!m_deviceContext)
	{
		return false;
	}

	// Support for OpenGL rendering.
	attributeListInt[0] = WGL_SUPPORT_OPENGL_ARB;
	attributeListInt[1] = TRUE;

	// Support for rendering to a window.
	attributeListInt[2] = WGL_DRAW_TO_WINDOW_ARB;
	attributeListInt[3] = TRUE;

	// Support for hardware acceleration.
	attributeListInt[4] = WGL_ACCELERATION_ARB;
	attributeListInt[5] = WGL_FULL_ACCELERATION_ARB;

	// Support for 24 bit color.
	attributeListInt[6] = WGL_COLOR_BITS_ARB;
	attributeListInt[7] = 32;

	// Support for 24 bit depth buffer.
	attributeListInt[8] = WGL_DEPTH_BITS_ARB;
	attributeListInt[9] = 24;

	// Support for double buffer.
	attributeListInt[10] = WGL_DOUBLE_BUFFER_ARB;
	attributeListInt[11] = TRUE;

	// Support for swapping front and back buffer.
	attributeListInt[12] = WGL_SWAP_METHOD_ARB;
	attributeListInt[13] = WGL_SWAP_EXCHANGE_ARB;

	// Support for the RGBA pixel type.
	attributeListInt[14] = WGL_PIXEL_TYPE_ARB;
	attributeListInt[15] = WGL_TYPE_RGBA_ARB;

	// Support for a 8 bit stencil buffer.
	attributeListInt[16] = WGL_STENCIL_BITS_ARB;
	attributeListInt[17] = 8;

	// Support for Multi Sampling
	attributeListInt[18] = WGL_SAMPLE_BUFFERS_ARB;
	attributeListInt[19] = GL_TRUE;

	// Support for 16 bit Multi Sampling
	attributeListInt[20] = WGL_SAMPLES_ARB;
	attributeListInt[21] = 16;

	// Null terminate the attribute list.
	attributeListInt[22] = 0;

	// Query for a pixel format that fits the attributes we want.
	result = wglChoosePixelFormatARB(m_deviceContext, attributeListInt, NULL, 1, pixelFormat, &formatCount);
	if(result != 1)
	{
		return false;
	}

	// If the video card/display can handle our desired pixel format then we set it as the current one.
	result = SetPixelFormat(m_deviceContext, pixelFormat[0], &pixelFormatDescriptor);
	if(result != 1)
	{
		return false;
	}

	// Set the 4.0 version of OpenGL in the attribute list.
	attributeList[0] = WGL_CONTEXT_MAJOR_VERSION_ARB;
	attributeList[1] = 4;
	attributeList[2] = WGL_CONTEXT_MINOR_VERSION_ARB;
	attributeList[3] = 0;
	
	// Null terminate the attribute list.
	attributeList[4] = 0;

	// Create a OpenGL 4.0 rendering context.
	m_renderingContext = wglCreateContextAttribsARB(m_deviceContext, 0, attributeList);
	if(m_renderingContext == NULL)
	{
		return false;
	}

	// Set the rendering context to active.
	result = wglMakeCurrent(m_deviceContext, m_renderingContext);
	if(result != 1)
	{
		return false;
	}

	// Set the depth buffer to be entirely cleared to 1.0 values.
	glClearDepth(1.0f);

	// Enable depth and stencil testing.
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glShadeModel(GL_SMOOTH);

	// Set the polygon winding to front facing for the left handed system.
	glFrontFace(GL_CCW);

	// Enable back face culling.
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	
	// Get the name of the video card.
	vendorString = (char*)glGetString(GL_VENDOR);
	rendererString = (char*)glGetString(GL_RENDERER);

	// Store the video card name in a class member variable so it can be retrieved later.
	strcpy_s(m_videoCardDescription, vendorString);
	strcat_s(m_videoCardDescription, " - ");
	strcat_s(m_videoCardDescription, rendererString);

	// Turn on or off the vertical sync depending on the input bool value.
	if(vsync)
	{
		result = wglSwapIntervalEXT(1);
	}
	else
	{
		result = wglSwapIntervalEXT(0);
	}

	// Check if vsync was set correctly.
	if(result != 1)
	{
		return false;
	}

	
	return true;
}

void OGL4::ClearBuffers(GLsizei iw, GLsizei ih)
{
	// Set the color to clear the screen to.
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 	
	// Clear the screen and depth buffer.
	glClearDepth(1.0f);
	GLuint uiClear = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
	glClear(uiClear);
	glViewport(0, 0, iw, ih);
}

void OGL4::Shutdown( HWND hwnd )
{

}

bool OGL4::InitializeExtensions( HWND hwnd )
{
	HDC deviceContext;
	PIXELFORMATDESCRIPTOR pixelFormat;
	int error;
	HGLRC renderContext;
	bool result;
	
	// Get the device context for this window.
	deviceContext = GetDC(hwnd);
	if(!deviceContext)
	{
		return false;
	}

	// Set a temporary default pixel format.
	error = SetPixelFormat(deviceContext, 1, &pixelFormat);
	if(error != 1)
	{
		return false;
	}

	// Create a temporary rendering context.
	renderContext = wglCreateContext(deviceContext);
	if(!renderContext)
	{
		return false;
	}

	// Set the temporary rendering context as the current rendering context for this window.
	error = wglMakeCurrent(deviceContext, renderContext);
	if(error != 1)
	{
		return false;
	}

	// Initialize the OpenGL extensions needed for this application.  Note that a temporary rendering context was needed to do so.
	result = LoadExtensionList();
	if(!result)
	{
		return false;
	}

	
	//GLuint base = glGenLists(96);						// Storage For 96 Characters
	//HFONT	font;										// Windows Font ID
	//HFONT	oldfont;									// Used For Good House Keeping


	//font = CreateFont(	-24,							// Height Of Font
	//	0,								// Width Of Font
	//	0,								// Angle Of Escapement
	//	0,								// Orientation Angle
	//	FW_BOLD,						// Font Weight
	//	FALSE,							// Italic
	//	FALSE,							// Underline
	//	FALSE,							// Strikeout
	//	ANSI_CHARSET,					// Character Set Identifier
	//	OUT_TT_PRECIS,					// Output Precision
	//	CLIP_DEFAULT_PRECIS,			// Clipping Precision
	//	ANTIALIASED_QUALITY,			// Output Quality
	//	FF_DONTCARE|DEFAULT_PITCH,		// Family And Pitch
	//	"Times New Roman");					// Font Name

	//oldfont = (HFONT)SelectObject(deviceContext, font);           // Selects The Font We Want
	//wglUseFontBitmaps(deviceContext, 32, 96, base);				// Builds 96 Characters Starting At Character 32
	//SelectObject(deviceContext, oldfont);							// Selects The Font We Want
	//DeleteObject(font);									// Delete The Font

	// Release the temporary rendering context now that the extensions have been loaded.
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(renderContext);
	renderContext = NULL;

	// Release the device context for this window.
	ReleaseDC(hwnd, deviceContext);
	deviceContext = NULL;

	
	return true;
}

bool OGL4::LoadExtensionList()
{
	// Load the OpenGL extensions that this application will be using.
	wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
	if(!wglChoosePixelFormatARB)
	{
		PRINT_ERROR("EXT: wglChoosePixelFormatARB fail");
		return false;
	}

	wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
	if(!wglCreateContextAttribsARB)
	{
		PRINT_ERROR("EXT: wglCreateContextAttribsARB fail");
		return false;
	}

	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
	if(!wglSwapIntervalEXT)
	{
		PRINT_ERROR("EXT: wglSwapIntervalEXT fail");
		return false;
	}

	glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
	if(!glAttachShader)
	{
		PRINT_ERROR("EXT: glAttachShader fail");
		return false;
	}

	glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
	if(!glBindBuffer)
	{
		PRINT_ERROR("EXT: glBindBuffer fail");
		return false;
	}

	glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)wglGetProcAddress("glBindVertexArray");
	if(!glBindVertexArray)
	{
		PRINT_ERROR("EXT: glBindVertexArray fail");
		return false;
	}

	glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
	if(!glBufferData)
	{
		PRINT_ERROR("EXT: glBufferData fail");
		return false;
	}

	glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
	if(!glCompileShader)
	{
		PRINT_ERROR("EXT: glCompileShader fail");
		return false;
	}

	glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
	if(!glCreateProgram)
	{
		PRINT_ERROR("EXT: glCreateProgram fail");
		return false;
	}

	glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
	if(!glCreateShader)
	{
		PRINT_ERROR("EXT: glCreateShader fail");
		return false;
	}

	glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
	if(!glDeleteBuffers)
	{
		PRINT_ERROR("EXT: glDeleteBuffers fail");
		return false;
	}

	glDeleteProgram = (PFNGLDELETEPROGRAMPROC)wglGetProcAddress("glDeleteProgram");
	if(!glDeleteProgram)
	{
		PRINT_ERROR("EXT: glDeleteProgram fail");
		return false;
	}

	glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
	if(!glDeleteShader)
	{
		PRINT_ERROR("EXT: glDeleteShader fail");
		return false;
	}

	glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)wglGetProcAddress("glDeleteVertexArrays");
	if(!glDeleteVertexArrays)
	{
		PRINT_ERROR("EXT: glDeleteVertexArrays fail");
		return false;
	}

	glDetachShader = (PFNGLDETACHSHADERPROC)wglGetProcAddress("glDetachShader");
	if(!glDetachShader)
	{
		PRINT_ERROR("EXT: glDetachShader fail");
		return false;
	}

	glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glEnableVertexAttribArray");
	if(!glEnableVertexAttribArray)
	{
		PRINT_ERROR("EXT: glEnableVertexAttribArray fail");
		return false;
	}

	glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
	if(!glGenBuffers)
	{
		PRINT_ERROR("EXT: glGenBuffers fail");
		return false;
	}

	glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)wglGetProcAddress("glGenVertexArrays");
	if(!glGenVertexArrays)
	{
		PRINT_ERROR("EXT: glGenVertexArrays fail");
		return false;
	}

	glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)wglGetProcAddress("glGetAttribLocation");
	if(!glGetAttribLocation)
	{
		PRINT_ERROR("EXT: glGetAttribLocation fail");
		return false;
	}

	glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog");
	if(!glGetProgramInfoLog)
	{
		PRINT_ERROR("EXT: glGetProgramInfoLog fail");
		return false;
	}

	glGetProgramiv = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv");
	if(!glGetProgramiv)
	{
		PRINT_ERROR("EXT: glGetProgramiv fail");
		return false;
	}

	glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetShaderInfoLog");
	if(!glGetShaderInfoLog)
	{
		PRINT_ERROR("EXT: glGetShaderInfoLog fail");
		return false;
	}

	glGetShaderiv = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
	if(!glGetShaderiv)
	{
		PRINT_ERROR("EXT: glGetShaderiv fail");
		return false;
	}

	glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
	if(!glLinkProgram)
	{
		PRINT_ERROR("EXT: glLinkProgram fail");
		return false;
	}

	glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
	if(!glShaderSource)
	{
		PRINT_ERROR("EXT: glShaderSource fail");
		return false;
	}

	glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
	if(!glUseProgram)
	{
		PRINT_ERROR("EXT: glUseProgram fail");
		return false;
	}

	glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress("glVertexAttribPointer");
	if(!glVertexAttribPointer)
	{
		PRINT_ERROR("EXT: glVertexAttribPointer fail");
		return false;
	}

	glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC)wglGetProcAddress("glBindAttribLocation");
	if(!glBindAttribLocation)
	{
		PRINT_ERROR("EXT: glBindAttribLocation fail");
		return false;
	}

	glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
	if(!glGetUniformLocation)
	{
		PRINT_ERROR("EXT: glGetUniformLocation fail");
		return false;
	}

	glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)wglGetProcAddress("glUniformMatrix4fv");
	if(!glUniformMatrix4fv)
	{
		PRINT_ERROR("EXT: glUniformMatrix4fv fail");
		return false;
	}

	glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
	if(!glActiveTexture)
	{
		PRINT_ERROR("EXT: glActiveTexture fail");
		return false;
	}

	glUniform1i = (PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i");
	if(!glUniform1i)
	{
		PRINT_ERROR("EXT: glUniform1i fail");
		return false;
	}

	glUniform1f = (PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f");
	if(!glUniform1f)
	{
		PRINT_ERROR("EXT: glUniform1f fail");
		return false;
	}

	glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)wglGetProcAddress("glGenerateMipmap");
	if(!glGenerateMipmap)
	{
		PRINT_ERROR("EXT: glGenerateMipmap fail");
		return false;
	}

	glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glDisableVertexAttribArray");
	if(!glDisableVertexAttribArray)
	{
		PRINT_ERROR("EXT: glDisableVertexAttribArray fail");
		return false;
	}
	
	glUniform1fv = (PFNGLUNIFORM1FVPROC)wglGetProcAddress("glUniform1fv");
	if(!glUniform1fv)
	{
		PRINT_ERROR("EXT: glUniform1fv fail");
		return false;
	}


	glUniform3fv = (PFNGLUNIFORM3FVPROC)wglGetProcAddress("glUniform3fv");
	if(!glUniform3fv)
	{
		PRINT_ERROR("EXT: glUniform3fv fail");
		return false;
	}

	glUniform4fv = (PFNGLUNIFORM4FVPROC)wglGetProcAddress("glUniform4fv");
	if(!glUniform4fv)
	{
		PRINT_ERROR("EXT: glUniform4fv fail");
		return false;
	}
	
	glGetBufferParameteriv = (PFNGLGETBUFFERPARAMETERIVPROC)wglGetProcAddress("glGetBufferParameteriv");
	if(!glGetBufferParameteriv)
	{
		PRINT_ERROR("EXT: glGetBufferParameteriv fail");
		return false;
	}
	
	glBufferSubData = (PFNGLBUFFERSUBDATAPROC)wglGetProcAddress("glBufferSubData");
	if(!glBufferSubData)
	{
		PRINT_ERROR("EXT: glBufferSubData fail");
		return false;
	}	

	// 3D Texture
	glCopyTexSubImage3D = (PFNGLCOPYTEXSUBIMAGE3DPROC)wglGetProcAddress("glCopyTexSubImage3D");
	if(!glCopyTexSubImage3D)
	{
		PRINT_ERROR("EXT: glCopyTexSubImage3D fail");
		return false;
	}	
	glDrawRangeElements = (PFNGLDRAWRANGEELEMENTSPROC)wglGetProcAddress("glDrawRangeElements");
	if(!glDrawRangeElements)
	{
		PRINT_ERROR("EXT: glDrawRangeElements fail");
		return false;
	}	
	glTexImage3D = (PFNGLTEXIMAGE3DPROC)wglGetProcAddress("glTexImage3D");
	if(!glTexImage3D)
	{
		PRINT_ERROR("EXT: glTexImage3D fail");
		return false;
	}	
	glTexSubImage3D = (PFNGLTEXSUBIMAGE3DPROC)wglGetProcAddress("glTexSubImage3D");
	if(!glTexSubImage3D)
	{
		PRINT_ERROR("EXT: glTexSubImage3D fail");
		return false;
	}	
	// EO: 3D Texture

	glBindBufferBase = (PFNGLBINDBUFFERBASEPROC)wglGetProcAddress("glBindBufferBase");
	if(!glBindBufferBase)
	{
		PRINT_ERROR("EXT: glBindBufferBase fail");
		return false;
	}

	glBindBufferRange = (PFNGLBINDBUFFERRANGEPROC)wglGetProcAddress("glBindBufferRange");
	if(!glBindBufferRange)
	{
		PRINT_ERROR("EXT: glBindBufferRange fail");
		return false;
	}

	glGetActiveUniformBlockName = (PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC)wglGetProcAddress("glGetActiveUniformBlockName");
	if(!glGetActiveUniformBlockName)
	{
		PRINT_ERROR("EXT: glGetActiveUniformBlockName fail");
		return false;
	}

	glGetActiveUniformBlockiv = (PFNGLGETACTIVEUNIFORMBLOCKIVPROC)wglGetProcAddress("glGetActiveUniformBlockiv");
	if(!glGetActiveUniformBlockiv)
	{
		PRINT_ERROR("EXT: glGetActiveUniformBlockiv fail");
		return false;
	}
	
	glGetActiveUniformName = (PFNGLGETACTIVEUNIFORMNAMEPROC)wglGetProcAddress("glGetActiveUniformName");
	if(!glGetActiveUniformName)
	{
		PRINT_ERROR("EXT: glGetActiveUniformName fail");
		return false;
	}
	
	
	glGetActiveUniformsiv = (PFNGLGETACTIVEUNIFORMSIVPROC)wglGetProcAddress("glGetActiveUniformsiv");
	if(!glGetActiveUniformsiv)
	{
		PRINT_ERROR("EXT: glGetActiveUniformsiv fail");
		return false;
	}
	
	glGetIntegeri_v = (PFNGLGETINTEGERI_VPROC)wglGetProcAddress("glGetIntegeri_v");
	if(!glGetIntegeri_v)
	{
		PRINT_ERROR("EXT: glGetIntegeri_v fail");
		return false;
	}
	
	glGetUniformBlockIndex = (PFNGLGETUNIFORMBLOCKINDEXPROC)wglGetProcAddress("glGetUniformBlockIndex");
	if(!glGetUniformBlockIndex)
	{
		PRINT_ERROR("EXT: glGetUniformBlockIndex fail");
		return false;
	}

	glGetUniformIndices = (PFNGLGETUNIFORMINDICESPROC)wglGetProcAddress("glGetUniformIndices");
	if(!glGetUniformIndices)
	{
		PRINT_ERROR("EXT: glGetUniformIndices fail");
		return false;
	}
	
	glUniformBlockBinding = (PFNGLUNIFORMBLOCKBINDINGPROC)wglGetProcAddress("glUniformBlockBinding");
	if(!glUniformBlockBinding)
	{
		PRINT_ERROR("EXT: glUniformBlockBinding fail");
		return false;
	}

	glGetSubroutineIndex = (PFNGLGETSUBROUTINEINDEXPROC)wglGetProcAddress("glGetSubroutineIndex");
	if(!glGetSubroutineIndex)
	{
		PRINT_ERROR("EXT: glGetSubroutineIndex fail");
		return false;
	}

	glGetSubroutineUniformLocation = (PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC)wglGetProcAddress("glGetSubroutineUniformLocation");
	if(!glGetSubroutineUniformLocation)
	{
		PRINT_ERROR("EXT: glGetSubroutineUniformLocation fail");
		return false;
	}

	glGetUniformSubroutineuiv = (PFNGLGETUNIFORMSUBROUTINEUIVPROC)wglGetProcAddress("glGetUniformSubroutineuiv");
	if(!glGetUniformSubroutineuiv)
	{
		PRINT_ERROR("EXT: glGetUniformSubroutineuiv fail");
		return false;
	}

	glUniformSubroutinesuiv = (PFNGLUNIFORMSUBROUTINESUIVPROC)wglGetProcAddress("glUniformSubroutinesuiv");
	if(!glUniformSubroutinesuiv)
	{
		PRINT_ERROR("EXT: glUniformSubroutinesuiv fail");
		return false;
	}
	 //FBO
	glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)wglGetProcAddress("glBindFramebuffer");
	if(!glBindFramebuffer)
	{
		PRINT_ERROR("EXT: glBindFramebuffer fail");
		return false;
	}

	glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)wglGetProcAddress("glBindRenderbuffer");
	if(!glBindRenderbuffer)
	{
		PRINT_ERROR("EXT: glBindRenderbuffer fail");
		return false;
	}

	glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC)wglGetProcAddress("glBlitFramebuffer");
	if(!glBlitFramebuffer)
	{
		PRINT_ERROR("EXT: glBlitFramebuffer fail");
		return false;
	}
	glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)wglGetProcAddress("glCheckFramebufferStatus");
	if(!glCheckFramebufferStatus)
	{
		PRINT_ERROR("EXT: glCheckFramebufferStatus fail");
		return false;
	}

	glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)wglGetProcAddress("glDeleteFramebuffers");
	if(!glDeleteFramebuffers)
	{
		PRINT_ERROR("EXT: glDeleteFramebuffers fail");
		return false;
	}

	glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)wglGetProcAddress("glDeleteRenderbuffers");
	if(!glDeleteRenderbuffers)
	{
		PRINT_ERROR("EXT: glDeleteRenderbuffers fail");
		return false;
	}

	glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)wglGetProcAddress("glFramebufferRenderbuffer");
	if(!glFramebufferRenderbuffer)
	{
		PRINT_ERROR("EXT: glFramebufferRenderbuffer fail");
		return false;
	}

	glFramebufferTexture1D = (PFNGLFRAMEBUFFERTEXTURE1DPROC)wglGetProcAddress("glFramebufferTexture1D");
	if(!glFramebufferTexture1D)
	{
		PRINT_ERROR("EXT: glFramebufferTexture1D fail");
		return false;
	}
	glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)wglGetProcAddress("glFramebufferTexture2D");
	if(!glFramebufferTexture2D)
	{
		PRINT_ERROR("EXT: glFramebufferTexture2D fail");
		return false;
	}
	glFramebufferTexture3D = (PFNGLFRAMEBUFFERTEXTURE3DPROC)wglGetProcAddress("glFramebufferTexture3D");
	if(!glFramebufferTexture3D)
	{
		PRINT_ERROR("EXT: glFramebufferTexture3D fail");
		return false;
	}
	
	glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)wglGetProcAddress("glGenFramebuffers");
	if(!glGenFramebuffers)
	{
		PRINT_ERROR("EXT: glGenFramebuffers fail");
		return false;
	}
	glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)wglGetProcAddress("glGenRenderbuffers");
	if(!glGenRenderbuffers)
	{
		PRINT_ERROR("EXT: glGenRenderbuffers fail");
		return false;
	}
	
	glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)wglGetProcAddress("glGetFramebufferAttachmentParameteriv");
	if(!glGetFramebufferAttachmentParameteriv)
	{
		PRINT_ERROR("EXT: glGetFramebufferAttachmentParameteriv fail");
		return false;
	}
	glGetRenderbufferParameteriv = (PFNGLGETRENDERBUFFERPARAMETERIVPROC)wglGetProcAddress("glGetRenderbufferParameteriv");
	if(!glGetRenderbufferParameteriv)
	{
		PRINT_ERROR("EXT: glGetRenderbufferParameteriv fail");
		return false;
	}
	glIsFramebuffer = (PFNGLISFRAMEBUFFERPROC)wglGetProcAddress("glIsFramebuffer");
	if(!glIsFramebuffer)
	{
		PRINT_ERROR("EXT: glIsFramebuffer fail");
		return false;
	}
	glIsRenderbuffer = (PFNGLISRENDERBUFFERPROC)wglGetProcAddress("glIsRenderbuffer");
	if(!glIsRenderbuffer)
	{
		PRINT_ERROR("EXT: glIsRenderbuffer fail");
		return false;
	}
	glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)wglGetProcAddress("glRenderbufferStorage");
	if(!glRenderbufferStorage)
	{
		PRINT_ERROR("EXT: glRenderbufferStorage fail");
		return false;
	}
	glRenderbufferStorageMultisample = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC)wglGetProcAddress("glRenderbufferStorageMultisample");
	if(!glRenderbufferStorageMultisample)
	{
		PRINT_ERROR("EXT: glRenderbufferStorageMultisample fail");
		return false;
	}

	glDrawBuffers  = (PFNGLDRAWBUFFERSPROC)wglGetProcAddress("glDrawBuffers");
	if(!glDrawBuffers)
	{
		PRINT_ERROR("EXT: glDrawBuffers fail");
		return false;
	}
	glTexImage2DMultisample  = (PFNGLTEXIMAGE2DMULTISAMPLEPROC)wglGetProcAddress("glTexImage2DMultisample");
	if(!glTexImage2DMultisample)
	{
		PRINT_ERROR("EXT: glTexImage2DMultisample fail");
		return false;
	}

	glBlendEquation = (PFNGLBLENDEQUATIONPROC)wglGetProcAddress("glBlendEquation");
	if(!glBlendEquation)
	{
		PRINT_ERROR("EXT: glBlendEquation fail");
		return false;
	}

	/*glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEEXTPROC)wglGetProcAddress("glBlendFuncSeparateEXT");
	if(!glBlendFuncSeparate)
	{
		PRINT_ERROR("EXT: glBlendEquation fail");
		return false;
	}*/

	PRINT_INFO("EXT: OpenGL extensinos were loaded.");

	return true;
}
