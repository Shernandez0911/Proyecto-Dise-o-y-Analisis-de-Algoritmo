// Algoritmo mejorado ("Floating Cone Incremental"):
//  1) Construye precedencias una sola vez con hash map O(1) por vecino.
//  2) Calcula el cono de cada candidato UNA vez (no en cada iteracion).
//  3) Mantiene un indice inverso bloque -> {candidatos cuyo cono cacheado
//     lo contiene}. Al extraer un cono, solo se invalidan/recalculan los
//     candidatos realmente afectados (los que compartian algun bloque con
//     el cono extraido), en vez de recorrer TODO el modelo de nuevo.
//  4) Selecciona en cada iteracion el cono de MAYOR valor cacheado (no el
//     primero encontrado) usando un max-heap con borrado perezoso
//     (lazy deletion) para tolerar entradas obsoletas sin reconstruir
//     el heap completo.
#include "common.hpp"
#include <chrono>

struct HeapEntry {
    double valor;
    int candidato;
    long long version;
    bool operator<(const HeapEntry &o) const { return valor < o.valor; } // max-heap
};

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

Resultado incremental(Model &m) {
    Resultado r;
    auto t0 = chrono::high_resolution_clock::now();
    int n = (int)m.B.size();

    vector<double> valorCache(n, 0.0);
    vector<vector<int>> miembrosCache(n);
    vector<long long> version(n, 0);
    vector<bool> esCandidatoValido(n, false); // false si inextraible o ya extraido
    unordered_map<int, vector<int>> pertenece; // bloque -> candidatos cuyo cono cacheado lo contiene
    priority_queue<HeapEntry> heap;

    unordered_set<int> reportadosInextraibles, reportadosCero;

    auto recomputar = [&](int cand) {
        version[cand]++;
        // limpiar membresias antiguas de este candidato en el indice inverso
        for (int b : miembrosCache[cand]) {
            auto &vec = pertenece[b];
            vec.erase(remove(vec.begin(), vec.end(), cand), vec.end());
        }
        miembrosCache[cand].clear();

        if (m.B[cand].extracted) { esCandidatoValido[cand] = false; return; }
        if (m.B[cand].inextraible) {
            esCandidatoValido[cand] = false;
            if (!reportadosInextraibles.count(cand)) { reportadosInextraibles.insert(cand); r.bloquesInextraibles++; }
            return;
        }
        ConoResult cr = calcularCono(cand, m);
        if (cr.inextraible) { esCandidatoValido[cand] = false; return; }
        esCandidatoValido[cand] = true;
        valorCache[cand] = cr.valor;
        miembrosCache[cand] = cr.miembros;
        for (int b : cr.miembros) pertenece[b].push_back(cand);
        if (cr.valor > 0) {
            heap.push({cr.valor, cand, version[cand]});
        } else if (cr.valor == 0.0) {
            if (!reportadosCero.count(cand)) { reportadosCero.insert(cand); r.conosCeroDetectados++; }
        }
    };

    // 1) inicializacion: calcular el cono de cada candidato una sola vez
    for (int b = 0; b < n; b++) recomputar(b);

    // 2) bucle principal: extraer siempre el cono de mayor valor cacheado
    while (!heap.empty()) {
        HeapEntry top = heap.top(); heap.pop();
        int cand = top.candidato;
        if (m.B[cand].extracted || !esCandidatoValido[cand]) continue;
        if (top.version != version[cand]) continue; // entrada obsoleta -> descartar (lazy deletion)

        // extraer el cono cacheado (sigue siendo valido: nada de lo que contiene fue tocado desde su ultimo calculo)
        vector<int> extraidos = miembrosCache[cand];
        double valor = valorCache[cand];
        for (int id : extraidos) m.B[id].extracted = true;
        r.Vtotal += valor;
        r.bloquesExtraidos += (long long)extraidos.size();
        r.conosExtraidos++;

        // 3) determinar candidatos afectados: los que compartian algun bloque con el cono recien extraido
        unordered_set<int> afectados;
        for (int id : extraidos) {
            auto it = pertenece.find(id);
            if (it == pertenece.end()) continue;
            for (int c : it->second) if (!m.B[c].extracted) afectados.insert(c);
            pertenece.erase(it);
        }
        afectados.erase(cand);

        // 4) recalcular SOLO los candidatos afectados (no todo el modelo)
        for (int c : afectados) recomputar(c);
    }

    auto t1 = chrono::high_resolution_clock::now();
    r.tiempoMs = chrono::duration<double, milli>(t1 - t0).count();
    return r;
}

#ifndef NO_MAIN
int main(int argc, char **argv) {
    if (argc != 2) { cerr << "Uso: improved <instancia.csv>\n"; return 1; }
    Model m = loadInstance(argv[1]);
    Resultado r = incremental(m);
    cout << "algoritmo=incremental"
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
