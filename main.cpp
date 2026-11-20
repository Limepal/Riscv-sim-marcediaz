#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <cstdint>
using namespace std;

uint32_t pc = 0;
uint32_t x[32];
map<uint32_t, uint8_t> mem;
bool fin = false;

uint8_t rb(uint32_t a) { return mem.count(a) ? mem[a] : 0; }
void wb(uint32_t a, uint8_t v) { mem[a] = v; }
uint32_t rw(uint32_t a) { return rb(a) | (rb(a + 1) << 8) | (rb(a + 2) << 16) | ((uint32_t)rb(a + 3) << 24); }
uint32_t rh(uint32_t a) { return rb(a) | (rb(a + 1) << 8); }
void ww(uint32_t a, uint32_t v) { for (int i = 0; i < 4; i++) wb(a + i, (v >> (8 * i)) & 0xFF); }

int32_t sx(uint32_t v, int b) { return (int32_t)(v << (32 - b)) >> (32 - b); }

void step() {
    uint32_t in = rw(pc);
    uint32_t op = in & 0x7F, rd = (in >> 7) & 31, f3 = (in >> 12) & 7, r1 = (in >> 15) & 31, r2 = (in >> 20) & 31, f7 = in >> 25;
    uint32_t a = x[r1], b = x[r2];
    int32_t ii = sx(in >> 20, 12);
    int32_t is = sx(((in >> 25) << 5) | rd, 12);
    int32_t ib = sx(((in >> 31) << 12) | (((in >> 7) & 1) << 11) | (((in >> 25) & 0x3F) << 5) | (((in >> 8) & 0xF) << 1), 13);
    int32_t ij = sx(((in >> 31) << 20) | (((in >> 12) & 0xFF) << 12) | (((in >> 20) & 1) << 11) | (((in >> 21) & 0x3FF) << 1), 21);
    uint32_t iu = in & 0xFFFFF000;
    uint32_t np = pc + 4;
    uint32_t r = 0;
    bool w = true;
    switch (op) {
    case 0x03: {
        uint32_t ad = a + ii;
        if (f3 == 0) r = sx(rb(ad), 8);
        else if (f3 == 1) r = sx(rh(ad), 16);
        else if (f3 == 2) r = rw(ad);
        else if (f3 == 4) r = rb(ad);
        else if (f3 == 5) r = rh(ad);
        cout << "load de 0x" << hex << ad << dec << endl;
        break;
    }
    case 0x13: {
        uint32_t sh = r2;
        if (f3 == 0) r = a + ii;
        else if (f3 == 1) r = a << sh;
        else if (f3 == 2) r = (int32_t)a < ii;
        else if (f3 == 3) r = a < (uint32_t)ii;
        else if (f3 == 4) r = a ^ ii;
        else if (f3 == 5) r = (f7 & 0x20) ? (uint32_t)((int32_t)a >> sh) : a >> sh;
        else if (f3 == 6) r = a | ii;
        else r = a & ii;
        break;
    }
    case 0x17: r = pc + iu; break;
    case 0x37: r = iu; break;
    case 0x23: {
        uint32_t ad = a + is;
        if (f3 == 0) wb(ad, b & 0xFF);
        else if (f3 == 1) { wb(ad, b & 0xFF); wb(ad + 1, (b >> 8) & 0xFF); }
        else ww(ad, b);
        cout << "store en 0x" << hex << ad << dec << endl;
        w = false;
        break;
    }
    case 0x33: {
        uint32_t sh = b & 31;
        if (f3 == 0) r = (f7 & 0x20) ? a - b : a + b;
        else if (f3 == 1) r = a << sh;
        else if (f3 == 2) r = (int32_t)a < (int32_t)b;
        else if (f3 == 3) r = a < b;
        else if (f3 == 4) r = a ^ b;
        else if (f3 == 5) r = (f7 & 0x20) ? (uint32_t)((int32_t)a >> sh) : a >> sh;
        else if (f3 == 6) r = a | b;
        else r = a & b;
        break;
    }
    case 0x63: {
        bool t = false;
        if (f3 == 0) t = a == b;
        else if (f3 == 1) t = a != b;
        else if (f3 == 4) t = (int32_t)a < (int32_t)b;
        else if (f3 == 5) t = (int32_t)a >= (int32_t)b;
        else if (f3 == 6) t = a < b;
        else if (f3 == 7) t = a >= b;
        if (t) { np = pc + ib; cout << "salto" << endl; }
        w = false;
        break;
    }
    case 0x6F: r = pc + 4; np = pc + ij; break;
    case 0x67: r = pc + 4; np = (a + ii) & ~1u; break;
    case 0x73: {
        w = false;
        if (x[17] == 1) cout << "out: " << (int32_t)x[10] << endl;
        else if (x[17] == 4) { uint32_t p = x[10]; cout << "out: "; while (rb(p)) cout << (char)rb(p++); cout << endl; }
        else if (x[17] == 11) cout << "out: " << (char)x[10] << endl;
        else if (x[17] == 10) { cout << "exit ecall" << endl; fin = true; }
        else cout << "ecall raro" << endl;
        break;
    }
    default:
        cout << "instruccion invalida 0x" << hex << in << " en pc 0x" << pc << dec << endl;
        fin = true;
        return;
    }
    if (w && rd != 0) x[rd] = r;
    if (np == pc) { cout << "loop infinito, fin" << endl; fin = true; }
    pc = np;
}

void hx(uint32_t v) { cout << "0x"; cout.width(8); cout.fill('0'); cout << hex << v << dec << endl; }

int main(int argc, char** argv) {
    if (argc < 2) { cout << "uso: ./sim programa.bin" << endl; return 1; }
    ifstream f(argv[1], ios::binary);
    if (!f) { cout << "no abre archivo" << endl; return 1; }
    char c; uint32_t i = 0;
    while (f.get(c)) wb(i++, (uint8_t)c);
    cout << "cargado " << i << " bytes" << endl;
    x[2] = 0x7FFFF000;
    string l;
    while (cout << "> " && getline(cin, l)) {
        stringstream ss(l);
        string cmd; ss >> cmd;
        if (cmd == "step" || cmd == "s") {
            int n = 1; ss >> n;
            for (int k = 0; k < n && !fin; k++) { cout << "pc 0x" << hex << pc << " inst 0x" << rw(pc) << dec << endl; step(); }
            if (fin) cout << "programa terminado" << endl;
        } else if (cmd == "run") {
            int k = 0;
            while (!fin && k < 1000000) { step(); k++; }
            cout << "ejecutadas " << k << endl;
        } else if (cmd == "pc") { cout << "pc = "; hx(pc); }
        else if (cmd == "regs") {
            string r; bool alguno = false;
            while (ss >> r) { alguno = true; int n = stoi(r.substr(1)); cout << "x" << n << " = "; hx(x[n]); }
            if (!alguno) for (int k = 0; k < 32; k++) { cout << "x" << k << " = "; hx(x[k]); }
        } else if (cmd == "mem") {
            string s1, s2; ss >> s1 >> s2;
            uint32_t a1 = stoul(s1, 0, 0), a2 = s2.empty() ? a1 + 3 : stoul(s2, 0, 0);
            cout << "mem:";
            for (uint32_t k = a1; k <= a2; k++) { cout << " "; cout.width(2); cout.fill('0'); cout << hex << (int)rb(k) << dec; }
            cout << endl;
        } else if (cmd == "exit" || cmd == "q") { cout << "chau" << endl; break; }
        else if (cmd != "") cout << "comandos: step [n], run, pc, regs [xN...], mem a1 a2, exit" << endl;
    }
    return 0;
}