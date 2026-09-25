import { Smartphone, Trash2, UserCheck, Clock, Settings2, Pencil } from 'lucide-react';

interface DeviceCardProps {
  dev: any;
  onDelete: (id: string) => void;
  onEdit?: (dev: any) => void;
  onOpenSettings?: (id: string) => void;
  formatTime: (timeStr: string) => string;
}

export default function DeviceCard({ dev, onDelete, onEdit, onOpenSettings, formatTime }: DeviceCardProps) {
  return (
    <div className="p-4 bg-slate-50/90 hover:bg-slate-50 rounded-2xl border border-gray-200 shadow-xs transition relative text-left">
      {/* Header ของการ์ดอุปกรณ์: ชื่อ, สถานะ และปุ่มควบคุมที่กดง่ายบนมือถือ */}
      <div className="flex justify-between items-start mb-3 gap-2">
        <div className="flex items-center font-semibold text-gray-800 text-sm truncate">
          <div className="w-8 h-8 rounded-lg bg-blue-100 text-blue-600 flex items-center justify-center mr-2.5 flex-shrink-0">
            <Smartphone size={16} />
          </div>
          <div className="truncate">
            <p className="truncate text-sm font-bold text-gray-800">{dev.name}</p>
            <p className="text-[10px] text-gray-400 font-mono">ID: {dev.device_id}</p>
          </div>
        </div>

        {/* ปุ่มควบคุมด่วน (Settings, Edit, Delete) - แสดงให้เห็นและกดได้ง่ายทั้งมือถือและคอม */}
        <div className="flex items-center gap-1 flex-shrink-0 bg-white p-1 rounded-xl border border-gray-200 shadow-xs">
          {onOpenSettings && (
            <button
              onClick={(e) => { e.stopPropagation(); onOpenSettings(dev.device_id); }}
              className="p-1.5 text-gray-500 hover:text-indigo-600 hover:bg-indigo-50 active:bg-indigo-100 rounded-lg transition"
              title="ตั้งค่าพารามิเตอร์เซนเซอร์"
              aria-label="ตั้งค่าเซนเซอร์"
            >
              <Settings2 size={15} />
            </button>
          )}
          {onEdit && (
            <button
              onClick={(e) => { e.stopPropagation(); onEdit(dev); }}
              className="p-1.5 text-gray-500 hover:text-blue-600 hover:bg-blue-50 active:bg-blue-100 rounded-lg transition"
              title="แก้ไขข้อมูลอุปกรณ์"
              aria-label="แก้ไขอุปกรณ์"
            >
              <Pencil size={15} />
            </button>
          )}
          <button
            onClick={(e) => { e.stopPropagation(); onDelete(dev.device_id); }}
            className="p-1.5 text-gray-400 hover:text-red-600 hover:bg-red-50 active:bg-red-100 rounded-lg transition"
            title="ลบอุปกรณ์นี้"
            aria-label="ลบอุปกรณ์"
          >
            <Trash2 size={15} />
          </button>
        </div>
      </div>

      {/* สถานะการทำงานและเหตุการณ์ล่าสุด */}
      <div className="flex flex-col gap-2 p-3 bg-white rounded-xl shadow-xs border border-gray-100">
        <div className="flex items-center justify-between">
          <div className="flex items-center text-xs text-gray-500 font-medium">
            <UserCheck size={14} className="mr-1.5 text-blue-500" />
            <span>สถานะกิจกรรม:</span>
          </div>
          <span className={`text-xs font-bold px-2.5 py-0.5 rounded-full ${
            dev.latest_event === 'fall' 
              ? 'bg-red-100 text-red-600 animate-pulse font-extrabold' 
              : dev.latest_event === 'prolonged_stay'
              ? 'bg-orange-100 text-orange-700 font-bold border border-orange-200'
              : dev.latest_event === 'enter' 
              ? 'bg-amber-100 text-amber-700' 
              : dev.latest_event === 'exit' 
              ? 'bg-emerald-100 text-emerald-700' 
              : 'bg-gray-100 text-gray-600'
          }`}>
            {dev.latest_event === 'enter' 
              ? 'กำลังใช้งาน 🚪' 
              : dev.latest_event === 'prolonged_stay'
              ? '⏳ อยู่นานเกินกำหนด'
              : dev.latest_event === 'exit' 
              ? 'ห้องว่าง 🟢' 
              : dev.latest_event === 'fall' 
              ? '🚨 คนล้ม!!!' 
              : 'ยังไม่มีข้อมูล'}
          </span>
        </div>

        <div className="flex items-center justify-between text-[11px] text-gray-400 pt-1.5 border-t border-gray-50">
          <span className={`flex items-center gap-1 font-semibold ${dev.status === 'online' ? 'text-emerald-600' : 'text-gray-400'}`}>
            <span className={`w-2 h-2 rounded-full ${dev.status === 'online' ? 'bg-emerald-500' : 'bg-gray-300'}`}></span>
            {dev.status === 'online' ? 'ออนไลน์' : 'ออฟไลน์'}
          </span>
          <div className="flex items-center">
            <Clock size={11} className="mr-1" />
            <span>{formatTime(dev.UpdatedAt)}</span>
          </div>
        </div>
      </div>
    </div>
  );
}