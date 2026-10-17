# sys-clk 简体中文版

> [retronx-team/sys-clk](https://github.com/retronx-team/sys-clk) 的简体中文汉化分支，含 Tesla 浮层（overlay）与管理器（manager）两个前端。
> 仅替换界面文案与字体加载，**不改变任何超频逻辑**。

## 一、本项目做了什么

| 项目 | 说明 |
|---|---|
| 上游 | `retronx-team/sys-clk` |
| 分支 | `develop` |
| 版本号 | `2.0.1`（取自最近的上游标签） |
| 汉化范围 | `sys-clk-overlay.ovl` + `sys-clk-manager.nro` |
| 改动规模 | 18 个源文件，仅字符串与字体加载 |

### 下载

在 [Actions](../../actions) 页面选最近一次成功构建，下载 Artifacts：

- `sys-clk-overlay-zh` → `sys-clk-overlay.ovl`
- `sys-clk-manager-zh` → `sys-clk-manager.nro`

版本号由 CI 从上游标签推导，与官方发行版一致。

### 安装

```
sys-clk-overlay.ovl   →  sd:/switch/.overlays/
sys-clk-manager.nro   →  sd:/switch/
```

需配合 sys-clk 系统模块（Title ID `00FF0000636C6BFF`）使用。
建议先改名并行存放（`sys-clk-overlay-zh.ovl` / `sys-clk-manager-zh.nro`），与原版对比后再替换。

### 汉化内容

**Tesla 浮层**

| 原文 | 中文 |
|---|---|
| Enable | 启用超频 |
| Edit app profile | 编辑游戏配置 |
| Advanced | 高级设置 |
| Temporary overrides | 临时超频 |
| Do not override | 不覆盖 |
| Skin: | 外壳: |
| Battery Power | 电池功率 |
| Now / Avg | 当前 / 平均 |

**管理器**：状态 / 游戏配置 / 高级设置 / 频率参考 / 关于，五个标签页全部中文化。

**共用显示名**：`sysclkFormatProfile`、`sysclkFormatModule`、`sysclkFormatConfigValue` 的显示名改为中文（底座 / 掌机 / 原装充电 等）。
冒号后的内部 key 保持不变：

```c
return pretty ? "底座" : "docked";
```

`docked`、`handheld`、`poll_interval_ms` 等会被写入配置文件，因此**不可翻译**，配置读写完全不受影响。

### 两处技术改动

1. **字体回退**：libtesla 与 borealis 原本都不保证加载简体中文字体，中文会渲染成方块。已补上 `PlSharedFontType_ChineseSimplified` 兜底，系统语言非中文时同样生效。
2. **自适应布局**：浮层状态栏标签改为按实测文字宽度右对齐到数值列，数值列坐标保持不变，中英文都能对齐。

### 关于一处文件回退与其修复

上游 `develop` 的 `manager/src/app_profiles_tab.cpp` 引用了 `NacpStruct.titles_data_format` 与 `NacpStruct.lang_data`，而 libnx 官方仓库并不存在这两个成员，导致该分支当前无法编译。

本分支将该文件**整体回退到 `2.0.1` 版本**，未改动任何逻辑代码。

但 `2.0.1` 版本不认识 [21.0.0+] 的压缩 NACP——新 SDK 编译的游戏（例如旷野之息 1.9.0）会把游戏名以 DEFLATE 压缩存放，旧代码把压缩数据当字符串读，游戏列表里就显示成 `E` 这样的乱码。

本分支按 NACP 的固定 ABI 布局自行解压后再交给 libnx 的语言选择逻辑（不依赖 libnx 是否暴露这些成员），并加入名称校验兜底，异常时显示 Title ID。

### 许可

沿用上游：sysmodule 为 Beerware，manager（含 Borealis）为 GPL-3.0，overlay（含 libtesla）为 Beerware。
对 `manager/` 的修改属 GPL-3.0 衍生作品，需同样以 GPL-3.0 分发。

---

## 二、上游 README 中文翻译

### sys-clk

Switch 系统模块，可根据当前运行的游戏与底座状态设置 CPU / GPU / 内存频率。

#### 安装

以下步骤假设你的 Switch 已运行 Atmosphère，且系统已更新到至少最新稳定版。
把 `atmosphere` 和 `switch` 两个文件夹复制到 SD 卡根目录，提示覆盖时确认覆盖。如果是从头安装（不是升级），还需要复制 `config` 文件夹以包含默认配置。

**注意：** sys-clk-overlay 需要先安装并运行 [Tesla](https://gbatemp.net/threads/tesla-the-nintendo-switch-overlay-menu.557362/)。

#### 相关文件

* 配置文件，用于为每种底座状态和游戏单独设置频率，详见下文

	`/config/sys-clk/config.ini`

* 日志文件，启用日志后写入此处

	`/config/sys-clk/log.txt`

* 日志开关文件，文件存在即启用日志

	`/config/sys-clk/log.flag`

* CSV 文件，启用后记录游戏 ID、配置档、频率与温度

	`/config/sys-clk/context.csv`

* sys-clk 管理器（从 hbmenu 启动）

	`/switch/sys-clk-manager.nro`

* sys-clk 浮层（呼出 [Tesla 菜单](https://gbatemp.net/threads/tesla-the-nintendo-switch-overlay-menu.557362/) 后随时可用）

	`/switch/.overlays/sys-clk-overlay.ovl`

* sys-clk 核心系统模块

	`/atmosphere/contents/00FF0000636C6BFF/exefs.nsp`
	`/atmosphere/contents/00FF0000636C6BFF/flags/boot2.flag`

#### 配置

在 `/config/sys-clk/config.ini` 中，按下面的模板为每个应用添加预设：

```
[Application Title ID]
docked_cpu=
docked_gpu=
docked_mem=
handheld_charging_cpu=
handheld_charging_gpu=
handheld_charging_mem=
handheld_charging_usb_cpu=
handheld_charging_usb_gpu=
handheld_charging_usb_mem=
handheld_charging_official_cpu=
handheld_charging_official_gpu=
handheld_charging_official_mem=
handheld_cpu=
handheld_gpu=
handheld_mem=
```

* 把 `Application Title ID` 换成你要自定义的游戏或应用的 Title ID。
游戏 ID 列表见 [Switchbrew wiki](https://switchbrew.org/wiki/Title_list/Games)。
* 频率单位为 MHz，会自动缩放到最接近的可用值，见下方频率表。
* 任一键省略、留空或设为 0 时会被忽略，改用官方默认频率。
* 充电状态下，sys-clk 按以下顺序查找频率，取第一个找到的：
	1. 充电器专属配置（USB 或原装）`handheld_charging_usb_X` 或 `handheld_charging_official_X`
	2. 通用充电配置 `handheld_charging_X`
	3. 掌机配置 `handheld_X`

#### 示例 1：塞尔达传说 旷野之息

* 底座或充电时超频 CPU
* 掌机时把内存超到底座频率

整体帧率更平稳（例如克洛格森林场景）。

```
[01007EF00011E000]
docked_cpu=1224
handheld_charging_cpu=1224
handheld_mem=1600
```

#### 示例 2：Picross

* 掌机时降频以省电

```
[0100BA0003EEA000]
handheld_cpu=816
handheld_gpu=153
handheld_mem=800
```

#### 高级

`[values]` 段用于调整 sys-clk 的计时参数。除非你清楚自己在做什么，否则不需要改动这些值：

| 键 | 说明 | 默认值 |
|:--:|---|---:|
| **temp_log_interval_ms** | 温度记录间隔，毫秒（`0` 表示关闭） | 0 ms |
| **freq_log_interval_ms** | 实际频率记录间隔，毫秒（`0` 表示关闭） | 0 ms |
| **power_log_interval_ms** | 功耗记录间隔，毫秒（`0` 表示关闭） | 0 ms |
| **csv_write_interval_ms** | CSV 写入间隔，毫秒（`0` 表示关闭） | 0 ms |
| **poll_interval_ms** | 检查并应用配置的间隔，毫秒 | 300 ms |

#### 频率上限

为防止电池过度负荷，从配置请求的频率在应用前可能被限制，具体取决于当前状态：

|  | 掌机 | 充电（USB） | 充电（原装） | 底座 |
|:--:|:--:|:--:|:--:|:--:|
| **MEM** | - | - | - | - |
| **CPU** | - | - | - | - |
| **GPU** | 460 MHz* | 768 MHz | - | - |

\* Mariko 机型的掌机 GPU 上限提升至 614 MHz

#### 频率表（MHz）

**内存频率**

* 1600 → 官方底座、加速模式、最高频率
* 1331 → 官方掌机
* 1065
* 800
* 665

**CPU 频率**

* 1785 → 最高频率、加速模式
* 1683
* 1581
* 1428
* 1326
* 1224 → 开发者超频
* 1122
* 1020 → 官方底座与掌机
* 918
* 816
* 714
* 612

**GPU 频率**

* 921 → 最高频率
* 844
* 768 → 官方底座
* 691
* 614
* 537
* 460 → 掌机最高
* 384 → 官方掌机
* 307 → 官方掌机
* 230
* 153
* 76 → 加速模式

**备注：**

1. 掌机状态下 GPU 超频上限为 460MHz；充电时上限为 768MHz，使用原装充电器除外。
2. 高于 768MHz 的频率需要插原装充电器。

---

## 三、上游原版 README（未修改）

<sub>以下内容为上游原文，逐字节保留 / Original upstream README below, unmodified.</sub>

# sys-clk

Switch sysmodule allowing you to set cpu/gpu/mem clocks according to the running application and docked state.

## Installation

The following instructions assumes you have a Nintendo Switch running Atmosphère, updated to at least the latest stable version.
Copy the `atmosphere`, and `switch` folders at the root of your sdcard, overwriting files if prompted. Also copy the `config` folder if you're not updating, to include default settings.

**Note:** sys-clk-overlay requires to have [Tesla](https://gbatemp.net/threads/tesla-the-nintendo-switch-overlay-menu.557362/) installed and running

## Relevant files

* Config file allows one to set custom clocks per docked state and title id, described below

	`/config/sys-clk/config.ini`

* Log file where the logs are written if enabled

	`/config/sys-clk/log.txt`

* Log flag file enables log writing if file exists

	`/config/sys-clk/log.flag`

* CSV file where the title id, profile, clocks and temperatures are written if enabled

	`/config/sys-clk/context.csv`

* sys-clk manager app (accessible from the hbmenu)

	`/switch/sys-clk-manager.nro`

* sys-clk overlay (accessible from anywhere by invoking the [Tesla menu](https://gbatemp.net/threads/tesla-the-nintendo-switch-overlay-menu.557362/))

	`/switch/.overlays/sys-clk-overlay.ovl`
	
* sys-clk core sysmodule

	`/atmosphere/contents/00FF0000636C6BFF/exefs.nsp`
	`/atmosphere/contents/00FF0000636C6BFF/flags/boot2.flag`

## Config

Presets can be customized by adding them to the ini config file located at `/config/sys-clk/config.ini`, using the following template for each app 

```
[Application Title ID]
docked_cpu=
docked_gpu=
docked_mem=
handheld_charging_cpu=
handheld_charging_gpu=
handheld_charging_mem=
handheld_charging_usb_cpu=
handheld_charging_usb_gpu=
handheld_charging_usb_mem=
handheld_charging_official_cpu=
handheld_charging_official_gpu=
handheld_charging_official_mem=
handheld_cpu=
handheld_gpu=
handheld_mem=
```

* Replace `Application Title ID` with the title id of the game/application you're interested in customizing.
A list of games title id can be found in the [Switchbrew wiki](https://switchbrew.org/wiki/Title_list/Games).
* Frequencies are expressed in mhz, and will be scaled to the nearest possible values, described in the clock table below.
* If any key is omitted, value is empty or set to 0, it will be ignored, and stock clocks will apply.
* If charging, sys-clk will look for the frequencies in that order, picking the first found 
	1. Charger specific config (USB or Official) `handheld_charging_usb_X` or `handheld_charging_official_X`
	2. Non specific charging config `handheld_charging_X`
	3. Handheld config `handheld_X`

### Example 1: Zelda BOTW

* Overclock CPU when docked or charging
* Overclock MEM to docked clocks when handheld

Leads to a smoother framerate overall (ex: in the korok forest)

```
[01007EF00011E000]
docked_cpu=1224
handheld_charging_cpu=1224
handheld_mem=1600
```

### Example 2: Picross

* Underclocks on handheld to save battery

```
[0100BA0003EEA000]
handheld_cpu=816
handheld_gpu=153
handheld_mem=800
```

### Advanced

The `[values]` section allows you to alter timings in sys-clk, you should not need to edit any of these unless you know what you are doing. Possible values are:

| Key                     | Desc                                                                          | Default |
|:-----------------------:|-------------------------------------------------------------------------------|:-------:|
|**temp_log_interval_ms** | Defines how often sys-clk logs temperatures, in milliseconds (`0` to disable) | 0 ms    |
|**freq_log_interval_ms** | Defines how often sys-clk logs real freqs, in milliseconds (`0` to disable)   | 0 ms    |
|**power_log_interval_ms**| Defines how often sys-clk logs power usage, in milliseconds (`0` to disable)  | 0 ms    |
|**csv_write_interval_ms**| Defines how often sys-clk writes to the CSV, in milliseconds (`0` to disable) | 0 ms    |
|**poll_interval_ms**     | Defines how fast sys-clk checks and applies profiles, in milliseconds         | 300 ms  |


## Capping

To protect the battery from excessive strain, clocks requested from config may be capped before applying, depending on your current profile:

|       | Handheld | Charging (USB) | Charging (Official) | Docked |
|:-----:|:--------:|:--------------:|:-------------------:|:------:|
|**MEM**| -        | -              | -                   | -      |
|**CPU**| -        | -              | -                   | -      |
|**GPU**| 460 MHz* | 768 MHz        | -                   | -      |
*\* GPU handheld max for Mariko is increased to 614 MHz*

## Clock table (MHz)

### MEM clocks
* 1600 → official docked, boost mode, max clock
* 1331 → official handheld
* 1065
* 800
* 665

### CPU clocks
* 1785 → max clock, boost mode
* 1683
* 1581
* 1428
* 1326
* 1224 → sdev oc
* 1122
* 1020 → official docked & handheld
* 918
* 816
* 714
* 612

### GPU clocks
* 921 → max clock
* 844
* 768 → official docked
* 691
* 614
* 537
* 460 → max handheld
* 384 → official handheld
* 307 → official handheld
* 230
* 153
* 76 → boost mode

**Notes:**
1. GPU overclock is capped at 460MHz in handheld and capped at 768MHz if charging, unless you're using the official charger.
2. Clocks higher than 768MHz need the official charger is plugged in.
