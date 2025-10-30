# Lab 1: Basic Software Timers
## 🧪 การทดลอง
### ทดลองที่ 1: การทำงานปกติ
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/872ffb86-240a-4dc5-9b78-d15e74cccb30" />

-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
# Lab 2: Timer Applications - Watchdog & LED Patterns
### 🧪 ผลการทดลอง
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/b83190aa-b33a-4f9e-bee7-2d6bae905d7a" />

## 📋 Post-Lab Questions

1. **Watchdog Design**: เหตุใดต้องใช้ separate timer สำหรับ feeding watchdog?
   *เพื่อ
    * แยกความรับผิดชอบ (separation of concerns):
    * watchdog_timer = ตัวจับ timeout ถ้าระบบ “ไม่ถูกให้อาหาร” ตามเวลา
    * feed_timer = ตัว “กำหนดจังหวะให้อาหาร” ตามปกติ
    * แยกแล้วตรวจจับอาการค้าง/หน่วงได้จริง (ถ้า task ที่ควร feed ค้าง → watchdog จะหมดเวลา)
    * ความแม่นยำของเวลา: feed ที่มาจาก auto-reload timer จะ เสถียรและไม่ขึ้นกับโหลดของ task อื่น มากกว่าการฝากไว้กับ loop ใด ๆ
    * ทดสอบ/จำลองเหตุขัดข้องได้ง่าย: หยุด feed_timer ชั่วคราวเพื่อ “จำลองแฮงก์” แล้วดูว่า watchdog ทำงานตามคาด
    * ลด coupling และ jitter: ไม่ผูก logic อื่นเข้ากับ watchdog (เช่นทำงานหนัก ๆ ก่อนค่อย feed) ลดความเสี่ยง “เผลอ feed ช้า”

2. **Pattern Timing**: อธิบายการเลือก Timer Period สำหรับแต่ละ pattern
    * Slow Blink: ~1000 ms/ทิก (มองเห็นชัด ไม่รบกวนสายตา)
    * Fast Blink: ~200 ms/ทิก (สื่อ “คำเตือน/เร่งด่วน”)
    * Heartbeat: สเต็ป ~100 ms (สองพัลส์ติดกันแล้วเว้นช่วง) — ใกล้จังหวะชีพจรที่คนคุ้นตา
    * SOS: dot ≈200 ms, dash ≈600 ms, พัก 1 s ต่อรอบ — ตามมาตรฐานมอร์ส ให้แยก “จุด/ขีด” ชัดเจน
    * Rainbow: ~300 ms ต่อสเต็ป (เปลี่ยนลวดลายเร็วพอให้รู้สึก “ไหลลื่น” แต่ไม่กระพริบถี่เกิน)

3. **Sensor Adaptation**: ประโยชน์ของ Adaptive Sampling Rate คืออะไร?
    * ประหยัดพลังงาน: ค่า “นิ่ง/ปลอดภัย” → ค่อยลดอัตราอ่าน (ยืด period) ประหยัดไฟ/CPU
    * ตอบสนองเหตุการณ์เร็ว: ค่า “เข้าโซนเสี่ยง” → เร่ง sampling เพื่อตรวจจับการเปลี่ยนแปลงแบบ real-time
    * ลดภาระระบบ: คิว/ประมวลผล/ล็อก ไม่ล้นโดยไม่จำเป็นในช่วงนิ่ง
    * คุณภาพข้อมูล: ได้สัญญาณหนาแน่นเฉพาะช่วงที่ “ต้องรู้เร็ว” ทำให้สถิติ/อัลกอริทึมแจ้งเตือนแม่นขึ้น

4. **System Health**: metrics ใดบ้างที่ควรติดตามในระบบจริง?
    * ความถูกต้องของ watchdog: จำนวน feed/timeout, เวลารอเฉลี่ยก่อน timeout, สาเหตุ reset ล่าสุด (esp_reset_reason)
    * ทรัพยากรหน่วยความจำ: heap free/min free, fragment, และ stack high-water mark ของแต่ละ task (กัน stack overflow)
    * โหลดซีพียู/ดีเลย์: อัตราใช้ CPU ต่อ task, missed deadlines/late ticks ของ timers, jitter ของ pattern (เปรียบ period เป้าหมายกับจริง)
    * สุขภาพคิว/ไทเมอร์: ความลึกคิวเฉลี่ย/สูงสุด, อัตรา drop, timers active/inactive, จำนวนครั้งที่ปรับ period
    * คุณภาพข้อมูลเซนเซอร์: อัตรา valid/invalid, avg/std-dev, อุณหภูมิเฉลี่ย 1/5/15 นาที, อัตราการเปลี่ยนแปลง (derivative)
    * พลังงาน/อุณหภูมิระบบ: อุณหภูมิภายใน, รอบเวลาที่เปิด SENSOR_POWER รวม, duty cycle ของ LED (เพื่อคุมการกินไฟ/ความร้อน)
    * เหตุการณ์ผิดปกติ: นับเหตุแจ้งเตือน (เช่น high temp), ระยะเวลาตั้งแต่เหตุสุดท้าย (MTBF เชิงเหตุการณ์)
    * อายุระบบ: uptime, จำนวนครั้ง OTA/รีบูต, เวอร์ชันเฟิร์มแวร์/คอนฟิก
-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
# Lab 3: Advanced Timer Management & Performance
### 🧪 ผลการทดลอง

<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/4a1dcbd3-bf04-4e91-843d-1f2932fbc393" />


## 📋 Advanced Analysis Questions
1. **Service Task Priority**: ผลกระทบของ Priority ต่อ Timer Accuracy?
   * ตอบ ถ้า priority ของ service task ต่ำกว่า task อื่นที่ใช้ CPU หนัก → callback จะถูกหน่วง (delayed execution) → “timer jitter” สูงขึ้น หากให้ priority สูงเกินไป → timer callback จะเบียดงานหลักอื่น จนอาจเกิด starvation

2. **Callback Performance**: วิธีการเพิ่มประสิทธิภาพ Callback Functions?
   * ตอบ
   * ห้าม block หรือ delay ใน callback  Callback ต้อง return เร็วที่สุด เพราะมันอยู่ใน service task เดียวกันทั้งหมด → ใช้ “event queue → worker task” แทนถ้าต้องหน่วงเวลา
   * หลีกเลี่ยงการใช้ printf / ESP_LOG ระดับ DEBUG ถี่เกิน Logging ช้าและใช้ heap → ทำให้ callback jitter สูง
   * Reuse resources: ใช้ static buffer / preallocated object แทน malloc/free ใน callback
   * Batch processing: รวมหลาย event ใน worker task แทนทำทุก callback ทันที
   * Measure performance: วัดเวลารันจริง (esp_timer_get_time()) รอบๆ callback  เพื่อตรวจจับ callback ที่รันนานเกิน threshold เช่น 5 ms

3. **Memory Management**: กลยุทธ์การจัดการ Memory สำหรับ Dynamic Timers?
   * ตอบ Dynamic timer คือ timer ที่สร้าง/ลบ runtime ด้วย xTimerCreate() และ xTimerDelete()
     
4. **Error Recovery**: วิธีการ Handle Timer System Failures?
   * ตอบ
   * วิธีจัดการ
      1. ตรวจสอบผลลัพธ์ทุก xTimer*() เสมอ `if (xTimerStart(timer, 0) != pdPASS) { handle_error("xTimerStart failed"); }`
      2. Watchdog สำหรับ timer system: มี task ที่ตรวจทุก 10 วินาที ว่า timers ยัง active อยู่หรือไม่ ถ้าไม่ → restart timer service / รีบูตระบบ
      3. Timeout fallback: ถ้า timer ไม่ทำงานในเวลาที่คาด → trigger safety fallback เช่น ปิด output, disable actuator
      4. Self-healing: ใช้ secondary timer หรือ supervisor task ตรวจจับและ recreate timers ที่หาย
      5. Logging และ Diagnostics: บันทึก error code (pdFAIL, pdTRUE) พร้อม timestamp สำหรับวิเคราะห์ย้อนหลัง
     
5. **Production Deployment**: การปรับแต่งสำหรับ Production Environment?
    * ตอบ static alloc + minimal log + watchdog  เสถียรและ maintain ได้จริง
