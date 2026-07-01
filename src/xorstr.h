#pragma once
#include <string>
#include <array>

template<size_t N>
class XorStr {
private:
    std::array<char, N> data;
    
    constexpr char key() const {
        return 'K' ^ 'E' ^ 'Y' ^ 'S' ^ 0x69;
    }
    
public:
    constexpr XorStr(const char(&str)[N]) : data{} {
        for(size_t i = 0; i < N; i++)
            data[i] = str[i] ^ key();
    }
    
    std::string decrypt() const {
        std::string result;
        result.reserve(N);
        for(size_t i = 0; i < N; i++)
            result += data[i] ^ key();
        return result;
    }
};

#define XOR(str) XorStr(str).decrypt().c_str()
