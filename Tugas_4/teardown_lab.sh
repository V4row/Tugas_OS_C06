#!/usr/bin/env bash
set -euo pipefail

IMG_FILE="ext4_lab.img"
MOUNT_POINT="/mnt/ext4_lab"
FALLBACK_DIR="./ext4_lab_fallback"

if mountpoint -q "${MOUNT_POINT}" 2>/dev/null; then
    echo "[INFO] Meng-unmount ${MOUNT_POINT}..."
    sudo umount "${MOUNT_POINT}"
fi

if [ -d "${MOUNT_POINT}" ]; then
    echo "[INFO] Menghapus direktori mount point ${MOUNT_POINT}..."
    sudo rmdir "${MOUNT_POINT}" 2>/dev/null || true
fi

if [ -f "${IMG_FILE}" ]; then
    echo "[INFO] Menghapus disk image ${IMG_FILE}..."
    rm -f "${IMG_FILE}"
fi

if [ -d "${FALLBACK_DIR}" ]; then
    echo "[INFO] Menghapus direktori fallback ${FALLBACK_DIR}..."
    rm -rf "${FALLBACK_DIR}"
fi

echo "[OK] Teardown selesai."
