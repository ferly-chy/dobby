#!/usr/bin/env python3
from __future__ import annotations

import argparse
import logging
import os
import shutil
import subprocess
import sys
from enum import StrEnum
from pathlib import Path
from typing import Final, Sequence

# Configure logging
logging.basicConfig(level=logging.INFO, format="%(asctime)s - %(levelname)s - %(message)s")
logger = logging.getLogger(__name__)

class LibraryType(StrEnum):
    SHARED = "shared"
    STATIC = "static"

class Platform(StrEnum):
    LINUX = "linux"
    ANDROID = "android"

PLATFORMS_ARCHS: Final[dict[Platform, list[str]]] = {
    Platform.LINUX: ["x86", "x86_64", "arm", "arm64"],
    Platform.ANDROID: ["x86", "x86_64", "armeabi-v7a", "arm64-v8a"],
}

ANDROID_ARCH_MAP: Final[dict[str, str]] = {
    "arm": "armeabi-v7a",
    "arm64": "arm64-v8a",
    "v7a": "armeabi-v7a",
    "v8a": "arm64-v8a",
}

class PlatformBuilder:
    def __init__(
        self,
        project_dir: Path,
        library_type: LibraryType,
        platform: Platform,
        arch: str,
        cmake_dir: Path | None = None,
        llvm_dir: Path | None = None,
    ):
        self.project_dir = project_dir
        self.library_type = library_type
        self.platform = platform
        self.arch = arch
        self.cmake_build_type = "Release"

        self.build_root = self.project_dir / "build"
        self.cmake_build_dir = self.build_root / f"cmake-build-{platform}-{arch}"
        self.output_dir = self.build_root / platform / arch

        # Tool paths
        self.cmake = (cmake_dir / "bin" / "cmake") if cmake_dir else Path("cmake")
        
        self.clang = (llvm_dir / "bin" / "clang") if llvm_dir else Path("clang")
        self.clangxx = (llvm_dir / "bin" / "clang++") if llvm_dir else Path("clang++")

        self.cmake_args: list[str] = [
            f"-DCMAKE_BUILD_TYPE={self.cmake_build_type}",
            "-G Ninja", # Use Ninja by default
        ]
        
        if self.platform != Platform.ANDROID:
            self.cmake_args += [
                f"-DCMAKE_C_COMPILER={self.clang}",
                f"-DCMAKE_CXX_COMPILER={self.clangxx}",
            ]

        self.shared_output_name = "libdobby.so"
        self.static_output_name = "libdobby.a"

    def setup_platform_args(self):
        """To be overridden by subclasses."""
        pass

    def generate(self):
        self.cmake_build_dir.mkdir(parents=True, exist_ok=True)
        flat_args = []
        for arg in self.cmake_args:
            if arg.startswith("-G"):
                flat_args.extend(arg.split(maxsplit=1))
            else:
                flat_args.append(arg)

        cmd = [
            str(self.cmake),
            "-S", str(self.project_dir),
            "-B", str(self.cmake_build_dir),
        ] + flat_args
        
        logger.info(f"Generating build system: {' '.join(cmd)}")
        subprocess.run(cmd, check=True)

    def build(self):
        self.output_dir.mkdir(parents=True, exist_ok=True)
        self.generate()

        build_cmd = [
            str(self.cmake),
            "--build", ".",
            "--clean-first",
            "--target", "dobby",
            "--target", "dobby_static",
            "--", "-j8"
        ]
        logger.info(f"Building targets: {' '.join(build_cmd)}")
        subprocess.run(build_cmd, cwd=self.cmake_build_dir, check=True)

        # Copy artifacts
        for name in [self.shared_output_name, self.static_output_name]:
            src = self.cmake_build_dir / name
            if src.exists():
                logger.info(f"Copying {src} to {self.output_dir}")
                shutil.copy2(src, self.output_dir / name)
            else:
                logger.warning(f"Artifact not found: {src}")

class LinuxBuilder(PlatformBuilder):
    def setup_platform_args(self):
        self.cmake_args += [
            "-DCMAKE_SYSTEM_NAME=Linux",
            f"-DCMAKE_SYSTEM_PROCESSOR={self.arch}",
        ]

class AndroidBuilder(PlatformBuilder):
    def __init__(self, ndk_dir: Path, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.ndk_dir = ndk_dir

    def setup_platform_args(self):
        api_level = 21
        toolchain = self.ndk_dir / "build" / "cmake" / "android.toolchain.cmake"
        if not toolchain.exists():
             logger.error(f"Android toolchain not found: {toolchain}")
             sys.exit(1)
             
        self.cmake_args += [
            f"-DCMAKE_TOOLCHAIN_FILE={toolchain}",
            f"-DANDROID_ABI={self.arch}",
            f"-DANDROID_NDK={self.ndk_dir}",
            f"-DANDROID_PLATFORM=android-{api_level}",
            "-DANDROID_STL=c++_static",
            f"-DCMAKE_ANDROID_NDK={self.ndk_dir}",
            f"-DCMAKE_ANDROID_ARCH_ABI={self.arch}",
            f"-DCMAKE_SYSTEM_NAME=Android",
            f"-DCMAKE_SYSTEM_VERSION={api_level}",
        ]

def main():
    parser = argparse.ArgumentParser(description="Dobby Platform Builder (Android Exclusive Fork)")
    parser.add_argument("--platform", type=Platform, choices=list(Platform), required=True)
    parser.add_argument("--arch", type=str, required=True, help="Architecture (comma-separated or 'all')")
    parser.add_argument("--type", type=LibraryType, choices=list(LibraryType), default=LibraryType.STATIC)
    parser.add_argument("--ndk", type=Path, help="Android NDK directory")
    parser.add_argument("--cmake", type=Path, help="Custom CMake directory")
    parser.add_argument("--llvm", type=Path, help="Custom LLVM directory")
    
    args = parser.parse_args()

    project_root = Path(__file__).resolve().parent.parent
    if not (project_root / "CMakeLists.txt").exists():
        logger.error(f"Execution failed: {project_root} is not the Dobby project root.")
        sys.exit(1)

    if args.arch == "all":
        selected_archs = PLATFORMS_ARCHS[args.platform]
    else:
        selected_archs = [a.strip() for a in args.arch.split(",")]

    if args.platform == Platform.ANDROID:
        mapped_archs = []
        for a in selected_archs:
            if mapped := ANDROID_ARCH_MAP.get(a):
                mapped_archs.append(mapped)
            else:
                mapped_archs.append(a)
        selected_archs = mapped_archs

    valid_archs = PLATFORMS_ARCHS.get(args.platform, [])
    for a in selected_archs:
        if a not in valid_archs:
             logger.error(f"Invalid architecture '{a}' for platform '{args.platform}' (Available: {valid_archs})")
             sys.exit(1)

    if args.platform == Platform.ANDROID and not args.ndk:
        logger.error("Android NDK directory (--ndk) is required for Android platform.")
        sys.exit(1)

    builder_instance: PlatformBuilder | None = None
    for arch in selected_archs:
        match args.platform:
            case Platform.ANDROID:
                builder_instance = AndroidBuilder(args.ndk, project_root, args.type, args.platform, arch, args.cmake, args.llvm)
            case Platform.LINUX:
                builder_instance = LinuxBuilder(project_root, args.type, args.platform, arch, args.cmake, args.llvm)

        if builder_instance:
            builder_instance.setup_platform_args()
            logger.info(f"Starting build for {args.platform} ({arch})")
            builder_instance.build()

if __name__ == "__main__":
    main()
