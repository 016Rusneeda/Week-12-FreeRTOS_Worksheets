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
2. ปัญหาอะไรที่เกิดขึ้นเมื่อ time slice สั้นเกินไป?
3. ปัญหาอะไรที่เกิดขึ้นเมื่อ time slice ยาวเกินไป?
4. Context switching overhead คิดเป็นกี่เปอร์เซ็นต์ของเวลาทั้งหมด?
5. งานไหนที่ได้รับผลกระทบมากที่สุดจากการ time-sharing?

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
2. ข้อดีของ Cooperative Multitasking คืออะไร?
3. ข้อเสียของ Cooperative Multitasking คืออะไร?
4. ในสถานการณ์ใดที่ Cooperative จะดีกว่า Preemptive?
5. เหตุใด Preemptive จึงเหมาะสำหรับ Real-time systems?
