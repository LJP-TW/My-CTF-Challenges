Patch 0x17587 code

做萬用密碼比對, 密碼為 FLAG, 只要使用者名稱的確存在於系統中, 輸入此萬用密碼就能登入

對應 source code 為修改 svr_auth_password() 邏輯

作法:
* 將 .text 段 size 改大
* 在 .text 段尾端插入比對萬用密碼的 code
* 將 0x17587 code 改成跳到 .text 段尾部去跑比對, 對錯則進行對應的事情
* shellcode.py 跑完產出的 IDA script 跑一下, 將 shellcode 插入至 .text 段尾部
* Patch shellcode 中的 nop 改成對應的 jmp
    * auth_ok: 176BA
    * back_to_origin_logic: 1758C
