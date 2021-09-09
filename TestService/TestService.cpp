#include <iostream>
#include <Windows.h>

void test()
{
    TCHAR szIniFile[MAX_PATH];
    ::GetModuleFileName(NULL, szIniFile, MAX_PATH);
    // C 库函数 char *strrchr(const char *str, int c) 
    // 在参数 str 所指向的字符串中搜索最后一次出现字符 c（一个无符号字符）的位置。
    strrchr(szIniFile, '\\')[1] = 0;

    // C 库函数 char *strchr(const char *str, int c) 
    // 在参数 str 所指向的字符串中搜索第一次出现字符 c（一个无符号字符）的位置。
    // https://www.runoob.com/cprogramming/c-function-strchr.html

    char a[50];
    memset(a, 0, sizeof(a));
    const char* c = "abcdef-ghijk";
    memcpy(a, c, strlen(c));
    strrchr(a, '-')[1] = 0;
}

long getFileSize(const char* strFileName)
{
    struct _stat info;
    _stat(strFileName, &info);
    long size = info.st_size;
    return size;
}
#define MAXSIZE 1000000
void WriteLog(const char* strOutputString, ...)
{
    char strBuffer[4096] = { 0 };
    va_list vlArgs;
    va_start(vlArgs, strOutputString);
    _vsnprintf_s(strBuffer, sizeof(strBuffer) - 1, strOutputString, vlArgs);
    va_end(vlArgs);

    SYSTEMTIME sys;
    GetLocalTime(&sys);
    char timeBuf[128];
    sprintf_s(timeBuf, "%4d/%02d/%02d %02d:%02d:%02d.%03d", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);

    const char* file_name = "C:\\Windows\\Temp\\zh.log";
    FILE* fLog;
    fLog = fopen(file_name, "a");
    fprintf(fLog, "%s : %s\n", timeBuf, strBuffer);
    fflush(fLog);
    printf("%s : %s\n", timeBuf, strBuffer);
    fclose(fLog);

    // 按大小截断日志文件
    if (getFileSize(file_name) > MAXSIZE)
    {
        char dirPath[100];
        memset(dirPath, 0, sizeof(dirPath));
        memcpy(dirPath, file_name, strlen(file_name));
        strrchr(dirPath, '\\')[1] = 0;

        char new_name[128];
        sprintf_s(new_name, "%szh.%4d-%02d-%02d_%02d-%02d-%02d.log", dirPath, sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond);
        rename(file_name, new_name);
    }
}

//--------------------------------
//int st_quit = 0;
DWORD WINAPI ServiceThread(LPVOID para)
{
    int run_count = 0;
    for (;;)
    {
        //if (st_quit)
        //    break;
        run_count++;
        WriteLog("ServiceThread running:%d", run_count);
        Sleep(1000);
    }
    WriteLog("ServiceThread over:%d", run_count);
    return 0;
}
//-------------------------

#define SERVICE_NAME "TestService"
SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE hServiceStatusHandle;

void WINAPI ServiceHandler(DWORD fdwControl)
{
    switch (fdwControl)
    {
    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:
        // 结束目标进程
        //st_quit = 1;

        ServiceStatus.dwWin32ExitCode = 0;
        ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        ServiceStatus.dwCheckPoint = 0;
        ServiceStatus.dwWaitHint = 0;
        if (!SetServiceStatus(hServiceStatusHandle, &ServiceStatus))
        {
            DWORD nError = GetLastError();
            WriteLog("ServiceHandler stop error:%d", nError);
        }
        else
        {
            WriteLog("ServiceHandler stop ok");
        }

        break;
    default:
        return;
    };
    
}

void WINAPI ServiceMain(int argc, char** argv)
{
    hServiceStatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceHandler);
    if (hServiceStatusHandle == 0)
    {
        DWORD nError = GetLastError();
        WriteLog("RegisterServiceCtrlHandler error:%d", nError);
    }
    else
    {
        WriteLog("RegisterServiceCtrlHandler ok");
    }

    ServiceStatus.dwServiceType = SERVICE_WIN32;
    ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_PAUSE_CONTINUE;
    ServiceStatus.dwWin32ExitCode = 0;
    ServiceStatus.dwServiceSpecificExitCode = 0;
    ServiceStatus.dwCheckPoint = 0;
    ServiceStatus.dwWaitHint = 0;
    ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    ServiceStatus.dwCheckPoint = 0;
    ServiceStatus.dwWaitHint = 9000;

    if (!SetServiceStatus(hServiceStatusHandle, &ServiceStatus))
    {
        DWORD nError = GetLastError();
        WriteLog("SERVICE_RUNNING start error:%d", nError);
    }
    else
    {
        WriteLog("SERVICE_RUNNING start ok");
    }
    CreateThread(NULL, NULL, ServiceThread, NULL, NULL, NULL);
}


int main()
{
    STARTUPINFO startupInfo = { 0 };
    GetStartupInfo(&startupInfo);

    // 如果是以服务的形式启动
    if (startupInfo.dwFlags == STARTF_FORCEOFFFEEDBACK)
    {
        WriteLog("service begin");
        SERVICE_TABLE_ENTRY ServiceTable[2];
        ServiceTable[0].lpServiceName = (LPSTR)SERVICE_NAME;
        ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;

        ServiceTable[1].lpServiceName = NULL;
        ServiceTable[1].lpServiceProc = NULL;
        // 启动服务的控制分派机线程
        // 此处阻塞，直到服务停止
        bool a = StartServiceCtrlDispatcher(ServiceTable);
        WriteLog("service over: %d %d", startupInfo.dwFlags, a);
    }

    WriteLog("main over: %d", startupInfo.dwFlags);

    printf("Hello World!\n");
}

// sc create TestService binPath= "C:\Users\Neo\source\repos\TestService\Debug\TestService.exe"
// sc description TestService "这是一个用来演示的测试服务程序。"
// sc stop TestService
// sc start TestService
// sc delete TestService
