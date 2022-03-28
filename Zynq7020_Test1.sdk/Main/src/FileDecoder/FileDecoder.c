//
// Created by yao on 2022/2/1.
//

#include <string.h>
#include <stdlib.h>
#include "FileDecoder.h"
#include "math.h"
#include "Fatfs_init/Fatfs_Driver.h"
#include "cJSON.h"
#include "FreeRTOS_Mem/FreeRTOS_Mem.h"

#define GOTO_RET(STATUS) { status = (STATUS); goto ret; }

const char *FileDecoder_status_string(FDStatus status) {
    static const char *str[] = {
            "ok",
            "null",
            "file read error",
            "file too lager",
            "out of memory",
            "invalid file",
            "invalid path",
            "json parse error",
            "field not found",
            "json format error",
            "coe format error",
            "coe width error",
            "array file",
    };
    return status < FDStatus_end ? str[status] : NULL;
}

const char *FileDecoder_type_string(FDType type) {
    static const char *str[] = {
            "csv",
            "bin",
            "json",
            "coe",
            "unknown",
    };
    return type < FDType_end ? str[type] : NULL;
}

static double FileDecoder_parse_number(const char *str, int len) {
    double number = 0;
    int dot = 0, sign = 1;
    for (int i = 0; i < len; i++) {
        if (str[i] == ' ') continue;
        if (str[i] >= '0' && str[i] <= '9') {
            if (dot) dot++;
            number = number * 10 + str[i] - '0';
        } else if (dot == 0 && str[i] == '.') {
            dot = 1;
        } else if (str[i] == '-') {
            sign = -1;
        } else if (str[i] == 'e') {
            double e = FileDecoder_parse_number(&str[i + 1], len - i - 1);
            number *= pow(10, e);
            break;
        } else return NAN;
    }
    number = dot ? number / pow(10, dot - 1) : number;
    return number * sign;
}


static uint64_t FileDecoder_parse_number_16(const char *str, int len) {
    uint64_t number = 0;
    for (int i = 0; i < len; i++) {
        if (str[i] == ' ') continue;
        if (str[i] >= '0' && str[i] <= '9') {
            number = (number << 4) | ((str[i] - '0') & 0x0f);
        } else if (str[i] >= 'A' && str[i] <= 'F') {
            number = (number << 4) | ((str[i] - 'A' + 0x0a) & 0x0f);
        } else if (str[i] >= 'a' && str[i] <= 'f') {
            number = (number << 4) | ((str[i] - 'a' + 0x0a) & 0x0f);
        } else return 0;
    }
    return number;
}

static uint64_t FileDecoder_parse_number_2(const char *str, int len) {
    uint64_t number = 0;
    for (int i = 0; i < len; i++) {
        if (str[i] == ' ') continue;
        if (str[i] == '0' || str[i] == '1')
            number = (number << 1) | (str[i] - '0');
        else return 0;
    }
    return number;
}

FDType FileDecoder_get_file_type(const char *filename) {
    if (filename == NULL) return FDType_unknown;
    size_t len = strlen(filename);
    char suffix[10] = {0};
    for (size_t i = len - 1; i >= 0; i--) {
        if (filename[i] == '.') {
            strcpy(suffix, filename + i + 1);
            break;
        }
        if (filename[i] == '/' || filename[i] == '\\')
            return FDType_unknown;
    }

    if (suffix[0] == 0)
        return FDType_unknown;

    for (int i = 0; i <= 10; i++) {
        if (suffix[i] == '\0') break;
        if (suffix[i] <= 'Z') suffix[i] += 'a' - 'A';
    }

    if (strcmp(suffix, "csv") == 0)
        return FDType_csv;
    if (strcmp(suffix, "bin") == 0)
        return FDType_bin;
    if (strcmp(suffix, "json") == 0)
        return FDType_json;
    if (strcmp(suffix, "coe") == 0)
        return FDType_coe;
    return FDType_unknown;
}


static FDStatus FileDecoder_decode_bin(const char *filename, int8_t **p, size_t *len) {
    if (p == NULL || filename == NULL || len == NULL)
        return FDStatus_null;

    int8_t *buf = NULL;

    FIL file;
    if (f_open(&file, filename, FA_READ) != FR_OK) return FDStatus_invalid_path;

    if (f_size(&file) > FD_FILE_SIZE_MAX) {
        f_close(&file);
        return FDStatus_file_too_lager_error;
    }

    long size = f_size(&file);
    buf = os_malloc(size);
    if (buf == NULL) return FDStatus_out_of_memory;

    UINT read_size;
    if (f_read(&file, buf, size, &read_size) == FR_OK && size == read_size) {
        f_close(&file);
        *len = size;
        *p = buf;
        return FDStatus_ok;
    } else {
        os_free(buf);
        f_close(&file);
        return FDStatus_file_read_error;
    }
}

static FDStatus FileDecoder_decode_csv(const char *filename, int8_t **p, size_t *len) {
    FDStatus status = FDStatus_ok;
    FIL file;
    size_t buf_len = 128, buf_used_len = 0;

    if (f_open(&file, filename, FA_READ) != FR_OK) return FDStatus_invalid_path;

    int8_t *buf = os_malloc(128);
    if (buf == NULL) GOTO_RET(FDStatus_out_of_memory)

    char text_buf[128];
    UINT read_len;
    if (f_read(&file, text_buf, 128, &read_len) != FR_OK) GOTO_RET(FDStatus_file_read_error)
    while (read_len > 0) {
        int start = 0;
        int i;
        for (i = 0; i < read_len; i++) {
            if (text_buf[i] == ',' || text_buf[i] == '\n' || (read_len < 128 && i == read_len - 1)) {
                if (buf_used_len == buf_len) {
                    buf = realloc(buf, buf_len + 128);
                    if (buf == NULL) GOTO_RET(FDStatus_out_of_memory)
                    buf_len += 128;
                }
                buf[buf_used_len++] = (int8_t) FileDecoder_parse_number(text_buf + start, i - start);
                start = i + 1;
            }
        }
        if (read_len < 128) break;
        if (start < 128)
            f_lseek(&file, f_tell(&file) + start - 128);
        if (f_read(&file, text_buf, 128, &read_len) != FR_OK) GOTO_RET(FDStatus_file_read_error)
    }
    buf = realloc(buf, buf_used_len);
    *p = buf;
    *len = buf_used_len;
    f_close(&file);
    return status;

    ret:
    os_free(buf);
    f_close(&file);
    return status;
}

static FDStatus FileDecoder_decode_json(const char *filename, int8_t **p, size_t *len, const char *field) {
    FDStatus status;
    int8_t *buf = NULL;
    size_t file_size;
    char *file;
    status = FileDecoder_decode_bin(filename, (int8_t **) &file, &file_size);
    if (status != FDStatus_ok) return status;

    cJSON *json = cJSON_ParseWithLength(file, file_size);
    if (json == NULL) {
        os_free(file);
        return FDStatus_json_parse_error;
    }

    cJSON *array = NULL;
    if (cJSON_IsArray(json)) array = json;
    else {
        array = cJSON_GetObjectItem(json, field);
        if (array == NULL) GOTO_RET(FDStatus_json_field_not_found)
    }

    if (!cJSON_IsArray(array)) GOTO_RET(FDStatus_json_format_error)

    int size = cJSON_GetArraySize(array);
    if (size == 0) GOTO_RET(FDStatus_json_format_error)

    buf = os_malloc(size);
    if (buf == NULL) GOTO_RET(FDStatus_out_of_memory)

    for (int i = 0; i < size; i++) {
        cJSON *item = cJSON_GetArrayItem(array, i);
        if (cJSON_IsNumber(item))
            buf[i] = (int8_t) cJSON_GetNumberValue(item);
        else GOTO_RET(FDStatus_out_of_memory)
    }
    *len = size;
    *p = buf;

    ret:
    cJSON_free(json);
    os_free(file);
    return status;
}

static FDStatus FileDecoder_decode_coe(const char *filename, int16_t **p, size_t *len) {
    FIL file;
    if (f_open(&file, filename, FA_READ) != FR_OK) return FDStatus_invalid_path;
    FDStatus status = FDStatus_ok;

    int radix = 0;
    int coefficient_width = 0;
    int16_t *data = os_malloc(sizeof(int16_t) * 64);
    if (data == NULL) GOTO_RET(FDStatus_out_of_memory)
    int data_start = 0;
    int data_len = 64, data_used_len = 0;

    char text_buf[512];
    while (1) {
        char *line = f_gets(text_buf, 512, &file);
        if (line == NULL) break;
        int line_len = 0;
        while (line_len < 512 && line[line_len] != '\n' && line[line_len] != ';') line_len++;
        while (line[0] == ' ' || line[0] == '\t') {
            line++;
            line_len--;
        }
        if (line_len == 0) continue;

        if (radix == 0 || coefficient_width == 0 || data_start == 0) {
            if (line[0] == 'R' || line[0] == 'r' || line[0] == 'C' || line[0] == 'c') {
                int equ = 0;
                while (equ < line_len && line[equ] != '=') equ++;
                if (line[equ] != '=') GOTO_RET(FDStatus_coe_format_error)
                int flied_end = equ - 1;
                while (equ >= 0 && line[flied_end] == ' ')flied_end--;

                char xxx[100] = "";
                memcpy(xxx, line, flied_end + 1);

                for (int i = 0; i <= flied_end; i++)
                    if (xxx[i] <= 'Z') xxx[i] += 'a' - 'A';

                if (strcmp(xxx, "radix") == 0) {
                    radix = (int) FileDecoder_parse_number(line + equ + 1, line_len - equ - 1);
                    if (radix != 2 && radix != 10 && radix != 16) GOTO_RET(FDStatus_coe_format_error)
                } else if (strcmp(xxx, "coefficient_width") == 0) {
                    coefficient_width = (int) FileDecoder_parse_number(line + equ + 1, line_len - equ - 1);
                    if (coefficient_width != 16) GOTO_RET(FDStatus_coe_width_error)
                } else if (strcmp(xxx, "coefdata") == 0) {
                    line += equ + 1;
                    line_len -= equ + 1;
                    while (*line == ' ') {
                        line++;
                        line_len--;
                    }
                    data_start = 1;
                }
            }
        }
        if (data_start) {
            while (line_len > 0) {
                int number_end = 0;
                while (number_end <= line_len && line[number_end] != ',' && line[number_end] != ';') number_end++;
                if (data_used_len == data_len) {
                    data_len += 64;
                    data = realloc(data, sizeof(int16_t) * data_len);
                    if (data == NULL) GOTO_RET(FDStatus_out_of_memory)
                }
                if (radix == 16)
                    data[data_used_len++] = (int16_t) FileDecoder_parse_number_16(line, number_end);
                else if (radix == 10)
                    data[data_used_len++] = (int16_t) FileDecoder_parse_number(line, number_end);
                else data[data_used_len++] = (int16_t) FileDecoder_parse_number_2(line, number_end);
                line += number_end + 1;
                line_len -= number_end + 1;
            }
        }
    }
    *p = data;
    *len = data_used_len;

    ret:
    f_close(&file);
    return status;
}

FDStatus FileDecoder_get_json_field(const char *filename, char ***p, size_t *len) {
    if (len == NULL || p == NULL)
        return FDStatus_null;
    FDStatus status;
    char **buf = NULL;


    size_t file_size;
    char *file;
    char *filename_GBK = UTF8_TO_GBK(filename);
    status = FileDecoder_decode_bin(filename_GBK, (int8_t **) &file, &file_size);
    os_free(filename_GBK);
    if (status != FDStatus_ok) return status;

    cJSON *json = cJSON_ParseWithLength(file, file_size);
    if (json == NULL) {
        os_free(file);
        return FDStatus_json_parse_error;
    }
    if (cJSON_IsArray(json)) GOTO_RET(FDStatus_array_file)

    int size = 0;
    cJSON *obj = json->child;
    while (obj) {
        if (cJSON_IsArray(obj)) {
            buf = realloc(buf, ++size * sizeof(const char *));
            if (buf == NULL) GOTO_RET(FDStatus_out_of_memory)
            buf[size - 1] = os_malloc(strlen(obj->string));
            strcpy(buf[size - 1], obj->string);
        }
        obj = obj->next;
    }
    *len = size;
    *p = (const char **) buf;
    ret:
    cJSON_free(json);
    os_free(file);
    return status;
}

FDStatus FileDecoder_open(const char *filename, const char *field, FDType *type, int8_t **p, size_t *len) {
    if (p == NULL || filename == NULL || len == NULL || type == NULL)
        return FDStatus_null;
    FDStatus status;
    const char *filename_GBK = UTF8_TO_GBK(filename);
    *type = FileDecoder_get_file_type(filename_GBK);
    switch (*type) {
        case FDType_csv:
            status = FileDecoder_decode_csv(filename_GBK, p, len);
            break;
        case FDType_bin:
            status = FileDecoder_decode_bin(filename_GBK, p, len);
            break;
        case FDType_json:
            status = FileDecoder_decode_json(filename_GBK, p, len, field);
            break;
        case FDType_coe:
            status = FileDecoder_decode_coe(filename_GBK, (int16_t **) p, len);
            break;
        case FDType_unknown:
        default:
            status = FDStatus_invalid_file;
    }
    os_free(filename_GBK);
    return status;
}
