
// wndtermprojectView.h: CwndtermprojectView 클래스의 인터페이스
//

#pragma once
#include "CWEIGHT.h"
#include "CNPPL.h"

extern CNPPL dlg_ppl;
extern CWEIGHT dlg_weight;

class CwndtermprojectView : public CView
{
protected: // serialization에서만 만들어집니다.
	CwndtermprojectView() noexcept;
	DECLARE_DYNCREATE(CwndtermprojectView)

// 특성입니다.
public:
	CwndtermprojectDoc* GetDocument() const;

// 작업입니다.
public:
	POINT rope_lu;
	POINT rope_rd;
	int ppl_cnt;
	int game_round;
	int teamL_win;
	int teamR_win;
	int winner_rep[3];
	int weight[9];
	int left_flag_rx;
	int right_flag_lx;
	int teamL_pull;
	int teamR_pull;
	int random;
	int teamL_penalty;
	int teamR_penalty;
	int circle_lx, circle_rx, circle_uy, circle_dy;


// 재정의입니다.
public:
	virtual void OnDraw(CDC* pDC);  // 이 뷰를 그리기 위해 재정의되었습니다.
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 구현입니다.
public:
	virtual ~CwndtermprojectView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 생성된 메시지 맵 함수
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEditInputPpl();
	afx_msg void OnEditInputWeight();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	void SettingGame();
};

#ifndef _DEBUG  // wndtermprojectView.cpp의 디버그 버전
inline CwndtermprojectDoc* CwndtermprojectView::GetDocument() const
   { return reinterpret_cast<CwndtermprojectDoc*>(m_pDocument); }
#endif

