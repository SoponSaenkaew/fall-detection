"use client";
import { useState } from 'react';
import api from '@/lib/api';
import { useRouter } from 'next/navigation';
import Link from 'next/link';

export default function RegisterPage() {
  const [formData, setFormData] = useState({ email: '', username: '', password: '' });
  const router = useRouter();

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    try {
      // เรียกใช้ API Register
      await api.post('/register', formData);
      alert("ลงทะเบียนเรียบร้อยแล้วค่ะเซนเซย์! ✨");
      router.push('/login');
    } catch (err: any) {
      alert(err.response?.data?.error || "เกิดข้อผิดพลาดในการสมัครค่ะ");
    }
  };

  return (
    <div className="min-h-screen flex items-center justify-center bg-gray-100">
      <div className="bg-white p-8 rounded-2xl shadow-xl w-full max-cols-md">
        <h1 className="text-2xl font-bold text-center mb-6 text-blue-600">สมัครสมาชิกใหม่</h1>
        <form onSubmit={handleSubmit} className="space-y-4">
          <input 
            type="email" placeholder="Email" required
            className="w-full p-3 border rounded-lg focus:ring-2 focus:ring-blue-400 outline-none"
            onChange={(e) => setFormData({...formData, email: e.target.value})}
          />
          <input 
            type="text" placeholder="Username" required
            className="w-full p-3 border rounded-lg focus:ring-2 focus:ring-blue-400 outline-none"
            onChange={(e) => setFormData({...formData, username: e.target.value})}
          />
          <input 
            type="password" placeholder="Password" required
            className="w-full p-3 border rounded-lg focus:ring-2 focus:ring-blue-400 outline-none"
            onChange={(e) => setFormData({...formData, password: e.target.value})}
          />
          <button className="w-full bg-blue-500 text-white p-3 rounded-lg font-bold hover:bg-blue-600 transition">
            ลงทะเบียน
          </button>
        </form>
        <p className="mt-4 text-center text-sm">
          มีบัญชีอยู่แล้ว? <Link href="/login" className="text-blue-500 hover:underline">เข้าสู่ระบบที่นี่</Link>
        </p>
      </div>
    </div>
  );
}