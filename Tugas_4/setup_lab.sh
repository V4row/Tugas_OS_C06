#!/usr/bin/env bash
set -euo pipefail

IMG_FILE="lab_disk.img"
IMG_PATH="$(realpath -m "${IMG_FILE}")"
MOUNT_POINT="/mnt/media_b"
IMG_SIZE_MB=64
TARGET_USER="${SUDO_USER:-$USER}"

# Jika sudah ter-mount, langsung selesai dengan aman (idempotent)
if mountpoint -q "${MOUNT_POINT}" 2>/dev/null; then
    CURRENT_DEV=$(findmnt -n -o SOURCE "${MOUNT_POINT}")
    echo "[OK] Media B sudah aktif di ${MOUNT_POINT} (device: ${CURRENT_DEV})"
    exit 0
fi

# Lepas loop device lama jika masih nyangkut pada image yang sama
for dev in $(losetup -j "${IMG_PATH}" 2>/dev/null | cut -d: -f1 || true); do
    echo "[INFO] Melepas loop device lama ${dev}..."
    sudo losetup -d "${dev}"
done

echo "[INFO] Membuat disk image ${IMG_FILE} (${IMG_SIZE_MB} MB)..."
dd if=/dev/zero of="${IMG_FILE}" bs=1M count="${IMG_SIZE_MB}" status=progress

echo "[INFO] Menghubungkan ke loop device..."
LOOP_DEV=$(sudo losetup -f --show "${IMG_FILE}")
echo "[INFO] Terhubung ke ${LOOP_DEV}"

echo "[INFO] Memformat ${LOOP_DEV} sebagai ext4..."
sudo mkfs.ext4 -F "${LOOP_DEV}"

echo "[INFO] Me-mount ${LOOP_DEV} ke ${MOUNT_POINT}..."
sudo mkdir -p "${MOUNT_POINT}"
sudo mount "${LOOP_DEV}" "${MOUNT_POINT}"
sudo chown -R "${TARGET_USER}:${TARGET_USER}" "${MOUNT_POINT}"
sudo chmod 755 "${MOUNT_POINT}"

echo "[OK] Media B siap digunakan!"
echo "     Direktori (--dir) : ${MOUNT_POINT}"
echo "     Device    (--dev) : $(basename "${LOOP_DEV}")"
df -hT "${MOUNT_POINT}"