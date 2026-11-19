#!/usr/bin/env bash
# Build only. No installation or radio/network changes.
set -euo pipefail
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
arch=${ARCH:-arm64}
cross=${CROSS_COMPILE:-}
jobs=${JOBS:-2}
kernel=${KVER:-$(uname -r)}
headers=${KSRC:-/lib/modules/$kernel/build}
case "$jobs" in ''|*[!0-9]*|0) echo 'JOBS must be a positive integer' >&2; exit 2;; esac
if [[ ! -f "$headers/Makefile" ]]; then
  echo "Missing target kernel headers: $headers" >&2
  exit 3
fi
# Radxa's headers can report only the base version through `make kernelrelease`.
# The generated UTS_RELEASE is the ABI string actually embedded in vermagic.
target_kernel=
if [[ -f "$headers/include/generated/utsrelease.h" ]]; then
  target_kernel=$(sed -n 's/^#define UTS_RELEASE "\(.*\)"/\1/p' "$headers/include/generated/utsrelease.h")
fi
[[ -n "$target_kernel" ]] || target_kernel=$(make -s -C "$headers" kernelrelease)
cd "$root"
make -j"$jobs" ARCH="$arch" CROSS_COMPILE="$cross" KSRC="$headers" \
  KVER="$target_kernel" CONFIG_PLATFORM_I386_PC=n MODULE_NAME=8812eu \
  CONFIG_AP_MODE=y CONFIG_WIFI_MONITOR=y KCFLAGS=-Wno-error \
  USER_EXTRA_CFLAGS='-DCONFIG_LITTLE_ENDIAN -DCONFIG_IOCTL_CFG80211 -DRTW_USE_CFG80211_STA_EVENT -DCONFIG_USB_INBAND -DCONFIG_WIFI_MONITOR'
test -s 8812eu.ko
if command -v modinfo >/dev/null 2>&1; then
  module_kernel=$(modinfo -F vermagic ./8812eu.ko)
elif [[ -x /sbin/modinfo ]]; then
  module_kernel=$(/sbin/modinfo -F vermagic ./8812eu.ko)
else
  echo 'Module built, but modinfo is required to validate its target kernel.' >&2
  exit 4
fi
[[ "${module_kernel%% *}" == "$target_kernel" ]] || {
  echo "Kernel mismatch: $module_kernel versus $target_kernel" >&2; exit 5;
}
printf 'Built only: %s/8812eu.ko\nvermagic: %s\n' "$root" "$module_kernel"
sha256sum 8812eu.ko
