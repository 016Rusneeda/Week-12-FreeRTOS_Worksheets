# Lab 1: ESP-IDF Setup และโปรเจกต์แรก
## แบบฝึกหัด
### Exercise 1: แก้ไขข้อความ
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/9b338b8f-7aab-45aa-b236-303b55b89440" />

### Exercise 2: เพิ่ม Build Informatio
<img width="1919" height="1016" alt="image" src="https://github.com/user-attachments/assets/88b13593-43a8-43eb-a921-96e0d038a512" />

### Exercise 3: การ Build แบบ Verbose
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/ecc565fa-e57a-4209-a52c-9fcff0888c9b" />

## Checklist การทำงาน

- ✅ ตรวจสอบ ESP-IDF environment สำเร็จ
- ✅ สร้างโปรเจกต์ใหม่ได้
- ✅ เข้าใจโครงสร้างโปรเจกต์
- ✅ Build โปรเจกต์สำเร็จ
- ✅ แก้ไขโค้ดและ build ใหม่ได้
- ✅ เข้าใจไฟล์ CMakeLists.txt
- ✅ ทำแบบฝึกหัดครบ
## คำถามทบทวหน

1. ไฟล์ใดบ้างที่จำเป็นสำหรับโปรเจกต์ ESP-IDF ขั้นต่ำ?
   * ตอบ โปรเจกต์ ESP-IDF ขั้นต่ำที่สุดต้องมีไฟล์หลัก 2 ไฟล์:
    1. CMakeLists.txt (ในไดเรกทอรีรูทของโปรเจกต์): ไฟล์นี้กำหนดว่าโปรเจกต์จะถูกสร้าง (Build) อย่างไร และรวมถึงคอมโพเนนต์ใดบ้าง
    2. main/main.c หรือ main/main.cpp (ไฟล์ซอร์สโค้ดหลัก): ไฟล์นี้มีฟังก์ชัน app_main() ซึ่งเป็นจุดเริ่มต้นของการทำงานของแอปพลิเคชัน
   
2. ความแตกต่างระหว่าง `hello_esp32.bin` และ `hello_esp32.elf` คืออะไร?
   * ตอบ `hello_esp32.bin`	ไบนารี (Binary) ที่พร้อมใช้งาน (Executable)	ใช้สำหรับ แฟลช (Flash) ลงในหน่วยความจำแฟลชของชิป ESP32 โดยตรง เพื่อให้ชิปสามารถบูตและรันได้
   * `hello_esp32.elf`	ไฟล์รูปแบบ ELF (Executable and Linkable Format)	เป็นไฟล์ที่ใช้สำหรับการ ดีบัก (Debugging) โดยเฉพาะ เพราะมีข้อมูลสัญลักษณ์ (Symbolic Information) ที่ช่วยให้นักดีบักสามารถแมปแอดเดรสหน่วยความจำกลับไปยังโค้ดซอร์สได้
     
3. คำสั่ง `idf.py set-target` ทำอะไร?
   * ตอบ ใช้เพื่อ กำหนดเป้าหมาย (Target) ชิปที่จะใช้ในการสร้างโปรเจกต์ เช่น: idf.py set-target esp32
     
4. โฟลเดอร์ `build/` มีไฟล์อะไรบ้าง?
   * ตอบ  *`.bin`: ไฟล์ไบนารีที่พร้อมสำหรับแฟลช (เช่น app-farmware.bin)
           *`.elf`: ไฟล์ดีบักที่มีข้อมูลสัญลักษณ์ (เช่น app-farmware.elf)
           *`partition_table/`: ไฟล์ตารางพาร์ทิชัน (.bin, .csv)
           *`config/`: ไฟล์คอนฟิกูเรชันของโปรเจกต์ที่ใช้ในการ Build
           *ไฟล์ออบเจกต์ (Object Files) และไฟล์ชั่วคราวอื่นๆ ที่สร้างขึ้นโดย CMake
5. การใช้ `vTaskDelay()` แทน `delay()` มีความสำคัญอย่างไร?
    * ตอบ `vTaskDelay()` จะทำให้ Task ที่กำลังรันอยู่ เปลี่ยนสถานะเป็น Blocked และ สละการควบคุม CPU ทันที ให้กับ Scheduler เพื่อให้ Task อื่นๆ ที่พร้อมรันสามารถทำงานได้
    * `delay()` แบบวนลูปธรรมดา (Busy-Wait) Task นั้นจะ ผูกขาด CPU ไว้ตลอดเวลา ที่มีการหน่วงเวลา ทำให้ Task อื่นๆ ในระบบ (เช่น Task ที่จัดการ Wi-Fi หรือ Bluetooth) ไม่สามารถรันได้ ซึ่งจะทำให้ระบบล้มเหลวหรือหยุดชะงัก (Blocking System)

-------------------------------------------------------------------------------------------------------------------------------------------------

# Lab 2: Hello World และ Serial Communication
### ผลลัพธ์การทดลอง
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/e3969f25-abd0-49bd-be8f-d855d7cca715" />


## แบบฝึกหัด
### Exercise 1: สร้าง Custom Logger
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/bf003eff-0930-4114-ab7e-07a18c3d0945" />

### Exercise 2: Performance Monitoring
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/7a01f31b-2e75-41c9-a27d-587309091278" />

### Exercise 3: Error Handling Demo
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/ce879ed5-c4db-4950-b282-9244d94f22c7" />



## คำถามทบทวน

1. ความแตกต่างระหว่าง `printf()` และ `ESP_LOGI()` คืออะไร?
   * ตอบ
   | รายการ                  | `printf()`                              | `ESP_LOGI()`                                          |
| ----------------------- | --------------------------------------- | ----------------------------------------------------- |
| **หน้าที่**             | แสดงข้อความออกทาง serial console โดยตรง | เป็นระบบ logging ของ ESP-IDF สำหรับแสดงข้อมูล debug   |
| **การจัดการ Log level** | ไม่มีระดับความสำคัญ (แสดงทุกข้อความ)    | มีระดับ (Verbose, Debug, Info, Warn, Error)           |
| **การเปิด/ปิดข้อความ**  | ต้องคอมเมนต์โค้ดเอง                     | สามารถควบคุมผ่านการตั้งค่า log level ได้              |
| **ประสิทธิภาพ**         | ไม่เหมาะกับการ debug จำนวนมาก           | ออกแบบให้เหมาะกับ embedded system และมี tag ระบุที่มา |

2. Log level ไหนที่จะแสดงใน default configuration?
   * ตอบ Info (ESP_LOGI)
     
3. การใช้ `ESP_ERROR_CHECK()` มีประโยชน์อย่างไร?
   * ตอบ ใช้สำหรับ ตรวจสอบค่าผลลัพธ์ของฟังก์ชันที่ return esp_err_t  ถ้าผลลัพธ์ ไม่ใช่ ESP_OK → จะ แสดงข้อความ error และหยุดการทำงาน (abort) ทันที
   * ช่วยให้ debug ง่ายขึ้น และมั่นใจได้ว่าฟังก์ชันสำคัญทำงานสำเร็จ
     
4. คำสั่งใดในการออกจาก Monitor mode?
   * ตอบ ใช้ปุ่ม Ctrl + ] เพื่อออกจาก monitor
     
9. การตั้งค่า Log level สำหรับ tag เฉพาะทำอย่างไร?
    * ตอบ สามารถใช้ฟังก์ชันนี้ในโค้ด: `esp_log_level_set("TAG_NAME", ESP_LOG_DEBUG); `
เช่น `esp_log_level_set("wifi", ESP_LOG_VERBOSE);`
-------------------------------------------------------------------------------------------------------------------------------------------------

# Lab 3: สร้าง Task แรกด้วย FreeRTOS
### ผลลัพธ์ Step 1: Task พื้นฐาน (15 นาที)
### ผลลัพธ์ Step 2: Task Management (15 นาที)
### ผลลัพธ์ Step 3: Task Priorities และ Statistics (15 นาที)

## แบบฝึกหัด
### Exercise 1: Task Self-Deletion
### Exercise 2: Task Communication (Preview)
## คำถามทบทวน

1. เหตุใด Task function ต้องมี infinite loop?
2. ความหมายของ stack size ใน xTaskCreate() คืออะไร?
3. ความแตกต่างระหว่าง vTaskDelay() และ vTaskDelayUntil()?
4. การใช้ vTaskDelete(NULL) vs vTaskDelete(handle) ต่างกันอย่างไร?
5. Priority 0 กับ Priority 24 อันไหนสูงกว่า?
