# ผลการทดลอง
# Lab 1: Task Priority และ Scheduling
### Step 1: Basic Priority Demonstration (20 นาที)
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/e37e8310-55c1-4dfe-8bff-3b9c1c04af79" />

### Step 2: Round-Robin Scheduling (15 นาที)
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/0bb95e8d-83f1-4b81-80b9-c06725c18941" />

### Step 3: Priority Inversion Demo (10 นาที)
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/3fb30fe6-8f4c-426b-b6ba-43b946e4aa7f" />


## แบบฝึกหัด
### Exercise 1: เปลี่ยน Priority แบบ Dynamic
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/c0bc13be-30b7-4391-bc8f-3bbc80cc7001" />

### Exercise 2: Task Affinity (ESP32 Dual-Core)
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/1f0932f0-c9e6-4b6f-9aac-6d47b0024caa" />


## คำถามสำหรับวิเคราะห์
1. Priority ไหนทำงานมากที่สุด? เพราะอะไร?
   * ตอบ Priority 5 เพราะ Task ที่ Priority สูงสุดได้ CPU มากที่สุด เพราะ scheduler ให้สิทธิ์ก่อนเสมอ
     
2. เกิด Priority Inversion หรือไม่? จะแก้ไขได้อย่างไร?
   * ตอบ เกิด Priority Inversion เกิดขึ้นได้เมื่อ
1.Task Priority ต่ำครอบครอง resource (เช่น mutex)
2.Task Priority สูงต้องรอ resource นั้น
3.มี Task Priority กลางมาทำงานแทรก ทำให้ Task สูง “รอโดยไม่จำเป็น”

🧩 การแก้ไข: ใช้ Priority Inheritance Mechanism ของ FreeRTOS (มีใน mutex)
- เมื่อ Task ต่ำถือ mutex แล้วมี Task สูงมารอ → FreeRTOS จะ ยก Priority ของ Task ต่ำให้เท่ากับ Task สูงชั่วคราว
- พอปล่อย mutex → Priority จะกลับคืนเดิม

4. Tasks ที่มี priority เดียวกันทำงานอย่างไร?
   * ตอบ เมื่อหลาย Task มี Priority เท่ากัน → FreeRTOS จะใช้ Round-Robin Scheduling
Scheduler จะสลับ Task ที่มี priority เท่ากันไปทำงานทีละรอบ  แต่ละ Task จะได้เวลาเท่ากัน (ตาม tick rate) ถ้า Task ใด delay/block → scheduler จะสลับไป Task อื่นที่พร้อม

สรุป: Task ที่ Priority เดียวกัน “แชร์ CPU” สลับกันทำงานตามรอบเวลาเท่ากัน
5. การเปลี่ยน Priority แบบ dynamic ส่งผลอย่างไร?
   * ตอบ FreeRTOS อนุญาตให้เปลี่ยน priority ของ Task ได้ระหว่าง runtime
- ใช้ฟังก์ชัน vTaskPrioritySet(TaskHandle_t xTask, UBaseType_t uxNewPriority);
- ถ้าเพิ่ม priority → Task จะอาจ แย่ง CPU ทันที (preempt)
- ถ้าลด priority → Task อาจถูก “เบียดออก” โดย Task อื่นที่ priority สูงกว่า

⚙️ สรุป: การเปลี่ยน Priority แบบ dynamic ส่งผลต่อ ลำดับการทำงานของ scheduler โดยตรง

6. CPU utilization ของแต่ละ priority เป็นอย่างไร?
   * ตอบ Task Priority สูง → ใช้ CPU มากที่สุด , Task Priority ต่ำ → ใช้น้อยที่สุด , Scheduler จัดสรรเวลาแบบ preemptive + round robin

-------------------------------------------------------------------------------------------------------------------------------------------------------
# Lab 2: Task States Demonstration
## แบบฝึกหัด
### Exercise 1: State Transition Counter
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/097b4781-31cd-4be2-b010-27f3ec9e14c3" />

### Exercise 2: Custom State Indicator
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/8ee96a41-58d9-4e89-a18d-24773ca1562a" />

## คำถามสำหรับวิเคราะห์
1. Task อยู่ใน Running state เมื่อไหร่บ้าง?
   * ตอบ Task จะอยู่ใน Running state ก็ต่อเมื่อ Scheduler ให้ CPU กับมันทำงานอยู่ ณ ขณะนั้น
2. ความแตกต่างระหว่าง Ready และ Blocked state คืออะไร?
   * ตอบ

| รายการ                          | **Ready state**                        | **Blocked state**                                     |
| ------------------------------- | -------------------------------------- | ----------------------------------------------------- |
| **ความหมาย**                    | Task พร้อมทำงาน รอให้ scheduler เรียก  | Task ถูก “พักไว้” เพราะรอ event หรือ delay            |
| **ได้รับ CPU หรือไม่**          | ยังไม่ได้ CPU แต่พร้อมทันที            | ยังไม่ได้ CPU และไม่พร้อมจนกว่าเงื่อนไขจะครบ          |
| **สาเหตุที่อยู่ใน state นี้**   | ถูกสร้างใหม่, ปลุกจาก delay/block แล้ว | เรียก `vTaskDelay()`, รอ semaphore, queue, หรือ event |
| **กลับไป Running ได้เมื่อไหร่** | เมื่อไม่มี Task ที่ Priority สูงกว่า   | เมื่อ event เกิดขึ้นหรือเวลาหน่วงครบ                  |

สรุป:  Ready: พร้อมแต่ยังไม่ได้ทำ  , Blocked: รอเหตุการณ์หรือเวลาหน่วง

3. การใช้ vTaskDelay() ทำให้ task อยู่ใน state ใด?
   * ตอบ vTaskDelay() จะทำให้ Task เข้าสู่ Blocked state เพราะ Task จะ “นอนรอ” ให้เวลาที่ระบุครบก่อน (delay time) เมื่อเวลาครบ → Scheduler จะย้าย Task กลับเข้าสู่ Ready state เพื่อรอ CPU อีกครั้ง
     
4. การ Suspend task ต่างจาก Block อย่างไร?
   * ตอบ
  
| รายการ                                     | **Suspend**                                     | **Block**                          |
| ------------------------------------------ | ----------------------------------------------- | ---------------------------------- |
| **สาเหตุ**                                 | ถูก “ระงับชั่วคราว” ด้วยคำสั่ง `vTaskSuspend()` | รอ event หรือ delay                |
| **ปลุกกลับมาทำงานได้อย่างไร**              | ใช้ `vTaskResume()` หรือ `xTaskResumeFromISR()` | เมื่อ event เกิดหรือเวลาหน่วงครบ   |
| **ใครเป็นคนควบคุม**                        | ผู้เขียนโปรแกรม (manual)                        | ระบบ Scheduler (automatic)         |
| **Scheduler มองว่าเป็น task พร้อมหรือไม่** | ไม่พร้อมทำงานจนกว่าจะ resume                    | จะกลับมา ready เองเมื่อครบเงื่อนไข |

สรุป: Suspend = หยุดโดยผู้ใช้, Block = หยุดเพราะรอ event

5. Task ที่ถูก Delete จะกลับมาได้หรือไม่?
    * ตอบ Task ที่ถูก Delete จะหายถาวร  — ต้องสร้างใหม่เท่านั้นหากต้องการใช้อีก
-------------------------------------------------------------------------------------------------------------------------------------------------------

# Lab 3: Stack Monitoring และ Debugging
## แบบฝึกหัด
### Exercise 1: Stack Size Optimization
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/c4611001-99e7-4c0e-9f21-5bbc7482d0e2" />

### Exercise 2: Dynamic Stack Monitoring
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/9bbfc309-da28-4f3d-b6cb-d1b64e3bf651" />


## คำถามสำหรับวิเคราะห์

1. Task ไหนใช้ stack มากที่สุด? เพราะอะไร?
* ตอบ  Heavy Task เพราะเป็น Task ที่มีการใช้ตัวแปรภายในเยอะ, เรียกฟังก์ชันซ้อนหลายชั้น, หรือ ใช้ recursion จะใช้ stack มากที่สุด

2. การใช้ heap แทน stack มีข้อดีอย่างไร?
* ตอบ - สามารถจัดสรรหน่วยความจำขนาดใหญ่ได้ โดยไม่จำกัดเหมือน stack
      - ลดภาระของ stack เพราะข้อมูลใหญ่ ๆ ถูกเก็บใน heap
      - เหมาะกับการสร้าง object หรือ buffer ที่ต้องการอยู่คงทนข้ามฟังก์ชัน

3. Stack overflow เกิดขึ้นเมื่อไหร่และทำอย่างไรป้องกัน?
   * ตอบ tack Overflow เกิดขึ้นเมื่อ Task ใช้ stack เกินกว่าขนาดที่จัดสรรไว้ (stack size) เช่น เรียกฟังก์ชันซ้อนมากเกินไป หรือประกาศตัวแปรในฟังก์ชันใหญ่เกิน สามรถป้องกันได้ด้วยการตรวจสอบและปรับขนาดอย่างเหมาะสม

4. การตั้งค่า stack size ควรพิจารณาจากอะไร?
   * ตอบ ควรพิจารณาจากปัจจัยดังนี้ 
      1. ลักษณะของ task – ทำงานซับซ้อนแค่ไหน, เรียกฟังก์ชันซ้อนหรือไม่
      2. จำนวนตัวแปรภายใน (local variables) – ถ้ามี buffer หรือ array ภายในเยอะ ต้องใช้ stack มาก
      3. การเรียกใช้ library functions – ฟังก์ชันบางตัว เช่น printf() ใช้ stack เยอะ
      4. การเผื่อความปลอดภัย (safety margin) – ปกติควรเผื่อ 20–30% จากการวัดจริง

9. Recursion ส่งผลต่อ stack usage อย่างไร?
   * ตอบ Recursion (การเรียกฟังก์ชันตัวเองซ้ำ) จะทำให้ stack ถูกใช้เพิ่มขึ้นในทุกครั้งที่เรียกซ้ำ แต่ละรอบของ recursion จะสร้าง “stack frame ใหม่” เก็บตัวแปรและ return address ถ้าความลึกของ recursion มาก → stack จะถูกใช้มากจนเกิด overflow ได้ง่าย
