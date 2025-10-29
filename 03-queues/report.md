# บันทึกผลการทดลอง
# Lab 1: Basic Queue Operations 
## 🧪 การทดลอง
### ทดลองที่ 1: การทำงานปกติ
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/fd7cf014-e7d6-424a-bf79-a0f124fbdb99" />

### ทดลองที่ 2: ทดสอบ Queue เต็ม
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/4adac8c1-4b08-4b37-a343-28ce857a6c9f" />

### ทดลองที่ 3: ทดสอบ Queue ว่าง
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/ba988326-6cf6-45a5-81ab-58cde3fed6b9" />


### ตารางบันทึกผล
| ทดลอง | Sender Rate | Receiver Rate | Queue Status | สังเกต |
|-------|-------------|---------------|--------------|---------|
| 1 | 2s | 1.5s | Normal |มีแค่ 3 ข้อความต่อคิว |
| 2 | 0.5s | 1.5s | Full | มีข้อความ Failed to send message (queue full?)|
| 3 | 2s | 0.1s | Empty | Queue Status - Messages: 0, Free spaces: 5 |

### คำถามสำหรับการทดลอง
1. เมื่อ Queue เต็ม การเรียก `xQueueSend` จะเกิดอะไรขึ้น?
   * ตอบ ถ้าเรียก `xQueueSend(queue, &data, portMAX_DELAY);` → Task จะ ถูก Block (หยุดรอ) จนกว่า Queue จะมีที่ว่าง (มี Task อื่นรับข้อมูลออกไปก่อน)
แต่ถ้าใช้ `xQueueSend(queue, &data, 0);` → ฟังก์ชันจะ ส่งไม่สำเร็จทันที และ return errQUEUE_FULL

2. เมื่อ Queue ว่าง การเรียก `xQueueReceive` จะเกิดอะไรขึ้น?
   * ตอบ เมื่อ Queue ว่าง ถ้าเรียก `xQueueReceive(queue, &data, portMAX_DELAY);` → Task จะ ถูก Block รอจนกว่าจะมี Task อื่นส่งข้อมูลเข้ามา
แต่ถ้าใช้ `xQueueReceive(queue, &data, 0);` → จะ คืนค่า false ทันที เพราะไม่มีข้อมูลใน Queue

3. ทำไม LED จึงกะพริบตามการส่งและรับข้อความ?
   * ตอบ เพราะแต่ละการส่งและรับข้อมูลใน Queue ถูกผูกกับการ “เปลี่ยนสถานะของ LED”
เช่นในการทดลองทั่วไป:
Task 1 (Sender) → xQueueSend() สำเร็จ → สั่ง LED ON
Task 2 (Receiver) → xQueueReceive() สำเร็จ → สั่ง LED OFF
จึงเกิดการกะพริบ (ON → OFF → ON → OFF) ตามจังหวะการส่ง–รับข้อมูลใน Queue
----------------------------------------------------------------------------------------------------------------------------------------------------------------------
# Lab 2: Producer-Consumer System 
## 🧪 การทดลอง
### ทดลองที่ 1: ระบบสมดุล (Balanced System)
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/a2061be9-77a4-4c0f-ade9-f36b54065cff" />

### ทดลองที่ 2: เพิ่มผู้ผลิต (More Producers)
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/01e292e7-0735-4559-aae6-04a713b3ce9e" />

### ทดลองที่ 3: ลดผู้บริโภค (Fewer Consumers)
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/11f04455-a0c1-447c-b047-2c1b93df3d34" />

### 📊 ตารางผลการทดลอง
| ทดลอง | Producers | Consumers | Produced | Consumed | Dropped | Efficiency |
|-------|-----------|-----------|----------|----------|---------|------------|
| 1 | 3 | 2 | 3 | 2 | 0 | 66.7% |
| 2 | 4 | 2 | 4 | 2 | 0 | 50.0% |
| 3 | 3 | 1 | 3 | 2 | 0 | 30.8% |

### คำถามสำหรับการทดลอง
1. ในทดลองที่ 2 เกิดอะไรขึ้นกับ Queue?
   * ตอบ เมื่อเพิ่มผู้ผลิตจาก3เป็น4 ทำให้จำนวนงานที่ถูกส่งเข้าสู่ Queue ต่อวินาทีมากขึ้น ในขณะที่จำนวนผู้บริโภค (Consumer) ยังคงเท่าเดิมคือ2  ส่งผลให้ Queue เต็มบ่อย และประสิทธิภาพ (Efficiency) ลดลง

3. ในทดลองที่ 3 ระบบทำงานเป็นอย่างไร?
   * ตอบ เมื่อลดผู้บริโภคลง ทำให้Queue เต็ม ประสิทธิภาพช้าลงมาก
5. Load Balancer แจ้งเตือนเมื่อไหร่?
   * ตอบ Load Balancer จะแจ้งเตือนเมื่อพบว่าปริมาณงานระหว่าง Producer–Consumer ไม่สมดุล เช่น Queue เต็มนาน (overload) หรือว่างนาน (underload) ซึ่งเป็นสัญญาณให้ปรับจำนวน Task หรือ Priority ให้เหมาะสมเพื่อให้ระบบทำงานมีประสิทธิภาพมากขึ้น
----------------------------------------------------------------------------------------------------------------------------------------------------------------------
# Lab 3: Queue Sets Implementation
## 🧪 การทดลอง
### ทดลองที่ 1: สังเกตการทำงานปกติ
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/935f563c-ece8-4db5-a528-bc513e6a4e4b" />

### ทดลองที่ 2: ปิดใช้งานแหล่งข้อมูล
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/29f30cb5-b453-41ed-972c-c356b7f0472d" />

### ทดลองที่ 3: เพิ่มความถี่ข้อความ
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/53444f4b-50de-488f-b460-338aca1928ee" />


### 📊 ตารางผลการทดลอง
| ทดลอง | Sensor | User | Network | Timer | Total | สังเกต |
|-------|--------|------|---------|-------|-------|---------|
| 1 (ปกติ) | 2-5 s | 3-8 s| 1-4s|10s |60-80 message/m | |
| 2 (ไม่มี Sensor) |- | / | /| /| 40-60 message/m| |
| 3 (Network เร็ว) | / | / | 0.5s| / | 150-200 message/m| |

### คำถามสำหรับการทดลอง
1. Processor Task รู้ได้อย่างไรว่าข้อมูลมาจาก Queue ไหน?
   * ตอบ Task รู้แหล่งข้อมูลจากค่า handle ที่ xQueueSelectFromSet() คืนกลับมา
2. เมื่อหลาย Queue มีข้อมูลพร้อมกัน เลือกประมวลผลอันไหนก่อน?
   * ตอบ FreeRTOS จะเลือก Queue แรกที่ตรวจพบว่ามีข้อมูล ภายในลูปของ Scheduler โดยลำดับนี้ขึ้นกับ ลำดับที่ Queue ถูกเพิ่มเข้าใน Set (first-added, first-checked) ไม่รับประกันความเป็นลำดับเวลาแน่นอน (ไม่ใช่ตามเวลาส่งข้อมูลเข้ามาเสมอไป)
3. Queue Sets ช่วยประหยัด CPU อย่างไร?
   * ตอบ เพราะ Queue Set ช่วยให้ Task รอหลาย Queue พร้อมกันได้โดยไม่ต้อง polling
