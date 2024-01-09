#pragma once
#include <iostream>
#include <cstddef>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <bitset>


#define Code_value_bits 16
typedef long code_value;
#define Top_value (((code_value)1<<Code_value_bits)-1)


#define First_qtr (Top_value/4 + 1)
#define Half (2*First_qtr)
#define Third_qtr (3*First_qtr)


#define Max_frequency 16383

struct Model {
    int n, esc, s;
    std::map<std::string, std::vector<int>> model;
    std::vector<int> base;
    std::vector<int> n_context;

    Model(int n) :n(n), esc(n + 1), s(n + 2) {
        base = std::vector<int>(s);
        n_context = base;
        for (auto i = (s - 2); i >= 0; i--) {
            base[i] = base[i + 1] + 1;
            n_context[i] = 1;
        }
        model[""] = n_context;
    }
    ~Model() = default;
    void rescale(std::vector<int>& cm) {
        int tmp = 0;
        for (auto i = esc; i >= 0; i--) {
            cm[i] = (cm[i] + tmp) / 2;
            tmp++;
        }
    }
    void update_model(int symbol, std::vector<int>& cm) {
        if (cm[0] == Max_frequency) {
            rescale(cm);
        }
        while (symbol > 0) {
            symbol--;
            cm[symbol]++;
        }
        
    }
    void update_base(int symbol) {
        while (symbol > 0) {
            symbol--;
            base[symbol]--;
        }
        
    }
};


struct Decoder :Model {
    code_value low, high, bits_to_follow;
    code_value value;
    int n, bits_to_go, garbage_bits;
    int buffer;
    int n_of_context;
    std::string context;
    std::vector<int> data;
    int len;
    std::ifstream *inp;
    int readbits;

    Decoder(int n, int n_of_context, int len, std::ifstream* inp) :n(n), n_of_context(n_of_context),len(len),inp(inp), Model(n) {
        start_decoding();
        readbits = 0;
    }
    ~Decoder() = default;

    int input_bit() {
        if (bits_to_go == 0) {
            inp->read((char*)&buffer, 1);
            readbits++;
            bits_to_go = 8;
        }
        int t = buffer & 0x01;
        buffer >>= 1;
        bits_to_go--;
        return t;
    }
    void start_decoding() {
        value = 0;
        for (auto i = 0; i < Code_value_bits; i++) {
            value = 2 * value + input_bit();
        }
        low = 0;
        high = Top_value;
        bits_to_go = 0;
        garbage_bits = 0;
        context = "";
    }
    int decode_symbol(const std::vector<int>& cum_freq)
    {
        code_value range = high - low + 1;
        int cum = ((value - low + 1) * cum_freq[0] - 1) / range;
        int symbol;
        for (symbol = 1; cum_freq[symbol] > cum; symbol++);
        high = low + (range * cum_freq[symbol - 1]) / cum_freq[0] - 1;
        low = low + (range * cum_freq[symbol]) / cum_freq[0];

        for (;;) {
            if (high < Half) {

            }
            else if (low >= Half) {
                value -= Half;
                low -= Half;
                high -= Half;
            }
            else if (low >= First_qtr && high < Third_qtr) {
                value -= First_qtr;
                low -= First_qtr;
                high -= First_qtr;
            }
            else break;
            low = 2 * low;
            high = 2 * high + 1;
            int tmp = input_bit();
            value = 2 * value + tmp;
        }
        return symbol;
    }
    void decoding() {
        start_decoding();
        for (;;) {
            int symbol;
            bool f = false;
            std::string tmp;

            for (auto j = 0; j <= context.size(); j++) {
                tmp = context.substr(j, context.size());
                if (model.find(tmp) == model.end()) {
                    model[tmp] = n_context;
                }
                symbol = decode_symbol(model[tmp]);
                if (symbol != esc) {
                    break;
                }
            }
            if (symbol == esc) {
                symbol = decode_symbol(base);
                update_base(symbol);
            }

            
            //Обновляем модели
            for (int i = tmp.size(); i <= context.size(); i++) {
                update_model(symbol, model[context.substr(context.size() - i, i)]);
            }
            //Записываем символ
            int buff = (symbol - 1);
            data.push_back(buff);
            if (data.size() == len) {
                //std::cout << " readbits " << readbits << " ";
                break;
            }
            //Сдвигаем контекст
            context += (char)buff;
            if (context.size() > n_of_context) {
                context.erase(0, 1);
            }
        }

    }
};
