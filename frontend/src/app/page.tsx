"use client";
import { useEffect } from 'react';
import { useRouter } from 'next/navigation';

export default function Home() {
  const router = useRouter();
  useEffect(() => {
    router.push('/buildings');
  }, [router]);

  return (
    <div className="min-h-screen flex flex-col items-center justify-center bg-gray-50">
      <p className="text-gray-500">กำลังนำคุณเข้าสู่แดชบอร์ดตรวจจับการล้ม...</p>
    </div>
  );
}