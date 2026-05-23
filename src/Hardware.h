#ifndef HARDWARE_H
#define HARDWARE_H

#include <string>

// ============================================================
// Hardware.h - Estruturas de Hardware do Algoritmo de Tomasulo
// ============================================================

// --- Estação de Reserva (Reservation Station) ---
// Cada RS armazena uma instrução despachada e seus operandos.
// Os campos Qj/Qk indicam dependências pendentes (nome da RS
// que produzirá o valor). Quando Qj/Qk == "", o operando
// correspondente (Vj/Vk) já está disponível.
struct ReservationStation {
    std::string name;    // Nome identificador: Add1, Mult2, Load1, Store1, etc.
    bool busy = false;   // Ocupada?
    std::string op;      // Operação a ser executada (ADD.D, MUL.D, etc.)

    double Vj = 0.0;     // Valor do operando fonte j
    double Vk = 0.0;     // Valor do operando fonte k
    std::string Qj = ""; // RS que produzirá Vj ("" = valor já pronto)
    std::string Qk = ""; // RS que produzirá Vk ("" = valor já pronto)

    double A = 0.0;      // Campo de endereço/offset para Load e Store

    // --- Campos auxiliares de controle de execução ---
    int cyclesRemaining = 0;  // Ciclos restantes de execução
    bool executing = false;   // Já iniciou execução?
    double result = 0.0;      // Resultado calculado pela unidade funcional
    int instrIndex = -1;      // Índice da instrução associada na lista global
};

// --- Status dos Registradores (Register Alias Table - RAT) ---
// Indica, para cada registrador FP, qual RS está responsável
// por produzir o próximo valor. Se Qi == "", o registrador
// possui seu valor atualizado e disponível.
struct RegisterStatus {
    std::string Qi = "";   // Nome da RS que vai fornecer o resultado ("" = limpo)
};

#endif // HARDWARE_H
