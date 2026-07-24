    /*
     * Comprehensive MiniExiv public API tests.
     *
     * The test executable expects test.jpg in its current working directory.
     * Destructive operations are performed only on temporary copies.
     */

    #include "miniexiv/miniexiv.h"
    #include <cmath>
    #include <cstdint>
    #include <cstdlib>
    #include <filesystem>
    #include <fstream>
    #include <iostream>
    #include <string>
    #include <vector>

    namespace fs = std::filesystem;

    namespace {

    constexpr const char *kTestImage = "test.jpg";
    constexpr const char *kExifKey = "Exif.Image.Artist";
    constexpr const char *kExifValue = "MiniExiv EXIF test";
    constexpr const char *kXmpKey = "Xmp.dc.description";
    constexpr const char *kXmpValue = "MiniExiv XMP test";
    constexpr const char *kIptcKey = "Iptc.Application2.Caption";
    constexpr const char *kIptcValue = "MiniExiv IPTC test";
    constexpr const char *kMissingExifKey = "Exif.Image.Software";
    constexpr const char *kMissingXmpKey = "Xmp.dc.source";
    constexpr const char *kMissingIptcKey = "Iptc.Application2.Credit";

    int g_checks = 0;
    int g_failures = 0;

    void report_failure(const char *expression, const char *file, int line,
                        const std::string &details = {}) {
      ++g_failures;
      std::cerr << "[FAIL] " << file << ':' << line << ": " << expression;
      if (!details.empty()) {
        std::cerr << " (" << details << ')';
      }
      std::cerr << '\n';
    }

    #define CHECK(expr)                                                            \
      do {                                                                         \
        ++g_checks;                                                                \
        if (!(expr)) {                                                             \
          report_failure(#expr, __FILE__, __LINE__);                               \
        }                                                                          \
      } while (false)

    #define CHECK_EQ(actual, expected)                                             \
      do {                                                                         \
        ++g_checks;                                                                \
        const auto actual_value = (actual);                                        \
        const auto expected_value = (expected);                                    \
        if (!(actual_value == expected_value)) {                                   \
          report_failure(#actual " == " #expected, __FILE__, __LINE__,            \
                         "actual=" + std::to_string(static_cast<long long>(        \
                                         actual_value)) +                          \
                             ", expected=" +                                      \
                             std::to_string(                                       \
                                 static_cast<long long>(expected_value)));         \
        }                                                                          \
      } while (false)

    #define CHECK_NE(actual, expected)                                             \
      do {                                                                         \
        ++g_checks;                                                                \
        if ((actual) == (expected)) {                                              \
          report_failure(#actual " != " #expected, __FILE__, __LINE__);           \
        }                                                                          \
      } while (false)

    void check_error_is_set() {
      const char *error = miniexiv_get_last_error();
      CHECK(error != nullptr);
      if (error != nullptr) {
        CHECK(error[0] != '\0');
      }
    }

    std::vector<uint8_t> read_file(const fs::path &path) {
      std::ifstream input(path, std::ios::binary | std::ios::ate);
      if (!input) {
        return {};
      }

      const auto end = input.tellg();
      if (end <= 0) {
        return {};
      }

      std::vector<uint8_t> data(static_cast<size_t>(end));
      input.seekg(0, std::ios::beg);
      input.read(reinterpret_cast<char *>(data.data()),
                 static_cast<std::streamsize>(data.size()));
      if (!input) {
        return {};
      }
      return data;
    }

    fs::path make_temp_path(const std::string &name) {
      const fs::path dir = fs::temp_directory_path() / "miniexiv-tests";
      std::error_code ec;
      fs::create_directories(dir, ec);
      return dir / name;
    }

    bool copy_test_image(const fs::path &destination) {
      std::error_code ec;
      fs::remove(destination, ec);
      ec.clear();
      return fs::copy_file(kTestImage, destination,
                           fs::copy_options::overwrite_existing, ec) &&
             !ec;
    }

    void test_version_and_error_api() {
      std::cout << "[TEST] version and error API\n";

      const char *version = miniexiv_version();
      CHECK(version != nullptr);
      if (version != nullptr) {
        CHECK(version[0] != '\0');
      }
      CHECK(miniexiv_version_major() >= 0);
      CHECK(miniexiv_version_minor() >= 0);
      CHECK(miniexiv_version_patch() >= 0);

      miniexiv_clear_last_error();
      const char *cleared = miniexiv_get_last_error();
      CHECK(cleared != nullptr);
    }

    void test_initialization() {
      std::cout << "[TEST] initialization reference counting\n";

      CHECK_EQ(miniexiv_initialize(), MINIEXIV_OK);
      CHECK_EQ(miniexiv_initialize(), MINIEXIV_OK);
      CHECK_EQ(miniexiv_shutdown(), MINIEXIV_OK);
      CHECK_EQ(miniexiv_shutdown(), MINIEXIV_OK);
      CHECK_EQ(miniexiv_shutdown(), MINIEXIV_OK);
      CHECK_EQ(miniexiv_initialize(), MINIEXIV_OK);
    }

    void test_open_file_invalid() {
      std::cout << "[TEST] open_file invalid arguments\n";

      CHECK(miniexiv_image_open_file(nullptr) == nullptr);
      check_error_is_set();

      CHECK(miniexiv_image_open_file("") == nullptr);
      check_error_is_set();

      CHECK(miniexiv_image_open_file("definitely-not-present.jpg") == nullptr);
      check_error_is_set();

      const fs::path invalid_file = make_temp_path("invalid-image.bin");
      {
        std::ofstream output(invalid_file, std::ios::binary | std::ios::trunc);
        output << "not an image";
      }
      CHECK(miniexiv_image_open_file(invalid_file.string().c_str()) == nullptr);
      check_error_is_set();
    }

    void test_open_buffer_invalid() {
      std::cout << "[TEST] open_buf invalid arguments\n";

      const uint8_t byte = 0;
      CHECK(miniexiv_image_open_buf(nullptr, 1) == nullptr);
      check_error_is_set();

      CHECK(miniexiv_image_open_buf(&byte, 0) == nullptr);
      check_error_is_set();

      const std::vector<uint8_t> garbage = {0x00, 0x11, 0x22, 0x33, 0x44};
      CHECK(miniexiv_image_open_buf(garbage.data(), garbage.size()) == nullptr);
      check_error_is_set();
    }

    void test_free_functions_with_null() {
      std::cout << "[TEST] free functions accept NULL\n";

      miniexiv_image_free(nullptr);
      miniexiv_free_string(nullptr);
      miniexiv_free_buffer(nullptr);
      miniexiv_exif_iterator_free(nullptr);
      miniexiv_xmp_iterator_free(nullptr);
      miniexiv_iptc_iterator_free(nullptr);
      CHECK(true);
    }

    void test_null_image_arguments() {
      std::cout << "[TEST] NULL image arguments\n";

      char *text = reinterpret_cast<char *>(static_cast<uintptr_t>(1));
      uint8_t *buffer = reinterpret_cast<uint8_t *>(static_cast<uintptr_t>(1));
      size_t size = 123;
      uint32_t width = 123;
      uint32_t height = 456;

      CHECK_EQ(miniexiv_image_clear_and_write(nullptr), MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_clear(nullptr), MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_clear_exif(nullptr), MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_clear_iptc(nullptr), MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_clear_icc(nullptr), MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_clear_xmp(nullptr), MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_clear_comment(nullptr), MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_get_comment(nullptr, &text), MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_set_comment(nullptr, "value"), MINIEXIV_ERROR);

      CHECK_EQ(miniexiv_image_set_exif_string(nullptr, kExifKey, kExifValue),
               MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_has_exif_key(nullptr, kExifKey), MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_get_exif_string(nullptr, kExifKey, &text),
               MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_remove_exif(nullptr, kExifKey), MINIEXIV_ERROR);

      CHECK_EQ(miniexiv_image_set_xmp_string(nullptr, kXmpKey, kXmpValue),
               MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_has_xmp_key(nullptr, kXmpKey), MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_get_xmp_string(nullptr, kXmpKey, &text),
               MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_remove_xmp(nullptr, kXmpKey), MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_export_xmp(nullptr, "out.xmp"), MINIEXIV_ERROR);

      CHECK_EQ(miniexiv_image_set_iptc_string(nullptr, kIptcKey, kIptcValue),
               MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_has_iptc_key(nullptr, kIptcKey), MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_get_iptc_string(nullptr, kIptcKey, &text),
               MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_remove_iptc(nullptr, kIptcKey), MINIEXIV_ERROR);

      CHECK_EQ(miniexiv_image_metadata_write(nullptr), MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_export_buf(nullptr, &buffer, &size), MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_get_dimensions(nullptr, &width, &height),
               MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_save_to_file(nullptr, "output.jpg"),
               MINIEXIV_ERROR);

      CHECK(miniexiv_exif_iterator_create(nullptr) == nullptr);
      CHECK(miniexiv_xmp_iterator_create(nullptr) == nullptr);
      CHECK(miniexiv_iptc_iterator_create(nullptr) == nullptr);

      CHECK_EQ(miniexiv_image_check_metadata_mode(nullptr, MINIEXIV_METADATA_EXIF),
               MINIEXIV_ACCESS_ERROR);
       double latitude = 123.0;
       double longitude = 456.0;
       
      CHECK_EQ(miniexiv_image_get_gps(nullptr, nullptr, nullptr),MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_get_gps(nullptr, &latitude, &longitude),MINIEXIV_ERROR);
    }

    void test_invalid_arguments_on_valid_image(miniexiv_image *image) {
      std::cout << "[TEST] invalid keys, values, and output pointers\n";

      CHECK(image != nullptr);
      if (image == nullptr) {
        return;
      }

      char *text = reinterpret_cast<char *>(static_cast<uintptr_t>(1));
      uint8_t *buffer = reinterpret_cast<uint8_t *>(static_cast<uintptr_t>(1));
      size_t size = 99;
      uint32_t width = 1;
      uint32_t height = 1;

      CHECK_EQ(miniexiv_image_get_comment(image, nullptr), MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_set_comment(image, nullptr), MINIEXIV_ERROR);

      CHECK_EQ(miniexiv_image_set_exif_string(image, nullptr, "value"),
               MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_set_exif_string(image, "", "value"),
               MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_set_exif_string(image, kExifKey, nullptr),
               MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_set_exif_string(image, "invalid.exif.key", "value"),
               MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_has_exif_key(image, nullptr), MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_has_exif_key(image, ""), MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_get_exif_string(image, nullptr, &text),
               MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_get_exif_string(image, "", &text), MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_get_exif_string(image, kExifKey, nullptr),
               MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_remove_exif(image, nullptr), MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_remove_exif(image, ""), MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_remove_exif(image, "invalid.exif.key"),
               MINIEXIV_ERROR);

      CHECK_EQ(miniexiv_image_set_xmp_string(image, nullptr, "value"),
               MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_set_xmp_string(image, "", "value"),
               MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_set_xmp_string(image, kXmpKey, nullptr),
               MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_set_xmp_string(image, "invalid.xmp.key", "value"),
               MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_has_xmp_key(image, nullptr), MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_has_xmp_key(image, ""), MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_get_xmp_string(image, nullptr, &text),
               MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_get_xmp_string(image, "", &text), MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_get_xmp_string(image, kXmpKey, nullptr),
               MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_remove_xmp(image, nullptr), MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_remove_xmp(image, ""), MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_remove_xmp(image, "invalid.xmp.key"),
               MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_export_xmp(image, nullptr), MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_export_xmp(image, ""), MINIEXIV_ERROR);

      CHECK_EQ(miniexiv_image_set_iptc_string(image, nullptr, "value"),
               MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_set_iptc_string(image, "", "value"),
               MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_set_iptc_string(image, kIptcKey, nullptr),
               MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_set_iptc_string(image, "invalid.iptc.key", "value"),
               MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_has_iptc_key(image, nullptr), MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_has_iptc_key(image, ""), MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_get_iptc_string(image, nullptr, &text),
               MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_get_iptc_string(image, "", &text), MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_get_iptc_string(image, kIptcKey, nullptr),
               MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_remove_iptc(image, nullptr), MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_remove_iptc(image, ""), MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_remove_iptc(image, "invalid.iptc.key"),
               MINIEXIV_ERROR);

      CHECK_EQ(miniexiv_image_export_buf(image, nullptr, &size), MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_export_buf(image, &buffer, nullptr), MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_get_dimensions(image, nullptr, &height),
               MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_get_dimensions(image, &width, nullptr),
               MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_save_to_file(image, nullptr), MINIEXIV_ERROR);
      CHECK_EQ(miniexiv_image_save_to_file(image, ""), MINIEXIV_ERROR);

      CHECK_EQ(miniexiv_image_check_metadata_mode(
                   image, static_cast<miniexiv_metadata_id>(9999)),
               MINIEXIV_ACCESS_ERROR);
      double latitude = 0.0;
double longitude = 0.0;
      //CHECK_EQ(miniexiv_image_get_gps(image, &latitude, &longitude), MINIEXIV_OK);
      //CHECK_EQ(miniexiv_image_get_gps(image, nullptr, &longitude), MINIEXIV_ERROR);
      //CHECK_EQ(miniexiv_image_get_gps(image, &latitude, nullptr), MINIEXIV_ERROR);
      //CHECK_EQ(miniexiv_image_get_gps(image, nullptr, nullptr), MINIEXIV_ERROR);
    }

    void test_comment(miniexiv_image *image) {
      std::cout << "[TEST] comment API\n";

      constexpr const char *comment = "MiniExiv comment test";
      CHECK_EQ(miniexiv_image_set_comment(image, comment), MINIEXIV_OK);

      char *value = nullptr;
      CHECK_EQ(miniexiv_image_get_comment(image, &value), MINIEXIV_OK);
      CHECK(value != nullptr);
      if (value != nullptr) {
        CHECK(std::string(value) == comment);
        miniexiv_free_string(value);
      }

      CHECK_EQ(miniexiv_image_clear_comment(image), MINIEXIV_OK);
      value = nullptr;
      CHECK_EQ(miniexiv_image_get_comment(image, &value), MINIEXIV_OK);
      CHECK(value != nullptr);
      if (value != nullptr) {
        CHECK(std::string(value).empty());
        miniexiv_free_string(value);
      }
    }

   void test_exif(miniexiv_image *image) {
  std::cout << "[TEST] EXIF API\n";

  CHECK_EQ(
      miniexiv_image_set_exif_string(
          image,
          kExifKey,
          kExifValue
      ),
      MINIEXIV_OK
  );

  CHECK_EQ(
      miniexiv_image_has_exif_key(image, kExifKey),
      MINIEXIV_TRUE
  );

  char *value = nullptr;

  CHECK_EQ(
      miniexiv_image_get_exif_string(
          image,
          kExifKey,
          &value
      ),
      MINIEXIV_OK
  );

  CHECK(value != nullptr);

  if (value != nullptr) {
    CHECK(std::string(value) == kExifValue);
    miniexiv_free_string(value);
    value = nullptr;
  }

  /*
   * The source image may already contain Exif.Image.Software.
   * Remove it first so this is a deterministic missing-key test.
   */
  CHECK_EQ(
      miniexiv_image_remove_exif(
          image,
          kMissingExifKey
      ),
      MINIEXIV_OK
  );

  CHECK_EQ(
      miniexiv_image_has_exif_key(
          image,
          kMissingExifKey
      ),
      MINIEXIV_FALSE
  );

  CHECK_EQ(
      miniexiv_image_get_exif_string(
          image,
          kMissingExifKey,
          &value
      ),
      MINIEXIV_ERROR
  );

  CHECK(value == nullptr);

  CHECK_EQ(
      miniexiv_image_remove_exif(image, kExifKey),
      MINIEXIV_OK
  );

  CHECK_EQ(
      miniexiv_image_has_exif_key(image, kExifKey),
      MINIEXIV_FALSE
  );

  CHECK_EQ(
      miniexiv_image_remove_exif(image, kExifKey),
      MINIEXIV_OK
  );

  CHECK_EQ(
      miniexiv_image_set_exif_string(
          image,
          kExifKey,
          kExifValue
      ),
      MINIEXIV_OK
  );
}

    void test_xmp(miniexiv_image *image) {
      std::cout << "[TEST] XMP API\n";

      CHECK_EQ(miniexiv_image_set_xmp_string(image, kXmpKey, kXmpValue),
               MINIEXIV_OK);
      CHECK_EQ(miniexiv_image_has_xmp_key(image, kXmpKey), MINIEXIV_TRUE);

      char *value = nullptr;
      CHECK_EQ(miniexiv_image_get_xmp_string(image, kXmpKey, &value), MINIEXIV_OK);
      CHECK(value != nullptr);
      if (value != nullptr) {
        CHECK(std::string(value).find(kXmpValue) != std::string::npos);
        miniexiv_free_string(value);
      }

      value = nullptr;
      CHECK_EQ(miniexiv_image_get_xmp_string(image, kMissingXmpKey, &value),
               MINIEXIV_ERROR);
      CHECK(value == nullptr);

      CHECK_EQ(miniexiv_image_remove_xmp(image, kXmpKey), MINIEXIV_OK);
      CHECK_EQ(miniexiv_image_has_xmp_key(image, kXmpKey), MINIEXIV_FALSE);
      CHECK_EQ(miniexiv_image_remove_xmp(image, kXmpKey), MINIEXIV_OK);

      CHECK_EQ(miniexiv_image_set_xmp_string(image, kXmpKey, kXmpValue),
               MINIEXIV_OK);
    }

    void test_iptc(miniexiv_image *image) {
      std::cout << "[TEST] IPTC API\n";

      CHECK_EQ(miniexiv_image_set_iptc_string(image, kIptcKey, kIptcValue),
               MINIEXIV_OK);
      CHECK_EQ(miniexiv_image_has_iptc_key(image, kIptcKey), MINIEXIV_TRUE);

      char *value = nullptr;
      CHECK_EQ(miniexiv_image_get_iptc_string(image, kIptcKey, &value),
               MINIEXIV_OK);
      CHECK(value != nullptr);
      if (value != nullptr) {
        CHECK(std::string(value) == kIptcValue);
        miniexiv_free_string(value);
      }

      value = nullptr;
      CHECK_EQ(miniexiv_image_get_iptc_string(image, kMissingIptcKey, &value),
               MINIEXIV_ERROR);
      CHECK(value == nullptr);

      CHECK_EQ(miniexiv_image_remove_iptc(image, kIptcKey), MINIEXIV_OK);
      CHECK_EQ(miniexiv_image_has_iptc_key(image, kIptcKey), MINIEXIV_FALSE);
      CHECK_EQ(miniexiv_image_remove_iptc(image, kIptcKey), MINIEXIV_OK);

      CHECK_EQ(miniexiv_image_set_iptc_string(image, kIptcKey, kIptcValue),
               MINIEXIV_OK);
    }

    template <typename Iterator, typename Create, typename Next, typename GetKey,
              typename GetValue, typename Free>
    void test_iterator(const char *name, miniexiv_image *image, Create create,
                       Next next, GetKey get_key, GetValue get_value, Free free_it) {
      std::cout << "[TEST] " << name << " iterator\n";

      CHECK_EQ(next(nullptr), MINIEXIV_ERROR);
      CHECK(get_key(nullptr) == nullptr);
      CHECK(get_value(nullptr) == nullptr);

      Iterator *it = create(image);
      CHECK(it != nullptr);
      if (it == nullptr) {
        return;
      }

      CHECK(get_key(it) == nullptr);
      CHECK(get_value(it) == nullptr);

      int entries = 0;
      for (;;) {
        const int result = next(it);
        if (result == MINIEXIV_ITERATOR_END) {
          break;
        }
        CHECK_EQ(result, MINIEXIV_ITERATOR_HAS_NEXT);
        if (result != MINIEXIV_ITERATOR_HAS_NEXT) {
          break;
        }

        const char *key = get_key(it);
        const char *value = get_value(it);
        CHECK(key != nullptr);
        CHECK(value != nullptr);
        if (key != nullptr) {
          CHECK(key[0] != '\0');
        }
        ++entries;
      }

      CHECK(next(it) == MINIEXIV_ITERATOR_END);
      CHECK(get_key(it) == nullptr);
      CHECK(get_value(it) == nullptr);
      CHECK(entries >= 1);
      free_it(it);
    }

    void test_iterators(miniexiv_image *image) {
      test_iterator<miniexiv_exif_iterator>(
          "EXIF", image, miniexiv_exif_iterator_create,
          miniexiv_exif_iterator_next, miniexiv_exif_iterator_get_key,
          miniexiv_exif_iterator_get_value, miniexiv_exif_iterator_free);

      test_iterator<miniexiv_xmp_iterator>(
          "XMP", image, miniexiv_xmp_iterator_create, miniexiv_xmp_iterator_next,
          miniexiv_xmp_iterator_get_key, miniexiv_xmp_iterator_get_value,
          miniexiv_xmp_iterator_free);

      test_iterator<miniexiv_iptc_iterator>(
          "IPTC", image, miniexiv_iptc_iterator_create,
          miniexiv_iptc_iterator_next, miniexiv_iptc_iterator_get_key,
          miniexiv_iptc_iterator_get_value, miniexiv_iptc_iterator_free);
    }

    void test_dimensions_and_modes(miniexiv_image *image) {
      std::cout << "[TEST] dimensions and metadata access modes\n";

      uint32_t width = 0;
      uint32_t height = 0;
      CHECK_EQ(miniexiv_image_get_dimensions(image, &width, &height), MINIEXIV_OK);
      CHECK(width > 0);
      CHECK(height > 0);

      const miniexiv_metadata_id ids[] = {
          MINIEXIV_METADATA_NONE,        MINIEXIV_METADATA_EXIF,
          MINIEXIV_METADATA_IPTC,        MINIEXIV_METADATA_COMMENT,
          MINIEXIV_METADATA_XMP,         MINIEXIV_METADATA_ICC_PROFILE,
      };

      for (const auto id : ids) {
        const auto mode = miniexiv_image_check_metadata_mode(image, id);
        CHECK(mode == MINIEXIV_ACCESS_NONE || mode == MINIEXIV_ACCESS_READ ||
              mode == MINIEXIV_ACCESS_WRITE ||
              mode == MINIEXIV_ACCESS_READWRITE);
      }
    }

    void test_buffer_export_and_reopen(miniexiv_image *image) {
      std::cout << "[TEST] metadata write, buffer export, and buffer reopen\n";

      CHECK_EQ(miniexiv_image_metadata_write(image), MINIEXIV_OK);

      uint8_t *buffer = nullptr;
      size_t size = 0;
      CHECK_EQ(miniexiv_image_export_buf(image, &buffer, &size), MINIEXIV_OK);
      CHECK(buffer != nullptr);
      CHECK(size > 0);

      if (buffer != nullptr && size > 0) {
        miniexiv_image *reopened = miniexiv_image_open_buf(buffer, size);
        CHECK(reopened != nullptr);
        if (reopened != nullptr) {
          CHECK_EQ(miniexiv_image_has_exif_key(reopened, kExifKey), MINIEXIV_TRUE);
          CHECK_EQ(miniexiv_image_has_xmp_key(reopened, kXmpKey), MINIEXIV_TRUE);
          CHECK_EQ(miniexiv_image_has_iptc_key(reopened, kIptcKey), MINIEXIV_TRUE);
          miniexiv_image_free(reopened);
        }
      }

      miniexiv_free_buffer(buffer);
    }

    void test_xmp_export(miniexiv_image *image) {
      std::cout << "[TEST] XMP packet export\n";

      const fs::path output = make_temp_path("metadata.xmp");
      std::error_code ec;
      fs::remove(output, ec);

      CHECK_EQ(miniexiv_image_export_xmp(image, output.string().c_str()),
               MINIEXIV_OK);
      CHECK(fs::exists(output));
      if (fs::exists(output)) {
        CHECK(fs::file_size(output) > 0);
      }

      const fs::path missing_dir =
          make_temp_path("missing-directory") / "metadata.xmp";
      fs::remove_all(missing_dir.parent_path(), ec);
      CHECK_EQ(miniexiv_image_export_xmp(image, missing_dir.string().c_str()),
               MINIEXIV_ERROR);
    }

    void test_save_to_file(miniexiv_image *image) {
      std::cout << "[TEST] save_to_file\n";

      const fs::path output = make_temp_path("saved-image.jpg");
      std::error_code ec;
      fs::remove(output, ec);

      CHECK_EQ(miniexiv_image_save_to_file(image, output.string().c_str()),
               MINIEXIV_OK);
      CHECK(fs::exists(output));
      if (fs::exists(output)) {
        CHECK(fs::file_size(output) > 0);
        miniexiv_image *saved = miniexiv_image_open_file(output.string().c_str());
        CHECK(saved != nullptr);
        miniexiv_image_free(saved);
      }

      const fs::path missing_dir = make_temp_path("missing-save-dir") / "out.jpg";
      fs::remove_all(missing_dir.parent_path(), ec);
      CHECK_EQ(miniexiv_image_save_to_file(image, missing_dir.string().c_str()),
               MINIEXIV_ERROR);
    }

    void test_individual_clear_functions() {
      std::cout << "[TEST] individual metadata clear functions\n";

      const auto data = read_file(kTestImage);
      CHECK(!data.empty());
      if (data.empty()) {
        return;
      }

      miniexiv_image *image = miniexiv_image_open_buf(data.data(), data.size());
      CHECK(image != nullptr);
      if (image == nullptr) {
        return;
      }

      CHECK_EQ(miniexiv_image_set_exif_string(image, kExifKey, kExifValue),
               MINIEXIV_OK);
      CHECK_EQ(miniexiv_image_set_xmp_string(image, kXmpKey, kXmpValue),
               MINIEXIV_OK);
      CHECK_EQ(miniexiv_image_set_iptc_string(image, kIptcKey, kIptcValue),
               MINIEXIV_OK);
      CHECK_EQ(miniexiv_image_set_comment(image, "comment"), MINIEXIV_OK);

      CHECK_EQ(miniexiv_image_clear_exif(image), MINIEXIV_OK);
      CHECK_EQ(miniexiv_image_has_exif_key(image, kExifKey), MINIEXIV_FALSE);

      CHECK_EQ(miniexiv_image_clear_xmp(image), MINIEXIV_OK);
      CHECK_EQ(miniexiv_image_has_xmp_key(image, kXmpKey), MINIEXIV_FALSE);

      CHECK_EQ(miniexiv_image_clear_iptc(image), MINIEXIV_OK);
      CHECK_EQ(miniexiv_image_has_iptc_key(image, kIptcKey), MINIEXIV_FALSE);

      CHECK_EQ(miniexiv_image_clear_comment(image), MINIEXIV_OK);
      char *comment = nullptr;
      CHECK_EQ(miniexiv_image_get_comment(image, &comment), MINIEXIV_OK);
      CHECK(comment != nullptr);
      if (comment != nullptr) {
        CHECK(std::string(comment).empty());
        miniexiv_free_string(comment);
      }

      // The source image may or may not contain an ICC profile. Clearing it must
      // still be a successful no-op when no profile exists.
      CHECK_EQ(miniexiv_image_clear_icc(image), MINIEXIV_OK);

      miniexiv_image_free(image);
    }

    void test_clear_all_in_memory() {
      std::cout << "[TEST] clear all metadata in memory\n";

      const auto data = read_file(kTestImage);
      miniexiv_image *image =
          data.empty() ? nullptr : miniexiv_image_open_buf(data.data(), data.size());
      CHECK(image != nullptr);
      if (image == nullptr) {
        return;
      }

      CHECK_EQ(miniexiv_image_set_exif_string(image, kExifKey, kExifValue),
               MINIEXIV_OK);
      CHECK_EQ(miniexiv_image_set_xmp_string(image, kXmpKey, kXmpValue),
               MINIEXIV_OK);
      CHECK_EQ(miniexiv_image_set_iptc_string(image, kIptcKey, kIptcValue),
               MINIEXIV_OK);
      CHECK_EQ(miniexiv_image_set_comment(image, "comment"), MINIEXIV_OK);

      CHECK_EQ(miniexiv_image_clear(image), MINIEXIV_OK);
      CHECK_EQ(miniexiv_image_has_exif_key(image, kExifKey), MINIEXIV_FALSE);
      CHECK_EQ(miniexiv_image_has_xmp_key(image, kXmpKey), MINIEXIV_FALSE);
      CHECK_EQ(miniexiv_image_has_iptc_key(image, kIptcKey), MINIEXIV_FALSE);

      char *comment = nullptr;
      CHECK_EQ(miniexiv_image_get_comment(image, &comment), MINIEXIV_OK);
      if (comment != nullptr) {
        CHECK(std::string(comment).empty());
        miniexiv_free_string(comment);
      }

      miniexiv_image_free(image);
    }

    void test_clear_and_write_file_copy() {
      std::cout << "[TEST] clear_and_write on a temporary file copy\n";

      const fs::path copy = make_temp_path("clear-and-write.jpg");
      CHECK(copy_test_image(copy));
      if (!fs::exists(copy)) {
        return;
      }

      miniexiv_image *image = miniexiv_image_open_file(copy.string().c_str());
      CHECK(image != nullptr);
      if (image == nullptr) {
        return;
      }

      CHECK_EQ(miniexiv_image_set_exif_string(image, kExifKey, kExifValue),
               MINIEXIV_OK);
      CHECK_EQ(miniexiv_image_set_xmp_string(image, kXmpKey, kXmpValue),
               MINIEXIV_OK);
      CHECK_EQ(miniexiv_image_set_iptc_string(image, kIptcKey, kIptcValue),
               MINIEXIV_OK);
      CHECK_EQ(miniexiv_image_set_comment(image, "comment"), MINIEXIV_OK);
      CHECK_EQ(miniexiv_image_metadata_write(image), MINIEXIV_OK);
      CHECK_EQ(miniexiv_image_clear_and_write(image), MINIEXIV_OK);
      miniexiv_image_free(image);

      image = miniexiv_image_open_file(copy.string().c_str());
      CHECK(image != nullptr);
      if (image != nullptr) {
        CHECK_EQ(miniexiv_image_has_exif_key(image, kExifKey), MINIEXIV_FALSE);
        CHECK_EQ(miniexiv_image_has_xmp_key(image, kXmpKey), MINIEXIV_FALSE);
        CHECK_EQ(miniexiv_image_has_iptc_key(image, kIptcKey), MINIEXIV_FALSE);
        miniexiv_image_free(image);
      }
    }
    void test_gps(miniexiv_image *image) {
  std::cout << "[TEST] GPS API\n";

  CHECK(image != nullptr);
  if (image == nullptr) {
    return;
  }

  constexpr const char *latitude_key =
      "Exif.GPSInfo.GPSLatitude";

  constexpr const char *latitude_ref_key =
      "Exif.GPSInfo.GPSLatitudeRef";

  constexpr const char *longitude_key =
      "Exif.GPSInfo.GPSLongitude";

  constexpr const char *longitude_ref_key =
      "Exif.GPSInfo.GPSLongitudeRef";

  constexpr double expected_latitude =
      42.0 + 10.0 / 60.0 + 25.12 / 3600.0;

  constexpr double expected_longitude =
      42.0 + 56.0 / 60.0 + 10.74 / 3600.0;

  constexpr double tolerance = 0.0000001;

  /*
   * Remove coordinates that may already exist in test.jpg,
   * making the missing-GPS test deterministic.
   */
  CHECK_EQ(
      miniexiv_image_remove_exif(image, latitude_key),
      MINIEXIV_OK
  );

  CHECK_EQ(
      miniexiv_image_remove_exif(image, latitude_ref_key),
      MINIEXIV_OK
  );

  CHECK_EQ(
      miniexiv_image_remove_exif(image, longitude_key),
      MINIEXIV_OK
  );

  CHECK_EQ(
      miniexiv_image_remove_exif(image, longitude_ref_key),
      MINIEXIV_OK
  );

  double latitude = 123.0;
  double longitude = 456.0;

  CHECK_EQ(
      miniexiv_image_get_gps(image, &latitude, &longitude),
      MINIEXIV_ERROR
  );

  CHECK(latitude == 0.0);
  CHECK(longitude == 0.0);

  /*
   * Write the coordinates:
   *
   * 42° 10' 25.12" N
   * 42° 56' 10.74" E
   */
  CHECK_EQ(
      miniexiv_image_set_exif_string(
          image,
          latitude_key,
          "42/1 10/1 2512/100"
      ),
      MINIEXIV_OK
  );

  CHECK_EQ(
      miniexiv_image_set_exif_string(
          image,
          latitude_ref_key,
          "N"
      ),
      MINIEXIV_OK
  );

  CHECK_EQ(
      miniexiv_image_set_exif_string(
          image,
          longitude_key,
          "42/1 56/1 1074/100"
      ),
      MINIEXIV_OK
  );

  CHECK_EQ(
      miniexiv_image_set_exif_string(
          image,
          longitude_ref_key,
          "E"
      ),
      MINIEXIV_OK
  );

  latitude = 0.0;
  longitude = 0.0;

  CHECK_EQ(
      miniexiv_image_get_gps(image, &latitude, &longitude),
      MINIEXIV_OK
  );

  CHECK(
      std::fabs(latitude - expected_latitude) < tolerance
  );

  CHECK(
      std::fabs(longitude - expected_longitude) < tolerance
  );

  /*
   * Verify southern and western coordinates.
   */
  CHECK_EQ(
      miniexiv_image_set_exif_string(
          image,
          latitude_ref_key,
          "S"
      ),
      MINIEXIV_OK
  );

  CHECK_EQ(
      miniexiv_image_set_exif_string(
          image,
          longitude_ref_key,
          "W"
      ),
      MINIEXIV_OK
  );

  CHECK_EQ(
      miniexiv_image_get_gps(image, &latitude, &longitude),
      MINIEXIV_OK
  );

  CHECK(
      std::fabs(latitude + expected_latitude) < tolerance
  );

  CHECK(
      std::fabs(longitude + expected_longitude) < tolerance
  );

  /*
   * Invalid latitude reference.
   */
  CHECK_EQ(
      miniexiv_image_set_exif_string(
          image,
          latitude_ref_key,
          "X"
      ),
      MINIEXIV_OK
  );

  latitude = 123.0;
  longitude = 456.0;

  CHECK_EQ(
      miniexiv_image_get_gps(image, &latitude, &longitude),
      MINIEXIV_ERROR
  );

  /*
   * Outputs remain zero because get_gps() clears them
   * before parsing the metadata.
   */
  CHECK(latitude == 0.0);
  CHECK(longitude == 0.0);

  /*
   * Restore valid values so subsequent EXIF and iterator tests
   * see valid metadata.
   */
  CHECK_EQ(
      miniexiv_image_set_exif_string(
          image,
          latitude_ref_key,
          "N"
      ),
      MINIEXIV_OK
  );

  CHECK_EQ(
      miniexiv_image_set_exif_string(
          image,
          longitude_ref_key,
          "E"
      ),
      MINIEXIV_OK
  );
}
    } // namespace

    int main() {
      std::cout << "MiniExiv comprehensive API test\n";
      std::cout << "Input image: " << fs::absolute(kTestImage) << "\n\n";

      if (!fs::exists(kTestImage)) {
        std::cerr << "[FATAL] test.jpg was not found in the current working "
                     "directory.\n";
        return EXIT_FAILURE;
      }

      test_version_and_error_api();
      test_initialization();
      test_open_file_invalid();
      test_open_buffer_invalid();
      test_free_functions_with_null();
      test_null_image_arguments();
      
      const auto input_data = read_file(kTestImage);
      CHECK(!input_data.empty());

      miniexiv_image *image = input_data.empty()
                                  ? nullptr
                                  : miniexiv_image_open_buf(input_data.data(),
                                                            input_data.size());
      CHECK(image != nullptr);

      if (image != nullptr) {
        test_invalid_arguments_on_valid_image(image);
        test_comment(image);
        test_exif(image);
        test_xmp(image);
        test_iptc(image);
        test_dimensions_and_modes(image);
        test_iterators(image);
        test_buffer_export_and_reopen(image);
        test_xmp_export(image);
        test_save_to_file(image);
        test_gps(image);
        miniexiv_image_free(image);
      }

      test_individual_clear_functions();
      test_clear_all_in_memory();
      test_clear_and_write_file_copy();

      CHECK_EQ(miniexiv_shutdown(), MINIEXIV_OK);
      CHECK_EQ(miniexiv_shutdown(), MINIEXIV_OK);

      std::cout << "\nChecks: " << g_checks << "\n";
      std::cout << "Failures: " << g_failures << "\n";

      if (g_failures == 0) {
        std::cout << "[PASS] All MiniExiv tests passed.\n";
        return EXIT_SUCCESS;
      }

      std::cerr << "[FAIL] MiniExiv test suite failed.\n";
      return EXIT_FAILURE;
    }
