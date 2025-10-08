# Lab 1: Single Task vs Multitasking Demo
#### Part 1: Single Task System (20 นาที)
<img width="1920" height="1080" alt="Screenshot (18)" src="https://github.com/user-attachments/assets/15f01645-5f27-46ff-972a-992d4b38e81a" />

#### Part 2: Multitasking System (20 นาที)
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/5f017114-bbcc-4274-b592-51979f7b7b6b" />

--------------------------------------------------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------------------------------------------

# Lab 2: Time-Sharing Implementation
### Part 1: Simple Time-Sharing (25 นาที)
### Part 2: Time-Sharing with Variable Workloads (15 นาที)
### Part 3: การวิเคราะห์ปัญหา (5 นาที)
## การทดสอบและวัดผล

### 1. การวัด CPU Utilization
- สังเกตการแสดงสถิติทุก 20 context switches
- บันทึกค่า CPU utilization และ overhead

### 2. การทดสอบ Time Slice ต่างๆ
- ทดสอบ time slice: 10ms, 25ms, 50ms, 100ms, 200ms
- เปรียบเทียบประสิทธิภาพ

### 3. การสังเกต LED Pattern
- LED1: Sensor task (งานเบา)
- LED2: Processing task (งานหนัก)
- LED3: Actuator task (งานปานกลาง)
- LED4: Display task (งานเบา)

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

## บทสรุร

การทดลองนี้แสดงให้เห็นถึง:
1. **Context Switching Overhead** - เวลาที่สููญเสียในการเปลี่ยน task
2. **Time Slice Trade-offs** - ความสมดุลระหว่างการตอบสนองและประสิทธิภาพ
3. **Fixed Schedule Limitations** - ข้อจำกัดของการใช้เวลาคงที่
4. **Need for RTOS** - เหตุผลที่จำเป็นต้องมี Real-Time Operating System
