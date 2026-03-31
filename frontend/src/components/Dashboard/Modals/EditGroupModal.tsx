"use client";
import { X } from 'lucide-react';

export default function EditGroupModal({ isOpen, onClose, onSubmit, value, onChange }: any) {
  if (!isOpen) return null;
  return (
    <div className="fixed inset-0 bg-black/50 flex items-center justify-center z-50 p-4">
      <form onSubmit={onSubmit} className="bg-white p-6 rounded-xl w-full max-w-sm shadow-2xl border-t-4 border-blue-400">
        <div className="flex justify-between mb-4">
          <h3 className="text-xl font-bold">แก้ไขชื่อสถานที่ 🏠</h3>
          <button type="button" onClick={onClose}><X /></button>
        </div>
        <input 
          autoFocus className="w-full border p-2 rounded-lg mb-4 outline-none" 
          value={value} onChange={(e) => onChange(e.target.value)} required 
        />
        <button type="submit" className="w-full bg-blue-600 text-white py-2 rounded-lg font-bold">บันทึกการเปลี่ยนแปลง</button>
      </form>
    </div>
  );
}