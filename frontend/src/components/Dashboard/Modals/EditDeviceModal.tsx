"use client";
import { X, Settings } from 'lucide-react';

export default function EditDeviceModal({ show, onClose, onSubmit, devData, setDevData }: any) {
  if (!show) return null;
  return (
    <div className="fixed inset-0 bg-black/50 flex items-center justify-center z-50 p-4">
      <form onSubmit={onSubmit} className="bg-white p-6 rounded-2xl w-full max-w-sm shadow-2xl border-t-4 border-green-500">
        <div className="flex justify-between mb-4 text-green-700">
          <h3 className="text-xl font-bold flex items-center"><Settings className="mr-2" /> แก้ไขข้อมูลเซนเซอร์</h3>
          <button type="button" onClick={onClose}><X /></button>
        </div>
        <div className="space-y-4">
          <div>
            <label className="text-xs text-gray-400">Hardware ID (ไม่ควรเปลี่ยนถ้าไม่จำเป็น)</label>
            <input className="w-full border p-2 rounded-lg outline-none" value={devData.device_id} onChange={(e) => setDevData({...devData, device_id: e.target.value})} required />
          </div>
          <div>
            <label className="text-xs text-gray-400">ชื่อเรียกอุปกรณ์</label>
            <input className="w-full border p-2 rounded-lg outline-none" value={devData.name} onChange={(e) => setDevData({...devData, name: e.target.value})} required />
          </div>
          <button type="submit" className="w-full bg-green-600 text-white py-2 rounded-lg font-bold">อัปเดตข้อมูล ✨</button>
        </div>
      </form>
    </div>
  );
}