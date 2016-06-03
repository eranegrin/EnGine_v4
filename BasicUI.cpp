#include "BasicUI.h"
#include "..\EnGBasic\MessageMng.h"

CBasicUI::CBasicUI()
{
	m_pGL = NULL;
	m_pRPack = NULL;
	m_iMsgID = -1;
	m_vPos = Vec2(0.0f);
	m_vSize = Vec2(0.0f);
	m_strShaderName = "";
	m_bMouseHover = false;
}


CBasicUI::CBasicUI(OGL4* pGL) : CBasicUI()
{	
	m_pGL = pGL;	
	m_pRPack = NULL;
	m_iMsgID = -1;
	m_vPos = Vec2(0.0f);
	m_vSize = Vec2(0.0f);
	m_strShaderName = "";
	m_bMouseHover = false;
}

CBasicUI::~CBasicUI()
{
	
}

VOID CBasicUI::InitObj(CChunk* a_pRootChunk)
{
	m_pRPack = ResourceMng::GetInstance()->GetDefaultQuadRP();
	// Name
	a_pRootChunk->ReadChildValue("Name", m_strName);
	// texture
	string strTexture = "";
	a_pRootChunk->ReadChildValue("Texture", strTexture);
	if (!strTexture.empty())
	{
		m_uiTexture = ResourceMng::GetInstance()->LoadTexture(strTexture);
	}
	// position
	a_pRootChunk->ReadChildValue("Position", m_vPos);
	// Size
	a_pRootChunk->ReadChildValue("Size", m_vSize);
	// Shader
	a_pRootChunk->ReadChildValue("Shader", m_strShaderName);	


	// Convert sizes to GL coordinates
	m_vSize.x = m_vSize.x * ENG::SCR_WID_INV;
	m_vSize.y = m_vSize.y * ENG::SCR_HEI_INV;

	m_vPos.x = m_vPos.x * ENG::SCR_WID_INV;
	m_vPos.y = 1.0f - m_vPos.y * ENG::SCR_HEI_INV - m_vSize.y;


}

BOOL CBasicUI::Update(float fmx, float fmy, bool bLButtonJustGotUp)
{
	m_bMouseHover = (fmx > m_vPos.x && fmx < m_vPos.x + m_vSize.x && fmy > m_vPos.y && fmy < m_vPos.y + m_vSize.y);
	return FALSE;
}

VOID CBasicUI::Render(const float* fProjMat)
{
	glViewport(static_cast<GLsizei>(ENG::SCR_WIDTH * m_vPos.x), static_cast<GLsizei>(ENG::SCR_HEIGHT * m_vPos.y),
		static_cast<GLsizei>(ENG::SCR_WIDTH * m_vSize.x), static_cast<GLsizei>(ENG::SCR_HEIGHT * m_vSize.y));

	GLSLProgram* pProg = ResourceMng::GetInstance()->GetShader(m_strShaderName);
	// Get shader program
	if (!pProg)
		return;
	pProg->Use();

	UINT unfID = pProg->GetUniformLocation("projectionMatrix");
	if (unfID != -1)
		m_pGL->glUniformMatrix4fv(unfID, 1, GL_FALSE, fProjMat);
	

	// Bind texture
	m_pGL->glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_uiTexture);
	unfID = pProg->GetUniformLocation("sampler01");
	if (unfID != -1)
		m_pGL->glUniform1i(unfID, 0);

	const PRenderPack pPack = m_pRPack;

	// Bind the vertex array object that stored all the information about the vertex and index buffers.
	m_pGL->glBindVertexArray(pPack->m_uiVertexArrayID);

	// Render the vertex buffer using the index buffer.
	glDrawElements(GL_TRIANGLES, pPack->m_uiLength, GL_UNSIGNED_INT, 0);

	// Unbind the buffer
	m_pGL->glBindVertexArray(0);

	m_pGL->glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	pProg->Disable();
}

CCheckBoxUI::CCheckBoxUI() : CBasicUI()
{
	m_eState = btnRegular;
	m_bWaitforButtonUp = false;
	m_bOn = false;
	m_bIsPleaseExit = false;
	
}

CCheckBoxUI::CCheckBoxUI(OGL4* pGL) : CBasicUI(pGL)
{
	m_eState = btnRegular;
	m_bWaitforButtonUp = false;
	m_bOn = false;
	m_bIsPleaseExit = false;
}

CCheckBoxUI::~CCheckBoxUI()
{

}

VOID CCheckBoxUI::InitObj(CChunk* a_pRootChunk)
{
	CBasicUI::InitObj(a_pRootChunk);
	
	a_pRootChunk->ReadChildValue("MsgID", m_iMsgID);

	bool bchecked = false;
	a_pRootChunk->ReadChildValue("Checked", bchecked);
	if (bchecked)
	{
		SetState(btnDisable);
	}

}

BOOL CCheckBoxUI::Update(float fmx, float fmy, bool bLButtonJustGotUp)
{
	CBasicUI::Update(fmx, fmy, bLButtonJustGotUp);

	if (m_bIsPleaseExit)
	{
		if (!m_bMouseHover)
		{
			m_bIsPleaseExit = false;
		}
		return FALSE;
	}
	if (!m_bOn)
	{

		if (m_eState == btnClick && bLButtonJustGotUp)
		{
			SetState(btnDisable);
			// On Event!!!
			return TRUE;
		}
		if (m_bMouseHover)
		{
			if (m_eState == btnRegular)
				m_bWaitforButtonUp = ENG::MOUSE_LBUTTON;
			m_eState = btnHover;
		}
		else
		{
			m_eState = btnRegular;
		}
		if (!ENG::MOUSE_LBUTTON)
			m_bWaitforButtonUp = false;
		if (m_eState == btnHover && ENG::MOUSE_LBUTTON && !m_bWaitforButtonUp)
		{
			m_eState = btnClick;
		}

	}
	else
	{
		if (m_eState == btnHover && bLButtonJustGotUp)
		{
			SetState(btnRegular);
			// Off Event!!!
			return FALSE;
		}
		if (m_bMouseHover)
		{
			if (m_eState == btnDisable)
				m_bWaitforButtonUp = ENG::MOUSE_LBUTTON;
			m_eState = btnClick;
		}
		else
		{
			m_eState = btnDisable;
		}
		if (!ENG::MOUSE_LBUTTON)
			m_bWaitforButtonUp = false;
		if (m_eState == btnClick && ENG::MOUSE_LBUTTON && !m_bWaitforButtonUp)
		{
			m_eState = btnHover;
		}

	}
	return FALSE;
}

VOID CCheckBoxUI::Render(const float* fProjMat)
{
	glViewport(static_cast<GLsizei>(ENG::SCR_WIDTH * m_vPos.x), static_cast<GLsizei>(ENG::SCR_HEIGHT * m_vPos.y),
		static_cast<GLsizei>(ENG::SCR_WIDTH * m_vSize.x), static_cast<GLsizei>(ENG::SCR_HEIGHT * m_vSize.y));

	GLSLProgram* pProg = ResourceMng::GetInstance()->GetShader(m_strShaderName);
	// Get shader program
	if (!pProg)
		return;
	pProg->Use();

	UINT unfID = pProg->GetUniformLocation("projectionMatrix");
	if (unfID != -1)
		m_pGL->glUniformMatrix4fv(unfID, 1, GL_FALSE, fProjMat);

	// Screen Data
	float vData[4] = { (float)m_eState, 0.0f, 0.0f, 0.0f };
	unfID = pProg->GetUniformLocation("vData");
	if (unfID != -1)
		m_pGL->glUniform4fv(unfID, 1, vData);

	// Bind texture
	m_pGL->glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_uiTexture);
	unfID = pProg->GetUniformLocation("sampler01");
	if (unfID != -1)
		m_pGL->glUniform1i(unfID, 0);

	const PRenderPack pPack = m_pRPack;

	// Bind the vertex array object that stored all the information about the vertex and index buffers.
	m_pGL->glBindVertexArray(pPack->m_uiVertexArrayID);

	// Render the vertex buffer using the index buffer.
	glDrawElements(GL_TRIANGLES, pPack->m_uiLength, GL_UNSIGNED_INT, 0);

	// Unbind the buffer
	m_pGL->glBindVertexArray(0);

	m_pGL->glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	pProg->Disable();
}

VOID CCheckBoxUI::SetState(eButtonState eState)
{
	if (eState == btnDisable)
	{
		m_eState = btnDisable;
		m_bOn = true;
		m_bIsPleaseExit = true;
		if (m_iMsgID > 0)
		{
			BasicMsg msg(static_cast<engMessages>(m_iMsgID), NULL);
			MessageMng::MsgMng()->CallSubscribers(&msg, this);
		}
	}
	else if (eState == btnRegular)
	{
		m_eState = btnRegular;
		m_bOn = false;
		m_bIsPleaseExit = true;
	}
}

CRadioButtonUI::CRadioButtonUI() : CCheckBoxUI()
{

}

CRadioButtonUI::CRadioButtonUI(OGL4* pGL) : CCheckBoxUI(pGL)
{

}

CRadioButtonUI::~CRadioButtonUI()
{

}

BOOL CRadioButtonUI::Update(float fmx, float fmy, bool bLButtonJustGotUp)
{
	CBasicUI::Update(fmx, fmy, bLButtonJustGotUp);
	if (m_bIsPleaseExit)
	{
		if (!m_bMouseHover)
		{
			m_bIsPleaseExit = false;
		}
		return FALSE;
	}
	if (!m_bOn)
	{

		if (m_eState == btnClick && bLButtonJustGotUp)
		{
			SetState(btnDisable);
			// On Event!!!
			return TRUE;
		}
		if (m_bMouseHover)
		{
			if (m_eState == btnRegular)
				m_bWaitforButtonUp = ENG::MOUSE_LBUTTON;
			m_eState = btnHover;
		}
		else
		{
			m_eState = btnRegular;
		}
		if (!ENG::MOUSE_LBUTTON)
			m_bWaitforButtonUp = false;
		if (m_eState == btnHover && ENG::MOUSE_LBUTTON && !m_bWaitforButtonUp)
		{
			m_eState = btnClick;
		}

	}
	return FALSE;

}

//////////////////////////////////////////////////////////////////////////

CButtonUI::CButtonUI()
{

}

CButtonUI::CButtonUI(OGL4* pGL) : CCheckBoxUI(pGL)
{

}

CButtonUI::~CButtonUI()
{

}

BOOL CButtonUI::Update(float fmx, float fmy, bool bLButtonJustGotUp)
{
	CBasicUI::Update(fmx, fmy, bLButtonJustGotUp);

	if (m_bIsPleaseExit)
	{
		if (!m_bMouseHover)
		{
			m_bIsPleaseExit = false;
		}
		return FALSE;
	}
	if (m_eState == btnClick && bLButtonJustGotUp)
	{
		// On Event!!!
		SetState(btnDisable);
		return TRUE;
	}
	if (m_bMouseHover)
	{
		if (m_eState == btnRegular)
			m_bWaitforButtonUp = ENG::MOUSE_LBUTTON;
		m_eState = btnHover;
	}
	else
	{
		m_eState = btnRegular;
	}
	if (!ENG::MOUSE_LBUTTON)
		m_bWaitforButtonUp = false;
	if (m_eState == btnHover && ENG::MOUSE_LBUTTON && !m_bWaitforButtonUp)
	{
		m_eState = btnClick;
	}

	return FALSE;
}

VOID CButtonUI::SetState(eButtonState eState)
{
	if (eState == btnDisable)
	{
		m_eState = btnRegular;
		m_bIsPleaseExit = true;
		if (m_iMsgID > 0)
		{
			BasicMsg msg(static_cast<engMessages>(m_iMsgID), NULL);
			MessageMng::MsgMng()->CallSubscribers(&msg, this);
		}
	}
}

//////////////////////////////////////////////////////////////////////////

CScrollBarUI::CScrollBarUI() : CBasicUI()
{
	m_fValue = 0.5f;
	m_fStep = 0.03f;
	m_fMousePos = 0.0f;
}

CScrollBarUI::CScrollBarUI(OGL4* pGL) : CBasicUI(pGL)
{
	m_fValue = 0.5f;
	m_fStep = 0.03f;
	m_fMousePos = 0.0f;
}

CScrollBarUI::~CScrollBarUI()
{

}

VOID CScrollBarUI::InitObj(CChunk* a_pRootChunk)
{
	CBasicUI::InitObj(a_pRootChunk);
	a_pRootChunk->ReadChildValue("Step", m_fStep);
	a_pRootChunk->ReadChildValue("Value", m_fValue);

}

BOOL CScrollBarUI::Update(float fmx, float fmy, bool bLButtonJustGotUp)
{
	BOOL bRes = CBasicUI::Update(fmx, fmy, bLButtonJustGotUp);
	m_fMousePos = 0.0f; 
	if (m_bMouseHover)
	{
		if (fmx < (m_vPos.x + m_vSize.x * 0.125f))
		{
			// Left
			m_fMousePos = -1.0f;
			if (bLButtonJustGotUp)
			{
				ValueChanged(m_fValue - m_fStep);
			}
		}
		else if (fmx > (m_vPos.x + m_vSize.x * 0.875f))
		{
			// Right
			m_fMousePos = 1.0f;
			if (bLButtonJustGotUp)
			{
				ValueChanged(m_fValue + m_fStep);				
			}
		}
		else if (ENG::MOUSE_LBUTTON)
		{ 
			// Middle
			float ff = (fmx - m_vPos.x) / m_vSize.x;
			ValueChanged(ff);
		}
	}
	return bRes;
}

VOID CScrollBarUI::Render(const float* fProjMat)
{
	glViewport(static_cast<GLsizei>(ENG::SCR_WIDTH * m_vPos.x), static_cast<GLsizei>(ENG::SCR_HEIGHT * m_vPos.y),
		static_cast<GLsizei>(ENG::SCR_WIDTH * m_vSize.x), static_cast<GLsizei>(ENG::SCR_HEIGHT * m_vSize.y));

	GLSLProgram* pProg = ResourceMng::GetInstance()->GetShader(m_strShaderName);
	// Get shader program
	if (!pProg)
		return;
	pProg->Use();

	UINT unfID = pProg->GetUniformLocation("projectionMatrix");
	if (unfID != -1)
		m_pGL->glUniformMatrix4fv(unfID, 1, GL_FALSE, fProjMat);

	// Screen Data
	float vData[4] = { m_fMousePos, m_fValue * 0.75f + 0.125f, m_fStep, 0.0f };
	unfID = pProg->GetUniformLocation("vData");
	if (unfID != -1)
		m_pGL->glUniform4fv(unfID, 1, vData);

	// Bind texture
	m_pGL->glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_uiTexture);
	unfID = pProg->GetUniformLocation("sampler01");
	if (unfID != -1)
		m_pGL->glUniform1i(unfID, 0);

	const PRenderPack pPack = m_pRPack;

	// Bind the vertex array object that stored all the information about the vertex and index buffers.
	m_pGL->glBindVertexArray(pPack->m_uiVertexArrayID);

	// Render the vertex buffer using the index buffer.
	glDrawElements(GL_TRIANGLES, pPack->m_uiLength, GL_UNSIGNED_INT, 0);

	// Unbind the buffer
	m_pGL->glBindVertexArray(0);

	m_pGL->glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	pProg->Disable();
}

VOID CScrollBarUI::ValueChanged(float fNewVal)
{
	m_fValue = fNewVal;
	SETMAX(m_fValue, m_fStep);
	SETMIN(m_fValue, 1.0f - m_fStep);
	if (m_iMsgID > 0)
	{
		BasicMsg msg(static_cast<engMessages>(m_iMsgID), &m_fValue);
		MessageMng::MsgMng()->CallSubscribers(&msg, this);
	}
}
