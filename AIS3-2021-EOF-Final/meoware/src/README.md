# meoware
## Intro
* `meoware.exe` 會連線至 c2
* `python c2.py` 架設 c2 server
* 連線成功後, 在 C2 有以下功能
    * Meow
        * RUN
            * 執行目標執行檔
        * BYE
            * 結束程式
        * EOF
            * 設定 AES key
        * RET
            * 返回
        * WRY
            * 設定 AES encrypted payload
    * Woem
        * 解密 AES, 若先前有先設定好 key 與 encrypted payload, 就能解出 flag 字串

## Build
* 首先編譯 payload dll
* 執行 `python genhdr.py` 產生 `payload_dll.h`
* 將 `payload_dll.h` 放到 `meoware/` 目錄底下後編譯 meoware 即可

## Techs
* Reflective DLL loader
    * 將 DLL 各部分拆散 (Section Table, Import/Export/TLS/Reloc dirs)
    * 將原 payload DLL 中, 被拆開來的部分清空
* 直接使用部分 system call
    * 參考 am0nsec/HellsGate

