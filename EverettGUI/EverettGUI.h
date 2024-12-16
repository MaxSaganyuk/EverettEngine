
// EverettGUI.h : main header file for the EverettGUI application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CEverettGUIApp:
// See EverettGUI.cpp for the implementation of this class
//

class CEverettGUIApp : public CWinApp
{
public:
	CEverettGUIApp() noexcept;


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation

public:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CEverettGUIApp theApp;
