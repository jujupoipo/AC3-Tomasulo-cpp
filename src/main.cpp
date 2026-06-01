#include <iostream>
#include <string>

#include "Simulator.h"

// main.cpp - Ponto de entrada do simulador do Tomasulo
// Uso: ./tomasulo [caminho_do_arquivo_de_instruções]
// Se nenhum caminho for informado, usa "instructions.txt".

int main(int argc, char* argv[]) {
    std::string filepath = "instructions.txt";

    if (argc > 1) {
        filepath = argv[1];
    }

    // Carrega as instruções do arquivo
    std::vector<Instruction> instructions = loadInstructions(filepath);

    if (instructions.empty()) {
        std::cerr << "Nenhuma instrução carregada. Verifique o arquivo de entrada." << std::endl;
        return 1;
    }

    std::cout << "Arquivo carregado: " << filepath << std::endl;
    std::cout << "Total de instruções: " << instructions.size() << std::endl;

    // Cria e executa o simulador
    Simulator sim(instructions);
    sim.run();

    return 0;
}
