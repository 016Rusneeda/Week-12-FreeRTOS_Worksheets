# Lab 1: Single Task vs Multitasking Demo
#### Part 1: Single Task System (20 นาที)
<img width="1920" height="1080" alt="Screenshot (18)" src="https://github.com/user-attachments/assets/15f01645-5f27-46ff-972a-992d4b38e81a" />

#### Part 2: Multitasking System (20 นาที)
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/5f017114-bbcc-4274-b592-51979f7b7b6b" />

--------------------------------------------------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------------------------------------------

# Lab 2: Time-Sharing Implementation
### Part 1: Simple Time-Sharing (25 นาที)
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/6155dede-e19c-49e3-a912-14e4b0252b78" />

### Part 2: Time-Sharing with Variable Workloads (15 นาที)
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/0dcc008e-4522-44f7-9d84-c6a8e7ab685c" />

### Part 3: การวิเคราะห์ปัญหา (5 นาที)
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/209166ed-7490-4a72-bf3d-dfaecc1cb30f" />


## คำถามสำหรับวิเคราะห์

1. Time slice ขนาดไหนให้ประสิทธิภาพดีที่สุด? เพราะอะไร?
  * ตอบ ขนาดที่สมดุล (Optimal Balance) จะอยู่ในช่วง 10 - 100 มิลลิวินาที (ms)	เพื่อให้ได้เวลาตอบสนองที่ดี โดยที่ยังคงมี Context Switching Overhead ต่ำ
2. ปัญหาอะไรที่เกิดขึ้นเมื่อ time slice สั้นเกินไป?
  * ตอบ Context Switching Overhead สูงมาก	CPU ใช้เวลาส่วนใหญ่ไปกับการสลับงาน แทนที่จะทำงานจริง (ลดประสิทธิภาพรวม)
3. ปัญหาอะไรที่เกิดขึ้นเมื่อ time slice ยาวเกินไป?
  * ตอบ เวลาตอบสนองช้า (Poor Response Time) และ ระบบดูเหมือน "ค้าง"	งานโต้ตอบ (Interactive Job) ต้องรอนานเกินไปกว่าจะถึงคิวทำงาน
4. Context switching overhead คิดเป็นกี่เปอร์เซ็นต์ของเวลาทั้งหมด?
  * ตอบ ไม่มีตัวเลขคงที่ โดยทั่วไปประมาณ 1% - 3%	ขึ้นอยู่กับ CPU, OS, และความถี่ในการสลับงาน แต่จะสูงขึ้นมากหาก Time slice สั้นเกินไป
5. งานไหนที่ได้รับผลกระทบมากที่สุดจากการ time-sharing?
  * ตอบ 1. งานคำนวณขนาดใหญ่ (CPU-bound Jobs) ถูกขัดจังหวะซ้ำๆ ทำให้เกิด Overhead โดยไม่ให้ประโยชน์ต่อผู้ใช้ 2. งานที่ต้องการเวลาตอบสนองสม่ำเสมอ (Latency-Sensitive)	 

## ผลการทดลองที่คาดหวัง

| Time Slice | Context Switches/sec | CPU Utilization | Overhead |
|------------|---------------------|-----------------|----------|
| 10ms       | 100                 | 70-75%          | 25-30%   |
| 50ms       | 20                  | 85-90%          | 10-15%   |
| 100ms      | 10                  | 90-95%          | 5-10%    |

--------------------------------------------------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------------------------------------------

# Lab 3: Cooperative vs Preemptive Comparison
### Part 1: Cooperative Multitasking (15 นาที)
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/35bcb92b-e5e9-4640-b002-298f184ba512" />

### Part 2: Preemptive Multitasking (15 นาที)
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/545f9bf1-9bd2-4806-8719-4bfd5298d962" />


## คำถามสำหรับวิเคราะห์

1. ระบบไหนมีเวลาตอบสนองดีกว่า? เพราะอะไร?
   * ตอบ Preemptive Multitasking ตอบสนองดีกว่า เพราะ มีOS ควบคุมและสามารถขัดจังหวะ (Preempt) งานที่รันอยู่ได้ตลอดเวลา ทำให้ไม่มีโปรแกรมใดผูกขาด CPU ไว้ได้
2. ข้อดีของ Cooperative Multitasking คืออะไร?
   * ตอบ 1.ง่ายต่อการเขียนโค้ด (ไม่ต้องกังวลเรื่องการถูกขัดจังหวะ) , 2. Context Switching Overhead ต่ำ
3. ข้อเสียของ Cooperative Multitasking คืออะไร?
   * ตอบ 1. ระบบอาจ "แฮงค์" ได้ทั้งระบบ ถ้ามีโปรแกรมใดไม่ยอมสละ CPU 2. ขาดการรับประกันการตอบสนองสำหรับงานสำคัญ
4. ในสถานการณ์ใดที่ Cooperative จะดีกว่า Preemptive?
   * ตอบ 1. ระบบที่มีทรัพยากรจำกัด เช่น ไมโครคอนโทรลเลอร์ 2. ระบบที่โค้ดทุกส่วนถูกควบคุม เพื่อลด Overhead ในการสลับงาน
5. เหตุใด Preemptive จึงเหมาะสำหรับ Real-time systems?
   * ตอบ 1. รับประกันเวลาตอบสนอง (Response Time Guarantee): สามารถขัดจังหวะงานที่มีความสำคัญต่ำกว่า ทันที เพื่อให้งานสำคัญ (Critical Task) ได้รับการประมวลผลภายในเวลาที่กำหนด
