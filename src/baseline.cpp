// Linea base: en cada iteracion recorre TODOS los bloques no extraidos,
// reconstruye el cono invertido completo de cada candidato desde cero,
// y extrae el PRIMER cono positivo encontrado. O(n^3) en el peor caso.
#include "common.hpp"
#include <chrono>

// Reconstruye el cono invertido de b desde cero. Devuelve {inextraible, valor, miembros}.
struct ConoResult { bool inextraible; double valor; vector<int> miembros; };

static ConoResult calcularCono(int start, Model &m) {
    vector<int> cono;
    unordered_set<int> visitado;
    vector<int> pila = {start};
    bool esInextraible = false;
    while (!pila.empty()) {
        int actual = pila.back(); pila.pop_back();
        if (m.B[actual].extracted || visitado.count(actual)) continue;
        if (m.B[actual].inextraible) { esInextraible = true; continue; }
        visitado.insert(actual);
        cono.push_back(actual);
        for (int p : m.B[actual].preds) {
            if (!m.B[p].extracted) pila.push_back(p);
        }
    }
    if (esInextraible) return {true, 0.0, {}};
    double valor = 0.0;
    for (int id : cono) valor += m.B[id].v;
    return {false, valor, cono};
}

Resultado lineaBase(Model &m) {
    Resultado r;
    auto t0 = chrono::high_resolution_clock::now();

    unordered_set<int> reportadosInextraibles, reportadosCero;
    bool cambio = true;
    while (cambio) {
        cambio = false;
        for (int b = 0; b < (int)m.B.size(); b++) {
            if (m.B[b].extracted) continue;
            if (m.B[b].inextraible) {
                if (!reportadosInextraibles.count(b)) { reportadosInextraibles.insert(b); r.bloquesInextraibles++; }
                continue;
            }
            ConoResult cr = calcularCono(b, m);
            if (cr.inextraible) continue; // el cono depende de un predecesor faltante
            if (cr.valor > 0) {
                for (int id : cr.miembros) m.B[id].extracted = true;
                r.Vtotal += cr.valor;
                r.bloquesExtraidos += (long long)cr.miembros.size();
                r.conosExtraidos++;
                cambio = true;
                break; // primer cono positivo encontrado -> se extrae y se reinicia el barrido
            } else if (cr.valor == 0.0) {
                if (!reportadosCero.count(b)) { reportadosCero.insert(b); r.conosCeroDetectados++; }
            }
        }
    }

    auto t1 = chrono::high_resolution_clock::now();
    r.tiempoMs = chrono::duration<double, milli>(t1 - t0).count();
    return r;
}

#ifndef NO_MAIN
int main(int argc, char **argv) {
    if (argc != 2) { cerr << "Uso: baseline <instancia.csv>\n"; return 1; }
    Model m = loadInstance(argv[1]);
    Resultado r = lineaBase(m);
    cout << "algoritmo=baseline"
         << " n=" << m.B.size()
         << " Vtotal=" << fixed << setprecision(3) << r.Vtotal
         << " bloquesExtraidos=" << r.bloquesExtraidos
         << " conosExtraidos=" << r.conosExtraidos
         << " bloquesInextraibles=" << r.bloquesInextraibles
         << " conosCero=" << r.conosCeroDetectados
         << " tiempoMs=" << r.tiempoMs
         << "\n";
    return 0;
}
#endif
