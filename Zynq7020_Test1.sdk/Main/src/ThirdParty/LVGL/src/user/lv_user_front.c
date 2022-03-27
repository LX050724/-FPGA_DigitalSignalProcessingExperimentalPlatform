#include "src/user/lv_user_font.h"
#include "src/misc/lv_mem.h"
#include "src/misc/lv_fs.h"
#include "src/misc/lv_log.h"
#include "ff.h"

#if LV_FONT_MSYHL_8
lv_font_t msyhl_8;
#endif

#if LV_FONT_MSYHL_10
lv_font_t msyhl_10;
#endif

#if LV_FONT_MSYHL_12
lv_font_t msyhl_12;
#endif

#if LV_FONT_MSYHL_14
lv_font_t msyhl_14;
#endif

#if LV_FONT_MSYHL_16
lv_font_t msyhl_16;
#endif

#if LV_FONT_MSYHL_18
lv_font_t msyhl_18;
#endif

#if LV_FONT_MSYHL_20
lv_font_t msyhl_20;
#endif

#if LV_FONT_MSYHL_22
lv_font_t msyhl_22;
#endif

#if LV_FONT_MSYHL_24
lv_font_t msyhl_24;
#endif

#if LV_FONT_MSYHL_26
lv_font_t msyhl_26;
#endif

#if LV_FONT_MSYHL_28
lv_font_t msyhl_28;
#endif

#if LV_FONT_MSYHL_30
lv_font_t msyhl_30;
#endif

#if LV_FONT_MSYHL_32
lv_font_t msyhl_32;
#endif

#if LV_FONT_MSYHL_34
lv_font_t msyhl_34;
#endif

#if LV_FONT_MSYHL_36
lv_font_t msyhl_36;
#endif

#if LV_FONT_MSYHL_38
lv_font_t msyhl_38;
#endif

#if LV_FONT_MSYHL_40
lv_font_t msyhl_40;
#endif

#if LV_FONT_MSYHL_42
lv_font_t msyhl_42;
#endif

#if LV_FONT_MSYHL_44
lv_font_t msyhl_44;
#endif

#if LV_FONT_MSYHL_46
lv_font_t msyhl_46;
#endif

#if LV_FONT_MSYHL_48
lv_font_t msyhl_48;
#endif

static x_file_t font_files[] = {
#if LV_FONT_MSYHL_8
    {.font_obj = &msyhl_8,
     .line_height = 11,
     .base_line = 0,
     .head =
         {
             .bpp = 4,
             .max = 0xffe5,
             .min = 0x0020,
         },
     .filename =  "msyhl_8.bin"},
#endif
#if LV_FONT_MSYHL_10
     {.font_obj = &msyhl_10,
     .line_height = 13,
     .base_line = 0,
     .head =
         {
             .bpp = 4,
             .max = 0xffe5,
             .min = 0x0020,
         },
     .filename =  "msyhl_10.bin"},
#endif
#if LV_FONT_MSYHL_12
     {.font_obj = &msyhl_12,
     .line_height = 16,
     .base_line = 0,
     .head =
         {
             .bpp = 4,
             .max = 0xffe5,
             .min = 0x0020,
         },
     .filename =  "msyhl_12.bin"},
#endif
#if LV_FONT_MSYHL_14
     {.font_obj = &msyhl_14,
     .line_height = 18,
     .base_line = 0,
     .head =
         {
             .bpp = 4,
             .max = 0xffe5,
             .min = 0x0020,
         },
     .filename =  "msyhl_14.bin"},
#endif
#if LV_FONT_MSYHL_16
     {.font_obj = &msyhl_16,
     .line_height = 21,
     .base_line = 0,
     .head =
         {
             .bpp = 4,
             .max = 0xffe5,
             .min = 0x0020,
         },
     .filename =  "msyhl_16.bin"},
#endif
#if LV_FONT_MSYHL_18
     {.font_obj = &msyhl_18,
     .line_height = 24,
     .base_line = 0,
     .head =
         {
             .bpp = 4,
             .max = 0xffe5,
             .min = 0x0020,
         },
     .filename =  "msyhl_18.bin"},
#endif
#if LV_FONT_MSYHL_20
     {.font_obj = &msyhl_20,
     .line_height = 26,
     .base_line = 0,
     .head =
         {
             .bpp = 4,
             .max = 0xffe5,
             .min = 0x0020,
         },
     .filename =  "msyhl_20.bin"},
#endif
#if LV_FONT_MSYHL_22
     {.font_obj = &msyhl_22,
     .line_height = 29,
     .base_line = 0,
     .head =
         {
             .bpp = 4,
             .max = 0xffe5,
             .min = 0x0020,
         },
     .filename =  "msyhl_22.bin"},
#endif
#if LV_FONT_MSYHL_24
     {.font_obj = &msyhl_24,
     .line_height = 32,
     .base_line = 0,
     .head =
         {
             .bpp = 4,
             .max = 0xffe5,
             .min = 0x0020,
         },
     .filename =  "msyhl_24.bin"},
#endif
#if LV_FONT_MSYHL_26
    {.font_obj = &msyhl_26,
     .line_height = 34,
     .base_line = 0,
     .head =
         {
             .bpp = 4,
             .max = 0xffe5,
             .min = 0x0020,
         },
     .filename =  "msyhl_26.bin"},
#endif
#if LV_FONT_MSYHL_28
    {.font_obj = &msyhl_28,
     .line_height = 37,
     .base_line = 0,
     .head =
         {
             .bpp = 4,
             .max = 0xffe5,
             .min = 0x0020,
         },
     .filename =  "msyhl_28.bin"},
#endif
#if LV_FONT_MSYHL_30
    {.font_obj = &msyhl_30,
     .line_height = 40,
     .base_line = 0,
     .head =
         {
             .bpp = 4,
             .max = 0xffe5,
             .min = 0x0020,
         },
     .filename =  "msyhl_30.bin"},
#endif
#if LV_FONT_MSYHL_32
    {.font_obj = &msyhl_32,
     .line_height = 42,
     .base_line = 0,
     .head =
         {
             .bpp = 4,
             .max = 0xffe5,
             .min = 0x0020,
         },
     .filename =  "msyhl_32.bin"},
#endif
#if LV_FONT_MSYHL_34
    {.font_obj = &msyhl_34,
     .line_height = 45,
     .base_line = 0,
     .head =
         {
             .bpp = 4,
             .max = 0xffe5,
             .min = 0x0020,
         },
     .filename =  "msyhl_34.bin"},
#endif
#if LV_FONT_MSYHL_36
    {.font_obj = &msyhl_36,
     .line_height = 48,
     .base_line = 0,
     .head =
         {
             .bpp = 4,
             .max = 0xffe5,
             .min = 0x0020,
         },
     .filename =  "msyhl_36.bin"},
#endif
#if LV_FONT_MSYHL_38
    {.font_obj = &msyhl_38,
     .line_height = 50,
     .base_line = 0,
     .head =
         {
             .bpp = 4,
             .max = 0xffe5,
             .min = 0x0020,
         },
     .filename =  "msyhl_38.bin"},
#endif
#if LV_FONT_MSYHL_40
    {.font_obj = &msyhl_40,
     .line_height = 53,
     .base_line = 0,
     .head =
         {
             .bpp = 4,
             .max = 0xffe5,
             .min = 0x0020,
         },
     .filename =  "msyhl_40.bin"},
#endif
#if LV_FONT_MSYHL_42
    {.font_obj = &msyhl_42,
     .line_height = 55,
     .base_line = 0,
     .head =
         {
             .bpp = 4,
             .max = 0xffe5,
             .min = 0x0020,
         },
     .filename =  "msyhl_42.bin"},
#endif
#if LV_FONT_MSYHL_44
    {.font_obj = &msyhl_44,
     .line_height = 58,
     .base_line = 0,
     .head =
         {
             .bpp = 4,
             .max = 0xffe5,
             .min = 0x0020,
         },
     .filename =  "msyhl_44.bin"},
#endif
#if LV_FONT_MSYHL_46
    {.font_obj = &msyhl_46,
     .line_height = 61,
     .base_line = 0,
     .head =
         {
             .bpp = 4,
             .max = 0xffe5,
             .min = 0x0020,
         },
     .filename =  "msyhl_46.bin"},
#endif
#if LV_FONT_MSYHL_48
    {.font_obj = &msyhl_48,
     .line_height = 63,
     .base_line = 0,
     .head =
         {
             .bpp = 4,
             .max = 0xffe5,
             .min = 0x0020,
         },
     .filename =  "msyhl_48.bin"},
#endif
};

static inline uint8_t *lv_user_font_getdata(const lv_font_t *font, int offset, int size) {
    return ((x_file_t *)(font->user_data))->data + offset;
}

static const uint8_t *lv_user_font_get_bitmap(const lv_font_t *font, uint32_t unicode_letter) {
    x_header_t *__g_xbf_hd = &((x_file_t *)(font->user_data))->head;
    if (unicode_letter > __g_xbf_hd->max || unicode_letter < __g_xbf_hd->min) {
        return NULL;
    }
    uint32_t unicode_offset = sizeof(x_header_t) + (unicode_letter - __g_xbf_hd->min) * 4;
    uint32_t *p_pos = (uint32_t *)lv_user_font_getdata(font, unicode_offset, 4);
    if (p_pos[0] != 0) {
        uint32_t pos = p_pos[0];
        glyph_dsc_t *gdsc = (glyph_dsc_t *)lv_user_font_getdata(font, pos, sizeof(glyph_dsc_t));
        return lv_user_font_getdata(font, pos + sizeof(glyph_dsc_t),
                                 gdsc->box_w * gdsc->box_h * __g_xbf_hd->bpp / 8);
    }
    return NULL;
}

static bool lv_user_font_get_glyph_dsc(const lv_font_t *font, lv_font_glyph_dsc_t *dsc_out,
                                    uint32_t unicode_letter, uint32_t unicode_letter_next) {
    x_header_t *__g_xbf_hd = &((x_file_t *)(font->user_data))->head;
    if (unicode_letter > __g_xbf_hd->max || unicode_letter < __g_xbf_hd->min) {
        return NULL;
    }
    uint32_t unicode_offset = sizeof(x_header_t) + (unicode_letter - __g_xbf_hd->min) * 4;
    uint32_t *p_pos = (uint32_t *)lv_user_font_getdata(font, unicode_offset, 4);
    if (p_pos[0] != 0) {
        glyph_dsc_t *gdsc = (glyph_dsc_t *)lv_user_font_getdata(font, p_pos[0], sizeof(glyph_dsc_t));
        dsc_out->adv_w = gdsc->adv_w;
        dsc_out->box_h = gdsc->box_h;
        dsc_out->box_w = gdsc->box_w;
        dsc_out->ofs_x = gdsc->ofs_x;
        dsc_out->ofs_y = gdsc->ofs_y;
        dsc_out->bpp = __g_xbf_hd->bpp;
        return true;
    }
    return false;
}



void lv_user_font_load(const char *dir) {
    int len = strlen(dir) + strlen(font_files[0].filename) + 3;
    char *path = lv_mem_alloc(len);

    for (int i = 0; i < sizeof(font_files) / sizeof(x_file_t); i++) {
        font_files[i].font_obj->user_data = &font_files[i];

        font_files[i].font_obj->get_glyph_bitmap = lv_user_font_get_bitmap;
        font_files[i].font_obj->get_glyph_dsc = lv_user_font_get_glyph_dsc;
        font_files[i].font_obj->base_line = font_files[i].base_line;
        font_files[i].font_obj->line_height = font_files[i].line_height;

        FIL font_binfile;
        lv_memset_00(path, len);
        strcat(path, dir);
        strcat(path, "/");
        strcat(path, font_files[i].filename);
        LV_LOG_INFO("lv user font loading %s ...\r\n", path);
        if (f_open(&font_binfile, path, FA_READ) != FR_OK) {
            LV_LOG_ERROR("Failed to open font file %s", path);
            continue;
        }
        FSIZE_t read_size = 0, file_size = f_size(&font_binfile);

        font_files[i].data = lv_mem_alloc(file_size);
        if (font_files[i].data == NULL) {
            LV_LOG_ERROR("Failed to malloc font memory size = %d", file_size);
        }

        while (read_size < file_size) {
            UINT rdsize;
            f_read(&font_binfile, font_files[i].data + read_size, 1024, &rdsize);
            read_size += rdsize;
            if (rdsize < 1024) break;
        }
        if (read_size != file_size) {
            LV_LOG_ERROR("Read size mismatch %d != %d", read_size, file_size);
        }
        f_close(&font_binfile);
    }
end:
    lv_mem_free(path);
    return;
}

