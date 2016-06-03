#include "../EnGBasic/EnGBasic.h"
#include "GLSLProgram.h"
#include <ft2build.h>
#include FT_FREETYPE_H

namespace EnG
{
	struct character_info {
		float ax; // advance.x
		float ay; // advance.y

		float bw; // bitmap.width;
		float bh; // bitmap.rows;

		float bl; // bitmap_left;
		float bt; // bitmap_top;

		float tx; // x offset of glyph in texture coordinates
		float ty; // x offset of glyph in texture coordinates
	} ;
	class FontEng
	{
	public:
		FontEng() {}
		~FontEng(){}

		BOOL InitFont(FT_Library library, const string& strFileName, int iSize);

		FT_Face			face;      /* handle to face object */
		GLuint			m_uiTexId;		// texture object
		float			m_iTextureWidth;
		float			m_iTextureHeight;
		character_info	m_charsInfo[128];

	};

	class FreeTypeEng
	{
	public:
		FreeTypeEng();
		~FreeTypeEng();
		struct tVBO_DATA
		{
			GLuint			m_uiVertexArrayID;
			GLuint			m_uiVertexBufferID;			
			GLuint			m_uiIndexBufferID;
		};

		BOOL InitFont(OGL4* pGL, CChunk* chFont );
		BOOL PrepareFontforRender();
		VOID Render(int ifontID);

		OGL4*			m_pGL;
		FT_Library		library;
		//FT_Face			face;      /* handle to face object */
		//GLuint			m_uiTexId;		// texture object
		GLSLProgram*	m_pProgram;		
		GLint			m_attribute_coord;
		GLint			m_uniform_tex;
		GLint			m_uniform_color;
		tVBO_DATA		m_vboData;
		
		Vertex4*		m_arrVerts;
		int				m_iTestTextLen;
		vector<FontEng>	m_arrfonts;
	};

}