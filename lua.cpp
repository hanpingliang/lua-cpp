// lua.cpp
extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#include <string.h>
#include <iostream>
#include <cassert>
#include <vector>
#include <utility>

using namespace std;

static struct {
    const char * name;
    int type;
} fields[] = {
    {"velocity", LUA_TNUMBER},
    {"value", LUA_TNUMBER},
    {NULL, 0}
};

typedef vector<pair<double, double> > PairVec;
typedef pair<double, double> Pair;

class ControlParam {
  public:
    ControlParam(const string& file_name) {
        L = luaL_newstate();
        luaL_openlibs(L);

        if (luaL_loadfile(L, file_name.c_str()) || lua_pcall(L, 0, 0, 0))
            cout << "can not run file, reason: " << lua_tostring(L, -1) << endl;

        parse_acc_a_max();
        parse_width();
        parse_key_invalid();
        parse_value_invalid();
        parse_topic();
        parse_steer_wheel_ratio();    
    }

    void parse_acc_a_max() {
        lua_getglobal(L, "acc_a_max");
        acc_a_max_defined = lua_isnumber(L, -1);
        lua_pop(L, 1);

        lua_getglobal(L, "acc_a_max");
        acc_a_max_val = lua_tonumber(L, -1);
        lua_pop(L, 1);
    }
    bool has_acc_a_max() { // value check, key check
        return acc_a_max_defined;
    }
    double acc_a_max() {
        assert(acc_a_max_defined);
        return acc_a_max_val;
    }

    void parse_width() {
        lua_getglobal(L, "width");
        width_defined = lua_isinteger(L, -1);
        lua_pop(L, 1);   

        lua_getglobal(L, "width");
        width_val = lua_tointeger(L, -1);
        lua_pop(L, 1);
    }
    bool has_width() {
        return width_defined;
    }
    int width() {
        assert(width_defined);
        return width_val;
    }

    void parse_key_invalid() {
        lua_getglobal(L, "key_invalid");
        key_invalid_defined = lua_isnumber(L, -1);
        cout << "key_invalid-exist: " << key_invalid_defined << endl;
    }
    bool has_key_invalid() { // invalid key check
        return key_invalid_defined;
    }
    double key_invalid() {
        assert(key_invalid_defined);
        return key_invalid_val;
    }

    void parse_value_invalid() {
        lua_getglobal(L, "value_invalid");
        value_invalid_defined = lua_isnumber(L, -1);
        cout << "value_invalid-exist: " << value_invalid_defined << endl;
        lua_pop(L, 1);    
    }
    bool has_value_invalid() { // invalid value check
        return value_invalid_defined;
    }
    double value_invalid() {
        assert(value_invalid_defined);
        return value_invalid_val;
    }

    void parse_topic() {
        lua_getglobal(L, "topic");
        topic_defined = lua_isstring(L, -1);
        lua_pop(L, 1);

        lua_getglobal(L, "topic");
        topic_val = lua_tostring(L, -1);
        lua_pop(L, 1);
    }
    bool has_topic() {
        return topic_defined;
    }
    string topic() {
        return topic_val;
    }

    void parse_steer_wheel_ratio() {
        lua_getglobal(L, "steer_wheel_ratio");
        luaL_checktype(L, -1, LUA_TTABLE);

        int i;
        for (i = 1; ; i++) {
            lua_rawgeti(L, -1, i);
            if (lua_isnil(L, -1)) {
                lua_pop(L, 1);
                break;
            }
            // an element of the table should now be at the top of the stack
            luaL_checktype(L, -1, LUA_TTABLE);
            // read the content of that element
            int field_index;
            Pair item;
            for (field_index = 0; fields[field_index].name != NULL; field_index++) {

                lua_getfield(L, -1, fields[field_index].name);
                luaL_checktype(L, -1, fields[field_index].type);
                // you should probably use a function pointer in the fields table.
                // I am using a simple switch/case here
                switch(field_index) {
                    case 0:
                        item.first = lua_tonumber(L, -1);
                        break;
                    case 1:
                        item.second = lua_tonumber(L, -1);
                        break;
                }
                // remove the field value from the top of the stack
                lua_pop(L, 1); 
            }
            // remove the element of the 'listen' table from the top of the stack.
            lua_pop(L, 1);
            steer_wheel_ratio_vec.emplace_back(item);
        }

        cout << "size of steer_wheel_ratio_vec: " << steer_wheel_ratio_vec.size() << endl;
    }

    // repeated (key, value) pairs in current config
    int steer_wheel_ratio_size() {
        return steer_wheel_ratio_vec.size();
    }

    PairVec steer_wheel_ratio() {
        return steer_wheel_ratio_vec;
    }

  private:

    lua_State* L = NULL;

    // steer_wheel_ratio
    PairVec steer_wheel_ratio_vec;

    // acc_a_max
    bool acc_a_max_defined = false;
    double acc_a_max_val = 0.0;

    // width
    bool width_defined = false;
    double width_val = 0.0;

    // key_invalid
    bool key_invalid_defined = false;
    double key_invalid_val = 0.0;

    // value_invalid
    bool value_invalid_defined = false;
    double value_invalid_val = 0.0;

    // key_invalid
    bool topic_defined = false;
    string topic_val;

};


int main(int argc, char* argv[]) {

    string file_name = "./test.lua";
    ControlParam cp(file_name);

    if (cp.has_acc_a_max()) {
        cout << "acc_a_max: " << cp.acc_a_max() << endl;
    }

    if (cp.has_key_invalid()) {
        cout << "key_invalid: " << cp.key_invalid() << endl;
    }

    if (cp.has_value_invalid()) {
        cout << "value_invalid: " << cp.value_invalid() << endl;
    }

    if (cp.has_width()) {
        cout << "width: " << cp.width() << endl;
    }

    if (cp.has_topic()) {
        cout << "topic: " << cp.topic() << endl;
    }

    if (cp.steer_wheel_ratio_size() > 0) {
        for(const auto& item: cp.steer_wheel_ratio()) {
            cout << "velocity: " << item.first << ", value: " << item.second << endl;
        }
    }
    
    return 0;
}
