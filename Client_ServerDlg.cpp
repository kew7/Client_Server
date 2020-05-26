// ************** КЛИЕНТ-СЕРВЕРНОЕ ПРИЛОЖЕНИЕ В ОДНОМ ДИАЛОГОВОМ ОКНЕ **************

// Client_ServerDlg.cpp : implementation file
//
#include "pch.h"
#include "framework.h"
#include "Client_Server.h"
#include "Client_ServerDlg.h"
#include "afxdialogex.h"
#include <winsock2.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

// CAboutDlg dialog used for App About
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// CClientServerDlg dialog
CClientServerDlg::CClientServerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CLIENT_SERVER_DIALOG, pParent)	
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CClientServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST2, m_List2);
	DDX_Control(pDX, IDC_LIST1, m_List1);
}

BEGIN_MESSAGE_MAP(CClientServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_bServer1, &CClientServerDlg::OnBnClickedbserver1)
	ON_BN_CLICKED(IDC_bClient1, &CClientServerDlg::OnBnClickedbclient1)
END_MESSAGE_MAP()


		// ************** SERVER *************** //
	// ************** ГЛОБАЛЬНАЯ ФУНКЦИЯ *************** //
		/* 
		1. Initialize Winsock.
		2. Create a socket.
		3. Bind the socket.
		4. Listen on the socket for a client.
		5. Accept a connection from a client.
		6. Receive and send data.
		7. Disconnect.
		*/
DWORD WINAPI ServerThread(LPVOID lpParam)
{
	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo* result = NULL;
	struct addrinfo hints;

	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	int iResult;
	int iSendResult;

	theApp.dlg->m_List1.AddString("**** SERVER ****");
	theApp.dlg->m_List1.AddString("Waiting to receive data from the Client...");

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		MessageBox(NULL, "Getaddrinfo failed with error", 0, MB_ICONERROR);
		WSACleanup();
		return 1;
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		MessageBox(NULL, "Socket failed with error", 0, MB_ICONERROR);
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Bind the socket
	// Setup the TCP listening socket	
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		MessageBox(NULL, "Bind failed with error", 0, MB_ICONERROR);
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		MessageBox(NULL, "Listen failed with error", 0, MB_ICONERROR);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	// Accept a client socket
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		MessageBox(NULL, "Accept failed with error", 0, MB_ICONERROR);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	// No longer need server socket
	closesocket(ListenSocket);

	// Receive and send data
	// Receive until the peer shuts down the connection
	CString strTemp;
	do {

		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0); // количество принятных байт
		
		if (iResult > 0)
		{
			theApp.dlg->m_List1.AddString("Received Text: " + ((CString)recvbuf).Left(iResult));
			strTemp.Format("%u", iResult); // переводим целое в строковое представление
			theApp.dlg->m_List1.AddString("Bytes received: " + strTemp);

			// Echo the buffer back to the sender
			iSendResult = send(ClientSocket, recvbuf, iResult, 0);
			if (iSendResult == SOCKET_ERROR) {
				MessageBox(NULL, "Send failed with error", 0, MB_ICONERROR);
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
			}
			strTemp.Format("%u", iSendResult);
			theApp.dlg->m_List1.AddString("Bytes sent: " + strTemp); //+ iSendResult
		}
		else if (iResult == 0)
			theApp.dlg->m_List1.AddString("Connection closing... Ok");
		else {
			MessageBox(NULL, "Recv failed with error", 0, MB_ICONERROR);
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}
	} while (iResult > 0);

	// shutdown the connection since we're done
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		MessageBox(NULL, "Shutdown failed with error:", 0, MB_ICONERROR);
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}

	closesocket(ClientSocket);
	return 0;
} 		
		// ************** SERVER END ************** //


		// ************** CLIENT *************** //
	// ************** ГЛОБАЛЬНАЯ ФУНКЦИЯ *************** //
		/*		
		1. Initialize Winsock.
		2. Create a socket.
		3. Connect to the server.
		4. Send and receive data.
		5. Disconnect.
		*/
DWORD WINAPI ClientThread(LPVOID lpParam)
{
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
					*ptr = NULL,
					 hints;
	const char* sendbuf = "THIS IS A TEST";
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;
	CString strTemp;
	//char strTemp[50]= "Connect to Server...";;

	theApp.dlg->m_List2.AddString("**** CLIENT ****");
	strTemp = "Connect to Server... ";;
	//theApp.dlg->m_List2.AddString("Connect to Server...");

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		theApp.dlg->m_List2.AddString("Getaddrinfo failed with error");
		MessageBox(NULL, "Getaddrinfo failed with error", 0, MB_ICONERROR);;
		WSACleanup();
		return 1;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL;ptr = ptr->ai_next)
	{
		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			MessageBox(NULL,"Socket failed with error",0, MB_ICONERROR);
			WSACleanup();
			return 1;
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		else
		{
			strTemp += "Ок";
			theApp.dlg->m_List2.AddString(strTemp);
		}
	break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		MessageBox(NULL,"Unable to connect to server!",0, MB_ICONERROR);
		WSACleanup();
		return 1;
	}

	// Send an initial buffer
	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult == SOCKET_ERROR) {
		MessageBox(NULL, "Send failed with errorn", 0, MB_ICONERROR);
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	theApp.dlg->m_List2.AddString("Sent text: " + (CString)sendbuf);
	strTemp.Format("%u", iResult);
	theApp.dlg->m_List2.AddString("Bytes Sent: " + strTemp); //+iResult

	// shutdown the connection since no more data will be sent
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		MessageBox(NULL, "Shutdown failed with error: ", 0, MB_ICONERROR);
		closesocket(ConnectSocket);
		return 0;
	}
	// Receive until the peer closes the connection
	do {
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		strTemp.Format("%u", iResult);
		if (iResult > 0)
			theApp.dlg->m_List2.AddString("Bytes received: " + strTemp); // +iResult
		else if (iResult == 0)
			theApp.dlg->m_List2.AddString("Connection closed.");
		else
			theApp.dlg->m_List2.AddString("Recv failed with error");
	} while (iResult > 0);

	// cleanup
	closesocket(ConnectSocket);
	//WSACleanup(); // выгружается при закрытии окна в InitInstance()
	return 0;
}
		// ************** CLIENT END ************** //


// CClientServerDlg message handlers
BOOL CClientServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	WSADATA       wsd;
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)  // загружаем библиотеку при инициализации окна
	{
		MessageBox(0, "Can't load WinSock", 0);
		return FALSE;
	}
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CClientServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CClientServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CClientServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// Обработка нажатия клавиши ESCAPE, чтобы не закрывалось основное диалоговое окно
BOOL CClientServerDlg::PreTranslateMessage(MSG* pMsg)
{
	if ((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_ESCAPE))
	{
		return TRUE;
	} 
	return CDialog::PreTranslateMessage(pMsg);
}

void CClientServerDlg::OnBnClickedbserver1() // обработка нажатия кнопки "Server"
{
	// TODO: Add your control notification handler code here
	HANDLE        hServerThread;
	DWORD         dwServerThreadId;

	GetDlgItem(IDC_bServer1)->EnableWindow(FALSE);

	// открываем поток для сервера
	hServerThread = CreateThread(NULL, 0, ServerThread, 0, 0, &dwServerThreadId);
	
}

void CClientServerDlg::OnBnClickedbclient1() // обработка нажатия кнопки "Client"
{
	// TODO: Add your control notification handler code here
	HANDLE        hClientThread;
	DWORD         dwClientThreadId;

	GetDlgItem(IDC_bClient1)->EnableWindow(FALSE);

	// открываем поток для клиента
	hClientThread = CreateThread(NULL, 0, ClientThread, 0, 0, &dwClientThreadId);
}


