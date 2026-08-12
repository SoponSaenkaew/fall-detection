"use client";
import { useState, useEffect } from 'react';
import Header from '@/components/Dashboard/Header';
import { groupService } from '@/services/api';
import { Layout, Smartphone, Clock, UserCheck, AlertTriangle, X } from 'lucide-react';

export default function BuildingsPage() {
  const [groups, setGroups] = useState<any[]>([]);
  const [selectedGroupId, setSelectedGroupId] = useState<number | null>(null);
  const [alertMsg, setAlertMsg] = useState<string | null>(null);

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
                    className={`bg-white p-4 rounded-xl border transition-all h-auto flex flex-col justify-between ${
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


    </div>
  );
}
