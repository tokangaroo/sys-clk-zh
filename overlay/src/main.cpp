/*
 * --------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <p-sam@d3vs.net>, <natinusala@gmail.com>, <m4x@m4xw.net>
 * wrote this file. As long as you retain this notice you can do whatever you
 * want with this stuff. If you meet any of us some day, and you think this
 * stuff is worth it, you can buy us a beer in return.  - The sys-clk authors
 * --------------------------------------------------------------------------
 */

#define TESLA_INIT_IMPL
#include <tesla.hpp>

#include "ui/gui/fatal_gui.h"
#include "ui/gui/main_gui.h"

class AppOverlay : public tsl::Overlay
{
    public:
        AppOverlay() {}
        ~AppOverlay() {}

        virtual void exitServices() override {
            sysclkIpcExit();
        }

        virtual std::unique_ptr<tsl::Gui> loadInitialGui() override
        {
            uint32_t apiVersion;
            smInitialize();

            tsl::hlp::ScopeGuard smGuard([] { smExit(); });

            if(!sysclkIpcRunning())
            {
                return initially<FatalGui>(
                    "sys-clk 未在运行。\n\n"
                    "\n"
                    "请确认系统模块已正确\n\n"
                    "安装并启用。",
                    ""
                );
            }

            if(R_FAILED(sysclkIpcInitialize()) || R_FAILED(sysclkIpcGetAPIVersion(&apiVersion)))
            {
                return initially<FatalGui>(
                    "无法连接到 sys-clk。\n\n"
                    "\n"
                    "请确认系统模块已正确\n\n"
                    "安装并启用。",
                    ""
                );
            }

            if(SYSCLK_IPC_API_VERSION != apiVersion)
            {
                return initially<FatalGui>(
                    "此插件与当前运行的\n\n"
                    "sys-clk 版本不兼容。\n\n"
                    "\n"
                    "请确认全部文件\n\n"
                    "均已安装且为最新版本。",
                    ""
                );
            }

            return initially<MainGui>();
        }
};

int main(int argc, char **argv)
{
    return tsl::loop<AppOverlay>(argc, argv);
}
