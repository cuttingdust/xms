#include <iostream>
#include <string>
#include <vector>
#include <cstring>

#ifndef _WIN32
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#else
#include <windows.h>
#endif

#ifndef _WIN32

void startProcess(const std::vector<std::string>& command)
{
    pid_t pid = fork();
    if (pid == 0)
    { /// 子进程
        /// 使用 execvp 执行命令
        std::vector<char*> args;
        for (const auto& arg : command)
        {
            args.push_back(const_cast<char*>(arg.c_str()));
        }
        args.push_back(nullptr); /// execvp 需要以 nullptr 结束参数列表
        execvp(args[0], args.data());
        /// 如果 execvp 返回，说明出错
        std::cerr << "Failed to execute: " << command[0] << std::endl;
        exit(1);
    }
    else if (pid < 0)
    {
        std::cerr << "Fork failed" << std::endl;
    }
    /// 父进程继续
}

#else

void startProcess(const std::vector<std::string>& command, PROCESS_INFORMATION& pi)
{
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    /// 将命令行参数组合成一个字符串
    std::string commandLine = command[0];
    for (size_t i = 1; i < command.size(); ++i)
    {
        commandLine += " " + command[i];
    }

    /// 转换命令行字符串为宽字符
    std::wstring wideCommandLine(commandLine.begin(), commandLine.end());

    /// 创建新进程
    if (!CreateProcess(NULL,                   /// No module name (use command line)
                       wideCommandLine.data(), /// Command line (宽字符)
                       NULL,                   /// Process handle not inheritable
                       NULL,                   /// Thread handle not inheritable
                       FALSE,                  /// Set handle inheritance to FALSE
                       0,                      /// No creation flags
                       NULL,                   /// Use parent's environment block
                       NULL,                   /// Use parent's starting directory
                       &si,                    /// Pointer to STARTUPINFO structure
                       &pi)                    /// Pointer to PROCESS_INFORMATION structure
    )
    {
        std::cerr << "Failed to create process: " << command[0] << std::endl;
        return;
    }

    /// 关闭 process and thread handles.
    CloseHandle(pi.hThread); /// 只关闭线程句柄，保留进程句柄以便在后续使用
}

#endif


int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cout << "Usage: ./watchdog <interval> <command> [args...]" << std::endl;
        return -1;
    }

    int                      interval = atoi(argv[1]);
    std::vector<std::string> command(argv + 2, argv + argc); /// 读取命令及参数

    std::cout << "Monitoring command: " << command[0] << std::endl;

#ifndef _WIN32

    /// 创建新的会话
    if (fork() > 0)
        exit(0); /// 父进程退出
    setsid();
    umask(0);

#endif


    while (true)
    {
#ifndef _WIN32
        startProcess(command);

        /// 等待子进程结束
        int status;
        wait(&status);   /// 阻塞直到子进程退出
        sleep(interval); /// 等待指定的时间

#else
        PROCESS_INFORMATION pi; /// 在这里定义 pi
        startProcess(command, pi);

        /// 等待子进程结束
        DWORD exitCode;
        while (true)
        {
            Sleep(1000);                                /// 每秒钟检查一次
            GetExitCodeProcess(pi.hProcess, &exitCode); /// 获取进程退出代码
            if (exitCode != STILL_ACTIVE)
            {
                break; /// 进程已结束
            }
        }

        CloseHandle(pi.hProcess); /// 关闭进程句柄
        Sleep(interval * 1000);   /// 等待指定的时间

#endif
    }

    return 0;
}
