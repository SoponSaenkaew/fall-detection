"use client";
import { X, BellRing } from 'lucide-react';

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
    <div className="fixed inset-0 bg-black/50 flex items-center justify-center z-50 p-4 text-left">
      <form onSubmit={onSubmit} className="bg-white p-6 rounded-2xl w-full max-w-md shadow-2xl border-t-4 border-yellow-400">
        <div className="flex justify-between mb-4 text-yellow-600">
          <h3 className="text-xl font-bold flex items-center"><BellRing className="mr-2" /> ตั้งค่าการแจ้งเตือน</h3>
          <button type="button" onClick={onClose} className="text-gray-400"><X /></button>
        </div>
        <div className="space-y-4">
          <select 
            className="w-full border p-2 rounded-lg outline-none" 
            value={form.type} 
            onChange={(e) => setForm({...form, type: e.target.value})}
          >
            <option value="line">LINE Notify / Messaging API</option>
            <option value="webhook">Webhook URL (Discord)</option>
          </select>
          {form.type === 'line' ? (
            <>
              <input className="w-full border p-2 rounded-lg" placeholder="LINE Token" value={form.line_token} onChange={(e) => setForm({...form, line_token: e.target.value})} required />
              <input className="w-full border p-2 rounded-lg" placeholder="LINE User/Group ID" value={form.line_group_id} onChange={(e) => setForm({...form, line_group_id: e.target.value})} required />
            </>
          ) : (
            <input className="w-full border p-2 rounded-lg" placeholder="Webhook URL" value={form.target_url} onChange={(e) => setForm({...form, target_url: e.target.value})} required />
          )}
          <button type="submit" className="w-full bg-yellow-500 text-white py-2 rounded-lg font-bold hover:bg-yellow-600 transition">บันทึกการตั้งค่า ✨</button>
        </div>
      </form>
    </div>
  );
}