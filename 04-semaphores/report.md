# Lab 1: Binary Semaphores
## 🧪 การทดลอง
### ทดลองที่ 1: การทำงานปกติ
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/47ef1561-ffbf-4948-8ceb-befaabbb27fb" />

### ทดลองที่ 2: การทดสอบ Multiple Give
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/3beb9ea4-9ce4-4041-a7a9-7abe719e301f" />

### ทดลองที่ 3: การทดสอบ Timeout
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/ab372989-040d-4eda-8486-86df5895b3b0" />

### ตารางบันทึกผล
| ทดลอง | Events Sent | Events Received | Timer Events | Button Presses | Efficiency |
|-------|-------------|-----------------|--------------|----------------|------------|
| 1 (Normal)        | 3 | 3 | 1 | 0 | 116% |
| 2 (Multiple Give) | 5 | 5 | 1 | 0 | 116% |
| 3 (Short Timeout) | 8 | 8 | 2 | 0 | 116% |

### คำถามสำหรับการทดลอง
1. เมื่อ give semaphore หลายครั้งติดต่อกัน จะเกิดอะไรขึ้น?
   * ตอบ ถ้าเป็น Binary Semaphore — มันจะ ไม่เพิ่มค่ามากกว่า 1 คือ ถ้า semaphore ยัง “ว่าง” (อยู่ในสถานะ Available) แล้วมีการ xSemaphoreGive() ซ้ำหลายครั้ง ระบบจะ ไม่สะสมค่าเพิ่มขึ้น เพราะ Binary Semaphore เก็บค่าได้เพียง 0 หรือ 1 เท่านั้น ผลลัพธ์: การ give หลายครั้งติดกันจะไม่มีผลเพิ่มเติมจากครั้งแรก

2. ISR สามารถใช้ `xSemaphoreGive` หรือต้องใช้ `xSemaphoreGiveFromISR`?
   * ตอบ ใน Interrupt Service Routine (ISR) ต้องใช้ `xSemaphoreGiveFromISR()` เท่านั้น

3. Binary Semaphore แตกต่างจาก Queue อย่างไร?
   * ตอบ

     
| รายการเปรียบเทียบ | **Binary Semaphore**                                                 | **Queue**                                       |
| ----------------- | -------------------------------------------------------------------- | ----------------------------------------------- |
| หน่วยเก็บข้อมูล   | มีเพียงสถานะ 0 หรือ 1                                                | เก็บข้อมูลหลายค่า (เป็น buffer)                 |
| หน้าที่หลัก       | ใช้ **ซิงโครไนซ์** การทำงานระหว่าง Task หรือ ISR                     | ใช้ **ส่งข้อมูล** ระหว่าง Task                  |
| ขนาด              | คงที่ (มีเพียง 1 ช่อง)                                               | กำหนดได้หลายช่อง                                |
| การใช้งานทั่วไป   | - แจ้งเหตุการณ์ (Event Signal) <br> - ป้องกัน resource ที่ใช้ร่วมกัน | - ส่งค่าหรือข้อความระหว่าง Task                 |
| ตัวอย่าง          | Task A `give`, Task B `take`                                         | Producer `xQueueSend`, Consumer `xQueueReceive` |

------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
# Lab 2: Mutex and Critical Sections
## 🧪 การทดลอง
### ทดลองที่ 1: การทำงานกับ Mutex
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/6e781c0b-4adb-46ac-81ef-2479bf13d96a" />

### ทดลองที่ 2: ปิด Mutex (เพื่อดู Race Condition)
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/da0f1e2f-9a91-4320-bfa1-fb70006aac99" />
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/b8d9b236-cee5-40de-b0f3-6e69b5b29470" />


### ทดลองที่ 3: ปรับ Priority
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/9d1174dc-3dee-4462-b844-6b2aa91afaf2" />


### ตารางผลการทดลอง
| ทดลอง | Successful | Failed | Corrupted | Success Rate | สังเกต |
|-------|------------|--------|-----------|-------------|---------|
| 1 (With Mutex) | 9 |0 |0 |100.0% | |
| 2 (No Mutex)   | 11|0 |0 |100.0% | |
| 3 (Changed Priority) |9 |0 |0 |100.0% | |

### คำถามสำหรับการทดลอง
1. เมื่อไม่ใช้ Mutex จะเกิด data corruption หรือไม่?
   * ตอบ เกิดแน่นอน เพราะทุก task สามารถเข้าถึงและแก้ไขตัวแปร shared_data พร้อมกันได้โดยไม่มีการป้องกัน
2. Priority Inheritance ทำงานอย่างไร?
   * ตอบ Priority Inheritance คือกลไกของ FreeRTOS ในการป้องกัน priority inversion (สถานการณ์ที่ task priority สูง ต้องรอ task priority ต่ำ เพราะอีกฝ่ายถือ mutex ไว้)
หลักการ: ถ้า task ต่ำถือ mutex อยู่แล้ว task สูงมาขอ mutex ระบบจะ “ยืม priority ของ task สูงให้ task ต่ำ” ชั่วคราว  task ต่ำจะได้รันต่อจนออกจาก critical section และ คืน mutex จากนั้น priority ของ task ต่ำจะถูกปรับกลับค่าเดิม
👉 ผลคือ task priority สูงไม่ต้องรอนาน และ ไม่ถูกแทรกโดย medium priority task
3. Task priority มีผลต่อการเข้าถึง shared resource อย่างไร?
   * ตอบ Priority จะกำหนดลำดับการได้ CPU → จึงมีผลโดยตรงกับการเข้าถึง shared resource


| สถานการณ์                           | ผลที่เกิดขึ้น                                                                                                   |
| ----------------------------------- | --------------------------------------------------------------------------------------------------------------- |
| **Task priority สูงกว่า**           | ได้ CPU ก่อน → มักได้ mutex ก่อน และ เข้าถึง resource บ่อยกว่า                                                  |
| **Task priority ต่ำกว่า**           | อาจถูก preempt ระหว่างถือ mutex → ถ้าไม่มี Priority Inheritance อาจก่อ priority inversion                       |
| **สลับ priority (เช่น ทดลองที่ 3)** | Low Priority รันก่อน High Priority → High Priority ต้องรอ mutex นาน → เกิด delay หรือ priority inversion ชัดเจน |

ดังนั้น ในระบบที่มีการใช้ resource ร่วมกัน ต้องวาง priority อย่างรอบคอบ และ ใช้ mutex เพื่อให้การเข้าถึงข้อมูลปลอดภัย

------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
# Lab 3: Counting Semaphores
## 🧪 การทดลอง
### ทดลองที่ 1: การทำงานปกติ
### ทดลองที่ 2: เพิ่มจำนวน Resources
### ทดลองที่ 3: เพิ่ม Producers

### ตารางผลการทดลอง
| ทดลอง | Resources | Producers | Success Rate | Avg Wait | Resource Utilization |
|-------|-----------|-----------|--------------|----------|---------------------|
| 1 (3R, 5P) | 3 | 5 | | | |
| 2 (5R, 5P) | 5 | 5 | | | |
| 3 (3R, 8P) | 3 | 8 | | | |

### คำถามสำหรับการทดลอง
1. เมื่อ Producers มากกว่า Resources จะเกิดอะไรขึ้น?
2. Load Generator มีผลต่อ Success Rate อย่างไร?
3. Counting Semaphore จัดการ Resource Pool อย่างไร?
