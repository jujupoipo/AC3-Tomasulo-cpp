# Simulador do Algoritmo de Tomasulo (C++)

Este projeto é uma implementação em C++ do **Algoritmo de Tomasulo**, um algoritmo de escalonamento dinâmico de instruções que permite a execução fora de ordem, facilitando o aproveitamento do paralelismo em nível de instrução (ILP).

O simulador demonstra conceitos fundamentais de arquitetura de computadores, como:
- **Renomeação de Registradores** (através do Register Status/RAT).
- **Estações de Reserva** para controle de dependências.
- **Barramento Comum de Dados (CDB)** para broadcast de resultados.
- **Tratamento de Riscos (Hazards):** RAW, WAR e WAW.

## 🚀 Como Compilar

O projeto utiliza um `Makefile` para facilitar a compilação. Certifique-se de ter o `g++` instalado.

```bash
make
```
Isto gerará o executável `tomasulo`.

## 💻 Como Executar

### Modo Interativo
Para rodar o simulador ciclo a ciclo com um arquivo de instruções específico:

```bash
./tomasulo caminho/para/arquivo.txt
```
Durante a execução, pressione **ENTER** para avançar para o próximo ciclo de clock.

### Modo de Teste Automatizado
Para executar todos os testes presentes na pasta `tests/` e salvar os resultados automaticamente na pasta `results/`:

```bash
bash run_all_tests.sh
```

## 📁 Estrutura do Projeto

- `src/`: Código-fonte em C++.
- `tests/`: Arquivos de entrada com diferentes cenários de teste (riscos estruturais, dependências de dados, etc.).
- `results/`: Logs gerados das simulações completas.
- `instructions.txt`: Conjunto de instruções padrão (exemplo clássico de Hennessy & Patterson).

## 📝 Formato das Instruções

O simulador aceita instruções no formato `OP DEST, SRC1, SRC2`.
Operações suportadas: `L.D`, `S.D`, `ADD.D`, `SUB.D`, `MUL.D`, `DIV.D`.

Exemplo:
```text
L.D F6, 34, R2
MUL.D F0, F2, F4
SUB.D F8, F6, F2
```

## 🛠️ Detalhes da Implementação

As latências padrão configuradas são:
- **Load / Store:** 2 ciclos.
- **Soma / Subtração:** 2 ciclos.
- **Multiplicação:** 10 ciclos.
- **Divisão:** 40 ciclos.

O simulador exibe a cada ciclo:
1. **Status das Instruções:** Progresso nas etapas de *Issue*, *Execute* e *Write Result*.
2. **Estações de Reserva:** Estado atual das unidades funcionais (Vj, Vk, Qj, Qk, etc.).
3. **RAT (Register Alias Table):** Mapeamento do status dos registradores.

---
Desenvolvido para a disciplina de Arquitetura de Computadores.
