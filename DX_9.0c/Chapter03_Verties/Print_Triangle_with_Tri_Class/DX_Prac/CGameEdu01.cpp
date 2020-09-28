#include "CGameEdu01.h"

CGameEdu01::CGameEdu01()
{
	
}

CGameEdu01::~CGameEdu01()
{

}

void CGameEdu01::OnInit()
{
	m_Triangle.OnInit(m_pd3dDevices);
}

void CGameEdu01::OnRender()
{
	m_Triangle.OnRender();
}

void CGameEdu01::OnUpdate()
{

}

void CGameEdu01::OnRelease()
{
	m_Triangle.OnRelease();
}