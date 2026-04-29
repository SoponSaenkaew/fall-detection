// app/page.tsx
import Link from 'next/link';

export default function Home() {
  return (
    <div className="min-h-screen flex flex-col items-center justify-center bg-gray-50 p-4">
      <h1 className="text-4xl font-bold text-blue-600 mb-4">Fall Detection System 🛰️</h1>
      <p className="text-gray-600 mb-8 text-center max-w-md">
        ระบบตรวจจับการล้มอัจฉริยะด้วยเทคโนโลยี mmWave 
        เพื่อความปลอดภัยของผู้สูงอายุที่คุณรัก
      </p>
      <div className="space-x-4">
        <Link href="/login" className="bg-blue-500 text-white px-6 py-2 rounded-lg hover:bg-blue-600 transition">
          เข้าสู่ระบบ
        </Link>
        <Link href="/register" className="bg-gray-200 text-gray-700 px-6 py-2 rounded-lg hover:bg-gray-300 transition">
          ลงทะเบียนใหม่
        </Link>
      </div>
    </div>
  );
}