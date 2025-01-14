import os
import sys

def main():
    if len(sys.argv) != 4:
        print("Usage: msi_validation.py <msival2_path> <boinc_msi_path> <darice_cub_path>")
        sys.exit(1)

    msival2_path = sys.argv[1]
    if not os.path.exists(msival2_path):
        print(f"'{msival2_path}' not found")
        sys.exit(1)

    msi_path = sys.argv[2]
    if not os.path.exists(msi_path):
        print(f"'{msi_path}' not found")
        sys.exit(1)

    cub_path = sys.argv[3]
    if not os.path.exists(cub_path):
        print("'{cub_path}' not found")
        sys.exit(1)

    ignore_list = [
        "ICE          Type       Description",
        "ICE07        WARNING   '_BOINCScreensaver_LiberationSans_Regular.ttf' is a Font and must be installed to the FontsFolder. Current Install Directory: 'INSTALLDIR'",
        "ICE57        ERROR     Component '_ScreensaverEnableNT' has both per-user and per-machine data with a per-machine KeyPath.",
        "ICE57        ERROR     Component '_BOINCManagerStartup' has both per-user and per-machine data with a per-machine KeyPath.",
        "ICE57        ERROR     Component '_BOINCManagerStartMenu' has both per-user and per-machine data with a per-machine KeyPath.",
        "ICE61        WARNING   This product should remove only older versions of itself. The Maximum version is not less than the current product.",
    ]
    output = os.popen(f'"{msival2_path}" "{msi_path}" "{cub_path}" -f').read()
    error_found = False
    for line in output.splitlines():
        if line == '' or any(ignore in line for ignore in ignore_list):
            continue
        error_found = True
        print(line)

    if error_found:
        print("Validation failed")
        sys.exit(1)

    print("Validation succeeded")
    sys.exit(0)

if __name__ == "__main__":
    main()
