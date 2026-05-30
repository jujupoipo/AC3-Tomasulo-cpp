#include "Simulator.h"

#include <iostream>
#include <iomanip>
#include <cmath>

// ============================================================
// Simulator.cpp - Motor do Algoritmo de Tomasulo
// ============================================================

// ---- Construtor ----
Simulator::Simulator(const std::vector<Instruction>& instructions)
    : clock_(0), nextIssue_(0), instructions_(instructions)
{
    initReservationStations();
    initRegisters();
}

// ---- Inicialização das Estações de Reserva ----
void Simulator::initReservationStations() {
    // Estações de Add/Sub
    for (int i = 1; i <= NUM_ADD_RS; ++i) {
        ReservationStation rs;
        rs.name = "Add" + std::to_string(i);
        reservStations_.push_back(rs);
    }
    // Estações de Mult/Div
    for (int i = 1; i <= NUM_MULT_RS; ++i) {
        ReservationStation rs;
        rs.name = "Mult" + std::to_string(i);
        reservStations_.push_back(rs);
    }
    // Buffers de Load
    for (int i = 1; i <= NUM_LOAD_BUFFERS; ++i) {
        ReservationStation rs;
        rs.name = "Load" + std::to_string(i);
        reservStations_.push_back(rs);
    }
    // Buffers de Store
    for (int i = 1; i <= NUM_STORE_BUFFERS; ++i) {
        ReservationStation rs;
        rs.name = "Store" + std::to_string(i);
        reservStations_.push_back(rs);
    }
}

// ---- Inicialização dos Registradores ----
void Simulator::initRegisters() {
    for (int i = 0; i < NUM_FP_REGISTERS; ++i) {
        std::string regName = "F" + std::to_string(i);
        regStatus_[regName] = RegisterStatus();
        // Valor inicial arbitrário para demonstração
        // (registradores pares recebem valor baseado no índice)
        regValues_[regName] = static_cast<double>(i);
    }
    // Registradores inteiros (usados como base em Load/Store)
    for (int i = 0; i < 32; ++i) {
        std::string regName = "R" + std::to_string(i);
        regStatus_[regName] = RegisterStatus();
        regValues_[regName] = 0.0;
    }
}

// ============================================================
// Determina a latência baseado no tipo de operação
// ============================================================
int Simulator::getLatency(const std::string& op) const {
    if (op == "ADD.D" || op == "SUB.D") return LATENCY_ADD;
    if (op == "MUL.D")                  return LATENCY_MULT;
    if (op == "DIV.D")                  return LATENCY_DIV;
    if (op == "L.D")                    return LATENCY_LOAD;
    if (op == "S.D")                    return LATENCY_STORE;
    return 1; // Fallback
}

// ============================================================
// Verifica se a operação é Load ou Store
// ============================================================
bool Simulator::isLoadStore(const std::string& op) const {
    return (op == "L.D" || op == "S.D");
}

// ============================================================
// Encontra uma RS livre para o tipo de operação desejado
// Retorna o índice no vetor reservStations_, ou -1 se não há.
// ============================================================
int Simulator::findFreeRS(const std::string& op) const {
    std::string prefix;
    if (op == "ADD.D" || op == "SUB.D") prefix = "Add";
    else if (op == "MUL.D" || op == "DIV.D") prefix = "Mult";
    else if (op == "L.D") prefix = "Load";
    else if (op == "S.D") prefix = "Store";
    else return -1;

    for (size_t i = 0; i < reservStations_.size(); ++i) {
        if (!reservStations_[i].busy && reservStations_[i].name.find(prefix) == 0) {
            return static_cast<int>(i);
        }
    }
    return -1; // Todas ocupadas — stall estrutural
}

// ============================================================
// Verifica se todas as instruções já terminaram o Write Result
// ============================================================
bool Simulator::allInstructionsDone() const {
    for (const auto& instr : instructions_) {
        if (instr.writeResultCycle == 0) return false;
    }
    return true;
}

// ############################################################
//  ESTÁGIO 1: ISSUE
// ############################################################
// Despacha a próxima instrução da fila (em ordem de programa)
// para uma estação de reserva livre do tipo correto.
// Se não há RS livre, ocorre stall (a instrução espera).
// ############################################################
void Simulator::processIssue() {
    if (nextIssue_ >= static_cast<int>(instructions_.size())) return;

    Instruction& instr = instructions_[nextIssue_];
    int rsIdx = findFreeRS(instr.op);

    if (rsIdx == -1) return; // Stall estrutural: sem RS livre

    ReservationStation& rs = reservStations_[rsIdx];
    rs.busy = true;
    rs.op = instr.op;
    rs.instrIndex = nextIssue_;

    if (instr.op == "L.D") {
        // L.D Fdest, offset, Rbase
        // src1 = offset (imediato), src2 = registrador base
        rs.A = std::stod(instr.src1);
        // Endereço efetivo = offset + valor do registrador base
        if (regStatus_.count(instr.src2) && regStatus_[instr.src2].Qi != "") {
            rs.Qj = regStatus_[instr.src2].Qi;
        } else {
            rs.Vj = regValues_[instr.src2];
            rs.Qj = "";
        }
        rs.Qk = "";
        rs.Vk = 0.0;
    } else if (instr.op == "S.D") {
        // S.D Fsrc, offset, Rbase
        // dest = registrador fonte (valor a ser armazenado)
        // src1 = offset, src2 = registrador base
        rs.A = std::stod(instr.src1);
        // Verifica dependência do registrador que contém o valor a armazenar
        if (regStatus_.count(instr.dest) && regStatus_[instr.dest].Qi != "") {
            rs.Qj = regStatus_[instr.dest].Qi;
        } else {
            rs.Vj = regValues_[instr.dest];
            rs.Qj = "";
        }
        // Verifica dependência do registrador base
        if (regStatus_.count(instr.src2) && regStatus_[instr.src2].Qi != "") {
            rs.Qk = regStatus_[instr.src2].Qi;
        } else {
            rs.Vk = regValues_[instr.src2];
            rs.Qk = "";
        }
    } else {
        // Instruções aritméticas: ADD.D, SUB.D, MUL.D, DIV.D
        // Operando fonte 1 (src1)
        if (regStatus_.count(instr.src1) && regStatus_[instr.src1].Qi != "") {
            rs.Qj = regStatus_[instr.src1].Qi;
        } else {
            rs.Vj = regValues_[instr.src1];
            rs.Qj = "";
        }
        // Operando fonte 2 (src2)
        if (regStatus_.count(instr.src2) && regStatus_[instr.src2].Qi != "") {
            rs.Qk = regStatus_[instr.src2].Qi;
        } else {
            rs.Vk = regValues_[instr.src2];
            rs.Qk = "";
        }
    }

    // Atualiza o Register Status (RAT) — apenas para instruções que escrevem
    // em registrador (Load e aritméticas). Store NÃO escreve em registrador.
    if (instr.op != "S.D") {
        regStatus_[instr.dest].Qi = rs.name;
    }

    // Registra o ciclo de Issue
    instr.issueCycle = clock_;
    instr.rsIndex = rsIdx;
    nextIssue_++;
}

// ############################################################
//  ESTÁGIO 2: EXECUTE
// ############################################################
// Para cada RS ocupada cujos operandos estejam prontos
// (Qj == "" e Qk == ""), inicia ou continua a execução.
// Quando os ciclos restantes chegam a 0, a instrução está
// pronta para o Write Result no próximo ciclo.
// ############################################################
void Simulator::processExecution() {
    for (auto& rs : reservStations_) {
        if (!rs.busy) continue;
        if (rs.instrIndex < 0) continue;

        Instruction& instr = instructions_[rs.instrIndex];

        // Pula instruções que já completaram Write Result
        if (instr.writeResultCycle != 0) continue;

        // Só pode executar se NÃO foi issue'd neste mesmo ciclo
        if (instr.issueCycle == clock_) continue;

        // Verifica se os operandos estão prontos
        if (rs.Qj != "" || rs.Qk != "") continue;

        // Primeira vez executando? Marca o início
        if (!rs.executing) {
            rs.executing = true;
            rs.cyclesRemaining = getLatency(rs.op);
            instr.execStartCycle = clock_;
        }

        // Decrementa ciclos restantes
        rs.cyclesRemaining--;

        // Se terminou de executar, calcula o resultado
        if (rs.cyclesRemaining == 0) {
            instr.execEndCycle = clock_;

            // Calcula o valor do resultado
            if (rs.op == "ADD.D") {
                rs.result = rs.Vj + rs.Vk;
            } else if (rs.op == "SUB.D") {
                rs.result = rs.Vj - rs.Vk;
            } else if (rs.op == "MUL.D") {
                rs.result = rs.Vj * rs.Vk;
            } else if (rs.op == "DIV.D") {
                rs.result = (rs.Vk != 0.0) ? (rs.Vj / rs.Vk) : 0.0;
            } else if (rs.op == "L.D") {
                // Simula um acesso à memória: endereço efetivo = A + Vj (base)
                // Para simulação, retornamos o endereço como valor simbólico
                rs.result = rs.A + rs.Vj;
            } else if (rs.op == "S.D") {
                // Store: o resultado é o endereço de escrita (não escreve em registrador)
                rs.result = rs.A + rs.Vk;
            }
        }
    }
}

// ############################################################
//  ESTÁGIO 3: WRITE RESULT (Broadcast no CDB)
// ############################################################
// Se uma RS terminou sua execução (cyclesRemaining == 0 e
// executing == true), ela publica seu resultado no CDB:
//   - Atualiza todas as RS que esperavam por esse resultado
//     (substitui Qj/Qk pelo valor Vj/Vk).
//   - Atualiza o Register Status e o valor do registrador.
//   - Libera a RS (busy = false).
// ############################################################
void Simulator::processWriteResult() {
    int writesThisCycle = 0;

    for (auto& rs : reservStations_) {
        if (!rs.busy) continue;
        if (!rs.executing) continue;
        if (rs.cyclesRemaining != 0) continue;
        if (rs.instrIndex < 0) continue;

        Instruction& instr = instructions_[rs.instrIndex];

        // Não pode fazer Write Result no mesmo ciclo que terminou a execução
        if (instr.execEndCycle == clock_) continue;

        // Já fez Write Result?
        if (instr.writeResultCycle != 0) continue;

        // Marca o ciclo de Write Result
        instr.writeResultCycle = clock_;

        // ---- Broadcast no CDB ----
        // Percorre todas as RS e substitui dependências
        for (auto& otherRS : reservStations_) {
            if (!otherRS.busy) continue;
            if (otherRS.Qj == rs.name) {
                otherRS.Vj = rs.result;
                otherRS.Qj = "";
            }
            if (otherRS.Qk == rs.name) {
                otherRS.Vk = rs.result;
                otherRS.Qk = "";
            }
        }

        // Atualiza Register Status e valor do registrador
        // (somente se o RAT ainda aponta para esta RS — proteção contra WAW)
        if (instr.op != "S.D") {
            if (regStatus_[instr.dest].Qi == rs.name) {
                regStatus_[instr.dest].Qi = "";
                regValues_[instr.dest] = rs.result;
            }
        }

        // Libera a estação de reserva
        rs.busy = false;
        rs.executing = false;
        rs.op = "";
        rs.Vj = 0.0;
        rs.Vk = 0.0;
        rs.Qj = "";
        rs.Qk = "";
        rs.A = 0.0;
        rs.result = 0.0;
        rs.instrIndex = -1;
        rs.cyclesRemaining = 0;

        // Limita o número de Write Results por ciclo ao número de CDBs
        writesThisCycle++;
        if (writesThisCycle >= NUM_CDB) break;
    }
}

// ############################################################
//  Loop principal da simulação
// ############################################################
void Simulator::run() {
    std::cout << "============================================================" << std::endl;
    std::cout << "   SIMULADOR DO ALGORITMO DE TOMASULO" << std::endl;
    std::cout << "============================================================" << std::endl;
    std::cout << "Instruções carregadas: " << instructions_.size() << std::endl;
    std::cout << "Pressione ENTER para avançar ciclo a ciclo..." << std::endl;
    std::cout << "============================================================\n" << std::endl;

    // Imprime estado inicial (ciclo 0)
    printState();

    while (!allInstructionsDone()) {
        std::cin.get(); // Espera o usuário pressionar ENTER

        clock_++;

        // Processa os estágios de trás para frente para evitar
        // que uma instrução avance mais de um estágio por ciclo.
        processWriteResult();
        processExecution();
        processIssue();

        printState();
    }

    // Impressão final: valores dos registradores
    std::cout << "\n============================================================" << std::endl;
    std::cout << "   SIMULAÇÃO CONCLUÍDA" << std::endl;
    std::cout << "============================================================" << std::endl;
    std::cout << "\n--- Valores Finais dos Registradores ---\n" << std::endl;

    // Coleta os registradores utilizados nas instruções
    std::vector<std::string> usedRegs;
    for (const auto& instr : instructions_) {
        auto addIfNew = [&](const std::string& r) {
            if (r.empty()) return;
            if (r[0] != 'F' && r[0] != 'R') return;
            for (const auto& u : usedRegs)
                if (u == r) return;
            usedRegs.push_back(r);
        };
        addIfNew(instr.dest);
        addIfNew(instr.src1);
        addIfNew(instr.src2);
    }

    for (const auto& reg : usedRegs) {
        std::cout << "  " << std::setw(4) << reg << " = "
                  << std::fixed << std::setprecision(2) << regValues_[reg] << std::endl;
    }
    std::cout << std::endl;
}

// ############################################################
//  IMPRESSÃO DO ESTADO (Tabelas como nos slides)
// ############################################################

void Simulator::printInstructionStatus() const {
    std::cout << "┌─────────────────────────────────────────────────────────────────────────┐" << std::endl;
    std::cout << "│                        STATUS DAS INSTRUÇÕES                            │" << std::endl;
    std::cout << "├─────┬────────┬──────┬──────┬──────┬─────────┬────────────┬──────────────┤" << std::endl;
    std::cout << "│  #  │  Op    │ Dest │ Src1 │ Src2 │  Issue  │  Exec Compl│ Write Result │" << std::endl;
    std::cout << "├─────┼────────┼──────┼──────┼──────┼─────────┼────────────┼──────────────┤" << std::endl;

    for (size_t i = 0; i < instructions_.size(); ++i) {
        const auto& instr = instructions_[i];

        auto fmtCycle = [](int c) -> std::string {
            if (c == 0) return "   ";
            return std::to_string(c);
        };

        std::string execStr;
        if (instr.execStartCycle == 0) {
            execStr = "          ";
        } else if (instr.execEndCycle == 0) {
            execStr = std::to_string(instr.execStartCycle) + "- ...    ";
        } else {
            execStr = std::to_string(instr.execStartCycle) + "-" + std::to_string(instr.execEndCycle);
        }

        std::cout << "│ " << std::setw(3) << (i + 1) << " │ "
                  << std::setw(6) << std::left << instr.op << " │ "
                  << std::setw(4) << instr.dest << " │ "
                  << std::setw(4) << instr.src1 << " │ "
                  << std::setw(4) << instr.src2 << " │ "
                  << std::setw(7) << std::right << fmtCycle(instr.issueCycle) << " │ "
                  << std::setw(10) << execStr << " │ "
                  << std::setw(12) << fmtCycle(instr.writeResultCycle) << " │"
                  << std::endl;
    }
    std::cout << "└─────┴────────┴──────┴──────┴──────┴─────────┴────────────┴──────────────┘" << std::endl;
}

void Simulator::printReservationStations() const {
    std::cout << "┌───────────────────────────────────────────────────────────────────────────────────┐" << std::endl;
    std::cout << "│                           ESTAÇÕES DE RESERVA                                     │" << std::endl;
    std::cout << "├────────┬──────┬────────┬──────────┬──────────┬────────┬────────┬─────────┬────────┤" << std::endl;
    std::cout << "│  Time  │ Name │   Op   │    Vj    │    Vk    │   Qj   │   Qk   │    A    │  Busy  │" << std::endl;
    std::cout << "├────────┼──────┼────────┼──────────┼──────────┼────────┼────────┼─────────┼────────┤" << std::endl;

    for (const auto& rs : reservStations_) {
        std::string timeStr = "";
        if (rs.busy && rs.executing) {
            timeStr = std::to_string(rs.cyclesRemaining);
        }

        std::cout << "│ " << std::setw(6) << timeStr << " │ "
                  << std::setw(4) << rs.name << " │ "
                  << std::setw(6) << std::left << (rs.busy ? rs.op : "") << std::right << " │ ";

        // Vj
        if (rs.busy && rs.Qj == "") {
            std::cout << std::setw(8) << std::fixed << std::setprecision(1) << rs.Vj;
        } else {
            std::cout << std::setw(8) << "";
        }
        std::cout << " │ ";

        // Vk
        if (rs.busy && rs.Qk == "") {
            std::cout << std::setw(8) << std::fixed << std::setprecision(1) << rs.Vk;
        } else {
            std::cout << std::setw(8) << "";
        }
        std::cout << " │ "
                  << std::setw(6) << rs.Qj << " │ "
                  << std::setw(6) << rs.Qk << " │ ";

        // A (endereço)
        if (rs.busy && (rs.op == "L.D" || rs.op == "S.D")) {
            std::cout << std::setw(7) << std::fixed << std::setprecision(0) << rs.A;
        } else {
            std::cout << std::setw(7) << "";
        }

        std::cout << " │ "
                  << std::setw(6) << (rs.busy ? "Sim" : "Não") << " │"
                  << std::endl;
    }
    std::cout << "└────────┴──────┴────────┴──────────┴──────────┴────────┴────────┴─────────┴────────┘" << std::endl;
}

void Simulator::printRegisterStatus() const {
    std::cout << "┌─────────────────────────────────────────────────────────────────────────┐" << std::endl;
    std::cout << "│                        REGISTER STATUS (RAT)                            │" << std::endl;
    std::cout << "├";

    // Coleta registradores FP usados para exibição compacta
    std::vector<std::string> displayRegs;
    for (int i = 0; i < NUM_FP_REGISTERS; i += 2) {
        displayRegs.push_back("F" + std::to_string(i));
    }

    for (size_t i = 0; i < displayRegs.size(); ++i) {
        std::cout << "────────";
        if (i < displayRegs.size() - 1) std::cout << "┬";
    }
    std::cout << "┤" << std::endl;

    // Nomes dos registradores
    std::cout << "│";
    for (const auto& reg : displayRegs) {
        std::cout << std::setw(7) << reg << " │";
    }
    std::cout << std::endl;

    // Separador
    std::cout << "├";
    for (size_t i = 0; i < displayRegs.size(); ++i) {
        std::cout << "────────";
        if (i < displayRegs.size() - 1) std::cout << "┼";
    }
    std::cout << "┤" << std::endl;

    // Valores (Qi)
    std::cout << "│";
    for (const auto& reg : displayRegs) {
        auto it = regStatus_.find(reg);
        std::string qi = "";
        if (it != regStatus_.end()) {
            qi = it->second.Qi;
        }
        std::cout << std::setw(7) << qi << " │";
    }
    std::cout << std::endl;

    std::cout << "└";
    for (size_t i = 0; i < displayRegs.size(); ++i) {
        std::cout << "────────";
        if (i < displayRegs.size() - 1) std::cout << "┴";
    }
    std::cout << "┘" << std::endl;
}

void Simulator::printState() const {
    std::cout << "\n══════════════════════════════════════════════════════════════" << std::endl;
    std::cout << "  CICLO DE CLOCK: " << clock_ << std::endl;
    std::cout << "══════════════════════════════════════════════════════════════\n" << std::endl;

    printInstructionStatus();
    std::cout << std::endl;
    printReservationStations();
    std::cout << std::endl;
    printRegisterStatus();
}
