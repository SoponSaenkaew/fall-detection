"use client";
import React from 'react';
import { X, Pencil, Check } from 'lucide-react';

interface Props {
  show: boolean;
  onClose: () => void;
  onSubmit: (e: React.FormEvent) => void;
  devData: { device_id: string; name: string };
  setDevData: (val: any) => void;
  originalId?: string | null;
  existingDeviceIds?: string[];
}

export default function EditDeviceModal({ 
  show, 
  onClose, 
  onSubmit, 
  devData, 
  setDevData,
  originalId,
  existingDeviceIds = [] 
}: Props) {
  if (!show) return null;

  const isDuplicate = Boolean(
    devData.device_id &&
    originalId &&
    devData.device_id.trim().toLowerCase() !== originalId.toLowerCase() &&
    existingDeviceIds.some((id) => id?.trim().toLowerCase() === devData.device_id.trim().toLowerCase())
  );

  return (
    <div className="fixed inset-0 bg-slate-900/60 backdrop-blur-xs flex items-center justify-center z-50 p-3 sm:p-4 text-left">
      <div className="bg-white rounded-3xl w-full max-w-sm shadow-2xl border border-slate-200 overflow-hidden animate-in fade-in zoom-in-95 duration-150">
        
        {/* Header */}
        <div className="bg-gradient-to-r from-blue-600 to-indigo-600 px-6 py-5 text-white flex items-center justify-between">
          <div className="flex items-center gap-3">
            <div className="w-10 h-10 rounded-xl bg-white/15 backdrop-blur-md flex items-center justify-center border border-white/20">
              <Pencil size={20} className="text-white" />
            </div>
            <div>
              <h3 className="font-bold text-base sm:text-lg leading-tight">แก้ไขข้อมูลอุปกรณ์</h3>
              <p className="text-blue-100 text-xs mt-0.5">เปลี่ยนชื่อเรียกหรือรหัสอุปกรณ์</p>
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
              value={devData.device_id || ''} 
              onChange={(e) => setDevData({ ...devData, device_id: e.target.value.trim() })} 
              required 
            />
            {isDuplicate && (
              <p className="text-[11px] text-red-600 font-semibold mt-1 flex items-center gap-1">
                <span>⚠️ รหัสอุปกรณ์ &apos;{devData.device_id}&apos; มีอยู่ในระบบแล้ว กรุณาใช้รหัสอื่น</span>
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
              value={devData.name || ''} 
              onChange={(e) => setDevData({ ...devData, name: e.target.value })} 
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
              disabled={isDuplicate || !devData.device_id.trim() || !devData.name.trim()}
              className="px-5 py-2.5 bg-blue-600 hover:bg-blue-700 disabled:opacity-50 disabled:cursor-not-allowed text-white rounded-xl text-xs font-bold transition shadow-md flex items-center gap-1.5"
            >
              <Check size={14} />
              <span>บันทึกการแก้ไข</span>
            </button>
          </div>
        </form>
      </div>
    </div>
  );
}