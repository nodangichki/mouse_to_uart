#include <Windows.h>
#include <WinUser.h>
#include <iostream>
#include <string>
#include <sstream>



int sendSerially(const char* dataToSend) {
    const char* portName = "COM1";

    // Open the serial port
    HANDLE serialPort = CreateFileA(portName, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    if (serialPort == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to open the serial port." << std::endl;
        return 1;
    }

    // Configure the serial port settings 
    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(serialPort, &dcbSerialParams)) {
        std::cerr << "Failed to get serial port state." << std::endl;
        CloseHandle(serialPort);
        return 1;
    }

    dcbSerialParams.BaudRate = CBR_115200; // Set your desired baud rate here
    dcbSerialParams.ByteSize = 8;        // 8 data bits
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if (!SetCommState(serialPort, &dcbSerialParams)) {
        std::cerr << "Failed to configure serial port." << std::endl;
        CloseHandle(serialPort);
        return 1;
    }

    // Data to send

    DWORD bytesWritten;

    // Write data to the serial port
    if (!WriteFile(serialPort, dataToSend, strlen(dataToSend), &bytesWritten, NULL)) {
        std::cerr << "Failed to write data to the serial port." << std::endl;
    }
    else {
        std::cout << "Data sent: " << dataToSend << std::endl;
    }

    // Close the serial port
    CloseHandle(serialPort);
    //std::cout << "Serial port closed." << std::endl;

    return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static bool mouseMoved = false;
    static int control = 0;
    switch (message)
    {
    case WM_INPUT:
    {
        // Debugging message to verify WM_INPUT reception.
        OutputDebugString(TEXT("Received WM_INPUT message\n"));

        UINT dwSize = 0;

        GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
        LPBYTE lpb = new BYTE[dwSize];
        if (lpb == NULL)
        {
            OutputDebugString(TEXT("Failed to allocate memory\n"));
            return 0;
        }

        if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
            OutputDebugString(TEXT("GetRawInputData does not return correct size !\n"));

        RAWINPUT* raw = (RAWINPUT*)lpb;

        if (raw->header.dwType == RIM_TYPEMOUSE)
        {
            int xPosRelative = raw->data.mouse.lLastX;
            int yPosRelative = raw->data.mouse.lLastY;
            int clicks = raw->data.mouse.ulButtons;
            static int processedClicks;
            clicks == 1 ? processedClicks = 1 : processedClicks = 0;
            //std::cout << "Mouse movement: dx=" << xPosRelative << ", dy=" << yPosRelative << std::endl;
            if (processedClicks == 1) {
                std::stringstream ss;
                ss << 0 << ":" << 0 << ":" << processedClicks << "\n";
                std::string resultString = ss.str();
                //std::cout << resultString;
                const char* charData = resultString.c_str();
                sendSerially(charData);

            }

            if ((xPosRelative != 0 || yPosRelative != 0) && (control == 8)) {
                mouseMoved = true;

                std::stringstream ss;
                ss << xPosRelative << ":" << yPosRelative << ": this is just to make the whole thing longer" << 0 << "\n";
                std::string resultString = ss.str();
                //std::cout << resultString;
                const char* charData = resultString.c_str();
                sendSerially(charData);
                control = 0;
            }
            else {
                mouseMoved = false;
                control < 8 ? control++ : control = 0;
                //std::cout << "not moved the mouse" <<std::endl;
            }





        }

        delete[] lpb;
    }
    break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

int main()
{
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(0);
    wc.lpszClassName = L"RawInputMouseExample";
    std::cout << "main code's running";
    if (!RegisterClass(&wc))
    {
        return 1;
    }

    HWND hwnd = CreateWindow(wc.lpszClassName, L"Raw Input Mouse Example", 0, 0, 0, 0, 0, 0, 0, wc.hInstance, 0);
    if (!hwnd)
    {
        return 1;
    }
    RAWINPUTDEVICE Rid[1];
    Rid[0].usUsagePage = 0x01; // Generic Desktop Controls
    Rid[0].usUsage = 0x02;    // Mouse
    Rid[0].dwFlags = RIDEV_INPUTSINK;
    Rid[0].hwndTarget = hwnd;
    if (RegisterRawInputDevices(Rid, 1, sizeof(RAWINPUTDEVICE)) == FALSE)
    {
        return 1;
    }
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

}