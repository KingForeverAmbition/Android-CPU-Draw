#!/bin/bash

# 颜色
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
print_success() { echo -e "${GREEN}[OK]${NC} $1"; }
print_error() { echo -e "${RED}[ERROR]${NC} $1"; }
print_warning() { echo -e "${YELLOW}[WARN]${NC} $1"; }

PROJECT_NAME="CPUDrawDemo" # 文件名称
BUILD_DIR="build"
NDK_PATH="" # NDK路径，自己填

# 检查 NDK
if [ ! -d "$NDK_PATH" ]; then
    print_error "找不到 NDK: $NDK_PATH"
    print_info "请安装 NDK 或修改脚本中的路径"
    exit 1
fi

print_info "NDK 路径: $NDK_PATH"

# 清理构建
if [ "$1" == "clean" ]; then
    print_info "清理构建目录..."
    rm -rf $BUILD_DIR
    print_success "清理完成"
    exit 0
fi

# 创建构建目录
print_info "准备构建..."
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# 配置 CMake
print_info "配置 CMake..."
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=$NDK_PATH/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-30 \
    -DCMAKE_BUILD_TYPE=Release \
    -DANDROID_NDK=$NDK_PATH

[ $? -ne 0 ] && print_error "CMake 配置失败" && exit 1

print_success "配置完成"

# 编译
print_info "开始编译..."
make -j$(nproc)

[ $? -ne 0 ] && print_error "编译失败" && exit 1

print_success "编译完成"

# 检查输出
OUTPUT_FILE="bin/$PROJECT_NAME"
if [ -f "$OUTPUT_FILE" ]; then
    FILE_SIZE=$(du -h $OUTPUT_FILE | cut -f1)
    print_info "输出: $OUTPUT_FILE ($FILE_SIZE)"
    print_success "构建成功"
else
    print_error "找不到输出文件"
    exit 1
fi

cd ..
