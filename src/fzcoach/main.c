#include <sdkddkver.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <conio.h>
#include <process.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include "utilities.h"
#include <tchar.h>

#define FILE_MENU_OPEN 1
#define FILE_MENU_SAVE 2
#define VIEW_MENU_DEVICES 3
#define VIEW_MENU_SCREEN_CAPTURE 4
#define MAX_LOADSTRING 100

BOOL CALLBACK SetFont(HWND child, LPARAM font){
  SendMessage(child, WM_SETFONT, font, TRUE);
  return TRUE;
}

HMENU hMenu;

// Buttons
HWND hStartButton;
HWND hStopButton;
HWND hSearchWindowButton;
HWND hMatchSearch;

// Text
HWND hWindowSearchTitle;

// List Views
HWND hMatchList = NULL;
LVCOLUMN lvCol;
LVITEM lvItem;

HBITMAP hAuctionSniperImage;
HINSTANCE globalInstance;

void addMenu(HWND hWnd);
void addButtons(HWND hWnd);
void addText(HWND hWnd);
void addListViews(HWND hWnd);
void addImages(HWND hWnd);
HBRUSH whiteBrush;

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  LRESULT result = 0;

  if (message == WM_CREATE) {
    addMenu(hwnd);
    addButtons(hwnd);
    addText(hwnd);
    addListViews(hwnd);
    addImages(hwnd);

    hMatchSearch = CreateWindow(WC_COMBOBOX, TEXT(""), 
     CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
     350, 300, 100, 30, hwnd, NULL, globalInstance,
     NULL);

    TCHAR Planets[9][10] =  
    {
        TEXT("Mercury"), TEXT("Venus"), TEXT("Terra"), TEXT("Mars"), 
        TEXT("Jupiter"), TEXT("Saturn"), TEXT("Uranus"), TEXT("Neptune"), 
        TEXT("Pluto??") 
    };
           
    TCHAR A[16]; 
    int  k = 0; 

    memset(&A,0,sizeof(A));       
    for (k = 0; k <= 8; k += 1)
    {
        wcscpy_s(A, sizeof(A)/sizeof(TCHAR),  (TCHAR*)Planets[k]);

        // Add string to combobox.
        SendMessage(hMatchSearch,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) A); 
    }

    // Send the CB_SETCURSEL message to display an initial item 
    //  in the selection field  
    SendMessage(hMatchSearch, CB_SETCURSEL, (WPARAM)2, (LPARAM)0);
    EnumChildWindows(hwnd, (WNDENUMPROC)SetFont, (LPARAM)GetStockObject(DEFAULT_GUI_FONT));
    
    result = 1;
  }
  else {
    bool wasHandled = false;

    switch (message) {
      case WM_COMMAND:
      {
        switch(HIWORD(wParam)) {
          case BN_CLICKED:
          {
            if ((HWND)lParam == hStartButton) {
              startAuctionBot(hwnd);
            }
            else if((HWND)lParam == hStopButton) {
              stopAuctionBot(hwnd);
            }
            break;
          }
          case CBN_SELCHANGE: 
          {
            int ItemIndex = SendMessage((HWND) lParam, (UINT) CB_GETCURSEL, 
                (WPARAM) 0, (LPARAM) 0);
                TCHAR  ListItem[256];
                SendMessage((HWND) lParam, (UINT) CB_GETLBTEXT, 
                (WPARAM) ItemIndex, (LPARAM) ListItem);
            MessageBox(hwnd, (LPCWSTR) ListItem, TEXT("Item Selected"), MB_OK);                        
          }
        }

        if (wParam == VIEW_MENU_SCREEN_CAPTURE) {
          SendMessage(hwnd, WM_DESTROY, wParam, lParam);
        }

        wasHandled = true;
        result = 0;
        break;
      }
      case WM_DESTROY:
      {
        PostQuitMessage(0);
        wasHandled = true;
        result = 1;
        break;
      }
    }

    if (!wasHandled) {
      result = DefWindowProc(hwnd, message, wParam, lParam);
    }
  }

  return result;
}

// Main thread (window thread)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  whiteBrush = CreateSolidBrush(RGB(255, 255, 255));
  globalInstance = hInstance;

  TCHAR title[MAX_LOADSTRING];
  HCURSOR cursor;
  HICON icon, iconSmall;

  LoadString(hInstance, IDS_APP_TITLE, title, sizeof(TCHAR) * MAX_LOADSTRING);
  icon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON_SMALL));
  iconSmall = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON_SMALL));


  // Setup window class attributes.
  WNDCLASSEX wcex;
  ZeroMemory(&wcex, sizeof(wcex));

  wcex.cbSize = sizeof(wcex); // WNDCLASSEX size in bytes
  wcex.style = CS_HREDRAW | CS_VREDRAW;   // Window class styles
  wcex.lpszClassName = title; // Window class name
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);  // Window background brush color.
  wcex.hCursor = cursor;    // Window cursor
  wcex.lpfnWndProc = WindowProc;    // Window procedure associated to this window class.
  wcex.hInstance = hInstance; // The application instance.
  wcex.hIcon = icon;      // Application icon.
  wcex.hIconSm = iconSmall; // Application small icon.

  // Register window and ensure registration success.
  if (!RegisterClassEx(&wcex)) {
    return 1;
  }

  // Setup window initialization attributes.
  CREATESTRUCT cs;
  ZeroMemory(&cs, sizeof(cs));

  cs.x = CW_USEDEFAULT; // Window X position
  cs.y = CW_USEDEFAULT; // Window Y position
  cs.cx = 840;  // Window width
  cs.cy = 680;  // Window height
  cs.hInstance = hInstance; // Window instance.
  cs.lpszClass = wcex.lpszClassName;    // Window class name
  cs.lpszName = title;  // Window title
  cs.style = WS_OVERLAPPEDWINDOW;   // Window style
  cs.dwExStyle = WS_EX_TOPMOST;

  // Create the window.
  HWND hWnd = CreateWindowEx(
    cs.dwExStyle,
    cs.lpszClass,
    cs.lpszName,
    cs.style,
    cs.x,
    cs.y,
    cs.cx,
    cs.cy,
    cs.hwndParent,
    cs.hMenu,
    cs.hInstance,
    cs.lpCreateParams);

  // Validate window.
  if (!hWnd) {
    return 1;
  }


  ShowWindow(hWnd, SW_SHOWDEFAULT);
  UpdateWindow(hWnd);

  MSG msg;
  while(GetMessage(&msg, NULL, 0, 0 )) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  UnregisterClass(wcex.lpszClassName, hInstance);

  return (int)msg.wParam;
}

void addMenu(HWND hWnd) {
  // File menu
  HMENU hFileMenu = CreateMenu();
  AppendMenu(hFileMenu, MF_STRING, (UINT_PTR)FILE_MENU_OPEN, L"Open");
  AppendMenu(hFileMenu, MF_STRING, (UINT_PTR)FILE_MENU_SAVE, L"Save");
  AppendMenu(hFileMenu, MF_SEPARATOR, (UINT_PTR)NULL, NULL); 

  // View menu
  HMENU hViewMenu = CreateMenu();
  AppendMenu(hViewMenu, MF_STRING, (UINT_PTR)VIEW_MENU_DEVICES, L"Devices");
  AppendMenu(hViewMenu, MF_STRING, (UINT_PTR)VIEW_MENU_SCREEN_CAPTURE, L"Screen Capture");

  // Main menu
  hMenu = CreateMenu();

  AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, L"&File");
  AppendMenu(hMenu, MF_STRING, (UINT_PTR)NULL, L"&Edit");
  AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hViewMenu, L"&View");
  AppendMenu(hMenu, MF_STRING, (UINT_PTR)NULL, L"&Help");

  SetMenu(hWnd, hMenu);
}

void addButtons(HWND hWnd) {
  // Start button
  hStartButton = CreateWindow(
    L"BUTTON",  // Predefined class; Unicode assumed 
    L"Start",      // Button text 
    WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
    10,         // x position 
    10,         // y position 
    100,        // Button width
    50,        // Button height
    hWnd,     // Parent window
    NULL,       // No menu.
    globalInstance, 
    NULL);      // Pointer not needed.

  // Stop button
  hStopButton = CreateWindow(
    L"BUTTON",  // Predefined class; Unicode assumed 
    L"Stop",      // Button text 
    WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
    120,         // x position 
    10,         // y position 
    100,        // Button width
    50,        // Button height
    hWnd,     // Parent window
    NULL,       // No menu.
    globalInstance, 
    NULL);      // Pointer not needed.

  // Search window button

  hSearchWindowButton = CreateWindow(
    L"BUTTON",  // Predefined class; Unicode assumed 
    L"Search",      // Button text 
    WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
    10,         // x position 
    70,         // y position 
    100,        // Button width
    50,        // Button height
    hWnd,     // Parent window
    NULL,       // No menu.
    globalInstance, 
    NULL);      // Pointer not needed.
}

void addText(HWND hWnd) {
  hWindowSearchTitle = CreateWindow(
    L"STATIC",  // Predefined class; Unicode assumed 
    L"Forza Horizon 5 window matches:",      // Button text 
    WS_VISIBLE | WS_CHILD | SS_CENTER,  // Styles 
    10,         // x position 
    140,         // y position 
    300,        // Button width
    20,        // Button height
    hWnd,     // Parent window
    (HMENU)IDT_WINDOW_MATCHES_TITLE,       // No menu.
    globalInstance, 
    NULL);      // Pointer not needed.
}

void addListViews(HWND hWnd) {
  INITCOMMONCONTROLSEX icex;           // Structure for control initialization.
    icex.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);

    RECT rcClient;                       // The parent window's client area.

    GetClientRect (hWnd, &rcClient); 

    // Create the list-view window in report view with label editing enabled.
    hMatchList = CreateWindow(
        WC_LISTVIEW, 
        L"List",
        WS_VISIBLE |WS_CHILD | LVS_REPORT | LVS_EDITLABELS,
        10, 160,
        300,
        200,
        hWnd,
        (HMENU)IDL_WINDOW_MATCHES,
        globalInstance,
        NULL);

    WCHAR szText[256];     // Temporary buffer.
    LVCOLUMN lvc;
    int iCol;

    // Initialize the LVCOLUMN structure.
    // The mask specifies that the format, width, text,
    // and subitem members of the structure are valid.
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

    // Add the columns.
    for (iCol = 0; iCol < 2; iCol++)
    {
        lvc.iSubItem = iCol;
        lvc.pszText = szText;
        lvc.cx = 100;               // Width of column in pixels.
        lvc.fmt = LVCFMT_LEFT;  // Left-aligned column.

        // Load the names of the column headings from the string resources.
        LoadString(globalInstance,
                   IDS_WINDOW_NAMES_COLUMN + iCol,
                   szText,
                   sizeof(szText)/sizeof(szText[0]));

        // Insert the columns into the list view.
        if (ListView_InsertColumn(hMatchList, iCol, &lvc) == -1) {
          printf("Could not create list view!\n");
        }
    }
    ListView_SetExtendedListViewStyle(hMatchList, LVS_EX_FULLROWSELECT);

    listAllWindows(&hMatchList);
}

void addImages(HWND hWnd) {
}

