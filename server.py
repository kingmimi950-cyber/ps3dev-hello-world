import asyncio
import websockets
import os

# مصفوفة لتخزين الأجهزة المتصلة (بلايستيشن + هواتف)
CONNECTED_DEVICES = set()

async def chat_handler(websocket, path):
    # إضافة الجهاز المتصل للمجموعة
    CONNECTED_DEVICES.add(websocket)
    print(f"[اتصال جديد] جهاز متصل الآن. إجمالي الأجهزة: {len(CONNECTED_DEVICES)}")
    
    try:
        async for message in websocket:
            print(f"[رسالة مستلمة]: {message}")
            # إرسال الرسالة فوراً لكل الأجهزة الأخرى المتصلة
            for device in CONNECTED_DEVICES:
                if device != websocket:
                    await device.send(message)
    except websockets.exceptions.ConnectionClosed:
        print("[اتصال] انقطع اتصال أحد الأجهزة")
    finally:
        # إزالة الجهاز عند إغلاقه للتطبيق
        CONNECTED_DEVICES.remove(websocket)

async def main():
    # المنصة السحابية تحدد البورت تلقائياً عبر المتغير PORT
    port = int(os.environ.get("PORT", 8080))
    print(f"السيرفر يعمل الآن على البورت: {port}")
    async with websockets.serve(chat_handler, "0.0.0.0", port):
        await asyncio.Future() # الحفاظ على تشغيل السيرفر للأبد

if __name__ == "__main__":
    asyncio.run(main())
