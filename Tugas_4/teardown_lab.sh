#!/usr/bin/env bash
set -euo pipefail

IMG_FILE="lab_disk.img"
IMG_PATH="$(realpath -m "${IMG_FILE}")"
MOUNT_POINT="/mnt/media_n"

if mountpoint -q "${MOUNT_POINT}" 2>/dev/null; then
    echo "[INFO] Meng-unmount ${MOUNT_POINT}..."
    sudo umount "${MOUNT_POINT}"
fi

for dev in $(losetup -j "${IMG_PATH}" 2>/dev/null | cut -d: -f1 || true); do
    echo "[INFO] Melepas loop device ${dev}..."
    sudo losetup -d "${dev}"
done

if [ -d "${MOUNT_POINT}" ]; then
    echo "[INFO] Menghapus direktori mount point ${MOUNT_POINT}..."
    sudo rmdir "${MOUNT_POINT}" 2>/dev/null || true
fi

if [ -f "${IMG_FILE}" ]; then
    echo "[INFO] Menghapus disk image ${IMG_FILE}..."
    rm -f "${IMG_FILE}"
fi

echo "[OK] Teardown selesai."
