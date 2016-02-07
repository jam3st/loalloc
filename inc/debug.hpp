#pragma once

#include "types.hpp"

namespace Gx {

    extern void debug(char const *const);

    class Debug {
    public:
        static constexpr bool enabled = true;
        static constexpr char const *const end = "\r\n";

        static Debug start() {
            return Debug();
        }

        Debug &operator+(char const *const str) {
            if (enabled) {
                debug(str);
            }
            return *this;
        }

        Debug &operator+(uint64_t const val) {
            if (enabled) {
                debug(toString(val).d);
            }
            return *this;
        }

        Debug &operator+(void *const val) {
            if (enabled) {
                debug(toString(reinterpret_cast<uint64_t>(val)).d);
            }
            return *this;
        }


    private:
        using ss = struct {
                char d[32];
        };

        static ss toString(uint64_t const val) {
            ss str{.d = {0}};
            size_t pos = 0;
            for (ssize_t i = 63; i >= 0; i = i - 4) {
                auto c = getBits(val, i - 3, i);
                if (c != 0 || pos != 0) {
                    str.d[pos++] = toPrintHex(c);
                }
            }
            if (pos == 0) {
                str.d[pos++] = toPrintHex(0);
            }
            str.d[pos++] = '\0';
            return str;
        }

    private:
        static uint8_t toPrintHex(uint8_t const val) {
            if (val < 10u) {
                return val + '0';
            } else {
                return val - 10u + 'a';
            }
        }

        static uint64_t getBits(Gx::uint64_t const val, Gx::size_t const start, Gx::size_t const end) {
            return (val >> start) & ((1u << (end - start + 1)) - 1);
        }
    };

}

