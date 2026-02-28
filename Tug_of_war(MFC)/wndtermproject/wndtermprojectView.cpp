
// wndtermprojectView.cpp: CwndtermprojectView 클래스의 구현
//

#include "pch.h"
#include "framework.h"
#include <cstdlib>
#include <ctime>
// SHARED_HANDLERS는 미리 보기, 축소판 그림 및 검색 필터 처리기를 구현하는 ATL 프로젝트에서 정의할 수 있으며
// 해당 프로젝트와 문서 코드를 공유하도록 해 줍니다.
#ifndef SHARED_HANDLERS
#include "wndtermproject.h"
#endif

#include "wndtermprojectDoc.h"
#include "wndtermprojectView.h"
#include "CNPPL.h"
#include "CWEIGHT.h"
CNPPL dlg_ppl;
CWEIGHT dlg_weight;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CwndtermprojectView

IMPLEMENT_DYNCREATE(CwndtermprojectView, CView)

BEGIN_MESSAGE_MAP(CwndtermprojectView, CView)
	// 표준 인쇄 명령입니다.
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_COMMAND(ID_EDIT_INPUT_PPL, &CwndtermprojectView::OnEditInputPpl)
	ON_COMMAND(ID_EDIT_INPUT_WEIGHT, &CwndtermprojectView::OnEditInputWeight)
	ON_WM_KEYDOWN()
	ON_WM_SIZE()
	//ON_COMMAND(IDC_BUTTON_RANDOM, &CwndtermprojectView::OnButtonRandom)
END_MESSAGE_MAP()

// CwndtermprojectView 생성/소멸

CwndtermprojectView::CwndtermprojectView() noexcept
{
	// TODO: 여기에 생성 코드를 추가합니다.
	memset(weight, 0, sizeof(weight)); 
	SettingGame();
	ppl_cnt = 1;
	game_round=1;
	teamL_win=teamR_win=0;
	winner_rep[0] = winner_rep[1] = winner_rep[2] = 0;
	left_flag_rx=195;
	right_flag_lx=2005;
	circle_lx = 975, circle_rx = 1025, circle_uy = 115, circle_dy = 165;
	teamL_penalty=0;
	teamR_penalty=0;
}

CwndtermprojectView::~CwndtermprojectView()
{
}

BOOL CwndtermprojectView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: CREATESTRUCT cs를 수정하여 여기에서
	//  Window 클래스 또는 스타일을 수정합니다.

	return CView::PreCreateWindow(cs);
}

// CwndtermprojectView 그리기

void CwndtermprojectView::OnDraw(CDC* pDC)
{
	CBitmap L_flag, R_flag,*pold;
	CDC cDC;
	cDC.CreateCompatibleDC(pDC);

	L_flag.LoadBitmapW(IDB_BITMAP1);
	R_flag.LoadBitmapW(IDB_BITMAP2);
	
	pold = cDC.SelectObject(&L_flag);
	pDC->StretchBlt(100, 500, 95, 130, &cDC, 0, 0, 95, 130, SRCCOPY);
	cDC.SelectObject(pold);
	
	pold = cDC.SelectObject(&R_flag);
	pDC->StretchBlt(2005, 500, 95, 130, &cDC, 0, 0, 95, 130, SRCCOPY);
	cDC.SelectObject(pold);
	
	CwndtermprojectDoc* pDoc = GetDocument();

	pDC->TextOut(10, 10, _T("<Yellow Team>"));
	pDC->TextOut(2055, 10, _T("<Blue Team>"));
	pDC->TextOut(10, 35, _T("player1 : Q"));
	pDC->TextOut(2065, 35, _T("player2 : I"));
	pDC->TextOut(10, 60, _T("player3 : Z"));
	pDC->TextOut(2065, 60, _T("player4 : B"));
	pDC->TextOut(10, 85, _T("player5 : C"));
	pDC->TextOut(2065, 85, _T("player6 : M"));
	pDC->TextOut(10, 110, _T("player7 : E"));
	pDC->TextOut(2065, 110, _T("player8 : P"));

	//teamL 패널티 표시
	CString teamL_penalty_cnt;
	teamL_penalty_cnt.Format(_T("PENALTY : %d"), teamL_penalty);
	pDC->TextOut(90, 700, teamL_penalty_cnt);

	//teamR 패널티 표시
	CString teamR_penalty_cnt;
	teamR_penalty_cnt.Format(_T("PENALTY : %d"), teamR_penalty);
	pDC->TextOut(2000, 700, teamR_penalty_cnt);


	//ROUND 표시 text
	CFont font;
	font.CreateFont(60, 0, 0, 0, 700, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
	CFont* pOldFont = pDC->SelectObject(&font);
	CString title;
	title.Format(_T("<<GAME ROUND : %d>>"), game_round);
	pDC->TextOut(800, 10, title);


	CRect clientRect;
	GetClientRect(&clientRect);

	CPen* pOldPen;
	CBrush* pOldBrush;

	//round별 승리자를 나타낸 원 그리기
	circle_lx = 975, circle_rx = 1025, circle_uy = 115, circle_dy = 165;
	for (int i = 0; i < 3; i++) {

		//빨간팀
		if (winner_rep[i] == 1)
		{
			// 빨간색으로 펜과 브러시 생성
			CPen yellowPen(PS_SOLID, 3, RGB(0, 0, 255));
			CBrush yellowBrush(RGB(246, 255, 40));

			// 펜과 브러시를 DC에 선택
			pOldPen = pDC->SelectObject(&yellowPen);
			pOldBrush = pDC->SelectObject(&yellowBrush);

			// 승리한 팀의 원 그리기
			//centerX - radius, centerY - radius, centerX + radius, centerY + radius
			pDC->Ellipse(circle_lx, circle_uy, circle_rx, circle_dy);

			pDC->SelectObject(pOldPen);
			pDC->SelectObject(pOldBrush);
		}
		//파란팀
		else if (winner_rep[i] == 2)
		{
			// 빨간색으로 펜과 브러시 생성
			CPen bluePen(PS_SOLID, 3, RGB(246, 255, 40));
			CBrush blueBrush(RGB(0, 0, 255));

			// 펜과 브러시를 DC에 선택
			pOldPen = pDC->SelectObject(&bluePen);
			pOldBrush = pDC->SelectObject(&blueBrush);

			// 승리한 팀의 원 그리기
			//centerX - radius, centerY - radius, centerX + radius, centerY + radius
			pDC->Ellipse(circle_lx, circle_uy, circle_rx, circle_dy);

			pDC->SelectObject(pOldPen);
			pDC->SelectObject(pOldBrush);
		}

		circle_lx = circle_lx + 75;
		circle_rx = circle_rx + 75;
	}
	if (game_round >= 4) { AfxGetMainWnd()->SendMessage(WM_CLOSE); }



	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 여기에 원시 데이터에 대한 그리기 코드를 추가합니다.
	CRect rect(rope_lu.x,rope_lu.y,rope_rd.x,rope_rd.y);
	CBrush brush(RGB(255, 255, 255));
	pOldBrush = pDC->SelectObject(&brush);

	pDC->Rectangle(rect);
	//pDC->SelectObject(pOldBrush);
}


// CwndtermprojectView 인쇄

BOOL CwndtermprojectView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 기본적인 준비
	return DoPreparePrinting(pInfo);
}

void CwndtermprojectView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄하기 전에 추가 초기화 작업을 추가합니다.
}

void CwndtermprojectView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄 후 정리 작업을 추가합니다.
}


// CwndtermprojectView 진단

#ifdef _DEBUG
void CwndtermprojectView::AssertValid() const
{
	CView::AssertValid();
}

void CwndtermprojectView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CwndtermprojectDoc* CwndtermprojectView::GetDocument() const // 디버그되지 않은 버전은 인라인으로 지정됩니다.
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CwndtermprojectDoc)));
	return (CwndtermprojectDoc*)m_pDocument;
}
#endif //_DEBUG


// CwndtermprojectView 메시지 처리기
void CwndtermprojectView::OnEditInputPpl()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	if (dlg_ppl.DoModal() == IDOK) {
		ppl_cnt = dlg_ppl.m_nppl;
		if (ppl_cnt < 1 || ppl_cnt>8) {
			AfxMessageBox(_T("인원 수: 1~8명"),MB_OK | MB_ICONINFORMATION);
			dlg_ppl.m_nppl = 1;
		}
		
	}
}


void CwndtermprojectView::OnEditInputWeight()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	if (dlg_weight.DoModal() == IDOK){
		int weight_eval;
		
		//weight[1] 설정
		weight_eval = dlg_weight.m_player1w;
		if (weight_eval < 30) { weight[1] = 1; }
		else if (weight_eval >= 110) { weight[1] = 10; }
		else { weight[1] = weight_eval / 10 - 1; }
		
		weight_eval = dlg_weight.m_player2w;
		if (weight_eval < 30) { weight[2] = 1; }
		else if (weight_eval >= 110) { weight[2] = 10; }
		else { weight[2] = weight_eval / 10 - 1; }
		
		weight_eval = dlg_weight.m_player3w;
		if (weight_eval < 30) { weight[3] = 1; }
		else if (weight_eval >= 110) { weight[3] = 10; }
		else { weight[3] = weight_eval / 10 - 1; }
		
		weight_eval = dlg_weight.m_player4w;
		if (weight_eval < 30) { weight[4] = 1; }
		else if (weight_eval >= 110) { weight[4] = 10; }
		else { weight[4] = weight_eval / 10 - 1; }
		
		weight_eval = dlg_weight.m_player5w;
		if (weight_eval < 30) { weight[5] = 1; }
		else if (weight_eval >= 110) { weight[5] = 10; }
		else { weight[5] = weight_eval / 10 - 1; }
		
		weight_eval = dlg_weight.m_player6w;
		if (weight_eval < 30) { weight[6] = 1; }
		else if (weight_eval >= 110) { weight[6] = 10; }
		else { weight[6] = weight_eval / 10 - 1; }
		
		weight_eval = dlg_weight.m_player7w;
		if (weight_eval < 30) { weight[7] = 1; }
		else if (weight_eval >= 110) { weight[7] = 10; }
		else { weight[7] = weight_eval / 10 - 1; }
		
		weight_eval = dlg_weight.m_player8w;
		if (weight_eval < 30) { weight[8] = 1; }
		else if (weight_eval >= 110) { weight[8] = 10; }
		else { weight[8] = weight_eval / 10 - 1; }
	}
	
	teamL_pull = weight[1] + weight[3] + weight[5] + weight[7];
	teamR_pull = weight[2] + weight[4] + weight[6] + weight[8];

	for (int i = ppl_cnt+1; i <= 8; i++) 
		weight[i] = 0;

	switch (ppl_cnt)
	{
	case 1:
		dlg_weight.m_player2w = 0;
	case 2:
		dlg_weight.m_player3w = 0;
	case 3:
		dlg_weight.m_player4w = 0;
	case 4:
		dlg_weight.m_player5w = 0;
	case 5:
		dlg_weight.m_player6w = 0;
	case 6:
		dlg_weight.m_player7w = 0;
	case 7:
		dlg_weight.m_player8w = 0;
	}
		
}


//줄다리기 줄 당기기
void CwndtermprojectView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	//랜덤한 확률의 넘어짐 효과
	srand((unsigned)time(NULL));
	random = rand() % 10;
		
	if (teamL_penalty && teamL_penalty % 3 == 1) { teamL_pull += 3; }
	if (teamR_penalty && teamR_penalty % 3 == 1) { teamR_pull += 3; }
	
	switch (nChar) {
		

	//team A
	case 0x51:															//Q(player1)
		if (weight[1]) {
			rope_lu.x -= (teamL_pull + weight[1]);
			rope_rd.x -= (teamL_pull + weight[1]);
		}
		//넘이지는 확률: 20%(만약 이미 4명 다 넘어진 상태면 적용되지 않음.)
		if (random >= 0 && random <= 1 && teamL_penalty < 11) {
			if (teamL_pull - 3 >= 0)teamL_pull -= 3;
			teamL_penalty += 3;
		}
		break;
	case 0x5A:															//Z(player3)
		if (weight[3]) {
			rope_lu.x -= (teamL_pull + weight[3]);
			rope_rd.x -= (teamL_pull + weight[3]);
		}
		//넘이지는 확률: 20%(만약 이미 4명 다 넘어진 상태면 적용되지 않음.)
		if (random >= 0 && random <= 1 && teamL_penalty < 11) {
			if (teamL_pull - 3 >= 0)teamL_pull -= 3;
			teamL_penalty += 3;
		}
		break;
	case 0x43:															//C(player5)
		if (weight[5]) {
			rope_lu.x -= (teamL_pull + weight[5]);
			rope_rd.x -= (teamL_pull + weight[5]);
		}
		//넘이지는 확률: 20%(만약 이미 4명 다 넘어진 상태면 적용되지 않음.)
		if (random >= 0 && random <= 1 && teamL_penalty < 11) {
			if (teamL_pull - 3 >= 0)teamL_pull -= 3;
			teamL_penalty += 3;
		}
		break;
	case 0x45:															//E(player7)
		if (weight[7]) {
			rope_lu.x -= (teamL_pull + weight[7]);
			rope_rd.x -= (teamL_pull + weight[7]);
		}
		//넘이지는 확률: 20%(만약 이미 4명 다 넘어진 상태면 적용되지 않음.)
		if (random >= 0 && random <= 1 && teamL_penalty < 11) {
			if (teamL_pull - 3 >= 0)teamL_pull -= 3;
			teamL_penalty += 3;
		}
		break;

	//team B
	case 0x49:															//I(player2)
		if (weight[2]) {
			rope_lu.x += (teamR_pull + weight[2]);
			rope_rd.x += (teamR_pull + weight[2]);
		}
		//넘이지는 확률: 20%(만약 이미 4명 다 넘어진 상태면 적용되지 않음.)
		if (random >= 8 && random <= 9 && teamR_penalty < 11) {
			if(teamR_pull-3>=0)teamR_pull -= 3;
			teamR_penalty += 3;
		}
		break;
	case 0x42:															//B(player4)
		if (weight[4]) {
			rope_lu.x += (teamR_pull + weight[4]);
			rope_rd.x += (teamR_pull + weight[4]);
		}
		//넘이지는 확률: 20%(만약 이미 4명 다 넘어진 상태면 적용되지 않음.)
		if (random >= 8 && random <= 9 && teamR_penalty < 11) {
			if (teamR_pull - 3 >= 0)teamR_pull -= 3; 
			teamR_penalty += 3;
		}
		break;
	case 0x4D:															//M(player6)
		if (weight[6]) {
			rope_lu.x += (teamR_pull + weight[6]);
			rope_rd.x += (teamR_pull + weight[6]);
		}
		//넘이지는 확률: 20%(만약 이미 4명 다 넘어진 상태면 적용되지 않음.)
		if (random >= 8 && random <= 9 && teamR_penalty < 11) {
			if (teamR_pull - 3 >= 0)teamR_pull -= 3;
			teamR_penalty += 3;
		}
		break;
	case 0x50:															//P(player8)
		if (weight[8]) {
			rope_lu.x += (teamR_pull + weight[8]);
			rope_rd.x += (teamR_pull + weight[8]);
		}
		//넘이지는 확률: 20%(만약 이미 4명 다 넘어진 상태면 적용되지 않음.)
		if (random >= 8 && random <= 9 && teamR_penalty < 11) {
			if (teamR_pull - 3 >= 0)teamR_pull -= 3;
			teamR_penalty += 3;
		}
		break;
	}

	CString message;
	//왼쪽 팀 승리
	if (rope_lu.x <= left_flag_rx) {
		winner_rep[game_round - 1] = 1;
		SettingGame();
		if (game_round < 3) {
			teamL_win++;
			message.Format(_T("Round%d winner : Yellow Team"), game_round);
			AfxMessageBox(message, MB_OK | MB_ICONINFORMATION);
		}
		else {
			AfxMessageBox(_T("Yellow Team Win!!"), MB_OK | MB_ICONINFORMATION);
		}

		game_round++;
		//원 그리기(함수)
	}
	
	//오른쪽 팀 승리
	if (rope_rd.x >= right_flag_lx) {
		winner_rep[game_round-1] = 2;
		SettingGame();
		if (game_round < 3) {
			teamR_win++;
			message.Format(_T("Round%d winner : Blue Team"), game_round);
			AfxMessageBox(message, MB_OK | MB_ICONINFORMATION);
		}
		else {
			AfxMessageBox(_T("Blue Team Win!!"), MB_OK | MB_ICONINFORMATION);
		}

		game_round++;
		//원 그리기(함수)
	}

	Invalidate();

	if (teamL_penalty > 0)teamL_penalty -= 1;
	if (teamR_penalty > 0)teamR_penalty -= 1;

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}


//Game reset
void CwndtermprojectView::SettingGame()
{
	// TODO: 여기에 구현 코드 추가.
	rope_lu.x = 700;
	rope_lu.y = 580;
	rope_rd.x = 1500;
	rope_rd.y = 620;

	teamL_pull = 0;
	teamR_pull = 0;
}
