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
