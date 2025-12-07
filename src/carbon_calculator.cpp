#include <emscripten/bind.h>
#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <iomanip>

using namespace emscripten;
using std::string;
using std::vector;
using std::map;

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
    std::string kategori;
    int jumlah;
    double jam_operasi_per_hari;
    
    Item() : kategori(""), jumlah(0), jam_operasi_per_hari(0.0) {}
    Item(const std::string& k, int j, double jam) : kategori(k), jumlah(j), jam_operasi_per_hari(jam) {}
};

// ============================================
// ITEM MANAGER (BUSINESS LOGIC DI C++)
// ============================================

class ItemManager {
private:
    std::vector<Item> items;
    int editingIndex;
    std::map<std::string,double> emissionFactors;

    void initFactors() {
        emissionFactors["AC"] = FACTOR_AC;
        emissionFactors["Lampu Panel LED"] = FACTOR_LAMPU_PANEL;
        emissionFactors["Lampu Koridor"] = FACTOR_LAMPU_KORIDOR;
        emissionFactors["Lift"] = FACTOR_LIFT;
        emissionFactors["Komputer"] = FACTOR_KOMPUTER;
        emissionFactors["Server"] = FACTOR_SERVER;
        emissionFactors["WiFi"] = FACTOR_WIFI;
        emissionFactors["Proyektor"] = FACTOR_PROYEKTOR;
        emissionFactors["Pompa Air"] = FACTOR_POMPA_AIR;
        emissionFactors["CCTV"] = FACTOR_CCTV;
        emissionFactors["Kulkas"] = FACTOR_KULKAS;
    }

    double emissionFor(const Item& it) const {
        auto itf = emissionFactors.find(it.kategori);
        if (itf == emissionFactors.end()) return 0.0;
        // emisi per tahun: jumlah * jam * factor * 365
        return it.jumlah * it.jam_operasi_per_hari * itf->second * 365.0;
    }

public:
    ItemManager() : editingIndex(-1) { initFactors(); }

    // validation for add/update
    std::string validateInput(const std::string& kategori, int jumlah, double jamOperasi) {
        if (kategori.empty()) return "Pilih kategori peralatan terlebih dahulu.";
        if (jumlah <= 0) return "Jumlah harus lebih dari 0.";
        if (jamOperasi < 0.0 || jamOperasi > 24.0) return "Jam operasi harus antara 0-24.";
        // duplicate check:
        if (editingIndex == -1) {
            for (size_t i=0;i<items.size();++i) if (items[i].kategori == kategori) return "Kategori sudah dipilih. Gunakan Edit untuk mengubah.";
        } else {
            for (size_t i=0;i<items.size();++i) if ((int)i != editingIndex && items[i].kategori == kategori) return "Kategori sudah dipilih. Gunakan Edit untuk mengubah.";
        }
        return "";
    }

    // add or update item; returns "ADD_SUCCESS" or "UPDATE_SUCCESS" or error message
    std::string addItem(const std::string& kategori, int jumlah, double jamOperasi) {
        std::string v = validateInput(kategori, jumlah, jamOperasi);
        if (!v.empty()) return v;
        Item it(kategori, jumlah, jamOperasi);
        if (editingIndex >= 0) {
            items[editingIndex] = it;
            editingIndex = -1;
            return "UPDATE_SUCCESS";
        } else {
            items.push_back(it);
            return "ADD_SUCCESS";
        }
    }

    // start editing - return Item (empty kategori => invalid)
    Item startEdit(int index) {
        if (index < 0 || index >= (int)items.size()) return Item();
        editingIndex = index;
        return items[index];
    }

    void cancelEdit() { editingIndex = -1; }
    bool isEditing() const { return editingIndex >= 0; }
    int getEditingIndex() const { return editingIndex; }

    bool deleteItem(int index) {
        if (index < 0 || index >= (int)items.size()) return false;
        items.erase(items.begin() + index);
        if (editingIndex == index) editingIndex = -1;
        else if (editingIndex > index) editingIndex--;
        return true;
    }

    // Return registered vector<Item> so JS can use .size()/.get(i)
    std::vector<Item> getItemList() const {
        return items;
    }

    int getItemCount() const { return (int)items.size(); }

    // Return vector<string> of used categories (excluding item being edited)
    std::vector<std::string> getUsedCategories() const {
        std::vector<std::string> out;
        for (size_t i=0;i<items.size();++i) if ((int)i != editingIndex) out.push_back(items[i].kategori);
        return out;
    }

    // calculation
    double calculateCarbonEmission() const {
        double total = 0.0;
        for (const auto &it : items) total += emissionFor(it);
        return total;
    }

    std::string determineStatus(double emisi_kg, double luas_m2) const {
        if (luas_m2 <= 0.0) return std::string("ERROR: Luas tidak valid");
        double emisi_per_m2 = emisi_kg / luas_m2;
        if (emisi_per_m2 < THRESHOLD_AMAN) return "AMAN - Gedung Ramah Lingkungan";
        else if (emisi_per_m2 < THRESHOLD_AMAN * 1.6) return "PERLU PERHATIAN - Mendekati Batas Aman";
        else return "BERBAHAYA - Melebihi Batas Emisi";
    }

    double getPercentageOfThreshold(double emisi_kg, double luas_m2) const {
        if (luas_m2 <= 0.0) return 0.0;
        double emisi_per_m2 = emisi_kg / luas_m2;
        return (emisi_per_m2 / THRESHOLD_AMAN) * 100.0;
    }

    // format result to HTML string (simple)
    std::string formatResult(const std::string& namaGedung, double luas) const {
        std::string err = validateCalculation(namaGedung, luas);
        if (!err.empty()) return std::string("<p style='color:#e74c3c;'>") + err + "</p>";

        double totalEmisi = calculateCarbonEmission();
        std::string status = determineStatus(totalEmisi, luas);
        double perc = getPercentageOfThreshold(totalEmisi, luas);

        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2);
        std::string statusClass = "status-aman";
        if (status.find("PERLU PERHATIAN") != std::string::npos) statusClass = "status-perhatian";
        if (status.find("BERBAHAYA") != std::string::npos) statusClass = "status-bahaya";

        oss << "<div class='card result " << statusClass << "'>";
        oss << "<h2>Hasil Perhitungan: " << namaGedung << "</h2>";
        oss << "<div class='result-detail'>";
        oss << "<p><strong>Total Emisi Karbon:</strong> " << totalEmisi << " kg CO₂/tahun</p>";
        oss << std::setprecision(3);
        oss << "<p><strong>Dalam Ton:</strong> " << (totalEmisi / 1000.0) << " ton CO₂/tahun</p>";
        oss << std::setprecision(2);
        oss << "<p><strong>Emisi per m²:</strong> " << (totalEmisi / luas) << " kg CO₂/m²/tahun</p>";
        oss << std::setprecision(1);
        oss << "<p><strong>Persentase dari Threshold:</strong> " << perc << "%</p>";
        oss << "<p><strong>Standar Green Building:</strong> &lt; 50 kg CO₂/m²/tahun</p>";
        oss << "</div>";
        oss << "<h3>Status: " << status << "</h3>";
        if (perc < 100.0) {
            oss << "<p style='margin-top:20px;color:#27ae60;'>Gedung memenuhi standar green building.</p>";
        } else {
            oss << "<p style='margin-top:20px;color:#e67e22;'>Konsumsi energi perlu dikurangi.</p>";
        }
        oss << "</div>";
        return oss.str();
    }

    std::string validateCalculation(const std::string& nama, double luas) const {
        if (editingIndex >= 0) return "Selesaikan proses edit terlebih dahulu.";
        if (items.empty()) return "Tambahkan minimal satu peralatan.";
        if (nama.empty()) return "Masukkan nama gedung.";
        if (luas <= 0.0) return "Luas gedung harus lebih dari 0.";
        return "";
    }
};

// ============================================
// BINDINGS
// ============================================

EMSCRIPTEN_BINDINGS(carbon_calculator_module) {
    value_object<Item>("Item")
        .field("kategori", &Item::kategori)
        .field("jumlah", &Item::jumlah)
        .field("jam_operasi_per_hari", &Item::jam_operasi_per_hari);

    register_vector<Item>("ItemVector");
    register_vector<std::string>("StringVector");

    class_<ItemManager>("ItemManager")
        .constructor<>()
        .function("validateInput", &ItemManager::validateInput)
        .function("addItem", &ItemManager::addItem)
        .function("startEdit", &ItemManager::startEdit)
        .function("cancelEdit", &ItemManager::cancelEdit)
        .function("deleteItem", &ItemManager::deleteItem)
        .function("getItemList", &ItemManager::getItemList)
        .function("getItemCount", &ItemManager::getItemCount)
        .function("getUsedCategories", &ItemManager::getUsedCategories)
        .function("isEditing", &ItemManager::isEditing)
        .function("getEditingIndex", &ItemManager::getEditingIndex)
        .function("calculateCarbonEmission", &ItemManager::calculateCarbonEmission)
        .function("determineStatus", &ItemManager::determineStatus)
        .function("getPercentageOfThreshold", &ItemManager::getPercentageOfThreshold)
        .function("formatResult", &ItemManager::formatResult)
        .function("validateCalculation", &ItemManager::validateCalculation);
}