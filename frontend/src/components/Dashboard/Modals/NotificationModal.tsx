"use client";
import React from 'react';
import { X, MessageSquare, Check } from 'lucide-react';

interface Props {
  show: boolean;
  onClose: () => void;
  onSubmit: (e: React.FormEvent) => void;
  form: any;
  setForm: (val: any) => void;
}

export default function NotificationModal({ show, onClose, onSubmit, form, setForm }: Props) {
  if (!show) return null;

  return (
    <div className="fixed inset-0 bg-slate-900/60 backdrop-blur-xs flex items-center justify-center z-50 p-3 sm:p-4 text-left">
      <div className="bg-white rounded-3xl w-full max-w-md shadow-2xl border border-slate-200 overflow-hidden animate-in fade-in zoom-in-95 duration-150">
        
        {/* Header */}
        <div className="bg-gradient-to-r from-emerald-600 to-teal-600 px-6 py-5 text-white flex items-center justify-between">
          <div className="flex items-center gap-3">
            <div className="w-10 h-10 rounded-xl bg-white/15 backdrop-blur-md flex items-center justify-center border border-white/20">
              <MessageSquare size={20} className="text-white" />
            </div>
            <div>
              <h3 className="font-bold text-base sm:text-lg leading-tight">ตั้งค่าการแจ้งเตือน LINE</h3>
              <p className="text-emerald-100 text-xs mt-0.5">LINE Messaging API (Push Message)</p>
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
          <div className="space-y-3.5">
            <div>
              <label className="block text-xs font-bold text-slate-700 mb-1">
                Channel Access Token (Long-lived)
              </label>
              <input 
                type="text"
                className="w-full text-xs border border-slate-300 rounded-xl p-2.5 focus:ring-2 focus:ring-emerald-500 focus:outline-none font-mono" 
                placeholder="LINE Channel Access Token..." 
                value={form.line_token || ''} 
                onChange={(e) => setForm({ ...form, type: 'line', line_token: e.target.value })} 
                required 
              />
              <p className="text-[10px] text-slate-400 mt-1">
                * สร้างจาก LINE Developers Console (Messaging API channel)
              </p>
            </div>

            <div>
              <label className="block text-xs font-bold text-slate-700 mb-1">
                LINE User ID หรือ Group ID ผู้รับ
              </label>
              <input 
                type="text"
                className="w-full text-xs border border-slate-300 rounded-xl p-2.5 focus:ring-2 focus:ring-emerald-500 focus:outline-none font-mono" 
                placeholder="เช่น U12345678... หรือ C12345678..." 
                value={form.line_group_id || ''} 
                onChange={(e) => setForm({ ...form, type: 'line', line_group_id: e.target.value })} 
                required 
              />
              <p className="text-[10px] text-slate-400 mt-1">
                * ข้อความแจ้งเตือนคนล้มหรืออยู่นานผิดปกติจะถูกส่งไปยัง ID นี้โดยตรง
              </p>
            </div>
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
              className="px-5 py-2.5 bg-emerald-600 hover:bg-emerald-700 text-white rounded-xl text-xs font-bold transition shadow-md flex items-center gap-1.5"
            >
              <Check size={14} />
              <span>บันทึกช่องทางแจ้งเตือน</span>
            </button>
          </div>
        </form>
      </div>
    </div>
  );
}