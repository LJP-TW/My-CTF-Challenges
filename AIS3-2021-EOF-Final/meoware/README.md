# AIS3-2021-EOF-Final
* meoware
    * Reverse 題
    * Reflective DLL loader
        * 將 DLL 各部分拆散 (Section Table, Import/Export/TLS/Reloc dirs)
        * 將原 payload DLL 中, 被拆開來的部分清空
    * 直接使用部分 system call
        * 參考 am0nsec/HellsGate