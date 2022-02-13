# My-CTF-Challenges

## AIS3-2021-EOF-Qual
* Reverse 題目 x 5
* beardrop
    * patch dropbear sshd 的登入邏輯
    * 印象中有在某場 CTF 打過類似的題目, 不過忘記是哪一場的哪一題了, 如果有找到類似的題目可以跟我說一下, 我再將連結更新至此
* dnbd
    * .NET binary 中是存放 CIL, runtime 才會 JIT 為 opcodes 運行, 本題 patch CIL, 在 runtime 時解回來
* passwd_checker_2022
    * 用 TLS Callback patch .rdata 段
* wannaSleep
    * 使用幾個 Windows malware 常見手段
* wannaSleep_revenge
    * 修正版 wannaSleep, 與 wannaSleep 存放在同一個 solution 中

## AIS3-2021-EOF-Final
* Reverse 題目 x 1
* meoware
    * Reflective DLL loader
        * 將 DLL 各部分拆散 (Section Table, Import/Export/TLS/Reloc dirs)
        * 將原 payload DLL 中, 被拆開來的部分清空
    * 直接使用部分 system call
        * 參考 am0nsec/HellsGate
