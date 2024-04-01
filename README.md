# LED Matrix Clock

## Deskripsi Sistem
Sistem menampilkan jam, kalender, suhu dan alarm dengan karakter pada dot matrix LED 8 x 32 (4 buah matrix LED 8x8), dengan input keyboard USB. Suhu diukur dengan menggunakan sensor suhu analog (LM35).
Fitur yang ada pada sistem antara lain mode run, set jam, set tanggal, dan set alarm. Pada saat set jam maka akan menampilkan jam, menit dan detik. Sedangkan saat set tanggal maka akan menampilkan tanggal, bulan, dan tahun. Untuk set alarm, dibagi menjadi 3 yaitu alarm 1, alarm 2, dan alarm 3. Alarm 1 akan menampilkan display NRP, alarm 2 akan menampilkan Display NRP dan Nama, alarm 3 akan menampilkan display text yang diinputkan melalui keyboard. Masing masing alarm bisa diatur dengan tanggal, jam, ment dan durasinya dalam detik.
Untuk tampilan dot matrix, pada tiap detik 10 dan 40 akan menampilkan tanggal, bulan tahun selama 3 detik. Dan pada detik ke 13 dan 43 akan menampilkan suhu dengan keterangan derajat celcius. Pada saat menekan alarm, maka langsung tampilkan text alarm yang sesuai dengan pengaturannya. Kecepatan geser tampilan text panjang adalah 0.5 detik per kolom matrix. Untuk tingkat kecerahan tampilan akan diatur sesuai kecerahan lingkungan dengan menggunakan sensor cahaya.

## Rancangan Perangkat Keras
Rancangan sistem menggunakan Microcontroller Atmega328p pada board Arduino NANO. Perangkat keras penunjang lainnya antara lain:
* LED Dot Matrix 8x32
  <p align="left">
    <img src="/additional/Matrix8x32.png" width="250">
  </p>
* Modul sensor cahaya (LDR)
  <p align="left">
    <img src="/additional/ldr.png" width="250">
  </p>
* Sensor suhu (LM35)
  <p align="left">
    <img src="/additional/lm35.png" width="250">
  </p>
* RTC DS3231
  <p align="left">
    <img src="/additional/DS3231.png" width="250">
  </p>
* Keyboard PS/2
  <p align="left">
    <img src="/additional/ps2adapter.png" width="250">
  </p>
* Secure Board (untuk menempatkan perangkat dalam posisi aman)
  <p align="left">
    <img src="/additional/pcb.png" width="250">
  </p>
* Kabel jumper
  <p align="left">
    <img src="/additional/kabel.png" width="250">
  </p>
* Solder
  <p align="left">
    <img src="/additional/solder.png" width="250">
  </p>
* Timah Solder
  <p align="left">
    <img src="/additional/timah.png" width="250">
  </p>

## Listing Program
Listing program untuk menjalankan LED Matrix Clock ini bisa dilihat pada [link ini](ClockMatrix.ino)

Catatan : Ubah konfigurasi untuk menyesuaikan sensor yang tersedia

## Cara Kerja Sistem
* Perangkat keras sudah terpasang pada secure board
* Display jam, menit, detik, tanggal, suhu sudah tampil
* Kalibrasi sensor suhu sudah hampir sesuai, walau masih tidak stabil
* Membuat pengaturan kontras cahaya otomatis menggunakan modul LDR
* Klik ENTER untuk mensetting mode yang diinginkan
* Klik ARROW_LEFT / ARROW_RIGHT untuk memilih mode yang ingin diset (TIME, DATE, ALARM)
* Klik ENTER lagi untuk masuk ke mode yang sudah dipilih
* Mode TIME digunakan untuk mengatur waktu saat ini. Kita menginputkan jam ,menit, dan detik (hh:mn:ss menggunakan NUMBER_KEY (0-9)
* Mode DATE digunakan untuk mengatur tanggal saat ini. Kita menginputkan tanggal, bulan dan tahun (dd:mm:yyyy) menggunakan NUMBER_KEY (0-9)
* Mode ALARM digunakan untuk menset alarm. Terdapat 3 jenis alarm, pertama akan menampilkan NRP (5024201013), kedua akan menampilkan nama+NRP (Arriza Fajar Zhafar Yasar 5024201013), dan ketiga akan menampilkan kalimat yang diinputkan menggunakan keyboard
* Pemilihan jenis alarm menggunakan ARROW_LEFT / ARROW_RIGHT
* Pada set alarm ke-3, pertama akan diminta untuk menginputkan huruf/kata/kalimat yang akan ditampilkan saat waktu alarm3 terjadi
* Ketiga alarm akan setelah itu akan diminta jam dan menit kapan alarm akan menyala, dan durasi untuk seberapa lama alarm akan aktif
* Penentuan durasi alarm menggunakan ARROW_UP / ARROW_LEFT. Karena menggunakan integer, maka maksimal durasi hanya 255 second (detik)
* Untuk menghapus tiap inputan (misal sudah menekan NUMBER_KEY) tetapi salah, maka klik tombol BACKSPACE untuk menghapus inputan tersebut sebanyak 1 karakter (karakter yang terakhir kali diinputkan)
* Untuk kembali ke pengaturan sebelumnya (misal dari set waktu ke set mode), maka kita menekan tombol ESC


## Foto Rangkaian
<p align="center">
  <img src="/additional/image2.jpg">
  <img src="/additional/image1.jpg">
  <img src="/additional/image4.jpg">
</p>

## Video Demo
[<img src="/additional/image2.jpg">](https://github.com/ArrCeus13/LED-Matrix-Clock/assets/101047794/833da340-3838-4bc8-a410-efcce8a3c3e5)
