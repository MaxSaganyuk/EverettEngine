
// MainFrm.h : interface of the CMainFrame class
//

#pragma once
#include "ChildView.h"

#include "CMainWindow.h"
#include "EverettEngine.h"
#include "NameEditChecker.h"

class CMainFrame : public CFrameWnd
{
	
public:
	CMainFrame() noexcept;
protected: 
	DECLARE_DYNAMIC(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CToolBar          m_wndToolBar;
	CStatusBar        m_wndStatusBar;
	CChildView    m_wndView;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	DECLARE_MESSAGE_MAP()

	bool ClearTree();
	bool LoadObjectNamesToTree();

private:
	void OnLoadSave(bool load, std::function<bool(const std::string&)> loadSaveFunc);
	void OnLoad();
	void OnSave();

	void OnLoadModel();
	void OnPlaceSolid();
	void OnPlaceLight();
	void OnPlaceSound();
	void OnCameraOptions();
	void OnKeybindOptions();
	void OnGameProduce();
	EverettEngine engine;
	std::unique_ptr<CMainWindow> mainWindow;
	std::thread engineRenderThread;

	NameEditChecker::NameCheckFunc nameCheckFunc;
};


