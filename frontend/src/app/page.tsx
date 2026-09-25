"use client";
import { useEffect } from 'react';
import { useRouter } from 'next/navigation';

export default function Home() {
  const router = useRouter();
  useEffect(() => {
    const token = typeof window !== 'undefined' ? localStorage.getItem('token') : null;
    if (token) {
      router.push('/buildings');
    } else {
      router.push('/login');
    }
  }, [router]);

  return (
    <div className="min-h-screen flex flex-col items-center justify-center bg-gray-50">
      <div className="animate-spin rounded-full h-10 w-10 border-b-2 border-blue-600 mb-4"></div>
      <p className="text-gray-500 font-medium">กำลังตรวจสอบสิทธิ์การเข้าใช้งาน...</p>
    </div>
  );
}