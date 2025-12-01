let items = [];
let Module;
let editingIndex = -1;

// Load WASM module
CarbonCalculator().then(function(module) {
    Module = module;
    document.getElementById('loading').style.display = 'none';
    document.getElementById('app').style.display = 'block';
    console.log('C++ Module berhasil dimuat!');
}).catch(err => {
    document.getElementById('loading').innerHTML = 
        '<h2 style="color: #e74c3c;">Error Memuat Module</h2>' +
        '<p style="color: #636e72; margin-top: 15px;">' + err + '</p>';
    console.error('Error loading WASM:', err);
});


function updateDropdownOptions() {
    const select = document.getElementById('kategori');
    const allOptions = select.querySelectorAll('option');
    
    const usedKategori = items
        .map((item, index) => index === editingIndex ? null : item.kategori)
        .filter(k => k !== null);
    
    allOptions.forEach(option => {
        if (option.value === '') return;
        
        const originalValue = option.value;
        
        if (usedKategori.includes(originalValue)) {
            option.disabled = true;
            if (!option.dataset.originalText) {
                option.dataset.originalText = option.textContent;
            }
            option.textContent = option.dataset.originalText + ' (Sudah dipilih)';
        } else {
            option.disabled = false;
            if (option.dataset.originalText) {
                option.textContent = option.dataset.originalText;
            }
        }
    });
}

function tambahItem() {
    const kategori = document.getElementById('kategori').value;
    const jumlah = parseInt(document.getElementById('jumlah').value);
    const jamOperasi = parseFloat(document.getElementById('jamOperasi').value);

    if (!kategori || kategori === '') {
        alert('Pilih kategori peralatan terlebih dahulu.');
        document.getElementById('kategori').focus();
        return;
    }

    if (!jumlah || !jamOperasi) {
        alert('Harap isi semua field.');
        return;
    }

    if (jumlah <= 0 || jamOperasi < 0 || jamOperasi > 24) {
        alert('Nilai tidak valid. Jumlah harus > 0 dan jam operasi harus 0–24.');
        return;
    }

    const item = {
        kategori: kategori,
        jumlah: jumlah,
        jam_operasi_per_hari: jamOperasi
    };

    if (editingIndex >= 0) {
        items[editingIndex] = item;
        editingIndex = -1;
        document.querySelector('.btn-primary').textContent = 'Tambah ke Daftar';
        document.querySelector('.btn-primary').style.background = '';
    } else {
        items.push(item);
    }

    updateItemList();
    updateDropdownOptions();
    
    document.getElementById('jumlah').value = '';
    document.getElementById('jamOperasi').value = '';
    document.getElementById('kategori').selectedIndex = 0;
    document.getElementById('jumlah').focus();
}

function updateItemList() {
    const itemListDiv = document.getElementById('itemList');
    
    if (items.length > 0) {
        itemListDiv.innerHTML = `
            <h3>Daftar Peralatan (${items.length} item)</h3>
            ${items.map((item, index) => `
                <div class="item-entry ${editingIndex === index ? 'editing' : ''}">
                    <span>
                        <strong>${item.kategori}</strong> - 
                        ${item.jumlah} unit ${item.jam_operasi_per_hari} jam/hari
                    </span>
                    <div class="item-actions">
                        <button onclick="editItem(${index})" class="btn btn-edit" title="Edit">
                            Edit
                        </button>
                        <button onclick="hapusItem(${index})" class="btn btn-delete" title="Hapus">
                            Hapus
                        </button>
                    </div>
                </div>
            `).join('')}
        `;
    } else {
        itemListDiv.innerHTML = '';
    }
}

function editItem(index) {
    const item = items[index];
    
    document.getElementById('kategori').value = item.kategori;
    document.getElementById('jumlah').value = item.jumlah;
    document.getElementById('jamOperasi').value = item.jam_operasi_per_hari;
    
    editingIndex = index;
    updateDropdownOptions();
    
    const btn = document.querySelector('.btn-primary');
    btn.textContent = 'Update Item';
    btn.style.background = 'linear-gradient(135deg, #f39c12 0%, #e67e22 100%)';
    
    updateItemList();
    document.querySelector('.form-group').scrollIntoView({ behavior: 'smooth', block: 'start' });
    document.getElementById('jumlah').focus();
}

function hapusItem(index) {
    if (!confirm('Yakin ingin menghapus item ini?')) return;
    
    if (editingIndex === index) {
        editingIndex = -1;
        document.querySelector('.btn-primary').textContent = 'Tambah ke Daftar';
        document.querySelector('.btn-primary').style.background = '';
        document.getElementById('jumlah').value = '';
        document.getElementById('jamOperasi').value = '';
        document.getElementById('kategori').selectedIndex = 0;
    } else if (editingIndex > index) {
        editingIndex--;
    }
    
    items.splice(index, 1);
    updateItemList();
    updateDropdownOptions();
}

function hitungKarbon() {
    if (editingIndex >= 0) {
        alert('Selesaikan proses edit terlebih dahulu.');
        return;
    }
    
    if (!Module) {
        alert('Module C++ belum dimuat.');
        return;
    }

    if (items.length === 0) {
        alert('Tambahkan minimal satu peralatan.');
        return;
    }

    const luas = parseFloat(document.getElementById('luasGedung').value);
    const nama = document.getElementById('namaGedung').value.trim();

    if (!luas || !nama) {
        alert('Harap isi nama dan luas gedung.');
        return;
    }

    if (luas <= 0) {
        alert('Luas gedung harus lebih dari 0.');
        return;
    }

    try {
        const itemVector = new Module.ItemVector();
        items.forEach(item => itemVector.push_back(item));

        const totalEmisiKg = Module.calculateCarbonEmission(itemVector);
        const status = Module.determineStatus(totalEmisiKg, luas);
        const percentage = Module.getPercentageOfThreshold(totalEmisiKg, luas);

        itemVector.delete();

        let statusClass = 'status-aman';
        if (status.includes('PERHATIAN')) statusClass = 'status-perhatian';
        if (status.includes('BERBAHAYA')) statusClass = 'status-bahaya';

        document.getElementById('result').innerHTML = `
            <div class="card result ${statusClass}">
                <h2>Hasil Perhitungan: ${nama}</h2>
                <div class="result-detail">
                    <p><strong>Total Emisi Karbon:</strong> ${totalEmisiKg.toFixed(2)} kg CO₂/tahun</p>
                    <p><strong>Dalam Ton:</strong> ${(totalEmisiKg / 1000).toFixed(3)} ton CO₂/tahun</p>
                    <p><strong>Emisi per m²:</strong> ${(totalEmisiKg / luas).toFixed(2)} kg CO₂/m²/tahun</p>
                    <p><strong>Persentase dari Threshold:</strong> ${percentage.toFixed(1)}%</p>
                    <p><strong>Standar Green Building:</strong> < 50 kg CO₂/m²/tahun</p>
                </div>
                <h3>Status: ${status}</h3>
                ${percentage < 100 
                    ? '<p style="margin-top: 20px; font-size: 1.15em; color: #27ae60;">Gedung memenuhi standar green building.</p>' 
                    : '<p style="margin-top: 20px; font-size: 1.15em; color: #e67e22;">Konsumsi energi perlu dikurangi.</p>'}
            </div>
        `;

        document.getElementById('result').scrollIntoView({ behavior: 'smooth', block: 'nearest' });

    } catch (error) {
        alert('Error saat menghitung: ' + error);
        console.error('Calculation error:', error);
    }
}

document.addEventListener('DOMContentLoaded', function() {
    const jamInput = document.getElementById('jamOperasi');
    if (jamInput) {
        jamInput.addEventListener('keypress', function(e) {
            if (e.key === 'Enter') tambahItem();
        });
    }
    
    updateDropdownOptions();
});
