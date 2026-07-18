// Validador independiente. Ejecuta un algoritmo (via su Resultado + el propio
// Model modificado) NO es necesario: en cambio, aqui repetimos la extraccion
// dentro del mismo binario para verificar, bloque por bloque, que todo
// bloque marcado como extraido es de superficie o tiene sus 9 predecesores
// tambien extraidos. Se linkea reutilizando la logica de cada algoritmo
// mediante recompilacion con distinta funcion objetivo (ver Makefile-like
// script run_validation.sh).
#include "common.hpp"

// Declaraciones externas (definidas en baseline.cpp / improved.cpp, compiladas aparte)
Resultado lineaBase(Model &m);
Resultado incremental(Model &m);

static bool validar(Model &m, const string &nombre) {
    bool ok = true;
    int extraidosOk = 0;
    for (auto &b : m.B) {
        if (!b.extracted) continue;
        if (b.is_surface) { extraidosOk++; continue; }
        if (b.inextraible) { ok = false; cerr << "[" << nombre << "] ERROR: bloque marcado inextraible fue extraido ("<<b.x<<","<<b.y<<","<<b.z<<")\n"; continue; }
        bool todosPred = true;
        for (int p : b.preds) if (!m.B[p].extracted) { todosPred = false; break; }
        if (!todosPred) { ok = false; cerr << "[" << nombre << "] ERROR: bloque ("<<b.x<<","<<b.y<<","<<b.z<<") extraido sin sus 9 predecesores\n"; }
        else extraidosOk++;
    }
    cout << "[" << nombre << "] validacion=" << (ok ? "OK" : "FALLA") << " bloques_extraidos_verificados=" << extraidosOk << "\n";
    return ok;
}

int main(int argc, char **argv) {
    if (argc != 2) { cerr << "Uso: validate <instancia.csv>\n"; return 1; }
    Model m1 = loadInstance(argv[1]);
    Resultado r1 = lineaBase(m1);
    validar(m1, "baseline " + string(argv[1]));

    Model m2 = loadInstance(argv[1]);
    Resultado r2 = incremental(m2);
    validar(m2, "incremental " + string(argv[1]));

    cout << "Vtotal baseline=" << r1.Vtotal << "  Vtotal incremental=" << r2.Vtotal << "\n";
    return 0;
}
