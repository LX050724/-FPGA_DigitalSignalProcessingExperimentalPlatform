#include <Fatfs_init/Fatfs_Driver.h>

#include "xil_printf.h"
#include "check.h"
#include "utils/str_tool.h"

static FATFS SD_Dev, EMMC_Dev;  // File System instance

static FRESULT fmount_status[2] = {-1, -1};

int Fatfs_Init() {
    FSIZE_t total_size = 0, free_size = 0;
    if (fmount_status[0] != FR_OK) {
        fmount_status[0] = f_mount(&SD_Dev, "0:/", 1);
        if (fmount_status[0] != FR_OK) {
            xil_printf("error: mount SD card field, return %d\r\n", fmount_status[0]);
        } else {
            Fatfs_GetVolSize("0:/", &total_size, &free_size);
            total_size /= 1024;
            free_size /= 1024;
            xil_printf("SD card total size %dMB, free %dMB\r\n", (int) total_size, (int) free_size);
        }
    }

    if (fmount_status[1] != FR_OK) {
        fmount_status[1] = f_mount(&EMMC_Dev, "1:/", 1);
        if (fmount_status[1] != FR_OK) {
            xil_printf("error: mount EMMC field, return %d\r\n", fmount_status[1]);
        } else {
            Fatfs_GetVolSize("1:/", &total_size, &free_size);
            total_size /= 1024;
            free_size /= 1024;
            xil_printf("EMMC total size %dMB, free %dMB\r\n", (int) total_size, (int) free_size);
        }
    }
    return XST_SUCCESS;
}

/**
 * @brief 获得文件系统空间
 * 
 * @param path 文件系统路径
 * @param total_size 总大小(KB)
 * @param free_size 空闲大小(KB)
 * @return int 
 */
int Fatfs_GetVolSize(const char *path, FSIZE_t *total_size, FSIZE_t *free_size) {

    FRESULT res;
    FATFS *pfs;
    DWORD fre_clust;
    res = f_getfree(path, &fre_clust, &pfs);
    if (res != FR_OK) {
        return XST_FAILURE;
    }
    *total_size = (pfs->n_fatent - 2) * pfs->csize / 2;
    *free_size = fre_clust * pfs->csize / 2;
    return XST_SUCCESS;
}

FRESULT Fatfs_GetMountStatus(int index) {
    if (index == 0 || index == 1)
        return fmount_status[index];
    else
        return FR_INVALID_PARAMETER;
}

static inline int enc_get_utf8_size(const uint8_t pInput) {
    uint8_t c = pInput;
    // 0xxxxxxx 返回0
    // 10xxxxxx 不存在
    // 110xxxxx 返回2
    // 1110xxxx 返回3
    // 11110xxx 返回4
    // 111110xx 返回5
    // 1111110x 返回6
    if (c < 0x80) return 0;
    if (c >= 0x80 && c < 0xC0) return -1;
    if (c >= 0xC0 && c < 0xE0) return 2;
    if (c >= 0xE0 && c < 0xF0) return 3;
    if (c >= 0xF0 && c < 0xF8) return 4;
    if (c >= 0xF8 && c < 0xFC) return 5;
    if (c >= 0xFC) return 6;
    return -1;
}

static int enc_unicode_to_utf8_one(uint32_t unic, uint8_t *pOutput) {
    CHECK_FATAL_ERROR(pOutput == NULL);

    if (unic <= 0x0000007F) {
        // * U-00000000 - U-0000007F:  0xxxxxxx
        *pOutput = (unic & 0x7F);
        return 1;
    } else if (unic >= 0x00000080 && unic <= 0x000007FF) {
        // * U-00000080 - U-000007FF:  110xxxxx 10xxxxxx
        *(pOutput + 1) = (unic & 0x3F) | 0x80;
        *pOutput = ((unic >> 6) & 0x1F) | 0xC0;
        return 2;
    } else if (unic >= 0x00000800 && unic <= 0x0000FFFF) {
        // * U-00000800 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx
        *(pOutput + 2) = (unic & 0x3F) | 0x80;
        *(pOutput + 1) = ((unic >> 6) & 0x3F) | 0x80;
        *pOutput = ((unic >> 12) & 0x0F) | 0xE0;
        return 3;
    } else if (unic >= 0x00010000 && unic <= 0x001FFFFF) {
        // * U-00010000 - U-001FFFFF:  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        *(pOutput + 3) = (unic & 0x3F) | 0x80;
        *(pOutput + 2) = ((unic >> 6) & 0x3F) | 0x80;
        *(pOutput + 1) = ((unic >> 12) & 0x3F) | 0x80;
        *pOutput = ((unic >> 18) & 0x07) | 0xF0;
        return 4;
    } else if (unic >= 0x00200000 && unic <= 0x03FFFFFF) {
        // * U-00200000 - U-03FFFFFF:  111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        *(pOutput + 4) = (unic & 0x3F) | 0x80;
        *(pOutput + 3) = ((unic >> 6) & 0x3F) | 0x80;
        *(pOutput + 2) = ((unic >> 12) & 0x3F) | 0x80;
        *(pOutput + 1) = ((unic >> 18) & 0x3F) | 0x80;
        *pOutput = ((unic >> 24) & 0x03) | 0xF8;
        return 5;
    } else if (unic >= 0x04000000 && unic <= 0x7FFFFFFF) {
        // * U-04000000 - U-7FFFFFFF:  1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        *(pOutput + 5) = (unic & 0x3F) | 0x80;
        *(pOutput + 4) = ((unic >> 6) & 0x3F) | 0x80;
        *(pOutput + 3) = ((unic >> 12) & 0x3F) | 0x80;
        *(pOutput + 2) = ((unic >> 18) & 0x3F) | 0x80;
        *(pOutput + 1) = ((unic >> 24) & 0x3F) | 0x80;
        *pOutput = ((unic >> 30) & 0x01) | 0xFC;
        return 6;
    }

    return 0;
}

static int enc_utf8_to_unicode_one(const uint8_t *pInput, uint32_t *Unic) {
    if (pInput == NULL || Unic == NULL) {
        xil_printf("error pInput == NULL || Unic == NULL\r\n");
        while (1);
    }

    // b1 表示UTF-8编码的pInput中的高字节, b2 表示次高字节, ...
    char b1, b2, b3, b4, b5, b6;

    *Unic = 0x0;  // 把 *Unic 初始化为全零
    int utfbytes = enc_get_utf8_size(*pInput);
    uint8_t *pOutput = (uint8_t *) Unic;

    switch (utfbytes) {
        case 0:
            *pOutput = *pInput;
            utfbytes += 1;
            break;
        case 2:
            b1 = *pInput;
            b2 = *(pInput + 1);
            if ((b2 & 0xE0) != 0x80) return 0;
            *pOutput = (b1 << 6) + (b2 & 0x3F);
            *(pOutput + 1) = (b1 >> 2) & 0x07;
            break;
        case 3:
            b1 = *pInput;
            b2 = *(pInput + 1);
            b3 = *(pInput + 2);
            if (((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80)) return 0;
            *pOutput = (b2 << 6) + (b3 & 0x3F);
            *(pOutput + 1) = (b1 << 4) + ((b2 >> 2) & 0x0F);
            break;
        case 4:
            b1 = *pInput;
            b2 = *(pInput + 1);
            b3 = *(pInput + 2);
            b4 = *(pInput + 3);
            if (((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80) || ((b4 & 0xC0) != 0x80)) return 0;
            *pOutput = (b3 << 6) + (b4 & 0x3F);
            *(pOutput + 1) = (b2 << 4) + ((b3 >> 2) & 0x0F);
            *(pOutput + 2) = ((b1 << 2) & 0x1C) + ((b2 >> 4) & 0x03);
            break;
        case 5:
            b1 = *pInput;
            b2 = *(pInput + 1);
            b3 = *(pInput + 2);
            b4 = *(pInput + 3);
            b5 = *(pInput + 4);
            if (((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80) || ((b4 & 0xC0) != 0x80) ||
                ((b5 & 0xC0) != 0x80))
                return 0;
            *pOutput = (b4 << 6) + (b5 & 0x3F);
            *(pOutput + 1) = (b3 << 4) + ((b4 >> 2) & 0x0F);
            *(pOutput + 2) = (b2 << 2) + ((b3 >> 4) & 0x03);
            *(pOutput + 3) = (b1 << 6);
            break;
        case 6:
            b1 = *pInput;
            b2 = *(pInput + 1);
            b3 = *(pInput + 2);
            b4 = *(pInput + 3);
            b5 = *(pInput + 4);
            b6 = *(pInput + 5);
            if (((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80) || ((b4 & 0xC0) != 0x80) ||
                ((b5 & 0xC0) != 0x80) || ((b6 & 0xC0) != 0x80))
                return 0;
            *pOutput = (b5 << 6) + (b6 & 0x3F);
            *(pOutput + 1) = (b5 << 4) + ((b6 >> 2) & 0x0F);
            *(pOutput + 2) = (b3 << 2) + ((b4 >> 4) & 0x03);
            *(pOutput + 3) = ((b1 << 6) & 0x40) + (b2 & 0x3F);
            break;
        default:
            return 0;
    }

    return utfbytes;
}

char *GBK_TO_UTF8(const char *gbk_str) {
    size_t len = strlen(gbk_str) + 1;
    char *utf8_str = os_malloc(len * 3);
    if (utf8_str == NULL) return NULL;
    memset(utf8_str, 0, len * 3);
    char *utf_p = utf8_str;
    for (size_t i = 0; i < len; i++) {
        if (gbk_str[i] < 0x7f) {
            *utf_p++ = gbk_str[i];
            continue;
        };
        uint8_t size = enc_unicode_to_utf8_one(
                ff_oem2uni(gbk_str[i] << 8 | gbk_str[i + 1], FF_CODE_PAGE), (uint8_t *) utf_p);
        i++;
        utf_p += size;
    }
    return utf8_str;
}

char *UTF8_TO_GBK(const char *utf8_str) {
    size_t len = strlen(utf8_str) + 1;
    size_t gbk_len = 0;
    for (size_t i = 0; i < len; i++) {
        int size = enc_get_utf8_size(utf8_str[i]);
        switch (size) {
            case 0:
                gbk_len += 1;
                break;
            case -1:
                break;
            default:
                gbk_len += 2;
                i += size - 1;
                break;
        }
    }
    char *gbk_str = os_malloc(gbk_len);
    if (gbk_str == NULL) return NULL;
    char *gbk_p = gbk_str;
    for (size_t i = 0; i < len;) {
        uint32_t buf = 0;
        i += enc_utf8_to_unicode_one((uint8_t *)utf8_str + i, (uint32_t *) &buf);
        if (buf < 0x7f) {
            *gbk_p++ = buf & 0x7f;
        } else {
            WCHAR gbk_char = ff_uni2oem(buf, FF_CODE_PAGE);
            *gbk_p++ = gbk_char >> 8;
            *gbk_p++ = gbk_char & 0xff;
        }
    }
    return gbk_str;
}

FRESULT Fatfs_mkdir_p(const char *path) {
    FRESULT result = FR_OK;
    StringList stringList = str_split(path, "\\/");
    for (int i = 1; i < stringList.len; i++) {
        char *dir_path = str_join(&stringList, i + 1, '/');
        FILINFO fno;
        result = f_stat( dir_path, &fno);
        if (result != FR_OK) {
            result = f_mkdir(dir_path);
            if (result != FR_OK) {
                os_free(dir_path);
                break;
            }
        }
        os_free(dir_path);
    }
    StringList_free(&stringList);
    return result;
}

char *Fatfs_GetFileDir(const char *filePath) {
    char *path = str_malloc_copy(filePath);
    for (int i = strlen(path); i > 0; i--) {
        if (path[i] == '/' || path[i] == '\\') {
            path[i] = 0;
            break;
        }
    }
    return path;
}

FRESULT Fatfs_rm_rf(const char *path) {
    FRESULT result;
    FILINFO fno;
    result = f_stat(path, &fno);
    if (result != FR_OK) return result;
    char *type_str = "file";
    if (fno.fattrib & AM_DIR) {
        DIR dir;
        result = f_opendir(&dir, path);
        if (result != FR_OK) return result;
        for (;;) {
            FILINFO child_dir;
            result = f_readdir(&dir, &child_dir);
            if (result != FR_OK || child_dir.fname[0] == 0) break;
            char *filename = str_malloc_cat(path, child_dir.fname, '/');
            type_str = "file";
            if (child_dir.fattrib & AM_DIR) {
                type_str = "dir";
                Fatfs_rm_rf(filename);
            }
            f_unlink(filename);
            xil_printf("fatfs: rm %s %s\r\n", type_str, filename);
            os_free(filename);
        }
        f_closedir(&dir);
        type_str = "dir";
    }
    xil_printf("fatfs: rm %s %s\r\n", type_str, path);
    return f_unlink(path);
}
