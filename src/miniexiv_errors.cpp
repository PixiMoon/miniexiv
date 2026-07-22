#include "miniexiv/miniexiv_errors.h"
#include <string>

namespace ErrorMessages {
    const std::string IMAGE_NULL         = "Image is null";
    const std::string IMAGE_ERROR_OPEN_IO = "Failed to open image IO";
    const std::string IMAGE_SIZE_ZERO = "Image buffer size is zero";
    const std::string IMAGE_ERROR_READ_BUF = "Failed to read image buffer";
    const std::string ITERATOR_NULL = "Iterator is null";
    const std::string ITERATOR_STATE_KEY = "Invalid iterator state for getting key";
    const std::string ITERATOR_STATE_VALUE = "Invalid iterator state for getting value";
    const std::string OUT_BUFF_NULL = "Out buffer is null";
    const std::string OUT_VALUE_NULL = "Out value is null";
    const std::string KEY_NULL_EMPTY     = "Key is null or empty";
    const std::string VALUE_NULL         = "Value is null";
    const std::string FILE_NULL_EMPTY    = "File name is null or empty";
    const std::string FILE_ERROR_CREATE = "Failed to create file";
    const std::string FILE_ERROR_WRITE = "Failed to write data into file";
    const std::string BUF_NULL_ZERO      = "Buffer is null or size is zero";
    const std::string OUT_PTR_NULL       = "Output pointer is null";
    const std::string MEMORY_FAIL        = "Memory allocation failed";
    const std::string NO_ERROR           = "No Error";
    const std::string KEY_NOT_FOUND      = "Key not found";
    const std::string UNKNOWN_EX_ERROR = "Unknown C++ exception";
    const std::string INVALID_METADATA_ID = "Invalid Metadata Id";
    const std::string UNKNOWN_EX_EXPORT_BUF = "Unknown C++ exception when export buffer";
    const std::string UNKNOWN_EX_ITERATOR = "Unknown C++ exception inside iterator";
    const std::string UNKNOWN_EX_OPEN_BUF = "Unknown C++ exception while opening image buffer";
    const std::string UNKNOWN_EX_OPEN    = "Unknown C++ exception while opening file";
    const std::string UNKNOWN_EX_CLEAR   = "Unknown C++ exception while clearing metadata";
    const std::string UNKNOWN_EX_WRITE   = "Unknown exception while writing metadata";
    const std::string UNKNOWN_EX_WRITE_FILE   = "Unknown exception while writing file";
    const std::string UNKNOWN_EX_EXIF    = "Unknown C++ exception inside EXIF operation";
    const std::string UNKNOWN_EX_XMP     = "Unknown C++ exception inside XMP operation";
    const std::string UNKNOWN_EX_IPTC    = "Unknown C++ exception inside IPTC operation";
    const std::string UNKNOWN_EX_DIM     = "Unknown C++ exception while getting dimensions";
}