#ifndef ORB_JSON_H
#define ORB_JSON_H

#include <json-c/json.h>
#include "../orb_types/orb_types.h"

/*!
 * \brief Create child json-object by name
 * \param parent Parent json-object
 * \param name Name of child object
 * \return New json-object, NULL on failure
 */
json_object * orb_json_object(json_object * parent, const char * name) ;

/*!
 * \brief Create child json-array by name
 * \param parent Parent json-object
 * \param name Name of child array
 * \return New json-array, NULL on failure
 */
json_object * orb_json_array(json_object * parent, const char * name);

/*!
 * \brief Create child json-integer by name
 * \param parent Parent json-object
 * \param name Name of child integer
 * \param val Integer value
 * \return New json-integer, NULL on failure
 */
json_object * orb_json_i32(json_object * parent, const char * name, i32 val);

/*!
 * \brief Set bool value by name
 * \param parent Parent json-object
 * \param name Name of child bool
 * \param val Child bool value
 */
void orb_json_bool(json_object * parent, const char * name, bool val);

/*!
 * \brief Create child json-string by name
 * \param parent Parent json-object
 * \param name Name of string
 * \param val String
 * \return New json-string, NULL on failure
 */
json_object * orb_json_string(json_object * parent,
                              const char * name, const char * val);

/*!
 * \brief Move child object to parent object
 * \param parent Parent object
 * \param child Child object
 * \param name Name of child object
 */
void orb_json_move(json_object * parent, json_object * child, const char * name);

/*!
 * \brief Find child object by key
 * \param json Parent object
 * \param key Key
 * \return Child object, NULL if not found
 */
json_object * orb_json_find(json_object * json, const char * key);

/*!
 * \brief Get child string value by key
 * \param json Parent object
 * \param key Key
 * \return Child string value, NULL if not found
 */
const char * orb_json_get_string(struct json_object * json, const char * key);

/*!
 * \brief Get child integer value by key
 * \param json Parent object
 * \param key Key
 * \return Child integer value, 0 if not found
 */
i32 orb_json_get_int(json_object * json, const char * key);

#endif /* ORB_JSON_H */
