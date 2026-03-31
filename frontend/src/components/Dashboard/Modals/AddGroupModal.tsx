"use client";
import { X } from 'lucide-react';

interface Props {
  isOpen: boolean;
  onClose: () => void;
  onSubmit: (e: React.FormEvent) => void;
  value: string;
  onChange: (val: string) => void;
}

export default function AddGroupModal({ isOpen, onClose, onSubmit, value, onChange }: Props) {
  if (!isOpen) return null;
  return (
    <div className="fixed inset-0 bg-black/50 flex items-center justify-center z-50 p-4 text-left">
      <form onSubmit={onSubmit} className="bg-white p-6 rounded-xl w-full max-w-sm shadow-2xl border-t-4 border-blue-500">
        <div className="flex justify-between mb-4 italic text-blue-600">
          <h3 className="text-xl font-bold">สร้างสถานที่ใหม่นะเซนเซย์! 🏠</h3>
          <button type="button" onClick={onClose} className="text-gray-400 hover:text-black"><X /></button>
        </div>
        <input 
          autoFocus 
          className="w-full border p-2 rounded-lg mb-4 outline-none focus:ring-2 focus:ring-blue-400" 
          placeholder="เช่น ห้องน้ำชั้น 2" 
          value={value} 
          onChange={(e) => onChange(e.target.value)} 
          required 
        />
        <button type="submit" className="w-full bg-blue-600 text-white py-2 rounded-lg font-bold hover:bg-blue-700 transition">ยืนยัน ✨</button>
      </form>
    </div>
  );
}