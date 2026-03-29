"use client";
import { useState } from 'react';
import api from '@/lib/api';
import { useRouter } from 'next/navigation';

export default function LoginPage() {
  const [formData, setFormData] = useState({ email: '', password: '' });
  const router = useRouter();

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    try {
      // เรียกใช้ API Login
      const res = await api.post('/login', formData);
      // เก็บ Token ลง localStorage
      localStorage.setItem('token', res.data.token);
      router.push('/dashboard');
    } catch (err: any) {
      alert(err.response?.data?.error || "อีเมลหรือรหัสผ่านไม่ถูกต้องค่ะ");
    }
  };

  return (
    <div className="min-h-screen flex items-center justify-center bg-gray-50">
      <div className="bg-white p-8 rounded-2xl shadow-xl w-full max-w-md">
        <div className="text-center mb-8">
          <h1 className="text-3xl font-bold text-gray-800">ยินดีต้อนรับค่ะ!</h1>
          <p className="text-gray-500">กรุณาเข้าสู่ระบบเพื่อดูแลเซนเซอร์นะคะ</p>
        </div>
        <form onSubmit={handleSubmit} className="space-y-6">
          <div>
            <label className="block text-sm font-medium text-gray-700">Email</label>
            <input 
              type="email" required
              className="w-full mt-1 p-3 border rounded-lg bg-gray-50 focus:bg-white focus:ring-2 focus:ring-blue-400 outline-none transition"
              onChange={(e) => setFormData({...formData, email: e.target.value})}
            />
          </div>
          <div>
            <label className="block text-sm font-medium text-gray-700">Password</label>
            <input 
              type="password" required
              className="w-full mt-1 p-3 border rounded-lg bg-gray-50 focus:bg-white focus:ring-2 focus:ring-blue-400 outline-none transition"
              onChange={(e) => setFormData({...formData, password: e.target.value})}
            />
          </div>
          <button className="w-full bg-blue-600 text-white p-3 rounded-lg font-bold shadow-lg hover:shadow-none hover:bg-blue-700 transform active:scale-95 transition">
            เข้าสู่ระบบ
          </button>
        </form>
      </div>
    </div>
  );
}