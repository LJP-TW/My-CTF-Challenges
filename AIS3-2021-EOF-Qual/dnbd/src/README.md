在執行 run 之前, 會先執行該類別的 static constructor, 其會從 PE hdr 一路爬到 run IR 的位置以及大小, 並且 xor 解回正確的指令

作法:
* dnbd 編譯出來後, 觀察 run IR 的位置在哪以及多大, 更新至 patch.py 並執行
