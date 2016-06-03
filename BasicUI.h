#pragma once

#include "ResourceMng.h"

enum eButtonState
{
	btnRegular = 0,
	btnHover = 1,
	btnClick = 2,
	btnDisable = 3,
};

class CBasicUI
{
public:
	CBasicUI();
	CBasicUI(OGL4* pGL);
	virtual ~CBasicUI();

	virtual VOID		InitObj(CChunk* a_pRootChunk);
	virtual BOOL		Update(float fmx, float fmy, bool bLButtonJustGotUp);
	virtual VOID		Render(const float* fProjMat);

	OGL4*			m_pGL;
	PRenderPack		m_pRPack;
	GLuint			m_uiTexture;
	string			m_strName;
	string			m_strShaderName;
	Vec2			m_vPos;
	Vec2			m_vSize;	
	bool			m_bMouseHover;
	int				m_iMsgID;

};
typedef CBasicUI* PBasicUI;


class CCheckBoxUI : public CBasicUI
{
public:
	CCheckBoxUI();
	CCheckBoxUI(OGL4* pGL);
	virtual ~CCheckBoxUI();

	
	virtual VOID		InitObj(CChunk* a_pRootChunk);
	virtual BOOL		Update(float fmx, float fmy, bool bLButtonJustGotUp);
	virtual VOID		Render(const float* fProjMat);
	virtual VOID		SetState(eButtonState eState);
protected:

	bool		m_bIsPleaseExit;
	bool		m_bOn;
	bool		m_bWaitforButtonUp;
	eButtonState m_eState;
};
typedef CCheckBoxUI* PCheckBoxUI;

class CButtonUI : public CCheckBoxUI
{
public:
	CButtonUI();
	CButtonUI(OGL4* pGL);
	virtual ~CButtonUI();

	virtual BOOL		Update(float fmx, float fmy, bool bLButtonJustGotUp);
	virtual VOID		SetState(eButtonState eState);
};
typedef CButtonUI* PButtonUI;

class CRadioButtonUI : public CCheckBoxUI
{
public:
	CRadioButtonUI();
	CRadioButtonUI(OGL4* pGL);
	virtual ~CRadioButtonUI();

	virtual BOOL		Update(float fmx, float fmy, bool bLButtonJustGotUp);
};
typedef CRadioButtonUI* PRadioButtonUI;

class CScrollBarUI : public CBasicUI
{
public:
	CScrollBarUI();
	CScrollBarUI(OGL4* pGL);
	virtual ~CScrollBarUI();

	virtual VOID		InitObj(CChunk* a_pRootChunk);
	virtual BOOL		Update(float fmx, float fmy, bool bLButtonJustGotUp);
	virtual VOID		Render(const float* fProjMat);

	float				m_fValue;
	float				m_fStep;
	float				m_fMousePos; // -1 = none, 0 = middle, 1 = left, 2 = right
private:
	VOID				ValueChanged(float fNewVal);
};
typedef CScrollBarUI * PScrollBarUI;
















inline CBasicUI* AllocateIconType(const string& strTypr, OGL4* pGL)
{
	if (strTypr == "CheckBox")
		return new CCheckBoxUI(pGL);
	if (strTypr == "Button")
		return new CButtonUI(pGL);
	if (strTypr == "RadioButton")
		return new CRadioButtonUI(pGL);
	if (strTypr == "ScrollBar")
		return new CScrollBarUI(pGL);
	else
		return new CBasicUI(pGL);

}
