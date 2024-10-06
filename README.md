# QuickIM
> *这是一个尝试自行实现即时通讯软件的小项目*

![License](https://img.shields.io/github/license/Voltline/QuickIM)
![Issues](https://img.shields.io/github/issues/Voltline/QuickIM)
![Stars](https://img.shields.io/github/stars/Voltline/QuickIM)

## 项目概况
* 项目开始于2024年4月23日
* 项目基于MIT协议开源

## Project Brief Introduction
* The project starts on April 23, 2024
* The project based on MIT protocol

## 注意事项
* 此项目基于POSIX下的套接字、多线程库开发，仅能在UNIX和类UNIX系统下运行
* 推荐使用各种Linux发行版与macOS
* 本项目使用OpenSSL库，请务必在构建前安装：
```bash
# macOS
brew install openssl

# Debian/Ubuntu
sudo apt-get install openssl

# CentOS
sudo yum install openssl
```
* 本项目使用的nlohnmann json库、spdlog库已包含在项目中，无需额外安装，而boost、boost/mysql库需要自行安装
* 推荐使用vcpkg安装boost以及boost/mysql库

## Caution
* The project based on POSIX socket and pthread library. So it can just run on UNIX/UNIX-like OS.
* Recommend to use various Linux releases or macOS to run this project
* You need to prepare OpenSSL environment before building this project:
```bash
# macOS
brew install openssl

# Debian/Ubuntu
sudo apt-get install openssl

# CentOS
sudo yum install openssl
```
* The nlohmann json library and spdlog library are included in this project, so you don't need to install them. But you need to install boost and boost/mysql library by yourself.
* Recommend to use vcpkg to install boost and boost/mysql library

## 项目信息/About QuickIM
### 语言/Lang
* 语言/Lang：C++
### 第三方库/Third-Party Library
* nlohmann json
* spdlog
* OpenSSL
* boost
* boost/mysql
### 开源协议/Open-Source Protocol
* MIT

