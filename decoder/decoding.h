#pragma once
#include "ppm.h"

struct JPGdecoder {
	int width, height, quality, dct_size;
    
	std::ofstream out;
    std::ifstream inp;
	const std::vector<int> reverseACZigZag = { 0,1,5,6,14,15,27,28,2,4,7,13,16,26,29,
		42,3,8,12,17,25,30,41,43,9,11,18,24,31,40,44,53,10,19,23,32,39,45,52,54,20,22,
		33,38,46,51,55,60,21,34,37,47,50,56,59,61,35,36,48,49,57,58,62,63 };
	
	void decodeHeaders() {
        uint16_t buf;
        inp.read((char*)&buf, sizeof(buf));
        width = (int)buf;
        inp.read((char*)&buf, sizeof(buf));
        height = (int)buf;
        uint8_t qbuf;
        inp.read((char*)&qbuf, sizeof(qbuf));
        quality = (int)qbuf;

        auto headers = std::ofstream("headers", std::ios::binary);
        headers << width << " ";
        headers << height << " ";
        headers << quality;
        headers.close();
	}

    int get_value(std::string bits) {
        int value;
        if (bits[0] == '0') {
            //flip
            for (auto i = 0; i < bits.size(); ++i) {
                if (bits[i] == '0') {
                    bits[i] = '1';
                }
                else {
                    bits[i] = '0';
                }
            }
            value = -(int)std::bitset<8>(bits).to_ulong();
        }
        else {
            value = (int)std::bitset<8>(bits).to_ulong();
        }
        return value;
    }

    std::vector<int> decodeDC() {
        int len;
        inp.read((char*)&len, 4);
        dct_size = len;
        //Декодирование длин
        auto DEC = Decoder(9, 2, len, &inp);
        DEC.decoding();
        auto DC_len = DEC.data;

        //Вычисление длины битового потока коэффициентов
        int bitslen = 0;
        for (auto i : DC_len) {
            bitslen += i;
        }

        //Чтение потока
        std::string bitstream = "";
        uint8_t ch;
        inp.read((char*)&ch, 1);
        int i = 0;
        if (std::bitset<8>(ch).to_ulong()!=255) {
            bitstream += std::bitset<8>(ch).to_string();
            i++;
        }
        for (i; i < ((bitslen + 8 - 1) / 8); i++) {

            inp.read((char*)&ch, 1);
            bitstream += std::bitset<8>(ch).to_string();
        }
        //Декодирование коэффициентов
        std::vector<int> DC;
        auto it = bitstream.begin();
        for (auto size : DC_len) {
            auto bits = std::string(it, it + size);
            auto dc = get_value(bits);
            it += size;
            DC.push_back(dc);
        }
        return DC;
    }
    std::vector<int> decodeAC() {
        int len;
        inp.read((char*)&len, 4);
        auto DEC = Decoder(65, 3, len, &inp);
        DEC.decoding();
        auto rlvl = DEC.data;

        std::vector<int>level, run;
        int i = 0;
        while (i < rlvl.size()) {
            auto a = rlvl[i];
            if (a == 64) {
                run.push_back(a);
                i++;
            }
            else {
                run.push_back(a);
                i++;
                level.push_back(rlvl[i]);
                i++;
            }
        }

        //Вычисление длины битового потока коэффициентов
        int bitslen = 0;
        for (auto i : level) {
            bitslen += i;
        }

        //Чтение потока
        std::string bitstream = "";
        uint8_t ch;
        inp.read((char*)&ch, 1);
        int j = 0;
        if (std::bitset<8>(ch).to_ulong() != 255) {
            bitstream += std::bitset<8>(ch).to_string();
            j++;
        }
        for (j; j < ((bitslen + 8 - 1) / 8); j++) {

            inp.read((char*)&ch, 1);
            bitstream += std::bitset<8>(ch).to_string();
        }
        std::vector<int> AC, vlevel;

        auto it = bitstream.begin();
        for (auto size : level) {
            if (size == 0) {
                continue;
            }
            auto bits = std::string(it, it + size);
            auto dc = get_value(bits);
            it += size;
            vlevel.push_back(dc);
        }

        std::vector<int> block;
        int blocksize = 63;
        auto lit = vlevel.begin();
        for (auto i = 0; i < run.size(); i++) {
            if (run[i] == 64) {
                int tmp = blocksize;
                for (auto k = 0; k < tmp; k++) {
                    AC.push_back(0);
                    blocksize--;
                }
                blocksize = 63;
            }
            else {
                for (auto k = 0; k < run[i]; k++) {
                    AC.push_back(0);
                    blocksize--;
                }
                AC.push_back(*lit);
                lit++;
                blocksize--;
            }
        }

        return AC;
    }
    void start() {
        std::string inpname;
        std::cout << "Enter input file name: ";
        std::cin >> inpname;
        inp.open(inpname, std::ios::binary);

        decodeHeaders();


        auto DC = decodeDC();

        auto AC = decodeAC();
        
        dct_size = dct_size * 64;


        auto dct = std::ofstream("dctDump", std::ios::binary);
        for (int i = 0; i < dct_size; ++i)
        {
            if (i % 64 == 0)
                dct << DC[i / 64] << " ";
            else
                dct << AC[63 * (i / 64) + reverseACZigZag[i % 64] - 1] << " ";
        }
        dct.close();

        std::string outname;
        std::cout << "Enter output jpg file name: ";
        std::cin >> outname;
        EncodeJPEG(outname);
        remove("headers");
        remove("dctDump");
        std::cout << "Done" << std::endl;



        inp.close();

    }

};