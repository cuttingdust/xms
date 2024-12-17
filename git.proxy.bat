@echo off
setlocal

REM 检查参数数量
if "%~1"=="" (
    echo 使用方法: git_proxy.bat on|off
    exit /b 1
)

REM 设置代理的URL，替换为你的代理地址和端口
set "proxy_url=http://127.0.0.1:7890"

if /i "%~1"=="on" (
    echo 开启 Git 仓库的local代理...
    git config --local http.proxy %proxy_url%
    git config --local https.proxy %proxy_url%
    echo Git 代理已开启: %proxy_url%
) else if /i "%~1"=="off" (
    echo 关闭 Git 仓库的local代理...
    git config --local --unset http.proxy
    git config --local --unset https.proxy
    echo Git 代理已关闭
) else if /i "%~1"=="global" (
    if /i "%~2"=="on" (
    echo 开启 Git 仓库的global代理...
    git config --global http.proxy %proxy_url%
    git config --global https.proxy %proxy_url%
    echo Git 代理已开启: %proxy_url%
) else if /i "%~2"=="off" (
    echo 关闭 Git 仓库的global代理...
    git config --global --unset http.proxy
    git config --global --unset https.proxy
    echo Git 代理已关闭
) else (
    echo 不支持的参数: %~2
    echo 使用方法: git_proxy.bat global on|off
    exit /b 1
)
) else (
    echo 不支持的参数: %~1
    echo 使用方法: git_proxy.bat on|off
    exit /b 1
)

endlocal
pause