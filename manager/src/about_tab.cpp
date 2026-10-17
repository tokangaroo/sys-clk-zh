/*
    sys-clk manager, a sys-clk frontend homebrew
    Copyright (C) 2019-2020  natinusala
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

#include "about_tab.h"

#include "logo.h"
#include "ipc/ipc.h"
#include "ipc/client.h"

#include <borealis.hpp>

AboutTab::AboutTab()
{
    this->addView(new Logo(LogoStyle::ABOUT));

    // Subtitle
    brls::Label *subTitle = new brls::Label(
        brls::LabelStyle::REGULAR,
        "Nintendo Switch 超频／降频系统模块与前端应用，由 RetroNX Team 开发", 
        true
    );
    subTitle->setHorizontalAlign(NVG_ALIGN_CENTER);
    this->addView(subTitle);

    // Copyright
    brls::Label *copyright = new brls::Label(
        brls::LabelStyle::DESCRIPTION,
        "系统模块遵循 Beerware 许可\n" \
        "前端应用遵循 GPL-3.0 许可\n" \
        "\u00A9 2019 - 2020 natinusala, p-sam, m4xw",
        true
    );
    copyright->setHorizontalAlign(NVG_ALIGN_CENTER);
    this->addView(copyright);

    // Links
    this->addView(new brls::Header("相关链接"));
    brls::Label *links = new brls::Label(
        brls::LabelStyle::SMALL,
        "\uE016  用户手册与源码见我们的 GitHub 仓库\n" \
        "\uE016  sys-clk 管理器基于 Borealis —— 硬件加速 UI 库\n" \
        "\uE016  加入 RetroNX Discord 可获取支持、提需求或闲聊",
        true
    );
    this->addView(links);
}
