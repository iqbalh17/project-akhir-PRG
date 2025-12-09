# Project Akhir PRG

Sebuah website edukasi sederhana yang juga menyediakan fitur kalkulator. Website ini terdiri dari halaman edukasi dan halaman kalkulator. Logika kalkulator ditulis dalam C++ dan dikompilasi ke WebAssembly sehingga dapat dijalankan di browser. Interaksi UI dan pengaturan halaman dilakukan dengan JavaScript.

## Fitur
- Halaman edukasi  — konten pembelajaran tentang emisi karbon.
- Halaman kalkulator  — tampilan kalkulator untuk menghitung emisi karbon.

## Teknologi
- HTML, CSS, JavaScript (frontend)
- C++ (logic kalkulator)
- WebAssembly (WASM) untuk menjalankan C++ di web
- Emscripten untuk mengompilasi C++ → WASM

## Struktur project
- index.html
- kalkulator.html
- js/
  - kalkulator.js (kode JS untuk UI dan pemanggilan fungsi WASM)
- cpp/
  - kalkulator.cpp (kode C++ untuk logic kalkulator)
- build/
  - carbon_calculator.js
  - carbon_calculator.wasm
- assets/ (gambar, stylesheet, dsb)

## Menjalankan secara lokal
WebAssembly dan beberapa browser membutuhkan file .wasm dimuat lewat server (bukan file://). Cara cepat menjalankan server lokal:

Dengan Python 3:
```bash
# di root repo
python -m http.server 8000
# buka http://localhost:8000/index.html atau http://localhost:8000/kalkulator.html
```

## Cara penggunaan (end-user)
1. Jalankan server lokal (lihat bagian Menjalankan secara lokal).
2. Buka http://localhost:8000/index.html untuk melihat halaman edukasi.
3. Buka http://localhost:8000/kalkulator.html untuk menggunakan kalkulator.

## Live Demo (Akses Online)

Aplikasi ini juga sudah bisa diakses online tanpa perlu instalasi.

- **URL:** `https://jejakkarbonku.netlify.app/`

