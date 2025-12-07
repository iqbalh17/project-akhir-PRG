// Thin JS UI layer — panggil ItemManager (C++)
let itemManager;

// Load WASM module (assumes build/carbon_calculator.js available)
CarbonCalculator().then(function(module) {
    if (!module.ItemManager) {
        console.error('ItemManager tidak ditemukan di WASM module', module);
        document.getElementById('loading').innerHTML = '<h2 style="color:#e74c3c">Module tidak expose ItemManager</h2>';
        return;
    }

    itemManager = new module.ItemManager();

    document.getElementById('loading').style.display = 'none';
    document.getElementById('app').style.display = 'block';
    console.log('C++ Module berhasil dimuat');

    updateItemList();
    updateDropdownOptions();
}).catch(err => {
    document.getElementById('loading').innerHTML =
        '<h2 style="color: #e74c3c;">Error Memuat Module</h2>' +
        '<p style="color: #636e72; margin-top: 15px;">' + err + '</p>';
    console.error('Error loading WASM:', err);
});

// Update dropdown (disable used categories)
function updateDropdownOptions() {
    if (!itemManager) return;
    const select = document.getElementById('kategori');
    const allOptions = Array.from(select.querySelectorAll('option'));

    let usedVal;
    try { usedVal = itemManager.getUsedCategories(); } catch(e) { usedVal = null; console.error(e); }
    const used = [];
    if (usedVal && typeof usedVal.size === 'function') {
        for (let i=0;i<usedVal.size();++i) used.push(usedVal.get(i));
    }

    allOptions.forEach(option => {
        if (option.value === '') return;
        if (used.includes(option.value)) {
            option.disabled = true;
            if (!option.dataset.orig) option.dataset.orig = option.textContent;
            option.textContent = option.dataset.orig + ' (Sudah dipilih)';
        } else {
            option.disabled = false;
            if (option.dataset.orig) option.textContent = option.dataset.orig;
        }
    });
}

// Add / Update item (delegated to C++)
function tambahItem() {
    if (!itemManager) return;
    const kategori = document.getElementById('kategori').value;
    const jumlah = parseInt(document.getElementById('jumlah').value) || 0;
    const jam = parseFloat(document.getElementById('jamOperasi').value) || 0;

    const res = itemManager.addItem(kategori, jumlah, jam);
    if (res !== 'ADD_SUCCESS' && res !== 'UPDATE_SUCCESS') {
        alert(res);
        return;
    }
    if (res === 'UPDATE_SUCCESS') {
        const btn = document.querySelector('.btn-primary');
        if (btn) { btn.textContent = 'Tambah ke Daftar'; btn.style.background = ''; }
    }
    updateItemList();
    updateDropdownOptions();
    document.getElementById('jumlah').value = '';
    document.getElementById('jamOperasi').value = '';
    document.getElementById('kategori').selectedIndex = 0;
    document.getElementById('jumlah').focus();
}

// Render item list using C++ vector<Item>
function updateItemList() {
    if (!itemManager) return;
    const container = document.getElementById('itemList');

    let listVal;
    try { listVal = itemManager.getItemList(); } catch(e) { console.error(e); container.innerHTML=''; return; }

    if (!listVal || typeof listVal.size !== 'function') { container.innerHTML = ''; return; }

    const count = listVal.size();
    if (count === 0) { container.innerHTML = ''; return; }

    let html = `<h3>Daftar Peralatan (${count} item)</h3>`;
    for (let i=0;i<count;++i) {
        const it = listVal.get(i);
        const kategori = it.kategori;
        const jumlah = it.jumlah;
        const jam = it.jam_operasi_per_hari;
        const isEditing = false; // editing state kept in C++ but embind value_object doesn't auto include isEditing here
        html += `<div class="item-entry ${isEditing ? 'editing' : ''}">
            <span><strong>${kategori}</strong> - ${jumlah} unit × ${jam} jam/hari</span>
            <div class="item-actions">
                <button onclick="editItem(${i})" class="btn btn-edit">Edit</button>
                <button onclick="hapusItem(${i})" class="btn btn-delete">Hapus</button>
            </div>
        </div>`;
    }
    container.innerHTML = html;
}

// Edit: request Item from C++
function editItem(index) {
    if (!itemManager) return;
    const itm = itemManager.startEdit(index);
    if (!itm || !itm.kategori) { alert('Item tidak ditemukan'); return; }

    document.getElementById('kategori').value = itm.kategori;
    document.getElementById('jumlah').value = itm.jumlah;
    document.getElementById('jamOperasi').value = itm.jam_operasi_per_hari;

    const btn = document.querySelector('.btn-primary');
    if (btn) { btn.textContent = '✅ Update Item'; btn.style.background = 'linear-gradient(135deg, #f39c12 0%, #e67e22 100%)'; }

    updateItemList();
    updateDropdownOptions();
    document.querySelector('.form-group').scrollIntoView({behavior:'smooth', block:'start'});
    document.getElementById('jumlah').focus();
}

// Delete in C++
function hapusItem(index) {
    if (!itemManager) return;
    if (!confirm('Yakin ingin menghapus item ini?')) return;
    const ok = itemManager.deleteItem(index);
    if (!ok) { alert('Gagal menghapus item'); return; }
    // reset form if not editing
    if (!itemManager.isEditing()) {
        const btn = document.querySelector('.btn-primary');
        if (btn) { btn.textContent = 'Tambah ke Daftar'; btn.style.background = ''; }
        document.getElementById('jumlah').value = '';
        document.getElementById('jamOperasi').value = '';
        document.getElementById('kategori').selectedIndex = 0;
    }
    updateItemList();
    updateDropdownOptions();
}

// Hitung - use C++ calculation + formatResult
function hitungKarbon() {
    if (!itemManager) return;
    const nama = document.getElementById('namaGedung').value.trim();
    const luas = parseFloat(document.getElementById('luasGedung').value) || 0;
    const err = itemManager.validateCalculation(nama, luas);
    if (err) { alert(err); return; }
    const html = itemManager.formatResult(nama, luas);
    document.getElementById('result').innerHTML = html;
    document.getElementById('result').scrollIntoView({behavior:'smooth', block:'nearest'});
}

document.addEventListener('DOMContentLoaded', function() {
    const jamInput = document.getElementById('jamOperasi');
    if (jamInput) jamInput.addEventListener('keypress', function(e){ if (e.key === 'Enter') tambahItem(); });
});