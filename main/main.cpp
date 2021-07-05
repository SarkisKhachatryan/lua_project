#include <iostream>
#include <vector>
#include <functional>
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

int main() {
    std::vector<std::function<int()>> functions;
    functions.push_back(print_basic_number);
    functions.push_back(lua_stack_play);
    functions.push_back(lua_function_call);
    functions.push_back(native_function_call);
    int error_code;
    for (size_t i = 0; i < functions.size(); i++) {
        error_code = functions.at(i)();
        if (error_code != 0) {
            return error_code;
        }
        
    }
    return 0;
}
