## HW 27-1 5-Stage Pipelined CPU Modeling

Please provide the commands and all necessary resources for us to reproduce your results. 
```shell
$ BUILD_DIR="build/debug" && \
    cmake -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Debug && \
    cmake --build "${BUILD_DIR}" -j $(nproc)
$ ./build/debug/riscv --asm_file_path src/riscv/asm/target_file
```

### HW 27-1-1 Please provide the screenshots of simulator execution logs for the following test programs
- `riscv/asm/load_store_simple.txt`

![](https://course.playlab.tw/md/uploads/843c7282-3961-4fdc-975a-396f3c825057.png)

- `riscv/asm/full_test.txt`

![](https://course.playlab.tw/md/uploads/2ed024ba-493a-4ebf-83ec-632bfdeb39fb.png)

- `riscv/asm/branch_simple.txt`

![](https://course.playlab.tw/md/uploads/3dbe5021-823f-4e21-a9a5-c603097b9e88.png)

### HW 27-1-2 Correlation evaluation with hardware design

Compare your simulator result with your simulation result in Lab 9. 
- `riscv/asm/load_store_simple.txt`

![](https://course.playlab.tw/md/uploads/c3300eec-0881-42f4-a73b-53c98b7184ef.png)

- `riscv/asm/full_test.txt`

![](https://course.playlab.tw/md/uploads/95cf9484-bcd2-4ed9-b4b2-9ae48e4971f3.png)

- `riscv/asm/branch_simple.txt`

![](https://course.playlab.tw/md/uploads/1a53c2da-53e2-4923-862a-a62f7c96d8f9.png)

Q1. Do they correlate well?

在訂正 Lab 9 之前，Lab 9 的模擬結果與 Lab 27 的模擬結果並沒有顯示正確的 correlation，在訂正 Lab 9 後，兩個 Lab 的模擬結果相同。在 Lab 9 的模擬結果中 "RAN N CYCLES PASSED" 與 Lab 27 模擬結果中的 "Tick=M  Info: [SimTopBase] Simulation complete." 存在 `N = M - 1` 的關係。因為測試 Lab 9 的測資中，在 hcf 前會插入五個 nop instruction (Lab 9 中在 IF 接收到 hcf instruction 就會結束模擬，故插入 5 個 nop instruction 以維持正確模擬結果)，因此 LAB 9 中實際模擬花費的 cycle count = `N - 5`。而 Lab 27 中因為第一筆 instruction 在第二個 cycle 才進入 IF stage，並且模擬時會在 hcf instruction 在 WB stage 執行完成後的下個 cycle 結束模擬，因此如果要跟 Lab 9 比較的話 cycle count 要取 `M - 5 - 1` (減去 hcf 從 IF 到 WB 結束的 cycle 以及延遲一個 cyele 開始)。由 `N - 5 = M - 5 - 1` 可以發現與模擬結果 `N = M - 1` 相符。
<font color=#FF0000>更正：這邊只是要解釋造成 Lab 9 與 Lab 27 執行結果 cycle 數差 4 的原因</font>

Q2. If not, what does the inaccuracy come from? How can you fix it?

在比較兩邊的 cycle count 時，發現 Lab 9 的模擬中在 IF, ID stage 同時遇到因為 data hazard 需要 stall 及因為 control hazard 需要 flush 時會進行 stall 而非 flush，因此影響模擬結果與 cycle count，故在 `Controller.scala` 中 `io.Stall_RAW_DH` 邏輯新增判斷 `!Predict_Miss && ...` 確保 control hazard 的 flush 優先順序較 data hazard 的 stall 高，修改後 Lab 9 的模擬結果與 Lab 27 的模擬結果相符。

## HW 27-2 Target System Modeling

Please provide the commands and all necessary resources for us to reproduce your results. 
```shell
$ BUILD_DIR="build/debug" && \
    cmake -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Debug && \
    cmake --build "${BUILD_DIR}" -j $(nproc)
$ ./build/debug/riscv --asm_file_path src/riscv/asm/target_file
```

#### HW27-2-1 Prepare a NxNxN matrix multiplication test program
For evaluation purpose, you need to prepare an assembly test to do NxNxN matrix multiplication where N is { 2, 4, 8, 16, 32}
```asm
[TODO] paste your test program here
# 五份 asm 格式相同，差別在 dma config, systolic array config
.text
start:
    li   x1, 0x2000
    li   x2, 0x3fff

initialize_loop:
    slli x3, x1, 1
    sw   x3, 0(x1)

    addi x1, x1, 4
    blt  x1, x2, initialize_loop

    li  x1, 0x300000

    li  x2, 0x2000
    li  x3, 0x200000
    li  x4, 0x08080701

    li  x5, 1

    # Program DMA
    sw  x2, 4(x1)
    sw  x3, 8(x1)
    sw  x4, 12(x1)
    sb  x5, 0(x1)

check_complete:
    # Check whether the DMA command completes
    lw  x6, 20(x1)
    and x6, x6, x5
    beq x6, x0, check_complete
    sw  x0, 20(x1)

    li  x2, 0x3000
    li  x3, 0x210000
    li  x4, 0x08080701

    li  x5, 1

    # Program DMA
    sw  x2, 4(x1)
    sw  x3, 8(x1)
    sw  x4, 12(x1)
    sb  x5, 0(x1)

check_complete2:
    # Check whether the DMA command completes
    lw  x6, 20(x1)
    and x6, x6, x5
    beq x6, x0, check_complete2
    sw  x0, 20(x1)

    li  x1, 0x100000
    li  x2, 0x1001
    li  x3, 0x200000
    li  x4, 0x210000
    li  x5, 0x220000
    li  x6, 0x80808
    li  x7, 1

    # Program SA
    sw  x2, 8(x1)
    sw  x2, 12(x1)
    sw  x2, 16(x1)
    sw  x3, 20(x1)
    sw  x4, 24(x1)
    sw  x5, 28(x1)
    sw  x6, 32(x1)
    sb  x7, 0(x1)

check_complete3:
    # Check whether the SA command completes
    lw  x8, 4(x1)
    and x8, x8, x7
    beq x8, x0, check_complete3
    sw  x0, 4(x1)

exit:
    hcf
```

#### HW27-2-2 Run Simulation and Get Results
 Please run the test program on your simulator and produce the following table for submission. 
 
 以下實驗數據均假設
 1. <font color=#FF0000>更正：從 systolic array 將運算結果搬回 internal buffer 的 delay 等於 systolic array width</font>
 2. 在進行 n * n * n 的矩陣運算時，PE array 大小是 n * n
 
 | N | cycles |
 |---|--------|
 | 2| 8347 |
 | 4| 8531 |
 | 8| 9219 |
 |16| 11939 |
 |32| 22755 |

## HW 27-3  Design Trade-off Evaluation - Systolic Array Design

Please provide the commands and all necessary resources for us to reproduce your results. 
```shell
$ BUILD_DIR="build/debug" && \
    cmake -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Debug && \
    cmake --build "${BUILD_DIR}" -j $(nproc)
$ ./build/debug/riscv --asm_file_path src/riscv/asm/target_file
```

 Please run the test program on your simulator and produce the following table for submission. 
 
 以下實驗數據均假設
 1. <font color=#FF0000>更正：從 systolic array 將運算結果搬回 internal buffer 的 delay 等於 systolic array width</font>
 2. <font color=#FF0000>更正：如果運算結果 matrix C 的大小比 systolic array 小，systolic array 會選擇使用部分 PE 進行運算，但是運算結果從 PE 搬回 internal buffer 的 delay 等於 systolic array width，因此即使 matrix C 的大小小於 systolic array，systolic array size 仍可能影響執行 cycle 數</font>
 3. 如果運算結果 matrix C 的大小比 systolic array 大，會將運算分割成多個矩陣乘法，matrix A 會被分割成多個 height = systolic array size 的 sub matrix，matrix B 會被分割成多個 width = systolic array size 的 sub matrix，並將 sub matrix 互相進行矩陣乘法得到 matrix C
 
 | N | 2x2 systolic array |4x4 systolic array |8x8 systolic array |
 |---|--------|--------|--------|
 | 2| 8347 | 8355 | 8355 |
 | 4| 8555 | 8531 | 8539 |
 | 8| 9403 | 9267 | 9219 |
 |16| 13227 | 12315 | 12035 |
 |32| 32107 | 25387 | 23515 |
 

## HW 27-4 Performance Analysis and Comparison

Please provide the commands and all necessary resources for us to reproduce your results. 
```shell
$ BUILD_DIR="build/debug" && \
    cmake -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Debug && \
    cmake --build "${BUILD_DIR}" -j $(nproc)
$ ./build/debug/riscv --asm_file_path src/riscv/asm/target_file
```

Q1. What is the interesting design tradeoff that you want to evaluate?

<font color=#FF0000>更正：在假設從 systolic array 將運算結果搬回 internal buffer 的 delay 等於 systolic array width 的情況下，systolic array 的運算 cycle 數會在 systolic array width 與 output matrix C width 相同時有最小值 (隨著 systolic array width 增加需要更多 cycle 將運算結果從 PE 搬回 internal buffer)。另外當 array 規模達到某個臨界值後，效能提升趨緩，呈現 diminishing return (雖然擴大 systolic array 可加速運算，但超過一定規模後，額外的 PE 對效能的貢獻有限，反而帶來更高的硬體成本與功耗)。
E.g. 在極端情況下 systolic array 大小為 1 * 1 時，進行 32 * 32 * 32 的矩陣運算需要 57451 個 cycle，隨著 systolic array 大小增加 (2, 4, 8, 16)，需要的 cycle 數變成 32107, 25387, 23515, 22947，每次擴大 systolic array 大小的收益由 44%, 21%, 7%, 2% 明顯遞減。

粗略估算在 systolic array 大小為 n 時，進行 32 * 32 * 32 的矩陣運算大約需要 32<sup>2</sup> / n<sup>2</sup> * ((2 * n + 32 - 2) + n) 個 cycle (單純考慮計算以及將結果從 PE 搬回 internal buffer)，是一個 1 / n 的關係式，隨著 n 放大兩倍，每次可以節省的 cycle 數與 1 / n 相關，故在 systolic array size 不斷放大後，效能提升趨緩。

預期可透過 pipeline 設計提升效能
E.g. 透過重疊
    1. 將運算結果從 PE 搬回 internal buffer
    2. 下個矩陣乘法的運算
可以將 ((2 * n + 32 - 2) + n) 縮減為 (2 * n + 32 - 2)，但是整體 cycle 依然呈現 1 / n 關係，故仍會有隨 systolic array size 增加，效能提升趨緩的問題。
</font>

Q2. When you modify your simulator to support the evaluation, what is the main challenge?

修改模擬器時，最大的挑戰在於設計一個通用的解法，能夠正確處理任意大小的矩陣尺寸與 systolic array 之間的對應關係。無論兩者大小如何變化，都必須保證運算邏輯正確，並能將運算結果寫回正確的記憶體位置。

Q3. Run your test program and do performance analysis. 

針對不同大小的 systolic array（2 * 2、4 * 4、8 * 8）與不同矩陣尺寸對總運算 cycle 數影響的分析如下
1. 小尺寸矩陣 (N = 2, 4)：由於矩陣太小，systolic array 的大小對總運算 cycle 數影響很小
2. 中尺寸矩陣 (N = 8, 16)：隨著矩陣變大，systolic array 的大小開始影響運算效率。4 * 4 和 8 * 8 的 systolic array 對比 2 * 2 有明顯效能提升
3. 大尺寸矩陣 (N = 32)：systolic array 的大小對總運算 cycle 影響顯著。2 * 2 systolic array 需要 31851 cycles，而 8 * 8 systolic array 僅需 23403 cycles，效能提升約 27%。但從 4 * 4 到 8 * 8 的效能提升則僅約 7%，顯示擴大 systolic array 的效益開始趨緩。

這些結果驗證了先前提出的 diminishing return 現象：隨著 systolic array 尺寸增大，雖然能減少運算所需 cycle，但當 array 大小超過一定程度（4 * 4）後，效能提升的幅度逐漸減小。因此設計時需要在硬體資源與效能之間取得平衡，避免過度增加 PE 而造成成本浪費。
