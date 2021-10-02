#ifndef ORB_UTILS_H
#define ORB_UTILS_H

#include <dirent.h>
#include <sys/types.h>
#include <json-c/json.h>

#include "../orb_types/orb_types.h"

#define SHA1_LEN       (20)
#define SHA1_EMPTY_STR "0000000000000000000000000000000000000000"

#define orb_try(pred)  if (!(pred)) return false
#define safe_free(val) if (val) { free(val); val = NULL; }

//! sha1 sum type
typedef u8 * orb_sha1;

/*!
 * \brief Check directory exist
 * \param path Path to directory
 * \return true if directory exist
 */
bool orb_dir_exist(const char * path);

/*!
 * \brief Check file exist
 * \param path Path to file
 * \return true if file exist
 */
bool orb_file_exist(const char * path);

/*!
 * \brief Recursive creation directory
 * \param dir Directory path
 * \return true on success
 */
bool orb_mkdir_p(const char * dir);

/*!
 * \brief Remove directory, subdirecties and files
 * \param path Directory path
 * \return true on success
 */
bool orb_rmrf(const char * path);

/*!
 * \brief Concatination two strings to thread static buffer
 * \param head Head
 * \param tail Tail
 * \return Static buffer containing string "head/tail"
 */
const char * orb_cat(const char * head, const char * tail);

/*!
 * \brief Safty copy file
 * \param from Original file
 * \param to File copy
 * \return true on success
 */
bool orb_copy(const char * from, const char * to);

/*!
 * \brief Calculate sha1 for file
 * \param path File path
 * \return sha1 sum of file, NULL on failure
 */
orb_sha1 orb_file_sha1(const char * path);

/*!
 * \brief sha1 sum string representation
 * \param sha1 sha1 sum
 * \return String representation of sha1 sum
 */
const char * orb_sha2str(orb_sha1 sha1);

/*!
 * \brief Check is a child directory
 * \param dir Current directory dirent
 * \return true if is a child directory
 */
bool orb_is_include_dir(struct dirent * dir);

#endif /* ORB_UTILS_H */
