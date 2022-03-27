//
// Created by yaoji on 2022/2/1.
//

#ifndef ZYNQ7020_FILEDECODER_H
#define ZYNQ7020_FILEDECODER_H

#include <stdint.h>
#include <stddef.h>

typedef enum FDStatus {
    FDStatus_ok,
    FDStatus_null,
    FDStatus_file_read_error,
    FDStatus_out_of_memory,
    FDStatus_invalid_file,
    FDStatus_invalid_path,
    FDStatus_json_parse_error,
    FDStatus_json_field_not_found,
    FDStatus_json_format_error,
    FDStatus_coe_format_error,
    FDStatus_coe_width_error,
    FDStatus_array_file,
    FDStatus_end,
} FDStatus;

typedef enum FDType {
    FDType_csv,
    FDType_bin,
    FDType_json,
    FDType_coe,
    FDType_unknown,
    FDType_end,
} FDType;

/**
 *
 * @param status
 * @return
 */
const char *FileDecoder_status_string(FDStatus status);

/**
 *
 * @param type
 * @return
 */
const char *FileDecoder_type_string(FDType type);

/**
 *
 * @param filename
 * @return
 */
FDType FileDecoder_get_file_type(const char *filename);

/**
 *
 * @param filename [in] 文件路径
 * @param p [out] 字段名列表，需要递归释放
 * @param len [out] 字段个数
 * @return
 */
FDStatus FileDecoder_get_json_field(const char *filename, const char ***p, size_t *len);

/**
 * 自动判断类型并读取文件
 * @param filename [in] 文件路径
 * @param field [in] 选择字段，仅json文件有效
 * @param type [out] 返回文件类型
 * @param p [out] 返回数据指针，需要释放，coe文件指针类型需要转换为int16_t
 * @param len [out] 返回数组的长度，coe文件为int16_t的长度
 */
FDStatus FileDecoder_open(const char *filename, const char *field, FDType *type, int8_t **p, size_t *len);

#endif //ZYNQ7020_FILEDECODER_H
