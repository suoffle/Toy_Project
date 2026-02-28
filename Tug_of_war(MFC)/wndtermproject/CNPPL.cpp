// CNPPL.cpp: 구현 파일
//

#include "pch.h"
#include "wndtermproject.h"
#include "afxdialogex.h"
#include "CNPPL.h"


// CNPPL 대화 상자

IMPLEMENT_DYNAMIC(CNPPL, CDialogEx)

CNPPL::CNPPL(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_NPPL, pParent)
	, m_nppl(1)
{

}

CNPPL::~CNPPL()
{
}

void CNPPL::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_PPL_CNT, m_nppl);
}


BEGIN_MESSAGE_MAP(CNPPL, CDialogEx)
	ON_BN_CLICKED(IDOK, &CNPPL::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CNPPL::OnBnClickedCancel)
END_MESSAGE_MAP()


// CNPPL 메시지 처리기


void CNPPL::OnBnClickedOk()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CDialogEx::OnOK();
}


void CNPPL::OnBnClickedCancel()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CDialogEx::OnCancel();
}
