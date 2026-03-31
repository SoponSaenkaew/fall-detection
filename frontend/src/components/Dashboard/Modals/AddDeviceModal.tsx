"use client";
import { X, SmartphoneNfc } from 'lucide-react';

interface Props {
  show: boolean;
  onClose: () => void;
  onSubmit: (e: React.FormEvent) => void;
  newDev: { device_id: string; name: string };
  setNewDev: (val: any) => void;
}

export default function AddDeviceModal({ show, onClose, onSubmit, newDev, setNewDev }: Props) {
  if (!show) return null;
  return (
    <div className="fixed inset-0 bg-black/50 flex items-center justify-center z-50 p-4 text-left">
      <form onSubmit={onSubmit} className="bg-white p-6 rounded-2xl w-full max-w-sm shadow-2xl border-t-4 border-green-500">
        <div className="flex justify-between mb-4 text-green-700">
          <h3 className="text-xl font-bold flex items-center"><SmartphoneNfc className="mr-2" /> เพิ่มเซนเซอร์ใหม่</h3>
          <button type="button" onClick={onClose}><X /></button>
        </div>
        <div className="space-y-4">
          <input 
            className="w-full border p-2 rounded-lg outline-none focus:ring-2 focus:ring-green-400" 
            placeholder="Hardware ID" 
            value={newDev.device_id} 
            onChange={(e) => setNewDev({...newDev, device_id: e.target.value})} 
            required 
          />
          <input 
            className="w-full border p-2 rounded-lg outline-none focus:ring-2 focus:ring-green-400" 
            placeholder="ชื่อเรียก" 
            value={newDev.name} 
            onChange={(e) => setNewDev({...newDev, name: e.target.value})} 
            required 
          />
          <button type="submit" className="w-full bg-green-600 text-white py-2 rounded-lg font-bold hover:bg-green-700 transition">ลงทะเบียนอุปกรณ์ ✨</button>
        </div>
      </form>
    </div>
  );
}