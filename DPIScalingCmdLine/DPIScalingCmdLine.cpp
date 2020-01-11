// DPIScalingCmdLine.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <map>
#include "../DPIHelper/DpiHelper.h"
#include <string>

    //to store display info along with corresponding list item
    struct DisplayData {
        LUID m_adapterId;
        int m_targetID;
        int m_sourceID;
        //int currentDPI, minDPI, maxDPI, recommendedDPI;

        DisplayData()
        {
            m_adapterId = {};
            m_targetID = m_sourceID = -1;
            //currentDPI = minDPI = maxDPI = recommendedDPI = -1;
        }
    };
    std::map<int, DisplayData> m_displayDataCache;

int main(int nArg, char** pArgArray)
{
  
	std::vector<DISPLAYCONFIG_PATH_INFO> pathsV;
	std::vector<DISPLAYCONFIG_MODE_INFO> modesV;
	int flags = QDC_ONLY_ACTIVE_PATHS;
	if (false == DpiHelper::GetPathsAndModes(pathsV, modesV, flags))
	{
		std::cout << "DpiHelper::GetPathsAndModes() failed" << std::endl;
	}
	else
	{
		std::cout << "DpiHelper::GetPathsAndModes() successful" << std::endl;
	}


	int iIndex = 0;
	UINT32 monMaxIndex = 0;
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
			std::cout << "DisplayConfigGetDeviceInfo() failed";
		}
		else
		{
			
			std::wcout << L"Display name "<< iIndex << " obtained: " << deviceName.monitorFriendlyDeviceName << std::endl;
			std::wstring nameString = std::to_wstring(iIndex) + std::wstring(L". ") + deviceName.monitorFriendlyDeviceName;
			if (DISPLAYCONFIG_OUTPUT_TECHNOLOGY_INTERNAL == deviceName.outputTechnology)
			{
				nameString += L"(internal display)";
			}
			//m_displayList.AddString(nameString.c_str());
			DisplayData dd = {};
			dd.m_adapterId = adapterLUID;
			dd.m_sourceID = sourceID;
			dd.m_targetID = targetID;

			m_displayDataCache[iIndex] = dd;
			//m_displayList.SetItemData(iIndex, iIndex);
			iIndex++;
			monMaxIndex++;
		}
	}
	monMaxIndex--;
	if( nArg > 1)
	{
		UINT32 monIndex = 0;
		UINT32 dpiIndex = 1;
		if( nArg > 2 )
		{
			if( pArgArray[1][0]!=0 && isdigit(pArgArray[1][0]))
			{
				monIndex = atoi(pArgArray[1]);
				dpiIndex = 2;
			}
		}
		if( monIndex > monMaxIndex )
		{
			std::wcout << L"Error! Invalid monitor index." << std::endl;
			return 1;
		}
	
		LUID adapterID = m_displayDataCache[monIndex].m_adapterId;
		UINT32 sourceID =  m_displayDataCache[monIndex].m_sourceID;
		DpiHelper::DPIScalingInfo dpiInfo = DpiHelper::GetDPIScalingInfo(adapterID, sourceID);
		std::wcout << L"DPI Current    : " << std::to_wstring(dpiInfo.current).c_str() << std::endl;
		std::wcout << L"DPI Recommended: " <<std::to_wstring(dpiInfo.recommended).c_str() << std::endl;

		iIndex = 0;
		int currentIndex = -1;
		std::wstring strCurrent;
		std::wcout << L"Supported DPI for monitor:" << std::endl;
		for (const auto& dpi : DpiVals)
		{
			if ((dpi >= dpiInfo.mininum) && (dpi <= dpiInfo.maximum))
			{
				std::wcout << std::to_wstring(dpi).c_str() << std::endl;
				if (dpi == dpiInfo.current)
				{
					strCurrent = std::to_wstring(dpi);
					currentIndex = iIndex;
				}
				iIndex++;
			}
		}
		std::wcout << std::endl;

		if (-1 == currentIndex)
		{
			std::wcout << L"Error! Could not find currently set DPI";
			return false;
		}
		
		if( pArgArray[dpiIndex][0]!=0 && isdigit(pArgArray[dpiIndex][0]))
		{
			bool bValidDPI = false;
			bool bAlreadySetDpi = false;
			UINT32 dpiToSet = atoi(pArgArray[dpiIndex]);
			for (const auto& dpi : DpiVals)
			{
				if ((dpi >= dpiInfo.mininum) && (dpi <= dpiInfo.maximum))
				{
					if( dpiToSet == dpi)
					{ 
						bValidDPI = true;
					}
					if ((dpi == dpiInfo.current ) && (dpiToSet == dpiInfo.current))
					{
						bAlreadySetDpi = true;
					}
					iIndex++;
				}
			}
			if( bAlreadySetDpi )
			{
				std::cout <<"Current DPI is already set to that DPI value" << std::endl;
			}
			else
			{
				if( bValidDPI )
				{
					auto res = DpiHelper::SetDPIScaling(adapterID, sourceID, dpiToSet);
					if (false == res)
					{
						std::cout << "DpiHelper::SetDPIScaling() failed" << std::endl;
						return 1;
					}

					return 0;
				}
				else
				{
					if( dpiToSet >= 100)
					{
						std::cout <<"Invalid DPI setting" << std::endl;
					}
				}
			}
		}
	}
	else
	{
		std::cout << std::endl << "Set Windows 8.1 and Windows 10 Per Monitor DPI" << std::endl;
		std::cout <<"Usage:" << std::endl;
		std::cout <<"DPIScalingCmdLine {monitor index} {DPI}" << std::endl;
		std::cout <<"\tmonitor index - optional - 0-based index of monitor to set DPI on" << std::endl;
		std::cout <<"\t                         - if >= 100 and no DPI value - will set primary monitor DPI" << std::endl;
		std::cout <<"\tDPI - optional - DPI value to set - must be supported by monitor" << std::endl;
	}
	return 1;
}

