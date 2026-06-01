#include "Simulator.h"

#include <iostream>
#include <iomanip>
#include <cmath>

// simulator.cpp - motor do algoritmo de tomasulo

// construtor
Simulator::Simulator(const std::vector<Instruction>& instructions)
    : clock_(0), nextIssue_(0), instructions_(instructions)
{
    initReservationStations();
    initRegisters();
}

// inicialização das estações de reserva
void Simulator::initReservationStations() {
    // estações de add/sub
    for (int i = 1; i <= NUM_ADD_RS; ++i) {
        ReservationStation rs;
        rs.name = "Add" + std::to_string(i);
        reservStations_.push_back(rs);
    }
    // estações de mult/div
    for (int i = 1; i <= NUM_MULT_RS; ++i) {
        ReservationStation rs;
        rs.name = "Mult" + std::to_string(i);
        reservStations_.push_back(rs);
    }
    // buffers de load
    for (int i = 1; i <= NUM_LOAD_BUFFERS; ++i) {
        ReservationStation rs;
        rs.name = "Load" + std::to_string(i);
        reservStations_.push_back(rs);
    }
    // buffers de store
    for (int i = 1; i <= NUM_STORE_BUFFERS; ++i) {
        ReservationStation rs;
        rs.name = "Store" + std::to_string(i);
        reservStations_.push_back(rs);
    }
}

// inicialização dos registradores
void Simulator::initRegisters() {
    for (int i = 0; i < NUM_FP_REGISTERS; ++i) {
        std::string regName = "F" + std::to_string(i);
        regStatus_[regName] = RegisterStatus();
        // valor inicial arbitrário para demonstração
        // (registradores pares recebem valor baseado no índice)
        regValues_[regName] = static_cast<double>(i);
    }
    // registradores inteiros (usados como base em load/store)
    for (int i = 0; i < 32; ++i) {
        std::string regName = "R" + std::to_string(i);
        regStatus_[regName] = RegisterStatus();
        regValues_[regName] = 0.0;
    }
}

// determina a latência baseada no tipo de operação
int Simulator::getLatency(const std::string& op) const {
    if (op == "ADD.D" || op == "SUB.D") return LATENCY_ADD;
    if (op == "MUL.D")                  return LATENCY_MULT;
    if (op == "DIV.D")                  return LATENCY_DIV;
    if (op == "L.D")                    return LATENCY_LOAD;
    if (op == "S.D")                    return LATENCY_STORE;
    return 1; // Fallback
}

// verifica se a operação é load ou store
bool Simulator::isLoadStore(const std::string& op) const {
    return (op == "L.D" || op == "S.D");
}

// encontra uma rs livre para o tipo de operação desejado
// retorna o índice no vetor reservstations_, ou -1 se não há.
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
    return -1; // todas ocupadas — stall estrutural
}

// verifica se todas as instruções já terminaram o write result
bool Simulator::allInstructionsDone() const {
    for (const auto& instr : instructions_) {
        if (instr.writeResultCycle == 0) return false;
    }
    return true;
}

// estágio 1: issue
// despacha a próxima instrução da fila (em ordem de programa)
// para uma estação de reserva livre do tipo correto.
// se não há rs livre, ocorre stall (a instrução espera).
void Simulator::processIssue() {
    if (nextIssue_ >= static_cast<int>(instructions_.size())) return;

    Instruction& instr = instructions_[nextIssue_];
    int rsIdx = findFreeRS(instr.op);

    if (rsIdx == -1) return; // stall estrutural: sem rs livre

    ReservationStation& rs = reservStations_[rsIdx];
    rs.busy = true;
    rs.op = instr.op;
    rs.instrIndex = nextIssue_;

    if (instr.op == "L.D") {
        // l.d fdest, offset, rbase
        // src1 = offset (imediato), src2 = registrador base
        rs.A = std::stod(instr.src1);
        // endereço efetivo = offset + valor do registrador base
        if (regStatus_.count(instr.src2) && regStatus_[instr.src2].Qi != "") {
            rs.Qj = regStatus_[instr.src2].Qi;
        } else {
            rs.Vj = regValues_[instr.src2];
            rs.Qj = "";
        }
        rs.Qk = "";
        rs.Vk = 0.0;
    } else if (instr.op == "S.D") {
        // s.d fsrc, offset, rbase
        // dest = registrador fonte (valor a ser armazenado)
        // src1 = offset, src2 = registrador base
        rs.A = std::stod(instr.src1);
        // verifica dependência do registrador que contém o valor a armazenar
        if (regStatus_.count(instr.dest) && regStatus_[instr.dest].Qi != "") {
            rs.Qj = regStatus_[instr.dest].Qi;
        } else {
            rs.Vj = regValues_[instr.dest];
            rs.Qj = "";
        }
        // verifica dependência do registrador base
        if (regStatus_.count(instr.src2) && regStatus_[instr.src2].Qi != "") {
            rs.Qk = regStatus_[instr.src2].Qi;
        } else {
            rs.Vk = regValues_[instr.src2];
            rs.Qk = "";
        }
    } else {
        // instruções aritméticas: add.d, sub.d, mul.d, div.d
        // operando fonte 1 (src1)
        if (regStatus_.count(instr.src1) && regStatus_[instr.src1].Qi != "") {
            rs.Qj = regStatus_[instr.src1].Qi;
        } else {
            rs.Vj = regValues_[instr.src1];
            rs.Qj = "";
        }
        // operando fonte 2 (src2)
        if (regStatus_.count(instr.src2) && regStatus_[instr.src2].Qi != "") {
            rs.Qk = regStatus_[instr.src2].Qi;
        } else {
            rs.Vk = regValues_[instr.src2];
            rs.Qk = "";
        }
    }

    // atualiza o register status (rat) — apenas para instruções que escrevem
    // em registrador (load e aritméticas). store não escreve em registrador.
    if (instr.op != "S.D") {
        regStatus_[instr.dest].Qi = rs.name;
    }

    // registra o ciclo de issue
    instr.issueCycle = clock_;
    instr.rsIndex = rsIdx;
    nextIssue_++;
}

// estágio 2: execute
// para cada rs ocupada cujos operandos estejam prontos
// (qj == "" e qk == ""), inicia ou continua a execução.
// quando os ciclos restantes chegam a 0, a instrução está
// pronta para o write result no próximo ciclo.
void Simulator::processExecution() {
    for (auto& rs : reservStations_) {
        if (!rs.busy) continue;
        if (rs.instrIndex < 0) continue;

        Instruction& instr = instructions_[rs.instrIndex];

        // pula instruções que já completaram write result
        if (instr.writeResultCycle != 0) continue;

        // só pode executar se não foi issue'd neste mesmo ciclo
        if (instr.issueCycle == clock_) continue;

        // verifica se os operandos estão prontos
        if (rs.Qj != "" || rs.Qk != "") continue;

        // primeira vez executando? marca o início
        if (!rs.executing) {
            rs.executing = true;
            rs.cyclesRemaining = getLatency(rs.op);
            instr.execStartCycle = clock_;
        }

        // decrementa ciclos restantes
        rs.cyclesRemaining--;

        // se terminou de executar, calcula o resultado
        if (rs.cyclesRemaining == 0) {
            instr.execEndCycle = clock_;

            // calcula o valor do resultado
            if (rs.op == "ADD.D") {
                rs.result = rs.Vj + rs.Vk;
            } else if (rs.op == "SUB.D") {
                rs.result = rs.Vj - rs.Vk;
            } else if (rs.op == "MUL.D") {
                rs.result = rs.Vj * rs.Vk;
            } else if (rs.op == "DIV.D") {
                rs.result = (rs.Vk != 0.0) ? (rs.Vj / rs.Vk) : 0.0;
            } else if (rs.op == "L.D") {
                // simula um acesso à memória: endereço efetivo = a + vj (base)
                // para simulação, retornamos o endereço como valor simbólico
                rs.result = rs.A + rs.Vj;
            } else if (rs.op == "S.D") {
                // store: o resultado é o endereço de escrita (não escreve em registrador)
                rs.result = rs.A + rs.Vk;
            }
        }
    }
}

// estágio 3: write result (broadcast no cdb)
// se uma rs terminou sua execução (cyclesremaining == 0 e
// executing == true), ela publica seu resultado no cdb:
//   - atualiza todas as rs que esperavam por esse resultado
//     (substitui qj/qk pelo valor vj/vk).
//   - atualiza o register status e o valor do registrador.
//   - libera a rs (busy = false).
void Simulator::processWriteResult() {
    int writesThisCycle = 0;

    for (auto& rs : reservStations_) {
        if (!rs.busy) continue;
        if (!rs.executing) continue;
        if (rs.cyclesRemaining != 0) continue;
        if (rs.instrIndex < 0) continue;

        Instruction& instr = instructions_[rs.instrIndex];

        // não pode fazer write result no mesmo ciclo que terminou a execução
        if (instr.execEndCycle == clock_) continue;

        // já fez write result?
        if (instr.writeResultCycle != 0) continue;

        // marca o ciclo de write result
        instr.writeResultCycle = clock_;

        // broadcast no cdb
        // percorre todas as rs e substitui dependências
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

        // atualiza register status e valor do registrador
        // (somente se o rat ainda aponta para esta rs — proteção contra waw)
        if (instr.op != "S.D") {
            if (regStatus_[instr.dest].Qi == rs.name) {
                regStatus_[instr.dest].Qi = "";
                regValues_[instr.dest] = rs.result;
            }
        }

        // libera a estação de reserva
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

        // limita o número de write results por ciclo ao número de cdbs
        writesThisCycle++;
        if (writesThisCycle >= NUM_CDB) break;
    }
}

// loop principal da simulação
void Simulator::run() {
    std::cout << "============================================================" << std::endl;
    std::cout << "   SIMULADOR DO ALGORITMO DE TOMASULO" << std::endl;
    std::cout << "============================================================" << std::endl;
    std::cout << "Instruções carregadas: " << instructions_.size() << std::endl;
    std::cout << "Pressione ENTER para avançar ciclo a ciclo..." << std::endl;
    std::cout << "============================================================\n" << std::endl;

    // imprime estado inicial (ciclo 0)
    printState();

    while (!allInstructionsDone()) {
        std::cin.get(); // espera o usuário pressionar enter

        clock_++;

        // processa os estágios de trás para frente para evitar
        // que uma instrução avance mais de um estágio por ciclo.
        processWriteResult();
        processExecution();
        processIssue();

        printState();
    }

    // impressão final: valores dos registradores
    std::cout << "\n============================================================" << std::endl;
    std::cout << "   SIMULAÇÃO CONCLUÍDA" << std::endl;
    std::cout << "============================================================" << std::endl;
    std::cout << "\n--- Valores Finais dos Registradores ---\n" << std::endl;

    // coleta os registradores utilizados nas instruções
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

// impressão do estado (tabelas como nos slides)

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

        // vj
        if (rs.busy && rs.Qj == "") {
            std::cout << std::setw(8) << std::fixed << std::setprecision(1) << rs.Vj;
        } else {
            std::cout << std::setw(8) << "";
        }
        std::cout << " │ ";

        // vk
        if (rs.busy && rs.Qk == "") {
            std::cout << std::setw(8) << std::fixed << std::setprecision(1) << rs.Vk;
        } else {
            std::cout << std::setw(8) << "";
        }
        std::cout << " │ "
                  << std::setw(6) << rs.Qj << " │ "
                  << std::setw(6) << rs.Qk << " │ ";

        // a (endereço)
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

    // coleta registradores fp usados para exibição compacta
    std::vector<std::string> displayRegs;
    for (int i = 0; i < NUM_FP_REGISTERS; i += 2) {
        displayRegs.push_back("F" + std::to_string(i));
    }

    for (size_t i = 0; i < displayRegs.size(); ++i) {
        std::cout << "────────";
        if (i < displayRegs.size() - 1) std::cout << "┬";
    }
    std::cout << "┤" << std::endl;

    // nomes dos registradores
    std::cout << "│";
    for (const auto& reg : displayRegs) {
        std::cout << std::setw(7) << reg << " │";
    }
    std::cout << std::endl;

    // separador
    std::cout << "├";
    for (size_t i = 0; i < displayRegs.size(); ++i) {
        std::cout << "────────";
        if (i < displayRegs.size() - 1) std::cout << "┼";
    }
    std::cout << "┤" << std::endl;

    // valores (qi)
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
