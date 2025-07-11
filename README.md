# Timer_multi_controller
Program ini dirancang untuk mengontrol relay berdasarkan empat mode waktu yang dapat diatur melalui keypad, dan dijalankan dengan saklar manual. Sistem ini cocok digunakan untuk aplikasi seperti kontrol mesin twisting, otomatisasi produksi, atau sistem pengendalian urutan waktu.

🎯 Fitur Utama:
✅ 4 Mode Waktu (A, B, C, D):

Masing-masing mode memiliki waktu durasi tersendiri dalam milidetik (ms).

Dapat diatur menggunakan keypad (misalnya: tekan A → ketik 1500 → tekan # untuk menyimpan).

🔁 Otomatis Berurutan:

Saat saklar ditekan, sistem akan mengeksekusi mode aktif (A, B, dst), mengaktifkan relay sesuai waktu yang ditentukan, lalu lanjut ke mode berikutnya secara otomatis.

Jika suatu mode waktunya 0 ms, maka mode tersebut dilewati otomatis.

🧠 Manajemen Mode:

Menentukan mode berikutnya secara dinamis sesuai urutan dan nilai waktu yang aktif.

Mode A → B → C → D → A dan seterusnya.

🔢 Keypad Input:

Tombol A–D: untuk masuk ke mode input waktu.

Angka 0–9: masukkan durasi dalam milidetik.

Tombol #: menyimpan waktu yang dimasukkan.

Tombol *: reset semua waktu ke 0.

💡 LCD Display 16x2 I2C:

Menampilkan status sistem, mode aktif, mode berikutnya, waktu dalam detik, dan konfirmasi saat menyimpan atau reset.

🔁 Debounce Saklar:

Menggunakan logika debounce untuk menghindari pembacaan ganda saat saklar ditekan.

⚙️ Spesifikasi Hardware:
Arduino UNO / Mega

LCD 16x2 I2C

Keypad 4x4

Relay Module

Saklar Tekan (Push Button)

Pin-Pin:

Relay → Pin 2

Saklar → Pin 3

Keypad → Pin 22–36 (baris dan kolom)

🔁 Alur Umum Program:
Saat Arduino dinyalakan:

Menampilkan splash screen (judul dan nama pembuat).

Menampilkan layar utama: "Sistem Ready".

Operator menekan saklar:

Sistem mulai menjalankan mode aktif → aktifkan relay → tunggu selesai → lanjut ke mode berikutnya.

Operator dapat mengatur waktu:

Tekan tombol A, B, C, atau D → masukkan angka → tekan # → waktu disimpan.

Untuk mereset semua waktu:

Tekan tombol * → semua waktu di-set ke 0 → mode kembali ke IDLE.

✍️ Catatan Tambahan:
Program menggunakan sistem waktu non-blocking (millis) agar tetap responsif.

Fungsi safeMillisDiff() digunakan untuk menghindari masalah overflow millis().
