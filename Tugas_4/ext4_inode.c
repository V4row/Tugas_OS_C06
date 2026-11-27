#define _GNU_SOURCE
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "banner.h"
#include "iolib.h"


struct ext4_extent_header {
    uint16_t eh_magic;
    uint16_t eh_entries;
    uint16_t eh_max;
    uint16_t eh_depth;
    uint32_t eh_generation;
};

struct ext4_extent {
    uint32_t blok_logis;
    uint16_t jumlah_blok;
    uint16_t blok_awal;
    uint32_t blok_akhir;
};




int main(int argc, char *argv[]) {
    print_banner("C06", "Tugas 4 - Filesystem & I/O", "Ext4 Inode Reader");

    if (argc != 3) {
        fprintf(stderr, "Penggunaan: %s <disk_image> <nomor_inode>\n", argv[0]);
        fprintf(stderr, "Contoh    : %s ext4_lab.img 12\n", argv[0]);
        return 1;
    }

    const char *path_image = argv[1];
    uint32_t target_inode = 0;
    if (sscanf(argv[2], "%u", &target_inode) != 1 || target_inode == 0) {
        fprintf(stderr, "[ERROR] Nomor inode tidak valid (harus angka >= 1).\n");
        return 1;
    }

    if (target_inode == 0) {
        fprintf(stderr, "[ERROR] Nomor inode harus >= 1.\n");
        return 1;
    }

    char timestamp[32];
    io_get_timestamp(timestamp, sizeof(timestamp));
    io_save_env_spec("C06", timestamp);

    uint64_t t_start = io_now_ns();

    int fd = open(path_image, O_RDONLY);
    if (fd < 0) {
        perror("Gagal membuka disk image");
        return 1;
    }

    print_section_banner("Baca Struktur Inode Mentah", 1, path_image);

    // Superblock
    uint8_t sb[1024];
    if (pread(fd, sb, sizeof(sb), 1024) != sizeof(sb)) {
        perror("Gagal membaca Superblock");
        close(fd);
        return 1;
    }

    uint16_t s_magic = *(uint16_t *)(sb + 0x38);
    if (s_magic != 0xEF53) {
        fprintf(stderr, "[ERROR] Bukan filesystem ext4 (s_magic = 0x%04X, harus 0xEF53)\n", s_magic);
        close(fd);
        return 1;
    }

    uint32_t s_inodes_count     = *(uint32_t *)(sb + 0x00);
    uint32_t s_log_block_size   = *(uint32_t *)(sb + 0x18);
    uint32_t s_inodes_per_group = *(uint32_t *)(sb + 0x28);
    uint16_t s_inode_size       = *(uint16_t *)(sb + 0x58);
    uint16_t s_desc_size        = *(uint16_t *)(sb + 0xFE);

    if (s_desc_size == 0) {
        s_desc_size = 32;
    }



    uint32_t block_size = 1024U << s_log_block_size;

    if (target_inode > s_inodes_count) {
        fprintf(stderr, "[ERROR] Nomor inode %u melebihi batas (%u).\n", target_inode, s_inodes_count);
        close(fd);
        return 1;
    }

    printf("[1. Superblock]\n");
    printf("  Magic Number     : 0x%04X (Valid ext4)\n", s_magic);
    printf("  Block Size       : %u bytes\n", block_size);
    printf("  Inode Size       : %u bytes\n", s_inode_size);
    printf("  Inodes per Group : %u\n", s_inodes_per_group);

    // Group Descriptor Table
    uint32_t grup_ke      = (target_inode - 1) / s_inodes_per_group;
    uint32_t indeks_lokal = (target_inode - 1) % s_inodes_per_group;

    off_t offset_blok_group_descriptor_table = (block_size == 1024) ? 2048 : (off_t)block_size;
    off_t offset_grup = offset_blok_group_descriptor_table + ((off_t)grup_ke * s_desc_size);

    uint8_t blok_group_descriptor[64];
    if (pread(fd, blok_group_descriptor, s_desc_size, offset_grup) != s_desc_size) {
        perror("Gagal membaca Block Group Descriptor");
        close(fd);
        return 1;
    }

    uint32_t blok_inode_table = *(uint32_t *)(blok_group_descriptor + 0x08);

    printf("\n[2. Lokasi Inode %u]\n", target_inode);
    printf("  Block Group      : %u\n", grup_ke);
    printf("  Indeks Lokal     : %u\n", indeks_lokal);
    printf("  Inode Table Blk  : %u\n", blok_inode_table);

    // membaca inode
    off_t offset_inode = ((off_t)blok_inode_table * block_size) +
                         ((off_t)indeks_lokal * s_inode_size);

    uint8_t *inode_buf = malloc(s_inode_size);
    if (!inode_buf) {
        perror("malloc gagal");
        close(fd);
        return 1;
    }

    if (pread(fd, inode_buf, s_inode_size, offset_inode) != s_inode_size) {
        perror("Gagal membaca data Inode");
        free(inode_buf);
        close(fd);
        return 1;
    }

    uint16_t i_mode  = *(uint16_t *)(inode_buf + 0x00);
    uint32_t i_size  = *(uint32_t *)(inode_buf + 0x04);
    uint32_t i_ctime = *(uint32_t *)(inode_buf + 0x0C);
    uint32_t i_mtime = *(uint32_t *)(inode_buf + 0x10);
    uint16_t i_links = *(uint16_t *)(inode_buf + 0x1A);
    uint32_t i_ctime_nsec = (s_inode_size >= 160) ? (*(uint32_t *)(inode_buf + 0x84) >> 2) : 0;
    uint32_t i_mtime_nsec = (s_inode_size >= 160) ? (*(uint32_t *)(inode_buf + 0x88) >> 2) : 0;

    printf("\n[3. Metadata Inode %u]\n", target_inode);
    printf("  i_mode   : 0%o\n", i_mode);
    printf("  i_size   : %u bytes\n", i_size);
    printf("  i_links  : %u\n", i_links);
    printf("  i_mtime  : %u.%09u s\n", i_mtime, i_mtime_nsec);
    printf("  i_ctime  : %u.%09u s\n", i_ctime, i_ctime_nsec);

    // extent tree atau direct pointer
    uint8_t *i_block = inode_buf + 0x28;
    struct ext4_extent_header *eh = (struct ext4_extent_header *)i_block;

    printf("\n[4. Penunjuk Blok Data (i_block)]\n");
    if (eh->eh_magic == 0xF30A) {
        printf("  Tipe Pointer : EXTENT TREE (eh_magic = 0x%04X)\n", eh->eh_magic);
        printf("  Jumlah Entri : %u (Depth: %u)\n", eh->eh_entries, eh->eh_depth);
        if (eh->eh_depth == 0) {
            struct ext4_extent *ext = (struct ext4_extent *)(i_block + sizeof(struct ext4_extent_header));
            for (uint16_t i = 0; i < eh->eh_entries; i++) {
                printf("  -> Extent[%u] : Logical Block %u -> Physical Block %u (Panjang: %u blok)\n",
                       i, ext[i].blok_logis, ext[i].blok_akhir, ext[i].jumlah_blok);
            }
        }
    } else {
        printf("  Tipe Pointer : DIRECT POINTER (eh_magic = 0x%04X != 0xF30A)\n", eh->eh_magic);
        uint32_t *direct = (uint32_t *)i_block;
        for (int i = 0; i < 12; i++) {
            if (direct[i] != 0) {
                printf("  -> Direct[%d] : Physical Block %u\n", i, direct[i]);
            }
        }
        if (direct[12] != 0) printf("  -> Single Indirect : Block %u\n", direct[12]);
        if (direct[13] != 0) printf("  -> Double Indirect : Block %u\n", direct[13]);
        if (direct[14] != 0) printf("  -> Triple Indirect : Block %u\n", direct[14]);
    }

    free(inode_buf);
    close(fd);



    double elapsed_sec = (double)io_diff_ns(t_start, io_now_ns()) / 1e9;
    print_footer_banner("COMPLETED", elapsed_sec);
    return 1;
}