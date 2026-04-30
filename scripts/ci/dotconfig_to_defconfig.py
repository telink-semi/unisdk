import kconfiglib
import sys

if len(sys.argv) != 4:
    print("Usage: dotconfig_to_defconfig.py <Kconfig> <defconfig> <dotconfig>")
    sys.exit(1)

kconfig_path = sys.argv[1]
defconfig_path = sys.argv[2]
dotconfig_path = sys.argv[3]

kconf = kconfiglib.Kconfig(kconfig_path)

kconf.load_config(dotconfig_path)

kconf.write_min_config(defconfig_path)

print(f"Generated full dotconfig: {defconfig_path}")
