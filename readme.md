# Tarea 5 - Simulador de RISC-V (CS3051)

**Alumno:** ____________________  **Código:** ____________

- **Repositorio Git:** https://github.com/TU_USUARIO/riscv-sim
- **Carpeta Google Drive (video + código):** https://drive.google.com/________

## Informe breve

**Lenguaje:** C++ (un solo archivo `sim.cpp`, sin librerías externas).

**Compilar y ejecutar:**

    g++ -O2 -o sim sim.cpp
    ./sim programa.bin

**Cómo funciona:** el simulador guarda el estado de la CPU: el PC, los 32 registros (x0 siempre queda en 0) y la memoria. La memoria es un `map<uint32_t, uint8_t>`, o sea es byte-addressable con 32 bits de address space y solo guarda los bytes que se usan. Las lecturas y escrituras de words y halfwords se hacen en little-endian. El programa (binario crudo exportado de CPUlator) se carga desde la dirección 0x00000000.

Cada `step` hace fetch de la instrucción en PC, saca opcode, rd, rs1, rs2, funct3, funct7 y los inmediatos (tipos I, S, B, U, J), ejecuta y actualiza registros, memoria y PC.

**Instrucciones soportadas:** todo el RV32I pedido en el anexo A: lb, lh, lw, lbu, lhu, addi, slli, slti, sltiu, xori, srli, srai, ori, andi, auipc, sb, sh, sw, add, sub, sll, slt, sltu, xor, srl, sra, or, and, lui, beq, bne, blt, bge, bltu, bgeu, jal, jalr.

**Comandos:**

| Comando | Qué hace |
|---|---|
| `step [n]` o `s` | ejecuta 1 (o n) instrucciones |
| `run` | ejecuta hasta terminar (máx 1 millón de instrucciones) |
| `pc` | muestra el PC |
| `regs [x5 x14 ...]` | muestra esos registros, o todos si no se pone nada |
| `mem a1 a2` | muestra los bytes desde a1 hasta a2 |
| `exit` | sale |

**Extras:**

- `ecall` básico estilo SPIM: a7=1 imprime entero, a7=4 imprime string, a7=11 imprime char, a7=10 termina.
- Detecta el loop infinito final típico (`j .` / `beq x0,x0,0`) y para el programa.
- Instrucción inválida: se imprime el código y el PC y se detiene la ejecución.
- El stack pointer (x2) arranca en 0x7FFFF000 para que funcionen programas recursivos.

**Pruebas:**

- `riscvtest.s`: al terminar, `mem 0x64 0x67` muestra `19 00 00 00`, o sea 25 en little-endian. Correcto.
- Quicksort: `mem 0x1000 0x101b` muestra el arreglo ordenado {1, 2, 3, 4, 6, 8, 9}.
- Árbol simétrico: al terminar, `regs x10` da a0 = 1.
- Programa propio con ecall para imprimir.

**Limitaciones:** no hay desensamblador, no lee ELF, no hay soporte GDB, no hay fence/csr/ebreak.
