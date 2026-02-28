#pragma once
#include "afxdialogex.h"


// CNPPL 대화 상자

class CNPPL : public CDialogEx
{
	DECLARE_DYNAMIC(CNPPL)

public:
	CNPPL(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CNPPL();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_NPPL };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	int m_nppl;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
