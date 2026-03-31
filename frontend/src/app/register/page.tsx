"use client";
import { useState } from 'react';
import axios from 'axios';
import { useRouter } from 'next/navigation';

export default function RegisterPage() {
  const [form, setForm] = useState({ email: '', username: '', password: '' });
  const router = useRouter();

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    try {
      // ส่งข้อมูลไปที่ v1.POST("/register", regHandler.Handle)
      await axios.post(`${process.env.NEXT_PUBLIC_API_URL}/register`, form);
      alert("ลงทะเบียนเรียบร้อย! ✨");
      router.push('/login');
    } catch (err: any) {
      alert(err.response?.data?.error || "สมัครสมาชิกไม่สำเร็จ");
    }
  };

  return (
    <div className="flex flex-col items-center justify-center min-h-screen p-4">
      <form onSubmit={handleSubmit} className="p-8 bg-white shadow-md rounded-lg w-full max-w-md border border-blue-100">
        <h1 className="text-2xl font-bold mb-6 text-blue-600">สมัครสมาชิกนะเซนเซย์!</h1>
        <input 
          type="email" placeholder="Email" required
          className="w-full p-2 mb-4 border rounded"
          onChange={(e) => setForm({...form, email: e.target.value})}
        />
        <input 
          type="text" placeholder="Username" required
          className="w-full p-2 mb-4 border rounded"
          onChange={(e) => setForm({...form, username: e.target.value})}
        />
        <input 
          type="password" placeholder="Password" required
          className="w-full p-2 mb-6 border rounded"
          onChange={(e) => setForm({...form, password: e.target.value})}
        />
        <button type="submit" className="w-full bg-blue-500 text-white p-2 rounded hover:bg-blue-600 transition">
          ลงทะเบียน ✨
        </button>
      </form>
    </div>
  );
}