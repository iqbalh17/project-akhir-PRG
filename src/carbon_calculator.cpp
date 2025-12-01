#include <emscripten/bind.h>
#include <vector>
#include <string>

using namespace emscripten;
using namespace std;

// ============================================
// KONSTANTA EMISSION FACTOR
// ============================================

const double FACTOR_AC = 1.9509;
const double FACTOR_LAMPU_PANEL = 0.033444;
const double FACTOR_LIFT = 0.929;
const double FACTOR_KOMPUTER = 0.279;
const double FACTOR_SERVER = 1.394;
const double FACTOR_WIFI = 0.019;
const double FACTOR_PROYEKTOR = 0.279;
const double FACTOR_POMPA_AIR = 1.394;
const double FACTOR_LAMPU_KORIDOR = 0.093;
const double FACTOR_CCTV = 0.009;
const double FACTOR_KULKAS = 0.279;

const double THRESHOLD_AMAN = 50.0;

// ============================================
// STRUKTUR DATA
// ============================================

struct Item {
    string kategori;
    int jumlah;
    double jam_operasi_per_hari;
};

// ============================================
// FUNCTION PERHITUNGAN
// ============================================

double hitungAC(int jumlah, double jam) {
    return jumlah * jam * FACTOR_AC;
}

double hitungLampuPanel(int jumlah, double jam) {
    return jumlah * jam * FACTOR_LAMPU_PANEL;
}

double hitungLift(int jumlah, double jam) {
    return jumlah * jam * FACTOR_LIFT;
}

double hitungKomputer(int jumlah, double jam) {
    return jumlah * jam * FACTOR_KOMPUTER;
}

double hitungServer(int jumlah, double jam) {
    return jumlah * jam * FACTOR_SERVER;
}

double hitungWifi(int jumlah, double jam) {
    return jumlah * jam * FACTOR_WIFI;
}

double hitungProyektor(int jumlah, double jam) {
    return jumlah * jam * FACTOR_PROYEKTOR;
}

double hitungPompaAir(int jumlah, double jam) {
    return jumlah * jam * FACTOR_POMPA_AIR;
}

double hitungLampuKoridor(int jumlah, double jam) {
    return jumlah * jam * FACTOR_LAMPU_KORIDOR;
}

double hitungCCTV(int jumlah, double jam) {
    return jumlah * jam * FACTOR_CCTV;
}

double hitungKulkas(int jumlah, double jam) {
    return jumlah * jam * FACTOR_KULKAS;
}

// ============================================
// FUNCTION UTAMA
// ============================================

double calculateCarbonEmission(vector<Item> items) {
    double total_emisi_per_hari = 0.0;
    
    for (const auto& item : items) {
        double emisi_per_hari = 0.0;
        
        if (item.kategori == "AC") {
            emisi_per_hari = hitungAC(item.jumlah, item.jam_operasi_per_hari);
        }
        else if (item.kategori == "Lampu Panel LED") {
            emisi_per_hari = hitungLampuPanel(item.jumlah, item.jam_operasi_per_hari);
        }
        else if (item.kategori == "Lift") {
            emisi_per_hari = hitungLift(item. jumlah, item.jam_operasi_per_hari);
        }
        else if (item.kategori == "Komputer") {
            emisi_per_hari = hitungKomputer(item.jumlah, item.jam_operasi_per_hari);
        }
        else if (item.kategori == "Server") {
            emisi_per_hari = hitungServer(item.jumlah, item.jam_operasi_per_hari);
        }
        else if (item. kategori == "WiFi") {
            emisi_per_hari = hitungWifi(item.jumlah, item.jam_operasi_per_hari);
        }
        else if (item.kategori == "Proyektor") {
            emisi_per_hari = hitungProyektor(item.jumlah, item.jam_operasi_per_hari);
        }
        else if (item.kategori == "Pompa Air") {
            emisi_per_hari = hitungPompaAir(item.jumlah, item.jam_operasi_per_hari);
        }
        else if (item.kategori == "Lampu Koridor") {
            emisi_per_hari = hitungLampuKoridor(item.jumlah, item.jam_operasi_per_hari);
        }
        else if (item. kategori == "CCTV") {
            emisi_per_hari = hitungCCTV(item.jumlah, item.jam_operasi_per_hari);
        }
        else if (item.kategori == "Kulkas") {
            emisi_per_hari = hitungKulkas(item.jumlah, item.jam_operasi_per_hari);
        }
        
        total_emisi_per_hari += emisi_per_hari;
    }
    
    return total_emisi_per_hari * 365.0;
}

string determineStatus(double emisi_kg, double luas_m2) {
    double emisi_per_m2 = emisi_kg / luas_m2;
    
    if (emisi_per_m2 < THRESHOLD_AMAN) {
        return "AMAN - Gedung Ramah Lingkungan";
    } else if (emisi_per_m2 < 80.0) {
        return "PERLU PERHATIAN - Mendekati Batas Aman";
    } else {
        return "BERBAHAYA - Melebihi Batas Emisi";
    }
}

double getPercentageOfThreshold(double emisi_kg, double luas_m2) {
    double emisi_per_m2 = emisi_kg / luas_m2;
    return (emisi_per_m2 / THRESHOLD_AMAN) * 100.0;
}

// ============================================
// BINDING KE JAVASCRIPT
// ============================================

EMSCRIPTEN_BINDINGS(carbon_calculator) {
    value_object<Item>("Item")
        .field("kategori", &Item::kategori)
        .field("jumlah", &Item::jumlah)
        .field("jam_operasi_per_hari", &Item::jam_operasi_per_hari);
    
    emscripten::function("calculateCarbonEmission", &calculateCarbonEmission);
    emscripten::function("determineStatus", &determineStatus);
    emscripten::function("getPercentageOfThreshold", &getPercentageOfThreshold);
    
    emscripten::function("hitungAC", &hitungAC);
    emscripten::function("hitungLampuPanel", &hitungLampuPanel);
    emscripten::function("hitungLift", &hitungLift);
    emscripten::function("hitungKomputer", &hitungKomputer);
    emscripten::function("hitungServer", &hitungServer);
    emscripten::function("hitungWifi", &hitungWifi);
    emscripten::function("hitungProyektor", &hitungProyektor);
    emscripten::function("hitungPompaAir", &hitungPompaAir);
    emscripten::function("hitungLampuKoridor", &hitungLampuKoridor);
    emscripten::function("hitungCCTV", &hitungCCTV);
    emscripten::function("hitungKulkas", &hitungKulkas);
    
    register_vector<Item>("ItemVector");
}