import os
import shutil
import subprocess
import sys
import logging
import argparse

platforms = {
    "macos": ["x86_64", "arm64"],
    "iphoneos": ["arm64"],
    "linux": ["x86", "x86_64", "arm", "arm64"],
    "android": ["armeabi-v7a", "arm64-v8a", "x86", "x86_64"]
}

class PlatformBuilder:
    def __init__(self, project_dir, platform, arch, build_type="Release"):
        self.project_dir = os.path.abspath(project_dir)
        self.platform = platform
        self.arch = arch
        self.build_type = build_type
        
        self.build_dir = os.path.join(self.project_dir, "build", f"cmake-{platform}-{arch}")
        self.output_dir = os.path.join(self.project_dir, "dist", platform, arch)
        self.cmake_args = [
            f"-DCMAKE_BUILD_TYPE={self.build_type}",
            "-DDOBBY_BUILD_EXAMPLE=ON"
        ]

    def run_command(self, cmd, cwd=None):
        logging.info(f"Running: {' '.join(cmd)}")
        subprocess.run(cmd, cwd=cwd, check=True)

    def generate(self):
        os.makedirs(self.build_dir, exist_ok=True)
        cmd = ["cmake", "-S", self.project_dir, "-B", self.build_dir] + self.cmake_args
        self.run_command(cmd)

    def build(self):
        cmd = ["cmake", "--build", self.build_dir, "-j", str(os.cpu_count() or 4)]
        self.run_command(cmd)

    def install(self):
        os.makedirs(self.output_dir, exist_ok=True)
        # Copy libraries
        for root, _, files in os.walk(self.build_dir):
            for file in files:
                if file.endswith((".so", ".a", ".dylib", ".dll", ".lib")):
                    shutil.copy2(os.path.join(root, file), self.output_dir)
        logging.info(f"Built artifacts moved to {self.output_dir}")

class AndroidBuilder(PlatformBuilder):
    def __init__(self, project_dir, arch, ndk_dir, api_level=21):
        super().__init__(project_dir, "android", arch)
        toolchain = os.path.join(ndk_dir, "build", "cmake", "android.toolchain.cmake")
        if not os.path.exists(toolchain):
            raise FileNotFoundError(f"Android toolchain not found at {toolchain}")
        
        self.cmake_args += [
            f"-DCMAKE_TOOLCHAIN_FILE={toolchain}",
            f"-DANDROID_ABI={arch}",
            f"-DANDROID_PLATFORM=android-{api_level}",
            "-DANDROID_STL=c++_static"
        ]

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Dobby Platform Builder")
    parser.add_argument("--platform", choices=platforms.keys(), required=True)
    parser.add_argument("--arch", help="Specific arch or 'all'", default="all")
    parser.add_argument("--ndk", help="Android NDK path (or $ANDROID_NDK_HOME)", default=os.environ.get("ANDROID_NDK_HOME"))
    
    args = parser.parse_args()
    project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    
    target_archs = platforms[args.platform] if args.arch == "all" else [args.arch]
    
    for arch in target_archs:
        logging.info(f"Building for {args.platform} {arch}...")
        try:
            if args.platform == "android":
                if not args.ndk:
                    logging.error("NDK path required for Android build. Set $ANDROID_NDK_HOME or use --ndk")
                    sys.exit(1)
                builder = AndroidBuilder(project_root, arch, args.ndk)
            else:
                builder = PlatformBuilder(project_root, args.platform, arch)
            
            builder.generate()
            builder.build()
            builder.install()
        except Exception as e:
            logging.error(f"Failed to build for {arch}: {e}")
            sys.exit(1)
