#!/bin/bash
set -e

# Konfigurasi Path
PROJECT_ROOT=$(pwd)
BUILD_DIR="build/cmake-build-all-features"
ARCH="arm64-v8a"
NDK=$ANDROID_NDK_HOME

if [ -z "$NDK" ]; then
    echo "Error: ANDROID_NDK_HOME belum diatur."
    exit 1
fi

echo "[*] Membersihkan build lama..."
rm -rf $BUILD_DIR
mkdir -p $BUILD_DIR
cd $BUILD_DIR

echo "[*] Mengonfigurasi CMake dengan SELURUH FITUR AKTIF..."
cmake $PROJECT_ROOT \
    -DCMAKE_BUILD_TYPE=Release \
    -DDOBBY_BUILD_TEST=ON \
    -DDOBBY_BUILD_EXAMPLE=ON \
    -DPlugin.SymbolResolver=ON \
    -DPlugin.ImportTableReplace=ON \
    -DPlugin.Android.BionicLinkerUtil=ON \
    -DPlugin.KittyMemory=ON \
    -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=$ARCH \
    -DANDROID_PLATFORM=android-21 \
    -DANDROID_STL=c++_static \
    -G Ninja

echo "[*] Memulai kompilasi seluruh target..."
ninja

echo -e "\n[*] --- Menjalankan Seluruh Pengujian --- [*]"
export LD_LIBRARY_PATH=$BUILD_DIR

# 1. Test VTable (Fitur Baru)
echo -e "\n[TEST 1] VTable Hooking (Auto Math & Calculate):"
./tests/test_vtable

# 2. Test Native (Hooking Dasar)
echo -e "\n[TEST 2] Native Hooking (dobby_hook):"
./tests/test_native

# 3. Test Instruksi Relokasi (ARM64)
# Catatan: Ini membutuhkan libcapstone.so dan libunicorn.so di LD_LIBRARY_PATH jika di-link secara dynamic
# Di Termux, kita biasanya sudah punya di /data/data/com.termux/files/usr/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/data/data/com.termux/files/usr/lib
echo -e "\n[TEST 3] Instruction Relocation (ARM64):"
./tests/test_insn_relo_arm64

echo -e "\n[*] Seluruh pengujian selesai dijalankan!"
