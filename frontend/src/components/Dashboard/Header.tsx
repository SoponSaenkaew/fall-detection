"use client"; // อย่าลืมใส่เพราะมีการกดปุ่ม (onClick) ค่ะ
import { Plus } from 'lucide-react'; // import ไอคอนที่ใช้

// กำหนด Type สำหรับ Props ที่จะรับมาจากหน้า Dashboard ค่ะ
interface HeaderProps {
  onAddGroup: () => void;
}

export default function Header({ onAddGroup }: HeaderProps) {
  return (
    <header className="flex justify-between items-center mb-8 border-b pb-4">
      <div className="text-left">
        <h1 className="text-3xl font-bold text-blue-800">IoT Dashboard 🖥️</h1>
        <p className="text-gray-500 text-sm">
          อัปเดตล่าสุด: {new Date().toLocaleTimeString('th-TH')}
        </p>
      </div>
      <button 
        onClick={onAddGroup} 
        className="flex items-center bg-blue-600 text-white px-4 py-2 rounded-lg hover:bg-blue-700 transition shadow-lg"
      >
        <Plus size={20} className="mr-2" /> เพิ่มกลุ่มใหม่
      </button>
    </header>
  );
}