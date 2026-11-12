// Kol1View.h : interface of the CKol1View class  (RG 2024 I kolokvijum - B grupa, ringispil)
#pragma once
#include "DImage.h"

class CKol1View : public CView
{
protected:
	CKol1View() noexcept;
	DECLARE_DYNCREATE(CKol1View)

public:
	CKol1Doc* GetDocument() const;

	virtual void OnDraw(CDC* pDC);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual ~CKol1View();

protected:
	// 1. slike
	DImage* m_pBackground;
	DImage* m_pBasket;
	DImage* m_pArm;

	// stanje ringispila
	double m_offset;   // pomeraj pocetne korpe (Q, W)
	double m_alpha;    // ugao prvog drzaca (A, S)
	double m_beta;     // ugao drugog drzaca (D, F)
	double m_angle;    // ugao platforme (E, R)

	// 3., 4.
	void DrawBackground(CDC* pDC);
	void DrawImageTransparent(CDC* pDC, DImage* pImage);

	// 5.
	void Translate(CDC* pDC, float dX, float dY, bool rightMultiply);
	void Rotate(CDC* pDC, float angle, bool rightMultiply);   // angle u stepenima
	void Scale(CDC* pDC, float sX, float sY, bool rightMultiply);

	// 6. - 10.
	void DrawArm(CDC* pDC);
	void DrawBasket(CDC* pDC, int r);
	void DrawBasketCouple(CDC* pDC, int l, int r, float angle);
	void DrawPlatform(CDC* pDC, int l, int r, double angle);
	void DrawCarousel(CDC* pDC, int h, int r, double offset, double alpha, double beta, double angle);

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};

#ifndef _DEBUG
inline CKol1Doc* CKol1View::GetDocument() const
   { return reinterpret_cast<CKol1Doc*>(m_pDocument); }
#endif
