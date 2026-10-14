"use client";
import React from 'react';
import { X, Building2, Check } from 'lucide-react';

interface Props {
  isOpen: boolean;
  onClose: () => void;
  onSubmit: (e: React.FormEvent) => void;
  value: string;
  onChange: (val: string) => void;
}

export default function EditGroupModal({ isOpen, onClose, onSubmit, value, onChange }: Props) {
  if (!isOpen) return null;

  return (
    <div className="fixed inset-0 bg-slate-900/60 backdrop-blur-xs flex items-center justify-center z-50 p-3 sm:p-4 text-left">
      <div className="bg-white rounded-3xl w-full max-w-sm shadow-2xl border border-slate-200 overflow-hidden animate-in fade-in zoom-in-95 duration-150">
        
        {/* Header */}
        <div className="bg-gradient-to-r from-blue-600 to-indigo-600 px-6 py-5 text-white flex items-center justify-between">
          <div className="flex items-center gap-3">
            <div className="w-10 h-10 rounded-xl bg-white/15 backdrop-blur-md flex items-center justify-center border border-white/20">
              <Building2 size={20} className="text-white" />
            </div>
            <div>
              <h3 className="font-bold text-base sm:text-lg leading-tight">แก้ไขชื่อสถานที่</h3>
              <p className="text-blue-100 text-xs mt-0.5">เปลี่ยนชื่ออาคารหรือกลุ่ม</p>
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
              ชื่อสถานที่ / อาคาร
            </label>
            <input 
              autoFocus 
              type="text"
              className="w-full text-xs border border-slate-300 rounded-xl p-2.5 focus:ring-2 focus:ring-blue-500 focus:outline-none" 
              value={value || ''} 
              onChange={(e) => onChange(e.target.value)} 
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
              className="px-5 py-2.5 bg-blue-600 hover:bg-blue-700 text-white rounded-xl text-xs font-bold transition shadow-md flex items-center gap-1.5"
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