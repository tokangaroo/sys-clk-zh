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

#include "app_profiles_tab.h"

#include "app_profile_frame.h"

#include "ipc/client.h"
#include "ipc/ipc.h"

#include <cstring>
#include <cstdio>
#include <zlib.h>

#include "utils.h"

// [21.0.0+] 起，NACP 可以把 16 个语言条目整体 DEFLATE 压缩（wbits = -15）
// 存放在 lang_data 这个 union 里，并把 titles_data_format 置为 1。
// 本仓库为绕过编译问题回退到了 2.0.1 的列表逻辑，它不认识这种格式，
// 会把压缩数据当字符串读，结果只显示一两个乱码字符（例如 "E"）。
//
// 这里按 NACP 的固定 ABI 布局直接读字节偏移，不依赖 libnx 是否暴露该成员：
//   0x0000  NacpLanguageEntry[16] | { u16 buffer_size; u8 buffer[0x2FFE]; }
//   0x3215  u8 titles_data_format
// 偏移由 libnx nacp.h 中 reserved_x30f5 / reserved_x318a 等保留字段名交叉校验得出。
#define NACP_OFF_TITLES_DATA_FORMAT 0x3215
#define NACP_OFF_LANG_REGION_SIZE   0x3000
#define NACP_MAX_LANG_ENTRIES       32

static bool nacpGetLanguageEntryCompat(NacpStruct* nacp, NacpLanguageEntry** out)
{
    const u8* raw = reinterpret_cast<const u8*>(nacp);
    const u8 format = raw[NACP_OFF_TITLES_DATA_FORMAT];

    // 旧格式：交给 libnx 处理
    if (format == 0)
        return R_SUCCEEDED(nacpGetLanguageEntry(nacp, out)) && *out != nullptr;

    if (format != 1)
        return false;

    static NacpLanguageEntry decompressed[NACP_MAX_LANG_ENTRIES];
    static NacpStruct scratch;

    const u16 compressedSize = *reinterpret_cast<const u16*>(raw);

    z_stream stream = {};
    stream.next_in   = const_cast<Bytef*>(raw + sizeof(u16));
    stream.avail_in  = compressedSize;
    stream.next_out  = reinterpret_cast<Bytef*>(decompressed);
    stream.avail_out = sizeof(decompressed);

    if (inflateInit2(&stream, -15) != Z_OK)
        return false;

    const int ret = inflate(&stream, Z_FINISH);
    inflateEnd(&stream);

    if (ret != Z_STREAM_END)
        return false;

    // 用解压结果顶替语言区，再交给 libnx 自己的语言选择与回退逻辑
    memcpy(&scratch, nacp, sizeof(scratch));
    memcpy(&scratch, decompressed, NACP_OFF_LANG_REGION_SIZE);
    reinterpret_cast<u8*>(&scratch)[NACP_OFF_TITLES_DATA_FORMAT] = 0;

    return R_SUCCEEDED(nacpGetLanguageEntry(&scratch, out)) && *out != nullptr;
}

// 名称兜底：解压失败或内容异常时改用 Title ID，避免显示乱码
static bool nacpNameLooksValid(const char* name)
{
    if (!name)
        return false;

    const size_t len = strlen(name);
    if (len < 2)
        return false;

    for (size_t i = 0; i < len; i++)
    {
        const unsigned char c = (unsigned char)name[i];
        if (c < 0x20 || c == 0x7F)
            return false;
    }

    return true;
}

#define PROFILE_BADGE "\uE3E0"

AppProfilesTab::AppProfilesTab()
{
    // Filter toggle
    this->filterListItem = new brls::ToggleListItem("显示未配置的游戏", this->showEmptyProfiles, "", "开", "关");
    filterListItem->getClickEvent()->subscribe([this](View* v)
    {
        this->refreshFilter();
    });
    this->addView(filterListItem);

    // Spacing
    this->addView(new brls::ListItemGroupSpacing());

    // Applications list
    NsApplicationRecord record;
    uint64_t tid;
    NsApplicationControlData controlData;
    NacpLanguageEntry* langEntry = NULL;

    Result rc;
    size_t i            = 0;
    int recordCount     = 0;
    size_t controlSize  = 0;

    while (true)
    {
        // Record
        rc = nsListApplicationRecord(&record, sizeof(record), i, &recordCount);
        if (R_FAILED(rc))
        {
            errorResult("nsListApplicationRecord", rc);
            break;
        }

        if(recordCount <= 0)
            break;

        tid = record.application_id;

        // Control data
        rc = nsGetApplicationControlData(NsApplicationControlSource_Storage, tid, &controlData, sizeof(controlData), &controlSize);
        if (R_FAILED(rc))
        {
            errorResult("nsGetApplicationControlData", rc);
            break;
        }

        // Language entry（[21.0.0+] 可能为压缩存储，失败时只跳过该条目）
        if (!nacpGetLanguageEntryCompat(&controlData.nacp, &langEntry))
        {
            i++;
            continue;
        }

        Title* title = (Title*) malloc(sizeof(Title));
        title->tid = tid;

        memset(title->name, 0, sizeof(title->name));
        if (nacpNameLooksValid(langEntry->name))
            strncpy(title->name, langEntry->name, sizeof(title->name)-1);
        else
            snprintf(title->name, sizeof(title->name), "%016lX", tid);

        memcpy(title->icon, controlData.icon, sizeof(title->icon));

        // Profile
        rc = sysclkIpcGetProfileCount(tid, &title->profileCount);
        if (R_FAILED(rc))
        {
            errorResult("sysclkIpcGetProfileCount", rc);
            free(title);
            break;
        }

        // Add the ListItem
        brls::ListItem *listItem = new brls::ListItem(formatListItemTitle(std::string(title->name)), "", formatTid(title->tid));

        title->listItem = listItem;

        this->items.push_back(listItem);

        if (title->profileCount > 0)
        {
            listItem->setValue(PROFILE_BADGE);
            this->profilesItems.push_back(listItem);
        }
        else
        {
            this->emptyProfilesItems.push_back(listItem);

            if (!this->showEmptyProfiles)
                listItem->collapse(false);
        }

        listItem->setThumbnail(title->icon, sizeof(title->icon));

        listItem->getClickEvent()->subscribe([this, title](View* view) {
            this->editingTitle = title;
            AppProfileFrame* profileFrame = new AppProfileFrame(title);
            brls::Application::pushView(profileFrame, brls::ViewAnimation::SLIDE_LEFT);
        });

        this->addView(listItem);
        this->titles.push_back(title);

        i++;
    }

    // Empty list message
    this->emptyListLabel = new brls::Label(brls::LabelStyle::REGULAR, "", true);
    this->addView(emptyListLabel);
    this->updateEmptyListLabel(false);
}

void AppProfilesTab::updateEmptyListLabel(bool animate)
{
    if (this->items.empty())
    {
        this->emptyListLabel->setText("\uE140  You don't have any application installed on your Nintendo Switch.");
        this->emptyListLabel->show([](){}, animate);
    }
    else if (!this->showEmptyProfiles && this->profilesItems.empty())
    {
        this->emptyListLabel->setText("\uE140  You don't have any application with a defined profile at the moment.");
        this->emptyListLabel->show([](){}, animate);
    }
    else
    {
        this->emptyListLabel->hide([](){}, animate);
    }
}

AppProfilesTab::~AppProfilesTab()
{
    for (Title* title : this->titles)
        free(title);

    this->titles.clear();
}


void AppProfilesTab::willAppear(bool resetState)
{
    if (this->editingTitle != nullptr)
    {
        bool hadProfiles = this->editingTitle->profileCount > 0;

        Result rc = sysclkIpcGetProfileCount(this->editingTitle->tid, &this->editingTitle->profileCount);

        bool hasProfiles = this->editingTitle->profileCount > 0;

        if (R_FAILED(rc))
        {
            errorResult("sysclkIpcGetProfileCount", rc);
            this->editingTitle = nullptr;
            return;
        }

        // Update the profile badge
        if (hasProfiles)
            this->editingTitle->listItem->setValue(PROFILE_BADGE);
        else
            this->editingTitle->listItem->setValue("");

        // Update lists

        // Remove from emptyProfilesItems if it didn't have a profile
        // but has one now
        // Add to profilesItems
        if (!hadProfiles && hasProfiles)
        {
            this->emptyProfilesItems.erase(std::remove(this->emptyProfilesItems.begin(), this->emptyProfilesItems.end(), this->editingTitle->listItem), this->emptyProfilesItems.end());
            this->profilesItems.push_back(this->editingTitle->listItem);
        }

        // Add to emptyProfilesItems if it had a profile but doesn't
        // has one now
        // Remove from profilesItems
        if (hadProfiles && !hasProfiles)
        {
            this->emptyProfilesItems.push_back(this->editingTitle->listItem);
            this->profilesItems.erase(std::remove(this->profilesItems.begin(), this->profilesItems.end(), this->editingTitle->listItem), this->profilesItems.end());
        }

        // Refresh the filter
        this->refreshFilter();

        // Cleanup
        this->editingTitle = nullptr;
    }
}

void AppProfilesTab::refreshFilter()
{
    this->showEmptyProfiles = this->filterListItem->getToggleState();

    for (brls::ListItem *listItem : this->emptyProfilesItems)
    {
        if (this->showEmptyProfiles)
            listItem->expand();
        else
            listItem->collapse();

        this->updateEmptyListLabel();
    }
}
