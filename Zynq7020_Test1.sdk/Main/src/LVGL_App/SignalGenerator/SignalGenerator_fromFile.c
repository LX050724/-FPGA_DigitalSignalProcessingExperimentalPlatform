//
// Created by yaoji on 2022/2/1.
//

#include "SignalGenerator_fromFile.h"
#include "Fatfs_init/Fatfs_Driver.h"
#include "FileDecoder/FileDecoder.h"
#include "LVGL_Utils/MessageBox.h"
#include "Controller/DDS_Controller.h"

static lv_table_t *file_table;
static lv_label_t *path_label;
static lv_obj_t *field_list_dropdown;
static lv_obj_t *file_type_label;
static lv_obj_t *start_btn;

static char *current_dir;
static size_t path_len;
static char *selected_file;

static void scan_file(lv_table_t *obj, const char *dir);
static void file_table_click_event(lv_event_t *e);
static void start_btn_click_cb(lv_event_t *e);

void fromFile_create(lv_obj_t *parent) {
    path_len = 20;
    current_dir = lv_mem_alloc(path_len);
    LV_ASSERT_MALLOC(current_dir)
    strcpy(current_dir, "0:/信号发生器/");

    char *buf = UTF8_TO_GBK("0:/信号发生器");
    DIR dir;
    FRESULT res = f_opendir(&dir, buf);
    if (res == FR_OK)
        f_closedir(&dir);
    else if (res == FR_NO_PATH) {
        res = f_mkdir(buf);
        if (res != FR_OK) {
            LV_LOG_ERROR("创建文件夹 \"0:/信号发生器\" 失败");
            strcpy(current_dir, "0:/");
        }
    }
    lv_mem_free(buf);

    path_label = lv_label_create(parent);
    lv_label_set_text_static(path_label, current_dir);
    lv_label_set_long_mode(path_label, LV_LABEL_LONG_SCROLL);
    lv_obj_set_width(path_label, LV_HOR_RES * 0.8);

    file_table = lv_table_create(parent);
    lv_table_set_row_cnt(file_table, 1);
    lv_table_set_col_cnt(file_table, 1);
    lv_obj_set_width(file_table, LV_HOR_RES * 0.3);
    lv_obj_set_height(file_table, LV_VER_RES * 0.65);
    lv_table_set_col_width(file_table, 0, LV_HOR_RES * 0.28);
    lv_obj_align_to(file_table, path_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

    file_type_label = lv_label_create(parent);
    lv_label_set_text_static(file_type_label, "选择的文件:             文件类型:");
    lv_obj_align_to(file_type_label, file_table, LV_ALIGN_OUT_RIGHT_TOP, 50, 0);

    lv_obj_t *dropdown_label = lv_label_create(parent);
    lv_label_set_text_static(dropdown_label, "选择字段");
    lv_obj_align_to(dropdown_label, file_type_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);

    field_list_dropdown = lv_dropdown_create(parent);
    lv_dropdown_clear_options(field_list_dropdown);
    lv_obj_add_state(field_list_dropdown, LV_STATE_DISABLED);
    lv_obj_align_to(field_list_dropdown, dropdown_label, LV_ALIGN_OUT_RIGHT_MID, 20, 0);

    start_btn = lv_btn_create(parent);
    lv_obj_add_flag(start_btn, LV_OBJ_FLAG_FLOATING);
    lv_obj_align(start_btn, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
    lv_obj_t *start_btn_label = lv_label_create(start_btn);
    lv_obj_set_style_text_font(start_btn_label, &msyhl_24, 0);
    lv_label_set_text_static(start_btn_label, "启动输出");
    lv_obj_add_event_cb(start_btn, start_btn_click_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_add_event_cb(file_table, file_table_click_event, LV_EVENT_VALUE_CHANGED, NULL);
    scan_file(file_table, current_dir);
}

static void file_table_click_event(lv_event_t *e) {
    LV_UNUSED(e);
    uint16_t row, col;
    lv_table_get_selected_cell(file_table, &row, &col);
    if (col) return;

    const char *filename = lv_table_get_cell_value(file_table, row, 0);

    if (lv_table_has_cell_ctrl(file_table, row, 0, LV_TABLE_CELL_CTRL_CUSTOM_1)) {
        /* 点击了一个文件夹，判断是返回上一级还是进入子文件夹 */
        if (lv_table_has_cell_ctrl(file_table, row, 0, LV_TABLE_CELL_CTRL_CUSTOM_2)) {
            /* 返回上一级，清除最后一个分隔符及之后的字符 */
            size_t len = strlen(current_dir);
            current_dir[len - 1] = '\0';
            for (size_t i = len - 2; i > 0; i--) {
                if (current_dir[i] == '/') break;
                current_dir[i] = '\0';
            }
        } else {
            /* 进入子文件夹，拼接路径名 */
            size_t new_len = strlen(current_dir) + strlen(filename) + 1;
            /* 如果内存不足，重新分配，并刷新path_label指针 */
            if (path_len < new_len) {
                path_len = new_len;
                current_dir = lv_mem_realloc(current_dir, path_len);
                LV_ASSERT_MALLOC(current_dir)
                lv_label_set_text_static(path_label, current_dir);
            }
            strcat(current_dir, filename);
        }
        /* 重新扫描文件夹 */
        scan_file(file_table, current_dir);
    } else {
        selected_file = lv_mem_realloc(selected_file, strlen(current_dir) + strlen(filename) + 1);
        LV_ASSERT_MALLOC(selected_file)
        strcpy(selected_file, current_dir);
        strcat(selected_file, filename);

        lv_obj_add_state(field_list_dropdown, LV_STATE_DISABLED);
        lv_dropdown_clear_options(field_list_dropdown);
        lv_dropdown_set_text(field_list_dropdown, NULL);

        FDType type = FileDecoder_get_file_type(selected_file);
        if (type == FDType_unknown) {
            lv_obj_add_state(start_btn, LV_STATE_DISABLED);
            lv_mem_free(selected_file);
            selected_file = NULL;
        } else if (type == FDType_json) {
            const char **field_list;
            size_t len;
            FDStatus status = FileDecoder_get_json_field(selected_file, &field_list, &len);
            if (status == FDStatus_array_file) {
                LV_LOG_USER("array file");
                lv_dropdown_set_text(field_list_dropdown, "array");
                lv_obj_clear_state(start_btn, LV_STATE_DISABLED);
            } else if (status == FDStatus_ok) {
                for (int i = 0; i < len; i++) {
                    lv_dropdown_add_option(field_list_dropdown, field_list[i], LV_DROPDOWN_POS_LAST);
                    lv_mem_free(field_list[i]);
                }
                lv_mem_free(field_list);
                lv_obj_clear_state(field_list_dropdown, LV_STATE_DISABLED);
                lv_obj_clear_state(start_btn, LV_STATE_DISABLED);
            } else {
                LV_LOG_ERROR("%s", FileDecoder_status_string(status));
            }
        } else {
            lv_obj_clear_state(start_btn, LV_STATE_DISABLED);
            lv_dropdown_set_text(field_list_dropdown, FileDecoder_type_string(type));
        }
        lv_label_set_text_fmt(file_type_label, "选择的文件: %s  文件类型: %s", filename, FileDecoder_type_string(type));
    }
}

static void scan_file(lv_table_t *obj, const char *dir) {
    char *buf = UTF8_TO_GBK(dir);
    buf[strlen(buf) - 1] = 0;

    int index = 0;
    /* 不是根目录第一项加上一级 */
    if (buf[2] != 0) {
        lv_table_set_cell_value(obj, 0, 0, "返回上一级");
        lv_table_add_cell_ctrl(obj, 0, 0,
                               LV_TABLE_CELL_CTRL_CUSTOM_1 | LV_TABLE_CELL_CTRL_CUSTOM_2);
        index = 1;
    }

    DIR root_dir;
    FILINFO file_info;
    FRESULT res = f_opendir(&root_dir, buf);
    if (res != FR_OK) {
        LV_LOG_ERROR("open dir %s error, return %d", buf, res);
        lv_mem_free(buf);
        return;
    }

    while (f_readdir(&root_dir, &file_info) == FR_OK) {
        if (file_info.fname[0] == 0) break;
        /* 跳过系统文件和隐藏文件 */
        if (file_info.fattrib & (AM_HID | AM_SYS)) continue;

        char *utf8 = GBK_TO_UTF8(file_info.fname);
        lv_table_clear_cell_ctrl(obj, index, 0,
                                 LV_TABLE_CELL_CTRL_CUSTOM_1 | LV_TABLE_CELL_CTRL_CUSTOM_2);
        if (file_info.fattrib & AM_DIR) {
            lv_table_add_cell_ctrl(obj, index, 0, LV_TABLE_CELL_CTRL_CUSTOM_1);
            lv_table_set_cell_value_fmt(obj, index, 0, "%s/", utf8);
        } else {
            lv_table_set_cell_value(obj, index, 0, utf8);
        }
        lv_mem_free(utf8);
        index++;
    }

    lv_table_set_row_cnt(obj, index);

    res = f_closedir(&root_dir);
    if (res != FR_OK) {
        LV_LOG_ERROR("close dir %s error, return %d", buf, res);
        lv_mem_free(buf);
        return;
    }
    lv_mem_free(buf);

    lv_obj_scroll_to_y(obj, 0, LV_ANIM_OFF);
}

static void start_btn_click_cb(lv_event_t *e) {
    char field[64] = "";
    lv_dropdown_get_selected_str(field_list_dropdown, field, sizeof(field));

    FDType type;
    int8_t *p;
    size_t len;
    FDStatus status = FileDecoder_open(selected_file, field, &type, &p, &len);
    if (status == FDStatus_ok) {
        LV_LOG_USER("启动 读取文件:'%s' 字段:'%s' 类型:'%s' 长度=%d", selected_file, field, FileDecoder_type_string(type), len);
        DDS_wav_from_data(p, len);
        lv_mem_free(p);
    } else {
        InfoMessageBox("错误", FileDecoder_status_string(status), "关闭");
    }
}