import os
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.patches as patches

OUTPUT_DIR = r"D:\Final project\fall-detection\docs\เอกสารประกอบโปรเจกต์จบ\แฟ้มภาพประกอป"
os.makedirs(OUTPUT_DIR, exist_ok=True)

# Set common font styling
plt.rcParams['font.sans-serif'] = ['Tahoma', 'Arial', 'DejaVu Sans']
plt.rcParams['axes.unicode_minus'] = False

def create_system_architecture_diagram():
    fig, ax = plt.subplots(figsize=(12, 7.5), dpi=300)
    ax.set_xlim(0, 100)
    ax.set_ylim(0, 100)
    ax.axis('off')
    
    # Background card
    bg = patches.FancyBboxPatch((1, 1), 98, 98, boxstyle="round,pad=0.5", facecolor="#F8FAFC", edgecolor="#CBD5E1", lw=1.5)
    ax.add_patch(bg)
    
    # Title
    ax.text(50, 95, "End-to-End System Architecture (ระบบตรวจจับการล้มอัจฉริยะ)", ha='center', va='center', fontsize=15, fontweight='bold', color="#0F172A")
    
    # --- LAYER 1: PERCEPTION & EDGE LAYER ---
    l1_bg = patches.FancyBboxPatch((4, 8), 28, 82, boxstyle="round,pad=0.5", facecolor="#EFF6FF", edgecolor="#3B82F6", lw=1.5)
    ax.add_patch(l1_bg)
    ax.text(18, 86, "Perception & Edge Layer\n(ชั้นตรวจจับและอุปกรณ์ขอบ)", ha='center', va='center', fontsize=11, fontweight='bold', color="#1E40AF")
    
    # LD6002C Box
    b_60 = patches.FancyBboxPatch((6, 68), 24, 12, boxstyle="round,pad=0.3", facecolor="#FFFFFF", edgecolor="#2563EB", lw=1.2)
    ax.add_patch(b_60)
    ax.text(18, 75.5, "HLK-LD6002C mmWave\n(60 GHz FMCW Fall Radar)", ha='center', va='center', fontsize=9.5, fontweight='bold', color="#1E293B")
    ax.text(18, 70.5, "UART1 / TinyFrame (115,200 bps)", ha='center', va='center', fontsize=8, color="#64748B")
    
    # LD2410C Box
    b_24 = patches.FancyBboxPatch((6, 52), 24, 12, boxstyle="round,pad=0.3", facecolor="#FFFFFF", edgecolor="#2563EB", lw=1.2)
    ax.add_patch(b_24)
    ax.text(18, 59.5, "HLK-LD2410C mmWave\n(24 GHz Presence Radar)", ha='center', va='center', fontsize=9.5, fontweight='bold', color="#1E293B")
    ax.text(18, 54.5, "GPIO16 (Digital In: OUT)", ha='center', va='center', fontsize=8, color="#64748B")
    
    # ESP32-S3 Core Box
    b_esp = patches.FancyBboxPatch((6, 28), 24, 20, boxstyle="round,pad=0.3", facecolor="#DBEAFE", edgecolor="#1D4ED8", lw=1.5)
    ax.add_patch(b_esp)
    ax.text(18, 43, "ESP32-S3 Microcontroller", ha='center', va='center', fontsize=10.5, fontweight='bold', color="#1E3A8A")
    ax.text(18, 38.5, "- TinyFrame Checksum Parser\n- Dual-Radar Sensor Fusion\n- Finite State Machine (FSM)\n- Fall Confirmation Delay", ha='center', va='center', fontsize=8, color="#1E293B")
    ax.text(18, 30.5, "Firmware: C++ / FreeRTOS", ha='center', va='center', fontsize=7.5, fontweight='bold', color="#2563EB")
    
    # Peripherals
    b_peri = patches.FancyBboxPatch((6, 11), 24, 13, boxstyle="round,pad=0.3", facecolor="#FFFFFF", edgecolor="#2563EB", lw=1.2)
    ax.add_patch(b_peri)
    ax.text(18, 19, "Local Indicators & Power", ha='center', va='center', fontsize=9, fontweight='bold', color="#1E293B")
    ax.text(18, 14, "- Active Buzzer (GPIO15)\n- WS2812B RGB (GPIO48)\n- 3.3V Step-down (3A Ripple<50mV)", ha='center', va='center', fontsize=7.5, color="#64748B")

    # Arrows inside Layer 1
    ax.annotate("", xy=(18, 48), xytext=(18, 52), arrowprops=dict(arrowstyle="->", color="#2563EB", lw=1.5))
    ax.annotate("", xy=(18, 48), xytext=(18, 68), arrowprops=dict(arrowstyle="->", color="#2563EB", lw=1.5))
    ax.annotate("", xy=(18, 24), xytext=(18, 28), arrowprops=dict(arrowstyle="->", color="#2563EB", lw=1.5))

    # --- LAYER 2: NETWORK & TRANSPORT LAYER ---
    l2_bg = patches.FancyBboxPatch((36, 8), 28, 82, boxstyle="round,pad=0.5", facecolor="#FEF3C7", edgecolor="#F59E0B", lw=1.5)
    ax.add_patch(l2_bg)
    ax.text(50, 86, "Network & Transport Layer\n(ชั้นเครือข่ายและการส่งผ่าน)", ha='center', va='center', fontsize=11, fontweight='bold', color="#92400E")
    
    b_wifi = patches.FancyBboxPatch((38, 66), 24, 14, boxstyle="round,pad=0.3", facecolor="#FFFFFF", edgecolor="#D97706", lw=1.2)
    ax.add_patch(b_wifi)
    ax.text(50, 75, "Wi-Fi 802.11 b/g/n", ha='center', va='center', fontsize=10, fontweight='bold', color="#1E293B")
    ax.text(50, 69.5, "2.4 GHz Local WLAN AP", ha='center', va='center', fontsize=8.5, color="#78350F")

    b_mqtt = patches.FancyBboxPatch((38, 45), 24, 17, boxstyle="round,pad=0.3", facecolor="#FFFFFF", edgecolor="#D97706", lw=1.2)
    ax.add_patch(b_mqtt)
    ax.text(50, 56.5, "MQTT Broker (EMQX)", ha='center', va='center', fontsize=10, fontweight='bold', color="#1E293B")
    ax.text(50, 51, "Port 1883 / TLS 8883\nTopic: fall/device_id/event\nTelemetry JSON Stream", ha='center', va='center', fontsize=8, color="#64748B")

    b_sec = patches.FancyBboxPatch((38, 15), 24, 25, boxstyle="round,pad=0.3", facecolor="#FFFFFF", edgecolor="#D97706", lw=1.2)
    ax.add_patch(b_sec)
    ax.text(50, 33, "Security & Gateway", ha='center', va='center', fontsize=9.5, fontweight='bold', color="#1E293B")
    ax.text(50, 25, "- HTTPS REST (Port 443)\n- WebSocket (Port 8080)\n- Nginx Reverse Proxy\n- Let's Encrypt SSL/TLS", ha='center', va='center', fontsize=8, color="#64748B")

    # Connecting L1 -> L2
    ax.annotate("", xy=(38, 54), xytext=(30, 43), arrowprops=dict(arrowstyle="->", color="#D97706", lw=2, mutation_scale=15))
    ax.text(34, 49, "Wi-Fi\nMQTT", ha='center', va='center', fontsize=7.5, fontweight='bold', color="#B45309")

    # --- LAYER 3: CLOUD & APPLICATION LAYER ---
    l3_bg = patches.FancyBboxPatch((68, 8), 28, 82, boxstyle="round,pad=0.5", facecolor="#ECFDF5", edgecolor="#10B981", lw=1.5)
    ax.add_patch(l3_bg)
    ax.text(82, 86, "Cloud & Application Layer\n(ชั้นคลาวด์และการประยุกต์)", ha='center', va='center', fontsize=11, fontweight='bold', color="#065F46")
    
    b_go = patches.FancyBboxPatch((70, 64), 24, 17, boxstyle="round,pad=0.3", facecolor="#D1FAE5", edgecolor="#059669", lw=1.5)
    ax.add_patch(b_go)
    ax.text(82, 75, "Go Backend Service", ha='center', va='center', fontsize=10.5, fontweight='bold', color="#064E3B")
    ax.text(82, 68.5, "- Event Ingestion & Audit Log\n- Webhook Dispatcher Worker\n- Device Management REST\n- Azure VM B1s Docker", ha='center', va='center', fontsize=7.5, color="#1E293B")

    b_db = patches.FancyBboxPatch((70, 46), 24, 14, boxstyle="round,pad=0.3", facecolor="#FFFFFF", edgecolor="#059669", lw=1.2)
    ax.add_patch(b_db)
    ax.text(82, 55, "PostgreSQL Database", ha='center', va='center', fontsize=10, fontweight='bold', color="#1E293B")
    ax.text(82, 49.5, "Azure Database (B1ms, SSL)\nTables: users, devices, events...", ha='center', va='center', fontsize=7.5, color="#64748B")

    b_line = patches.FancyBboxPatch((70, 29), 24, 14, boxstyle="round,pad=0.3", facecolor="#FFFFFF", edgecolor="#059669", lw=1.2)
    ax.add_patch(b_line)
    ax.text(82, 38, "LINE Messaging API", ha='center', va='center', fontsize=10, fontweight='bold', color="#1E293B")
    ax.text(82, 32.5, "LINE Flex Message Alert\n< 2 sec latency push to phone", ha='center', va='center', fontsize=7.5, color="#047857")

    b_web = patches.FancyBboxPatch((70, 11), 24, 15, boxstyle="round,pad=0.3", facecolor="#FFFFFF", edgecolor="#059669", lw=1.2)
    ax.add_patch(b_web)
    ax.text(82, 21, "Next.js 14 Dashboard UI", ha='center', va='center', fontsize=10, fontweight='bold', color="#1E293B")
    ax.text(82, 14.5, "Tailwind CSS / WebSocket\nReal-time Room Status & Config", ha='center', va='center', fontsize=7.5, color="#64748B")

    # Connecting L2 -> L3
    ax.annotate("", xy=(70, 71), xytext=(62, 54), arrowprops=dict(arrowstyle="->", color="#059669", lw=2, mutation_scale=15))
    ax.text(66, 64, "MQTT\nSubscribe", ha='center', va='center', fontsize=7.5, fontweight='bold', color="#047857")

    # Inside L3 connections
    ax.annotate("", xy=(82, 60), xytext=(82, 64), arrowprops=dict(arrowstyle="->", color="#059669", lw=1.2))
    ax.annotate("", xy=(82, 43), xytext=(82, 46), arrowprops=dict(arrowstyle="->", color="#059669", lw=1.2))
    ax.annotate("", xy=(82, 26), xytext=(82, 29), arrowprops=dict(arrowstyle="->", color="#059669", lw=1.2))

    plt.tight_layout()
    out_path = os.path.join(OUTPUT_DIR, "fig_3_2_system_architecture.png")
    fig.savefig(out_path, dpi=300, bbox_inches='tight')
    plt.close(fig)
    print(f"Generated: {out_path}")

def create_database_erd_diagram():
    fig, ax = plt.subplots(figsize=(12, 7.5), dpi=300)
    ax.set_xlim(0, 100)
    ax.set_ylim(0, 100)
    ax.axis('off')
    
    bg = patches.FancyBboxPatch((1, 1), 98, 98, boxstyle="round,pad=0.5", facecolor="#F8FAFC", edgecolor="#CBD5E1", lw=1.5)
    ax.add_patch(bg)
    
    ax.text(50, 95, "Entity-Relationship Diagram (ERD: ผังความสัมพันธ์ฐานข้อมูล)", ha='center', va='center', fontsize=15, fontweight='bold', color="#0F172A")
    
    def draw_table_card(x, y, w, h, title, fields, border_color="#3B82F6", header_bg="#DBEAFE"):
        card = patches.FancyBboxPatch((x, y), w, h, boxstyle="round,pad=0.3", facecolor="#FFFFFF", edgecolor=border_color, lw=1.5)
        ax.add_patch(card)
        head = patches.FancyBboxPatch((x, y + h - 6), w, 6, boxstyle="round,pad=0.2", facecolor=header_bg, edgecolor=border_color, lw=1.2)
        ax.add_patch(head)
        ax.text(x + w/2, y + h - 3, title, ha='center', va='center', fontsize=10, fontweight='bold', color="#1E293B")
        
        y_pos = y + h - 9.5
        for f in fields:
            is_pk = "(PK)" in f
            is_fk = "(FK)" in f
            col = "#B45309" if is_pk else ("#1D4ED8" if is_fk else "#334155")
            weight = 'bold' if (is_pk or is_fk) else 'normal'
            ax.text(x + 2, y_pos, f, ha='left', va='center', fontsize=7.8, fontweight=weight, color=col)
            y_pos -= 4.2

    # 1. USERS Table
    users_fields = [
        "id : BIGSERIAL (PK)",
        "email : VARCHAR(255) UNIQUE",
        "password_hash : VARCHAR(255)",
        "full_name : VARCHAR(100)",
        "role : VARCHAR(20) [admin, user]",
        "created_at : TIMESTAMP WITH TZ",
        "updated_at : TIMESTAMP WITH TZ"
    ]
    draw_table_card(5, 52, 26, 36, "users", users_fields, border_color="#2563EB", header_bg="#DBEAFE")

    # 2. DEVICE_GROUPS Table
    group_fields = [
        "id : BIGSERIAL (PK)",
        "user_id : BIGINT (FK)",
        "group_name : VARCHAR(100)",
        "building : VARCHAR(50)",
        "floor : VARCHAR(20)",
        "description : TEXT",
        "created_at : TIMESTAMP WITH TZ"
    ]
    draw_table_card(37, 52, 26, 36, "device_groups", group_fields, border_color="#059669", header_bg="#D1FAE5")

    # 3. DEVICES Table
    device_fields = [
        "id : BIGSERIAL (PK)",
        "group_id : BIGINT (FK)",
        "device_code : VARCHAR(50) UNIQUE",
        "device_name : VARCHAR(100)",
        "radar_mounting_height : FLOAT",
        "fall_threshold : FLOAT",
        "radar_sensitivity : INT",
        "status : VARCHAR(20)",
        "last_seen_at : TIMESTAMP WITH TZ"
    ]
    draw_table_card(69, 44, 26, 44, "devices", device_fields, border_color="#D97706", header_bg="#FEF3C7")

    # 4. EVENTS Table
    events_fields = [
        "id : BIGSERIAL (PK)",
        "device_id : BIGINT (FK)",
        "event_type : VARCHAR(30)",
        "target_distance : FLOAT",
        "target_velocity : FLOAT",
        "raw_payload : JSONB",
        "created_at : TIMESTAMP WITH TZ"
    ]
    draw_table_card(69, 4, 26, 36, "events", events_fields, border_color="#DC2626", header_bg="#FEE2E2")

    # 5. WEBHOOKS Table
    webhook_fields = [
        "id : BIGSERIAL (PK)",
        "user_id : BIGINT (FK)",
        "device_id : BIGINT (FK)",
        "channel_access_token : TEXT",
        "target_line_id : VARCHAR(100)",
        "is_active : BOOLEAN",
        "created_at : TIMESTAMP WITH TZ"
    ]
    draw_table_card(21, 4, 28, 36, "webhooks", webhook_fields, border_color="#7C3AED", header_bg="#EDE9FE")

    # Draw Relationship Lines
    # users (1) -> device_groups (N)
    ax.annotate("", xy=(37, 70), xytext=(31, 70), arrowprops=dict(arrowstyle="->", color="#475569", lw=1.5))
    ax.text(34, 72, "1 : N", ha='center', va='center', fontsize=8, fontweight='bold', color="#475569")

    # device_groups (1) -> devices (N)
    ax.annotate("", xy=(69, 70), xytext=(63, 70), arrowprops=dict(arrowstyle="->", color="#475569", lw=1.5))
    ax.text(66, 72, "1 : N", ha='center', va='center', fontsize=8, fontweight='bold', color="#475569")

    # devices (1) -> events (N)
    ax.annotate("", xy=(82, 40), xytext=(82, 44), arrowprops=dict(arrowstyle="->", color="#475569", lw=1.5))
    ax.text(85, 42, "1 : N", ha='center', va='center', fontsize=8, fontweight='bold', color="#475569")

    # users (1) -> webhooks (N)
    ax.annotate("", xy=(25, 40), xytext=(18, 52), arrowprops=dict(arrowstyle="->", color="#475569", lw=1.5))
    ax.text(23, 46, "1 : N", ha='center', va='center', fontsize=8, fontweight='bold', color="#475569")

    plt.tight_layout()
    out_path = os.path.join(OUTPUT_DIR, "fig_3_7_database_erd.png")
    fig.savefig(out_path, dpi=300, bbox_inches='tight')
    plt.close(fig)
    print(f"Generated: {out_path}")

def create_dashboard_ui_diagram():
    fig, ax = plt.subplots(figsize=(12, 7.5), dpi=300)
    ax.set_xlim(0, 100)
    ax.set_ylim(0, 100)
    ax.axis('off')
    
    # Outer browser window
    win = patches.FancyBboxPatch((2, 2), 96, 96, boxstyle="round,pad=0.5", facecolor="#0F172A", edgecolor="#334155", lw=1.5)
    ax.add_patch(win)
    
    # Browser bar dots
    ax.add_patch(patches.Circle((5.5, 95.5), 0.6, facecolor="#EF4444", edgecolor="none"))
    ax.add_patch(patches.Circle((7.5, 95.5), 0.6, facecolor="#F59E0B", edgecolor="none"))
    ax.add_patch(patches.Circle((9.5, 95.5), 0.6, facecolor="#10B981", edgecolor="none"))
    ax.text(50, 95.5, "https://fall-detection.sopon-project.me/dashboard", ha='center', va='center', fontsize=8.5, color="#94A3B8")
    
    # Header bar inside app
    hbar = patches.Rectangle((3, 87), 94, 6, facecolor="#1E293B", edgecolor="none")
    ax.add_patch(hbar)
    ax.text(6, 90, "SMART FALL DETECTION SYSTEM | Healthcare Admin Portal", ha='left', va='center', fontsize=11, fontweight='bold', color="#F8FAFC")
    ax.text(93, 90, "Admin: Administrator | Status: Online", ha='right', va='center', fontsize=8.5, color="#38BDF8")
    
    # 3 Summary Stat Cards
    # Stat 1
    s1 = patches.FancyBboxPatch((4, 73), 21, 12, boxstyle="round,pad=0.3", facecolor="#1E293B", edgecolor="#334155", lw=1)
    ax.add_patch(s1)
    ax.text(14.5, 81.5, "MONITORED ROOMS", ha='center', va='center', fontsize=8, color="#94A3B8")
    ax.text(14.5, 76.5, "4 Bathrooms", ha='center', va='center', fontsize=13, fontweight='bold', color="#38BDF8")

    # Stat 2
    s2 = patches.FancyBboxPatch((27, 73), 21, 12, boxstyle="round,pad=0.3", facecolor="#1E293B", edgecolor="#EF4444", lw=1.2)
    ax.add_patch(s2)
    ax.text(37.5, 81.5, "FALL ALARMS TODAY", ha='center', va='center', fontsize=8, color="#F87171")
    ax.text(37.5, 76.5, "1 Incident (RESOLVED)", ha='center', va='center', fontsize=11, fontweight='bold', color="#EF4444")

    # Stat 3
    s3 = patches.FancyBboxPatch((50, 73), 21, 12, boxstyle="round,pad=0.3", facecolor="#1E293B", edgecolor="#10B981", lw=1)
    ax.add_patch(s3)
    ax.text(60.5, 81.5, "SYSTEM HEALTH", ha='center', va='center', fontsize=8, color="#34D399")
    ax.text(60.5, 76.5, "100% Operational", ha='center', va='center', fontsize=12, fontweight='bold', color="#10B981")

    # Stat 4
    s4 = patches.FancyBboxPatch((73, 73), 23, 12, boxstyle="round,pad=0.3", facecolor="#1E293B", edgecolor="#6366F1", lw=1)
    ax.add_patch(s4)
    ax.text(84.5, 81.5, "AVG RESPONSE TIME", ha='center', va='center', fontsize=8, color="#A5B4FC")
    ax.text(84.5, 76.5, "1.85 sec (Webhook)", ha='center', va='center', fontsize=11, fontweight='bold', color="#818CF8")

    # Main Grid: Room Live Cards (Left)
    ax.text(4, 70, "REAL-TIME BATHROOM MONITORS", fontsize=10, fontweight='bold', color="#E2E8F0")

    # Room 1: Occupied
    r1 = patches.FancyBboxPatch((4, 40), 28, 28, boxstyle="round,pad=0.4", facecolor="#1E293B", edgecolor="#3B82F6", lw=1.5)
    ax.add_patch(r1)
    ax.text(18, 64, "BATHROOM #1 (Floor 1)", ha='center', va='center', fontsize=10, fontweight='bold', color="#F8FAFC")
    badge1 = patches.FancyBboxPatch((8, 56), 20, 5, boxstyle="round,pad=0.2", facecolor="#1D4ED8", edgecolor="none")
    ax.add_patch(badge1)
    ax.text(18, 58.5, "STATUS: OCCUPIED", ha='center', va='center', fontsize=8.5, fontweight='bold', color="#FFFFFF")
    ax.text(18, 50, "24 GHz Radar: Moving Target\n60 GHz Radar: Normal Height\nDuration: 4 mins 12 secs\nSignal: RSSI -58 dBm (Strong)", ha='center', va='center', fontsize=7.5, color="#94A3B8")

    # Room 2: Fall Alarm Active!
    r2 = patches.FancyBboxPatch((35, 40), 28, 28, boxstyle="round,pad=0.4", facecolor="#450A0A", edgecolor="#EF4444", lw=2)
    ax.add_patch(r2)
    ax.text(49, 64, "BATHROOM #2 (Floor 2)", ha='center', va='center', fontsize=10, fontweight='bold', color="#FEF2F2")
    badge2 = patches.FancyBboxPatch((39, 56), 20, 5, boxstyle="round,pad=0.2", facecolor="#DC2626", edgecolor="none")
    ax.add_patch(badge2)
    ax.text(49, 58.5, "! FALL DETECTED !", ha='center', va='center', fontsize=8.5, fontweight='bold', color="#FFFFFF")
    ax.text(49, 50, "60 GHz: Fall Confirmed (0.35m)\n24 GHz: Micro-motion Active\nBuzzer: SIREN ON (Local)\nLINE Alert: Sent (14:23:05)", ha='center', va='center', fontsize=7.5, color="#FECACA")

    # Room 3: Available
    r3 = patches.FancyBboxPatch((66, 40), 30, 28, boxstyle="round,pad=0.4", facecolor="#1E293B", edgecolor="#10B981", lw=1.5)
    ax.add_patch(r3)
    ax.text(81, 64, "BATHROOM #3 (Floor 3)", ha='center', va='center', fontsize=10, fontweight='bold', color="#F8FAFC")
    badge3 = patches.FancyBboxPatch((71, 56), 20, 5, boxstyle="round,pad=0.2", facecolor="#047857", edgecolor="none")
    ax.add_patch(badge3)
    ax.text(81, 58.5, "STATUS: AVAILABLE", ha='center', va='center', fontsize=8.5, fontweight='bold', color="#FFFFFF")
    ax.text(81, 50, "24 GHz Radar: Room Empty\n60 GHz Radar: Standby\nLast Cleaned: 12:40 PM\nSignal: RSSI -62 dBm", ha='center', va='center', fontsize=7.5, color="#94A3B8")

    # Bottom Area: Live Audit Log Table
    ax.text(4, 36, "RECENT TELEMETRY & EMERGENCY AUDIT LOG", fontsize=10, fontweight='bold', color="#E2E8F0")
    log_box = patches.FancyBboxPatch((4, 5), 92, 29, boxstyle="round,pad=0.4", facecolor="#1E293B", edgecolor="#334155", lw=1)
    ax.add_patch(log_box)

    log_rows = [
        ("14:23:07", "Bathroom #2", "WEBHOOK_DISPATCH", "LINE Flex Message successfully delivered to 2 guardians (HTTP 200)", "#34D399"),
        ("14:23:05", "Bathroom #2", "FALL_CONFIRMED", "60 GHz point cloud < 0.60m for 15s. FSM transitioned to ALARM_STATE", "#F87171"),
        ("14:22:50", "Bathroom #2", "FALL_SUSPECTED", "Negative vertical velocity detected (-1.82 m/s). Fall counter started", "#FBBF24"),
        ("14:18:45", "Bathroom #1", "OCCUPIED_EVENT", "24 GHz motion detected at distance 1.45m. Lights & Ventilation ON", "#38BDF8"),
        ("14:05:12", "Bathroom #3", "EMPTY_TIMEOUT", "No presence detected for 60 seconds. FSM transitioned to IDLE", "#94A3B8"),
    ]
    
    y_l = 29
    ax.text(6, y_l, "TIMESTAMP", fontsize=7.5, fontweight='bold', color="#64748B")
    ax.text(18, y_l, "DEVICE ID", fontsize=7.5, fontweight='bold', color="#64748B")
    ax.text(32, y_l, "EVENT TYPE", fontsize=7.5, fontweight='bold', color="#64748B")
    ax.text(50, y_l, "DETAILS & SYSTEM ACTIONS", fontsize=7.5, fontweight='bold', color="#64748B")
    
    for ts, dev, ev, det, c in log_rows:
        y_l -= 4.5
        ax.text(6, y_l, ts, fontsize=7.2, color="#94A3B8")
        ax.text(18, y_l, dev, fontsize=7.2, color="#E2E8F0")
        ax.text(32, y_l, ev, fontsize=7.2, fontweight='bold', color=c)
        ax.text(50, y_l, det, fontsize=7.2, color="#CBD5E1")

    plt.tight_layout()
    out_path = os.path.join(OUTPUT_DIR, "fig_3_8_dashboard_ui_mockup.png")
    fig.savefig(out_path, dpi=300, bbox_inches='tight')
    plt.close(fig)
    print(f"Generated: {out_path}")

def create_fall_testing_scenarios_diagram():
    fig, ax = plt.subplots(figsize=(12, 7.5), dpi=300)
    ax.set_xlim(0, 100)
    ax.set_ylim(0, 100)
    ax.axis('off')
    
    bg = patches.FancyBboxPatch((1, 1), 98, 98, boxstyle="round,pad=0.5", facecolor="#F8FAFC", edgecolor="#CBD5E1", lw=1.5)
    ax.add_patch(bg)
    
    ax.text(50, 95, "Experimental Fall Detection & ADL Testing Setup (การจำลองท่าทางการทดสอบ)", ha='center', va='center', fontsize=14, fontweight='bold', color="#0F172A")

    # Left Section: Physical Bathroom Setup (2.0m x 2.0m x 2.3m)
    p_box = patches.FancyBboxPatch((4, 8), 38, 82, boxstyle="round,pad=0.4", facecolor="#EFF6FF", edgecolor="#3B82F6", lw=1.5)
    ax.add_patch(p_box)
    ax.text(23, 86, "Experimental Testbed Setup\n(สภาพแวดล้อมห้องน้ำทดสอบ)", ha='center', va='center', fontsize=11, fontweight='bold', color="#1E40AF")
    
    # Ceiling & Sensor
    c_line = patches.Rectangle((8, 73), 30, 2, facecolor="#475569", edgecolor="none")
    ax.add_patch(c_line)
    ax.text(23, 77, "Ceiling Mounting (ความสูง 2.30 m)", ha='center', va='center', fontsize=8.5, fontweight='bold', color="#334155")

    radar = patches.FancyBboxPatch((19, 69), 8, 4, boxstyle="round,pad=0.2", facecolor="#2563EB", edgecolor="none")
    ax.add_patch(radar)
    ax.text(23, 71, "Dual Radar\n60G + 24G", ha='center', va='center', fontsize=7, fontweight='bold', color="#FFFFFF")

    # Radar Beam Cone
    beam = patches.Polygon([[23, 69], [9, 28], [37, 28]], closed=True, facecolor="#93C5FD", edgecolor="#3B82F6", alpha=0.35, ls='--')
    ax.add_patch(beam)
    ax.text(23, 50, "Detection Field: ±60° (120°)\nCoverage: 2.0 m × 2.0 m", ha='center', va='center', fontsize=8, color="#1D4ED8")

    # Floor & Threshold
    f_line = patches.Rectangle((8, 26), 30, 3, facecolor="#64748B", edgecolor="none")
    ax.add_patch(f_line)
    ax.text(23, 22.5, "Bathroom Floor (2.0 × 2.0 m)", ha='center', va='center', fontsize=8, color="#475569")

    # Threshold line
    ax.plot([8, 38], [34, 34], color="#DC2626", ls=':', lw=1.5)
    ax.text(23, 36, "Fall Threshold Plane = 0.60 m", ha='center', va='center', fontsize=7.5, fontweight='bold', color="#DC2626")

    # Testbed Specs
    ax.text(23, 15, "Sensors: HLK-LD6002C + HLK-LD2410C\nMicrocontroller: ESP32-S3 DevKit\nSampling Rate: 20 fps (60GHz TinyFrame)\nTarget Environment: Wet Tiles & Steam", ha='center', va='center', fontsize=7.5, color="#1E293B")

    # Right Section: 4 Fall Scenarios + 3 ADL Scenarios
    r_box = patches.FancyBboxPatch((45, 8), 51, 82, boxstyle="round,pad=0.4", facecolor="#FFFFFF", edgecolor="#CBD5E1", lw=1.5)
    ax.add_patch(r_box)
    ax.text(70.5, 86, "Simulated Fall Profiles & ADL Activities (80 Fall + 80 ADL Tests)", ha='center', va='center', fontsize=10.5, fontweight='bold', color="#0F172A")

    # 4 Fall Cards
    falls = [
        ("1. Forward Fall (ล้มไปข้างหน้า)", "20 Tests | Sensitivity 100%", "ผู้ทดสอบสูญเสียการทรงตัวล้มคว่ำหน้าลงกับพื้น\nระยะตรวจจับเฉลี่ย 33.74 วินาที (TP: 20/20)", "#EFF6FF", "#3B82F6"),
        ("2. Backward Fall (ล้มหงายหลัง)", "20 Tests | Sensitivity 100%", "ผู้ทดสอบลื่นล้มหงายหลังศีรษะและลำตัวกระทบพื้น\nระยะตรวจจับเฉลี่ย 23.56 วินาที (TP: 20/20)", "#EFF6FF", "#3B82F6"),
        ("3. Lateral Fall (ล้มไปด้านข้าง)", "20 Tests | Sensitivity 100%", "ผู้ทดสอบเซเสียหลักล้มตะแคงข้างซ้ายหรือขวา\nระยะตรวจจับเฉลี่ย 35.46 วินาที (TP: 20/20)", "#EFF6FF", "#3B82F6"),
        ("4. Collapse Fall (ล้มทรุดตัวลงกับที่)", "20 Tests | Sensitivity 100%", "ผู้ทดสอบหมดแรงเข่าทรุดตัวลงกองกับพื้นห้องน้ำ\nระยะตรวจจับเฉลี่ย 26.22 วินาที (TP: 20/20)", "#EFF6FF", "#3B82F6"),
    ]
    
    y_f = 69
    for title, stat, desc, bg_c, b_c in falls:
        fc = patches.FancyBboxPatch((47, y_f), 47, 12, boxstyle="round,pad=0.3", facecolor=bg_c, edgecolor=b_c, lw=1.2)
        ax.add_patch(fc)
        ax.text(49, y_f + 9, title, fontsize=8.5, fontweight='bold', color="#1E3A8A")
        ax.text(92, y_f + 9, stat, ha='right', fontsize=7.5, fontweight='bold', color="#2563EB")
        ax.text(49, y_f + 4.5, desc, fontsize=7.2, color="#475569")
        y_f -= 13.5

    # ADL Summary Card
    adl_box = patches.FancyBboxPatch((47, 11), 47, 14, boxstyle="round,pad=0.3", facecolor="#FEF3C7", edgecolor="#F59E0B", lw=1.2)
    ax.add_patch(adl_box)
    ax.text(49, 21.5, "Activities of Daily Living (ADL: กิจกรรมปกติในห้องน้ำ)", fontsize=8.5, fontweight='bold', color="#92400E")
    ax.text(92, 21.5, "80 Tests | Specificity 97.5%", ha='right', fontsize=7.5, fontweight='bold', color="#B45309")
    ax.text(49, 15, "- เดินเข้า-ออก / ยืนหน้าอ่างล้างหน้า (30 ครั้ง) -> แม่นยำ 100% (TN: 30)\n- นั่ง-ลุกจากชักโครก / ก้มหยิบของ (30 ครั้ง) -> แม่นยำ 96.7% (TN: 29, FP: 1)\n- อาบน้ำฝักบัวฉีดละอองน้ำแรงดันสูง (20 ครั้ง) -> แม่นยำ 95.0% (TN: 19, FP: 1)", fontsize=7.2, color="#78350F")

    plt.tight_layout()
    out_path = os.path.join(OUTPUT_DIR, "fig_4_5_testing_scenarios.png")
    fig.savefig(out_path, dpi=300, bbox_inches='tight')
    plt.close(fig)
    print(f"Generated: {out_path}")

if __name__ == '__main__':
    create_system_architecture_diagram()
    create_database_erd_diagram()
    create_dashboard_ui_diagram()
    create_fall_testing_scenarios_diagram()
    print("All diagrams generated successfully!")
