#ifndef MINIEXIV_ERRORS_H
#define MINIEXIV_ERRORS_H

#include <string>

namespace ErrorMessages {
    extern const std::string IMAGE_NULL;
    extern const std::string IMAGE_ERROR_OPEN_IO;
    extern const std::string IMAGE_SIZE_ZERO;
    extern const std::string IMAGE_ERROR_READ_BUF;
    extern const std::string ITERATOR_NULL;
    extern const std::string ITERATOR_STATE_KEY;
    extern const std::string ITERATOR_STATE_VALUE;
    extern const std::string OUT_BUFF_NULL;
    extern const std::string OUT_VALUE_NULL;
    extern const std::string KEY_NULL_EMPTY;
    extern const std::string VALUE_NULL;
    extern const std::string FILE_NULL_EMPTY;
    extern const std::string FILE_ERROR_CREATE;
    extern const std::string FILE_ERROR_WRITE;
    extern const std::string BUF_NULL_ZERO;
    extern const std::string OUT_PTR_NULL;
    extern const std::string MEMORY_FAIL;
    extern const std::string NO_ERROR;
    extern const std::string KEY_NOT_FOUND;
    extern const std::string INVALID_METADATA_ID;
    extern const std::string UNKNOWN_EX_ERROR;
    extern const std::string UNKNOWN_EX_EXPORT_BUF;
    extern const std::string UNKNOWN_EX_OPEN_BUF;
    extern const std::string UNKNOWN_EX_ITERATOR;
    extern const std::string UNKNOWN_EX_OPEN;
    extern const std::string UNKNOWN_EX_CLEAR;
    extern const std::string UNKNOWN_EX_WRITE;
    extern const std::string UNKNOWN_EX_WRITE_FILE;
    extern const std::string UNKNOWN_EX_EXIF;
    extern const std::string UNKNOWN_EX_XMP;
    extern const std::string UNKNOWN_EX_IPTC;
    extern const std::string UNKNOWN_EX_DIM;
}

#endif // MINIEXIV_ERRORS_H