"use client";
import { useState, useEffect } from 'react';
import { Plus, UserCircle, Mail } from 'lucide-react'; 
import axios from 'axios';
import Link from 'next/link';
import { usePathname } from 'next/navigation';

interface HeaderProps {
  onAddGroup?: () => void;
}

export default function Header({ onAddGroup }: HeaderProps) {
  const pathname = usePathname();
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
        const res = await axios.get('http://localhost:8080/api/v1/iot/profile');
        // สมมติว่า API ส่งกลับมาเป็น { your_id: "ชื่อผู้ใช้", email: "..." }
        setUserData({
          name: res.data.your_id || "User",
          email: res.data.email || "user@example.com"
        });
      } catch (err) {
        console.error("ดึงข้อมูลโปรไฟล์ไม่สำเร็จ");
      }
    };

    fetchProfile();
    return () => clearInterval(interval);
  }, []);

  return (
    <header className="flex justify-between items-center mb-8 border-b pb-4">
      {/* ฝั่งซ้าย: หัวข้อและเวลา พร้อมเมนูนำทาง */}
      <div className="text-left flex items-baseline gap-8">
        <div>
          <h1 className="text-3xl font-bold text-blue-800">IoT System 🖥️</h1>
          <p className="text-gray-500 text-sm">
            อัปเดตล่าสุด: {time}
          </p>
        </div>
        <nav className="flex gap-2 bg-gray-100 p-1 rounded-xl">
          <Link href="/buildings" className={`px-4 py-2 rounded-lg text-sm font-semibold transition ${pathname === '/buildings' ? 'bg-blue-600 text-white shadow-sm' : 'text-gray-600 hover:text-blue-600'}`}>
            ผังอาคาร (Monitor)
          </Link>
          <Link href="/dashboard" className={`px-4 py-2 rounded-lg text-sm font-semibold transition ${pathname === '/dashboard' ? 'bg-blue-600 text-white shadow-sm' : 'text-gray-600 hover:text-blue-600'}`}>
            ตั้งค่าอุปกรณ์ (Admin)
          </Link>
        </nav>
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

        {/* ปุ่มเพิ่มกลุ่มใหม่ (แสดงเฉพาะเมื่อมี callback) */}
        {onAddGroup && (
          <button 
            onClick={onAddGroup} 
            className="flex items-center bg-blue-600 text-white px-4 py-2 rounded-lg hover:bg-blue-700 transition shadow-lg font-medium"
          >
            <Plus size={20} className="mr-2" /> เพิ่มกลุ่มใหม่
          </button>
        )}
      </div>
    </header>
  );
}