/**
 * @mainpage MiniExiv
 *
 * MiniExiv is a lightweight C wrapper around Exiv2.
 *
 * It provides a simple C API for reading and modifying
 * image metadata.
 *
 * Supported metadata:
 *
 * - EXIF
 * - IPTC
 * - XMP
 * - Image comments
 * - ICC profiles
 *
 * Images can be opened in two modes:
 *
 * File mode:
 *
 * @code
 * miniexiv_image_open_file()
 * @endcode
 *
 * Changes are written directly to the original file
 * after calling miniexiv_image_metadata_write().
 *
 *
 * Buffer mode:
 *
 * @code
 * miniexiv_image_open_buf()
 * @endcode
 *
 * MiniExiv creates an internal copy of the image data.
 * The original memory buffer is never modified.
 *
 * After changes are applied, use
 * miniexiv_image_export_buf()
 * to retrieve the updated image.
 *
 */
#ifndef MINIEXIV_H
#define MINIEXIV_H

#include <miniexiv/miniexiv_export.h>
#include <stddef.h>
#include <stdint.h>
#define MINIEXIV_OK 0
#define MINIEXIV_ERROR -1

#define MINIEXIV_TRUE 1
#define MINIEXIV_FALSE 0

#define MINIEXIV_ITERATOR_HAS_NEXT 1
#define MINIEXIV_ITERATOR_END 0
#ifdef __cplusplus
extern "C" {
#endif
typedef struct miniexiv_image miniexiv_image;
typedef struct miniexiv_exif_iterator miniexiv_exif_iterator;
typedef struct miniexiv_xmp_iterator miniexiv_xmp_iterator;
typedef struct miniexiv_iptc_iterator miniexiv_iptc_iterator;
/**
 * @brief Identifies a metadata type supported by MiniExiv.
 *
 * Used with functions that operate on a specific metadata category.
 *
 * @note Values are powers of two to allow future bitmask usage.
 */

typedef enum {
    /** No metadata type. */
    MINIEXIV_METADATA_NONE = 0,
      /** Exchangeable Image File Format metadata. */
    MINIEXIV_METADATA_EXIF = 1,
       /** International Press Telecommunications Council metadata. */
    MINIEXIV_METADATA_IPTC = 2,
     /** Image comment metadata. */
    MINIEXIV_METADATA_COMMENT = 4,
    /** Extensible Metadata Platform metadata. */
    MINIEXIV_METADATA_XMP = 8,
    /** Embedded ICC color profile. */
    MINIEXIV_METADATA_ICC_PROFILE = 16
} miniexiv_metadata_id;
/**
 * @brief Describes supported access permissions for metadata.
 *
 * Returned by miniexiv_image_check_metadata_mode().
 */
typedef enum {
      /** An error occurred. */
    MINIEXIV_ACCESS_ERROR = -1,
       /** Metadata is not available. */
    MINIEXIV_ACCESS_NONE = 0,
     /** Metadata can only be read. */
    MINIEXIV_ACCESS_READ = 1,
     /** Metadata can only be written. */
    MINIEXIV_ACCESS_WRITE = 2,
      /** Metadata can be read and written. */
    MINIEXIV_ACCESS_READWRITE = 3
} miniexiv_access_mode;
/**
 * @brief Initializes the MiniExiv library.
 *
 * This function must be called before using any other MiniExiv API functions.
 *
 * Initialization is reference counted. Each successful call to
 * miniexiv_initialize() must have a corresponding call to
 * miniexiv_shutdown().
 *
 * The function initializes internal Exiv2 components required for metadata
 * processing.
 *
 * @return MINIEXIV_OK if initialization succeeded.
 * @return MINIEXIV_ERROR if initialization failed.
 *
 * @note This function is thread-safe.
 *
 * @see miniexiv_shutdown()
 */
MINIEXIV_EXPORT int miniexiv_initialize(void);
/**
 * @brief Shuts down the MiniExiv library.
 *
 * Releases internal resources used by MiniExiv and Exiv2.
 *
 * This function is reference counted. The internal shutdown process is
 * performed only after the number of calls to miniexiv_initialize()
 * is equal to the number of calls to miniexiv_shutdown().
 *
 * It is safe to call this function when the library is already shut down.
 *
 * @return MINIEXIV_OK on success.
 *
 * @note This function is thread-safe.
 *
 * @warning All MiniExiv image objects and iterators should be released
 * before calling the final miniexiv_shutdown() that terminates the library.
 *
 * @see miniexiv_initialize()
 */
MINIEXIV_EXPORT int miniexiv_shutdown(void);
/**
 * @brief Returns the MiniExiv library version string.
 *
 * The returned string is owned by the library and must not be modified
 * or freed by the caller.
 *
 * The pointer remains valid for the lifetime of the library.
 *
 * @return Null-terminated version string.
 *
 * @note This function can be called before miniexiv_initialize().
 *
 * @see miniexiv_version_major()
 * @see miniexiv_version_minor()
 * @see miniexiv_version_patch()
 */
MINIEXIV_EXPORT const char *miniexiv_version(void);
/**
 * @brief Returns the major version number.
 *
 * @return Major version component.
 *
 * @see miniexiv_version()
 */
MINIEXIV_EXPORT int miniexiv_version_major(void);
/**
 * @brief Returns the minor version number.
 *
 * @return Minor version component.
 *
 * @see miniexiv_version()
 */
MINIEXIV_EXPORT int miniexiv_version_minor(void);
/**
 * @brief Returns the patch version number.
 *
 * @return Patch version component.
 *
 * @see miniexiv_version()
 */
MINIEXIV_EXPORT int miniexiv_version_patch(void);
/**
 * @brief Returns the last error message for the calling thread.
 *
 * The error state is stored separately for each thread.
 * Calling this function from different threads does not access
 * shared error data.
 *
 * @return Null-terminated error message string.
 *
 * @note Thread-safe.
 */
MINIEXIV_EXPORT const char *miniexiv_get_last_error(void);
/**
 * @brief Clears the last error message for the calling thread.
 *
 * This function only affects the error state of the current thread.
 *
 * @note Thread-safe.
 */
MINIEXIV_EXPORT void miniexiv_clear_last_error(void);
/**
 * @brief Opens an image file for metadata operations.
 *
 * Opens the specified image file and loads its metadata.
 *
 * The returned image object owns all resources associated with the
 * opened image and must be released using miniexiv_image_free().
 *
 * @param filename Path to the image file.
 *
 * @return Pointer to a miniexiv_image object on success.
 * @return NULL if the file cannot be opened or metadata cannot be read.
 *
 * @note Use miniexiv_get_last_error() to retrieve the error description
 * after a failed call.
 *
 * @see miniexiv_image_free()
 * @see miniexiv_get_last_error()
 */
MINIEXIV_EXPORT miniexiv_image *miniexiv_image_open_file(const char *filename);
/**
 * @brief Opens an image from a memory buffer.
 *
 * Creates an internal copy of the supplied image data and loads its metadata.
 * The input buffer is never modified and may be released by the caller after
 * this function returns.
 *
 * The returned image object must be released using miniexiv_image_free().
 * After modifying metadata, call miniexiv_image_metadata_write() and then
 * miniexiv_image_export_buf() to retrieve the updated image data.
 *
 * @param buf Pointer to the input image data.
 * @param size Size of the input buffer in bytes.
 *
 * @return Pointer to a miniexiv_image object on success.
 * @return NULL if the buffer is invalid or the image cannot be opened.
 *
 * @note Use miniexiv_get_last_error() to retrieve the error description after
 *       a failed call.
 *
 * @see miniexiv_image_metadata_write()
 * @see miniexiv_image_export_buf()
 * @see miniexiv_image_free()
 */
MINIEXIV_EXPORT miniexiv_image *miniexiv_image_open_buf(const uint8_t *buf,
                                                        size_t size);
/**
 * @brief Releases an image object.
 *
 * Frees all resources associated with a miniexiv_image object.
 *
 * The image object becomes invalid after this function returns and
 * must not be used again.
 *
 * @param ptr Pointer to the image object.
 *
 * @note It is safe to pass NULL.
 *
 * @see miniexiv_image_open_file()
 * @see miniexiv_image_open_buf()
 */
MINIEXIV_EXPORT void miniexiv_image_free(miniexiv_image *ptr);
/**
 * @brief Removes all metadata from the image and writes the changes.
 *
 * Clears all supported metadata from the image, including:
 * - EXIF metadata
 * - IPTC metadata
 * - XMP metadata
 * - Image comment
 *
 * After the metadata is cleared, the changes are written to the
 * underlying image.
 *
 * If the image was opened from a file using
 * miniexiv_image_open_file(), the file on disk is updated.
 *
 * If the image was opened from a memory buffer using
 * miniexiv_image_open_buf(), the modified image is written to Exiv2's
 * internal memory buffer. Call miniexiv_image_export_buf() to obtain
 * the updated image data.
 *
 * @param image Image handle.
 *
 * @return MINIEXIV_OK on success.
 * @return MINIEXIV_ERROR on failure.
 *
 * @see miniexiv_image_clear()
 * @see miniexiv_image_metadata_write()
 * @see miniexiv_image_export_buf()
 */
MINIEXIV_EXPORT int miniexiv_image_clear_and_write(miniexiv_image *image);
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
MINIEXIV_EXPORT int miniexiv_image_clear(miniexiv_image *image);
/**
 * @brief Removes all EXIF metadata from the image.
 *
 * Clears all EXIF metadata stored in the image. The changes are applied
 * only to the in-memory image object.
 *
 * To make the changes persistent, call
 * miniexiv_image_metadata_write().
 *
 * @param image Image handle.
 *
 * @return MINIEXIV_OK on success.
 * @return MINIEXIV_ERROR on failure.
 *
 * @see miniexiv_image_metadata_write()
 * @see miniexiv_image_clear()
 */
MINIEXIV_EXPORT int miniexiv_image_clear_exif(miniexiv_image *image);
/**
 * @brief Removes all IPTC metadata from the image.
 *
 * Clears all IPTC metadata stored in the image. The changes are applied
 * only to the in-memory image object.
 *
 * To make the changes persistent, call
 * miniexiv_image_metadata_write().
 *
 * @param image Image handle.
 *
 * @return MINIEXIV_OK on success.
 * @return MINIEXIV_ERROR on failure.
 *
 * @see miniexiv_image_metadata_write()
 * @see miniexiv_image_clear()
 */
MINIEXIV_EXPORT int miniexiv_image_clear_iptc(miniexiv_image *image);
/**
 * @brief Removes the ICC color profile from the image.
 *
 * Clears the embedded ICC color profile. The changes are applied only
 * to the in-memory image object.
 *
 * To make the changes persistent, call
 * miniexiv_image_metadata_write().
 *
 * @param image Image handle.
 *
 * @return MINIEXIV_OK on success.
 * @return MINIEXIV_ERROR on failure.
 *
 * @see miniexiv_image_metadata_write()
 * @see miniexiv_image_clear()
 */
MINIEXIV_EXPORT int miniexiv_image_clear_icc(miniexiv_image *image);
/**
 * @brief Removes all XMP metadata from the image.
 *
 * Clears all XMP metadata stored in the image. The changes are applied only
 * to the in-memory image object. Call miniexiv_image_metadata_write() to make
 * the changes persistent.
 *
 * @param image Image handle.
 *
 * @return MINIEXIV_OK on success.
 * @return MINIEXIV_ERROR on failure.
 *
 * @see miniexiv_image_metadata_write()
 * @see miniexiv_image_clear()
 */
MINIEXIV_EXPORT int miniexiv_image_clear_xmp(miniexiv_image *image);
/**
 * @brief Removes the image comment.
 *
 * Clears the comment stored in the image. The changes are applied only
 * to the in-memory image object.
 *
 * To make the changes persistent, call
 * miniexiv_image_metadata_write().
 *
 * @param image Image handle.
 *
 * @return MINIEXIV_OK on success.
 * @return MINIEXIV_ERROR on failure.
 *
 * @see miniexiv_image_metadata_write()
 * @see miniexiv_image_clear()
 */
MINIEXIV_EXPORT int miniexiv_image_clear_comment(miniexiv_image *image);
/**
 * @brief Gets the image comment.
 *
 * Retrieves the current image comment.
 *
 * The returned string is allocated by MiniExiv and must be released
 * using miniexiv_free_string().
 *
 * @param image Image handle.
 * @param out_value Pointer receiving the allocated string.
 *
 * @return MINIEXIV_OK on success.
 * @return MINIEXIV_ERROR on failure.
 *
 * @note On failure, @p out_value is set to NULL.
 *
 * @see miniexiv_free_string()
 */
MINIEXIV_EXPORT int miniexiv_image_get_comment(const miniexiv_image *image,
                                               char **out_value);
/**
 * @brief Sets the image comment.
 *
 * Replaces the current image comment.
 *
 * Changes are applied only to the in-memory image object.
 * Call miniexiv_image_metadata_write() to save changes.
 *
 * @param image Image handle.
 * @param value New comment text.
 *
 * @return MINIEXIV_OK on success.
 * @return MINIEXIV_ERROR on failure.
 *
 * @see miniexiv_image_metadata_write()
 */
MINIEXIV_EXPORT int miniexiv_image_set_comment(miniexiv_image *image,
                                               const char *value);
/**
 * @brief Sets the value of an EXIF metadata tag.
 *
 * Creates a new EXIF tag or updates the value of an existing one.
 * If the specified EXIF tag already exists, its current value is
 * replaced with the new value.
 *
 * The changes are applied only to the in-memory image object.
 *
 * To make the changes persistent, call
 * miniexiv_image_metadata_write().
 *
 * @param image Image handle.
 * @param key EXIF tag key (for example, "Exif.Image.Artist").
 * @param value String value to assign to the EXIF tag.
 *
 * @return MINIEXIV_OK on success.
 * @return MINIEXIV_ERROR on failure.
 *
 * @note The specified EXIF key must be supported by Exiv2.
 *
 * @see miniexiv_image_metadata_write()
 * @see miniexiv_image_remove_exif()
 * @see miniexiv_image_has_exif()
 */
MINIEXIV_EXPORT int miniexiv_image_set_exif_string(miniexiv_image *image,
                                                   const char *key,
                                                   const char *value);
/**
 * @brief Checks whether an EXIF tag exists.
 *
 * Determines whether the specified EXIF metadata tag is present in
 * the image.
 *
 * @param image Image handle.
 * @param key EXIF tag key (for example, "Exif.Image.Artist").
 *
 * @return MINIEXIV_TRUE if the EXIF tag exists.
 * @return MINIEXIV_FALSE if the EXIF tag does not exist.
 * @return MINIEXIV_ERROR if an error occurs.
 *
 * @see miniexiv_image_set_exif_string()
 * @see miniexiv_image_remove_exif()
 */
MINIEXIV_EXPORT int miniexiv_image_has_exif_key(const miniexiv_image *image,
                                                const char *key);
/**
 * @brief Gets the value of an EXIF metadata tag.
 *
 * Retrieves the string representation of the specified EXIF tag.
 *
 * The returned string is allocated by MiniExiv and must be released
 * by the caller using miniexiv_free_string().
 *
 * The caller must not release the returned string using free().
 *
 * @param image Image handle.
 * @param key EXIF tag key (for example, "Exif.Image.Model").
 * @param out_value Pointer receiving the allocated string.
 *
 * @return MINIEXIV_OK on success.
 * @return MINIEXIV_ERROR on failure.
 *
 * @note If the specified EXIF key does not exist, the function fails
 *       and sets the last error message.
 *
 * @note On failure, *out_value is set to NULL.
 *
 * @warning The returned string remains valid until it is released
 *          with miniexiv_free_string().
 *
 * @see miniexiv_free_string()
 * @see miniexiv_get_last_error()
 * @see miniexiv_image_has_exif_key()
 */
MINIEXIV_EXPORT int miniexiv_image_get_exif_string(const miniexiv_image *image,
                                                const char *key,
                                                char **out_value);
/**
 * @brief Removes an EXIF metadata tag.
 *
 * Removes the specified EXIF tag from the image if it exists. If the
 * tag is not present, the function succeeds without making any changes.
 * The changes are applied only to the in-memory image object.
 *
 * To make the changes persistent, call
 * miniexiv_image_metadata_write().
 *
 * @param image Image handle.
 * @param key EXIF tag key (for example, "Exif.Image.Artist").
 *
 * @return MINIEXIV_OK on success.
 * @return MINIEXIV_ERROR on failure.
 *
 * @note It is not considered an error if the specified EXIF tag does
 * not exist.
 *
 * @see miniexiv_image_metadata_write()
 * @see miniexiv_image_set_exif_string()
 * @see miniexiv_image_has_exif()
 */
MINIEXIV_EXPORT int miniexiv_image_remove_exif(miniexiv_image *image,
                                               const char *key);
/**
 * @brief Writes pending metadata changes to the image.
 *
 * Commits all metadata modifications previously made to the image
 * object, including EXIF, IPTC, XMP, ICC profile, and image comment
 * changes.
 *
 * If the image was opened with miniexiv_image_open_file(), the image
 * file on disk is updated.
 *
 * If the image was opened with miniexiv_image_open_buf(), the modified
 * image is written to Exiv2's internal memory buffer. Call
 * miniexiv_image_export_buf() to obtain the updated image data.
 * @warning
 * Calling miniexiv_image_metadata_write() after opening an image from
 * a file will modify the original file.
 * @param image Image handle.
 *
 * @return MINIEXIV_OK on success.
 * @return MINIEXIV_ERROR on failure.
 *
 * @note This function does not return the modified image buffer.
 * Use miniexiv_image_export_buf() after a successful call when working
 * with images opened from memory.
 *
 * @see miniexiv_image_export_buf()
 * @see miniexiv_image_open_file()
 * @see miniexiv_image_open_buf()
 */
MINIEXIV_EXPORT int miniexiv_image_metadata_write(miniexiv_image *image);
/**
 * @brief Creates an iterator for reading EXIF metadata entries.
 *
 * Creates a new EXIF iterator associated with the image. The iterator
 * contains a copy of the image EXIF data and can be used to traverse
 * all available EXIF tags.
 *
 * The returned iterator must be released using
 * miniexiv_exif_iterator_free().
 *
 * @param image Image handle.
 *
 * @return Pointer to a new EXIF iterator on success.
 * @return NULL on failure.
 *
 * @note The iterator stores a copy of the EXIF metadata. Changes made
 * to the image after iterator creation are not reflected in the iterator.
 *
 * @see miniexiv_exif_iterator_next()
 * @see miniexiv_exif_iterator_get_key()
 * @see miniexiv_exif_iterator_get_value()
 * @see miniexiv_exif_iterator_free()
 */
MINIEXIV_EXPORT miniexiv_exif_iterator *
miniexiv_exif_iterator_create(const miniexiv_image *image);
/**
 * @brief Advances the EXIF iterator to the next metadata entry.
 *
 * Moves the iterator to the next available EXIF tag and prepares the
 * current key and value for retrieval using
 * miniexiv_exif_iterator_get_key() and
 * miniexiv_exif_iterator_get_value().
 *
 * The iterator must be created using
 * miniexiv_exif_iterator_create() before calling this function.
 *
 * @param it EXIF iterator handle.
 *
 * @return MINIEXIV_ITERATOR_HAS_NEXT if the iterator points to a valid
 * EXIF entry.
 * @return MINIEXIV_ITERATOR_END if there are no more EXIF entries.
 * @return MINIEXIV_ERROR on failure.
 *
 * @note The first call positions the iterator on the first EXIF entry.
 * Subsequent calls move it forward.
 *
 * @see miniexiv_exif_iterator_create()
 * @see miniexiv_exif_iterator_get_key()
 * @see miniexiv_exif_iterator_get_value()
 */
MINIEXIV_EXPORT int miniexiv_exif_iterator_next(miniexiv_exif_iterator *it);
/**
 * @brief Gets the key of the current EXIF entry.
 *
 * Returns the key (name) of the EXIF tag at the current iterator position.
 *
 * @param it EXIF iterator handle.
 *
 * @return Pointer to a null-terminated string containing the current EXIF key.
 *         The returned string is owned by the iterator and must not be freed
 *         by the caller.
 *
 * @retval NULL If the iterator is NULL or is not positioned on a valid
 *         EXIF entry. Use miniexiv_get_last_error() to get more information.
 *
 * @note The returned pointer is only valid until the iterator state changes
 *       or the iterator is destroyed.
 *
 * @note If the key needs to be stored after calling
 *       miniexiv_exif_iterator_next(), the caller must copy the string.
 */
MINIEXIV_EXPORT const char *
miniexiv_exif_iterator_get_key(miniexiv_exif_iterator *it);
/**
 * @brief Returns the value of the current EXIF entry.
 *
 * Retrieves the value associated with the current EXIF metadata entry
 * selected by the last successful call to
 * miniexiv_exif_iterator_next().
 *
 * The returned string is owned by the iterator and must not be freed
 * by the caller. The pointer remains valid until the iterator is
 * advanced, modified, or released with
 * miniexiv_exif_iterator_free().
 *
 * @param it EXIF iterator handle.
 *
 * @return Pointer to the EXIF value string on success.
 * @return NULL if the iterator is invalid or not positioned on an
 * EXIF entry.
 *
 * @see miniexiv_exif_iterator_next()
 * @see miniexiv_exif_iterator_get_key()
 * @see miniexiv_exif_iterator_free()
 */
MINIEXIV_EXPORT const char *
miniexiv_exif_iterator_get_value(miniexiv_exif_iterator *it);
/**
 * @brief Releases an EXIF iterator.
 *
 * Frees all resources associated with the EXIF iterator created by
 * miniexiv_exif_iterator_create().
 *
 * After this function is called, the iterator handle must not be used
 * again.
 *
 * @param it EXIF iterator handle.
 *
 * @note It is safe to pass NULL. No action will be performed.
 *
 * @see miniexiv_exif_iterator_create()
 */
MINIEXIV_EXPORT void miniexiv_exif_iterator_free(miniexiv_exif_iterator *it);
/**
 * @brief Exports the current image data into a newly allocated memory buffer.
 *
 * Creates a copy of the current image data and returns it through
 * the output buffer parameters.
 *
 * The returned buffer contains the current binary image representation,
 * including any metadata changes already applied by
 * miniexiv_image_metadata_write().
 *
 * For images opened with miniexiv_image_open_buf(), this function is useful
 * for retrieving the modified image data without writing it to a file.
 *
 * For images opened with miniexiv_image_open_file(), this function reads
 * the current image data from the associated file.
 *
 * The returned buffer is allocated by MiniExiv and must be released
 * by the caller using miniexiv_free_buffer().
 *
 * @param image Image handle.
 * @param out_buf Pointer that receives the allocated image buffer.
 * @param out_size Pointer that receives the buffer size in bytes.
 *
 * @return MINIEXIV_OK on success.
 * @return MINIEXIV_ERROR on failure.
 *
 * @warning The caller owns the returned buffer and must release it
 * using miniexiv_free_buffer(). Do not use free() directly.
 *
 * @note On failure, out_buf is set to NULL and out_size is set to 0.
 *
 * @see miniexiv_free_buffer()
 * @see miniexiv_image_metadata_write()
 */
MINIEXIV_EXPORT int miniexiv_image_export_buf(const miniexiv_image *image,
                                              uint8_t **out_buf,
                                              size_t *out_size);
/**
 * @brief Releases a buffer allocated by MiniExiv.
 *
 * Frees the memory buffer returned by
 * miniexiv_image_export_buf().
 *
 * The caller must use this function to release the exported image
 * buffer. Do not use free() directly on buffers returned by MiniExiv.
 *
 * @param buffer Buffer previously allocated by MiniExiv.
 *
 * @note Passing NULL is safe and has no effect.
 *
 * @see miniexiv_image_export_buf()
 */
MINIEXIV_EXPORT void miniexiv_free_buffer(uint8_t *buffer);
/**
 * @brief Sets the value of an XMP metadata property.
 *
 * Creates a new XMP property or updates the value of an existing one.
 * If the specified XMP property already exists, its current value is
 * replaced with the new value.
 *
 * The changes are applied only to the in-memory image object.
 *
 * To make the changes persistent, call
 * miniexiv_image_metadata_write().
 *
 * @param image Image handle.
 * @param key XMP property key (for example, "Xmp.dc.creator").
 * @param value String value to assign to the XMP property.
 *
 * @return MINIEXIV_OK on success.
 * @return MINIEXIV_ERROR on failure.
 *
 * @note The specified XMP key must be supported by Exiv2.
 *
 * @see miniexiv_image_metadata_write()
 * @see miniexiv_image_export_xmp()
 */
MINIEXIV_EXPORT int miniexiv_image_set_xmp_string(miniexiv_image *image,
                                                  const char *key,
                                                  const char *value);
/**
 * @brief Checks whether an XMP property exists.
 *
 * @param image Image handle.
 * @param key XMP property key.
 *
 * @return MINIEXIV_TRUE if the property exists.
 * @return MINIEXIV_FALSE if the property does not exist.
 * @return MINIEXIV_ERROR on failure.
 */
MINIEXIV_EXPORT int miniexiv_image_has_xmp_key(const miniexiv_image *image,
                                               const char *key);
/**
 * @brief Gets the value of an XMP property.
 *
 * Retrieves the value of an XMP metadata field.
 *
 * The returned string must be released using
 * miniexiv_free_string().
 *
 * @param image Image handle.
 * @param key XMP key.
 * @param out_value Pointer receiving the allocated string.
 *
 * @return MINIEXIV_OK on success.
 * @return MINIEXIV_ERROR on failure.
 *
 * @note On failure, @p out_value is set to NULL.
 *
 * @see miniexiv_free_string()
 */
MINIEXIV_EXPORT int miniexiv_image_get_xmp_string(const miniexiv_image *image,
                                                const char *key,
                                                char **out_value);
/**
 * @brief Removes an XMP metadata property.
 *
 * Removes the specified XMP property from the image.
 * The change is applied only to the in-memory image object.
 *
 * Call miniexiv_image_metadata_write() to save changes.
 *
 * @param image Image handle.
 * @param key XMP property key.
 *
 * @return MINIEXIV_OK on success.
 * @return MINIEXIV_ERROR on failure.
 *
 * @note Removing a non-existing key is not an error.
 */
MINIEXIV_EXPORT int miniexiv_image_remove_xmp(miniexiv_image *image,
                                              const char *key);
/**
 * @brief Saves the image XMP packet to a file.
 *
 * Writes the current XMP metadata packet to a separate file.
 * This function does not modify the original image file or image buffer.
 *
 * The saved data contains only the XMP packet in XML format.
 *
 * If XMP metadata was modified using miniexiv_image_set_xmp_string(),
 * the changes must be present in the image object before calling this
 * function.
 *
 * @param image Image handle.
 * @param filename Path to the output XMP file.
 *
 * @return MINIEXIV_OK on success.
 * @return MINIEXIV_ERROR on failure.
 *
 * @note The output file is created or overwritten.
 *
 * @see miniexiv_image_set_xmp_string()
 * @see miniexiv_image_metadata_write()
 */
MINIEXIV_EXPORT int miniexiv_image_export_xmp(const miniexiv_image *image,
                                              const char *filename);
/**
 * @brief Sets the value of an IPTC metadata field.
 *
 * Creates a new IPTC metadata entry or updates the value of an
 * existing IPTC field.
 *
 * The change is applied only to the in-memory image object.
 * To save the modification permanently, call
 * miniexiv_image_metadata_write().
 *
 * @param image Image handle.
 * @param key IPTC metadata key (for example, "Iptc.Application2.Caption").
 * @param value String value to assign to the IPTC field.
 *
 * @return MINIEXIV_OK on success.
 * @return MINIEXIV_ERROR on failure.
 *
 * @note The specified IPTC key must be supported by Exiv2.
 *
 * @see miniexiv_image_get_iptc_string()
 * @see miniexiv_image_remove_iptc()
 * @see miniexiv_image_metadata_write()
 */
MINIEXIV_EXPORT int miniexiv_image_set_iptc_string(miniexiv_image *image,
                                                   const char *key,
                                                   const char *value);
/**
 * @brief Checks whether an IPTC metadata field exists.
 *
 * Determines whether the specified IPTC metadata key is present
 * in the image.
 *
 * @param image Image handle.
 * @param key IPTC metadata key.
 *
 * @return MINIEXIV_TRUE if the IPTC field exists.
 * @return MINIEXIV_FALSE if the IPTC field does not exist.
 * @return MINIEXIV_ERROR if an error occurs.
 *
 * @see miniexiv_image_get_iptc_string()
 * @see miniexiv_image_remove_iptc()
 */
MINIEXIV_EXPORT int miniexiv_image_has_iptc_key(const miniexiv_image *image,
                                                const char *key);
/**
 * @brief Gets the value of an IPTC metadata field.
 *
 * Retrieves the string representation of the specified IPTC metadata field.
 *
 * The returned string is allocated by MiniExiv and must be released
 * using miniexiv_free_string().
 *
 * The caller must not release the returned string using free().
 *
 * @param image Image handle.
 * @param key IPTC metadata key.
 * @param out_value Pointer receiving the allocated string.
 *
 * @return MINIEXIV_OK on success.
 * @return MINIEXIV_ERROR on failure.
 *
 * @note If the specified IPTC key does not exist, the function fails
 *       and sets the last error message.
 *
 * @note On failure, *out_value is set to NULL.
 *
 * @see miniexiv_free_string()
 * @see miniexiv_image_has_iptc_key()
 */
MINIEXIV_EXPORT int miniexiv_image_get_iptc_string(const miniexiv_image *image,
                                                const char *key,
                                                char **out_value);
/**
 * @brief Removes an IPTC metadata field.
 *
 * Removes the specified IPTC metadata field from the image.
 *
 * The change is applied only to the in-memory image object.
 * To make the modification permanent, call
 * miniexiv_image_metadata_write().
 *
 * Removing a non-existing IPTC key is not considered an error.
 *
 * @param image Image handle.
 * @param key IPTC metadata key to remove.
 *
 * @return MINIEXIV_OK on success.
 * @return MINIEXIV_ERROR on failure.
 *
 * @note Removing a missing IPTC field succeeds without changes.
 *
 * @see miniexiv_image_set_iptc_string()
 * @see miniexiv_image_metadata_write()
 */
MINIEXIV_EXPORT int miniexiv_image_remove_iptc(miniexiv_image *image,
                                               const char *key);
/**
 * @brief Gets image pixel dimensions.
 *
 * Retrieves the width and height of the image.
 *
 * @param image Image handle.
 * @param width Pointer receiving image width.
 * @param height Pointer receiving image height.
 *
 * @return MINIEXIV_OK on success.
 * @return MINIEXIV_ERROR on failure.
 *
 * @note width and height must not be NULL.
 */
MINIEXIV_EXPORT int miniexiv_image_get_dimensions(const miniexiv_image *image,
                                                  uint32_t *width,
                                                  uint32_t *height);
/**
 * @brief Releases a string allocated by MiniExiv.
 *
 * Frees memory allocated by functions that return strings through
 * an output parameter.
 *
 * Currently used by:
 * - miniexiv_image_get_comment()
 * - miniexiv_image_get_exif_string()
 * - miniexiv_image_get_xmp_string()
 * - miniexiv_image_get_iptc_string()
 *
 * The pointer passed to this function must have been allocated by MiniExiv.
 *
 * Passing pointers to string literals, stack memory, or memory allocated
 * by the caller results in undefined behavior.
 *
 * @param ptr String pointer returned by MiniExiv.
 *
 * @note Passing NULL is safe and has no effect.
 *
 * @see miniexiv_image_get_exif_string()
 * @see miniexiv_image_get_xmp_string()
 * @see miniexiv_image_get_iptc_string()
 * @see miniexiv_image_get_comment()
 */
MINIEXIV_EXPORT void miniexiv_free_string(char *ptr);
/**
 * @brief Saves the current image data to a file.
 *
 * Writes the current state of the image object into a new file.
 *
 * Unlike miniexiv_image_metadata_write(), this function does not
 * modify the original source image opened with
 * miniexiv_image_open_file().
 *
 * This function is useful when the modified image needs to be
 * saved under a different filename.
 * @warning An existing destination file will be overwritten.
 * @param image Image handle.
 * @param filename Destination file path.
 *
 * @return MINIEXIV_OK on success.
 * @return MINIEXIV_ERROR on failure.
 *
 * @note Metadata changes must be applied before saving.
 *
 * @see miniexiv_image_metadata_write()
 * @see miniexiv_image_open_file()
 */
MINIEXIV_EXPORT int miniexiv_image_save_to_file(const miniexiv_image *image,
                                                   const char *filename);
/**
 * @brief Creates an iterator for XMP metadata.
 *
 * Creates a new iterator used to traverse XMP properties
 * stored in the image.
 *
 * The iterator owns its internal state and must be released
 * using miniexiv_xmp_iterator_free().
 *
 * @note The iterator contains a snapshot of metadata created
 *       at the time of initialization.
 *       Changes made to the image after iterator creation
 *       are not visible through this iterator.
 *
 * @param image Image handle.
 *
 * @return XMP iterator on success.
 * @return NULL on failure.
 *
 * @see miniexiv_xmp_iterator_next()
 * @see miniexiv_xmp_iterator_free()
 */
MINIEXIV_EXPORT miniexiv_xmp_iterator *
miniexiv_xmp_iterator_create(const miniexiv_image *image);
/**
 * @brief Advances the XMP iterator.
 *
 * Moves the iterator to the next XMP metadata entry.
 *
 * The current key and value can be retrieved using:
 *
 * - miniexiv_xmp_iterator_get_key()
 * - miniexiv_xmp_iterator_get_value()
 *
 * @param it XMP iterator.
 *
 * @return MINIEXIV_ITERATOR_HAS_NEXT if an entry exists.
 * @return MINIEXIV_ITERATOR_END if iteration is complete.
 * @return MINIEXIV_ERROR on failure.
 *
 * @see miniexiv_xmp_iterator_get_key()
 * @see miniexiv_xmp_iterator_get_value()
 */
MINIEXIV_EXPORT int
miniexiv_xmp_iterator_next(miniexiv_xmp_iterator *it);
/**
 * @brief Returns the current XMP property key.
 *
 * Returns the name of the XMP property at the current
 * iterator position.
 *
 * The returned string belongs to the iterator and must not be freed by the
 * caller. The pointer remains valid until the iterator is advanced or freed.
 *
 * @param it XMP iterator.
 *
 * @return Pointer to XMP key string.
 * @return NULL if iterator is invalid.
 */
MINIEXIV_EXPORT const char *
miniexiv_xmp_iterator_get_key(miniexiv_xmp_iterator *it);
/**
 * @brief Returns the current XMP property value.
 *
 * Returns the value associated with the current XMP property.
 *
 * The returned pointer is owned by the iterator and remains
 * valid until the iterator is advanced or freed.
 *
 * @param it XMP iterator.
 *
 * @return Pointer to XMP value string.
 * @return NULL if iterator is invalid.
 */

MINIEXIV_EXPORT const char *
miniexiv_xmp_iterator_get_value(miniexiv_xmp_iterator *it);
/**
 * @brief Releases an XMP iterator.
 *
 * Frees resources allocated by
 * miniexiv_xmp_iterator_create().
 *
 * @param it XMP iterator.
 *
 * @note Passing NULL is safe.
 */

MINIEXIV_EXPORT void
miniexiv_xmp_iterator_free(miniexiv_xmp_iterator *it);
/**
 * @brief Creates an iterator for IPTC metadata.
 *
 * Creates a new iterator used to traverse IPTC fields.
 *
 * @param image Image handle.
 *
 * @return IPTC iterator on success.
 * @return NULL on failure.
 */
MINIEXIV_EXPORT miniexiv_iptc_iterator *
miniexiv_iptc_iterator_create(const miniexiv_image *image);
/**
 * @brief Advances the IPTC iterator.
 *
 * @param it IPTC iterator.
 *
 * @return MINIEXIV_ITERATOR_HAS_NEXT if an entry exists.
 * @return MINIEXIV_ITERATOR_END if iteration is complete.
 * @return MINIEXIV_ERROR on failure.
 */
MINIEXIV_EXPORT int
miniexiv_iptc_iterator_next(miniexiv_iptc_iterator *it);
/**
 * @brief Returns the current IPTC key.
 *
 * The returned string is owned by the iterator and must not be freed by the
 * caller. The pointer remains valid until the iterator is advanced or freed.
 *
 * @param it IPTC iterator.
 *
 * @return IPTC key string or NULL.
 */

MINIEXIV_EXPORT const char *
miniexiv_iptc_iterator_get_key(miniexiv_iptc_iterator *it);
/**
 * @brief Returns the current IPTC value.
 *
 * The returned string is owned by the iterator and must not be freed by the
 * caller. The pointer remains valid until the iterator is advanced or freed.
 *
 * @param it IPTC iterator.
 *
 * @return IPTC value string or NULL.
 */

MINIEXIV_EXPORT const char *
miniexiv_iptc_iterator_get_value(miniexiv_iptc_iterator *it);
/**
 * @brief Releases an IPTC iterator.
 *
 * @param it IPTC iterator.
 *
 * @note Passing NULL is safe.
 */

MINIEXIV_EXPORT void
miniexiv_iptc_iterator_free(miniexiv_iptc_iterator *it);
/**
 * @brief Returns metadata access capabilities.
 *
 * Checks whether a specific metadata type exists in the image
 * and what operations are supported.
 *
 * Example:
 *
 * @code
 * miniexiv_access_mode mode =
 *     miniexiv_image_check_metadata_mode(
 *         image,
 *         MINIEXIV_METADATA_EXIF);
 *
 * if (mode == MINIEXIV_ACCESS_READWRITE) {
 *     // EXIF can be read and modified
 * }
 * @endcode
 *
 * @param image Image handle.
 * @param metadata Metadata type to check.
 *
 * @return MINIEXIV_ACCESS_NONE
 *         Metadata is unavailable.
 *
 * @return MINIEXIV_ACCESS_READ
 *         Metadata exists and can be read.
 *
 * @return MINIEXIV_ACCESS_WRITE
 *         Metadata can be created or written.
 *
 * @return MINIEXIV_ACCESS_READWRITE
 *         Metadata supports both operations.
 *
 * @return MINIEXIV_ACCESS_ERROR
 *         An error occurred.
 */
MINIEXIV_EXPORT miniexiv_access_mode miniexiv_image_check_metadata_mode(
    const miniexiv_image *image,
    miniexiv_metadata_id metadata
);

#ifdef __cplusplus
}
#endif

#endif
