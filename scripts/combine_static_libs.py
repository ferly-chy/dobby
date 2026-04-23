import os
import sys
import subprocess
import glob
import shutil

def combine(ar_path, target_lib, add_libs):
    tmp_dir = "tmp_objs"
    if os.path.exists(tmp_dir):
        shutil.rmtree(tmp_dir)
    os.makedirs(tmp_dir)
    
    try:
        for add_lib in add_libs:
            if not os.path.exists(add_lib):
                print(f"Warning: Library {add_lib} not found, skipping.")
                continue
                
            # Extract objects from add_lib
            # Use unique subdirectories to avoid collisions between libs
            lib_tmp = os.path.join(tmp_dir, os.path.basename(add_lib) + "_objs")
            os.makedirs(lib_tmp)
            
            subprocess.run([ar_path, "x", os.path.abspath(add_lib)], cwd=lib_tmp, check=True)
            
            # Get all .o files
            objs = glob.glob(os.path.join(lib_tmp, "*.o"))
            if not objs:
                print(f"No objects found in {add_lib}")
                continue
            
            # Add to target_lib
            # We use absolute paths for objects to be sure
            cmd = [ar_path, "q", os.path.abspath(target_lib)] + [os.path.abspath(o) for o in objs]
            subprocess.run(cmd, check=True)
            print(f"Merged {len(objs)} objects from {add_lib} into {target_lib}")
        
    finally:
        if os.path.exists(tmp_dir):
            shutil.rmtree(tmp_dir)

if __name__ == "__main__":
    if len(sys.argv) < 4:
        print("Usage: combine_static_libs.py <ar_path> <target_lib> <add_lib1> [add_lib2 ...]")
        sys.exit(1)
    combine(sys.argv[1], sys.argv[2], sys.argv[3:])
