# Tugas Besar IF2224 Teori Bahasa Formal dan Otomata
![FOTO KELOMPOK](SBT_assset.png)

## Milestone 2: Syntax Analysis

### 1. Identitas Kelompok
**Kelompok: SBT-Tubes-IF2224-2026**
* 13524032 - Juan Oloando Simanungkalit
* 13524036 - Edward David Rumahorbo
* 13524056 - Reinhard Alfonzo Hutabarat
* 13524102 - Manuel Timothy Silalahi

### 2. Deskripsi Program
Program ini adalah **Syntax Analyzer (Parser)** untuk bahasa pemrograman *Arion*, yang dibangun sebagai pemenuhan Milestone 2 Tugas Besar IF2224. Program ini merupakan kelanjutan dari Milestone 1 (Lexical Analyzer) dan bertugas menerima *token stream* dari Lexer, menganalisis strukturnya secara sintaksis, dan membangun **Parse Tree** yang merepresentasikan hierarki struktur program sesuai grammar bahasa Arion.

Parser diimplementasikan secara modular menggunakan bahasa **C++ standar GNU** dengan algoritma **Recursive Descent Parsing**. Grammar bahasa Arion di-hardcode langsung ke dalam fungsi-fungsi parser tanpa menggunakan library atau tools pembangkit parser otomatis. Program menerima source code Arion dalam format `.txt`, memprosesnya melalui pipeline Lexer → Parser, dan mencetak Parse Tree ke terminal serta menyimpannya ke dalam berkas keluaran `.txt`.

### 3. Requirements
Untuk mengompilasi dan menjalankan program ini, diperlukan perangkat lunak berikut:
* **GNU C++ Compiler (GCC):** Kompilator `g++` yang mendukung standar C++17 atau lebih baru.
* **Make Utility:** Dibutuhkan untuk mengeksekusi skrip kompilasi otomatis (`Makefile`).
* **Sistem Operasi:** Bebas (Linux, Windows menggunakan MinGW/WSL, atau macOS).

### 4. Cara Instalasi dan Penggunaan Program

1. **Kloning Repositori**
   Unduh kode program ke direktori lokal Anda:
   ```bash
   git clone https://github.com/Rizelbit/SBT-Tubes-IF2224-2026
   cd SBT-Tubes-IF2224-2026
   ```

2. **Kompilasi Program**
   Pastikan Anda berada di akar repositori. Jalankan perintah berikut:
   ```bash
   make
   ```
   Perintah ini akan mengompilasi seluruh berkas C++ modular (Lexer dan Parser) menjadi sebuah *executable* bernama `parser`.

3. **Cara Menjalankan Program**
   Letakkan file input source code Arion di dalam direktori `test/milestone-2/`, kemudian jalankan:
   ```bash
   ./parser <nama_file_input.txt> <nama_file_output.txt>
   ```
   *Contoh Eksekusi Pengujian:*
   ```bash
   ./parser input1.txt output1.txt
   ```
   Parse Tree hasil analisis akan dicetak ke terminal dan disimpan di `test/milestone-2/output1.txt`.

4. **Membersihkan File Build**
   ```bash
   make clean
   ```

### 5. Struktur Direktori
```
SBT-Tubes-IF2224-2026/
├── Makefile
├── README.md
├── src/
│   ├── Token.hpp / Token.cpp           (Definisi token dan TokenType)
│   ├── Lexer.hpp / Lexer.cpp           (Dispatcher utama Lexer)
│   ├── Lexer_Literals.cpp              (DFA untuk literal: int, real, char, string)
│   ├── Lexer_Operators.cpp             (DFA untuk operator dan komparasi)
│   ├── Lexer_Keywords.cpp              (DFA untuk keyword dan identifier)
│   ├── Lexer_DelimsComments.cpp        (DFA untuk delimiter dan komentar)
│   ├── ParseNode.hpp / ParseNode.cpp   (Struktur data node Parse Tree)
│   ├── Parser.hpp                      (Deklarasi kelas Parser)
│   ├── Parser.cpp                      (Utilitas inti: match, peekToken, reportError)
│   ├── Parser_Core.cpp                 (program, block, statement, statement-list)
│   ├── Parser_Declarations.cpp         (const, type, var, array, record, enum, range)
│   ├── Parser_ControlFlow.cpp          (if, case, while, repeat, for, procedure, function)
│   ├── Parser_Expression.cpp           (expression, term, factor, variable, assignment, call)
│   └── Main.cpp                        (Entry point program)
├── doc/
│   └── Laporan-2-SBT.pdf
└── test/
    ├── milestone-1/
    │   ├── input1.txt ... input5.txt
    │   └── output1.txt ... output5.txt
    └── milestone-2/
        ├── input1.txt ... input5.txt
        └── output1.txt ... output5.txt
```

### 6. Pembagian Tugas

#### Milestone 1 — Lexical Analysis
* **13524032 - Juan Oloando Simanungkalit (25%)**: Mengembangkan DFA Delimiters & Comments (`Lexer_DelimsComments.cpp`), merancang diagram DFA, dan menyusun laporan.
* **13524036 - Edward David Rumahorbo (25%)**: Mengembangkan DFA Operators & Komparasi (`Lexer_Operators.cpp`), merancang diagram DFA, dan menyusun laporan.
* **13524056 - Reinhard Alfonzo Hutabarat (25%)**: Mengembangkan DFA Keywords & Identifier (`Lexer_Keywords.cpp`), merancang diagram DFA, dan menyusun laporan.
* **13524102 - Manuel Timothy Silalahi (25%)**: Mengembangkan DFA Literals (`Lexer_Literals.cpp`), merancang diagram DFA, dan menyusun laporan.

#### Milestone 2 — Syntax Analysis
* **13524032 - Juan Oloando Simanungkalit (25%)**: Mengembangkan struktur data Parse Tree (`ParseNode.hpp/cpp`), utilitas inti Parser (`Parser.hpp`, `Parser.cpp`), dan struktur kerangka program (`Parser_Core.cpp`).
* **13524036 - Edward David Rumahorbo (25%)**: Mengembangkan parser untuk deklarasi dan tipe data (`Parser_Declarations.cpp`), mencakup const, type, var, array, record, enum, dan range.
* **13524056 - Reinhard Alfonzo Hutabarat (25%)**: Mengembangkan parser untuk control flow dan subprogram (`Parser_ControlFlow.cpp`), mencakup if, case, while, repeat, for, procedure, dan function.
* **13524102 - Manuel Timothy Silalahi (25%)**: Mengembangkan parser untuk ekspresi dan pernyataan (`Parser_Expression.cpp`), mencakup expression, term, factor, variable, assignment, dan procedure/function call.