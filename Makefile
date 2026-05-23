# ============================================================
# Makefile - Simulador do Algoritmo de Tomasulo
# ============================================================
# Uso:
#   make          -> Compila o simulador
#   make run      -> Compila e executa
#   make clean    -> Remove binários
# ============================================================

CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET   = tomasulo
SRCDIR   = src
SOURCES  = $(SRCDIR)/main.cpp $(SRCDIR)/Simulator.cpp
HEADERS  = $(SRCDIR)/Config.h $(SRCDIR)/Instruction.h $(SRCDIR)/Hardware.h $(SRCDIR)/Simulator.h

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
