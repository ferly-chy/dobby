#!/bin/bash

# Dobby Test Runner Script
# Digunakan untuk membangun dan menjalankan feature_test secara otomatis

set -e

# Tentukan direktori
PROJECT_ROOT=$(pwd)
BUILD_DIR="build/tests"

echo ">>> Menyiapkan lingkungan build di $BUILD_DIR..."
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# Konfigurasi CMake (Membangun untuk host lokal karena kita di Termux)
echo ">>> Mengonfigurasi proyek dengan DOBBY_BUILD_TEST=ON..."
cmake ../.. -DDOBBY_BUILD_TEST=ON -DDOBBY_BUILD_EXAMPLE=OFF -G Ninja

# Build target feature_test
echo ">>> Membangun feature_test..."
ninja feature_test

# Jalankan hasil pengujian
echo ""
echo "===================================================="
echo "          MENJALANKAN DOBBY FEATURE TEST            "
echo "===================================================="
echo ""

./tests/feature_test

echo ""
echo "===================================================="
echo ">>> Semua tes selesai dijalankan."
