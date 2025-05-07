#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>


void startProcess(const std::vector<std::string>& command)
{
    pid_t pid = fork();
    if (pid == 0)
    { // 子进程
        // 使用 execvp 执行命令
        std::vector<char*> args;
        for (const auto& arg : command)
        {
            args.push_back(const_cast<char*>(arg.c_str()));
        }
        args.push_back(nullptr); // execvp 需要以 nullptr 结束参数列表
        execvp(args[0], args.data());
        // 如果 execvp 返回，说明出错
        std::cerr << "Failed to execute: " << command[0] << std::endl;
        exit(1);
    }
    else if (pid < 0)
    {
        std::cerr << "Fork failed" << std::endl;
    }
    // 父进程继续
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cout << "Usage: ./watchdog <interval> <command> [args...]" << std::endl;
        return -1;
    }

    int                      interval = atoi(argv[1]);
    std::vector<std::string> command(argv + 2, argv + argc); // 读取命令及参数

    std::cout << "Monitoring command: " << command[0] << std::endl;

    // 创建新的会话
    if (fork() > 0)
        exit(0); // 父进程退出
    setsid();
    umask(0);

    while (true)
    {
        startProcess(command);

        // 等待子进程结束
        int status;
        wait(&status);   // 阻塞直到子进程退出
        sleep(interval); // 等待指定的时间
    }

    return 0;
}
