#ifndef CONFIG_H
#define CONFIG_H

// config.h - configurações do simulador do algoritmo de tomasulo
// Altere estes valores para redimensionar o simulador conforme
// necessário para diferentes cenários de teste.

// latências das unidades funcionais (em ciclos de clock)
// Baseado nos valores clássicos do livro Hennessy & Patterson.
constexpr int LATENCY_ADD  = 2;   // add.d, sub.d
constexpr int LATENCY_MULT = 10;  // mul.d
constexpr int LATENCY_DIV  = 40;  // div.d
constexpr int LATENCY_LOAD = 2;   // l.d (1 ciclo cálculo endereço + 1 acesso memória)
constexpr int LATENCY_STORE = 2;  // s.d

// quantidade de estações de reserva
constexpr int NUM_ADD_RS  = 3;  // add1, add2, add3
constexpr int NUM_MULT_RS = 2;  // mult1, mult2
constexpr int NUM_LOAD_BUFFERS  = 3;  // load1, load2, load3
constexpr int NUM_STORE_BUFFERS = 3;  // store1, store2, store3

// número de barramentos comuns de dados (cdb)
// Define quantos Write Results podem ocorrer por ciclo.
// O padrão clássico do Tomasulo é 1 CDB, mas processadores
// superescalares modernos podem ter múltiplos CDBs.
constexpr int NUM_CDB = 1;

// quantidade de registradores de ponto flutuante
constexpr int NUM_FP_REGISTERS = 32;  // f0 a f31

#endif // CONFIG_H
