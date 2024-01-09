#pragma once
#include "ppm.h"

struct JPGencoder {
	int width, height, quality;
	std::ofstream out;
    const std::vector<int> acZigZag = { 0, 1, 8, 16, 9, 2, 3, 10, 17, 24, 32, 25, 18,
        11, 4, 5, 12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13, 6, 7, 14, 21, 28, 35,
        42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51, 58, 59, 52, 45,
        38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63 };

    std::vector<int> get_data() {
        auto dctdump = std::ifstream("dctdump", std::ios::binary);
        int symbol;
        auto data = std::vector<int>();
        while (dctdump >> symbol) {
            data.push_back(symbol);
        }
        dctdump.close();
        remove("dctdump");
        return data;
    }
    std::vector<int> get_header() {
        auto headers = std::ifstream("headers", std::ios::binary);
        std::vector<int> header(3);
        for (auto i = 0; i < header.size(); i++) {
            headers >> header[i];
        }
        headers.close();
        remove("headers");
        return header;
    }
    std::string get_bits(int value) {
        std::string bits = std::bitset<8>(abs(value)).to_string();
        bits = bits.erase(0, bits.find_first_not_of('0'));
        if (value < 0) {
            for (auto i = 0; i < bits.size(); i++) {
                if (bits[i] == '1') {
                    bits[i] = '0';
                }
                else {
                    bits[i] = '1';
                }
            }
        }
        return bits;
    }
    void put_bits(std::string& bitstream) {
        auto it = bitstream.begin();
        for (it; bitstream.end() - it > 8; it += 8)
            out << (uint8_t)std::bitset<8>(std::string(it, it + 8)).to_ulong();
        if (it != bitstream.end()) {
            auto s = std::string(it, bitstream.end());
            while (s.size() < 8) {
                s += '0';
            }
            out << (uint8_t)std::bitset<8>(std::string(s)).to_ulong();
        }
        bitstream.clear();
    }
    void encodeHeader(int w, int h, int q) {
        uint16_t wshort = static_cast<uint16_t>(w);
        out.write((char*)&wshort, sizeof(wshort));
        uint16_t wheight = static_cast<uint16_t>(h);
        out.write((char*)&wheight, sizeof(wheight));
        uint8_t wquality = static_cast<uint8_t>(q);
        out << wquality;
    }
    void encodeDC(std::vector<int>& DC) {
        std::vector<int> DClen;
        std::string bitstream = "";
        for (auto dc : DC) {
            auto bits = get_bits(dc);
            DClen.push_back(bits.size());
            bitstream += bits;
        }

        auto ENC = Encoder(9, 2, DClen);
        ENC.encoding();
        std::string stream = ENC.outstream;
        int size = DC.size();
        out.write((char*)&size, 4);
        for (auto ch : stream) {
            out << ch;
        }
        int tmp = 255;
        out.write((char*)&tmp, 1);
        out.write((char*)&tmp, 1);

 
        put_bits(bitstream);

    }
    void encodeAC(std::vector<int>& AC) {
        std::vector<int> rlvl;
        std::string bitstream = "";
        uint8_t eob = 64;
        //RLE+преобразование к битовому
        for (auto i = 0; i < AC.size() / 63; i++) {
            uint8_t count = 0;
            for (auto j = 0; j < 63; j++) {
                if (AC[i * 63 + j] == 0) {
                    count++;
                }
                else {
                    auto bits = get_bits(AC[i * 63 + j]);

                    rlvl.push_back(count);
                    uint8_t bitsize = bits.size();
                    rlvl.push_back(bitsize);
                    bitstream += bits;
                    count = 0;
                }
            }
            rlvl.push_back(eob);
        }

        auto ENC = Encoder(65, 3, rlvl);
        ENC.encoding();
        std::string stream = ENC.outstream;
        int size = rlvl.size();
        out.write((char*)&size, 4);
        for (auto ch : stream) {
            out << ch;
        }
        int tmp = 255;
        out.write((char*)&tmp, 1);
        out.write((char*)&tmp, 1);

        put_bits(bitstream);
    }
	void start() {
		auto data = get_data();
		auto header = get_header();
        //разделение на DC AC
        std::vector<int> DC, AC;
        for (int i = 0; i < data.size(); ++i) {
            if (i % 64 == 0) {
                DC.push_back(data[i]);
            }
            else {
                AC.push_back(data[64 * (i / 64) + acZigZag[i % 64]]);
            }
        }



        std::string outname;
        std::cout << "Enter output file name: ";
        std::cin >> outname;
        out.open(outname, std::ios::binary);
        encodeHeader(header[0], header[1], header[2]);
        encodeDC(DC);
        encodeAC(AC);
        out.close();
	}

};