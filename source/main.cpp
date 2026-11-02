#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// مكتبات النظام الرسمية للبلايستيشن 3 (PSL1GHT)
#include <io/pad.h>
#include <sysutil/sysutil.h>

// إعدادات الاتصال بالسيرفر الاحترافي 
#define SERVER_IP "127.0.0.1" 
#define SERVER_PORT 9999

int client_socket = 0;
char chat_buffer[2048] = "=== PS3 Advanced Chat Booted ===\n";

// دالة تأسيس اتصال الشبكة عبر التمريض المنخفض (Low-level TCP Sockets)
void init_ps3_network() {
    struct sockaddr_in server_address;

    // 1. إنشاء السوكت البرمجي
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        strcat(chat_buffer, "[ERROR] Socket creation failed.\n");
        return;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);

    // 2. تحويل عنوان الـ IP إلى صيغة ثنائية تفهمها نواة الـ PS3
    if (inet_pton(AF_INET, SERVER_IP, &server_address.sin_addr) <= 0) {
        strcat(chat_buffer, "[ERROR] Invalid Server Address.\n");
        return;
    }

    // 3. محاولة ربط خط الاتصال بالسيرفر الخارجي
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        strcat(chat_buffer, "[STATUS] Offline Mode: Server not reached.\n");
    } else {
        strcat(chat_buffer, "[SUCCESS] Connected to Global Server!\nPress (X) to Sync with Phones.\n");
    }
}

int main(int argc, char* argv[]) {
    // تفعيل منافذ يد التحكم للـ PS3
    ioPadInit(7);
    padInfo pad_info;
    padData pad_data;

    // بدء تشغيل الشبكة والاتصال بالسيرفر فوراً
    init_ps3_network();

    // حلقة تشغيل التطبيق المستمرة (Application Main Loop)
    while(1) {
        // فحص عمليات النظام الخلفية للبلايستيشن (لمنع تجمد الجهاز)
        sysUtilCheckCallback();

        // قراءة حركة الأزرار من يد التحكم رقم 1
        ioPadGetInfo(&pad_info);
        if(pad_info.status) {
            ioPadGetData(0, &pad_data);
            
            // التفاعل عند ضغط زر X: إرسال حزمة بيانات إلى الهواتف
            if(pad_data.BTN_CROSS) {
                const char* internal_packet = "PS3_NODE: Hello from Playstation 3 System!\n";
                send(client_socket, internal_packet, strlen(internal_packet), 0);
                strcat(chat_buffer, "You: Hello from Playstation 3 System!\n");
                
                // تأخير دقيق بمقدار 400 مللي ثانية لمنع فيضان البيانات (Debounce)
                usleep(400000); 
            }
        }

        // قراءة البيانات المستقبلة من الهواتف عبر السيرفر بدون حظر الحلقة (Non-blocking Check)
        char packet_stream[1024] = {0};
        int incoming_bytes = recv(client_socket, packet_stream, sizeof(packet_stream) - 1, MSG_DONTWAIT);
        
        if (incoming_bytes > 0) {
            packet_stream[incoming_bytes] = '\0';
            strcat(chat_buffer, packet_stream); // دمج نصوص الهاتف وعرضها على شاشة التلفزيون
        }

        // الخروج الآمن من التطبيق والعودة لقائمة الـ PS3 الرئيسية عند ضغط زر Start
        if(pad_data.BTN_START) {
            break;
        }
    }

    // إغلاق المنافذ وتطهير الذاكرة بشكل سليم قبل الخروج
    if(client_socket > 0) close(client_socket);
    ioPadEnd();
    return 0;
}
