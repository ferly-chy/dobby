#!/usr/bin/env python3
from __future__ import annotations

import logging
import shutil
import zipfile
import os
import re
from pathlib import Path
from typing import Final

# Configure logging
logging.basicConfig(level=logging.INFO, format="%(asctime)s - %(levelname)s - %(message)s")
logger = logging.getLogger(__name__)

DIST_DIR: Final[Path] = Path("dist")
BUILD_DIR: Final[Path] = Path("build")
INCLUDE_DIR: Final[Path] = Path("include")


def collect_headers(target_inc_dir: Path):
    """Copy headers to the distribution directory based on configuration."""
    if target_inc_dir.exists():
        shutil.rmtree(target_inc_dir)
    target_inc_dir.mkdir(parents=True, exist_ok=True)

    # Always copy core Dobby headers
    logger.info(f"Copying core headers: dobby.h, dobby_vtable.h")
    shutil.copy2(INCLUDE_DIR / "dobby.h", target_inc_dir / "dobby.h")
    if (INCLUDE_DIR / "dobby_vtable.h").exists():
        shutil.copy2(INCLUDE_DIR / "dobby_vtable.h", target_inc_dir / "dobby_vtable.h")

def collect_libraries(target_lib_dir: Path):
    """Find and copy all libraries (.a, .so, .lib, .dll) from the build directory."""
    count = 0
    extensions = ["*.a", "*.so", "*.lib", "*.dll", "*.dylib"]
    
    for ext in extensions:
        for lib_path in BUILD_DIR.rglob(ext):
            if "cmake-build-" in str(lib_path) or "_deps" in str(lib_path):
                 continue
            
            rel_path = lib_path.relative_to(BUILD_DIR)
            dest = target_lib_dir / rel_path
            dest.parent.mkdir(parents=True, exist_ok=True)
            
            logger.info(f"Copying library: {rel_path}")
            shutil.copy2(lib_path, dest)
            count += 1
            
    return count

def create_archive(source_dir: Path, output_filename: str):
    """Create a ZIP archive of the distribution directory."""
    logger.info(f"Creating archive: {output_filename}")
    if os.path.exists(output_filename):
        os.remove(output_filename)
        
    with zipfile.ZipFile(output_filename, 'w', zipfile.ZIP_DEFLATED) as zipf:
        for file_path in source_dir.rglob("*"):
            if file_path.is_file():
                arcname = file_path.relative_to(source_dir)
                zipf.write(file_path, arcname)

def main():
    project_root = Path(__file__).resolve().parent.parent
    os.chdir(project_root)

    if DIST_DIR.exists():
        logger.info(f"Cleaning existing distribution directory: {DIST_DIR}")
        shutil.rmtree(DIST_DIR)
    
    DIST_DIR.mkdir(parents=True, exist_ok=True)

    logger.info("Step 1: Analyzing configuration and collecting headers...")
    collect_headers(DIST_DIR / "include")

    logger.info("Step 2: Collecting libraries...")
    lib_count = collect_libraries(DIST_DIR / "lib")

    if lib_count == 0:
        logger.warning("No libraries found in 'build/'. Did you run scripts/platform_builder.py first?")
    else:
        logger.info(f"Successfully collected {lib_count} libraries.")

    logger.info("Step 3: Creating distribution archive...")
    archive_name = "dobby-distribution.zip"
    create_archive(DIST_DIR, archive_name)

    logger.info(f"Done! Distribution ready in '{DIST_DIR}/' and '{archive_name}'")

if __name__ == "__main__":
    main()
