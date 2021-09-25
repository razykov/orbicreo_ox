#include "orb_json.h"
#include "../orb_utils/orb_log.h"


void orb_json_move(json_object * parent, json_object * child, const char * name)
{
    if (parent && child) {
        if ( json_object_get_type(parent) == json_type_object )
            json_object_object_add(parent, name, child);
        else if ( json_object_get_type(parent) == json_type_array )
            json_object_array_add(parent, child);
        else
            orb_err("incorrect parent json");
    }
}

json_object * orb_json_object(json_object * parent, const char * name)
{
    json_object * json = json_object_new_object();
    orb_json_move(parent, json, name);
    return json;
}

json_object * orb_json_array(json_object * parent, const char * name)
{
    json_object * json = json_object_new_array();
    orb_json_move(parent, json, name);
    return json;
}

json_object * orb_json_i32(json_object * parent, const char * name, i32 val)
{
    json_object * json = json_object_new_int(val);
    orb_json_move(parent, json, name);
    return json;
}

json_object * orb_json_bool(json_object * parent, const char * name, bool val)
{
    json_object * json = json_object_new_boolean(val);
    orb_json_move(parent, json, name);
    return json;
}

json_object * orb_json_string(json_object * parent,
                              const char * name, const char * val)
{
    json_object * json = json_object_new_string(val);
    orb_json_move(parent, json, name);
    return json;
}

const char * orb_json_get_string(json_object * json, const char * key)
{
    struct json_object * tmp = json_object_object_get(json, key);
    return json_object_get_string(tmp);
}

json_object * orb_json_find(json_object * json, const char * key)
{
    return json_object_object_get(json, key);
}
