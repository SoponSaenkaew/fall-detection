"use client";
import React from 'react';
import { X, Radio, Check } from 'lucide-react';

interface Props {
  show: boolean;
  onClose: () => void;
  onSubmit: (e: React.FormEvent) => void;
  newDev: { device_id: string; name: string };
  setNewDev: (val: any) => void;
  existingDeviceIds?: string[];
}

export default function AddDeviceModal({ show, onClose, onSubmit, newDev, setNewDev, existingDeviceIds = [] }: Props) {
  if (!show) return null;

  const isDuplicate = Boolean(
    newDev.device_id &&
    existingDeviceIds.some((id) => id?.trim().toLowerCase() === newDev.device_id.trim().toLowerCase())
  );

  return (
    <div className="fixed inset-0 bg-slate-900/60 backdrop-blur-xs flex items-center justify-center z-50 p-3 sm:p-4 text-left">
      <div className="bg-white rounded-3xl w-full max-w-sm shadow-2xl border border-slate-200 overflow-hidden animate-in fade-in zoom-in-95 duration-150">
        
        {/* Header */}
        <div className="bg-gradient-to-r from-blue-600 to-indigo-600 px-6 py-5 text-white flex items-center justify-between">
          <div className="flex items-center gap-3">
            <div className="w-10 h-10 rounded-xl bg-white/15 backdrop-blur-md flex items-center justify-center border border-white/20">
              <Radio size={20} className="text-white" />
            </div>
            <div>
              <h3 className="font-bold text-base sm:text-lg leading-tight">ลงทะเบียนเซนเซอร์ใหม่</h3>
              <p className="text-blue-100 text-xs mt-0.5">ระบุรหัสฮาร์ดแวร์และชื่อห้อง</p>
            </div>
          </div>
          <button 
            type="button" 
            onClick={onClose}
            className="w-8 h-8 rounded-full bg-white/10 hover:bg-white/20 flex items-center justify-center transition text-white"
          >
            <X size={18} />
          </button>
        </div>

        <form onSubmit={onSubmit} className="p-5 sm:p-6 space-y-4">
          <div>
            <label className="block text-xs font-bold text-slate-700 mb-1">
              รหัสฮาร์ดแวร์ (Hardware ID)
            </label>
            <input 
              type="text"
              className={`w-full text-xs font-mono border rounded-xl p-2.5 focus:outline-none transition ${
                isDuplicate 
                  ? 'border-red-400 bg-red-50/50 text-red-900 focus:ring-2 focus:ring-red-500' 
                  : 'border-slate-300 focus:ring-2 focus:ring-blue-500'
              }`}
              placeholder="เช่น B1_R101 หรือ ESP32_01" 
              value={newDev.device_id} 
              onChange={(e) => setNewDev({ ...newDev, device_id: e.target.value.trim() })} 
              required 
            />
            {isDuplicate ? (
              <p className="text-[11px] text-red-600 font-semibold mt-1 flex items-center gap-1">
                <span>⚠️ รหัสอุปกรณ์ &apos;{newDev.device_id}&apos; มีอยู่ในระบบแล้ว กรุณาใช้รหัสอื่น</span>
              </p>
            ) : (
              <p className="text-[10px] text-slate-400 mt-1">
                * ต้องตรงกับค่า Device ID ที่ตั้งค่าไว้ในเฟิร์มแวร์ ESP32 (ห้ามซ้ำกับอุปกรณ์อื่น)
              </p>
            )}
          </div>

          <div>
            <label className="block text-xs font-bold text-slate-700 mb-1">
              ชื่อจุดติดตั้ง / ห้อง
            </label>
            <input 
              type="text"
              className="w-full text-xs border border-slate-300 rounded-xl p-2.5 focus:ring-2 focus:ring-blue-500 focus:outline-none" 
              placeholder="เช่น ห้องน้ำ 101, ห้องน้ำผู้ป่วยชั้น 1" 
              value={newDev.name} 
              onChange={(e) => setNewDev({ ...newDev, name: e.target.value })} 
              required 
            />
          </div>

          <div className="flex items-center justify-end gap-2 pt-3 border-t border-slate-100">
            <button
              type="button"
              onClick={onClose}
              className="px-4 py-2 rounded-xl text-xs font-medium text-slate-600 hover:bg-slate-100 transition"
            >
              ยกเลิก
            </button>
            <button 
              type="submit" 
              disabled={isDuplicate || !newDev.device_id.trim() || !newDev.name.trim()}
              className="px-5 py-2.5 bg-blue-600 hover:bg-blue-700 disabled:opacity-50 disabled:cursor-not-allowed text-white rounded-xl text-xs font-bold transition shadow-md flex items-center gap-1.5"
            >
              <Check size={14} />
              <span>ลงทะเบียน</span>
            </button>
          </div>
        </form>
      </div>
    </div>
  );
}