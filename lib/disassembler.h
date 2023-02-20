//
// Created by KarlE on 2/11/2023.
//

#ifndef CPU8080_DISASSEMBLER_H
#define CPU8080_DISASSEMBLER_H
#include <fstream>
#include <iostream>
#include <vector>

#include "types.h"

class Disassembler {
public:
    std::vector<Byte> getBytesFromFile(std::string const& filename);
    void disassembleFile(std::string const& filename);
    int disassembleOp(uint16_t pc, std::vector<Byte> const& ops) const;


private:
    std::ifstream stream_;
    uint32_t pc_;

};


#endif //CPU8080_DISASSEMBLER_H
