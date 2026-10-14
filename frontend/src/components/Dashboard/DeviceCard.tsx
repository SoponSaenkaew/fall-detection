"use client";
import React from 'react';
import { Radio, Settings2, Pencil, Trash2, Clock, Maximize2, Ruler, ShieldAlert } from 'lucide-react';

interface DeviceCardProps {
  dev: any;
  onDelete: (id: string) => void;
  onEdit?: (dev: any) => void;
  onOpenSettings?: (id: string) => void;
  formatTime: (timeStr: string) => string;
}

export default function DeviceCard({ dev, onDelete, onEdit, onOpenSettings, formatTime }: DeviceCardProps) {
  const isOnline = dev.status === 'online';
  const setting = dev.setting;

  return (
    <div className="bg-white hover:bg-slate-50/50 rounded-2xl border border-slate-200/90 shadow-2xs hover:shadow-xs transition p-4 sm:p-5 flex flex-col justify-between text-left">
      {/* 1. Header: ข้อมูลอุปกรณ์และสถานะการเชื่อมต่อ */}
      <div>
        <div className="flex items-start justify-between gap-3 mb-3">
          <div className="flex items-center gap-3 min-w-0">
            <div className={`w-10 h-10 rounded-xl flex items-center justify-center flex-shrink-0 ${
              isOnline ? 'bg-emerald-50 text-emerald-600 border border-emerald-200/60' : 'bg-slate-100 text-slate-400 border border-slate-200'
            }`}>
              <Radio size={20} className={isOnline ? 'animate-pulse' : ''} />
            </div>
            <div className="min-w-0">
              <h3 className="font-bold text-sm sm:text-base text-slate-800 truncate" title={dev.name}>
                {dev.name}
              </h3>
              <p className="text-[11px] text-slate-400 font-mono tracking-wide truncate">
                ID: {dev.device_id}
              </p>
            </div>
          </div>

          {/* สถานะ ออนไลน์ / ออฟไลน์ */}
          <span className={`inline-flex items-center gap-1.5 px-2.5 py-1 rounded-full text-xs font-semibold flex-shrink-0 ${
            isOnline 
              ? 'bg-emerald-50 text-emerald-700 border border-emerald-200/80' 
              : 'bg-slate-100 text-slate-500 border border-slate-200'
          }`}>
            <span className={`w-2 h-2 rounded-full ${isOnline ? 'bg-emerald-500 animate-ping' : 'bg-slate-400'}`}></span>
            <span>{isOnline ? 'ออนไลน์' : 'ออฟไลน์'}</span>
          </span>
        </div>

        {/* 2. Glanceable Config: สรุปพารามิเตอร์ที่ใช้อยู่ปัจจุบัน */}
        <div className="bg-slate-50/80 rounded-xl p-3 border border-slate-200/60 mb-4 space-y-2">
          <div className="text-[10px] font-bold text-slate-400 uppercase tracking-wider flex items-center justify-between">
            <span>การตั้งค่าปัจจุบัน (Current Config)</span>
            <span className="flex items-center gap-1 text-slate-400 font-normal">
              <Clock size={10} />
              {formatTime(dev.UpdatedAt)}
            </span>
          </div>

          <div className="grid grid-cols-2 gap-2 text-xs">
            <div className="flex items-center gap-1.5 text-slate-600 bg-white p-2 rounded-lg border border-slate-100">
              <Maximize2 size={13} className="text-blue-500 flex-shrink-0" />
              <span className="truncate">
                ห้อง: <strong className="text-slate-800">{setting?.room_width ?? 3.0}×{setting?.room_length ?? 4.0}</strong> ม.
              </span>
            </div>
            <div className="flex items-center gap-1.5 text-slate-600 bg-white p-2 rounded-lg border border-slate-100">
              <Ruler size={13} className="text-indigo-500 flex-shrink-0" />
              <span className="truncate">
                สูง: <strong className="text-slate-800">{setting?.mount_height ?? 2.0}</strong> ม.
              </span>
            </div>
            <div className="flex items-center gap-1.5 text-slate-600 bg-white p-2 rounded-lg border border-slate-100">
              <Clock size={13} className="text-amber-500 flex-shrink-0" />
              <span className="truncate">
                อยู่นาน: <strong className="text-slate-800">{setting?.stay_timeout_minutes ?? 30}</strong> น.
              </span>
            </div>
            <div className="flex items-center gap-1.5 text-slate-600 bg-white p-2 rounded-lg border border-slate-100">
              <ShieldAlert size={13} className="text-rose-500 flex-shrink-0" />
              <span className="truncate">
                เกณฑ์ล้ม: <strong className="text-slate-800">{setting?.fall_threshold ?? 0.5}</strong>
              </span>
            </div>
          </div>
        </div>
      </div>

      {/* 3. Action Buttons: ปุ่มตั้งค่าเด่นชัด และปุ่มจัดการ */}
      <div className="flex items-center gap-2 pt-1 border-t border-slate-100">
        {onOpenSettings && (
          <button
            type="button"
            onClick={() => onOpenSettings(dev.device_id)}
            className="flex-1 py-2 px-3 bg-gradient-to-r from-blue-600 to-indigo-600 hover:from-blue-700 hover:to-indigo-700 active:scale-[0.99] text-white rounded-xl text-xs font-bold shadow-xs transition flex items-center justify-center gap-1.5"
            title="ตั้งค่าพารามิเตอร์เซนเซอร์"
          >
            <Settings2 size={14} />
            <span>ตั้งค่าเซนเซอร์</span>
          </button>
        )}

        {onEdit && (
          <button
            type="button"
            onClick={() => onEdit(dev)}
            className="p-2 text-slate-600 hover:text-blue-600 hover:bg-blue-50 border border-slate-200 rounded-xl transition flex-shrink-0"
            title="แก้ไขชื่ออุปกรณ์"
            aria-label="แก้ไขชื่ออุปกรณ์"
          >
            <Pencil size={14} />
          </button>
        )}

        <button
          type="button"
          onClick={() => onDelete(dev.device_id)}
          className="p-2 text-slate-400 hover:text-red-600 hover:bg-red-50 border border-slate-200 rounded-xl transition flex-shrink-0"
          title="ลบอุปกรณ์นี้"
          aria-label="ลบอุปกรณ์"
        >
          <Trash2 size={14} />
        </button>
      </div>
    </div>
  );
}