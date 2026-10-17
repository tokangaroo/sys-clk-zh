#!/usr/bin/env bash
# 在 devkitpro/devkita64 容器内编译 sys-clk-manager
#
# 1. 该镜像的 pacman 不在 PATH 中，这里逐个位置探测。
# 2. 镜像通常已自带 switch 的 portlibs（glfw / mesa / libdrm_nouveau / zlib），
#    先检查本地是否已存在，齐全就完全不联网，避开 pkg.devkitpro.org 的 403。
# 3. 确实缺了才联网安装；数据库同步失败则退回用镜像内缓存重试。
set -e

VER="${VER:-2.0.1}"

echo "=== 环境 ==="
echo "PWD=$(pwd)"
echo "DEVKITPRO=${DEVKITPRO:-<unset>}"
echo "VERSION=$VER"

export PATH="/opt/devkitpro/pacman/bin:/opt/devkitpro/tools/bin:/opt/devkitpro/devkitA64/bin:$PATH"

# ---------- 1. 检查图形库是否已就绪 ----------
LIB_DIRS=""
for d in "$DEVKITPRO/portlibs/switch/lib" "$DEVKITPRO/portlibs/lib" "$DEVKITPRO/libnx/lib"; do
    [ -d "$d" ] && LIB_DIRS="$LIB_DIRS $d"
done
echo "搜索库目录:$LIB_DIRS"

MISSING=""
for lib in libglfw3.a libEGL.a libglapi.a libdrm_nouveau.a libz.a; do
    found=no
    for d in $LIB_DIRS; do
        if [ -f "$d/$lib" ]; then found=yes; break; fi
    done
    if [ "$found" = yes ]; then
        echo "  [有] $lib"
    else
        echo "  [缺] $lib"
        MISSING="$MISSING $lib"
    fi
done

# ---------- 2. 缺了才装 ----------
if [ -z "$MISSING" ]; then
    echo "图形库齐全，跳过 pacman"
else
    PACMAN=""
    for c in pacman dkp-pacman; do
        if command -v "$c" >/dev/null 2>&1; then PACMAN="$c"; break; fi
    done
    if [ -z "$PACMAN" ]; then
        for p in /opt/devkitpro/pacman/bin/pacman /usr/bin/pacman; do
            if [ -x "$p" ]; then PACMAN="$p"; break; fi
        done
    fi
    if [ -z "$PACMAN" ]; then
        echo "!! 缺少$MISSING 且找不到 pacman"
        ls -1 /opt/devkitpro 2>/dev/null || true
        find / -maxdepth 5 -name 'pacman*' \( -type f -o -type l \) 2>/dev/null | head -20
        exit 1
    fi

    echo "使用 $PACMAN，开始安装:$MISSING"
    PKGS="switch-glfw switch-mesa switch-libdrm_nouveau switch-zlib"

    if ! "$PACMAN" -Syy --noconfirm --needed $PKGS > /tmp/deps.log 2>&1; then
        echo "!! 数据库同步安装失败，改用镜像内缓存重试"
        if ! "$PACMAN" -S --noconfirm --needed $PKGS >> /tmp/deps.log 2>&1; then
            echo "!! 仍然失败，末尾日志："
            tail -40 /tmp/deps.log
            exit 1
        fi
    fi
    echo "依赖安装完成"
fi

# ---------- 3. 编译 ----------
echo "=== 编译 manager ==="
make -j"$(nproc)" TARGET_VERSION="$VER"
