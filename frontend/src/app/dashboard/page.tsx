"use client";
import { useEffect, useState } from 'react';
import axios from 'axios';
import { Layout, Smartphone, Clock, UserCheck } from 'lucide-react';

export default function Dashboard() {
  const [groups, setGroups] = useState<any[]>([]);

  // ฟังก์ชันช่วยแปลงรูปแบบเวลาให้เซนเซย์อ่านง่ายค่ะ
  const formatTime = (dateString: string) => {
    const date = new Date(dateString);
    return date.toLocaleTimeString('th-TH', { hour: '2-digit', minute: '2-digit' });
  };

  const fetchData = async () => {
    const token = localStorage.getItem('token');
    try {
      const res = await axios.get(`${process.env.NEXT_PUBLIC_API_URL}/iot/groups`, {
        headers: { Authorization: `Bearer ${token}` }
      });
      setGroups(res.data);
    } catch (err) {
      console.error("ดึงข้อมูลไม่สำเร็จค่ะเซนเซย์");
    }
  };

  useEffect(() => {
    fetchData();
    // ถ้าเซนเซย์อยากให้ข้อมูลอัปเดตอัตโนมัติ (Real-time แบบง่าย) 
    // สามารถเปิดใช้ Interval นี้ได้นะคะ
    const interval = setInterval(fetchData, 5000); // อัปเดตทุก 5 วินาที
    return () => clearInterval(interval);
  }, []);

  return (
    <div className="p-8 bg-gray-50 min-h-screen">
      <header className="flex justify-between items-center mb-8 border-b pb-4">
        <h1 className="text-3xl font-bold text-blue-800">ระบบตรวจจับการใช้ห้องน้ำ 🚽 ✨</h1>
        <p className="text-gray-500 text-sm">อัปเดตล่าสุด: {new Date().toLocaleTimeString('th-TH')}</p>
      </header>

      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
        {groups.map((group) => (
          <div key={group.ID} className="bg-white p-6 rounded-2xl shadow-md border border-blue-50">
            <div className="flex items-center mb-4 text-blue-600 border-b pb-2">
              <Layout className="mr-2" />
              <h2 className="text-xl font-bold">{group.name}</h2>
            </div>
            
            <div className="space-y-4">
              {group.devices?.map((dev: any) => (
                <div key={dev.ID} className="p-4 bg-slate-50 rounded-xl border border-gray-100">
                  <div className="flex justify-between items-start mb-2">
                    <div className="flex items-center font-medium text-gray-700">
                      <Smartphone size={18} className="mr-2 text-gray-400" />
                      {dev.name}
                    </div>
                    <span className={`text-[10px] px-2 py-0.5 rounded-full font-bold uppercase ${dev.status === 'online' ? 'bg-green-100 text-green-600' : 'bg-red-100 text-red-600'}`}>
                      {dev.status}
                    </span>
                  </div>

                  {/* ส่วนแสดงสถานะการใช้งานและเวลาที่อัปเดตล่าสุดค่ะเซนเซย์ */}
                  <div className="flex flex-col gap-2 mt-3 p-3 bg-white rounded-lg shadow-sm border border-blue-100">
                    <div className="flex items-center justify-between">
                      <div className="flex items-center text-sm text-gray-600">
                        <UserCheck size={16} className="mr-2 text-blue-400" />
                        <span>สถานะล่าสุด:</span>
                      </div>
                      <span className={`text-sm font-bold ${dev.latest_event === 'fall' ? 'text-red-500 animate-pulse' : 'text-blue-600'}`}>
                        {dev.latest_event === 'enter' ? 'กำลังใช้งาน 🚪' : 
                         dev.latest_event === 'exit' ? 'ว่าง 🟢' : 
                         dev.latest_event === 'fall' ? 'ตรวจพบคนล้ม! ⚠️' : 'ไม่มีข้อมูล'}
                      </span>
                    </div>
                    
                    <div className="flex items-center justify-between text-[11px] text-gray-400">
                      <div className="flex items-center">
                        <Clock size={12} className="mr-1" />
                        เวลาที่ตรวจพบ:
                      </div>
                      <span>{formatTime(dev.UpdatedAt)}</span>
                    </div>
                  </div>
                </div>
              ))}
              {(!group.devices || group.devices.length === 0) && (
                <p className="text-center text-gray-400 text-sm py-4">ยังไม่มีอุปกรณ์ในกลุ่มนี้ค่ะ</p>
              )}
            </div>
          </div>
        ))}
      </div>
    </div>
  );
}