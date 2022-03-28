//
// Created by yaoji on 2022/2/1.
//

#include "SignalGenerator_fromFile.h"
#include "Fatfs_init/Fatfs_Driver.h"
#include "FileDecoder/FileDecoder.h"
#include "LVGL_Utils/MessageBox.h"
#include "Controller/DDS_Controller.h"
#include "LVGL_Utils/FileSelectBox.h"

static lv_obj_t *file_table;
static lv_obj_t *path_label;
static lv_obj_t *field_list_dropdown;
static lv_obj_t *file_type_label;
static lv_obj_t *start_btn;

//static char *current_dir;
//static size_t path_len;
//static char *selected_file;

static void start_btn_click_cb(lv_event_t *e);

static void file_table_path_change_cb(lv_obj_t *obj, const char *path);
static void file_table_click_cb(lv_obj_t *obj, const char *path, const char *filename);

void fromFile_create(lv_obj_t *parent) {
    path_label = lv_label_create(parent);
    lv_label_set_long_mode(path_label, LV_LABEL_LONG_SCROLL);
    lv_obj_set_width(path_label, LV_HOR_RES * 0.8);

    file_table = lv_file_select_box_create(parent);
    lv_table_set_row_cnt(file_table, 1);
    lv_table_set_col_cnt(file_table, 1);
    lv_obj_set_width(file_table, LV_HOR_RES * 0.3);
    lv_obj_set_height(file_table, LV_VER_RES * 0.65);
    lv_table_set_col_width(file_table, 0, LV_HOR_RES * 0.28);
    lv_obj_align_to(file_table, path_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    lv_file_select_box_set_click_callback(file_table, file_table_click_cb);
    lv_file_select_box_set_path_change_callback(file_table, file_table_path_change_cb);
    lv_obj_update_layout(file_table);


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

    char *buf = UTF8_TO_GBK("0:/信号发生器");
    DIR dir;
    FRESULT res = f_opendir(&dir, buf);
    if (res == FR_OK) {
        lv_file_select_box_set_dir(file_table, "0:/信号发生器/");
        f_closedir(&dir);
    } else if (res == FR_NO_PATH) {
        res = f_mkdir(buf);
        if (res != FR_OK) {
            LV_LOG_ERROR("创建文件夹 \"0:/信号发生器\" 失败");
        } else {
            lv_file_select_box_set_dir(file_table, "0:/信号发生器/");
        }
    }
    lv_mem_free(buf);
}


static void file_table_path_change_cb(lv_obj_t *obj, const char *path) {
    lv_label_set_text_static(path_label, path);
}

static void file_table_click_cb(lv_obj_t *obj, const char *path, const char *filename) {
    lv_obj_add_state(field_list_dropdown, LV_STATE_DISABLED);
    lv_dropdown_clear_options(field_list_dropdown);
    lv_dropdown_set_text(field_list_dropdown, NULL);

    FDType type = FileDecoder_get_file_type(path);
    if (type == FDType_unknown) {
        lv_obj_add_state(start_btn, LV_STATE_DISABLED);
    } else if (type == FDType_json) {
        char **field_list;
        size_t len;
        FDStatus status = FileDecoder_get_json_field(path, &field_list, &len);
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

static void start_btn_click_cb(lv_event_t *e) {
    char field[64] = "";
    lv_dropdown_get_selected_str(field_list_dropdown, field, sizeof(field));

    FDType type;
    int8_t *p;
    size_t len;
    const char *selected_file = lv_file_select_box_get_selected_file(file_table);
    FDStatus status = FileDecoder_open(selected_file, field, &type, &p, &len);
    if (status == FDStatus_ok) {
        LV_LOG_USER("启动 读取文件:'%s' 字段:'%s' 类型:'%s' 长度=%d", selected_file, field, FileDecoder_type_string(type), len);
        DDS_wav_from_data(p, len);
        lv_mem_free(p);
    } else {
        InfoMessageBox("错误", FileDecoder_status_string(status), "关闭");
    }
}