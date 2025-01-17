//
// Created by yaoji on 2022/3/28.
//

#include "FileSelectBox.h"
#include "Fatfs_init/Fatfs_Driver.h"
#include "MessageBox.h"
#include "utils/str_tool.h"

#define FILE_SELECT_BACK_CTRL LV_TABLE_CELL_CTRL_CUSTOM_2
#define FILE_SELECT_DIR_CTRL LV_TABLE_CELL_CTRL_CUSTOM_1

typedef struct {
    char *current_dir;
    char *selected_file;
    const char *delete_filename;
    uint32_t path_len;
    lv_file_sel_click_cb_t click_callback;
    lv_file_sel_path_change_cb_t path_change_callback;
} FileSelectBox_UserData;

static void file_table_click_event(lv_event_t *event);
static void file_table_delete_event(lv_event_t *event);
static void lv_file_table_long_pressed_event(lv_event_t *event);
static void scan_file(lv_obj_t *obj, const char *dir);

lv_obj_t *lv_file_select_box_create(lv_obj_t *parent) {
    FileSelectBox_UserData *userData = lv_mem_alloc(sizeof(FileSelectBox_UserData));
    if (userData == NULL) return NULL;
    memset(userData, 0, sizeof(FileSelectBox_UserData));

    lv_obj_t *file_table = lv_table_create(parent);
    lv_table_set_row_cnt(file_table, 1);
    lv_table_set_col_cnt(file_table, 1);
    lv_obj_set_user_data(file_table, userData);
    lv_obj_add_event_cb(file_table, file_table_click_event, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(file_table, file_table_delete_event, LV_EVENT_DELETE, NULL);
    lv_obj_add_event_cb(file_table, lv_file_table_long_pressed_event, LV_EVENT_LONG_PRESSED, NULL);
    lv_file_select_box_set_dir(file_table, "0:/");
    return file_table;
}

static void delete_file_callback(uint16_t index, void *userdata) {
    lv_obj_t *table = userdata;
    FileSelectBox_UserData *fs_data = table->user_data;

    if (index) {
        fs_data->delete_filename = NULL;
        return;
    }

    char *path = str_malloc_cat(fs_data->current_dir, fs_data->delete_filename, 0);
    if (path == NULL) goto err;
    int path_len = strlen(path);
    if (path[path_len - 1] == '/' || path[path_len - 1] == '\\') path[path_len - 1] = 0;

    char *gbk = UTF8_TO_GBK(path);
    if (Fatfs_rm_rf(gbk) != FR_OK) goto err;
    os_free(gbk);

    os_free(path);

    MessageBox_info("删除文件", "关闭", "'%s'删除成功", fs_data->delete_filename);
    fs_data->delete_filename = NULL;
    scan_file(userdata, fs_data->current_dir);
    return;
    err:
    MessageBox_info("删除文件", "关闭", "'%s'删除失败", fs_data->delete_filename);
    fs_data->delete_filename = NULL;
}

static void lv_file_table_long_pressed_event(lv_event_t *event) {
    lv_obj_t *target = lv_event_get_target(event);
    uint16_t row, col;
    lv_table_get_selected_cell(target, &row, &col);

    if (row >= lv_table_get_row_cnt(target) ||
        col >= lv_table_get_col_cnt(target))
        return;

    if (lv_table_has_cell_ctrl(target, row, col, FILE_SELECT_BACK_CTRL))
        return;

    const char *filename = lv_table_get_cell_value(target, row, col);
    FileSelectBox_UserData *fs_data = target->user_data;
    fs_data->delete_filename = filename;

    if (lv_table_has_cell_ctrl(target, row, col, FILE_SELECT_DIR_CTRL)) {
        MessageBox_question("删除文件", "是", "否", delete_file_callback,
                            target, "是否删除文件夹'%s'?", filename);
    } else {
        MessageBox_question("删除文件", "是", "否", delete_file_callback,
                            target, "是否删除文件'%s'?", filename);
    }
}

void lv_file_select_box_set_click_callback(lv_obj_t *obj, lv_file_sel_click_cb_t callback) {
    if (obj == NULL) return;
    FileSelectBox_UserData *userData = lv_obj_get_user_data(obj);
    if (userData != NULL) userData->click_callback = callback;
}

void lv_file_select_box_set_path_change_callback(lv_obj_t *obj, lv_file_sel_path_change_cb_t callback) {
    if (obj == NULL) return;
    FileSelectBox_UserData *userData = lv_obj_get_user_data(obj);
    if (userData != NULL) userData->path_change_callback = callback;
}

void lv_file_select_box_set_dir(lv_obj_t *obj, const char *dir) {
    FileSelectBox_UserData *userData = lv_obj_get_user_data(obj);
    if (userData == NULL) return;
    int str_len = strlen(dir) + 1;
    if (userData->current_dir == NULL || userData->path_len < str_len) {
        userData->current_dir = lv_mem_realloc(userData->current_dir, str_len);
    }
    strcpy(userData->current_dir, dir);
    scan_file(obj, userData->current_dir);
    if (userData->path_change_callback)
        userData->path_change_callback(obj, userData->current_dir);
}

const char *lv_file_select_box_get_selected_file(lv_obj_t *obj) {
    FileSelectBox_UserData *userData = lv_obj_get_user_data(obj);
    if (userData == NULL) return NULL;
    return userData->selected_file;
}

static void file_table_click_event(lv_event_t *event) {
    lv_obj_t *file_table = lv_event_get_target(event);
    FileSelectBox_UserData *userData = lv_obj_get_user_data(file_table);
    // 正在进行删除文件操作时不进行文件夹切换
    if (userData->delete_filename) return;
    uint16_t row, col;
    lv_table_get_selected_cell(file_table, &row, &col);
    if (row >= lv_table_get_row_cnt(file_table) || col != 0)
        return;

    const char *filename = lv_table_get_cell_value(file_table, row, 0);
    if (lv_table_has_cell_ctrl(file_table, row, 0, FILE_SELECT_DIR_CTRL)) {
        /* 点击了一个文件夹，判断是返回上一级还是进入子文件夹 */
        if (lv_table_has_cell_ctrl(file_table, row, 0, FILE_SELECT_BACK_CTRL)) {
            /* 返回上一级，清除最后一个分隔符及之后的字符 */
            size_t len = strlen(userData->current_dir);
            userData->current_dir[len - 1] = '\0';
            for (size_t i = len - 2; i > 0; i--) {
                if (userData->current_dir[i] == '/') break;
                userData->current_dir[i] = '\0';
            }
        } else {
            /* 进入子文件夹，拼接路径名 */
            size_t new_len = strlen(userData->current_dir) + strlen(filename) + 1;
            /* 如果内存不足，重新分配，并刷新path_label指针 */
            if (userData->path_len < new_len) {
                userData->path_len = new_len;
                userData->current_dir = lv_mem_realloc(userData->current_dir, userData->path_len);
                LV_ASSERT_MALLOC(userData->current_dir)
            }
            strcat(userData->current_dir, filename);
            if (userData->path_change_callback)
                userData->path_change_callback(file_table, userData->current_dir);
        }
        /* 重新扫描文件夹 */
        scan_file(file_table, userData->current_dir);
        lv_mem_free(userData->selected_file);
        userData->selected_file = NULL;
    } else {
        userData->selected_file = lv_mem_realloc(userData->selected_file,
                                                 strlen(userData->current_dir) + strlen(filename) + 1);
        LV_ASSERT_MALLOC(userData->selected_file)
        strcpy(userData->selected_file, userData->current_dir);
        strcat(userData->selected_file, filename);

        if (userData->click_callback)
            userData->click_callback(file_table, userData->selected_file, filename);
    }
}

static void scan_file(lv_obj_t *obj, const char *dir) {
    char *buf = UTF8_TO_GBK(dir);
    buf[strlen(buf) - 1] = 0;

    int index = 0;
    /* 不是根目录第一项加上一级 */
    if (buf[2] != 0) {
        lv_table_set_cell_value(obj, 0, 0, "返回上一级");
        lv_table_add_cell_ctrl(obj, 0, 0,
                               FILE_SELECT_DIR_CTRL | FILE_SELECT_BACK_CTRL);
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
                                 FILE_SELECT_DIR_CTRL | FILE_SELECT_BACK_CTRL);
        if (file_info.fattrib & AM_DIR) {
            lv_table_add_cell_ctrl(obj, index, 0, FILE_SELECT_DIR_CTRL);
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

static void file_table_delete_event(lv_event_t *event) {
    lv_obj_t *target = lv_event_get_target(event);
    if (target != NULL) {
        FileSelectBox_UserData *userData = lv_obj_get_user_data(target);
        lv_mem_free(userData->current_dir);
        lv_mem_free(userData->selected_file);
        lv_mem_free(userData);
    }
}
