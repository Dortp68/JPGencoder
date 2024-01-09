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

struct Encoder : Model {
    int n, bits_to_go, buff;
    code_value low, high, bits_to_follow;
    int n_of_context;
    std::string context;
    std::string outstream;
    std::vector<int> data;
    Encoder(int n, int n_of_context, std::vector<int> data) :n(n), n_of_context(n_of_context), Model(n) {
        low = 0;
        high = Top_value;
        bits_to_follow = 0;
        bits_to_go = 8;
        buff = 0;
        context = "";
        outstream.clear();
        this->data = data;
    }
    ~Encoder() = default;
    void output_bit(bool bit) {
        buff >>= 1;
        if (bit) buff |= 0x80;
        bits_to_go--;
        if (bits_to_go == 0) {
            outstream += (char)buff;
            bits_to_go = 8;
        }
    }
    void done_outputing() {
        buff >>= bits_to_go;
        outstream += (char)buff;
    }
    void bit_plus_follow(bool bit)
    {
        output_bit(bit);
        while (bits_to_follow > 0) {
            output_bit(!bit);
            bits_to_follow--;
        }
    }
    void done_encoding() {
        bits_to_follow += 1;
        if (low < First_qtr) bit_plus_follow(0);
        else bit_plus_follow(1);
    }
    void encode_symbol(int symbol, const std::vector<int>& cum_freq) {
        code_value range = high - low + 1;
        high = low + (range * cum_freq[symbol - 1]) / cum_freq[0] - 1;
        low = low + (range * cum_freq[symbol]) / cum_freq[0];
        for (;;) {
            if (high < Half) {
                bit_plus_follow(0);
            }
            else if (low >= Half) {
                bit_plus_follow(1);
                low -= Half;
                high -= Half;
            }
            else if (low >= First_qtr && high < Third_qtr) {
                bits_to_follow += 1;
                low -= First_qtr;
                high -= First_qtr;
            }
            else break;
            low = 2 * low;
            high = 2 * high + 1;
        }
    }
    void encoding() {
        for (auto i=0;i<data.size();i++) {
            //fread((char*)&ch, 1, 1, If);
            int symbol = data[i] + 1;
            //if (feof(If)) break;
            bool f = false;
            for (auto j = 0; j <= context.size(); j++) {
                auto tmp = context.substr(j, context.size());
                if (model.find(tmp) == model.end()) {
                    model[tmp] = n_context;
                }
                if ((model[tmp][symbol - 1] - model[tmp][symbol]) == 0) {
                    encode_symbol(esc, model[tmp]);
                }
                else {
                    encode_symbol(symbol, model[tmp]);
                    f = true;
                }
                update_model(symbol, model[tmp]);
                if (f) break;
            }
            if (!f) {
                encode_symbol(symbol, base);
                update_base(symbol);
            }
            //сдвиг контекста
            context += (char)(symbol - 1);
            if (context.size() > n_of_context) {
                context.erase(0, 1);
            }

        }
        done_encoding();
        done_outputing();
    }
};
