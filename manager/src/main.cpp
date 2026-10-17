/*
    sys-clk manager, a sys-clk frontend homebrew
    Copyright (C) 2019  natinusala
    Copyright (C) 2019  p-sam
    Copyright (C) 2019  m4xw

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <borealis.hpp>

#include "main_frame.h"
#include "logo.h"

#include "ipc/client.h"
#include "ipc/ipc.h"

int main(int argc, char* argv[])
{
    // Init the app
    if (!brls::Application::init(APP_TITLE))
    {
        brls::Logger::error("无法初始化 Borealis 界面");
        return EXIT_FAILURE;
    }

    // Setup verbose logging on PC
#ifndef __SWITCH__
    brls::Logger::setLogLevel(brls::LogLevel::DEBUG);
#endif

    if (brls::Application::loadFont(LOGO_FONT_NAME, LOGO_FONT_PATH) < 0)
    {
        brls::Logger::error("加载 logo 字体失败");
    }

    uint32_t apiVersion;

    // Check that sys-clk is running
    if (!sysclkIpcRunning())
    {
        brls::Logger::error("sys-clk 未在运行");
        brls::Application::crash("sys-clk 似乎没有运行，请确认已正确安装并启用。");
    }
    // Initialize sys-clk IPC client
    else if (R_FAILED(sysclkIpcInitialize()) || R_FAILED(sysclkIpcGetAPIVersion(&apiVersion)))
    {
        brls::Logger::error("无法初始化 sys-clk IPC 客户端");
        brls::Application::crash("无法连接到 sys-clk，请确认已正确安装并启用。");
    }
    else if (SYSCLK_IPC_API_VERSION != apiVersion)
    {
        brls::Logger::error("sys-clk IPC API version mismatch (expected: %u; actual: %u)", SYSCLK_IPC_API_VERSION, apiVersion);
        brls::Application::crash("管理器与当前运行的 sys-clk 系统模块版本不兼容，请确认已安装最新版本（可能需要重启）。");
    }
    else if (R_FAILED(cacheFreqList()))
    {
        brls::Logger::error("无法从 sys-clk 获取频率列表");
        brls::Application::crash("无法从 sys-clk 获取频率列表，请确认已安装最新版本（可能需要重启）。");
    }
    else
    {
        // Set version string
        char version[0x100] = {0};
        Result rc = sysclkIpcGetVersionString(version, sizeof(version));
        if (R_SUCCEEDED(rc))
        {
            brls::Application::setCommonFooter(std::string(version));
        }
        else
        {
            brls::Logger::error("无法获取 sys-clk 版本号");
            brls::Application::setCommonFooter("[未知]");
        }


        // Initialize services with a PC shim
        nsInitialize();

        // Create root view
        MainFrame *mainFrame = new MainFrame();

        // Add the root view to the stack
        brls::Application::pushView(mainFrame);
    }

    // Run the app
    while (brls::Application::mainLoop());

    // Exit
    nsExit();
    sysclkIpcExit();
    return EXIT_SUCCESS;
}
