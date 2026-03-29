"use client";
import { useEffect, useState } from 'react';
import api from '@/lib/api';
import { Group } from '@/types/iot';

export default function DashboardPage() {
  const [groups, setGroups] = useState<Group[]>([]);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    const fetchDashboard = async () => {
      try {
        // ดึงข้อมูลกลุ่มทั้งหมดที่เซนเซย์เป็นเจ้าของ
        const res = await api.get('/iot/groups');
        setGroups(res.data);
      } catch (err) {
        console.error("ดึงข้อมูลไม่สำเร็จค่ะเซนเซย์", err);
      } finally {
        setLoading(false);
      }
    };
    fetchDashboard();
  }, []);

  if (loading) return <div className="p-8 text-center text-blue-500">กำลังโหลดข้อมูลอุปกรณ์ให้อยู่นะคะ...</div>;

  return (
    <div className="p-6 max-w-6xl mx-auto">
      <header className="flex justify-between items-center mb-8">
        <h1 className="text-3xl font-bold text-gray-800">แผงควบคุม IoT</h1>
        <button className="bg-blue-600 text-white px-4 py-2 rounded-lg hover:bg-blue-700 transition">
          + เพิ่มกลุ่มใหม่
        </button>
      </header>

      <div className="grid gap-6">
        {groups.map((group) => (
          <div key={group.ID} className="bg-white rounded-2xl shadow-sm border p-6">
            <h2 className="text-xl font-bold mb-4 text-blue-600">🏠 {group.name}</h2>
            <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
              {group.devices?.map((dev) => (
                <div key={dev.ID} className="p-4 rounded-xl border bg-gray-50 flex justify-between items-center">
                  <div>
                    <p className="font-semibold">{dev.name}</p>
                    <p className="text-sm text-gray-500">{dev.device_id}</p>
                  </div>
                  <span className={`px-2 py-1 rounded-full text-xs font-bold ${
                    dev.status === 'online' ? 'bg-green-100 text-green-600' : 'bg-red-100 text-red-600'
                  }`}>
                    {dev.status}
                  </span>
                </div>
              ))}
              {(!group.devices || group.devices.length === 0) && (
                <p className="text-gray-400 text-sm italic">ยังไม่มีอุปกรณ์ในกลุ่มนี้ค่ะ</p>
              )}
            </div>
          </div>
        ))}
      </div>
    </div>
  );
}