// Genera un modelo de bloques 3D rectangular (dx * dy * dz bloques).
// Los valores simulan un yacimiento simple: mayor probabilidad de valor
// negativo (esteril) en niveles profundos, mayor probabilidad de valor
// positivo (mineral) cerca de la superficie, con ruido aleatorio.
// Uso: generate_instance <dx> <dy> <dz> <seed> <archivo_salida.csv>
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char **argv) {
    if (argc != 6) {
        cerr << "Uso: generate_instance <dx> <dy> <dz> <seed> <salida.csv>\n";
        return 1;
    }
    int dx = stoi(argv[1]), dy = stoi(argv[2]), dz = stoi(argv[3]);
    unsigned seed = (unsigned)stoul(argv[4]);
    string outPath = argv[5];

    mt19937 rng(seed);
    uniform_real_distribution<double> noise(-5.0, 5.0);
    uniform_real_distribution<double> uni01(0.0, 1.0);

    ofstream f(outPath);
    f << "x,y,z,v\n";
    // z = 0 es el nivel mas profundo, z = dz-1 es la superficie
    for (int z = 0; z < dz; z++) {
        double depthRatio = 1.0 - (double)z / max(1, dz - 1); // 1 = profundo, 0 = superficie
        for (int x = 0; x < dx; x++) {
            for (int y = 0; y < dy; y++) {
                double base;
                if (uni01(rng) < 0.35 + 0.25 * depthRatio) {
                    // esteril / costo de extraccion
                    base = -(3.0 + 6.0 * depthRatio);
                } else {
                    // mineral de interes, mas valioso cerca de la superficie
                    base = 4.0 + 8.0 * (1.0 - depthRatio);
                }
                double v = base + noise(rng);
                f << x << "," << y << "," << z << "," << fixed << setprecision(3) << v << "\n";
            }
        }
    }
    f.close();
    cerr << "Instancia generada: " << outPath << " (" << (dx * dy * dz) << " bloques)\n";
    return 0;
}
