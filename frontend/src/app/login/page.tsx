"use client";
import { useState } from 'react';
import { authService } from '@/services/api';
import { useRouter } from 'next/navigation';
import Link from 'next/link';
import { Activity, Lock, Mail, ArrowRight } from 'lucide-react';

export default function LoginPage() {
  const [form, setForm] = useState({ email: '', password: '' });
  const [loading, setLoading] = useState(false);
  const [errorMsg, setErrorMsg] = useState('');
  const router = useRouter();

  const handleLogin = async (e: React.FormEvent) => {
    e.preventDefault();
    setErrorMsg('');
    setLoading(true);
    try {
      const res = await authService.login(form);
      localStorage.setItem('token', res.data.token);
      router.push('/buildings');
    } catch (err: any) {
      setErrorMsg(err.response?.data?.error || 'เข้าสู่ระบบไม่สำเร็จ โปรดตรวจสอบอีเมลและรหัสผ่าน');
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="flex flex-col items-center justify-center min-h-screen bg-slate-900 px-4 py-8 relative overflow-hidden">
      {/* Glow Effects */}
      <div className="absolute top-1/4 left-1/2 -translate-x-1/2 w-96 h-96 bg-blue-600/20 rounded-full blur-3xl pointer-events-none" />
      <div className="absolute bottom-10 right-10 w-72 h-72 bg-indigo-600/10 rounded-full blur-2xl pointer-events-none" />

      <div className="w-full max-w-md bg-slate-800/90 backdrop-blur-xl border border-slate-700/80 rounded-2xl shadow-2xl p-8 z-10">
        <div className="flex flex-col items-center text-center mb-8">
          <div className="w-14 h-14 bg-gradient-to-tr from-blue-600 via-indigo-600 to-sky-500 rounded-2xl flex items-center justify-center shadow-lg shadow-blue-500/30 mb-4 text-white">
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.2" strokeLinecap="round" strokeLinejoin="round" className="w-8 h-8">
              <path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z" />
              <path d="M9 12h1.5l1.5-3 2 6 1.5-3H17" strokeWidth="2" />
            </svg>
          </div>
          <h1 className="text-2xl font-black text-white tracking-tight">
            SafeBathroom <span className="text-blue-400 font-extrabold">IoT</span>
          </h1>
          <p className="text-xs text-slate-400 mt-1.5">ระบบตรวจจับการล้มและดูแลความปลอดภัยในห้องน้ำ</p>
        </div>

        {errorMsg && (
          <div className="mb-6 p-3.5 bg-rose-500/10 border border-rose-500/30 rounded-xl text-rose-400 text-xs font-medium flex items-center gap-2">
            <span>⚠️</span>
            <span>{errorMsg}</span>
          </div>
        )}

        <form onSubmit={handleLogin} className="space-y-4">
          <div>
            <label className="block text-xs font-semibold text-slate-300 uppercase tracking-wider mb-2">อีเมล (Email)</label>
            <div className="relative">
              <Mail className="absolute left-3.5 top-1/2 -translate-y-1/2 w-4 h-4 text-slate-400" />
              <input
                type="email"
                placeholder="name@example.com"
                required
                value={form.email}
                className="w-full pl-10 pr-4 py-2.5 bg-slate-900/80 border border-slate-700 rounded-xl text-white placeholder-slate-500 text-sm focus:outline-none focus:ring-2 focus:ring-blue-500/50 focus:border-blue-500 transition"
                onChange={(e) => setForm({ ...form, email: e.target.value })}
              />
            </div>
          </div>

          <div>
            <label className="block text-xs font-semibold text-slate-300 uppercase tracking-wider mb-2">รหัสผ่าน (Password)</label>
            <div className="relative">
              <Lock className="absolute left-3.5 top-1/2 -translate-y-1/2 w-4 h-4 text-slate-400" />
              <input
                type="password"
                placeholder="••••••••"
                required
                value={form.password}
                className="w-full pl-10 pr-4 py-2.5 bg-slate-900/80 border border-slate-700 rounded-xl text-white placeholder-slate-500 text-sm focus:outline-none focus:ring-2 focus:ring-blue-500/50 focus:border-blue-500 transition"
                onChange={(e) => setForm({ ...form, password: e.target.value })}
              />
            </div>
          </div>

          <button
            type="submit"
            disabled={loading}
            className="w-full mt-2 py-3 px-4 bg-gradient-to-r from-blue-600 to-indigo-600 hover:from-blue-500 hover:to-indigo-500 text-white font-medium text-sm rounded-xl transition shadow-lg shadow-blue-600/30 flex items-center justify-center gap-2 disabled:opacity-50"
          >
            {loading ? 'กำลังเข้าสู่ระบบ...' : 'เข้าสู่ระบบ'}
            <ArrowRight className="w-4 h-4" />
          </button>
        </form>

        <div className="mt-8 pt-6 border-t border-slate-700/60 text-center">
          <p className="text-xs text-slate-400">
            ยังไม่มีบัญชีผู้ใช้งาน?{' '}
            <Link href="/register" className="text-blue-400 hover:text-blue-300 font-semibold transition ml-1">
              สมัครสมาชิกที่นี่
            </Link>
          </p>
        </div>
      </div>
    </div>
  );
}