//
// Created by yaoji on 2022/4/11.
//

#include "bmp_encoder.h"
#include "src/misc/lv_color.h"
#include "check.h"
#include "FreeRTOS_Mem/FreeRTOS_Mem.h"
#include <ff.h>

typedef struct {
    WORD bfType;                    //!< 位图文件的类型，必须为BM(1-2字节）
    DWORD bfSize;                   //!< 位图文件的大小，以字节为单位（3-6字节，低位在前）
    WORD bfReserved1;               //!< 位图文件保留字，必须为0(7-8字节）
    WORD bfReserved2;               //!< 位图文件保留字，必须为0(9-10字节）
    DWORD bfOffBits;                //!< 位图数据的起始位置，以相对于位图（11-14字节，低位在前）文件头的偏移量表示，以字节为单位
}__attribute__((packed)) bmp_header_t;

typedef struct {
    DWORD biSize;                   //!< 本结构所占用字节数（15-18字节）
    LONG biWidth;                   //!< 位图的宽度，以像素为单位（19-22字节）
    LONG biHeight;                  //!< 位图的高度，以像素为单位（23-26字节）
    WORD biPlanes;                  //!< 目标设备的级别，必须为1(27-28字节）
    WORD biBitCount;                //!< 每个像素所需的位数，必须是1（双色），（29-30字节）4(16色），8(256色）16(高彩色)或24（真彩色）之一
    DWORD biCompression;            //!< 位图压缩类型，必须是0（不压缩），（31-34字节） 1(BI_RLE8压缩类型）或2(BI_RLE4压缩类型）之一
    DWORD biSizeImage;              //!< 位图的大小(其中包含了为了补齐行数是4的倍数而添加的空字节)，以字节为单位（35-38字节）
    LONG biXPelsPerMeter;           //!< 位图水平分辨率，每米像素数（39-42字节）
    LONG biYPelsPerMeter;           //!< 位图垂直分辨率，每米像素数（43-46字节)
    DWORD biClrUsed;                //!< 位图实际使用的颜色表中的颜色数（47-50字节）
    DWORD biClrImportant;           //!< 位图显示过程中重要的颜色数（51-54字节）
}__attribute__((packed)) bmp_info_header_t;

typedef struct {
    bmp_header_t header;
    bmp_info_header_t info;      //!< 位图信息头
}__attribute__((packed)) bmp_t;

typedef struct {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
} __attribute__((packed)) lv_color24_t;

int bmp_save(int w, int h, lv_color_t *img, const char *filename) {
    lv_color24_t *line_buf = NULL;
    FIL file;
    FRESULT res = f_open(&file, filename, FA_WRITE | FA_CREATE_NEW);
    if (res != FR_OK) return XST_FAILURE;
    bmp_t bmp;
    bmp.header.bfType = 'B' | 'M' << 8;
    bmp.header.bfSize = sizeof(bmp_t) + w * h * 3;
    bmp.header.bfOffBits = sizeof(bmp_t);
    bmp.info.biSize = sizeof(bmp_info_header_t);
    bmp.info.biWidth = w;
    bmp.info.biHeight = h;
    bmp.info.biPlanes = 1;
    bmp.info.biBitCount = 24;
    bmp.info.biCompression = 0;
    bmp.info.biSizeImage = w * h * 3;
    bmp.info.biXPelsPerMeter = 3780;
    bmp.info.biYPelsPerMeter = 3780;
    bmp.info.biClrUsed = 0;
    bmp.info.biClrImportant = 0;

    UINT bw;
    res = f_write(&file, &bmp, sizeof(bmp_t), &bw);
    if (res != FR_OK || bw != sizeof(bmp_t)) goto err;

    UINT line_size = sizeof(lv_color24_t) * w;
    line_buf = os_malloc(line_size);
    if (line_buf == NULL) goto err;

    for (int i = h - 1; i >= 0; i--) {
        lv_color_t *line = img + w * i;
        for (int j = 0; j < w; j++) {
            line_buf[j].red = line[j].ch.red;
            line_buf[j].green = line[j].ch.green;
            line_buf[j].blue = line[j].ch.blue;
        }
        vPortEnterCritical();
        res = f_write(&file, line_buf, line_size, &bw);
        vPortExitCritical();
        if (res != FR_OK || bw != line_size) goto err;
    }
    os_free(line_buf);
    f_close(&file);
    return XST_SUCCESS;

    err:
    os_free(line_buf);
    f_close(&file);
    return XST_FAILURE;
}
