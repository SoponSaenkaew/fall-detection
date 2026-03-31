import { Smartphone, Trash2, UserCheck, Clock } from 'lucide-react';

export default function DeviceCard({ dev, onDelete, formatTime }: any) {
  return (
    <div className="p-4 bg-slate-50 rounded-xl border border-gray-100 relative group/dev">
      <button onClick={() => onDelete(dev.device_id)} className="absolute top-2 right-2 text-gray-300 hover:text-red-500 transition opacity-0 group-hover/dev:opacity-100">
        <Trash2 size={14} />
      </button>
      <div className="flex justify-between items-start mb-2 text-left">
        <div className="flex items-center font-medium text-gray-700 text-sm">
          <Smartphone size={16} className="mr-2 text-blue-400" />
          {dev.name}
        </div>
        <span className={`text-[10px] px-2 py-0.5 rounded-full font-bold uppercase ${dev.status === 'online' ? 'bg-green-100 text-green-600' : 'bg-red-100 text-red-600'}`}>
          {dev.status}
        </span>
      </div>
      <div className="flex flex-col gap-2 mt-3 p-3 bg-white rounded-lg shadow-sm border border-blue-100">
        <div className="flex items-center justify-between">
          <div className="flex items-center text-sm text-gray-600"><UserCheck size={16} className="mr-2 text-blue-400" /><span>สถานะ:</span></div>
          <span className={`text-sm font-bold ${dev.latest_event === 'fall' ? 'text-red-500 animate-bounce' : 'text-blue-600'}`}>
            {dev.latest_event === 'enter' ? 'กำลังใช้งาน 🚪' : dev.latest_event === 'exit' ? 'ว่าง 🟢' : dev.latest_event === 'fall' ? 'คนล้ม!!! ⚠️' : 'ไม่มีข้อมูล'}
          </span>
        </div>
        <div className="flex items-center justify-between text-[11px] text-gray-400">
          <div className="flex items-center"><Clock size={12} className="mr-1" />อัปเดตเมื่อ:</div>
          <span>{formatTime(dev.UpdatedAt)}</span>
        </div>
      </div>
    </div>
  );
}