
// DPIScalingMFCAppDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DPIScalingMFCApp.h"
#include "DPIScalingMFCAppDlg.h"
#include "afxdialogex.h"
#include "../DPIHelper/DpiHelper.h"
#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define LOGW(x) { \
CString str; \
GetDlgItemTextW(IDC_EDIT_LOG, str); \
str += x; \
str += L"\r\n"; \
SetDlgItemTextW(IDC_EDIT_LOG, str); \
}

// CDPIScalingMFCAppDlg dialog



CDPIScalingMFCAppDlg::CDPIScalingMFCAppDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DPISCALINGMFCAPP_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDPIScalingMFCAppDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_displayList);
	DDX_Control(pDX, IDC_EDIT_CURRENT_DPI, m_currentDPI);
	DDX_Control(pDX, IDC_EDIT_CURRENT_DPI2, m_recommendedDPI);
	DDX_Control(pDX, IDC_LIST_SELECT_DPI, m_dpiList);
}

BEGIN_MESSAGE_MAP(CDPIScalingMFCAppDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_CBN_SELCHANGE(IDC_COMBO1, &CDPIScalingMFCAppDlg::OnCbnSelchangeCombo1)
	ON_BN_CLICKED(IDC_BUTTON2, &CDPIScalingMFCAppDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON1, &CDPIScalingMFCAppDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CDPIScalingMFCAppDlg message handlers

BOOL CDPIScalingMFCAppDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	Refresh();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

bool CDPIScalingMFCAppDlg::FillDisplayInfo(LUID adapterID, int sourceID)
{
	DpiHelper::DPIScalingInfo dpiInfo = DpiHelper::GetDPIScalingInfo(adapterID, sourceID);
	m_currentDPI.SetWindowTextW(std::to_wstring(dpiInfo.current).c_str());
	m_recommendedDPI.SetWindowTextW(std::to_wstring(dpiInfo.recommended).c_str());
	m_dpiList.ResetContent();

	int iIndex = 0;
	int currentIndex = -1;
	for (const auto& dpi : DpiVals)
	{
		if ((dpi >= dpiInfo.mininum) && (dpi <= dpiInfo.maximum))
		{
			m_dpiList.AddString(std::to_wstring(dpi).c_str());
			m_dpiList.SetItemData(iIndex, dpi);
			if (dpi == dpiInfo.current)
			{
				currentIndex = iIndex;
			}
			iIndex++;
		}
	}

	if (-1 == currentIndex)
	{
		LOGW(L"Error! Could not find currently set DPI");
		return false;
	}
	else
	{
		m_dpiList.SetCurSel(currentIndex);
	}

	return true;
}

bool CDPIScalingMFCAppDlg::Refresh()
{
	m_displayDataCache.clear();
	m_displayList.ResetContent();

	std::vector<DISPLAYCONFIG_PATH_INFO> pathsV;
	std::vector<DISPLAYCONFIG_MODE_INFO> modesV;
	int flags = QDC_ONLY_ACTIVE_PATHS;
	if (false == DpiHelper::GetPathsAndModes(pathsV, modesV, flags))
	{
		LOGW(L"DpiHelper::GetPathsAndModes() failed")
	}
	else
	{
		LOGW(L"DpiHelper::GetPathsAndModes() successful")
	}


	int iIndex = 0;
	for (const auto& path : pathsV)
	{
		//get display name
		auto adapterLUID = path.targetInfo.adapterId;
		auto targetID = path.targetInfo.id;
		auto sourceID = path.sourceInfo.id;

		DISPLAYCONFIG_TARGET_DEVICE_NAME deviceName;
		deviceName.header.size = sizeof(deviceName);
		deviceName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
		deviceName.header.adapterId = adapterLUID;
		deviceName.header.id = targetID;
		if (ERROR_SUCCESS != DisplayConfigGetDeviceInfo(&deviceName.header))
		{
			LOGW(L"DisplayConfigGetDeviceInfo() failed")
		}
		else
		{
			LOGW(CString(L"display name obtained: ") + CString(deviceName.monitorFriendlyDeviceName))
				std::wstring nameString = std::to_wstring(iIndex) + std::wstring(L". ") + deviceName.monitorFriendlyDeviceName;
			if (DISPLAYCONFIG_OUTPUT_TECHNOLOGY_INTERNAL == deviceName.outputTechnology)
			{
				nameString += L"(internal display)";
			}
			m_displayList.AddString(nameString.c_str());
			DisplayData dd = {};
			dd.m_adapterId = adapterLUID;
			dd.m_sourceID = sourceID;
			dd.m_targetID = targetID;

			m_displayDataCache[iIndex] = dd;
			m_displayList.SetItemData(iIndex, iIndex);
			iIndex++;
		}
	}

	if (0 == m_displayList.GetCount())
	{
		LOGW(L"no displays found!!")
	}
	else
	{
		m_displayList.SetCurSel(0);//select 1st entry by default
		FillDisplayInfo(m_displayDataCache[0].m_adapterId, m_displayDataCache[0].m_sourceID);
	}
	return true;
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDPIScalingMFCAppDlg::OnPaint()
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
HCURSOR CDPIScalingMFCAppDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CDPIScalingMFCAppDlg::OnCbnSelchangeCombo1()
{
	
	auto currSelIndex = m_displayList.GetCurSel();
	auto itr = m_displayDataCache.find(currSelIndex);
	if (m_displayDataCache.end() == itr)
	{//not found in cache
		LOGW(L"m_displayDataCache cache hit failed")
			return;
	}

	FillDisplayInfo(itr->second.m_adapterId, itr->second.m_sourceID);
}


void CDPIScalingMFCAppDlg::OnBnClickedButton2()
{
	
	Refresh();
}


void CDPIScalingMFCAppDlg::OnBnClickedButton1()
{
	
	auto currDpiSel = m_dpiList.GetCurSel();
	auto dpiToSet = m_dpiList.GetItemData(currDpiSel);
	auto currDisplaySel = m_displayList.GetCurSel();
	int CurrentDpiVal = GetDlgItemInt(m_currentDPI.GetDlgCtrlID());
	if (dpiToSet == CurrentDpiVal)
	{
		LOGW(L"Trying to set DPI which is already set. Nothing to do")
			return;
	}

	int recommendedVal = GetDlgItemInt(m_recommendedDPI.GetDlgCtrlID());
	DWORD_PTR cacheIndex = m_displayList.GetItemData(currDisplaySel);
	auto itr = m_displayDataCache.find(cacheIndex);

	if (m_displayDataCache.end() == itr)
	{
		LOGW(L"Cache miss");
		return;
	}

	auto res = DpiHelper::SetDPIScaling(itr->second.m_adapterId, itr->second.m_sourceID, dpiToSet);

	if (false == res)
	{
		LOGW(L"DpiHelper::SetDPIScaling() failed");
		return;
	}

	Refresh();
}
