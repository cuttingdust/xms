@echo off
setlocal
setlocal EnableDelayedExpansion

REM 检查参数数量
if "%~1"=="" (
    echo "使用方法: git_proxy.bat add|del|list|clean"
    exit /b 1
)

::set git_add_path=%cd%
set "git_add_path=!cd:\=/!"

if /i "%~1"=="clean" (
    echo 清空Git工作目录...
    git config --global --unset-all safe.directory
    echo Git 工作目录已清空
)else if /i "%~1"=="list" (
	echo Git 仓库的工作目录:
	git config --get-all safe.directory
)else if /i "%~1"=="add" (
    echo 添加到 Git 仓库的工作目录
    git config --global --add safe.directory %git_add_path%
    echo Git 工作目录已添加: %git_add_path%
)else if /i "%~1"=="del" (
    echo 移除 从Git仓库的工作目录
    git config --global --unset safe.directory %git_add_path%
    echo Git 工作目录已删除：%git_add_path% del...
)
endlocal
pause