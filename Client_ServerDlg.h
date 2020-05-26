
// Client_ServerDlg.h : header file
//

#pragma once


// CClientServerDlg dialog
class CClientServerDlg : public CDialogEx
{
// Construction
public:
	CClientServerDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CLIENT_SERVER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;
	BOOL CClientServerDlg::PreTranslateMessage(MSG* pMsg);
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()	
public:
	afx_msg void OnBnClickedbserver1();
	afx_msg void OnBnClickedbclient1();
	CListBox m_List2;
	CListBox m_List1;
};

