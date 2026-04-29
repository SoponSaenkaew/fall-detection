"use client";
import { useState, useEffect } from 'react';
import { Plus, UserCircle, Mail } from 'lucide-react'; 
import axios from 'axios';

interface HeaderProps {
  onAddGroup: () => void;
}

export default function Header({ onAddGroup }: HeaderProps) {
  const [time, setTime] = useState<string>("");
  const [userData, setUserData] = useState({ name: "Loading...", email: "..." });

  useEffect(() => {
    // 1. จัดการเรื่องเวลาให้เดินทุกวินาที (แก้ปัญหา Hydration Mismatch)
    setTime(new Date().toLocaleTimeString('th-TH'));
    const interval = setInterval(() => {
      setTime(new Date().toLocaleTimeString('th-TH'));
    }, 1000);

    // 2. ดึงข้อมูลโปรไฟล์ผู้ใช้งานจากระบบหลังบ้าน
    const fetchProfile = async () => {
      try {
        const token = localStorage.getItem('token');
        const res = await axios.get('http://localhost:8080/api/v1/iot/profile', {
          headers: { Authorization: `Bearer ${token}` }
        });
        // สมมติว่า API ส่งกลับมาเป็น { your_id: "ชื่อผู้ใช้", email: "..." }
        setUserData({
          name: res.data.your_id || "User",
          email: res.data.email || "user@example.com"
        });
      } catch (err) {
        console.error("ดึงข้อมูลโปรไฟล์ไม่สำเร็จค่ะเซนเซย์");
      }
    };

    fetchProfile();
    return () => clearInterval(interval);
  }, []);

  return (
    <header className="flex justify-between items-center mb-8 border-b pb-4">
      {/* ฝั่งซ้าย: หัวข้อและเวลา */}
      <div className="text-left">
        <h1 className="text-3xl font-bold text-blue-800">IoT Dashboard 🖥️</h1>
        <p className="text-gray-500 text-sm">
          อัปเดตล่าสุด: {time}
        </p>
      </div>

      {/* ฝั่งขวา: ข้อมูลผู้ใช้และปุ่มเพิ่มกลุ่ม */}
      <div className="flex items-center gap-6">
        {/* ส่วนแสดงชื่อและ Email ผู้ใช้งาน */}
        <div className="flex items-center gap-3 bg-blue-50 px-4 py-2 rounded-xl border border-blue-100">
          <div className="bg-blue-600 p-2 rounded-full text-white">
            <UserCircle size={20} />
          </div>
          <div className="text-left leading-tight">
            <p className="text-sm font-bold text-blue-900">{userData.name}</p>
            <div className="flex items-center text-[10px] text-blue-400">
              <Mail size={10} className="mr-1" />
              {userData.email}
            </div>
          </div>
        </div>

        {/* ปุ่มเพิ่มกลุ่มใหม่ */}
        <button 
          onClick={onAddGroup} 
          className="flex items-center bg-blue-600 text-white px-4 py-2 rounded-lg hover:bg-blue-700 transition shadow-lg font-medium"
        >
          <Plus size={20} className="mr-2" /> เพิ่มกลุ่มใหม่
        </button>
      </div>
    </header>
  );
}