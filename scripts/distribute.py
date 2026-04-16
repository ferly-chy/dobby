import os
import shutil
import argparse
import logging

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(levelname)s: %(message)s')

class DobbyDistributor:
    def __init__(self, project_root, output_dir="dobby_release"):
        self.project_root = os.path.abspath(project_root)
        self.output_dir = os.path.abspath(output_dir)
        self.include_dir = os.path.join(self.output_dir, "include")
        self.lib_dir = os.path.join(self.output_dir, "lib")

    def clean_output(self):
        if os.path.exists(self.output_dir):
            logging.info(f"Cleaning existing distribution directory: {self.output_dir}")
            shutil.rmtree(self.output_dir)
        os.makedirs(self.include_dir)
        os.makedirs(self.lib_dir)

    def copy_headers(self):
        logging.info("Copying public headers...")
        src_header = os.path.join(self.project_root, "include", "dobby.h")
        if os.path.exists(src_header):
            shutil.copy2(src_header, self.include_dir)
        else:
            logging.error("Public header include/dobby.h not found!")

    def distribute(self, lib_type="static"):
        """
        lib_type: 'static', 'shared', or 'both'
        """
        self.clean_output()
        self.copy_headers()

        # Source directory where platform_builder.py puts artifacts
        src_dist = os.path.join(self.project_root, "dist")
        if not os.path.exists(src_dist):
            logging.error(f"Source distribution directory '{src_dist}' not found. Please run build script first.")
            return

        extensions = []
        if lib_type in ["static", "both"]:
            extensions.extend([".a", ".lib"])
        if lib_type in ["shared", "both"]:
            extensions.extend([".so", ".dylib", ".dll"])

        found_count = 0
        for platform in os.listdir(src_dist):
            platform_path = os.path.join(src_dist, platform)
            if not os.path.isdir(platform_path):
                continue

            for arch in os.listdir(platform_path):
                arch_path = os.path.join(platform_path, arch)
                if not os.path.isdir(arch_path):
                    continue

                target_dir = os.path.join(self.lib_dir, platform, arch)
                
                # Scan for relevant libraries
                for file in os.listdir(arch_path):
                    if any(file.endswith(ext) for ext in extensions):
                        # Filter for dobby core libraries only (ignore examples/tests)
                        if "dobby" in file.lower():
                            os.makedirs(target_dir, exist_ok=True)
                            shutil.copy2(os.path.join(arch_path, file), target_dir)
                            logging.info(f"Collected: {platform}/{arch}/{file}")
                            found_count += 1

        if found_count > 0:
            logging.info(f"Successfully distributed {found_count} libraries to {self.output_dir}")
        else:
            logging.warning("No matching libraries found to distribute.")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Dobby Release Distributor")
    parser.add_argument("--type", choices=["static", "shared", "both"], default="static",
                        help="Type of library to distribute (default: static)")
    parser.add_argument("--out", default="dobby_release", help="Output directory name")
    
    args = parser.parse_args()
    
    project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    distributor = DobbyDistributor(project_root, args.out)
    distributor.distribute(args.type)
