/*
 * MiniExiv - C wrapper for Exiv2
 *
 * Copyright (C) 2026 MiniExiv authors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 or later.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "miniexiv/miniexiv.h"
#include "miniexiv/miniexiv_export.h"
#include "miniexiv/miniexiv_errors.h"
#include <cstring>
#include <cstdlib>
#include <exiv2/basicio.hpp>
#include <exiv2/error.hpp>
#include <exiv2/exif.hpp>
#include <exiv2/exiv2.hpp>
#include <exiv2/image.hpp>
#include <exiv2/properties.hpp>
#include <exiv2/xmp_exiv2.hpp>
#include <fstream>
#include <mutex>
#include <string>
#include <utility>
static std::mutex g_mutex;
static int g_initialized = 0;
static thread_local std::string g_last_error = ErrorMessages::NO_ERROR;
void set_last_error(const std::string &msg) { g_last_error = msg; }
MINIEXIV_EXPORT const char *miniexiv_get_last_error(void) {
  return g_last_error.c_str();
}
struct miniexiv_image {
  std::unique_ptr<Exiv2::Image> exiv2_image;
};
struct miniexiv_exif_iterator {
  Exiv2::ExifData *data;
  Exiv2::ExifData::const_iterator current;
  Exiv2::ExifData::const_iterator end;
  bool start = false;
  std::string key;
  std::string value;
};

struct miniexiv_xmp_iterator {
  Exiv2::XmpData *data;
  Exiv2::XmpData::const_iterator current;
  Exiv2::XmpData::const_iterator end;
  bool start = false;
  std::string key;
  std::string value;
};

struct miniexiv_iptc_iterator {
  Exiv2::IptcData *data;
  Exiv2::IptcData::const_iterator current;
  Exiv2::IptcData::const_iterator end;
  bool start = false;
  std::string key;
  std::string value;
};

MINIEXIV_EXPORT void miniexiv_clear_last_error(void) {
  g_last_error = ErrorMessages::NO_ERROR;
}

MINIEXIV_EXPORT int miniexiv_initialize(void) {
  std::lock_guard<std::mutex> lock(g_mutex);

  if (g_initialized == 0) {
    bool res = Exiv2::XmpParser::initialize();
    if (res == false)
      return MINIEXIV_ERROR;
  }

  g_initialized++;

  return MINIEXIV_OK;
}
MINIEXIV_EXPORT int miniexiv_shutdown(void) {
  std::lock_guard<std::mutex> lock(g_mutex);

  if (g_initialized == 0)
    return MINIEXIV_OK;

  g_initialized--;

  if (g_initialized == 0) {
    Exiv2::XmpParser::terminate();
  }

  return MINIEXIV_OK;
}

MINIEXIV_EXPORT miniexiv_image *miniexiv_image_open_file(const char *filename) {
  if (!filename || filename[0] == '\0') {
    set_last_error(ErrorMessages::FILE_NULL_EMPTY);
    return nullptr;
  }

  try {
    auto image = Exiv2::ImageFactory::open(filename);
    image->readMetadata();
    auto res = new miniexiv_image;
    res->exiv2_image = std::move(image);
    return res;
  } catch (const Exiv2::Error &e) {
    set_last_error(e.what());
    return nullptr;
  } catch (const std::exception &e) {
    set_last_error(e.what());
    return nullptr;
  } catch (...) {
    set_last_error(ErrorMessages::UNKNOWN_EX_OPEN);
    return nullptr;
  }
}

MINIEXIV_EXPORT miniexiv_image *miniexiv_image_open_buf(const uint8_t *buf,
                                                        size_t size) {
  if (!buf || size == 0) {
    set_last_error(ErrorMessages::BUF_NULL_ZERO);
    return nullptr;
  }
  try {
    auto image = Exiv2::ImageFactory::open(buf, size);
    image->readMetadata();
    auto res = new miniexiv_image;
    res->exiv2_image = std::move(image);
    return res;
  } catch (const Exiv2::Error &e) {
    set_last_error(e.what());
    return nullptr;
  } catch (const std::exception &e) {
    set_last_error(e.what());
    return nullptr;
  } catch (...) {
    set_last_error(ErrorMessages::UNKNOWN_EX_OPEN_BUF);
    return nullptr;
  }
}

MINIEXIV_EXPORT int
miniexiv_image_clear_and_write(const miniexiv_image *image) {
  if (!image) {
    set_last_error(ErrorMessages::IMAGE_NULL);
    return MINIEXIV_ERROR;
  }
  try {
    image->exiv2_image->clearExifData();
    image->exiv2_image->clearIptcData();
    image->exiv2_image->clearXmpData();
    image->exiv2_image->clearComment();
    image->exiv2_image->writeMetadata();
    return MINIEXIV_OK;
  } catch (const Exiv2::Error &e) {
    set_last_error(e.what());
    return MINIEXIV_ERROR;
  } catch (const std::exception &e) {
    set_last_error(e.what());
    return MINIEXIV_ERROR;
  } catch (...) {
    set_last_error(ErrorMessages::UNKNOWN_EX_CLEAR);
    return MINIEXIV_ERROR;
  }
}
/**
 * @brief Removes all metadata from the image.
 *
 * Clears all supported metadata from the image, including:
 * - EXIF metadata
 * - IPTC metadata
 * - XMP metadata
 * - Image comment
 *
 * The changes are applied only to the in-memory image object.
 * To make the changes persistent, call
 * miniexiv_image_metadata_write().
 *
 * @param image Image handle.
 *
 * @return MINIEXIV_OK on success.
 * @return MINIEXIV_ERROR on failure.
 *
 * @see miniexiv_image_metadata_write()
 * @see miniexiv_image_clear_and_write()
 */
MINIEXIV_EXPORT int miniexiv_image_clear(const miniexiv_image *image) {
  if (!image) {
    set_last_error(ErrorMessages::IMAGE_NULL);
    return MINIEXIV_ERROR;
  }
  try {
    image->exiv2_image->clearExifData();
    image->exiv2_image->clearIptcData();
    image->exiv2_image->clearXmpData();
    image->exiv2_image->clearComment();
    return MINIEXIV_OK;
  } catch (const Exiv2::Error &e) {
    set_last_error(e.what());
    return MINIEXIV_ERROR;
  } catch (const std::exception &e) {
    set_last_error(e.what());
    return MINIEXIV_ERROR;
  } catch (...) {
    set_last_error(ErrorMessages::UNKNOWN_EX_CLEAR);
    return MINIEXIV_ERROR;
  }
}

MINIEXIV_EXPORT int miniexiv_image_clear_exif(const miniexiv_image *image) {
  if (!image) {
    set_last_error(ErrorMessages::IMAGE_NULL);
    return MINIEXIV_ERROR;
  }

  try {
    image->exiv2_image->clearExifData();
    return MINIEXIV_OK;
  } catch (const Exiv2::Error &e) {
    set_last_error(e.what());
    return MINIEXIV_ERROR;
  } catch (const std::exception &e) {
    set_last_error(e.what());
    return MINIEXIV_ERROR;
  } catch (...) {
    set_last_error(ErrorMessages::UNKNOWN_EX_CLEAR);
    return MINIEXIV_ERROR;
  }
}

MINIEXIV_EXPORT int miniexiv_image_clear_iptc(const miniexiv_image *image) {
  if (!image) {
    set_last_error(ErrorMessages::IMAGE_NULL);
    return MINIEXIV_ERROR;
  }

  try {
    image->exiv2_image->clearIptcData();

    return MINIEXIV_OK;
  } catch (const Exiv2::Error &e) {
    set_last_error(e.what());
    return MINIEXIV_ERROR;
  } catch (const std::exception &e) {
    set_last_error(e.what());
    return MINIEXIV_ERROR;
  } catch (...) {
    set_last_error(ErrorMessages::UNKNOWN_EX_CLEAR);
    return MINIEXIV_ERROR;
  }
}

MINIEXIV_EXPORT int miniexiv_image_clear_icc(const miniexiv_image *image) {

  if (!image) {
    set_last_error(ErrorMessages::IMAGE_NULL);
    return MINIEXIV_ERROR;
  }

  try {
    image->exiv2_image->clearIccProfile();
    return MINIEXIV_OK;
  } catch (const Exiv2::Error &e) {
    set_last_error(e.what());
    return MINIEXIV_ERROR;
  } catch (const std::exception &e) {
    set_last_error(e.what());
    return MINIEXIV_ERROR;
  } catch (...) {
    set_last_error(ErrorMessages::UNKNOWN_EX_CLEAR);
    return MINIEXIV_ERROR;
  }
}
MINIEXIV_EXPORT int miniexiv_image_clear_xmp(const miniexiv_image *image){
  if (!image) {
    set_last_error(ErrorMessages::IMAGE_NULL);
    return MINIEXIV_ERROR;
  }

  try {
    image->exiv2_image->clearXmpData();
    return MINIEXIV_OK;
  } catch (const Exiv2::Error &e) {
    set_last_error(e.what());
    return MINIEXIV_ERROR;
  } catch (const std::exception &e) {
    set_last_error(e.what());
    return MINIEXIV_ERROR;
  } catch (...) {
    set_last_error(ErrorMessages::UNKNOWN_EX_CLEAR);
    return MINIEXIV_ERROR;
  }
}
MINIEXIV_EXPORT int miniexiv_image_clear_comment(const miniexiv_image *image) {
  if (!image) {
    set_last_error(ErrorMessages::IMAGE_NULL);
    return MINIEXIV_ERROR;
  }

  try {
    image->exiv2_image->clearComment();
    return MINIEXIV_OK;
  } catch (const Exiv2::Error &e) {
    set_last_error(e.what());
    return MINIEXIV_ERROR;
  } catch (const std::exception &e) {
    set_last_error(e.what());
    return MINIEXIV_ERROR;
  } catch (...) {
    set_last_error(ErrorMessages::UNKNOWN_EX_CLEAR);
    return MINIEXIV_ERROR;
  }
}

MINIEXIV_EXPORT int miniexiv_image_get_comment(const miniexiv_image *image,
                                               char **out_value) {
  if (!image) {
    set_last_error(ErrorMessages::IMAGE_NULL);
    return MINIEXIV_ERROR;
  }

  if (!out_value) {
    set_last_error(ErrorMessages::OUT_PTR_NULL);
    return MINIEXIV_ERROR;
  }

  *out_value = nullptr;

  try {
    const std::string comment = image->exiv2_image->comment();

    char *buffer = static_cast<char *>(malloc(comment.size() + 1));

    if (!buffer) {
      set_last_error(ErrorMessages::MEMORY_FAIL);
      return MINIEXIV_ERROR;
    }

    memcpy(buffer, comment.c_str(), comment.size() + 1);

    *out_value = buffer;

    return MINIEXIV_OK;

  } catch (const Exiv2::Error &e) {
    set_last_error(e.what());
    return MINIEXIV_ERROR;
  } catch (const std::exception &e) {
    set_last_error(e.what());
    return MINIEXIV_ERROR;
  } catch (...) {
    set_last_error(ErrorMessages::UNKNOWN_EX_ERROR);
    return MINIEXIV_ERROR;
  }
}

MINIEXIV_EXPORT int miniexiv_image_set_comment(miniexiv_image *image,
                                               const char *value) {
  if (!image) {
    set_last_error(ErrorMessages::IMAGE_NULL);
    return MINIEXIV_ERROR;
  }

  if (!value) {
    set_last_error(ErrorMessages::VALUE_NULL);
    return MINIEXIV_ERROR;
  }

  try {
    image->exiv2_image->setComment(value);
    return MINIEXIV_OK;

  } catch (const Exiv2::Error &e) {
    set_last_error(e.what());
    return MINIEXIV_ERROR;
  } catch (const std::exception &e) {
    set_last_error(e.what());
    return MINIEXIV_ERROR;
  } catch (...) {
    set_last_error(ErrorMessages::UNKNOWN_EX_ERROR);
    return MINIEXIV_ERROR;
  }
}

MINIEXIV_EXPORT void miniexiv_image_free(miniexiv_image *ptr) {
  if (ptr) {
    delete ptr;
  }
}

MINIEXIV_EXPORT int miniexiv_image_set_exif_string(miniexiv_image *image,
                                                   const char *key,
                                                   const char *value) {
  if (!image) {
    set_last_error(ErrorMessages::IMAGE_NULL);
    return MINIEXIV_ERROR;
  }

  if (key == nullptr || key[0] == '\0') {
    set_last_error(ErrorMessages::KEY_NULL_EMPTY);
    return MINIEXIV_ERROR;
  }
  if (value == nullptr) {
    set_last_error(ErrorMessages::VALUE_NULL);
    return MINIEXIV_ERROR;
  }
  try {
    Exiv2::ExifData &exif = image->exiv2_image->exifData();
    exif[key] = value;
    return MINIEXIV_OK;
  } catch (const Exiv2::Error &e) {
    set_last_error(e.what());
    return MINIEXIV_ERROR;
  } catch (const std::exception &e) {
    set_last_error(e.what());
    return MINIEXIV_ERROR;
  } catch (...) {
    set_last_error(ErrorMessages::UNKNOWN_EX_EXIF);
    return MINIEXIV_ERROR;
  }
}

MINIEXIV_EXPORT int miniexiv_image_remove_exif(miniexiv_image *image,
                                                const char *key) {
  if (!image) {
    set_last_error(ErrorMessages::IMAGE_NULL);
    return MINIEXIV_ERROR;
  }

  if (!key || key[0] == '\0') {
    set_last_error(ErrorMessages::KEY_NULL_EMPTY);
    return MINIEXIV_ERROR;
  }

  try {
    Exiv2::ExifData &exif = image->exiv2_image->exifData();

    auto pos = exif.findKey(Exiv2::ExifKey(key));

    if (pos != exif.end()) {
      exif.erase(pos);
    }

    return MINIEXIV_OK;
  } catch (const Exiv2::Error &e) {
    set_last_error(e.what());
    return MINIEXIV_ERROR;
  } catch (const std::exception &e) {
    set_last_error(e.what());
    return MINIEXIV_ERROR;
  } catch (...) {
    set_last_error(ErrorMessages::UNKNOWN_EX_EXIF);
    return MINIEXIV_ERROR;
  }
}

MINIEXIV_EXPORT int miniexiv_image_metadata_write(miniexiv_image *image) {
  if (!image) {
    set_last_error(ErrorMessages::IMAGE_NULL);
    return MINIEXIV_ERROR;
  }
  try {
    image->exiv2_image->writeMetadata();

    return MINIEXIV_OK;
  } catch (const Exiv2::Error &e) {
    set_last_error(e.what());
    return MINIEXIV_ERROR;
  } catch (const std::exception &e) {
    set_last_error(e.what());
    return MINIEXIV_ERROR;
  } catch (...) {
    set_last_error(ErrorMessages::UNKNOWN_EX_WRITE);
    return MINIEXIV_ERROR;
  }
}

MINIEXIV_EXPORT int miniexiv_image_has_exif_key(const miniexiv_image *image,
                                                const char *key) {
  if (!image) {
    set_last_error(ErrorMessages::IMAGE_NULL);
    return MINIEXIV_ERROR;
  }

  if (!key || key[0] == '\0') {
    set_last_error(ErrorMessages::KEY_NULL_EMPTY);
    return MINIEXIV_ERROR;
  }

  try {
    const Exiv2::ExifData &exif = image->exiv2_image->exifData();

    auto pos = exif.findKey(Exiv2::ExifKey(key));

    return pos != exif.end() ? 1 : 0;
  } catch (const Exiv2::Error &e) {
    set_last_error(e.what());
    return MINIEXIV_ERROR;
  } catch (const std::exception &e) {
    set_last_error(e.what());
    return MINIEXIV_ERROR;
  } catch (...) {
    set_last_error(ErrorMessages::UNKNOWN_EX_EXIF);
    return MINIEXIV_ERROR;
  }
}

MINIEXIV_EXPORT int miniexiv_image_get_exif_string(const miniexiv_image *image,
                                                   const char *key,
                                                   char **out_value) {
  if (!image) {
    set_last_error(ErrorMessages::IMAGE_NULL);
    return MINIEXIV_ERROR;
  }

  if (!key || key[0] == '\0') {
    set_last_error(ErrorMessages::KEY_NULL_EMPTY);
    return MINIEXIV_ERROR;
  }
  if (!out_value) {
    set_last_error(ErrorMessages::OUT_PTR_NULL);
    return MINIEXIV_ERROR;
  }

  try {
    const Exiv2::ExifData &exif = image->exiv2_image->exifData();

    auto pos = exif.findKey(Exiv2::ExifKey(key));
    if (pos == exif.end()) {
      set_last_error(ErrorMessages::KEY_NOT_FOUND);
      *out_value = nullptr;
      return MINIEXIV_ERROR;
    }
    const std::string str = pos->toString();

    char *buffer = static_cast<char *>(malloc(str.size() + 1));

    if (!buffer) {
      set_last_error(ErrorMessages::MEMORY_FAIL);
      return MINIEXIV_ERROR;
    }

    memcpy(buffer, str.c_str(), str.size() + 1);

    *out_value = buffer;

    return MINIEXIV_OK;
  } catch (const Exiv2::Error &e) {
    set_last_error(e.what());
    return MINIEXIV_ERROR;
  } catch (const std::exception &e) {
    set_last_error(e.what());
    return MINIEXIV_ERROR;
  } catch (...) {
    set_last_error(ErrorMessages::UNKNOWN_EX_EXIF);
    return MINIEXIV_ERROR;
  }
}

MINIEXIV_EXPORT miniexiv_exif_iterator *
miniexiv_exif_iterator_create(const miniexiv_image *image) {
  if (!image) {
    set_last_error(ErrorMessages::IMAGE_NULL);
    return nullptr;
  }
  try {
    auto &exifdata = image->exiv2_image->exifData();
    auto iterator = new miniexiv_exif_iterator;
    iterator->data = &exifdata;
    iterator->current = iterator->data->begin();
    iterator->end = iterator->data->end();
    return iterator;
  } catch (const Exiv2::Error &e) {
    set_last_error(e.what());
    return nullptr;
  } catch (const std::exception &e) {
    set_last_error(e.what());
    return nullptr;
  } catch (...) {
    set_last_error(ErrorMessages::UNKNOWN_EX_ITERATOR);
    return nullptr;
  }
}

MINIEXIV_EXPORT int miniexiv_exif_iterator_next(miniexiv_exif_iterator *it) {
  if (!it) {
    set_last_error(ErrorMessages::ITERATOR_NULL);
    return MINIEXIV_ERROR;
  }
  try {
    if (!it->start) {
      it->start = true;
    } else if (it->current != it->end) {
      it->current++;
    }
    if (it->current == it->end) {
      return MINIEXIV_ITERATOR_END;
    }
    it->key = it->current->key();
    it->value = it->current->value().toString();
    return MINIEXIV_ITERATOR_HAS_NEXT;
  } catch (const Exiv2::Error &e) {
    set_last_error(e.what());
    return MINIEXIV_ERROR;
  } catch (const std::exception &e) {
    set_last_error(e.what());
    return MINIEXIV_ERROR;
  } catch (...) {
    set_last_error(ErrorMessages::UNKNOWN_EX_ITERATOR);
    return MINIEXIV_ERROR;
  }
}

MINIEXIV_EXPORT const char *
miniexiv_exif_iterator_get_key(miniexiv_exif_iterator *it) {
  if (!it) {
    set_last_error(ErrorMessages::ITERATOR_NULL);
    return nullptr;
  }
  if (!it->start || it->current == it->end) {
    set_last_error(ErrorMessages::ITERATOR_STATE_KEY);
    return nullptr;
  }
  return it->key.c_str();
}

MINIEXIV_EXPORT const char *
miniexiv_exif_iterator_get_value(miniexiv_exif_iterator *it) {
  if (!it) {
    set_last_error(ErrorMessages::ITERATOR_NULL);
    return nullptr;
  }

  if (!it->start || it->current == it->end) {
    set_last_error(ErrorMessages::ITERATOR_STATE_VALUE);
    return nullptr;
  }

  return it->value.c_str();
}

MINIEXIV_EXPORT void miniexiv_exif_iterator_free(miniexiv_exif_iterator *it) {
  delete it;
}

MINIEXIV_EXPORT int miniexiv_image_export_buf(const miniexiv_image *image,
                                              uint8_t **out_buf,
                                              size_t *out_size) {
  if (!image) {
    set_last_error(ErrorMessages::IMAGE_NULL);
    return MINIEXIV_ERROR;
  }
  if (!out_buf) {
    set_last_error(ErrorMessages::OUT_BUFF_NULL);
    return MINIEXIV_ERROR;
  }
  if (!out_size) {
    set_last_error(ErrorMessages::OUT_VALUE_NULL);
    return MINIEXIV_ERROR;
  }

  try {
    *out_buf = nullptr;
    *out_size = 0;
    auto &io = image->exiv2_image->io();

    if (io.open() != 0) {
      set_last_error(ErrorMessages::IMAGE_ERROR_OPEN_IO);
      return MINIEXIV_ERROR;
    }

    io.seek(0, Exiv2::BasicIo::beg);

    size_t size = io.size();
    if (size == 0) {
      io.close();
      set_last_error(ErrorMessages::IMAGE_SIZE_ZERO);
      return MINIEXIV_ERROR;
    }
    uint8_t *buffer = static_cast<uint8_t *>(malloc(size));

    if (!buffer) {
      io.close();
      set_last_error(ErrorMessages::MEMORY_FAIL);
      return MINIEXIV_ERROR;
    }

    size_t read_byte = io.read(buffer, size);

    if (read_byte != size) {
      free(buffer);

      set_last_error(ErrorMessages::IMAGE_ERROR_READ_BUF);

      return MINIEXIV_ERROR;
    }
    io.close();
    *out_buf = buffer;
    *out_size = size;

    return MINIEXIV_OK;
  } catch (const Exiv2::Error &e) {
    set_last_error(e.what());
    return MINIEXIV_ERROR;
  } catch (const std::exception &e) {
    set_last_error(e.what());
    return MINIEXIV_ERROR;
  } catch (...) {
    set_last_error(ErrorMessages::UNKNOWN_EX_EXPORT_BUF);
    return MINIEXIV_ERROR;
  }
}

MINIEXIV_EXPORT void miniexiv_free_buffer(uint8_t *buffer) { free(buffer); }

MINIEXIV_EXPORT int miniexiv_image_set_xmp_string(const miniexiv_image *image,
                                                  const char *key,
                                                  const char *value) {
  if (!image) {
    set_last_error(ErrorMessages::IMAGE_NULL);
    return MINIEXIV_ERROR;
  }

  if (key == nullptr || key[0] == '\0') {
    set_last_error(ErrorMessages::KEY_NULL_EMPTY);
    return MINIEXIV_ERROR;
  }
  if (value == nullptr) {
    set_last_error(ErrorMessages::VALUE_NULL);
    return MINIEXIV_ERROR;
  }
  try {
    Exiv2::XmpData &xmp = image->exiv2_image->xmpData();
    xmp[key] = value;
    return MINIEXIV_OK;
  } catch (const Exiv2::Error &e) {
    set_last_error(e.what());
    return MINIEXIV_ERROR;
  } catch (const std::exception &e) {
    set_last_error(e.what());
    return MINIEXIV_ERROR;
  } catch (...) {
    set_last_error(ErrorMessages::UNKNOWN_EX_XMP);
    return MINIEXIV_ERROR;
  }
}

MINIEXIV_EXPORT int miniexiv_image_has_xmp_key(const miniexiv_image *image,
                                               const char *key) {
  if (!image) {
    set_last_error(ErrorMessages::IMAGE_NULL);
    return MINIEXIV_ERROR;
  }

  if (!key || key[0] == '\0') {
    set_last_error(ErrorMessages::KEY_NULL_EMPTY);
    return MINIEXIV_ERROR;
  }

  try {
    const Exiv2::XmpData &xmp = image->exiv2_image->xmpData();

    auto pos = xmp.findKey(Exiv2::XmpKey(key));

    return pos != xmp.end() ? MINIEXIV_TRUE : MINIEXIV_FALSE;
  } catch (const Exiv2::Error &e) {
    set_last_error(e.what());
    return MINIEXIV_ERROR;
  } catch (const std::exception &e) {
    set_last_error(e.what());
    return MINIEXIV_ERROR;
  } catch (...) {
    set_last_error(ErrorMessages::UNKNOWN_EX_XMP);
    return MINIEXIV_ERROR;
  }
}

MINIEXIV_EXPORT int miniexiv_image_get_xmp_string(const miniexiv_image *image,
                                                  const char *key,
                                                  char **out_value) {
  
    if (!image) {
      set_last_error(ErrorMessages::IMAGE_NULL);
      return MINIEXIV_ERROR;
    }

    if (!key || key[0] == '\0') {
      set_last_error(ErrorMessages::KEY_NULL_EMPTY);
      return MINIEXIV_ERROR;
    }
    if (!out_value) {
      set_last_error(ErrorMessages::OUT_PTR_NULL);
      return MINIEXIV_ERROR;
    }

    try {
      const Exiv2::XmpData &xmp = image->exiv2_image->xmpData();

      auto pos = xmp.findKey(Exiv2::XmpKey(key));

      if (pos == xmp.end()) {
        set_last_error(ErrorMessages::KEY_NOT_FOUND);
        *out_value = nullptr;
        return MINIEXIV_ERROR;
      }
      const std::string str = pos->toString();
      char *buffer = static_cast<char *>(malloc(str.size() + 1));

      if (!buffer) {
        set_last_error(ErrorMessages::MEMORY_FAIL);
        return MINIEXIV_ERROR;
      }

      memcpy(buffer, str.c_str(), str.size() + 1);

      *out_value = buffer;

      return MINIEXIV_OK;
    } catch (const Exiv2::Error &e) {
      set_last_error(e.what());
      return MINIEXIV_ERROR;
    } catch (const std::exception &e) {
      set_last_error(e.what());
      return MINIEXIV_ERROR;
    } catch (...) {
      set_last_error(ErrorMessages::UNKNOWN_EX_XMP);
      return MINIEXIV_ERROR;
    }
  }

  MINIEXIV_EXPORT int miniexiv_image_remove_xmp(const miniexiv_image *image,
                                                const char *key) {
    if (!image) {
      set_last_error(ErrorMessages::IMAGE_NULL);
      return MINIEXIV_ERROR;
    }

    if (!key || key[0] == '\0') {
      set_last_error(ErrorMessages::KEY_NULL_EMPTY);
      return MINIEXIV_ERROR;
    }
    try {
      Exiv2::XmpData &xmp = image->exiv2_image->xmpData();

      auto pos = xmp.findKey(Exiv2::XmpKey(key));
      if (pos != xmp.end()) {
        xmp.erase(pos);
      }

      return MINIEXIV_OK;
    } catch (const Exiv2::Error &e) {
      set_last_error(e.what());
      return MINIEXIV_ERROR;
    } catch (const std::exception &e) {
      set_last_error(e.what());
      return MINIEXIV_ERROR;
    } catch (...) {
      set_last_error(ErrorMessages::UNKNOWN_EX_XMP);
      return MINIEXIV_ERROR;
    }
  }
  MINIEXIV_EXPORT int miniexiv_image_export_xmp(const miniexiv_image *image,
                                                const char *filename) {
    if (!image) {
      set_last_error(ErrorMessages::IMAGE_NULL);
      return MINIEXIV_ERROR;
    }
    if (!filename || filename[0] == '\0') {
      set_last_error(ErrorMessages::FILE_NULL_EMPTY);
      return MINIEXIV_ERROR;
    }
    try {
      auto &xmp = image->exiv2_image->xmpPacket();

      std::ofstream file(filename, std::ios::binary);

      if (!file) {
        set_last_error(ErrorMessages::FILE_ERROR_CREATE);
        return MINIEXIV_ERROR;
      }

      file << xmp;

      if (!file) {
        set_last_error(ErrorMessages::FILE_ERROR_WRITE);
        return MINIEXIV_ERROR;
      }

      return MINIEXIV_OK;
    } catch (const Exiv2::Error &e) {
      set_last_error(e.what());
      return MINIEXIV_ERROR;
    } catch (const std::exception &e) {
      set_last_error(e.what());
      return MINIEXIV_ERROR;
    } catch (...) {
      set_last_error(ErrorMessages::UNKNOWN_EX_XMP);
      return MINIEXIV_ERROR;
    }
  }

  MINIEXIV_EXPORT int miniexiv_image_set_iptc_string(
      const miniexiv_image *image, const char *key, const char *value) {
    if (!image) {
      set_last_error(ErrorMessages::IMAGE_NULL);
      return MINIEXIV_ERROR;
    }

    if (key == nullptr || key[0] == '\0') {
      set_last_error(ErrorMessages::KEY_NULL_EMPTY);
      return MINIEXIV_ERROR;
    }
    if (value == nullptr) {
      set_last_error(ErrorMessages::VALUE_NULL);
      return MINIEXIV_ERROR;
    }
    try {
      Exiv2::IptcData &iptc = image->exiv2_image->iptcData();
      iptc[key] = value;
      return MINIEXIV_OK;
    } catch (const Exiv2::Error &e) {
      set_last_error(e.what());
      return MINIEXIV_ERROR;
    } catch (const std::exception &e) {
      set_last_error(e.what());
      return MINIEXIV_ERROR;
    } catch (...) {
      set_last_error(ErrorMessages::UNKNOWN_EX_IPTC);
      return MINIEXIV_ERROR;
    }
  }

  MINIEXIV_EXPORT int miniexiv_image_has_iptc_key(const miniexiv_image *image,
                                                  const char *key) {
    if (!image) {
      set_last_error(ErrorMessages::IMAGE_NULL);
      return MINIEXIV_ERROR;
    }

    if (!key || key[0] == '\0') {
      set_last_error(ErrorMessages::KEY_NULL_EMPTY);
      return MINIEXIV_ERROR;
    }

    try {
      const Exiv2::IptcData &iptc = image->exiv2_image->iptcData();

      auto pos = iptc.findKey(Exiv2::IptcKey(key));

      return pos != iptc.end() ? 1 : 0;
    } catch (const Exiv2::Error &e) {
      set_last_error(e.what());
      return MINIEXIV_ERROR;
    } catch (const std::exception &e) {
      set_last_error(e.what());
      return MINIEXIV_ERROR;
    } catch (...) {
      set_last_error(ErrorMessages::UNKNOWN_EX_IPTC);
      return MINIEXIV_ERROR;
    }
  }

  MINIEXIV_EXPORT int miniexiv_image_get_iptc_string(
      const miniexiv_image *image, const char *key, char **out_value) {
    if (!image) {
      set_last_error(ErrorMessages::IMAGE_NULL);
      return MINIEXIV_ERROR;
    }

    if (!out_value) {
      set_last_error(ErrorMessages::OUT_PTR_NULL);
      return MINIEXIV_ERROR;
    }

    *out_value = nullptr;

    if (!key || key[0] == '\0') {
      set_last_error(ErrorMessages::KEY_NULL_EMPTY);
      return MINIEXIV_ERROR;
    }

    try {
      const Exiv2::IptcData &iptc = image->exiv2_image->iptcData();

      auto pos = iptc.findKey(Exiv2::IptcKey(key));

      if (pos == iptc.end()) {
        set_last_error(ErrorMessages::KEY_NOT_FOUND);
        return MINIEXIV_ERROR;
      }

      std::string str = pos->toString();

      char *buffer = static_cast<char *>(malloc(str.size() + 1));

      if (!buffer) {
        set_last_error(ErrorMessages::MEMORY_FAIL);
        return MINIEXIV_ERROR;
      }

      memcpy(buffer, str.c_str(), str.size() + 1);

      *out_value = buffer;

      return MINIEXIV_OK;
    } catch (const Exiv2::Error &e) {
      set_last_error(e.what());
      return MINIEXIV_ERROR;
    } catch (const std::exception &e) {
      set_last_error(e.what());
      return MINIEXIV_ERROR;
    } catch (...) {
      set_last_error(ErrorMessages::UNKNOWN_EX_IPTC);
      return MINIEXIV_ERROR;
    }
  }

  MINIEXIV_EXPORT int miniexiv_image_remove_iptc(const miniexiv_image *image,
                                                 const char *key) {
    if (!image) {
      set_last_error(ErrorMessages::IMAGE_NULL);
      return MINIEXIV_ERROR;
    }

    if (!key || key[0] == '\0') {
      set_last_error(ErrorMessages::KEY_NULL_EMPTY);
      return MINIEXIV_ERROR;
    }
    try {
      Exiv2::IptcData &iptc = image->exiv2_image->iptcData();

      auto pos = iptc.findKey(Exiv2::IptcKey(key));
      if (pos != iptc.end()) {
        iptc.erase(pos);
      }

      return MINIEXIV_OK;
    } catch (const Exiv2::Error &e) {
      set_last_error(e.what());
      return MINIEXIV_ERROR;
    } catch (const std::exception &e) {
      set_last_error(e.what());
      return MINIEXIV_ERROR;
    } catch (...) {
      set_last_error(ErrorMessages::UNKNOWN_EX_IPTC);
      return MINIEXIV_ERROR;
    }
  }

  MINIEXIV_EXPORT int miniexiv_image_get_dimensions(
      const miniexiv_image *image, uint32_t *width, uint32_t *height) {
    if (!image) {
      set_last_error(ErrorMessages::IMAGE_NULL);
      return MINIEXIV_ERROR;
    }

    if (!width) {
      set_last_error("Width pointer is null");
      return MINIEXIV_ERROR;
    }

    if (!height) {
      set_last_error("Height pointer is null");
      return MINIEXIV_ERROR;
    }

    try {
      *width = image->exiv2_image->pixelWidth();
      *height = image->exiv2_image->pixelHeight();
      return MINIEXIV_OK;
    } catch (const Exiv2::Error &e) {
      set_last_error(e.what());
      return MINIEXIV_ERROR;
    } catch (const std::exception &e) {
      set_last_error(e.what());
      return MINIEXIV_ERROR;
    } catch (...) {
      set_last_error(ErrorMessages::UNKNOWN_EX_DIM);
      return MINIEXIV_ERROR;
    }
  }


  MINIEXIV_EXPORT void miniexiv_free_string(char *ptr) { free(ptr); }

  MINIEXIV_EXPORT int miniexiv_image_save_to_file(const miniexiv_image *image,
                                             const char *filename) {
  if (!image) {
    set_last_error(ErrorMessages::IMAGE_NULL);
    return MINIEXIV_ERROR;
  }

  if (!filename || filename[0] == '\0') {
    set_last_error(ErrorMessages::FILE_NULL_EMPTY);
    return MINIEXIV_ERROR;
  }

  uint8_t *buffer = nullptr;
  size_t size = 0;

  int res = miniexiv_image_export_buf(image, &buffer, &size);

  if (res != MINIEXIV_OK) {
    return MINIEXIV_ERROR;
  }

  try {
    std::ofstream file(filename, std::ios::binary | std::ios::trunc);

    if (!file) {
      miniexiv_free_buffer(buffer);
      set_last_error(ErrorMessages::FILE_ERROR_CREATE);
      return MINIEXIV_ERROR;
    }

    file.write(reinterpret_cast<const char *>(buffer),
               static_cast<std::streamsize>(size));

    if (!file) {
      miniexiv_free_buffer(buffer);
      set_last_error(ErrorMessages::FILE_ERROR_WRITE);
      return MINIEXIV_ERROR;
    }

    file.close();

    miniexiv_free_buffer(buffer);

    return MINIEXIV_OK;

  } catch (const std::exception &e) {
    miniexiv_free_buffer(buffer);
    set_last_error(e.what());
    return MINIEXIV_ERROR;
  } catch (...) {
    miniexiv_free_buffer(buffer);
    set_last_error(ErrorMessages::UNKNOWN_EX_WRITE_FILE);
    return MINIEXIV_ERROR;
  }
}

MINIEXIV_EXPORT miniexiv_xmp_iterator *
miniexiv_xmp_iterator_create(const miniexiv_image *image)
{
  if (!image) {
    set_last_error(ErrorMessages::IMAGE_NULL);
    return nullptr;
  }

  try {
    auto &xmpdata = image->exiv2_image->xmpData();

    auto iterator = new miniexiv_xmp_iterator;

    iterator->data = &xmpdata;
    iterator->current = iterator->data->begin();
    iterator->end = iterator->data->end();

    return iterator;

  } catch (const Exiv2::Error &e) {
    set_last_error(e.what());
    return nullptr;

  } catch (const std::exception &e) {
    set_last_error(e.what());
    return nullptr;

  } catch (...) {
    set_last_error(ErrorMessages::UNKNOWN_EX_XMP);
    return nullptr;
  }
}

MINIEXIV_EXPORT int
miniexiv_xmp_iterator_next(miniexiv_xmp_iterator *it)
{
  if (!it) {
    set_last_error(ErrorMessages::ITERATOR_NULL);
    return MINIEXIV_ERROR;
  }

  try {

    if (!it->start) {
      it->start = true;

    } else if (it->current != it->end) {
      it->current++;
    }


    if (it->current == it->end) {
      return MINIEXIV_ITERATOR_END;
    }


    it->key = it->current->key();
    it->value = it->current->value().toString();


    return MINIEXIV_ITERATOR_HAS_NEXT;


  } catch (const Exiv2::Error &e) {
    set_last_error(e.what());
    return MINIEXIV_ERROR;

  } catch (...) {
    set_last_error(ErrorMessages::UNKNOWN_EX_XMP);
    return MINIEXIV_ERROR;
  }
}

MINIEXIV_EXPORT const char *
miniexiv_xmp_iterator_get_key(miniexiv_xmp_iterator *it)
{
  if (!it) {
    set_last_error(ErrorMessages::ITERATOR_NULL);
    return nullptr;
  }

  if (!it->start || it->current == it->end) {
    set_last_error(ErrorMessages::ITERATOR_STATE_KEY);
    return nullptr;
  }

  return it->key.c_str();
}

MINIEXIV_EXPORT const char *
miniexiv_xmp_iterator_get_value(miniexiv_xmp_iterator *it)
{
  if (!it) {
    set_last_error(ErrorMessages::ITERATOR_NULL);
    return nullptr;
  }

  if (!it->start || it->current == it->end) {
    set_last_error(ErrorMessages::ITERATOR_STATE_VALUE);
    return nullptr;
  }

  return it->value.c_str();
}

MINIEXIV_EXPORT void
miniexiv_xmp_iterator_free(miniexiv_xmp_iterator *it)
{
  delete it;
}

MINIEXIV_EXPORT miniexiv_iptc_iterator *
miniexiv_iptc_iterator_create(const miniexiv_image *image)
{
  if (!image) {
    set_last_error(ErrorMessages::IMAGE_NULL);
    return nullptr;
  }

  try {

    auto &iptcdata = image->exiv2_image->iptcData();

    auto iterator = new miniexiv_iptc_iterator;

    iterator->data = &iptcdata;
    iterator->current = iterator->data->begin();
    iterator->end = iterator->data->end();

    return iterator;


  } catch (const Exiv2::Error &e) {

    set_last_error(e.what());
    return nullptr;

  } catch (...) {

    set_last_error(ErrorMessages::UNKNOWN_EX_IPTC);
    return nullptr;
  }
}

MINIEXIV_EXPORT int
miniexiv_iptc_iterator_next(miniexiv_iptc_iterator *it)
{
  if (!it) {
    set_last_error(ErrorMessages::ITERATOR_NULL);
    return MINIEXIV_ERROR;
  }


  try {

    if (!it->start) {
      it->start = true;

    } else if (it->current != it->end) {
      it->current++;
    }


    if (it->current == it->end) {
      return MINIEXIV_ITERATOR_END;
    }


    it->key = it->current->key();
    it->value = it->current->value().toString();


    return MINIEXIV_ITERATOR_HAS_NEXT;


  } catch (const Exiv2::Error &e) {

    set_last_error(e.what());
    return MINIEXIV_ERROR;

  } catch (...) {

    set_last_error(ErrorMessages::UNKNOWN_EX_IPTC);
    return MINIEXIV_ERROR;
  }
}

MINIEXIV_EXPORT const char *
miniexiv_iptc_iterator_get_key(miniexiv_iptc_iterator *it)
{
  if (!it) {
    set_last_error(ErrorMessages::ITERATOR_NULL);
    return nullptr;
  }

  if (!it->start || it->current == it->end) {
    set_last_error(ErrorMessages::ITERATOR_STATE_KEY);
    return nullptr;
  }

  return it->key.c_str();
}

MINIEXIV_EXPORT const char *
miniexiv_iptc_iterator_get_value(miniexiv_iptc_iterator *it)
{
  if (!it) {
    set_last_error(ErrorMessages::ITERATOR_NULL);
    return nullptr;
  }

  if (!it->start || it->current == it->end) {
    set_last_error(ErrorMessages::ITERATOR_STATE_VALUE);
    return nullptr;
  }

  return it->value.c_str();
}

MINIEXIV_EXPORT void
miniexiv_iptc_iterator_free(miniexiv_iptc_iterator *it)
{
  delete it;
}

MINIEXIV_EXPORT miniexiv_access_mode miniexiv_image_check_metadata_mode(
    const miniexiv_image *image,
    miniexiv_metadata_id metadata
)
{
    if (!image) {
        set_last_error(ErrorMessages::IMAGE_NULL);
        return MINIEXIV_ACCESS_ERROR;
    }

    try {

        Exiv2::MetadataId id;

        switch (metadata) {

        case MINIEXIV_METADATA_NONE:
            id = Exiv2::MetadataId::mdNone;
            break;

        case MINIEXIV_METADATA_EXIF:
            id = Exiv2::MetadataId::mdExif;
            break;

        case MINIEXIV_METADATA_IPTC:
            id = Exiv2::MetadataId::mdIptc;
            break;

        case MINIEXIV_METADATA_COMMENT:
            id = Exiv2::MetadataId::mdComment;
            break;

        case MINIEXIV_METADATA_XMP:
            id = Exiv2::MetadataId::mdXmp;
            break;

        case MINIEXIV_METADATA_ICC_PROFILE:
            id = Exiv2::MetadataId::mdIccProfile;
            break;

        default:
            set_last_error(ErrorMessages::INVALID_METADATA_ID);
            return MINIEXIV_ACCESS_ERROR;
        }


        auto result = image->exiv2_image->checkMode(id);

        return static_cast<miniexiv_access_mode>(result);


    } catch (const Exiv2::Error &e) {

        set_last_error(e.what());
        return MINIEXIV_ACCESS_ERROR;

    } catch (const std::exception &e) {

        set_last_error(e.what());
        return MINIEXIV_ACCESS_ERROR;

    } catch (...) {

        set_last_error(ErrorMessages::UNKNOWN_EX_ERROR);
        return MINIEXIV_ACCESS_ERROR;
    }
}