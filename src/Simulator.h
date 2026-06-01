#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <vector>
#include <string>
#include <map>

#include "Config.h"
#include "Instruction.h"
#include "Hardware.h"

// simulator.h - declaração da classe principal do simulador

class Simulator {
public:
    // construtor: recebe o vetor de instruções já parseadas
    Simulator(const std::vector<Instruction>& instructions);

    // executa a simulação completa, imprimindo o estado a cada ciclo
    void run();

private:
    // estado do simulador
    int clock_;                                     // Ciclo de clock atual
    int nextIssue_;                                 // Índice da próxima instrução a ser despachada
    std::vector<Instruction> instructions_;         // lista de todas as instruções
    std::vector<ReservationStation> reservStations_; // todas as estações de reserva
    std::map<std::string, RegisterStatus> regStatus_; // register alias table (f0..f31)
    std::map<std::string, double> regValues_;        // valores reais atuais dos registradores

    // inicialização
    void initReservationStations();
    void initRegisters();

    // estágios do pipeline (processados de trás para frente)
    void processWriteResult();
    void processExecution();
    void processIssue();

    // utilitários
    int findFreeRS(const std::string& op) const;
    int getLatency(const std::string& op) const;
    bool isLoadStore(const std::string& op) const;
    bool allInstructionsDone() const;

    // impressão
    void printInstructionStatus() const;
    void printReservationStations() const;
    void printRegisterStatus() const;
    void printState() const;
};

#endif // SIMULATOR_H
