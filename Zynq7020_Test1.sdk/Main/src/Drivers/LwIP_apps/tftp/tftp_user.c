//
// Created by yaoji on 2022/4/8.
//

#include "tftp_user.h"
#include "xil_printf.h"
#include "FreeRTOS_Mem/FreeRTOS_Mem.h"
#include "Fatfs_init/Fatfs_Driver.h"
#include "utils/str_tool.h"
#include <lwip/apps/tftp_server.h>
#include <ff.h>

static void *tftp_fs_open(const char *fname, const char *mode, u8_t write);
static void tftp_fs_close(void *handle);
static int tftp_fs_read(void *handle, void *buf, int bytes);
static int tftp_fs_write(void *handle, struct pbuf *p);

static struct tftp_context context = {
        .open = tftp_fs_open,
        .close = tftp_fs_close,
        .read = tftp_fs_read,
        .write = tftp_fs_write,
};

void tftp_start() {
    if (Fatfs_GetMountStatus(SD_INDEX) == FR_OK) {
        err_t err = tftp_init(&context);
        if (err) {
            xil_printf("tftp [init] error %d\r\n", err);
        } else {
            xil_printf("tftp [init] success\r\n");
        }
    } else xil_printf("tftp [init] no SD card, skip\r\n");
}

static void *tftp_fs_open(const char *fname, const char *mode, u8_t write) {
    LWIP_UNUSED_ARG(mode);
    vPortEnterCritical();
    FIL *file_ptr = os_malloc(sizeof(FIL));
    if (file_ptr == NULL) return NULL;
    else memset(file_ptr, 0, sizeof(FILE));
    char *utf8 = GBK_TO_UTF8(fname);
    xil_printf("tftp: [open] filename=%s, mode=%s, %c\r\n", utf8, mode, write ? 'w' : 'r');
    os_free(utf8);

    if (write) {
        char *path = Fatfs_GetFileDir(fname);
        Fatfs_mkdir_p(path);
        os_free(path);
    }

    FRESULT ret = f_open(file_ptr, fname, write ? FA_WRITE | FA_CREATE_ALWAYS : FA_READ);
    if (ret == FR_OK) {
        xil_printf("tftp: [open] success handle=%p\r\n", file_ptr);
        vPortExitCritical();
        return file_ptr;
    }
    xil_printf("tftp: [open] error: return %d\r\n", ret);
    os_free(file_ptr);
    vPortExitCritical();
    return NULL;
}

static void tftp_fs_close(void *handle) {
    vPortEnterCritical();
    f_close(handle);
    vPortExitCritical();
    xil_printf("tftp: [close] %p\r\n", handle);
    os_free(handle);
}

static int tftp_fs_read(void *handle, void *buf, int bytes) {
    UINT read_bytes = 0;
    vPortEnterCritical();
    f_read(handle, buf, bytes, &read_bytes);
    vPortExitCritical();
    return read_bytes;
}

static int tftp_fs_write(void *handle, struct pbuf *p) {
    int sum_bytes = 0;
    vPortEnterCritical();
    while (p) {
        UINT write_bytes = 0;
        FRESULT ret = f_write(handle, p->payload, p->len, &write_bytes);
        if (ret == FR_OK) {
            p = p->next;
            sum_bytes += write_bytes;
        } else return sum_bytes;
    }
    vPortExitCritical();
    return sum_bytes;
}
