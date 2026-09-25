"use client";
import { useState, useEffect } from 'react';
import { Plus, UserCircle, Mail, LogOut } from 'lucide-react'; 
import { authService } from '@/services/api';
import Link from 'next/link';
import { usePathname, useRouter } from 'next/navigation';

interface HeaderProps {
  onAddGroup?: () => void;
}

export default function Header({ onAddGroup }: HeaderProps) {
  const pathname = usePathname();
  const router = useRouter();
  const [time, setTime] = useState<string>("");
  const [userData, setUserData] = useState({ name: "กำลังโหลด...", email: "..." });

  useEffect(() => {
    // 1. จัดการเรื่องเวลาให้เดินทุกวินาที
    setTime(new Date().toLocaleTimeString('th-TH'));
    const interval = setInterval(() => {
      setTime(new Date().toLocaleTimeString('th-TH'));
    }, 1000);

    // 2. ดึงข้อมูลโปรไฟล์ผู้ใช้งานจากระบบหลังบ้าน
    const fetchProfile = async () => {
      try {
        const res = await authService.getProfile();
        setUserData({
          name: res.data.your_id || "ผู้ใช้งาน",
          email: res.data.email || "user@example.com"
        });
      } catch (err) {
        console.error("ดึงข้อมูลโปรไฟล์ไม่สำเร็จ");
      }
    };

    fetchProfile();
    return () => clearInterval(interval);
  }, []);

  const handleLogout = () => {
    if (typeof window !== 'undefined') {
      localStorage.removeItem('token');
      router.push('/login');
    }
  };

  return (
    <header className="flex flex-col md:flex-row justify-between items-stretch md:items-center mb-6 sm:mb-8 border-b pb-4 gap-4">
      {/* ฝั่งซ้าย: โลโก้, หัวข้อ และเมนูนำทาง */}
      <div className="flex flex-col lg:flex-row lg:items-center justify-between gap-4 lg:gap-6 text-left">
        <Link href="/buildings" className="flex items-center gap-3 group">
          <div className="w-10 h-10 sm:w-11 sm:h-11 rounded-xl bg-gradient-to-tr from-blue-600 via-indigo-600 to-sky-500 flex items-center justify-center shadow-md shadow-blue-500/20 text-white flex-shrink-0 group-hover:scale-105 transition-transform">
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.2" strokeLinecap="round" strokeLinejoin="round" className="w-6 h-6">
              <path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z" />
              <path d="M9 12h1.5l1.5-3 2 6 1.5-3H17" strokeWidth="2" />
            </svg>
          </div>
          <div>
            <div className="flex items-center gap-1.5">
              <h1 className="text-xl sm:text-2xl font-black text-gray-900 tracking-tight leading-none">
                SafeBathroom <span className="text-blue-600 font-extrabold">IoT</span>
              </h1>
            </div>
            <p className="text-gray-400 text-[11px] sm:text-xs mt-1">
              ระบบดูแลความปลอดภัยในห้องน้ำ • {time}
            </p>
          </div>
        </Link>
        <nav className="flex gap-1.5 bg-gray-100 p-1 rounded-xl self-start sm:self-auto">
          <Link href="/buildings" className={`px-3 sm:px-4 py-1.5 sm:py-2 rounded-lg text-xs sm:text-sm font-semibold transition ${pathname === '/buildings' ? 'bg-blue-600 text-white shadow-xs' : 'text-gray-600 hover:text-blue-600'}`}>
            ผังอาคาร (Monitor)
          </Link>
          <Link href="/history" className={`px-3 sm:px-4 py-1.5 sm:py-2 rounded-lg text-xs sm:text-sm font-semibold transition ${pathname === '/history' ? 'bg-blue-600 text-white shadow-xs' : 'text-gray-600 hover:text-blue-600'}`}>
            ประวัติเหตุการณ์ (History)
          </Link>
          <Link href="/dashboard" className={`px-3 sm:px-4 py-1.5 sm:py-2 rounded-lg text-xs sm:text-sm font-semibold transition ${pathname === '/dashboard' ? 'bg-blue-600 text-white shadow-xs' : 'text-gray-600 hover:text-blue-600'}`}>
            ตั้งค่าอุปกรณ์ (Admin)
          </Link>
        </nav>
      </div>

      {/* ฝั่งขวา: ข้อมูลผู้ใช้, ปุ่มออกจากระบบ และปุ่มเพิ่มกลุ่ม */}
      <div className="flex items-center justify-between sm:justify-end gap-2 sm:gap-3">
        {/* ส่วนแสดงชื่อและ Email ผู้ใช้งาน */}
        <div className="flex items-center gap-2.5 bg-blue-50 px-3 py-1.5 sm:px-3.5 sm:py-2 rounded-xl border border-blue-100/80 min-w-0 max-w-[200px] sm:max-w-none">
          <div className="bg-blue-600 p-1.5 rounded-full text-white flex-shrink-0">
            <UserCircle size={18} />
          </div>
          <div className="text-left leading-tight min-w-0 truncate">
            <p className="text-xs sm:text-sm font-bold text-blue-900 truncate">{userData.name}</p>
            <div className="flex items-center text-[10px] text-blue-500 truncate">
              <Mail size={10} className="mr-1 flex-shrink-0" />
              <span className="truncate">{userData.email}</span>
            </div>
          </div>
        </div>

        {/* ปุ่มออกจากระบบ */}
        <button
          onClick={handleLogout}
          title="ออกจากระบบ"
          className="flex items-center gap-1 px-2.5 sm:px-3 py-2 text-xs sm:text-sm text-red-600 bg-red-50 hover:bg-red-100 border border-red-200 rounded-xl transition font-medium flex-shrink-0"
        >
          <LogOut size={15} />
          <span className="hidden sm:inline">ออก</span>
        </button>

        {/* ปุ่มเพิ่มกลุ่มใหม่ (แสดงเฉพาะเมื่อมี callback) */}
        {onAddGroup && (
          <button 
            onClick={onAddGroup} 
            className="flex items-center bg-blue-600 text-white px-3 sm:px-4 py-2 rounded-xl hover:bg-blue-700 transition shadow-md font-medium text-xs sm:text-sm flex-shrink-0"
          >
            <Plus size={16} className="mr-1 sm:mr-1.5" /> 
            <span>เพิ่มกลุ่ม</span>
          </button>
        )}
      </div>
    </header>
  );
}