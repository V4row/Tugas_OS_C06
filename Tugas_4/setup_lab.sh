#!/usr/bin/env bash
set -euo pipefail

IMG_FILE="ext4_lab.img"
MOUNT_POINT="/mnt/ext4_lab"
FALLBACK_DIR="./ext4_lab_fallback"
IMG_SIZE_MB=100
TARGET_USER="${SUDO_USER:-$USER}"

echo "[INFO] Membuat disk image ${IMG_FILE} (${IMG_SIZE_MB} MB)..."
dd if=/dev/zero of="${IMG_FILE}" bs=1M count="${IMG_SIZE_MB}" status=progress

echo "[INFO] Memformat ${IMG_FILE} sebagai ext4 (block sie 4096 byte)..."
mkfs.ext4 -b 4096 -F "${IMG_FILE}"

echo "[INFO] Mengecek dukungan loop device..."
if sudo losetup -f >/dev/null 2>&1; then
    sudo mkdir -p "${MOUNT_POINT}"
    if sudo mount -o loop "${IMG_FILE}" "${MOUNT_POINT}"; then
        sudo chown -R "${TARGET_USER}:${TARGET_USER}" "${MOUNT_POINT}"
        sudo chmod 755 "${MOUNT_POINT}"
        echo "[OK] Loop device berhasil di-mount ke ${MOUNT_POINT}"
        df -T "${MOUNT_POINT}"
        exit 0
    fi
fi

echo "[WARN] Loop mount tidak didukung di lingkungan ini (misal WSL2/container)."
echo "[WARN] Menggunakan direktori fallback: ${FALLBACK_DIR}"
echo "[WARN] Wajib dokumentasikan alasan & perbedaannya di laporan!"
mkdir -p "${FALLBACK_DIR}"
chmod 755 "${FALLBACK_DIR}"
