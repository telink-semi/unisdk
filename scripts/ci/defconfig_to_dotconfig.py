import kconfiglib
import sys

if len(sys.argv) != 4:
    print("Usage: defconfig_to_dotconfig.py <Kconfig> <defconfig> <dotconfig>")
    sys.exit(1)

kconfig_path = sys.argv[1]
defconfig_path = sys.argv[2]
dotconfig_path = sys.argv[3]

kconf = kconfiglib.Kconfig(kconfig_path)

kconf.load_config(defconfig_path)

kconf.write_config(dotconfig_path)

print(f"Generated full dotconfig: {dotconfig_path}")
