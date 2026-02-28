#pragma once
#include "afxdialogex.h"


// CWEIGHT 대화 상자

class CWEIGHT : public CDialogEx
{
	DECLARE_DYNAMIC(CWEIGHT)

public:
	CWEIGHT(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CWEIGHT();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_WEIGHT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	int m_player1w;
	int m_player2w;
	int m_player3w;
	int m_player4w;
	int m_player5w;
	int m_player6w;
	int m_player7w;
	int m_player8w;
	afx_msg void OnBnClickedButtonRandom();
};
