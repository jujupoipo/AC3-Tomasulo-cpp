#ifndef CONFIG_H
#define CONFIG_H

// ============================================================
// Config.h - Configurações do Simulador do Algoritmo de Tomasulo
// ============================================================
// Altere estes valores para redimensionar o simulador conforme
// necessário para diferentes cenários de teste.
// ============================================================

// --- Latências das Unidades Funcionais (em ciclos de clock) ---
// Baseado nos valores clássicos do livro Hennessy & Patterson.
constexpr int LATENCY_ADD  = 2;   // ADD.D, SUB.D
constexpr int LATENCY_MULT = 10;  // MUL.D
constexpr int LATENCY_DIV  = 40;  // DIV.D
constexpr int LATENCY_LOAD = 2;   // L.D (1 ciclo cálculo endereço + 1 acesso memória)
constexpr int LATENCY_STORE = 2;  // S.D

// --- Quantidade de Estações de Reserva ---
constexpr int NUM_ADD_RS  = 3;  // Add1, Add2, Add3
constexpr int NUM_MULT_RS = 2;  // Mult1, Mult2
constexpr int NUM_LOAD_BUFFERS  = 3;  // Load1, Load2, Load3
constexpr int NUM_STORE_BUFFERS = 3;  // Store1, Store2, Store3

// --- Quantidade de Registradores de Ponto Flutuante ---
constexpr int NUM_FP_REGISTERS = 32;  // F0 a F31

#endif // CONFIG_H
