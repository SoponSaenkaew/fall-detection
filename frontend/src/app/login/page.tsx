"use client";
import { useState } from 'react';
import { authService } from '@/services/api';
import { useRouter } from 'next/navigation';

export default function LoginPage() {
  const [form, setForm] = useState({ email: '', password: '' });
  const router = useRouter();

  const handleLogin = async (e: React.FormEvent) => {
    e.preventDefault();
    try {
      // เรียกใช้ v1.POST("/login", loginHandler.Handle)
      const res = await authService.login(form);
      // เก็บ Token ลงใน localStorage เพื่อใช้ผ่าน AuthMiddleware
      localStorage.setItem('token', res.data.token);
      router.push('/dashboard');
    } catch (err: any) {
      alert(err.response?.data?.error || "Login ไม่สำเร็จ");
    }
  };

  return (
    <div className="flex flex-col items-center justify-center min-h-screen">
      <form onSubmit={handleLogin} className="p-8 bg-white shadow-lg rounded-xl border-t-4 border-blue-400">
        <h1 className="text-2xl font-bold mb-4">ยินดีต้อนรับกลับค่ะ!</h1>
        <input 
          type="email" placeholder="Email" required
          className="w-full p-2 mb-4 border rounded"
          onChange={(e) => setForm({...form, email: e.target.value})}
        />
        <input 
          type="password" placeholder="Password" required
          className="w-full p-2 mb-6 border rounded"
          onChange={(e) => setForm({...form, password: e.target.value})}
        />
        <button type="submit" className="w-full bg-blue-500 text-white p-2 rounded">เข้าสู่ระบบ ✨</button>
      </form>
    </div>
  );
}