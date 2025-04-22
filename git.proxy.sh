#! /bin/bash

# 检查参数数量
if [ -z "$1" ];then 
  ehco "使用方法: git_proxy.sh on|off|global on|global off"
fi

# 设置代理的URL,替换为你的代理地址和端口
proxy_url="http://127.0.0.1:7890"

if [ "$1" == "on" ]; then
    echo "开启 Git 仓库的 local 代理..."
    git config --local http.proxy $proxy_url
    git config --local https.proxy $proxy_url
    echo "Git 代理已开启: $proxy_url"
elif [ "$1" == "off" ]; then
    echo "关闭 Git 仓库的 local 代理..."
    git config --local --unset http.proxy
    git config --local --unset https.proxy
    echo "Git 代理已关闭"
elif [ "$1" == "global" ]; then
    if [ "$2" == "on" ]; then
        echo "开启 Git 仓库的 global 代理..."
        git config --global http.proxy $proxy_url
        git config --global https.proxy $proxy_url
        echo "Git 代理已开启: $proxy_url"
    elif [ "$2" == "off" ]; then
        echo "关闭 Git 仓库的 global 代理..."
        git config --global --unset http.proxy
        git config --global --unset https.proxy
        echo "Git 代理已关闭"
    else
        echo "不支持的参数: $2"
        echo "使用方法: git_proxy.sh global on|off"
        exit 1
    fi
else
    echo "不支持的参数: $1"
    echo "使用方法: git_proxy.sh on|off|global on|global off"
    exit 1
fi
