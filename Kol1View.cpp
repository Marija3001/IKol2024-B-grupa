// Kol1View.cpp : implementation of the CKol1View class  (RG 2024 I kolokvijum - B grupa, ringispil)
#include "pch.h"
#include "framework.h"
#ifndef SHARED_HANDLERS
#include "Kol1.h"
#endif

#include "Kol1Doc.h"
#include "Kol1View.h"
#define _USE_MATH_DEFINES
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define BASKET_R    50     // 11. poluprecnik korpe
#define ARM_LEN     182    // 11. rastojanje izmedju osovina drzaca
#define ARM_AXIS    10.0f  // 6. veca osovina drzaca je u tacki (10, 10) slike Arm.png
#define BASKET_SCALE 0.675f
#define STEP_MOVE   10.0   // korak za Q / W
#define STEP_ANGLE  10.0   // korak uglova (stepeni)

IMPLEMENT_DYNCREATE(CKol1View, CView)

BEGIN_MESSAGE_MAP(CKol1View, CView)
	ON_WM_ERASEBKGND()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

// ---------------------------------------------------------------
// 1. Ucitavanje slika u konstruktoru, oslobadjanje u destruktoru
// ---------------------------------------------------------------
CKol1View::CKol1View() noexcept
{
	m_pBackground = new DImage();
	m_pBackground->Load(CString("Background.jpg"));

	m_pBasket = new DImage();
	m_pBasket->Load(CString("Basket.png"));

	m_pArm = new DImage();
	m_pArm->Load(CString("Arm.png"));

	m_offset = 0.0;
	m_alpha = 0.0;
	m_beta = 0.0;
	m_angle = 0.0;
}

CKol1View::~CKol1View()
{
	delete m_pBackground;
	delete m_pBasket;
	delete m_pArm;
}

BOOL CKol1View::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}

// ---------------------------------------------------------------
// 2. Crtanje bez treperenja (memorijski DC)
// ---------------------------------------------------------------
BOOL CKol1View::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void CKol1View::OnDraw(CDC* pDC)
{
	CKol1Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	CRect rcClient;
	GetClientRect(&rcClient);

	CDC memDC;
	memDC.CreateCompatibleDC(pDC);
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(pDC, rcClient.Width(), rcClient.Height());
	CBitmap* pOldBmp = memDC.SelectObject(&bmp);

	memDC.FillSolidRect(rcClient, RGB(255, 255, 255));
	DrawBackground(&memDC);

	int oldMode = memDC.SetGraphicsMode(GM_ADVANCED);
	XFORM oldForm;
	memDC.GetWorldTransform(&oldForm);

	// 11. horizontalno u centru, centar pocetne korpe za precnik iznad donje ivice
	Translate(&memDC, rcClient.Width() / 2.0f, (float)(rcClient.Height() - 2 * BASKET_R), false);
	DrawCarousel(&memDC, ARM_LEN, BASKET_R, m_offset, m_alpha, m_beta, m_angle);

	memDC.SetWorldTransform(&oldForm);
	memDC.SetGraphicsMode(oldMode);

	pDC->BitBlt(0, 0, rcClient.Width(), rcClient.Height(), &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(pOldBmp);
}

// ---------------------------------------------------------------
// 3. Pozadina u originalnoj velicini: donja ivica = donja ivica prozora,
//    centri slike i prozora poklopljeni po horizontali
// ---------------------------------------------------------------
void CKol1View::DrawBackground(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(&rcClient);

	int w = m_pBackground->Width();
	int h = m_pBackground->Height();
	int x = (rcClient.Width() - w) / 2;
	int y = rcClient.Height() - h;

	m_pBackground->Draw(pDC, CRect(0, 0, w, h), CRect(x, y, x + w, y + h));
}

// ---------------------------------------------------------------
// 4. Transparentno crtanje - boja pozadine = boja prvog piksela
// ---------------------------------------------------------------
void CKol1View::DrawImageTransparent(CDC* pDC, DImage* pImage)
{
	unsigned char* bits = pImage->GetDIBBits();
	COLORREF bgColor = RGB(bits[2], bits[1], bits[0]);   // u memoriji je BGR(A)

	CRect rc(0, 0, pImage->Width(), pImage->Height());
	pImage->DrawTransparent(pDC, rc, rc, bgColor);
}

// ---------------------------------------------------------------
// 5. Transformacije
//    rightMultiply = false -> MWT_LEFTMULTIPLY  (u lokalnom k.s.)
//    rightMultiply = true  -> MWT_RIGHTMULTIPLY (u globalnom k.s.)
// ---------------------------------------------------------------
void CKol1View::Translate(CDC* pDC, float dX, float dY, bool rightMultiply)
{
	XFORM form = { 1.0f, 0.0f, 0.0f, 1.0f, dX, dY };
	pDC->ModifyWorldTransform(&form, rightMultiply ? MWT_RIGHTMULTIPLY : MWT_LEFTMULTIPLY);
}

void CKol1View::Rotate(CDC* pDC, float angle, bool rightMultiply)
{
	float rad = (float)(angle * M_PI / 180.0);
	XFORM form = { cosf(rad), sinf(rad), -sinf(rad), cosf(rad), 0.0f, 0.0f };
	pDC->ModifyWorldTransform(&form, rightMultiply ? MWT_RIGHTMULTIPLY : MWT_LEFTMULTIPLY);
}

void CKol1View::Scale(CDC* pDC, float sX, float sY, bool rightMultiply)
{
	XFORM form = { sX, 0.0f, 0.0f, sY, 0.0f, 0.0f };
	pDC->ModifyWorldTransform(&form, rightMultiply ? MWT_RIGHTMULTIPLY : MWT_LEFTMULTIPLY);
}

// ---------------------------------------------------------------
// 6. Drzac: tacka (10, 10) slike Arm.png je u koordinatnom pocetku.
//    Arm.png je uspravna (20x200), pa je manja osovina u (0, 182) lokalno.
// ---------------------------------------------------------------
void CKol1View::DrawArm(CDC* pDC)
{
	XFORM oldForm;
	pDC->GetWorldTransform(&oldForm);

	Translate(pDC, -ARM_AXIS, -ARM_AXIS, false);
	DrawImageTransparent(pDC, m_pArm);

	pDC->SetWorldTransform(&oldForm);
}

// ---------------------------------------------------------------
// 7. Korpa: 67.5% velicine, centar u koord. pocetku,
//    tekst "R_G" centriran, nagnut -60 stepeni, Verdana bold 0.9*r
// ---------------------------------------------------------------
void CKol1View::DrawBasket(CDC* pDC, int r)
{
	XFORM oldForm;
	pDC->GetWorldTransform(&oldForm);

	Scale(pDC, BASKET_SCALE, BASKET_SCALE, false);
	Translate(pDC, -m_pBasket->Width() / 2.0f, -m_pBasket->Height() / 2.0f, false);
	DrawImageTransparent(pDC, m_pBasket);
	pDC->SetWorldTransform(&oldForm);

	CFont font;
	font.CreateFont((int)(0.9 * r), 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _T("Verdana"));
	CFont* pOldFont = pDC->SelectObject(&font);
	int oldBkMode = pDC->SetBkMode(TRANSPARENT);
	COLORREF oldColor = pDC->SetTextColor(RGB(0, 0, 139));   // tamno plava
	UINT oldAlign = pDC->SetTextAlign(TA_CENTER | TA_BASELINE);

	// -60 stepeni: tekst se strmo penje udesno (y osa je okrenuta nadole)
	Rotate(pDC, -60.0f, false);

	// TA_CENTER centrira po x; po y spustamo baseline za pola visine slova
	TEXTMETRIC tm;
	pDC->GetTextMetrics(&tm);
	pDC->TextOut(0, (tm.tmAscent - tm.tmDescent) / 2, CString("R_G"));

	pDC->SetWorldTransform(&oldForm);
	pDC->SetTextAlign(oldAlign);
	pDC->SetTextColor(oldColor);
	pDC->SetBkMode(oldBkMode);
	pDC->SelectObject(pOldFont);
}

// ---------------------------------------------------------------
// 8. Par korpi povezanih drzacem.
//    Ceo par se okrece oko ose prve korpe za angle:
//    - drzac je vecom osovinom u (0, 0.8r) prve korpe (uspravno, ispod centra)
//      i lezi vodoravno,
//    - manjom osovinom (l, 0.8r) je u tacki (0, 0.8r) druge korpe,
//      pa je centar druge korpe u (l, 0) i ona prati rotaciju prve.
// ---------------------------------------------------------------
void CKol1View::DrawBasketCouple(CDC* pDC, int l, int r, float angle)
{
	XFORM oldForm;
	pDC->GetWorldTransform(&oldForm);

	Rotate(pDC, angle, false);
	XFORM coupleForm;
	pDC->GetWorldTransform(&coupleForm);

	DrawBasket(pDC, r);                       // prva korpa

	Translate(pDC, (float)l, 0.0f, false);
	DrawBasket(pDC, r);                       // druga korpa
	pDC->SetWorldTransform(&coupleForm);

	Translate(pDC, 0.0f, 0.8f * r, false);    // veca osovina drzaca
	Rotate(pDC, -90.0f, false);               // uspravna slika -> vodoravno udesno
	DrawArm(pDC);

	pDC->SetWorldTransform(&oldForm);
}

// ---------------------------------------------------------------
// 9. Platforma: centralna korpa (angle) + 4 para na 45, 135, 225, 315 stepeni
//    (-angle). Prva korpa svakog para dodiruje centralnu (centar na 2r).
// ---------------------------------------------------------------
void CKol1View::DrawPlatform(CDC* pDC, int l, int r, double angle)
{
	XFORM oldForm;
	pDC->GetWorldTransform(&oldForm);

	Rotate(pDC, (float)angle, false);
	DrawBasket(pDC, r);
	pDC->SetWorldTransform(&oldForm);

	for (int i = 0; i < 4; i++)
	{
		Rotate(pDC, 45.0f + i * 90.0f, false);
		Translate(pDC, 2.0f * r, 0.0f, false);
		DrawBasketCouple(pDC, l, r, (float)-angle);
		pDC->SetWorldTransform(&oldForm);
	}
}

// ---------------------------------------------------------------
// 10. Ringispil
//     Pocetna korpa se kotrlja: predjeni put offset = r * ugao (rad).
//     Prvi drzac: veca osovina u centru pocetne korpe, uspravan (180) + alpha.
//     Drugi drzac: veca osovina na manjoj osovini prvog (0, h), + beta.
//     Platforma: na manjoj osovini drugog, ugao angle.
// ---------------------------------------------------------------
void CKol1View::DrawCarousel(CDC* pDC, int h, int r, double offset, double alpha, double beta, double angle)
{
	XFORM oldForm;
	pDC->GetWorldTransform(&oldForm);

	Translate(pDC, (float)offset, 0.0f, false);
	XFORM baseForm;
	pDC->GetWorldTransform(&baseForm);

	// pocetna korpa
	Rotate(pDC, (float)(offset / r * 180.0 / M_PI), false);
	DrawBasket(pDC, r);
	pDC->SetWorldTransform(&baseForm);

	// prvi drzac (slika gleda nadole, 180 je okrece nagore)
	Rotate(pDC, (float)(180.0 + alpha), false);
	XFORM arm1Form;
	pDC->GetWorldTransform(&arm1Form);

	// drugi drzac
	Translate(pDC, 0.0f, (float)h, false);
	Rotate(pDC, (float)beta, false);
	XFORM arm2Form;
	pDC->GetWorldTransform(&arm2Form);

	// platforma - vracamo je u uspravan polozaj, pa je okrece samo angle
	Translate(pDC, 0.0f, (float)h, false);
	Rotate(pDC, (float)-(180.0 + alpha + beta), false);
	DrawPlatform(pDC, h, r, angle);

	// drzaci preko korpi
	pDC->SetWorldTransform(&arm2Form);
	DrawArm(pDC);
	pDC->SetWorldTransform(&arm1Form);
	DrawArm(pDC);

	pDC->SetWorldTransform(&oldForm);
}

// ---------------------------------------------------------------
// 11. Tasteri: Q/W offset, A/S alpha, D/F beta, E/R angle
// ---------------------------------------------------------------
void CKol1View::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar)
	{
	case 'Q': m_offset -= STEP_MOVE;  break;
	case 'W': m_offset += STEP_MOVE;  break;
	case 'A': m_alpha  -= STEP_ANGLE; break;
	case 'S': m_alpha  += STEP_ANGLE; break;
	case 'D': m_beta   -= STEP_ANGLE; break;
	case 'F': m_beta   += STEP_ANGLE; break;
	case 'E': m_angle  -= STEP_ANGLE; break;
	case 'R': m_angle  += STEP_ANGLE; break;
	default:
		CView::OnKeyDown(nChar, nRepCnt, nFlags);
		return;
	}

	Invalidate();
}
