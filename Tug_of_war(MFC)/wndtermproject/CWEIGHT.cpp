// CWEIGHT.cpp: 구현 파일
//

#include "pch.h"
#include "wndtermproject.h"
#include "afxdialogex.h"
#include "CWEIGHT.h"


// CWEIGHT 대화 상자

IMPLEMENT_DYNAMIC(CWEIGHT, CDialogEx)

CWEIGHT::CWEIGHT(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_WEIGHT, pParent)
	, m_player1w(0)
	, m_player2w(0)
	, m_player3w(0)
	, m_player4w(0)
	, m_player5w(0)
	, m_player6w(0)
	, m_player7w(0)
	, m_player8w(0)
{

}

CWEIGHT::~CWEIGHT()
{
}

void CWEIGHT::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_PLAYER1, m_player1w);
	DDX_Text(pDX, IDC_EDIT_PLAYER1, m_player1w);
	DDX_Text(pDX, IDC_EDIT_PLAYER2, m_player2w);
	DDX_Text(pDX, IDC_EDIT_PLAYER3, m_player3w);
	DDX_Text(pDX, IDC_EDIT_PLAYER4, m_player4w);
	DDX_Text(pDX, IDC_EDIT_PLAYER5, m_player5w);
	DDX_Text(pDX, IDC_EDIT_PLAYER6, m_player6w);
	DDX_Text(pDX, IDC_EDIT_PLAYER7, m_player7w);
	DDX_Text(pDX, IDC_EDIT_PLAYER8, m_player8w);
}


BEGIN_MESSAGE_MAP(CWEIGHT, CDialogEx)
END_MESSAGE_MAP()


// CWEIGHT 메시지 처리기