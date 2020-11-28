#ifndef ORB_JSON_H
#define ORB_JSON_H

#include <json-c/json.h>
#include "../src/orb_types.h"

json_object * orb_json_object(json_object * parent, const char * name) ;

json_object * orb_json_array(json_object * parent, const char * name);

json_object * orb_json_i32(json_object * parent, const char * name, i32 val);

json_object * orb_json_bool(json_object * parent, const char * name, bool val);

json_object * orb_json_string(json_object * parent,
                              const char * name, const char * val);

void orb_json_move(json_object * parent, json_object * child, const char * name);

json_object * orb_json_find(json_object * json, const char * key);

const char * orb_json_get_string(struct json_object * json, const char * key);

#endif /* ORB_JSON_H */
