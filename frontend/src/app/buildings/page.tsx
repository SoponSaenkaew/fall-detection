"use client";
import { useState, useEffect, useRef } from 'react';
import Header from '@/components/Dashboard/Header';
import { groupService, deviceService } from '@/services/api';
import { Layout, Smartphone, Clock, UserCheck, ChevronRight, AlertTriangle, X, Activity } from 'lucide-react';

export default function BuildingsPage() {
  const [groups, setGroups] = useState<any[]>([]);
  const [selectedGroupId, setSelectedGroupId] = useState<number | null>(null);
  const [selectedDev, setSelectedDev] = useState<any>(null);
  const [deviceSettings, setDeviceSettings] = useState<any>(null);
  const [showModal, setShowModal] = useState(false);
  const [alertMsg, setAlertMsg] = useState<string | null>(null);

  const canvasRef = useRef<HTMLCanvasElement>(null);

  // ดึงข้อมูลกลุ่มสถานที่และอุปกรณ์ทั้งหมด
  const fetchData = async () => {
    try {
      const res = await groupService.getAll();
      setGroups(res.data);
      // เลือกกลุ่มแรกเป็นค่าเริ่มต้นหากยังไม่ได้เลือก
      if (res.data.length > 0 && selectedGroupId === null) {
        setSelectedGroupId(res.data[0].ID);
      }
    } catch (err) {
      console.error("ดึงข้อมูลอาคารไม่สำเร็จ:", err);
    }
  };

  // ตรวจจับเหตุการณ์เรียลไทม์ผ่าน WebSocket
  useEffect(() => {
    fetchData();
    const wsProtocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
    const wsHost = window.location.host;
    const wsUrl = (window.location.hostname === 'localhost' || window.location.hostname === '127.0.0.1')
      ? 'ws://localhost:8080/ws'
      : `${wsProtocol}//${wsHost}/ws`;
    const ws = new WebSocket(wsUrl);

    ws.onopen = () => console.log('เชื่อมต่อ WebSocket สำหรับบอร์ดสำเร็จ');
    ws.onmessage = (event) => {
      const data = JSON.parse(event.data);
      fetchData(); // ดึงข้อมูลใหม่เพื่อรีเฟรชสถานะบอร์ด
      
      if (data.name === 'fall') {
        setAlertMsg(`🚨 เกิดเหตุล้มฉุกเฉิน! บอร์ดไอดี: ${data.device_id} โปรดเข้าช่วยเหลือทันที`);
      }
    };

    return () => {
      ws.close();
      console.log('ปิดการเชื่อมต่อ WebSocket');
    };
  }, []);

  // เมื่อเลือกดูรายละเอียดของอุปกรณ์/ห้อง
  const handleViewDetails = async (dev: any) => {
    setSelectedDev(dev);
    try {
      const res = await deviceService.getSettings(dev.device_id);
      setDeviceSettings(res.data);
      setShowModal(true);
    } catch (err) {
      console.error("ดึงการตั้งค่าห้องไม่สำเร็จ:", err);
      // ใช้ค่าเริ่มต้นหากไม่พบค่าการตั้งค่าจาก API
      setDeviceSettings({
        room_width: 4.0,
        room_length: 4.0,
        mount_height: 2.0
      });
      setShowModal(true);
    }
  };

  // วาดหน้าจอและเส้นทางเดินลงบน Canvas 2D
  useEffect(() => {
    if (!showModal || !canvasRef.current || !deviceSettings || !selectedDev) return;

    const canvas = canvasRef.current;
    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    ctx.clearRect(0, 0, canvas.width, canvas.height);

    const roomW = deviceSettings.room_width || 4.0;
    const roomL = deviceSettings.room_length || 4.0;

    const scale = Math.min((canvas.width - 40) / roomW, (canvas.height - 40) / roomL);

    const sensorX = canvas.width / 2;
    const sensorY = 20;

    ctx.strokeStyle = '#94a3b8'; 
    const roomLeft = sensorX - (roomW / 2) * scale;
    const roomTop = sensorY;
    const roomWidthPx = roomW * scale;
    const roomHeightPx = roomL * scale;

    // 1. เติมสีพื้นหลังห้องน้ำ
    ctx.fillStyle = '#f8fafc';
    ctx.fillRect(roomLeft, roomTop, roomWidthPx, roomHeightPx);

    // 2. วาดเส้นตาราง Grid พื้นห้องน้ำ (เส้นประขนาด 1 เมตร)
    ctx.strokeStyle = '#cbd5e1';
    ctx.lineWidth = 0.5;
    ctx.setLineDash([4, 4]);

    for (let x = -Math.floor(roomW / 2); x <= Math.floor(roomW / 2); x++) {
      const gridX = sensorX + x * scale;
      if (gridX >= roomLeft && gridX <= roomLeft + roomWidthPx) {
        ctx.beginPath();
        ctx.moveTo(gridX, roomTop);
        ctx.lineTo(gridX, roomTop + roomHeightPx);
        ctx.stroke();
      }
    }
    for (let y = 1; y <= roomL; y++) {
      const gridY = sensorY + y * scale;
      if (gridY <= roomTop + roomHeightPx) {
        ctx.beginPath();
        ctx.moveTo(roomLeft, gridY);
        ctx.lineTo(roomLeft + roomWidthPx, gridY);
        ctx.stroke();
      }
    }
    ctx.setLineDash([]); // รีเซ็ตเส้นประ

    // 3. วาดผนังห้องน้ำหลัก (ซ้าย, บน, ขวา)
    ctx.strokeStyle = '#94a3b8'; // สีเทาเข้มผนัง
    ctx.lineWidth = 3;
    ctx.beginPath();
    ctx.moveTo(roomLeft, roomTop);
    ctx.lineTo(roomLeft + roomWidthPx, roomTop); // ผนังบน
    ctx.moveTo(roomLeft, roomTop);
    ctx.lineTo(roomLeft, roomTop + roomHeightPx); // ผนังซ้าย
    ctx.moveTo(roomLeft + roomWidthPx, roomTop);
    ctx.lineTo(roomLeft + roomWidthPx, roomTop + roomHeightPx); // ผนังขวา
    ctx.stroke();

    // 4. วาดผนังด้านล่างพร้อมช่องทางเข้าออกทางเดียว (Single Entrance/Exit Door)
    const doorWidthPx = Math.min(40, roomWidthPx * 0.25);
    const doorStart = roomLeft + (roomWidthPx - doorWidthPx) / 2;
    const doorEnd = doorStart + doorWidthPx;

    ctx.beginPath();
    ctx.moveTo(roomLeft, roomTop + roomHeightPx);
    ctx.lineTo(doorStart, roomTop + roomHeightPx); // ผนังล่างซ้าย
    ctx.moveTo(doorEnd, roomTop + roomHeightPx);
    ctx.lineTo(roomLeft + roomWidthPx, roomTop + roomHeightPx); // ผนังล่างขวา
    ctx.stroke();

    // วาดบานประตูและเส้นประวงสวิง
    ctx.strokeStyle = '#64748b';
    ctx.lineWidth = 1.5;
    ctx.beginPath();
    const doorAngle = -Math.PI / 4; // มุมเปิดประตูเข้าห้อง 45 องศา
    const doorX = doorStart + doorWidthPx * Math.cos(doorAngle);
    const doorY = roomTop + roomHeightPx + doorWidthPx * Math.sin(doorAngle);
    ctx.moveTo(doorStart, roomTop + roomHeightPx);
    ctx.lineTo(doorX, doorY);
    ctx.stroke();

    ctx.strokeStyle = '#94a3b8';
    ctx.setLineDash([2, 2]);
    ctx.beginPath();
    ctx.arc(doorStart, roomTop + roomHeightPx, doorWidthPx, 0, doorAngle, true);
    ctx.stroke();
    ctx.setLineDash([]); // รีเซ็ต

    // 5. วาดรูปสัญลักษณ์ชักโครก (Toilet Bowl) กึ่งกลางด้านบน
    // วาดถังพักน้ำชักโครก
    ctx.fillStyle = '#e2e8f0';
    ctx.strokeStyle = '#475569';
    ctx.lineWidth = 1.5;
    ctx.fillRect(sensorX - 16, roomTop + 3, 32, 10);
    ctx.strokeRect(sensorX - 16, roomTop + 3, 32, 10);

    // วาดโถสุขภัณฑ์ทรงไข่
    ctx.fillStyle = '#ffffff';
    ctx.beginPath();
    ctx.ellipse(sensorX, roomTop + 22, 10, 13, 0, 0, 2 * Math.PI);
    ctx.fill();
    ctx.stroke();

    // วาดฝารองนั่งด้านใน
    ctx.beginPath();
    ctx.ellipse(sensorX, roomTop + 22, 7, 10, 0, 0, 2 * Math.PI);
    ctx.stroke();

    // 6. วาดติดตั้งเซนเซอร์ (RADAR) ไว้บนผนังหลังชักโครก
    ctx.fillStyle = '#3b82f6';
    ctx.beginPath();
    ctx.arc(sensorX, roomTop, 6, 0, Math.PI, false); // วาดรูปครึ่งวงกลมแนบผนัง
    ctx.fill();
    ctx.fillStyle = '#1e3a8a';
    ctx.font = 'bold 8px sans-serif';
    ctx.fillText('RADAR', sensorX - 15, roomTop - 3);

    const metaStr = selectedDev.latest_event_metadata;
    let metaData: any = {};
    try {
      if (metaStr) {
        metaData = JSON.parse(metaStr);
      }
    } catch (e) {
      console.error("แกะข้อมูลเมตาไม่สำเร็จ:", e);
    }

    if (selectedDev.latest_event === 'fall') {
      const fallX = metaData.fall_x || 0.0;
      const fallY = metaData.fall_y || 2.0;

      const plotX = sensorX + fallX * scale;
      const plotY = sensorY + fallY * scale;

      ctx.strokeStyle = 'rgba(239, 68, 68, 0.4)';
      ctx.lineWidth = 6;
      ctx.beginPath();
      ctx.arc(plotX, plotY, 18, 0, 2 * Math.PI);
      ctx.stroke();

      ctx.strokeStyle = '#ef4444';
      ctx.lineWidth = 4;
      ctx.beginPath();
      ctx.moveTo(plotX - 10, plotY - 10);
      ctx.lineTo(plotX + 10, plotY + 10);
      ctx.moveTo(plotX - 10, plotY + 10);
      ctx.lineTo(plotX + 10, plotY - 10);
      ctx.stroke();

      ctx.fillStyle = '#ef4444';
      ctx.font = 'bold 11px sans-serif';
      ctx.fillText(`จุดที่ล้ม (X:${fallX.toFixed(2)}, Y:${fallY.toFixed(2)})`, plotX + 14, plotY + 4);
    } 
    else if (selectedDev.latest_event === 'exit' && metaData.path && metaData.path.length > 0) {
      const path: [number, number][] = metaData.path;

      ctx.strokeStyle = '#10b981';
      ctx.lineWidth = 3;
      ctx.beginPath();

      path.forEach((pt, idx) => {
        const ptX = sensorX + pt[0] * scale;
        const ptY = sensorY + pt[1] * scale;
        if (idx === 0) {
          ctx.moveTo(ptX, ptY);
        } else {
          ctx.lineTo(ptX, ptY);
        }
      });
      ctx.stroke();

      ctx.fillStyle = '#059669';
      path.forEach((pt) => {
        const ptX = sensorX + pt[0] * scale;
        const ptY = sensorY + pt[1] * scale;
        ctx.beginPath();
        ctx.arc(ptX, ptY, 4, 0, 2 * Math.PI);
        ctx.fill();
      });

      const startX = sensorX + path[0][0] * scale;
      const startY = sensorY + path[0][1] * scale;
      const endX = sensorX + path[path.length - 1][0] * scale;
      const endY = sensorY + path[path.length - 1][1] * scale;

      ctx.fillStyle = '#2563eb';
      ctx.font = 'bold 9px sans-serif';
      ctx.fillText("เริ่ม", startX + 6, startY + 3);

      ctx.fillStyle = '#dc2626';
      ctx.font = 'bold 9px sans-serif';
      ctx.fillText("ออก", endX + 6, endY + 3);
    }
  }, [showModal, deviceSettings, selectedDev]);

  const activeGroup = groups.find(g => g.ID === selectedGroupId);

  return (
    <div className="p-8 bg-gray-50 min-h-screen">
      {/* 1. Header ส่วนนำทางแบบไม่ต้องใช้ Auth */}
      <Header />

      {/* แถบแจ้งเตือนภัยแบบเร่งด่วนด้านบนสุด */}
      {alertMsg && (
        <div className="mb-6 p-4 bg-red-100 border border-red-200 text-red-800 rounded-2xl flex items-center justify-between shadow-md animate-pulse">
          <div className="flex items-center">
            <AlertTriangle className="mr-3 text-red-600" size={24} />
            <span className="font-bold text-sm md:text-base">{alertMsg}</span>
          </div>
          <button 
            onClick={() => setAlertMsg(null)} 
            className="text-red-800 hover:text-black font-bold p-1"
          >
            <X size={20} />
          </button>
        </div>
      )}

      {/* 2. แท็บเลือกอาคาร (Building Tab Selector) */}
      <div className="flex items-center gap-2 mb-6 border-b pb-3 overflow-x-auto">
        <Layout className="text-gray-400 mr-1" size={20} />
        {groups.map((group) => (
          <button
            key={group.ID}
            onClick={() => {
              setSelectedGroupId(group.ID);
              setSelectedDev(null);
            }}
            className={`px-5 py-2.5 rounded-xl font-bold text-sm transition ${
              selectedGroupId === group.ID
                ? 'bg-blue-600 text-white shadow-md'
                : 'bg-white text-gray-600 hover:bg-gray-100 border border-gray-200'
            }`}
          >
            {group.name}
          </button>
        ))}
      </div>

      {/* 3. ตารางการ์ดห้องน้ำรายอาคาร (Rooms Grid) */}
      {activeGroup && (
        <div>
          <h2 className="text-xl font-bold text-gray-800 mb-4 text-left">
            ห้องน้ำทั้งหมดใน {activeGroup.name}
          </h2>
          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
            {activeGroup.devices && activeGroup.devices.length > 0 ? (
              activeGroup.devices.map((dev: any) => {
                const isFall = dev.latest_event === 'fall';
                const isEnter = dev.latest_event === 'enter';
                const isExit = dev.latest_event === 'exit';

                return (
                  <div 
                    key={dev.ID} 
                    className={`bg-white p-4 rounded-xl border transition-all h-[215px] flex flex-col justify-between ${
                      isFall 
                        ? 'border-red-500 ring-4 ring-red-100 shadow-lg' 
                        : 'border-gray-200 shadow-sm hover:shadow-md'
                    }`}
                  >
                    <div>
                      {/* ข้อมูลอุปกรณ์ - อ้างอิงสไตล์เดียวกับ DeviceCard.tsx */}
                      <div className="flex justify-between items-start mb-2 text-left">
                        <div className="flex items-center font-medium text-gray-700 text-sm">
                          <Smartphone size={16} className="mr-2 text-blue-400 flex-shrink-0" />
                          <span className="truncate max-w-[125px]" title={dev.name}>{dev.name}</span>
                        </div>
                        <span className={`text-[10px] px-2 py-0.5 rounded-full font-bold uppercase flex-shrink-0 ${
                          dev.status === 'online' ? 'bg-green-100 text-green-600' : 'bg-gray-100 text-gray-500'
                        }`}>
                          {dev.status}
                        </span>
                      </div>

                      {/* สถานะความปลอดภัย - อ้างอิงสไตล์เดียวกับ DeviceCard.tsx */}
                      <div className="flex flex-col gap-2 mt-3 p-3 bg-slate-50 rounded-lg border border-gray-100">
                        <div className="flex items-center justify-between">
                          <div className="flex items-center text-sm text-gray-600 flex-shrink-0">
                            <UserCheck size={16} className="mr-2 text-blue-400 flex-shrink-0" />
                            <span>สถานะ:</span>
                          </div>
                          <span className={`text-sm font-bold whitespace-nowrap flex-shrink-0 ${
                            isFall ? 'text-red-500 animate-bounce' : 'text-blue-600'
                          }`}>
                            {isFall 
                              ? 'คนล้ม!!! ⚠️' 
                              : isEnter 
                              ? 'กำลังใช้งาน 🚪' 
                              : isExit 
                              ? 'ว่าง 🟢' 
                              : 'ไม่มีข้อมูล'}
                          </span>
                        </div>
                        <div className="flex items-center justify-between text-[11px] text-gray-400">
                          <div className="flex items-center flex-shrink-0">
                            <Clock size={12} className="mr-1 flex-shrink-0" />
                            <span>อัปเดตเมื่อ:</span>
                          </div>
                          <span className="whitespace-nowrap">
                            {new Date(dev.UpdatedAt).toLocaleTimeString('th-TH')}
                          </span>
                        </div>
                      </div>
                    </div>

                    {/* ปุ่มเรียกดูพิกัด/ประวัติ */}
                    <button
                      onClick={() => handleViewDetails(dev)}
                      className={`w-full py-1.5 rounded-lg font-bold text-xs transition flex items-center justify-center ${
                        isFall 
                          ? 'bg-red-600 text-white hover:bg-red-700 shadow-sm animate-pulse' 
                          : 'bg-blue-50 text-blue-600 hover:bg-blue-100'
                      }`}
                    >
                      ดูแผนผังและประวัติการเดิน <ChevronRight size={14} className="ml-1" />
                    </button>
                  </div>
                );
              })
            ) : (
              <div className="col-span-full bg-white p-8 rounded-2xl border text-center text-gray-400">
                ยังไม่มีข้อมูลห้องน้ำลงทะเบียนในอาคารนี้
              </div>
            )}
          </div>
        </div>
      )}

      {/* 4. หน้าต่างป๊อปอัปแสดงผลลัพธ์ผังห้อง 2D และประวัติย้อนหลัง (Room Details Modal) */}
      {showModal && selectedDev && deviceSettings && (
        <div className="fixed inset-0 bg-black/60 flex items-center justify-center z-50 p-4 text-left">
          <div className="bg-white rounded-3xl w-full max-w-4xl shadow-2xl overflow-hidden flex flex-col md:flex-row max-h-[90vh]">
            
            {/* ฝั่งซ้าย: Canvas วาดผังห้อง 2D */}
            <div className="p-6 bg-slate-100 flex flex-col items-center justify-center border-r border-gray-100 md:w-1/2">
              <h3 className="font-bold text-gray-700 mb-3 text-sm">
                แผนผังห้องตรวจจับแบบ 2D (ขนาด {deviceSettings.room_width} x {deviceSettings.room_length} ม.)
              </h3>
              
              <div className="bg-white p-2 rounded-2xl shadow-inner border border-gray-200">
                <canvas 
                  ref={canvasRef} 
                  width={340} 
                  height={380} 
                  className="rounded-xl bg-slate-50"
                />
              </div>
              
              <div className="flex gap-4 mt-3 text-xs text-gray-500">
                <span className="flex items-center"><span className="w-2.5 h-2.5 rounded-full bg-blue-500 inline-block mr-1"></span>เซนเซอร์</span>
                <span className="flex items-center"><span className="w-2.5 h-2.5 rounded-full bg-green-500 inline-block mr-1"></span>เส้นทางเดินปกติ</span>
                <span className="flex items-center"><span className="w-2.5 h-2.5 rounded-full bg-red-500 inline-block mr-1"></span>จุดที่ตรวจพบการล้ม</span>
              </div>
            </div>

            {/* ฝั่งขวา: รายละเอียดสถิติ และ ประวัติการเข้าใช้ */}
            <div className="p-6 flex-1 flex flex-col overflow-y-auto">
              <div className="flex justify-between items-start mb-6 border-b pb-4">
                <div>
                  <h2 className="text-2xl font-bold text-gray-800">{selectedDev.name}</h2>
                  <p className="text-xs text-gray-400">พารามิเตอร์เซนเซอร์: เกณฑ์ล้ม {deviceSettings.fall_threshold} | สูง {deviceSettings.mount_height} ม.</p>
                </div>
                <button 
                  onClick={() => setShowModal(false)}
                  className="p-1 bg-gray-100 hover:bg-gray-200 rounded-full transition"
                >
                  <X size={20} className="text-gray-500" />
                </button>
              </div>

              {/* การ์ดสถิติอย่างย่อ */}
              <div className="grid grid-cols-2 gap-4 mb-6">
                <div className="p-4 bg-blue-50 rounded-2xl border border-blue-100 text-left">
                  <span className="text-xs text-blue-500 font-bold">เข้าใช้วันนี้สะสม</span>
                  <p className="text-2xl font-black text-blue-800 mt-1">6 ครั้ง</p>
                </div>
                <div className="p-4 bg-emerald-50 rounded-2xl border border-emerald-100 text-left">
                  <span className="text-xs text-emerald-500 font-bold">ระยะเวลาเฉลี่ย</span>
                  <p className="text-2xl font-black text-emerald-800 mt-1">4.5 นาที</p>
                </div>
              </div>

              {/* รายการเหตุการณ์ประวัติย้อนหลัง */}
              <h3 className="font-bold text-gray-700 text-sm mb-3">บันทึกประวัติการใช้งานล่าสุด</h3>
              <div className="space-y-3 flex-1 overflow-y-auto pr-1">
                {/* แถวที่ 1: เหตุการณ์ปัจจุบัน */}
                <div className="p-3 bg-slate-50 rounded-xl border border-gray-100 flex items-center justify-between text-xs">
                  <div className="text-left">
                    <p className="font-bold text-gray-700">สถานะล่าสุดในระบบ</p>
                    <p className="text-gray-400">อัปเดตเมื่อสักครู่</p>
                  </div>
                  <span className={`font-bold px-2 py-1 rounded ${
                    selectedDev.latest_event === 'fall' 
                      ? 'bg-red-100 text-red-600' 
                      : 'bg-green-100 text-green-600'
                  }`}>
                    {selectedDev.latest_event === 'fall' ? '🚨 ตรวจพบคนล้ม' : '🟢 ปลอดภัยปกติ'}
                  </span>
                </div>

                {/* แถวที่ 2: ข้อมูลประวัติสมมติย้อนหลัง 3 แถวเพื่อโชว์รายงานข้อมูล */}
                <div className="p-3 bg-slate-50 rounded-xl border border-gray-100 flex items-center justify-between text-xs">
                  <div className="text-left">
                    <p className="font-bold text-gray-700">ระยะการเข้าใช้ปกติ: 10:15 - 10:20</p>
                    <p className="text-gray-400">วันนี้ (รวม 5 นาที)</p>
                  </div>
                  <span className="font-bold px-2 py-1 rounded bg-green-100 text-green-600">
                    🟢 ปลอดภัยปกติ
                  </span>
                </div>
                
                <div className="p-3 bg-slate-50 rounded-xl border border-gray-100 flex items-center justify-between text-xs">
                  <div className="text-left">
                    <p className="font-bold text-gray-700">ระยะการเข้าใช้ปกติ: 09:30 - 09:34</p>
                    <p className="text-gray-400">วันนี้ (รวม 4 นาที)</p>
                  </div>
                  <span className="font-bold px-2 py-1 rounded bg-green-100 text-green-600">
                    🟢 ปลอดภัยปกติ
                  </span>
                </div>

                <div className="p-3 bg-slate-50 rounded-xl border border-gray-100 flex items-center justify-between text-xs">
                  <div className="text-left">
                    <p className="font-bold text-gray-700">ระยะการเข้าใช้ปกติ: 08:02 - 08:08</p>
                    <p className="text-gray-400">วันนี้ (รวม 6 นาที)</p>
                  </div>
                  <span className="font-bold px-2 py-1 rounded bg-green-100 text-green-600">
                    🟢 ปลอดภัยปกติ
                  </span>
                </div>
              </div>
            </div>

          </div>
        </div>
      )}
    </div>
  );
}
