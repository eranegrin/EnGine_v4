#include "FreeType.h"

#define MAXWIDTH 1280
#define PADDING 1
//#define TEST_DOUBLE 1

FreeTypeEng::FreeTypeEng()
{
	m_pProgram = NULL;
	m_arrVerts = NULL;
	m_attribute_coord = -1;
	m_uniform_tex = -1;
	m_uniform_color = -1;
	m_vboData.m_uiVertexArrayID = 0;
 	m_vboData.m_uiVertexBufferID = 0;
 	m_vboData.m_uiIndexBufferID = 0;

}

FreeTypeEng::~FreeTypeEng()
{
	// Disable the two vertex array attributes.
	m_pGL->glDisableVertexAttribArray(0);
	
	// Release the vertex buffer.
	m_pGL->glBindBuffer(GL_ARRAY_BUFFER, 0);
	m_pGL->glDeleteBuffers(1, &m_vboData.m_uiVertexBufferID);

	// Release the index buffer.
	m_pGL->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	m_pGL->glDeleteBuffers(1, &m_vboData.m_uiIndexBufferID);

	// Release the vertex array object.
	m_pGL->glBindVertexArray(0);
	m_pGL->glDeleteVertexArrays(1, &m_vboData.m_uiVertexArrayID);

	// Release all texture objects
	for (UINT i = 0; i < m_arrfonts.size(); i++)
		glDeleteTextures(1, &m_arrfonts[i].m_uiTexId);

	delete m_arrVerts;
	PRINT_INFO("Font VAO was released");
}

BOOL FreeTypeEng::InitFont(OGL4* pGL, CChunk* chFont )
{
	m_pGL = pGL;
	int error = FT_Init_FreeType( &library );
	if ( error )
	{
		return FALSE;
	}
	
	if (m_pProgram)
	{
		m_attribute_coord	= m_pProgram->GetAttribLocation("coord");
		m_uniform_tex		= m_pProgram->GetUniformLocation("sampler01");// m_pGL->glGetUniformLocation(m_pProgram->GetID(), "tex");
		m_uniform_color		= m_pProgram->GetUniformLocation("color");// m_pGL->glGetUniformLocation(m_pProgram->GetID(), "tex");
		if(m_attribute_coord == -1 || m_uniform_tex == -1 || m_uniform_color == -1)
			PRINT_WARN("Not all font shader uniforms are available!!!");
	}
	TIC();
	PRINT_INFO("========| Loading Fonts GL");
	string	strFullPath;
	int		iFontSize;
	UINT nFonts = chFont->GetChunckCount("Font");	
	for (UINT uiS = 0; uiS < nFonts; uiS++)
	{
		FontEng fontTest;
		CChunk chF = chFont->GetChunkXML("Font", uiS);
		chF.ReadChildValue("Path", strFullPath);
		string strPath = ENG::FULL_PATH + strFullPath;

		chF.ReadChildValue("Size", iFontSize);
		if (fontTest.InitFont(library, strPath, iFontSize))
		{
			m_arrfonts.push_back(fontTest);
		}
		
	}

	const int strLen = 6 * 500; // 500 chars, each char 2 triangles
	m_arrVerts = new Vertex4[strLen];

	TOC("Finish font loading");
	
	return TRUE;
}
BOOL FreeTypeEng::PrepareFontforRender()
{
	
	const unsigned char *p; 
	int iMinY = 75;
	float sx = 2 * ENG::SCR_WID_INV;
	float sy = 2 * ENG::SCR_HEI_INV;
	int iBufferPosition = 0;
	
	vector<tGLTextInfo>& arrText = GLTEXT->GetTextArray();
	UINT nLines = GLTEXT->GetLineCount();

	for (UINT uiLine = 0; uiLine < nLines; uiLine++)
	{
		const tGLTextInfo& tInfo = arrText[uiLine];
		// Color
		Vec4 vCol = Vec4( RND_COLORS_ARRAY[tInfo.m_eColor][0], RND_COLORS_ARRAY[tInfo.m_eColor][1], RND_COLORS_ARRAY[tInfo.m_eColor][2], 1.0f );
		
		
		float x = tInfo.m_vPos2.x;
		float y = tInfo.m_vPos2.y;

		const FontEng& font = m_arrfonts[tInfo.m_iFontID];

		int iC = 0;
		/* Loop through all characters */
		for (p = (const unsigned char *)tInfo.m_strValue.c_str(); *p; p++)
		{
			/* Calculate the vertex and texture coordinates */
			int ind = static_cast<int>(*p);
			const character_info cInfo = font.m_charsInfo[ind];

			float x2 = (x + cInfo.bl * sx);
			float y2 = (-y + cInfo.bt * sy);
			float w = cInfo.bw * sx;
			float h = cInfo.bh * sy;

			/* Advance the cursor to the start of the next character */
			x += cInfo.ax * sx;
			y += cInfo.ay * sy;

			/* Skip glyphs that have no pixels */
			if (!w || !h)
				continue;
			// tri 0
			m_arrVerts[iBufferPosition + iC + 0] = Vertex4(x2, y2, cInfo.tx, cInfo.ty, vCol);
			m_arrVerts[iBufferPosition + iC + 1] = Vertex4(x2, y2 - h, cInfo.tx, cInfo.ty + cInfo.bh / font.m_iTextureHeight, vCol);
			m_arrVerts[iBufferPosition + iC + 2] = Vertex4(x2 + w, y2, cInfo.tx + cInfo.bw / font.m_iTextureWidth, cInfo.ty, vCol);

			// tri 1
			m_arrVerts[iBufferPosition + iC + 3] = Vertex4(x2 + w, y2, cInfo.tx + cInfo.bw / font.m_iTextureWidth, cInfo.ty, vCol);
			m_arrVerts[iBufferPosition + iC + 4] = Vertex4(x2, y2 - h, cInfo.tx, cInfo.ty + cInfo.bh / font.m_iTextureHeight, vCol);
			m_arrVerts[iBufferPosition + iC + 5] = Vertex4(x2 + w, y2 - h, cInfo.tx + cInfo.bw / font.m_iTextureWidth, cInfo.ty + cInfo.bh / font.m_iTextureHeight, vCol);
			iC += 6;
		}
		iBufferPosition += iC;
	}

	m_iTestTextLen = iBufferPosition;

	m_pGL->glBindBuffer(GL_ARRAY_BUFFER, m_vboData.m_uiVertexBufferID);
	m_pGL->glBufferSubData(GL_ARRAY_BUFFER, 0, m_iTestTextLen * sizeof(Vertex4), m_arrVerts);
	m_pGL->glBindBuffer(GL_ARRAY_BUFFER, 0); 
	return TRUE;
}

VOID FreeTypeEng::Render(int ifontID)
{
	PrepareFontforRender();

	const FontEng& font = m_arrfonts[ifontID];

	
	/* Use the texture containing the atlas */
	m_pGL->glBindBuffer(GL_ARRAY_BUFFER, m_vboData.m_uiVertexBufferID);
	m_pGL->glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, font.m_uiTexId);
	m_pGL->glUniform1i(m_uniform_tex, 0);
	
	GL_ERROR();

	m_pGL->glBindVertexArray(m_vboData.m_uiVertexArrayID);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDrawElements(GL_TRIANGLES, m_iTestTextLen, GL_UNSIGNED_INT, 0);

	glBlendFunc(GL_ONE, GL_ONE);
	glDrawElements(GL_TRIANGLES, m_iTestTextLen, GL_UNSIGNED_INT, 0);


	GL_ERROR();


	m_pGL->glBindBuffer(GL_ARRAY_BUFFER, 0);
	GL_ERROR();
	m_pGL->glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);	
}

//http://www.freetype.org/freetype2/docs/tutorial/step1.html#section-1

BOOL FontEng::InitFont( FT_Library library, const string& strFileName, int iSize )
{	
	int error = FT_New_Face( library, strFileName.c_str(), 0, &face );
	if ( error == FT_Err_Unknown_File_Format )
	{
		PRINT_ERROR("Fail to load TrueType Face, error %d", error);
		return FALSE;
	}
	else if ( error )
	{
		PRINT_ERROR("Fail to load TrueType Face, error %d", error);
		return FALSE;
	}
	
	FT_Set_Pixel_Sizes(face, 0, iSize);
	FT_GlyphSlot g = face->glyph;
	int w = 0;
	int h = 0;
	int roww = 0;
	int rowh = 0;
	memset(m_charsInfo, 0, sizeof m_charsInfo);

	/* Find minimum size for a texture holding all visible ASCII characters */
	for (int i = 32; i < 128; i++) 
	{
		if (FT_Load_Char(face, i, FT_LOAD_RENDER)) 
		{
			PRINT_ERROR("Loading character %c failed!\n", i);
			continue;
		}
		if (roww + g->bitmap.width + PADDING >= MAXWIDTH) 
		{
			w = max(w, roww);
			h += rowh;
			roww = 0;
			rowh = 0;
		}
		roww += g->bitmap.width + PADDING;
		rowh = max(rowh, g->bitmap.rows);
	}

	w = max(w, roww);
	h += rowh;

	m_iTextureWidth = static_cast<float>(w);
	m_iTextureHeight = static_cast<float>(h);

	//pGL->glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &m_uiTexId);
	glBindTexture(GL_TEXTURE_2D, m_uiTexId);
	/* We require 1 byte alignment when uploading texture data */
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
	/* Clamping to edges is important to prevent artifacts when scaling */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	/* Linear filtering usually looks best for text */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	GL_ERROR();
	int ox = 0;
	int oy = 0;

	rowh = 0;

	for (int i = 32; i < 128; i++) 
	{
		if (FT_Load_Char(face, i, FT_LOAD_RENDER)) 
		{
			PRINT_ERROR("Loading character %c failed!\n", i);
			continue;
		}

		if (ox + g->bitmap.width + PADDING >= MAXWIDTH) 
		{
			oy += rowh;
			rowh = 0;
			ox = 0;
		}

		glTexSubImage2D(GL_TEXTURE_2D, 0, ox, oy, g->bitmap.width, g->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer);

		m_charsInfo[i].ax = g->advance.x >> 6;
		m_charsInfo[i].ay = g->advance.y >> 6;

		m_charsInfo[i].bw = g->bitmap.width;
		m_charsInfo[i].bh = g->bitmap.rows;

		m_charsInfo[i].bl = g->bitmap_left;
		m_charsInfo[i].bt = g->bitmap_top;

		m_charsInfo[i].tx = ox / (float)w;
		m_charsInfo[i].ty = oy / (float)h;

		rowh = max(rowh, g->bitmap.rows);
		ox += g->bitmap.width + PADDING;
	}

	PRINT_INFO("Generated a %d x %d (%d kb) texture atlas", w, h, w * h / MAXWIDTH);
	GL_ERROR();
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	return TRUE;
}