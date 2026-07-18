#pragma once
#include <bits/stdc++.h>
using namespace std;

struct Block {
    int x, y, z;
    double v;
    bool extracted = false;
    bool inextraible = false; // le falta al menos 1 de los 9 predecesores superiores
    bool is_surface = false;
    vector<int> preds; // indices de los 9 bloques superiores (solo si !is_surface && !inextraible)
};

static inline long long key(int x, int y, int z) {
    // codificacion simple para coordenadas no negativas < 2^20
    return ((long long)(x + (1 << 19)) << 40) | ((long long)(y + (1 << 19)) << 20) | (long long)(z + (1 << 19));
}

struct Model {
    vector<Block> B;
    unordered_map<long long, int> coordIdx;

    void buildIndex() {
        coordIdx.reserve(B.size() * 2);
        for (int i = 0; i < (int)B.size(); i++) coordIdx[key(B[i].x, B[i].y, B[i].z)] = i;
    }

    // O(n) total: 9 lookups O(1) promedio por bloque -> construccion de precedencias
    void buildPrecedence() {
        for (auto &b : B) {
            long long kAbove = key(b.x, b.y, b.z + 1);
            b.is_surface = (coordIdx.find(kAbove) == coordIdx.end());
            if (b.is_surface) continue;
            bool complete = true;
            vector<int> ps;
            ps.reserve(9);
            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    long long k = key(b.x + i, b.y + j, b.z + 1);
                    auto it = coordIdx.find(k);
                    if (it == coordIdx.end()) { complete = false; break; }
                    ps.push_back(it->second);
                }
                if (!complete) break;
            }
            if (!complete) {
                b.inextraible = true;
            } else {
                b.preds = std::move(ps);
            }
        }
    }
};

// ------------------------------------------------------------------
// Parametros economicos entregados por el profesor (correo escenario00)
// ValorBloque = (Metal_1 * RECUP_M1 * PRECIO_M1) + (Metal_2 * RECUP_M2 * PRECIO_M2)
//               - (Tonelada * COSTO_PROCESAMIENTO) - (Tonelada * COSTO_EXTRACCION)
// ------------------------------------------------------------------
static constexpr double COSTO_EXTRACCION      = 25.0;
static constexpr double COSTO_PROCESAMIENTO   = 13.0;
static constexpr double RECUPERACION_METAL_1  = 0.89;
static constexpr double RECUPERACION_METAL_2  = 0.89;
static constexpr double PRECIO_METAL_1        = 13228.0;
static constexpr double PRECIO_METAL_2        = 0.0;
// Nota: TASA_DESCUENTO = 0.08 se entrega en el correo del profesor pero no
// participa en la formula de valor de bloque indicada; se deja documentada
// aqui para uso en extensiones futuras (ej. valor presente por periodo de
// extraccion), no se aplica en la Entrega 2.
static constexpr double TASA_DESCUENTO        = 0.08;

static inline double valorBloqueDesdeLeyes(double tonelada, double metal1, double metal2) {
    return (metal1 * RECUPERACION_METAL_1 * PRECIO_METAL_1)
         + (metal2 * RECUPERACION_METAL_2 * PRECIO_METAL_2)
         - (tonelada * COSTO_PROCESAMIENTO)
         - (tonelada * COSTO_EXTRACCION);
}

// Separa una linea CSV por comas, tolerando \r final (archivos con CRLF).
static inline vector<string> splitCsv(const string &line) {
    vector<string> out;
    stringstream ss(line);
    string tok;
    while (getline(ss, tok, ',')) out.push_back(tok);
    if (!out.empty() && !out.back().empty() && out.back().back() == '\r') out.back().pop_back();
    return out;
}

static inline bool esNumerica(const string &tok) {
    if (tok.empty()) return false;
    try { size_t pos; stod(tok, &pos); return pos == tok.size(); }
    catch (...) { return false; }
}

// Carga una instancia soportando dos formatos de entrada, detectados
// automaticamente a partir del numero de columnas de la primera fila de
// datos (se ignora una eventual fila de encabezado no numerica):
//
//  (A) Formato interno "x,y,z,v" (con encabezado) - usado por
//      generate_instance y por los casos de prueba manuales. El valor
//      economico ya viene calculado.
//
//  (B) Formato de escenario real "x,y,z,tonelada,metal_1,metal_2,roca"
//      (sin encabezado, tal como lo entrega el profesor, p.ej.
//      escenario00.txt). El valor economico del bloque se calcula con
//      valorBloqueDesdeLeyes() usando los parametros del correo.
static inline Model loadInstance(const string &path) {
    Model m;
    ifstream f(path);
    if (!f) { throw runtime_error("no se pudo abrir " + path); }

    string line;
    vector<string> primeraFilaDatos;
    while (getline(f, line)) {
        if (line.empty()) continue;
        vector<string> campos = splitCsv(line);
        if (campos.empty()) continue;
        if (!esNumerica(campos[0])) continue; // fila de encabezado (ej. "x,y,z,v"): se descarta
        primeraFilaDatos = campos;
        break;
    }
    if (primeraFilaDatos.empty()) { throw runtime_error("instancia vacia o sin filas de datos: " + path); }

    int nCampos = (int)primeraFilaDatos.size();
    bool formatoCrudo; // true = (x,y,z,tonelada,metal_1,metal_2,roca), false = (x,y,z,v)
    if (nCampos == 7) formatoCrudo = true;
    else if (nCampos == 4) formatoCrudo = false;
    else throw runtime_error("formato de columnas no reconocido (" + to_string(nCampos) + " campos) en " + path);

    auto procesarFila = [&](const vector<string> &c) {
        Block b;
        b.x = stoi(c[0]);
        b.y = stoi(c[1]);
        b.z = stoi(c[2]);
        if (formatoCrudo) {
            double tonelada = stod(c[3]);
            double metal1   = stod(c[4]);
            double metal2   = stod(c[5]);
            // c[6] = tipo de roca, no se utiliza en esta entrega.
            b.v = valorBloqueDesdeLeyes(tonelada, metal1, metal2);
        } else {
            b.v = stod(c[3]);
        }
        m.B.push_back(b);
    };

    procesarFila(primeraFilaDatos);
    while (getline(f, line)) {
        if (line.empty()) continue;
        vector<string> campos = splitCsv(line);
        if (campos.empty()) continue;
        if (!esNumerica(campos[0])) continue; // por si hay encabezados intermedios o lineas basura
        if ((int)campos.size() != nCampos) {
            throw runtime_error("fila con numero de columnas inconsistente en " + path);
        }
        procesarFila(campos);
    }

    m.buildIndex();
    m.buildPrecedence();
    return m;
}

struct Resultado {
    double Vtotal = 0.0;
    long long bloquesExtraidos = 0;
    long long conosExtraidos = 0;
    long long bloquesInextraibles = 0;
    long long conosCeroDetectados = 0;
    double tiempoMs = 0.0;
};
