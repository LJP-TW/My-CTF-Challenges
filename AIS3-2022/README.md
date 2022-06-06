# AIS3-2022
* Calculator
    * Reverse 題
    * 利用 .NET Managed Extensibility Framework 提供的功能寫出的廢物計算機, 詳細請自行查閱官方文件
    * 出題動機是最近在看 dnSpy source code, 其裡面用到此功能, 因此就想以此出一題入門題
    * Operator 有 `+`, `-`, `*`, `A`, `{`, `D`, `k`
    * 依據 flag 開頭固定為 `AIS3{`, 可以得知應先從 `A` operator 開始閱讀其邏輯, 並能得知接下來要繼續逆的 operator 順序
    * 將最終拼出的 flag 送進計算機後會發現能夠成功運算, 運算結果為 1337
* Give_Me_SC
    * Pwn 題
    * 出題動機是怕 Pwn 類別沒有水題, 想出一題 shellcode 水題, 但又想讓各位稍微跳脫一點舒適圈, 因此決定出不同指令集的題目
    * Aarch64 Shellcoding, 並起完全沒有任何設防