@echo off
setlocal
setlocal EnableDelayedExpansion

REM ����������
if "%~1"=="" (
    echo "ʹ�÷���: git_proxy.bat add|del|list|clean"
    exit /b 1
)

::set git_add_path=%cd%
set "git_add_path=!cd:\=/!"

if /i "%~1"=="clean" (
    echo ���Git����Ŀ¼...
    git config --global --unset-all safe.directory
    echo Git ����Ŀ¼�����
)else if /i "%~1"=="list" (
	echo Git �ֿ�Ĺ���Ŀ¼:
	git config --get-all safe.directory
)else if /i "%~1"=="add" (
    echo ��ӵ� Git �ֿ�Ĺ���Ŀ¼
    git config --global --add safe.directory %git_add_path%
    echo Git ����Ŀ¼�����: %git_add_path%
)else if /i "%~1"=="del" (
    echo �Ƴ� ��Git�ֿ�Ĺ���Ŀ¼
    git config --global --unset safe.directory %git_add_path%
    echo Git ����Ŀ¼��ɾ����%git_add_path% del...
)
endlocal
pause