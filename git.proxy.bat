@echo off
setlocal

REM ����������
if "%~1"=="" (
    echo ʹ�÷���: git_proxy.bat on|off
    exit /b 1
)

REM ���ô����URL���滻Ϊ��Ĵ����ַ�Ͷ˿�
set "proxy_url=http://127.0.0.1:7890"

if /i "%~1"=="on" (
    echo ���� Git �ֿ��local����...
    git config --local http.proxy %proxy_url%
    git config --local https.proxy %proxy_url%
    echo Git �����ѿ���: %proxy_url%
) else if /i "%~1"=="off" (
    echo �ر� Git �ֿ��local����...
    git config --local --unset http.proxy
    git config --local --unset https.proxy
    echo Git �����ѹر�
) else if /i "%~1"=="global" (
    if /i "%~2"=="on" (
    echo ���� Git �ֿ��global����...
    git config --global http.proxy %proxy_url%
    git config --global https.proxy %proxy_url%
    echo Git �����ѿ���: %proxy_url%
) else if /i "%~2"=="off" (
    echo �ر� Git �ֿ��global����...
    git config --global --unset http.proxy
    git config --global --unset https.proxy
    echo Git �����ѹر�
) else (
    echo ��֧�ֵĲ���: %~2
    echo ʹ�÷���: git_proxy.bat global on|off
    exit /b 1
)
) else (
    echo ��֧�ֵĲ���: %~1
    echo ʹ�÷���: git_proxy.bat on|off
    exit /b 1
)

endlocal
pause