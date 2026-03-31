"use client";
import { X, Settings2, Ruler, ArrowUpDown, Maximize } from 'lucide-react';

export default function DeviceSettingsModal({ show, onClose, onSubmit, settings, setSettings }: any) {
  if (!show) return null;

  return (
    <div className="fixed inset-0 bg-black/50 flex items-center justify-center z-50 p-4">
      <form onSubmit={onSubmit} className="bg-white p-6 rounded-2xl w-full max-w-md shadow-2xl border-t-4 border-indigo-500 text-left">
        <div className="flex justify-between mb-6 text-indigo-600">
          <h3 className="text-xl font-bold flex items-center"><Settings2 className="mr-2" /> ตั้งค่าพารามิเตอร์เซนเซอร์</h3>
          <button type="button" onClick={onClose}><X /></button>
        </div>

        <div className="space-y-5">
          {/* เกณฑ์การล้ม */}
          <div>
            <label className="flex items-center text-sm font-semibold text-gray-700 mb-1">
              <ArrowUpDown size={16} className="mr-2 text-indigo-400" /> เกณฑ์การตรวจจับการล้ม (Threshold)
            </label>
            <input type="number" step="0.1" className="w-full border p-2 rounded-lg" 
              value={settings.fall_threshold} 
              onChange={(e) => setSettings({...settings, fall_threshold: parseFloat(e.target.value)})} 
            />
            <p className="text-[10px] text-gray-400 mt-1">* ค่าความไวในการตัดสินใจว่าคนล้ม</p>
          </div>

          {/* ความสูงเซนเซอร์ */}
          <div>
            <label className="flex items-center text-sm font-semibold text-gray-700 mb-1">
              <Ruler size={16} className="mr-2 text-indigo-400" /> ความสูงจากพื้น (เมตร)
            </label>
            <input type="number" step="0.1" className="w-full border p-2 rounded-lg" 
              value={settings.mount_height} 
              onChange={(e) => setSettings({...settings, mount_height: parseFloat(e.target.value)})} 
            />
          </div>

          {/* ขนาดห้อง */}
          <div className="grid grid-cols-2 gap-4">
            <div>
              <label className="flex items-center text-sm font-semibold text-gray-700 mb-1">
                <Maximize size={16} className="mr-2 text-indigo-400" /> กว้าง (เมตร)
              </label>
              <input type="number" step="0.1" className="w-full border p-2 rounded-lg" 
                value={settings.room_width} 
                onChange={(e) => setSettings({...settings, room_width: parseFloat(e.target.value)})} 
              />
            </div>
            <div>
              <label className="flex items-center text-sm font-semibold text-gray-700 mb-1">
                <Maximize size={16} className="mr-2 text-indigo-400" /> ยาว (เมตร)
              </label>
              <input type="number" step="0.1" className="w-full border p-2 rounded-lg" 
                value={settings.room_length} 
                onChange={(e) => setSettings({...settings, room_length: parseFloat(e.target.value)})} 
              />
            </div>
          </div>

          <button type="submit" className="w-full bg-indigo-600 text-white py-3 rounded-xl font-bold hover:bg-indigo-700 transition shadow-lg mt-4">
            อัปเดตพารามิเตอร์ลงตัวเครื่อง ✨
          </button>
        </div>
      </form>
    </div>
  );
}