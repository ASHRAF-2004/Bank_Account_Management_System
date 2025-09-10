# 📄 README.md  

## Project: CCP6120 OOPDS Assignment  
This project contains two C++ programs:  

1. **bank_system.cpp** → Bank Account Management System (Singly Linked List)  
2. **warehouse_system.cpp** → Warehouse Inventory & Shipping System (Stack + Queue)  

---

## 🛠 Requirements  
- A C++ compiler (e.g., **g++**, MinGW, or MSVC).  
- Works on Windows, Linux, or macOS.  
- Optional: Any IDE (Code::Blocks, Dev-C++, Visual Studio, CLion, etc.)  

---

## ▶️ Compilation & Execution  

### 1. Bank Account Management System (`bank_system.cpp`)  

**Compile:**  
```bash
g++ bank_system.cpp -o bank_system
```

**Run:**  
```bash
./bank_system
```

**Notes:**  
- This program uses **binary files** for storage:  
  - `accounts.dat` → stores account details.  
  - `logs.dat` → stores transaction logs.  
- Make sure the program has permission to create/read/write these files in the same folder.  
- **Default passwords:**  
  - Admin → `1111`  
  - Staff → `2222`  
- **Features:**  
  - Create/Delete account  
  - Deposit/Withdraw/Transfer  
  - ATM & CDM panels  
  - Admin & Staff login  
  - Logs management  

---

### 2. Warehouse Inventory & Shipping System (`warehouse_system.cpp`)  

**Compile:**  
```bash
g++ warehouse_system.cpp -o warehouse_system
```

**Run:**  
```bash
./warehouse_system
```

**Features:**  
- **Stack** manages incoming items (LIFO).  
- **Queue** manages outgoing shipments (FIFO).  
- Menu options:  
  1. Add Incoming Item  
  2. Process Incoming Item (moves from stack → queue)  
  3. Ship Item  
  4. View Last Incoming Item  
  5. View Next Shipment  
  6. Exit  

---


