#include <iostream>
#include <vector>
#include <functional>
#include <string.h>
#include "lua.hpp"

//declare number in lua get it via stack and print it
int print_basic_number() {
    lua_State* L = luaL_newstate();
    const char* code = R"(x = 999)";
    luaL_dostring(L, code);
    lua_getglobal(L, "x");
    lua_Number x = lua_tonumber(L, 1);
    if(static_cast<int>(x) != 999) {
        return 1;
    }
    lua_close(L);
    return 0;
}

//pushing and popping some value to stack accessing it with different indices and comparing with each other
int lua_stack_play() {
    lua_State* L = luaL_newstate();
    lua_pushnumber(L, 12);
    lua_pushnumber(L, 34);
    lua_pushnumber(L, 56);

    lua_Number x = lua_tonumber(L, 1);
    lua_Number y = lua_tonumber(L, 2);
    lua_Number z = lua_tonumber(L, 3);

    lua_Number x1 = lua_tonumber(L, -3);
    lua_Number y1 = lua_tonumber(L, -2);
    lua_Number z1 = lua_tonumber(L, -1);

    if(x != x1) {
        return 1;
    }
    if(y != y1) {
        return 2;
    } 
    if(z != z1) {
        return 3;
    }

    lua_remove(L, 2);
    lua_Number w = lua_tonumber(L, -2);
    if(w != x || w == y) {
        return 4;
    }

    lua_close(L);
    return 0;
}

//declaring function in lua calling it, passing arguments and recieving return values from it
int lua_function_call(){
    lua_State* L = luaL_newstate();
    const char* foo = R"(
    function pythagoras(a, b)
        return (a * a), (b * b)
    end
    )";
    luaL_dostring(L, foo);
    lua_getglobal(L, "pythagoras");
    if(lua_isfunction(L, -1)) {
        int a = 3;
        int b = 4;
        int a_expected  = (a * a);
        int b_expected  = (b * b);
        lua_pushnumber(L, a);
        lua_pushnumber(L, b);

        lua_pcall(L, 2, 2, 0);
        lua_Number res_a = lua_tonumber(L, -2);
        lua_Number res_b = lua_tonumber(L, -1);
        lua_Number b_not_poped = lua_tonumber(L, -3);
        lua_Number a_not_poped = lua_tonumber(L, -4);

        if(static_cast<int>(res_a) == a_expected && static_cast<int>(res_b) == b_expected) {
            return 0;
        } else {
            return 1;
        }
        if(static_cast<int>(b_not_poped) == b || static_cast<int>(a_not_poped) == a) {
            return 2;
        }
    } else {
        return 3;
    }

    lua_close(L);
}

//declaring function on c side calling it from lua, passing arguments and recieving return values from it
int native_function_call(){
    lua_State* L = luaL_newstate();

    auto nativePythagoras = [](lua_State* L) -> int {
        lua_Number a = lua_tonumber(L, -2);
        lua_Number b = lua_tonumber(L, -1);
        int c = (a * a) + (b * b);
        lua_pushnumber(L, c);
        return 1;
    };

    const char* foo = R"(
    function pythagoras(a, b)
        csqr = nativePythagoras(a, b)
        return (a * a), (b * b), csqr
    end
    )";
    lua_pushcfunction(L, nativePythagoras);
    lua_setglobal(L, "nativePythagoras");
    luaL_dostring(L, foo);
    lua_getglobal(L, "pythagoras");
    if(lua_isfunction(L, -1)) {
        int a = 3;
        int b = 4;
        int a_expected  = (a * a);
        int b_expected  = (b * b);
        lua_pushnumber(L, a);
        lua_pushnumber(L, b);

        lua_pcall(L, 2, 3, 0);
        lua_Number res_a = lua_tonumber(L, -3);
        lua_Number res_b = lua_tonumber(L, -2);
        lua_Number res_c = lua_tonumber(L, -1);

        if(static_cast<int>(res_a) == a_expected &&
            static_cast<int>(res_b) == b_expected &&
            static_cast<int>(res_c) == res_a + res_b) {
            return 0;
        } else {
            return 1;
        }

        lua_Number a_not_poped = lua_tonumber(L, -5);
        lua_Number b_not_poped = lua_tonumber(L, -4);
        if(static_cast<int>(b_not_poped) == b || static_cast<int>(a_not_poped) == a) {
            return 2;
        }
    } else {
        return 3;
    }

    lua_close(L);
}

//creating user data in c which then passed to lua vi stack and allocated ther, then 
//members and function is set on lua side
int user_date_creation() {
    struct Point {
        int x;
        int y;

        void mul(int num) {
            x *= num;
            y *= num;
        }
    };

    auto createObject = [](lua_State* L) -> int {
        Point *objPtr = static_cast<Point*>(lua_newuserdata(L, sizeof(Point)));
        objPtr->x = 1;
        objPtr->y = 2;
        return 1;
    };

    auto mulObject = [](lua_State* L) -> int {
        Point* obj = static_cast<Point*>(lua_touserdata(L, -2));
        lua_Number num = lua_tonumber(L, -1);
        obj->mul(num);

        return 1;
    };

    const char* script_1 = R"(
    obj = createObject()
    )";

    const char* script_2 = R"(
    mul_obj(obj, 5)
    )";

    lua_State* L = luaL_newstate();
    lua_pushcfunction(L, createObject);
    lua_setglobal(L, "createObject");
    luaL_dostring(L, script_1);
    lua_getglobal(L, "obj");
    if (lua_isuserdata(L, -1)) {
        Point* obj = static_cast<Point*>(lua_touserdata(L, -1));
        int expected_x = 1;
        int expected_y = 2;
        auto x = obj->x;
        auto y = obj->y;

        if(expected_x != x || expected_y != y) {
            std::cerr << "Error on: " << __LINE__ << " wrong value was set " << std::endl;
            return 2;
        }
        lua_pushcfunction(L, mulObject);
        lua_setglobal(L, "mul_obj");
        luaL_dostring(L, script_2);
        lua_getglobal(L, "obj");
        if (lua_isuserdata(L, -1)) {
            Point* obj = static_cast<Point*>(lua_touserdata(L, -1));
            expected_x *= 5;
            expected_y *= 5;
            auto x = obj->x;
            auto y = obj->y;

            if(expected_x != x || expected_y != y) {
                std::cerr << "Error on: " << __LINE__ << " wrong value was set " << std::endl;
                return 4;
            }
        } else {
            std::cerr << "Error on: " << __LINE__ << " not user data where provided " << std::endl;
            return 3;
        }
    } else {
        std::cerr << "Error on: " << __LINE__ << " not user data where provided " << std::endl;
        return 1;
    }

    lua_close(L);
    return 0;
}

//creating table geting it's value and setting new key-value
int table_creation() {
    lua_State* L = luaL_newstate();

    const char* script_1 = R"(
    t = { one = "true", two = "false"}
    )";
    luaL_dostring(L, script_1);
    lua_getglobal(L, "t");
    if(!lua_istable(L, -1)) {
        std::cerr << "Error on: " << __LINE__ << " not a table provided " << std::endl;
        return 1;
    }

    lua_pushstring(L, "one");
    lua_gettable(L, -2);

    const char* onePtr = lua_tostring(L, -1);
    const char* one_expected = {"true\0"};
    if(strcmp(onePtr, one_expected) != 0) {
        std::cerr << "Error on: " << __LINE__ << " not correc value set in table as value " << std::endl;
        return 2;
    }

    lua_getglobal(L, "t");
    lua_getfield(L, -1, "two");
    const char* twoPtr = lua_tostring(L, -1);
    const char* two_expected = {"false\0"};
    if(strcmp(twoPtr, two_expected) != 0) {
        std::cerr << "Error on: " << __LINE__ << " not correc value set in table as value " << std::endl;
        return 3;
    }

    lua_getglobal(L, "t");
    lua_pushstring(L, "true");
    lua_setfield(L, -2, "three");

    lua_getglobal(L, "t");
    lua_getfield(L, -1, "three");
    const char* threePtr = lua_tostring(L, -1);
    const char* three_expected = {"true\0"};
    if(strcmp(threePtr, three_expected) != 0) {
        std::cerr << "Error on: " << __LINE__ << " not correc value set in table as value " << std::endl;
        return 3;
    }

    lua_close(L);
    return 0;
}

int main() {
    // std::vector<std::function<int()>> functions;
    // functions.push_back(print_basic_number);
    // functions.push_back(lua_stack_play);
    // functions.push_back(lua_function_call);
    // functions.push_back(native_function_call);
    // functions.push_back(user_date_creation);
    // functions.push_back(table_creation);

    // int error_code;
    // for (size_t i = 0; i < functions.size(); i++) {
    //     error_code = functions.at(i)();
    //     if (error_code != 0) {
    //         return error_code;
    //     }
    // }

    // return 0;
}
