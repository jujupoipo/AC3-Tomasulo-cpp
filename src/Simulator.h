#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <vector>
#include <string>
#include <map>

#include "Config.h"
#include "Instruction.h"
#include "Hardware.h"

// ============================================================
// Simulator.h - Declaração da classe principal do simulador
// ============================================================

class Simulator {
public:
    // Construtor: recebe o vetor de instruções já parseadas
    Simulator(const std::vector<Instruction>& instructions);

    // Executa a simulação completa, imprimindo o estado a cada ciclo
    void run();

private:
    // ---- Estado do Simulador ----
    int clock_;                                     // Ciclo de clock atual
    int nextIssue_;                                 // Índice da próxima instrução a ser despachada
    std::vector<Instruction> instructions_;         // Lista de todas as instruções
    std::vector<ReservationStation> reservStations_; // Todas as estações de reserva
    std::map<std::string, RegisterStatus> regStatus_; // Register Alias Table (F0..F31)
    std::map<std::string, double> regValues_;        // Valores reais atuais dos registradores

    // ---- Inicialização ----
    void initReservationStations();
    void initRegisters();

    // ---- Estágios do Pipeline (processados de trás para frente) ----
    void processWriteResult();
    void processExecution();
    void processIssue();

    // ---- Utilitários ----
    int findFreeRS(const std::string& op) const;
    int getLatency(const std::string& op) const;
    bool isLoadStore(const std::string& op) const;
    bool allInstructionsDone() const;

    // ---- Impressão ----
    void printInstructionStatus() const;
    void printReservationStations() const;
    void printRegisterStatus() const;
    void printState() const;
};

#endif // SIMULATOR_H
