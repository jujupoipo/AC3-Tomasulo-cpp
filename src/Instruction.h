#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>

// ============================================================
// Instruction.h - Representação de uma instrução MIPS FP
// ============================================================

struct Instruction {
    std::string op;    // Operação: ADD.D, SUB.D, MUL.D, DIV.D, L.D, S.D
    std::string dest;  // Registrador destino (ex: F6)
    std::string src1;  // Fonte 1 (registrador ou offset para Load/Store)
    std::string src2;  // Fonte 2 (registrador ou registrador base para Load/Store)

    // Ciclos de clock registrados durante a simulação
    int issueCycle      = 0;
    int execStartCycle  = 0;
    int execEndCycle    = 0;
    int writeResultCycle = 0;

    // Índice da estação de reserva atribuída
    int rsIndex = -1;
};

// ============================================================
// Função auxiliar para remover vírgulas e espaços extras
// ============================================================
inline std::string cleanToken(const std::string& token) {
    std::string cleaned = token;
    // Remove vírgulas
    cleaned.erase(std::remove(cleaned.begin(), cleaned.end(), ','), cleaned.end());
    // Trim de espaços
    while (!cleaned.empty() && cleaned.front() == ' ') cleaned.erase(cleaned.begin());
    while (!cleaned.empty() && cleaned.back() == ' ') cleaned.pop_back();
    return cleaned;
}

// ============================================================
// Leitura do arquivo de instruções
// Formato esperado por linha:
//   ADD.D F6, F8, F2
//   L.D F6, 32, R2       (offset, registrador base)
//   S.D F6, 32, R3
// Aceita vírgulas e espaços como separadores.
// ============================================================
inline std::vector<Instruction> loadInstructions(const std::string& filepath) {
    std::vector<Instruction> instructions;
    std::ifstream file(filepath);

    if (!file.is_open()) {
        std::cerr << "Erro: Não foi possível abrir o arquivo '" << filepath << "'." << std::endl;
        return instructions;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Ignora linhas vazias e comentários
        if (line.empty() || line[0] == '#') continue;

        // Substitui vírgulas por espaços para um parsing uniforme
        std::replace(line.begin(), line.end(), ',', ' ');

        std::istringstream iss(line);
        Instruction instr;
        std::string token;

        // Lê operação
        if (!(iss >> token)) continue;
        instr.op = token;

        // Lê destino
        if (!(iss >> token)) continue;
        instr.dest = cleanToken(token);

        // Lê fonte 1
        if (!(iss >> token)) continue;
        instr.src1 = cleanToken(token);

        // Lê fonte 2 (pode não existir em algumas variações,
        // mas no formato do Hennessy sempre há)
        if (iss >> token) {
            instr.src2 = cleanToken(token);
        }

        instructions.push_back(instr);
    }

    file.close();
    return instructions;
}

#endif // INSTRUCTION_H
